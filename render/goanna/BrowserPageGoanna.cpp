/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser &mdash; BrowserPageGoanna implementation. See BrowserPageGoanna.h.
 * Bridges the YAP BrowserPage command surface to GoannaRenderPage and emits the
 * server->adapter messages via IPageMessageSink.
 */
#include "BrowserPageGoanna.h"
#include "EngineCompat.h"
#include "EngineHost.h"
#include "GoannaRenderPage.h"
#include "BrowserOffscreenInfo.h"   // isis shmem header (from render/browserserver/Src)
#include "JihadLogo.h"              // JIHAD_LOGO_B64 (app icon for about: pages)
#include "JihadRuntimePaths.h"      // the ONE runtime-state dir (T-057 / R8)
#include "JihadCrashGuard.h"        // do not reopen a page that just killed the daemon twice
#include "JihadCertStore.h"         // webOS platform certificate store (R5)

#include <cstdio>
#include <sys/time.h>   // gettimeofday (paint render timing)
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <regex.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>   // chmod (popup menu data file 0644), mkfifo (dialog reply pipe)
#include <fcntl.h>      // open() the dialog reply FIFO
#include <sys/statvfs.h> // room on /tmp before an on-request frame dump
#include <poll.h>       // wait for the card's answer with a deadline
#include <unistd.h>
#include <pthread.h>    // Jihad audio-scenario poll thread
#include <sys/wait.h>   // waitpid for the detached audio-route helper
#include <cerrno>
#include <cstring>
#include <algorithm>   // std::remove (live-page registry)
#include <vector>

namespace { }  // (UrlRule defined below in the jihad namespace)

namespace jihad {

class BrowserPageGoanna;
// Every page that is alive, oldest first. Its only job is to decide who owns the process-wide
// dialog sink when a page goes away (see the constructor and destructor).
static std::vector<BrowserPageGoanna*>& jihadLivePages() {
  static std::vector<BrowserPageGoanna*> pages;
  return pages;
}

// How long after the last adapter scroll/zoom update an engine-driven repaint is held back. Long
// enough to cover the gaps between setScrollPosition messages during a fling, short enough that a
// page which updates itself while the user rests a finger still refreshes promptly.
// The scroll-settle gate constant is gone with the gate (see maybePaint). mLastScrollMs
// remains the "a pan is in flight" signal for echo suppression and fling-mode painting.

// A compiled URL redirect rule (POSIX regex &mdash; exception-free, unlike std::regex,
// which matters under -fno-exceptions).
struct BrowserPageGoanna::UrlRule {
  regex_t re;
  std::string userData;
  bool redirect;
};

// ── Jihad/webOS media audio routing ──────────────────────────────────────────────────────────
// webOS's audiod never routes OUR stream to the built-in speaker (it only manages registered media
// apps), and it will not scale our volume either — its media volume lives in pulse and only bites a
// stream it classified as media, which the forced route below deliberately is not. So the daemon
// drives the WM8994 mixer directly: while engine media plays it sets the speaker output level
// (numid 74/86) from the system media volume and closes the DAC1->speaker switches; at rest it
// mutes the speaker amp (numid 87) so the NEXT playback is silent until the level is set, which is
// what kills the max-volume burst. libcubeb (linked into this daemon) counts PLAYING output streams
// in jihad_pulse_active_streams(); a background thread polls it and runs the scripts on the 0<->N
// transition. The scripts live in the daemon's state dir and are (re)written from the constants
// below on every startup — edit them HERE, not on the device (a restart overwrites device edits).
// All best-effort: absent symbol / no state dir / exec failure => silent no-op. Because the amp is
// shared with system sounds, audio-off only mutes it and audiod turns it back on for its own
// scenarios (notifications/calls). See context/impl/impl-audio-backend.md. UNVERIFIED gaps: the
// system volume OSD does not track these direct mixer writes, and video-controls taps are the
// separate offscreen hit-test residual.
// jihad_pulse_active_streams: EngineCompat.h (optional engine export).

// application.name (not media.role): keeps our cubeb stream OFF the pmedia media-policy sink and on
// pcm_output (the real DAC), which is what the forced speaker mixer actually drives. media.role
// would send it to a null policy sink the mixer route never reaches.
static const char* const kJihadPulseProps = "application.name=Jihad Browser\n";

static const char* const kJihadAudioOnScript =
  "#!/bin/sh\n"
  "# Written by the daemon (BrowserPageGoanna.cpp). Runs when engine media starts. Sets the WM8994\n"
  "# speaker output level from the system media volume, then routes DAC1 -> speaker.\n"
  "A=\"/usr/bin/amixer -c 0\"; L=\"/usr/bin/luna-send -n 1\"\n"
  "V=$($L palm://com.palm.audio/media/getVolume '{}' 2>/dev/null | sed -n 's/.*\"volume\":\\([0-9]\\{1,3\\}\\).*/\\1/p')\n"
  "[ -z \"$V\" ] && V=50\n"
  "lvl=$((V*63/100))\n"
  "$A cset numid=74 $lvl >/dev/null 2>&1\n"   // Output Volume  (0-63)
  "$A cset numid=86 $lvl >/dev/null 2>&1\n"   // Speaker Volume (0-63)
  "$A cset numid=191 on >/dev/null 2>&1\n"    // SPKL DAC1 Switch
  "$A cset numid=186 on >/dev/null 2>&1\n"    // SPKR DAC1 Switch
  "$A cset numid=237 on >/dev/null 2>&1\n"    // SPKL Boost SPKL Switch
  "$A cset numid=235 on >/dev/null 2>&1\n"    // SPKR Boost SPKR Switch
  "$A cset numid=87  on >/dev/null 2>&1\n"    // Speaker Switch
  "exit 0\n";

static const char* const kJihadAudioOffScript =
  "#!/bin/sh\n"
  "# Written by the daemon. Runs at startup and when engine media stops. Mutes the speaker amp so\n"
  "# the next playback stays silent until audio-on.sh sets the level (kills the max-volume burst).\n"
  "# audiod re-enables the amp for its own scenarios, so system sounds are unaffected.\n"
  "/usr/bin/amixer -c 0 cset numid=87 off >/dev/null 2>&1\n"
  "exit 0\n";

// Runs repeatedly WHILE media plays. audiod moves a pulse volume that our forced pcm_output->speaker
// route bypasses, so the level would otherwise freeze at its playback-start value; mirror the live
// system media volume onto the speaker output level (numid 74/86) so volume-button presses track.
static const char* const kJihadAudioVolScript =
  "#!/bin/sh\n"
  "A=\"/usr/bin/amixer -c 0\"; L=\"/usr/bin/luna-send -n 1\"\n"
  "V=$($L palm://com.palm.audio/media/getVolume '{}' 2>/dev/null | sed -n 's/.*\"volume\":\\([0-9]*\\).*/\\1/p')\n"
  "[ -z \"$V\" ] && exit 0\n"
  "lvl=$((V*63/100))\n"
  "$A cset numid=74 $lvl >/dev/null 2>&1\n"
  "$A cset numid=86 $lvl >/dev/null 2>&1\n"
  "exit 0\n";

static void jihadWriteFile(const std::string& path, const char* content, mode_t mode) {
  if (path.empty()) { return; }
  FILE* f = fopen(path.c_str(), "w");
  if (!f) { return; }
  fwrite(content, 1, strlen(content), f);
  fclose(f);
  chmod(path.c_str(), mode);
}

static void jihadRunAudioScript(const char* script) {
  // fork twice so the grandchild (the script) is reparented to init: no zombie to reap, and we
  // never waitpid(-1) which would steal plugin-container's exit. Only async-signal-safe calls
  // between fork and exec. Clean env: an inherited (bundled glibc-2.23) LD_LIBRARY_PATH segfaults
  // /bin/sh + the device luna-send/amixer, so hand exec a minimal device PATH and nothing else.
  pid_t pid = fork();
  if (pid == 0) {
    if (fork() == 0) {
      char* const env[] = { (char*)"PATH=/usr/bin:/bin:/usr/sbin:/sbin", nullptr };
      execle("/bin/sh", "sh", script, (char*)nullptr, env);
      _exit(127);
    }
    _exit(0);
  } else if (pid > 0) {
    waitpid(pid, nullptr, 0);   // reap only our short-lived intermediate child
  }
}

static void* jihadAudioPollThread(void*) {
  const std::string& sd = jihad::RuntimeStateDir();
  if (sd.empty()) { return nullptr; }
  const std::string onp = sd + "/audio-on.sh";
  const std::string offp = sd + "/audio-off.sh";
  const std::string volp = sd + "/audio-vol.sh";
  int prev = 0, tick = 0;
  for (;;) {
    usleep(200 * 1000);
    if (!jihad_pulse_active_streams) { continue; }
    int n = jihad_pulse_active_streams();
    if (n > 0 && prev == 0) { jihadRunAudioScript(onp.c_str()); tick = 0; }
    else if (n == 0 && prev > 0) { jihadRunAudioScript(offp.c_str()); }
    else if (n > 0 && (++tick % 2) == 0) { jihadRunAudioScript(volp.c_str()); }  // ~400ms live track
    prev = n;
  }
  return nullptr;
}

static void jihadStartAudioPoll() {
  static bool started = false;
  if (started) { return; }
  started = true;
  const std::string& sd = jihad::RuntimeStateDir();
  if (sd.empty()) { return; }   // desktop / no state dir: inert
  // Export the resolved dir so libcubeb (same process, but it cannot call RuntimeStateDir) finds
  // pulse.props via getenv; setenv(,,0) leaves an explicit harness override in place. Provision all
  // three files and mute the amp SYNCHRONOUSLY here, in the page constructor, so pulse.props exists
  // before cubeb opens any stream and the resting state is muted before the first playback — only
  // then hand the steady-state polling to a background thread.
  setenv("JIHAD_STATE_DIR", sd.c_str(), 0);
  jihadWriteFile(sd + "/pulse.props", kJihadPulseProps, 0644);
  jihadWriteFile(sd + "/audio-on.sh", kJihadAudioOnScript, 0755);
  jihadWriteFile(sd + "/audio-vol.sh", kJihadAudioVolScript, 0755);
  const std::string offp = sd + "/audio-off.sh";
  jihadWriteFile(offp, kJihadAudioOffScript, 0755);
  jihadRunAudioScript(offp.c_str());
  pthread_t t;
  if (pthread_create(&t, nullptr, jihadAudioPollThread, nullptr) == 0) { pthread_detach(t); }
}

BrowserPageGoanna::BrowserPageGoanna(EngineHost& host, IPageMessageSink& sink)
  : mHost(host), mSink(sink), mPage(nullptr),
    mKey1(0), mKey2(0), mBufSize(0), mActiveKey(0),
    mLoadWasDone(false), mNeedsPaint(false),
    mLastContentW(-1), mLastContentH(-1),
    mLastScrollX(-1), mLastScrollY(-1), mZoom(1.0),
    mAdapterScrollX(0), mAdapterScrollY(0), mFrozen(false), mHadContent(false), mBlankSinceMs(0), mBlankRetryAtMs(0), mGeometryDirty(false), mGeometryTries(0),
    mNavGen(0), mLastScrollMs(0) {
  for (int i = 0; i < 3; ++i) { mShmBuf[i] = nullptr; mShmId[i] = -1; }
  jihadStartAudioPoll();   // Jihad: hold the audiod media scenario + speaker route during playback

  mTouchSeqActive = false; mTouchSeqConsumed = false;
  mTouchAnchorX = 0; mTouchAnchorY = 0;
  mTouchSeqStartMs = 0; mTouchSuppressUntilMs = 0;
  mInFlight[0] = mInFlight[1] = false; mPaintMs[0] = mPaintMs[1] = 0;
  mLastBackspaceMs = 0; mBackspaceRun = 0;
  mLoadStartMs = 0;
  mLastProgress = 0;
  mWatchdogDismissed = false;
  mDirtyPending = false; mLastDirtyPaintMs = 0; mPaceDeadlineMs = 0; mLastPluginFrameSeq = 0;
  mLastRequestedSeq = 0; mPluginReqMs = 0; mPluginReqTicks = 0; mPluginReqOutstanding = false;
  // Engine dialogs are process-wide (one prompter service), so the LAST page created owns
  // them — the same single-page-embedding assumption DebugRunChromeJs already makes. Without
  // this nothing implements DialogSink and every dialog silently takes its default.
  jihadLivePages().push_back(this);
  SetDialogSink(this);
  // Session certificates a previous daemon died holding are removed here, before
  // this page can install any of its own. It is idempotent and runs on the first
  // page ONLY, which is the earliest point this file owns; a daemon that never
  // creates a page also never installs a certificate, so nothing is missed.
  jihad::certstore::InitAndSweepStale();
}

// Deferred editing keys (run in pump(), not the keyDown YAP callback — Codex F-219). Enter may
// submit a form and navigate; Tab in an <input> focuses another field (fires focus/blur JS). Both
// must run in the page-lifetime-protected pump loop. (Tab in a <textarea> just inserts a tab, but
// HandleTab decides that, so all Tabs defer for uniformity.)
enum { PEA_NONE = 0, PEA_TAB = 1, PEA_TAB_BACK = 2, PEA_ENTER = 3 };

// Minimum gap between two damage-driven repaints.
//
// 8 ms, deliberately BELOW the tick period rather than at the target frame time. A floor of
// 33 ms looks like "cap at 30 fps" but a paint can only happen on a tick, and the tick runs at
// ~26 ms under load — so a 33 ms floor pushed every second frame to the following tick and
// produced a 26/52 ms beat: 22 fps average made of alternating fast and slow frames, which is
// exactly what uneven motion looks like. Measured before: gap min 34 avg 45 max 78.
// Keeping the floor under the tick lets every tick that has damage deliver, so the pacing is
// set by the damage rate and the tick, not by a beat between two similar periods. The real
// throttles remain: the in-flight buffer guard, and the cost of the frame itself.
static const int kJihadDirtyPaintMinMs = 8;

// Frame PACING for plugin-driven animation, on a fixed grid rather than a floor.
//
// The floor above is right for ordinary page damage, where a repaint should go out as soon as
// it can, but it is wrong for a plugin, which produces a CONTINUOUS stream of frames. Damage
// from an out-of-process plugin arrives whenever its IPC lands, which bears no relation to our
// 16 ms tick, so "paint on any tick that has damage" delivers frames at essentially random
// instants. Device-measured 2026-08-10 on a 30 fps SWF, and this is the whole diagnosis: the
// gap histogram was FLAT across every bucket from under 16 ms to 55 ms (<16=19 16-23=20
// 24-31=10 32-39=24 40-55=6) at a healthy 35-42 fps average. The compositor then held each
// frame for anywhere between half a refresh and three, which is what a viewer reads as stutter
// even though no frame was ever dropped and the average was ABOVE the content's own rate.
//
// A DEADLINE, not a floor. The distinction is the reason the earlier attempt failed: a floor
// measured from the last actual publish accumulates every late paint into the next interval, so
// it drifts and beats against the tick (the 2026-08 note above records a 26/52 ms alternation
// from exactly that). A deadline advances on its own fixed grid, so a late frame is followed by
// an early one and the spacing averages the target instead of walking away from it.
//
// 33 ms because it is two refreshes of a 60 Hz compositor and because Flash content is
// essentially all 12-30 fps; snapping to it means every frame is held the SAME number of
// refreshes. That trades a few fps of average for even spacing, which is the trade the eye
// wants. $JIHAD_PLUGIN_FRAME_MS overrides it (0 disables the pacer entirely and restores the
// old free-running behaviour) so the period can be A/B'd on the device without a rebuild.
// How long damage may sit unpainted on a plugin page before it is published regardless of the
// plugin. 250 ms is a quarter second of latency for a caret or an image decode in the worst
// case, and it is ~8 plugin periods, so it cannot fire while anything is animating.
static const int kJihadDirtyStaleMs = 250;

static int jihadPluginFramePeriodMs() {
  static int s_ms = -1;
  if (s_ms < 0) {
    const char* e = getenv("JIHAD_PLUGIN_FRAME_MS");
    s_ms = (e && *e) ? atoi(e) : 33;
    if (s_ms < 0) s_ms = 0;
  }
  return s_ms;
}

static uint32_t gJihadPaintWanted = 0;
static uint32_t gJihadPaintDeferInFlight = 0;
static uint32_t gJihadPaintDone = 0;
static uint32_t gJihadPaintPartial = 0;
// Why a frame took the FULL path instead of a partial one (reported next to partial=):
// nodmg = no damage box, force = a forced full repaint, slot = no valid buffer for this key,
// geom = the buffer's size/zoom/strip/scroll differ, small = the box was neither worth a
// partial nor eligible for band-only.
static uint32_t gJihadPartialMiss[5] = {0, 0, 0, 0, 0};
static uint32_t gJihadTicks = 0;
static uint32_t gJihadDirtyTrue = 0;

// Frame PACING, not frame rate. An average hides the thing a viewer actually notices: a run
// of 30 ms frames with an occasional 200 ms stall reads as stutter even though it averages
// well. This keeps the spread between consecutive delivered frames.
static int64_t  gJihadPrevFrameMs = 0;
static uint32_t gJihadGapMin = 0, gJihadGapMax = 0, gJihadGapN = 0;
static uint64_t gJihadGapSum = 0;

// Frame-gap HISTOGRAM, because min/avg/max cannot show the failure this is looking for.
// A publisher free-running near the content's own rate delivers a BEAT: most frames one tick
// apart and a few two ticks apart. That reads as stutter to a viewer while producing a healthy
// average and an unremarkable max, so the summary statistics all look fine. A bimodal bucket
// count is the signature, and it is the only cheap way to tell "evenly late" from "unevenly
// on time". Buckets are centred on the 33 ms a 30 fps plugin wants: everything in one bucket
// is smooth, weight split across two non-adjacent buckets is judder.
enum { kGapBuckets = 6 };
static uint32_t gJihadGapHist[kGapBuckets] = {0};
static const char* const kGapLabels[kGapBuckets] =
  { "<16", "16-23", "24-31", "32-39", "40-55", "56+" };
static void JihadRecordFrameGap(int64_t aNowMs)
{
  if (gJihadPrevFrameMs) {
    uint32_t gap = (uint32_t)(aNowMs - gJihadPrevFrameMs);
    if (!gJihadGapN || gap < gJihadGapMin) gJihadGapMin = gap;
    if (gap > gJihadGapMax) gJihadGapMax = gap;
    gJihadGapSum += gap;
    gJihadGapN++;
    gJihadGapHist[gap < 16 ? 0 : gap < 24 ? 1 : gap < 32 ? 2
                : gap < 40 ? 3 : gap < 56 ? 4 : 5]++;
  }
  gJihadPrevFrameMs = aNowMs;
}

// Monotonic-enough wall clock in ms.
//
// int64_t, NOT long. `long` is 32 BITS on this ARM target, and tv_sec*1000 for any date after
// 1970+49.7 days overflows it — right now the old version returned -379298136. Differences
// between two wrapped stamps still come out right, which is why this went unnoticed, but a
// comparison against a literal 0 sentinel does not: `now - 0 >= 150` is FALSE for the whole
// ~25-day window in which the wrapped value is negative, and TRUE for the next. That silently
// disabled the dirty-driven repaint path — every plugin frame and every JS animation — on a
// calendar cycle, which is about the least debuggable failure shape available. Measured:
// Flash drawing at 30fps while the card repainted 5 times in 25 seconds.
static int64_t jihadNowMs() {
  struct timeval tv; gettimeofday(&tv, NULL);
  return (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// The address as it may go to the card: at most 8 KiB. A page can make its own URL as long as it
// likes (history.replaceState with a 20 000-character query) and the YAP packet is a fixed
// 16 KiB, in which an oversized field is skipped while the ones after it are still written —
// the malformed message jihadBoundedTitle exists to prevent (inspector 2026-10-06, P3). EVERY
// address the daemon sends goes through this (link clicked, history, failed load, redirect, new
// page): a dropped field reached the adapter as NULL and its strlen crashed the card's host
// (inspector 2026-10-07, P1). The full address stays the engine's; the card gets this one.
static std::string jihadBoundedUri(const std::string& in) {
  return in.size() > 8192 ? in.substr(0, 8192) : in;
}


// Blank-over-good suppression (paintToSharedBuffer): how long a blank frame must persist AFTER
// the load completed before it is accepted as the page itself, and how often a still-suppressed
// blank is re-rendered. 500 ms is long enough to cover a reflow that momentarily renders white
// after load, short enough that a blank page appears to load at once; 100 ms keeps the first
// content frame of a slow load within one retry of when it became available.
static const int64_t kJihadBlankSettleMs = 500;
static const int64_t kJihadBlankRetryMs  = 100;

// How far (CSS px, scaled by the page zoom) the engine's damage box is grown before a
// damage-only repaint — see where TakeDirtyRect is consumed. 32 covers the media controls' time
// bubble (about 22 px above the bar and half a bubble either side of the thumb) with room.
static const double kJihadDamageMarginCssPx = 32.0;

// The daemon's tick period. Defined HERE rather than in Main.cpp, which used to own it, because
// the plugin pull has to express its request period in whole ticks — a request grid that is not
// a tick multiple beats against the tick and reintroduces the uneven spacing it exists to
// remove. Two copies of the env read would let the two drift apart silently, so there is one.
int jihadTickPeriodMs() {
  static int s_ms = -1;
  if (s_ms < 0) {
    s_ms = 8;
    if (const char* e = getenv("JIHAD_TICK_MS")) {
      int v = atoi(e);
      if (v >= 4 && v <= 64) s_ms = v;
    }
  }
  return s_ms;
}

// The tick period WHILE A PLUGIN IS ON THE PAGE: 16 ms, unless $JIHAD_TICK_MS says otherwise.
//
// The 8 ms default above is right for pages and wrong for a plugin, and that has now been
// measured on both OS versions. With an 8 ms timer and a plugin animating, the tick cannot hold
// its period (work is 11-18 ms), the 33 ms request grid becomes "4 ticks of whatever the tick
// actually took", and the plugin is drawn ~39 times a second for 30 fps content: webOS CE 3.1.0,
// 2026-10-01, jihad-anim.swf, 880 frames — gaps <16 ms 12%, 16-23 34%, 24-31 34%, 32-39 16%. At
// 16 ms the same clip gives 32.8 fps with 45% of gaps in the 32-39 ms bucket and nothing
// deferred. Every Flash measurement before 2026-10 was taken at 16 ms — but only because the
// device's job file carried JIHAD_TICK_MS=16 as a local edit, which a reinstall silently drops;
// the shipped build ran Flash on the 8 ms tick. This makes the measured configuration the
// shipped one, for plugin pages only. An explicit $JIHAD_TICK_MS still wins, for A/B runs.
int jihadPluginTickPeriodMs() {
  static int s_ms = -1;
  if (s_ms < 0) s_ms = getenv("JIHAD_TICK_MS") ? jihadTickPeriodMs() : 16;
  return s_ms;
}

static int jihadShmResolve(int keyOrId);   // defined below

int BrowserPageGoanna::slotForKey(int key) const {
  if (key && key == mKey1) return 0;
  if (key && key == mKey2) return 1;
  if (key && key == mKey3) return 2;
  return -1;
}

// YAP 0x1600. The adapter allocated a third segment and is telling us its key. Accepting it is
// what turns the paint rotation from "one adapter round trip per frame" into "the adapter holds
// one buffer while we still have a free one" — with two buffers roughly a quarter of paint
// attempts were refused outright (device: deferred 11 -> 38 -> 77 as the plugin sped up, while
// completed paints stayed pinned at ~26/s).
void BrowserPageGoanna::setExtraBuffer(int key3, int size) {
  if (!key3) return;
  // Must match the pair we already have, or the rotation would hand the adapter a buffer of a
  // different size than the header it reads describes.
  if (size > 0 && mBufSize > 0 && size != mBufSize) {
    fprintf(stderr, "[jihad-bs] extra buffer REFUSED: size %d != %d\n", size, mBufSize);
    return;
  }
  if (jihadShmResolve(key3) < 0) {
    fprintf(stderr, "[jihad-bs] extra buffer REFUSED: key 0x%x does not resolve\n", (unsigned)key3);
    return;
  }
  mKey3 = key3;
  mInFlight[2] = false; mPaintMs[2] = 0; mBufFull[2].valid = false;
  fprintf(stderr, "[jihad-bs] extra buffer ACCEPTED: 3-buffer rotation active (key 0x%x)\n",
          (unsigned)key3);
}

// Advance the paint target. With two buffers this is the original alternation; with three it
// cycles 1 -> 2 -> 3 -> 1, which is the whole point of the third: the adapter can be holding one
// displayed buffer and STILL leave us a free one, instead of gating every frame on its return.
int BrowserPageGoanna::nextBufferKey() const {
  if (mKey3) {
    if (mActiveKey == mKey1) return mKey2 ? mKey2 : mKey3;
    if (mActiveKey == mKey2) return mKey3;
    return mKey1;
  }
  return (mActiveKey == mKey1 && mKey2) ? mKey2 : mKey1;
}


// Resolve a key/shmid and return a cached attachment (attaching once, then reusing). shmat/shmdt
// per paint cost ~400ms/keystroke on device; the two alternating segments are attached at most once.
unsigned char* BrowserPageGoanna::attachShm(int keyOrId) {
  int id = jihadShmResolve(keyOrId);
  if (id < 0) return nullptr;
  for (int i = 0; i < 3; ++i) if (mShmId[i] == id && mShmBuf[i]) return mShmBuf[i];
  void* p = shmat(id, nullptr, 0);
  if (p == (void*)-1) { perror("[BrowserPageGoanna] shmat"); return nullptr; }
  // Fill the first empty slot; with a third buffer in rotation there are three of them.
  int slot = -1;
  for (int i = 0; i < 3; ++i) if (!mShmBuf[i]) { slot = i; break; }
  if (slot < 0) slot = 0;
  if (mShmBuf[slot]) { shmdt(mShmBuf[slot]); }
  mShmBuf[slot] = (unsigned char*)p; mShmId[slot] = id;
  return mShmBuf[slot];
}

void BrowserPageGoanna::detachShm() {
  for (int i = 0; i < 3; ++i) { if (mShmBuf[i]) shmdt(mShmBuf[i]); mShmBuf[i] = nullptr; mShmId[i] = -1; }
}

void BrowserPageGoanna::mapToContent(int sx, int sy, int* cx, int* cy) {
  // mZoom is clamped to a sane range on set (see setZoomAndScroll), so sx/z can't
  // blow up; still clamp the result to a safe int range (Codex P0).
  double z = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
  int scrollX = 0, scrollY = 0;
  if (mPage) mPage->GetScrollXY(&scrollX, &scrollY);   // content-space scroll
  auto clamp = [](double v) -> int {
    if (v < -1000000.0) return -1000000;
    if (v >  1000000.0) return  1000000;
    return (int)v;
  };
  if (cx) *cx = clamp((double)sx / z + scrollX);
  if (cy) *cy = clamp((double)sy / z + scrollY);
}

// Ask for a thumbnail of what this page is showing, written to `path` at its next real paint
// (one is forced). Asynchronous on purpose: the pixels only exist inside a paint.
void BrowserPageGoanna::requestThumbnail(const std::string& path) {
  mThumbPath = path;
  mDamageForceFull = true;
  mNeedsPaint = true;
}

// Box-filter `src` (w x h BGRA rows) down to at most 192 px wide and 144 px high — the top of
// the view, at the view's own aspect — and write it as a PNG.
bool BrowserPageGoanna::writeThumbnail(const unsigned char* src, int w, int h, const std::string& path) {
  const int TW = 192, THMAX = 144;
  if (!src || w < TW || h <= 0) return false;
  const int step = w / TW;                       // whole source pixels per thumbnail pixel
  int th = h / step; if (th > THMAX) th = THMAX;
  if (step <= 0 || th <= 0) return false;
  std::vector<unsigned char> out((size_t)TW * th * 4);
  for (int ty = 0; ty < th; ++ty) {
    for (int tx = 0; tx < TW; ++tx) {
      unsigned long b = 0, g = 0, r = 0;
      for (int yy = 0; yy < step; ++yy) {
        const unsigned char* p = src + ((size_t)(ty * step + yy) * w + (size_t)tx * step) * 4;
        for (int xx = 0; xx < step; ++xx, p += 4) { b += p[0]; g += p[1]; r += p[2]; }
      }
      const unsigned long n = (unsigned long)step * step;
      unsigned char* o = &out[((size_t)ty * TW + tx) * 4];
      o[0] = (unsigned char)(b / n); o[1] = (unsigned char)(g / n); o[2] = (unsigned char)(r / n);
      o[3] = 255;                                // opaque, whatever the buffer's alpha byte held
    }
  }
  return jihad::WritePngBgra(path.c_str(), out.data(), TW, th, TW * 4);
}

BrowserPageGoanna::~BrowserPageGoanna() {
  // Never leave the process-wide sink pointing at a destroyed page — and never take it away
  // from a page that is still alive. If this page owned the dialogs they pass to the newest
  // remaining one; if it did not (an older card closing while a newer one is open), nothing
  // changes. Before 2026-10-02 any closing page nulled the sink, so with two Enyo cards open
  // closing the first left the second's dialogs taking their silent defaults.
  { std::vector<BrowserPageGoanna*>& live = jihadLivePages();
    live.erase(std::remove(live.begin(), live.end(), this), live.end());
    if (GetDialogSink() == static_cast<DialogSink*>(this))
      SetDialogSink(live.empty() ? nullptr : static_cast<DialogSink*>(live.back())); }
  // "Trust once" ends with the page that asked, exactly as upstream's dtor did.
  // The startup sweep is what covers the case this one cannot: dying without
  // running at all.
  jihad::certstore::SweepSession(this);
  clearPluginRects();
  // The BrowserAdapter owns the shared segments (it allocated them and passed
  // the keys via connect()); the daemon must NOT IPC_RMID them (Codex P1).
  detachShm();
  delete mPage;
  for (UrlRule* r : mRedirectRules) { regfree(&r->re); delete r; }
}

void BrowserPageGoanna::addUrlRedirect(const char* urlRe, int /*type*/,
                                       bool redirect, const char* userData) {
  if (!urlRe || !*urlRe) return;
  UrlRule* r = new UrlRule();
  if (regcomp(&r->re, urlRe, REG_EXTENDED | REG_NOSUB) != 0) {   // invalid regex
    delete r; return;
  }
  r->userData = userData ? userData : "";
  r->redirect = redirect;
  mRedirectRules.push_back(r);
}

bool BrowserPageGoanna::applyRedirectRules(const char* url) {
  for (UrlRule* r : mRedirectRules) {
    if (regexec(&r->re, url, 0, nullptr, 0) == 0) {   // matched
      mSink.msgUrlRedirected(jihadBoundedUri(url ? url : "").c_str(), r->userData.c_str());   // R6: notify the client
      if (r->redirect) return true;   // handled externally; do not load it here
      break;
    }
  }
  return false;
}

// The passed shared-buffer handle can be one of two forms: the production isis adapter
// (IpcBuffer::create) IPC_RMID-marks each segment and sends its SysV SHMID (attach by id
// directly &mdash; shmget can't find an IPC_RMID'd segment); our standalone test adapter sends
// an ftok KEY (needs shmget). Resolve either to an attachable shmid: try it as a shmid
// (shmctl succeeds only for a real id), else look it up as a key.
static int jihadShmResolve(int keyOrId) {
  struct shmid_ds ds;
  if (shmctl(keyOrId, IPC_STAT, &ds) == 0) return keyOrId;   // already a valid shmid
  return shmget(keyOrId, 0, 0);                              // else treat as a key (-1 if neither)
}

bool BrowserPageGoanna::init(uint32_t width, uint32_t height,
                             int sharedBufferKey1, int sharedBufferKey2, int sharedBufferSize) {
  // Validate the adapter-provided geometry/buffers (Codex P2): no overflow, and
  // the segment must be large enough for width*height*4.
  if (width == 0 || height == 0 || width > 8192 || height > 8192) return false;
  const size_t need = (size_t)width * height * 4;
  if (sharedBufferSize <= 0 || (size_t)sharedBufferSize < need) return false;
  if (!sharedBufferKey1) return false;

  mKey1 = sharedBufferKey1;
  mKey2 = sharedBufferKey2;
  // A fresh Connect replaces the whole set, so a third announced against the PREVIOUS set is
  // stale — the adapter re-announces once it has allocated the new one.
  mKey3 = 0;
  mBufSize = sharedBufferSize;
  mActiveKey = mKey1;
  mInFlight[0] = mInFlight[1] = mInFlight[2] = false;   // fresh segments: nothing in flight (F-211)

  // Attach-only: the adapter (isis IpcBuffer::create) makes each segment, IPC_RMID-marks
  // it for auto-delete on last detach, and sends the SysV SHMID (not an ftok key). An
  // IPC_RMID'd segment is gone from the shmget key namespace but still reachable by shmid
  // via shmctl/shmat &mdash; so we validate with shmctl(IPC_STAT) by shmid, like IpcBuffer::attach.
  for (int k : { mKey1, mKey2 }) {
    if (!k) continue;
    if (jihadShmResolve(k) < 0) {
      fprintf(stderr, "[jihad-bs] init: shm resolve of 0x%x failed: %s\n", (unsigned)k, strerror(errno));
      return false;
    }
  }

  mPage = new GoannaRenderPage(mHost);
  if (!mPage->Create((int)width, (int)height)) {
    fprintf(stderr, "[jihad-bs] init: GoannaRenderPage::Create(%u,%u) FAILED\n", width, height);
    delete mPage; mPage = nullptr;
    return false;
  }
  return true;
}

void BrowserPageGoanna::setWindowSize(uint32_t width, uint32_t height) {
  // Same reasoning as the navigation reset: a resize replaces the whole frame, and both
  // buffers still hold the old one.
  mBufFull[0].valid = mBufFull[1].valid = mBufFull[2].valid = false;
  mLatestSlot = -1; mGrowOwed = false; mSweepLeft = 0; mSweepRow = 0;
  mDamageValid = false; mDamageForceFull = true;
  if (!mPage) return;
  // The BrowserAdapter owns the shared framebuffer and its size (Codex P1/P2);
  // the daemon must not paint beyond it. Only accept a surface that still fits
  // the segments handed to us at connect(). Growing past that needs the adapter
  // to re-allocate + re-key first (a returnBuffer/reconnect round-trip).
  if (width == 0 || height == 0) return;
  if (width > 8192 || height > 8192) return;                 // bound BEFORE the multiply (32-bit ARM, review #7 P3)
  if ((uint64_t)width * height * 4 > (uint64_t)mBufSize) return;
  // Defer the geometry emit to pump() (after PumpFor lets the reflow settle) rather than
  // emitting synchronously here, mid-resize, when GetContentSize can still read 0x0
  // (review #7 P2). mGeometryDirty is drained on the next tick.
  // T1 instrumentation (device 2026-07-17: "page pushed up off screen when the search bar is
  // focused"): log the engine scroll across the resize — the VKB shrink arrives as a
  // setWindowSize, so these lines show whether the reflow moved the scroll (engine-side push)
  // or the scroll was already moved before the resize (adapter/app-side push). GATED behind
  // JIHAD_T1_LOG (inspector P3): each GetScrollXY forces a layout flush, and two extra full
  // reflows per VKB toggle in the socket-callback context is a real cost on the ARMv7.
  static const bool t1log = getenv("JIHAD_T1_LOG") != nullptr;
  // T1 fix ("page pushed up off screen when the search box is focused"): a VKB-raise
  // arrives as a height SHRINK. The engine's Resize reflow re-scrolls the focused
  // editable into view, shoving the page up — even though the user just tapped that
  // field so it was already visible. Capture the scroll before the shrink and, if a
  // field is focused and the reflow pushed the scroll DOWN (content up), restore the
  // pre-resize offset so the page stays put under the keyboard. Only on a shrink
  // (VKB up); a grow (VKB down) keeps the engine's natural scroll. The field the user
  // tapped stays visible because ClickAt already ensured it was on-screen pre-resize.
  const bool shrink = (mLastWinH != 0 && (int)height < mLastWinH);
  const bool hadEditable = mPage->HasFocusedEditable();
  int sx0 = 0, sy0 = 0;
  if (t1log || (shrink && hadEditable)) mPage->GetScrollXY(&sx0, &sy0);
  if (mPage->Resize((int)width, (int)height)) { mNeedsPaint = true; mGeometryDirty = true; }
  // The window grew back by about a keyboard's height, at the same width, with a field focused:
  // the keyboard was put away, and the field gives up the focus with it (no caret left behind).
  // A rotation to portrait also grows the height, but changes the width.
  if (!shrink && hadEditable && mLastWinH != 0 && (int)width == mLastWinW && (int)height >= mLastWinH + 120)
    mPage->BlurFocusedEditable();
  if (shrink && hadEditable) {
    int sx1 = 0, sy1 = 0; mPage->GetScrollXY(&sx1, &sy1);
    // Counter an engine-induced downward push (field-into-view).
    // CORRECTION 2026-08-10 (T-133): the field-position guard the previous version of this
    // comment described WAS NEVER IMPLEMENTED. It claimed the restore runs only when the tapped
    // field's top stays inside the shrunken viewport; the test is just sy1 > sy0, so a field the
    // VKB genuinely covers gets pushed back under it. Left as-is on purpose: e1b3197 was tuned on
    // the device, T-119 has not re-run it, and adding the guard blind could regress the verified
    // top-of-page-search-box case.
    // The adapter cannot compensate either. scrollCaretIntoViewAfterResize is DEAD CODE in this
    // port: its first clause tests m_textCaretRect, which is only written by
    // msgGetTextCaretBoundsResponse, and JihadBrowserServer::asyncCmdGetTextCaretBounds is a
    // (void)proxy; no-op (TODO T-016), so that response never arrives and the rect stays (0,0).
    // 2026-10-01 (T-258): the guard is IMPLEMENTED now, with the device run that was missing.
    // Measured on a page four screens tall, scrolled to the end: the bottom field sat at
    // viewport y=829, the keyboard shrank the window to 651, the engine did not move the scroll at
    // all (`scroll 0,1758 -> 0,1758`) and the field stayed at y=829 — under the keyboard, typed
    // into blind. So the engine's reflow cannot be relied on to lift a covered field either.
    // Decide from where the field would be if the page stayed put:
    //   still fully inside the shrunken window -> keep the page exactly where it was (the T1 fix:
    //     the user tapped a visible field, nothing should move);
    //   covered by the keyboard (or pushed off the top) -> scroll just enough to bring its
    //     bottom edge a margin above the keyboard, and TELL THE ADAPTER, whose pan position is
    //     what the user sees. Sent directly rather than left to emitScrollIfChanged: that path
    //     suppresses anything near a recent adapter scroll, and a tap right after a pan is
    //     exactly this case.
    // Only at zoom 1 — the card zooms to 1.0 when a field takes focus, and elsewhere the engine
    // scroll is parked and the render pan carries the position.
    int ft = 0, fb = 0;
    const bool unzoomed = (mZoom >= 0.99 && mZoom <= 1.01);
    if (unzoomed && mPage->FocusedEditableBox(&ft, &fb)) {
      const int kMargin = 16;
      const int vh = (int)height;
      const int topAtOld = ft + (sy1 - sy0), botAtOld = fb + (sy1 - sy0);
      if (topAtOld >= 0 && botAtOld <= vh - kMargin) {
        if (sy1 != sy0) mPage->ScrollTo(sx0, sy0);
      } else {
        int target = sy0 + (botAtOld - (vh - kMargin));          // bottom edge above the keyboard
        if (topAtOld - (target - sy0) < kMargin) target = sy0 + topAtOld - kMargin;   // tall field: its top wins
        if (target < 0) target = 0;
        mPage->ScrollTo(sx0, target);
        int sx2 = 0, sy2 = 0;
        if (mPage->GetScrollXY(&sx2, &sy2)) {
          fprintf(stderr, "[jihad-bs] keyboard would cover the focused field (y=%d..%d in a %d px window): "
                          "scrolled %d -> %d\n", topAtOld, botAtOld, vh, sy0, sy2);
          mLastScrollX = sx2; mLastScrollY = sy2;
          mSink.msgScrolledTo(sx2, sy2);
        }
      }
    } else if (sy1 > sy0) {
      mPage->ScrollTo(sx0, sy0);
    }
  }
  if (mLastWinH != (int)height) { mLastWinResizeDh = (int)height - mLastWinH; mLastWinResizeMs = jihadNowMs(); }
  mLastWinH = (int)height;
  mLastWinW = (int)width;
  if (t1log) {
    int sx2 = 0, sy2 = 0; mPage->GetScrollXY(&sx2, &sy2);
    fprintf(stderr, "[jihad-bs] setWindowSize %ux%u scroll %d,%d -> %d,%d editable=%d shrink=%d\n",
            width, height, sx0, sy0, sx2, sy2, (int)hadEditable, (int)shrink);
  }
}

void BrowserPageGoanna::freeze() {
  // Card backgrounded: stop painting (the adapter may free/reuse the buffers).
  // Drop the cached shm attachments too: the production segments are IPC_RMID-marked,
  // so holding an attach keeps them alive and can exhaust the device's scarce shared
  // memory while backgrounded. thaw() re-validates and maybePaint() re-attaches on demand.
  mFrozen = true;
  detachShm();
}

void BrowserPageGoanna::thaw(int key1, int key2, int size) {
  // Reattach the (possibly new) shared buffers the adapter provides. Only resume
  // painting if BOTH required segments validate and fit the surface; otherwise
  // STAY FROZEN so maybePaint can't write a stale/reused segment (Codex P0). The
  // adapter must re-thaw with valid buffers.
  if (!mPage) return;
  const size_t need = (size_t)mPage->Width() * mPage->Height() * 4;
  bool ok = key1 && size > 0 && (size_t)size >= need;
  // keys may be SHMIDs (isis) or ftok keys (test adapter) &mdash; validate via the resolver.
  if (ok) for (int k : { key1, key2 }) { if (k && jihadShmResolve(k) < 0) ok = false; }
  if (!ok) return;   // remain frozen; keep the old (already-detached) keys inactive
  detachShm();       // the buffers may be new segments — drop stale cached attachments
  mKey1 = key1; mKey2 = key2; mBufSize = size; mActiveKey = mKey1;
  mKey3 = 0;   // re-announced by the adapter after thaw; the old third segment may be gone
  mInFlight[0] = mInFlight[1] = mInFlight[2] = false;   // reattached (possibly new) segments: clear in flight (F-211)
  // Possibly NEW segments — detachShm() above just dropped the old attachments — so nothing
  // we recorded about the old buffers' contents survives. Without this the first paint after
  // a card comes back to the foreground passes the damage-only eligibility test (w, h, zoom
  // and scroll are all unchanged by backgrounding) and renders ONLY the damage box into a
  // segment we have never written, publishing whatever its previous owner left there. With a
  // plugin animating, mDamageValid is true on essentially every tick, so that is the
  // guaranteed first frame after thaw, not a rare one.
  mBufFull[0].valid = mBufFull[1].valid = mBufFull[2].valid = false;
  mLatestSlot = -1; mGrowOwed = false; mSweepLeft = 0; mSweepRow = 0;
  mDamageValid = false; mDamageForceFull = true;
  mFrozen = false;
  mNeedsPaint = true;
}

bool BrowserPageGoanna::findString(const char* text, bool forward) {
  return mPage && text && mPage->Find(text, forward);
}

void BrowserPageGoanna::returnBuffer(int sharedBufferKey) {
  // The adapter is done blitting this buffer (F-211): clear its in-flight mark so the next
  // paint may reuse it. This is what makes per-keystroke immediate painting safe under fast
  // typing — without it the daemon overwrote a buffer the adapter was still reading and the
  // adapter (in LunaSysMgr) crashed. If a queued paint was deferred waiting on this return,
  // paint it now so the latest frame lands promptly instead of on the next tick.
  int s = slotForKey(sharedBufferKey);
  if (s >= 0) mInFlight[s] = false;
  if (mNeedsPaint && !mFrozen) maybePaint();
}

// Every navigation lands at the top, on BOTH sides of the YAP boundary. Stock isis reset via the
// msgContentsSizeChanged(0,0) "new page" sentinel (ref-BrowserServer BrowserPage.cpp:1614 —
// saveFrameStateRequested on commit; the adapter's (0,0) branch zeroes mScrollPos + zoom-fit), but
// this port deliberately never emits that sentinel (emitGeometry: a transient (0,0) during resize
// made the adapter drop the live buffer -> white card + zoom flicker). So nothing reset the
// adapter's scroll on navigation, and the daemon's own band state kept the OLD page's scroll: a
// card that navigated while scrolled down painted the new (short) page's band a full viewport
// below the content, and a static plugin at the top of the page never entered the shared buffer
// (device repro 2026-08-09: tall page -> scroll 942 -> flash page = no magenta, doc=0,942 bands).
// msgScrolledTo(0,0) is the sentinel-free reset: the adapter's handler clamps to exactly (0,0) at
// any zoom and moves its scroller (BrowserAdapter.cpp:4218-4224), and the KineticScroller listener
// echoes asyncCmdSetScrollPosition(0,0) back — both sides converge on 0,0. Stock never restored
// scroll on back/forward either (restoreFrameStateRequested is an empty stub), so calling this
// from all five nav commands reproduces stock exactly.
void BrowserPageGoanna::resetScrollForNavigation() {
  // A new document invalidates both buffers wholesale. The damage-only path proves its
  // premise from geometry (size, zoom, scroll), and a navigation can leave every one of
  // those identical while replacing all the pixels — so geometry alone cannot catch it and
  // the record has to be dropped explicitly.
  mBufFull[0].valid = mBufFull[1].valid = mBufFull[2].valid = false;
  mLatestSlot = -1; mGrowOwed = false; mSweepLeft = 0; mSweepRow = 0;
  mDamageValid = false; mDamageForceFull = true;
  // The old page's plugins are gone; their rects must not outlive them. The adapter never
  // clears mFlashRects itself, so leaving them behind makes the NEW page carry invisible
  // gesture dead zones wherever the OLD page happened to have a plugin.
  clearPluginRects();
  mAdapterScrollX = 0; mAdapterScrollY = 0;       // stale band origin + adY band extension
  if (mPage) { mPage->SetRenderPan(0.0, 0.0); mPage->ScrollTo(0, 0); }
  mViewportSent = false;                          // the new page's meta-viewport is sent afresh
  // The first-paint hold belongs to one document: a new one starts its own second, rather than
  // inheriting the remainder of the old one's (Codex review 2026-10-08).
  mWasSuppressed = false; mSuppressedSinceMs = 0;
  mLastContentW = -1; mLastContentH = -1;         // old page's clamp/bound inputs; also forces a
                                                  // re-emit at load-done even if the size repeats
  mLastScrollX = 0; mLastScrollY = 0;             // emitScrollIfChanged dedup cache
  mLastScrollMs = 0;                              // disarm the adapter-driven suppression window
  mPaintedX = 0; mPaintedW = 0; mPaintedLo = 0; mPaintedHi = 0;  // no stale coverage claim (setScrollPosition)
  mPrevPaintScrollY = 0;                          // overscan direction bias
  mSink.msgScrolledTo(0, 0);                      // the adapter-side half of the reset
}

// Is the adapter's viewport at (x,y) — zoomed px, its own scroll — inside the region the last
// paint covered, with room to spare? Repaint only when it nears the region's edge or leaves
// it: panning inside the buffer is exactly what the headroom is for.
bool BrowserPageGoanna::panCovered(int x, int y) {
  if (!mPage) return false;
  double z = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
  long h2 = (long)mPage->Height(), w2 = (long)mPage->Width();
  long slack = h2 / 2;   // trigger EARLY: the repaint takes real time on this CPU and the
                         // fling keeps consuming rows while it renders ("content doesn't
                         // stay consistently visible" — h/4 triggered too late)
  // ...but never more than the region can give. The paint puts as little as a sixth of its
  // spare rows behind the pan, and a region that is two viewports WIDE has half the rows:
  // with a fixed half-viewport margin the position just painted counted as uncovered, and
  // every pan message asked for another paint. An eighth of the spare rows always fits.
  {
    const long spare = (mPaintedHi - mPaintedLo) - h2;
    if (spare > 0 && spare / 8 < slack) slack = spare / 8;
    if (spare <= 0) slack = 0;
  }
  double Zc = (z >= 0.99 && z <= 1.01) ? 1.0 : z;   // the zoom the paint path would use
  long contentRows = (mLastContentH > 0) ? (long)((double)mLastContentH * Zc) : 0;
  long loBound = (mPaintedLo <= 0) ? 0 : mPaintedLo + slack;             // page top: no slack
  long hiBound = (contentRows > 0 && mPaintedHi >= contentRows)
                     ? mPaintedHi : mPaintedHi - slack;                   // page end: no slack
  // Columns. A region exactly one viewport wide (every unzoomed frame) is covered only at the
  // x it was painted for, as before. A wider one (zoomed in) is covered anywhere inside it,
  // less a quarter viewport of warning at any edge that is not the page's own.
  bool xCovered;
  if (mPaintedW <= w2) {
    xCovered = (x == mPaintedX);
  } else {
    long contentCols = (mLastContentW > 0) ? (long)((double)mLastContentW * Zc) : 0;
    long xs = w2 / 4;
    long loX = (mPaintedX <= 0) ? 0 : (long)mPaintedX + xs;
    long hiX = (long)mPaintedX + mPaintedW;
    if (!(contentCols > 0 && hiX >= contentCols)) hiX -= xs;
    xCovered = (long)x >= loX && (long)x + w2 <= hiX;
  }
  return mPaintedHi > mPaintedLo && xCovered &&
         (mPaintedZoom > Zc - 0.001 && mPaintedZoom < Zc + 0.001) &&
         (long)y >= loBound && (long)y + h2 <= hiBound;
}

void BrowserPageGoanna::setScrollPosition(int x, int y) {
  if (!mPage) return;
  mLastScrollMs = jihadNowMs();   // a pan is in flight (see maybePaint)
  // The adapter's scroll (x,y) is in ZOOMED-content px (its mScrollPos / buffer space); the
  // engine scrolls in CSS px, so divide by the zoom — EXACTLY as setZoomAndScroll does. And keep
  // BrowserOffscreenInfo::renderedX/Y (= mAdapterScrollX/Y) in step so the adapter's composite pan
  // lines up (renderedX == mScrollPos => identity blit). Without this, a drag-pan AFTER a zoom fed
  // zoomed px straight to the CSS-px engine and left renderedX/Y stale, so panned/zoomed content
  // landed off-screen ("cut off"). At zoom==1 this is unchanged (x/1==x) so plain scroll, rotation
  // and first render are unaffected (Codex review 2026-07-26, Finding 2).
  double z = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
  mAdapterScrollX = x; mAdapterScrollY = y;
  // Same zoom-aware routing as setZoomAndScroll: at z>1 pan the render's visual viewport (so
  // a drag while zoomed reaches the whole page in both axes) and park the engine scroll; at
  // z<=1 the javascript: engine scroll applies asynchronously (the scrolled-to message is
  // emitted from pump() once the offset actually moves, emitScrollIfChanged).
  if (z < 0.99 || z > 1.01) {
    // Zoomed (|z-1|>1%, so a near-1 fit like 1.0052 stays on the engine-scroll path below and
    // keeps window.onscroll/fixed/sticky correct for normal browsing): PURE visual-viewport pan
    // — this covers zoom-IN (z>1) and zoom-OUT (z<1, e.g. a wide fixed-width page fit into the
    // window). JihadRenderDocument renders an absolute DOCUMENT rect
    // (RENDER_DOCUMENT_RELATIVE), so the whole page is reachable in both axes with no engine
    // scroll and no display-list culling. Clamp the pan to the content so it can't drift into the
    // white margin past the page edge (Codex F6); mLastContentW/H staleness only affects the
    // clamp, not correctness (document-relative never goes blank). The engine scroll is left
    // untouched (the render ignores it) so no spurious scrolled-to echo corrupts the adapter's
    // pan (Codex F2).
    double vpX = x / z, vpY = y / z;                            // visual-viewport top-left, CSS px
    double vw = (double)mPage->Width() / z, vh = (double)mPage->Height() / z;  // visible CSS px @ zoom
    if (vpX < 0.0) vpX = 0.0;
    if (vpY < 0.0) vpY = 0.0;
    if (mLastContentW > 0 && vpX > mLastContentW - vw) vpX = (mLastContentW > vw) ? (mLastContentW - vw) : 0.0;
    if (mLastContentH > 0 && vpY > mLastContentH - vh) vpY = (mLastContentH > vh) ? (mLastContentH - vh) : 0.0;
    mPage->SetRenderPan(vpX, vpY);
  } else {
    mPage->SetRenderPan(0.0, 0.0);
    mPage->ScrollTo((int)(x / z), (int)(y / z));
  }
  // Coverage-aware repaint ("scrolling is still janky... optimize it", device 2026-08-02):
  // with overscan the last painted region usually already CONTAINS the pan target, and the
  // 2+-viewport repaint this used to force on every scroll message starved the
  // single-threaded daemon mid-drag (input, hit-tests and the engine scroll all queue
  // behind it). Repaint only when the viewport nears the painted region's edge (within
  // h/4) or leaves it — panning inside the buffer is exactly what the headroom is for.
  {
    long h2 = (long)mPage->Height();
    double Zc = (z >= 0.99 && z <= 1.01) ? 1.0 : z;   // the zoom the paint path would use
    // Covered by rows that may predate a change to the page (a sweep is owed) is not
    // covered: the viewport is repainted where it now is, so what is on screen is the page
    // as it stands and not as it was. The sweep itself waits for the pan to end.
    bool covered = panCovered(x, y) && mSweepLeft <= 0;
    if (!covered) mNeedsPaint = true;   // covered: keep whatever was already pending
    // T-133 INSTRUMENT: the white band on a VKB toggle, counted in ROWS instead of inferred.
    // This is the only point in the daemon holding the adapter's pan and the painted band in one
    // coordinate space, and the adapter's composite is a pure function of the two. handlePaint
    // computes sT = srcTop - renderedY + scrollY, then CLIPS the dst at whichever edge falls
    // outside the buffer (PGSurface path) or writes FILL = 0xffffffff (dstBuffer path). Rows above
    // mPaintedLo or below mPaintedHi are therefore never drawn, and an undrawn dst reads WHITE on
    // device, which is what handlePaint's own "VKB-whiteout" note records. gapTop/gapBot are those
    // row counts exactly.
    // Prints only when rows are actually missing: `covered` carries h/2 of slack, so !covered by
    // itself is the ordinary early-repaint trigger and proves nothing. sinceResize/dh separate the
    // two producers. A VKB toggle moves the ADAPTER's pan before the daemon can paint at the new
    // height (KineticScroller::setViewportDimensions re-runs constrain() against the new
    // bottomBoundary, so a GROW springs a bottom-of-page pan up by as much as dh), while a fling
    // that outran the overscan has no recent resize. Zoom-matched only, so a pinch cannot print a
    // row count computed at the wrong scale.
    if (mPaintedHi > mPaintedLo &&
        mPaintedZoom > Zc - 0.001 && mPaintedZoom < Zc + 0.001) {
      long gapTop = mPaintedLo - (long)y;
      long gapBot = ((long)y + h2) - mPaintedHi;
      if (gapTop > 0 || gapBot > 0) {
        // Bounded so a fling past the buffer edge cannot flood the log. A full /var freezes the
        // daemon log, and a frozen log has already read here as a live one.
        static int64_t sLastGapLogMs = 0;
        const int64_t gnow = jihadNowMs();
        if (gnow - sLastGapLogMs >= 100) {
          sLastGapLogMs = gnow;
          fprintf(stderr, "[jihad-bs] uncovered pan y=%d h=%ld band=[%ld,%ld) gapTop=%ld gapBot=%ld"
                          " sinceResize=%ldms dh=%d\n",
                  y, h2, mPaintedLo, mPaintedHi, gapTop > 0 ? gapTop : 0, gapBot > 0 ? gapBot : 0,
                  mLastWinResizeMs ? (long)(gnow - mLastWinResizeMs) : -1L, mLastWinResizeDh);
        }
      }
    }
  }
}

void BrowserPageGoanna::setZoomAndScroll(double zoom, int x, int y) {
  if (!mPage) return;
  // Trace what the CARD actually asks for. A zoom injected here for testing is immediately
  // overwritten by the card's next setZoomAndScroll, so the only way to see a real pinch is
  // to log the incoming values; "zoom is unreliable on about:addons" (device 2026-08-03)
  // cannot be told apart from "the card never asked for that zoom" without this. Rate-limited
  // to changes so a pan (which re-sends the same zoom every frame) does not flood the log.
  if (zoom != mLastLoggedZoom) {
    fprintf(stderr, "[jihad-bs] setZoomAndScroll zoom=%.4f (was %.4f) scroll=%d,%d\n",
            zoom, mLastLoggedZoom, x, y);
    mLastLoggedZoom = zoom;
  }
  mLastScrollMs = jihadNowMs();   // a pan is in flight (see maybePaint)
  if (zoom >= 0.05 && zoom <= 20.0) mZoom = zoom;   // sane range for coord mapping (R5, Codex P0)
  double z = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
  mPage->SetZoom(z);   // clamped — SetZoom(raw) diverged from mZoom/contentZoom (review #6 F-008)
  // The adapter's scroll (x,y) is in ZOOMED-content px (its mScrollPos, buffer space);
  // the engine scrolls in CSS px, so divide by the zoom. Keep the zoomed values for
  // BrowserOffscreenInfo::renderedX/Y so the adapter's pan math lines up (== mScrollPos).
  mAdapterScrollX = x; mAdapterScrollY = y;
  // When zoomed IN (z>1) the engine layout viewport is device-width, so it can't scroll
  // horizontally and can't reach past its unzoomed vertical range — drive the RENDER's
  // visual-viewport pan (CSS px) and park the engine scroll at 0 so the whole magnified page
  // is reachable both axes. (Pinch-zoom pan is a visual-viewport move — no window.onscroll —
  // matching real mobile browsers.) At z<=1 keep the engine scroll unchanged (fires onscroll,
  // drives position:fixed/sticky). See JihadRenderDocument (zoom fix 2026-07-27).
  if (z < 0.99 || z > 1.01) {
    // Zoomed (|z-1|>1%, so a near-1 fit like 1.0052 stays on the engine-scroll path below and
    // keeps window.onscroll/fixed/sticky correct for normal browsing): PURE visual-viewport pan
    // — this covers zoom-IN (z>1) and zoom-OUT (z<1, e.g. a wide fixed-width page fit into the
    // window). JihadRenderDocument renders an absolute DOCUMENT rect
    // (RENDER_DOCUMENT_RELATIVE), so the whole page is reachable in both axes with no engine
    // scroll and no display-list culling. Clamp the pan to the content so it can't drift into the
    // white margin past the page edge (Codex F6); mLastContentW/H staleness only affects the
    // clamp, not correctness (document-relative never goes blank). The engine scroll is left
    // untouched (the render ignores it) so no spurious scrolled-to echo corrupts the adapter's
    // pan (Codex F2).
    double vpX = x / z, vpY = y / z;                            // visual-viewport top-left, CSS px
    double vw = (double)mPage->Width() / z, vh = (double)mPage->Height() / z;  // visible CSS px @ zoom
    if (vpX < 0.0) vpX = 0.0;
    if (vpY < 0.0) vpY = 0.0;
    if (mLastContentW > 0 && vpX > mLastContentW - vw) vpX = (mLastContentW > vw) ? (mLastContentW - vw) : 0.0;
    if (mLastContentH > 0 && vpY > mLastContentH - vh) vpY = (mLastContentH > vh) ? (mLastContentH - vh) : 0.0;
    mPage->SetRenderPan(vpX, vpY);
  } else {
    mPage->SetRenderPan(0.0, 0.0);
    mPage->ScrollTo((int)(x / z), (int)(y / z));
  }
  // The card re-sends the zoom with every pan while zoomed. When the zoom is the one the
  // last frame was painted at and the viewport is still inside that frame, there is nothing
  // to paint — which is what lets a pan across a zoomed page run on the frame it has.
  if (!panCovered(x, y) || mSweepLeft > 0) mNeedsPaint = true;   // (stale rows: see setScrollPosition)
  // NB: do NOT emitGeometry() here. A zoom-only command does not change the page's
  // intrinsic CSS content size, but re-reporting it fed the adapter's fit-zoom, which sent
  // a new zoom, which re-reported size... a cross-process oscillation (mZoom flapped
  // 1.0<->0.75<->0.7837, size 768x942<->1024x686 on rotate). Content size is emitted only
  // from setWindowSize + load-done now; the adapter derives its own contentWidth from the
  // zoom it applied. (review #6 F-001)
}

// Internal about: pages served from inline HTML by the daemon (no engine/omni.ja
// change, so no libxul rebuild). Rendered via SetHtml -> data:text/html, so the markup
// MUST NOT contain '#' (data-URL fragment delimiter), '%' (escape) or newlines.
// navigator.userAgent is filled by script so it always matches the live UA.
static bool jihadAboutPage(const char* url, std::string* outHtml, std::string* outAlias) {
  if (!url) return false;
  // case-insensitive match, tolerate a trailing slash
  std::string u(url);
  for (char& c : u) c = (char)tolower((unsigned char)c);
  while (!u.empty() && (u.back() == '/' || u.back() == ' ')) u.pop_back();
  const char* kHead =
    "<html><head><title>%TITLE%</title>"
    "<meta name=viewport content='width=device-width,initial-scale=1'>"
    "<style>body{font-family:sans-serif;margin:0;padding:24px;"
    "background:rgb(17,17,17);color:rgb(235,235,235);line-height:1.5}"
    ".hdr{display:flex;align-items:center;gap:14px;margin-bottom:20px}"
    ".hdr img{width:64px;height:64px;border-radius:14px}"
    "h1{margin:0;font-size:28px}.sub{color:rgb(140,175,190);margin-top:2px}"
    ".k{color:rgb(140,175,190)}code{background:rgb(40,44,52);padding:16px 18px;"
    "border-radius:8px;font-size:15px;line-height:1.9;overflow-wrap:break-word;"
    "word-break:normal;display:block;margin-top:8px;color:rgb(210,225,240);"
    // The device has no quality monospace font (only Prelude sans + CJK), so the default
    // monospace fallback renders pixelated. Use the body sans-serif for the UA box.
    "font-family:inherit}"
    ".row{margin:10px 0}a{color:rgb(102,204,255)}</style></head><body>";
  // User-agent block (shown before the open-source section).
  const char* kUA =
    "<div class=row><span class=k>User agent</span><br><code id=ua></code></div>"
    "<script>document.getElementById('ua').textContent=navigator.userAgent</script>";
  const char* kClose = "</body></html>";
  // The Jihad Browser logo (app icon) shown on both pages. data-URL safe (no hash/percent).
  const std::string logo =
    std::string("<img src=data:image/png;base64,") + JIHAD_LOGO_B64 + " alt=logo>";
  // Shared blocks so about:jihad and about:isis stay in sync. The open-source section
  // (with each component's license) appears LAST on BOTH pages, ending with the source link.
  const char* kFork =
    "<div class=row>Jihad Browser is a fork of isis-browser with the UXP/Goanna engine "
    "replacing QtWebKit for rendering.</div>";
  const char* kCredits =
    "<div class=row><a href=https://github.com/NotAlexNoyle/Jihad-Browser>Open source</a> "
    "Jihad Browser builds on the work of:</div>"
    "<div class=row>&bull; <a href=https://github.com/isis-project/isis-browser>isis-browser</a> "
    "&mdash; the webOS browser interface (Apache License 2.0)</div>"
    "<div class=row>&bull; <a href=https://repo.palemoon.org/MoonchildProductions/UXP>UXP / Goanna</a> "
    "&mdash; the web rendering engine by Moonchild Productions (Mozilla Public License 2.0)</div>"
    "<div class=row>&bull; <a href=https://github.com/Herrie82/atlas-browser-app>Atlas by Herrie82</a> "
    "&mdash; webOS browser integration (Apache License 2.0)</div>"
    "<div class=row>&bull; <a href=https://github.com/martok/palefill>Palefill by Martok</a> "
    "&mdash; the bundled site-compatibility add-on, unmodified (Mozilla Public License 2.0)</div>";
  if (u == "about:jihad") {
    std::string body =
      "<div class=hdr>" + logo +
      "<div><h1>Jihad Browser</h1><div class=sub>webOS 3 - UXP/Goanna engine</div></div></div>"
      + kFork + kUA + kCredits + kClose;
    std::string html(kHead); html.replace(html.find("%TITLE%"), 7, "Jihad Browser");
    *outHtml = html + body; *outAlias = "about:jihad"; return true;
  }
  if (u == "about:isis") {
    std::string body =
      "<div class=hdr>" + logo +
      "<div><h1>isis Browser</h1><div class=sub>the project Jihad Browser is forked from</div></div></div>"
      + kFork + kUA + kCredits + kClose;
    std::string html(kHead); html.replace(html.find("%TITLE%"), 7, "isis Browser");
    *outHtml = html + body; *outAlias = "about:isis"; return true;
  }
  return false;
}

void BrowserPageGoanna::openUrl(const char* url, const char* referrer, uint32_t referrerPolicy, bool fromPage) {
  if (!mPage || !url) return;
  // A newer explicit navigation supersedes ALL queued input aimed at the old document, and the
  // generation bump stops an in-flight drain from continuing into the superseded page.
  mPendingMouse.clear(); ++mNavGen;
  mPendingEditActions.clear();   // queued Tab/Enter for the old page are stale after a navigation
  // So are queued page commands — but a copy the adapter is waiting on still gets its answer.
  for (const PendingPageCmd& c : mPendingPageCmds)
    if (c.queryNum >= 0) mSink.msgCopySuccessResponse(c.queryNum, false);
  mPendingPageCmds.clear();
  // Any navigation lowers the VKB (covers link tap, JS location.href re-drive, form submit,
  // typed URL, back/forward) — otherwise the keyboard stays up over the new page (F-005).
  if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false, 0, 0);
  // Internal about:jihad / about:isis pages: render inline HTML and report the typed
  // about: URL as the location (not the underlying data: URL). Any other load clears
  // the alias so a real page never inherits it.
  std::string aboutHtml, alias;
  if (jihadAboutPage(url, &aboutHtml, &alias)) {
    mAliasUrl = alias;
    setHTML(url, aboutHtml.c_str());
    return;
  }
  mAliasUrl.clear();
  // A page that took the last two daemons down is not opened a third time by itself: the
  // card reopens its address whenever a daemon comes back, and that made a crash a loop
  // (JihadCrashGuard.h). Shown once; the link on it, a reload or the address bar loads it.
  if (jihad::CrashGuardShouldRefuse(url)) {
    std::string safe;
    for (const char* c = url; *c && safe.size() < 1200; ++c) {
      switch (*c) {
        case '&': safe += "&amp;"; break;   case '<': safe += "&lt;"; break;
        case '>': safe += "&gt;"; break;    case '"': safe += "&quot;"; break;
        default: safe += *c;
      }
    }
    const std::string html =
      "<!DOCTYPE html><html><head><meta charset=utf-8><title>Page stopped</title>"
      "<meta name=viewport content='width=device-width, initial-scale=1'>"
      "<style>body{font:18px/1.5 sans-serif;background:#1c1c1c;color:#e6e6e6;margin:0;padding:48px 36px}"
      "h1{font-size:24px;margin:0 0 14px}p{margin:0 0 16px;color:#bdbdbd}"
      "a{color:#7fb2e6}.u{word-wrap:break-word;color:#8cafbe;font-size:15px}</style></head><body>"
      "<h1>This page stopped the browser</h1>"
      "<p>It did so twice in a row, so it was not opened again automatically.</p>"
      "<p class=u>" + safe + "</p>"
      "<p><a id=retry href=\"" + safe + "\">Try again</a></p></body></html>";
    // setHTML appends its argument to "data:text/html," as it is, so a '#' would start a
    // fragment and cut the page off there (the first version of this page was blank for
    // exactly that reason: its colours), and a '%' in the address would be decoded. Escape
    // everything that is not plainly safe.
    std::string enc; enc.reserve(html.size() * 2);
    static const char* kHex = "0123456789ABCDEF";
    for (unsigned char c : html) {
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
          c == '-' || c == '_' || c == '.' || c == '~') {
        enc += (char)c;
      } else {
        enc += '%'; enc += kHex[c >> 4]; enc += kHex[c & 15];
      }
    }
    mAliasUrl = url;
    setHTML(url, enc.c_str());
    return;
  }
  jihad::CrashGuardNoteLoad(url);
  // R6: a matching redirect rule hands the URL to the client (msgUrlRedirected)
  // and, if it is a redirect, is not loaded in the browser at all.
  if (applyRedirectRules(url)) return;
  resetScrollForNavigation();   // after the redirect/about: outs — those never navigate here
  mLoadWasDone = false;
  mNeedsPaint = false;
  mLastProgress = 0;
  mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted();
  if (!mPage->LoadUrl(url, referrer, referrerPolicy, fromPage)) {
    // Synchronous rejection (bad/unknown-scheme URL): report it as a failed load
    // and don't leave the adapter permanently "loading" (Codex P2 + R3).
    mSink.msgFailedLoad("Goanna", 0, jihadBoundedUri(url ? url : "").c_str(), "Load failed");
    mSink.msgLoadProgress(100);
    mSink.msgLoadStopped();
    mLoadWasDone = true;
  }
}

void BrowserPageGoanna::setHTML(const char* /*url*/, const char* body) {
  if (!mPage || !body) return;
  resetScrollForNavigation();
  mLoadWasDone = false; mNeedsPaint = false; mLastProgress = 0;
  mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted();
  if (!mPage->SetHtml(body)) { mSink.msgLoadStopped(); mLoadWasDone = true; }
}

// Nav commands restart the load lifecycle so completion re-emits load+location.
// back/forward/reload also clear editor focus so the VKB lowers over the new page (review #7 P2).
void BrowserPageGoanna::pageBackward() { if (mPage) { mPendingMouse.clear(); ++mNavGen; if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false,0,0); resetScrollForNavigation(); mLoadWasDone=false; mNeedsPaint=false; mLastProgress=0; mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted(); mPage->GoBack(); } }
void BrowserPageGoanna::pageForward() { if (mPage) { mPendingMouse.clear(); ++mNavGen; if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false,0,0); resetScrollForNavigation(); mLoadWasDone=false; mNeedsPaint=false; mLastProgress=0; mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted(); mPage->GoForward(); } }
void BrowserPageGoanna::pageReload()  { if (mPage) { mPendingMouse.clear(); ++mNavGen; if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false,0,0); resetScrollForNavigation(); mLoadWasDone=false; mNeedsPaint=false; mLastProgress=0; mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted(); mPage->Reload(); } }
void BrowserPageGoanna::pageStop()    { if (mPage) mPage->Stop(); }
void BrowserPageGoanna::clearHistory() { if (mPage) mPage->ClearHistory(); }
void BrowserPageGoanna::getHistoryState(bool* back, bool* fwd) {
  if (back) *back = mPage && mPage->CanGoBack();
  if (fwd)  *fwd  = mPage && mPage->CanGoForward();
}

void BrowserPageGoanna::clickAt(int x, int y, int numClicks) {
  if (!mPage) return;
  fprintf(stderr, "[jihad-bs] clickAt x=%d y=%d n=%d\n", x, y, numClicks);
  // The adapter already sends CONTENT coords (asyncCmdClickAt contentX/contentY =
  // (scroll+event)/zoom). The old mapToContent re-divided by zoom + re-added scroll, so
  // taps were double-transformed and missed the target at any zoom/scroll. Use directly.
  // Record ONLY. All engine interaction for the tap — activation, hit-test
  // (ElementFromPoint flushes layout), mouse/DOMClick synthesis, and navigation — runs in
  // pump() on the tick. clickAt is a YAP socket callback (JihadBrowserServer mInTick==false)
  // with no page-lifetime protection; any of those calls can run page script (onfocus,
  // onmousedown, onclick -> location/history/document.open) that tears the document down
  // mid-callback, then we keep using freed engine objects -> SIGSEGV whose core dump
  // stalled I/O hard enough to REBOOT the device. pump() runs inside the tick's
  // mInTick/mReap guard, so teardown there is safe.
  queueInput(PM_CLICK, x, y, numClicks);
  mNeedsPaint = true;
}
void BrowserPageGoanna::keyDown(int key, int modifiers, int chr) {
  if (!mPage) return;
  // T-131 instrument (cavekit-preferences-ui.md R6), same JIHAD_LOG_INSERT gate as InsertText's.
  // WARRANTED because of an asymmetry that would otherwise void the run: a VKB keystroke arriving
  // with no focused editable never reaches InsertText at all — it falls through to engine dispatch
  // below — so the InsertText probe alone prints NOTHING for the on-device typing case, and an empty
  // log reads as "the instrument did not fire" rather than as the answer. (The inject `text` channel
  // does NOT come through here; it calls InsertText directly, which is why one probe cannot cover
  // both.) NEVER the key or the char (F-163) — the branch bit only.
  { static const bool kLogInsert = getenv("JIHAD_LOG_INSERT") != nullptr;
    if (kLogInsert)
      fprintf(stderr, "[jihad-bs] keydown: editable=%d\n", mPage->HasFocusedEditable() ? 1 : 0); }
  // Never log key/chr — those are user keystrokes (F-163). When a tapped editable is the type
  // target, the webOS VKB delivers each character (and the editing keys) as a keyDown; route it
  // into the field via the engine editor (caret-aware value edits + SetSelectionRange), and render
  // IMMEDIATELY (don't wait for the ~16ms tick) so typing feels responsive. Non-editable focus
  // falls through to normal engine key dispatch.
  // JIHAD_GRE_KEYS — AN EXPERIMENT SWITCH, default off, for cavekit-gre-widgets.md R3: hand the
  // keys of a focused text field to the ENGINE's editor (mapped to DOM codes first) instead of
  // editing the field here. It exists to answer, on the device, the questions that criterion says
  // must be answered before any handover — does a synthesized keypress reach the focused editor,
  // and does lifting the guard in GoannaRenderPage::KeyEvent bring back the crash it was put
  // there for — and to give a person with the on-screen keyboard something to compare. Nothing
  // ships with it on; the kit records what the run showed.
  // Enter and Tab are NOT handed over, on purpose: the engine's Enter in a single-line field
  // clicks the form's submit control from inside this key callback (the crash FireFormSubmit
  // exists to avoid, and a navigation outside pump()), and its Enter in a <textarea> inserts
  // nothing because a synthesized keypress cannot carry the key name the editor tests for.
  { static const bool kGreKeys = getenv("JIHAD_GRE_KEYS") != nullptr;
    const bool enterOrTab = key == 13 || chr == 13 || key == 9 || chr == 9;
    if (kGreKeys && !enterOrTab && mPage->HasFocusedEditable()) {
      mPage->KeyEvent("keydown", key, chr, modifiers, true);
      mPage->KeyEvent("keypress", key, chr, modifiers, true);
      mNeedsPaint = true;
      maybePaint();
      return;
    } }
  if (mPage->HasFocusedEditable()) {
    // webOS delivers editing keys as ASCII control codes where they exist (ESC=27, Backspace=8,
    // Tab=9, Enter=13, DEL=127) and, for the arrows, its own private-use block 0xE0A0-0xE0A3.
    // The code arrives in `key` with `chr`==0, but we accept it from either field. A printable
    // character inserts at the caret; the editing keys drive the engine caret.
    //
    // T-127 (2026-08-10) DELETED the Qt (0x0100000x/1x) and Apple/NSEvent (0xF7xx) blocks that sat
    // here as "fallbacks". Both were guesses, written in the same commit (f88cd932) that measured
    // the real codes, and both are now PROVEN unreachable by reading the shipped device binary
    // build/webos-oe/device-binaries/libWebKitLuna.so (unstripped). No device run was involved:
    //  * the webOS key path is 16 BITS WIDE end to end. Palm::WebView::keyEvent(unsigned short,
    //    unsigned short, bool) @0x75f760 -> PlatformKeyboardEvent(unsigned short, unsigned short,
    //    bool) @0x540ca4 (540cbc: strh r1,[r7,#34]) -> PluginView::handleKeyboardEvent @0x4e8e68
    //    reads it back (4e8eec: ldrh r0,[r2,#34]) into NPEvent.data.keyEvent.rawkeyCode; `chr` is
    //    one UTF-16 unit of the same text (4e8eb8: ldrh r3,[r1]). Both are <= 0xFFFF, so no
    //    Qt::Key value (>= 0x01000001) can ever appear in either field.
    //  * WebCore::keyIdentifierForPalmCode @0x53fc78 is the authoritative webOS code table:
    //    0x0C Clear, 0x0D Enter, 0x12 Alt, 0x13 Pause, 0x1B Back, 0x2E "U+007F", 0x80-0x83
    //    Shift/Alt/Ctrl/Opt, 0x91 Scroll, 0xE0A0 Up, 0xE0A1 Down, 0xE0A2 Left, 0xE0A3 Right,
    //    0xE0B1 Home (the DEVICE Home button), 0xE0B2 Launcher, 0xE0B3/B4 PageUp/PageDown,
    //    0xE0E2/E3 Previous/Next, 0xE0E5 Menu, 0xE0E7 Meta. No 0xF7xx anywhere in it, nor in
    //    EditorClient::translateKeyWithStickyState or EditorStickyState::*. The NSEvent range
    //    came from WebKit's platform/ios/KeyEventCodesIOS.h, which only the Cocoa port compiles.
    // That table has NO End code at all, which is why Home/End went with the blocks (see below).
    typedef jihad::GoannaRenderPage GRP;
    // kDelete=127 is the 2026-07-15 on-device measurement and is left alone, but T-141 should
    // settle a conflict: the table above says the RAW webOS forward-Delete code is 0x2E and only
    // its keyIdentifier is "U+007F". If 127 only ever arrives in `chr`, a raw 0x2E types '.'.
    const int kBackspace = 8, kTab = 9, kEnter = 13, kDelete = 127;
    // webOS's own special-key block, MEASURED 2026-10-01 on webOS CE 3.1.0 with a kernel input
    // keyboard (render/goanna/test/uinput_kbd.c — LunaSysMgr takes it for a Bluetooth keyboard),
    // read at this daemon's key command with JIHAD_LOG_KEYS: every key arrives in `key` with
    // chr == 0, and
    //     Up 0xE0A0   Down 0xE0A1   Left 0xE0A2   Right 0xE0A3
    //     Home 0xE0B1 End 0xE0B2    PageUp 0xE0B3 PageDown 0xE0B4
    //     Delete 0x7F Backspace 0x08 Tab 0x09     Enter 0x0D        (Escape never arrives)
    // That SETTLES the Up/Down question this block carried since 2026-07-15: the device binary's
    // table was right and the names here were swapped (0xE0A0 was called Down). It also retires
    // two readings of that table — 0xE0B1/0xE0B2 are a keyboard's Home and End, not only the
    // device's Home button and Launcher, and forward Delete is 0x7F in `key`, not 0x2E.
    const int kWebUp = 0xE0A0, kWebDown = 0xE0A1, kWebLeft = 0xE0A2, kWebRight = 0xE0A3;
    const int kWebHome = 0xE0B1, kWebEnd = 0xE0B2;
    const bool shift = (modifiers & (1 << 2)) != 0;   // npPalmShiftKeyModifier
    auto km = [key, chr](int v) { return key == v || chr == v; };
    if (!km(kBackspace)) mBackspaceRun = 0;   // any non-Backspace key breaks the accelerate run
    if (km(kBackspace)) {
      // Accelerate a held Backspace: consecutive presses within the auto-repeat window build a run;
      // once sustained (~0.5s of holding), delete a whole word per repeat so long text clears fast.
      int64_t now = jihadNowMs();
      mBackspaceRun = (now - mLastBackspaceMs <= 250) ? (mBackspaceRun + 1) : 1;
      mLastBackspaceMs = now;
      if (mBackspaceRun >= 12) mPage->DeleteBackwardWord();
      else                     mPage->DeleteBackward();
      mNeedsPaint = true;
    } else if (km(kDelete)) {
      mPage->EditKey(GRP::EK_DELETE); mNeedsPaint = true;
    } else if (km(kTab)) {
      // Tab: <textarea> inserts a tab, single-line <input> moves to the next field (fires focus JS)
      // so defer to pump() (F-219). Shift+Tab goes backward off the MODIFIER; the Qt backtab code
      // that used to be tested here cannot arrive (16-bit path, see above).
      mPendingEditActions.push_back(shift ? PEA_TAB_BACK : PEA_TAB); mNeedsPaint = true;
    } else if (km(kEnter)) {
      // Enter may submit a form and navigate, so defer to pump() (never navigate in a key callback).
      mPendingEditActions.push_back(PEA_ENTER); mNeedsPaint = true;
    } else if (km(kWebLeft))  { mPage->EditKey(GRP::EK_LEFT);  mNeedsPaint = true; }
    else if (km(kWebRight)) { mPage->EditKey(GRP::EK_RIGHT); mNeedsPaint = true; }
    else if (km(kWebUp))    { mPage->EditKey(GRP::EK_UP);    mNeedsPaint = true; }
    else if (km(kWebDown))  { mPage->EditKey(GRP::EK_DOWN);  mNeedsPaint = true; }
    else if (km(kWebHome))  { mPage->EditKey(GRP::EK_HOME);  mNeedsPaint = true; }
    else if (km(kWebEnd))   { mPage->EditKey(GRP::EK_END);   mNeedsPaint = true; }
    // Home/End are back as of 2026-10-01, on the codes a keyboard really sends (see above); the
    // Qt and NSEvent codes they were once wired to cannot arrive here. On-screen keyboard users
    // have neither key and still get field start/end from Up/Down in a single-line field.
    else {
      // The webOS VKB puts the printable character (letters, digits, symbols, and non-ASCII) in
      // `key` (rawkeyCode) with `chr`==0 on device; accept it from whichever field carries it, for
      // the FULL Unicode range (not just ASCII), then UTF-8 encode -> InsertText. Anything >= 0x20
      // and not DEL is treated as a character; Backspace/Enter/Tab (< 0x20) are handled elsewhere.
      unsigned int c = (chr >= 0x20 && chr != 0x7f) ? (unsigned int)chr
                     : (key >= 0x20 && key != 0x7f) ? (unsigned int)key : 0u;
      // Only encode a valid Unicode scalar: reject the UTF-16 surrogate range and anything
      // above U+10FFFF so we never emit malformed UTF-8 into the DOM value (Codex F-210). We do
      // NOT reject keycodes like 0x25-0x28 as "arrow keys": on this device the VKB delivers the
      // actual character in `key`, so those values are the '%','&',apostrophe,'(' symbols — the
      // VKB has no arrow keys. Enter/Tab/Backspace (< 0x20) are handled/swallowed above.
      if (c >= 0xD800 && c <= 0xDFFF) c = 0u;
      else if (c > 0x10FFFF) c = 0u;
      // Function/nav keys arrive as Private-Use code points, NOT text. The ones we map are consumed
      // above; swallow the rest here so an unmapped nav key never inserts an invisible PUA glyph
      // (which read as "arrows type characters" + repaint lag + Backspace eating the invisible
      // chars). 0xE0A0-0xE0BF is webOS's live block, no longer a guess: keyIdentifierForPalmCode
      // (libWebKitLuna @0x53fc78) defines 0xE0A0-A3 arrows, 0xE0B1 Home button, 0xE0B2 Launcher,
      // 0xE0B3/B4 PageUp/PageDown. The same table's 0xE0E2-0xE0E7 (Previous/Next/Menu/Meta)
      // sit past 0xE0BF and are swallowed by their own range below (T-213, 2026-10-01) — until
      // then they fell through and inserted an invisible glyph like any other PUA code.
      // 0xF700-0xF8FF is now a GUARD ONLY - its mapping constants were deleted because nothing on
      // this platform emits them (same table, no 0xF7xx anywhere). Kept because the asymmetry is
      // one-sided: one compare, versus re-opening the invisible-glyph bug above if that binary
      // read is wrong. Do NOT read its presence as evidence that webOS uses the NSEvent range.
      else if ((c >= 0xE0A0 && c <= 0xE0BF) || (c >= 0xE0E2 && c <= 0xE0E7)) c = 0u;
      else if (c >= 0xF700 && c <= 0xF8FF) c = 0u;
      if (c) {
        char buf[5] = {0};
        if (c < 0x80) { buf[0] = (char)c; }
        else if (c < 0x800) { buf[0] = (char)(0xC0 | (c >> 6)); buf[1] = (char)(0x80 | (c & 0x3F)); }
        else if (c < 0x10000) { buf[0] = (char)(0xE0 | (c >> 12)); buf[1] = (char)(0x80 | ((c >> 6) & 0x3F)); buf[2] = (char)(0x80 | (c & 0x3F)); }
        else { buf[0] = (char)(0xF0 | (c >> 18)); buf[1] = (char)(0x80 | ((c >> 12) & 0x3F)); buf[2] = (char)(0x80 | ((c >> 6) & 0x3F)); buf[3] = (char)(0x80 | (c & 0x3F)); }
        mPage->InsertText(buf);
        mNeedsPaint = true;
      }
      // else: an unmapped control/nav key — swallow it (do not insert).
    }
    if (mNeedsPaint) maybePaint();   // paint now, in this keyDown, instead of waiting for the tick
    return;
  }
  mPage->KeyEvent("keydown", key, chr, modifiers);
  // …AND a keypress. Gecko does NOT derive keypress from keydown for SYNTHESIZED events — the
  // native widget code that normally does it is not in this path — so a caller that sends only
  // keydown/keyup produces a key sequence no keypress listener ever sees. That is not a corner
  // case: XUL `<key>` elements match on keypress (about:config's own
  // `<key keycode="VK_RETURN" oncommand="ModifySelected()">` is one), and plenty of web pages
  // listen for keypress rather than keydown. Measured 2026-08-04: with keydown/keyup alone the
  // document saw `down:65,up:65` and no `press:` at all, and Enter on a selected about:config
  // row did nothing.
  mPage->KeyEvent("keypress", key, chr, modifiers);
  mNeedsPaint = true;
}
void BrowserPageGoanna::keyUp(int key, int modifiers, int chr) {
  static const bool kGreKeys = getenv("JIHAD_GRE_KEYS") != nullptr;   // see keyDown
  if (mPage) { mPage->KeyEvent("keyup", key, chr, modifiers, kGreKeys); mNeedsPaint = true; }
}
// Shared enqueue for every input kind (F-9). Records only — pump() dispatches.
void BrowserPageGoanna::queueInput(int type, int x, int y, int detail) {
  // Coalesce a run of pointer moves into the newest position. A drag delivers a move per touch
  // sample (~60/s) while pump() runs on the ~16 ms tick, so without this the queue grows faster
  // than it drains and every stale intermediate position is still dispatched — latency that
  // compounds for as long as the finger is down. Only the LAST move matters for hover/drag
  // tracking. Merging only when the queue's LAST entry is also a move is what keeps this safe: a
  // move that must PRECEDE a down/click is never swallowed, and down/up/click/contextmenu — each
  // semantically significant — are never coalesced.
  if ((type == PM_MOVE || type == PM_TOUCHMOVE) && !mPendingMouse.empty() &&
      mPendingMouse.back().type == type) {
    mPendingMouse.back().x = x; mPendingMouse.back().y = y;
    return;
  }
  // Bound the queue so a page whose handler never yields cannot grow it without limit. Reaching
  // this needs a multi-second pump stall (coalescing keeps a normal drag at ~1 entry), so the
  // policy only has to be sane, not clever: drop the OLDEST entry — and, when that is a
  // press, its matching release too, so a half-gesture is never left behind (a lone mouseup is a
  // stray DOM event; a lone mousedown latches the element :active and the page's drag state until
  // the next gesture).
  if (mPendingMouse.size() >= 256) {
    int dropped = mPendingMouse.front().type;
    mPendingMouse.erase(mPendingMouse.begin());
    if (dropped == PM_DOWN || dropped == PM_TOUCHSTART) {
      const int mate = (dropped == PM_DOWN) ? PM_UP : PM_TOUCHEND;
      for (size_t i = 0; i < mPendingMouse.size(); ++i) {
        if (mPendingMouse[i].type == mate) { mPendingMouse.erase(mPendingMouse.begin() + i); break; }
        if (mPendingMouse[i].type == dropped) break;   // next gesture began; this one had no release
      }
    }
  }
  PendingMouse pm; pm.type = type; pm.x = x; pm.y = y; pm.detail = detail;
  mPendingMouse.push_back(pm);
}

void BrowserPageGoanna::mouseEvent(int type, int x, int y, int /*detail*/) {
  if (!mPage) return;
  // The BrowserAdapter's wire convention is 0=mousedown, 1=mouseup, 2=mousemove
  // (asyncCmdMouseEvent in BrowserClientBase). A tap sends down(0) then up(1); the old
  // mapping (1->down, 2->up, 0->move) was shifted, so a tap became move+down with no
  // mouseup -> links/buttons never activated. Match the adapter exactly.
  // RECORD ONLY (F-9) — see mPendingMouse in the header for why this must not dispatch here.
  // x,y are already CONTENT coords (see clickAt) -- do not re-map.
  int t = (type == 0) ? PM_DOWN : (type == 1) ? PM_UP : PM_MOVE;
  queueInput(t, x, y, 0);
  mNeedsPaint = true;
}
void BrowserPageGoanna::holdAt(int x, int y) {
  if (!mPage) return;
  // x,y are already CONTENT coords, exactly like mouseEvent/clickAt: the adapter's long-press
  // sends asyncCmdHoldAt(m_penDownDoc.x, m_penDownDoc.y) — the SAME m_penDownDoc it passes to
  // asyncCmdMouseEvent(mousedown) (render/adapter/BrowserAdapter.cpp). The old mapToContent here
  // re-divided by zoom and re-added the engine scroll, so a long-press on a scrolled page resolved
  // against a point far below the finger (scroll y=1000 -> contextmenu at y~2000: wrong element or
  // none). This is the same double-transform already fixed for clickAt. Removing it also drops a
  // layout flush (mapToContent -> GetScrollXY(flushLayout=true)) that ran in the YAP callback —
  // precisely the unguarded engine work this queue exists to eliminate.
  queueInput(PM_CONTEXTMENU, x, y, 2);
  mNeedsPaint = true;
}

void BrowserPageGoanna::docToViewport(int* x, int* y) {
  if (!mPage) return;
  double z = (mZoom >= 0.99 && mZoom <= 1.01) ? 1.0
           : ((mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0);
  // The adapter's document coords are already CSS px: every path that makes them divides by
  // its own zoom first (clickAt, the native tap, the pen-down anchor: (scroll + point) / zoom).
  // This divided AGAIN (since 2026-08-02, written for "zoomed doc px"), so on any page shown
  // at a zoom other than 1 a tap landed 1/zoom too far right and down — desktop Wikipedia at
  // its 0.69 fit took a tap at x=751 to x=1091, past the article (owner 2026-10-08: "mojo tap
  // targets seem inaccurate … I get duckduckgo when I click wikipedia").
  double cx = *x, cy = *y;
  if (z != 1.0) {
    // Zoomed: the engine scroll is parked at 0; the visual-viewport pan carries the offset.
    double px = 0, py = 0; mPage->GetRenderPan(&px, &py);
    cx -= px; cy -= py;
  } else {
    // z~1: the engine scroll carries it. No layout flush in an input path.
    int ex = 0, ey = 0;
    if (mPage->GetScrollXY(&ex, &ey, /*flushLayout*/false)) { cx -= ex; cy -= ey; }
  }
  *x = (int)(cx + 0.5); *y = (int)(cy + 0.5);
}

// Double-tap zoom (YAP ZoomSmartCalculateRequest → SmartZoomCalculateResponseSimple). The request
// was a stub here until 2026-10-07, so a double tap did nothing in any shell (owner: "double
// tapping to zoom does not work"). (x,y) arrive in CSS page px — the adapter has already added
// its scroll and divided by its zoom (prvSmartZoom) — so only the scroll/pan is taken off here
// (the second half of docToViewport), and put back on the answer. The adapter fits the block's
// width to the screen, and a second double tap on it goes back to the page's fit.
void BrowserPageGoanna::smartZoomRequest(int x, int y) {
  if (!mPage) return;
  double z = (mZoom >= 0.99 && mZoom <= 1.01) ? 1.0 : ((mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0);
  double ox = 0, oy = 0;
  if (z != 1.0) { mPage->GetRenderPan(&ox, &oy); }
  else { int ex = 0, ey = 0; if (mPage->GetScrollXY(&ex, &ey, false)) { ox = ex; oy = ey; } }
  int vx = (int)(x - ox + 0.5), vy = (int)(y - oy + 0.5);
  int l = 0, t = 0, r = 0, b = 0;
  if (!mPage->SmartZoomBlockAt(vx, vy, &l, &t, &r, &b)) {
    fprintf(stderr, "[jihad-bs] smartZoom at %d,%d: no block\n", x, y);
    return;
  }
  l += (int)ox; r += (int)ox; t += (int)oy; b += (int)oy;
  fprintf(stderr, "[jihad-bs] smartZoom at %d,%d -> block [%d,%d %d,%d]\n", x, y, l, t, r, b);
  mSink.msgSmartZoomCalculateResponseSimple(x, y, l, t, r, b, 0);
}

void BrowserPageGoanna::hitTest(int x, int y, std::string* json) {
  // x,y arrive as DOCUMENT coords (the adapter's m_penDownDoc); the engine query wants
  // viewport-relative CSS px — same mapping as the input drain. Runs the DOM query inline
  // (read-only — no event dispatch, no layout mutation), so the reply can be sent from the
  // YAP callback and the adapter's queued mousehold proceeds without waiting on a tick.
  if (!mPage) return;
  docToViewport(&x, &y);
  mPage->HitTestAt(x, y, json);
}

void BrowserPageGoanna::emitSelectPopupIfPending() {
  if (!mPage) return;
  std::string json, id;
  if (!mPage->TakeSelectPopup(&json, &id)) return;
  // The adapter reads the menu data from a FILE and unlinks it (BrowserAdapter::msgPopupMenuShow).
  // Write it to the variant's root-owned state dir (R8: never /media/internal, never /tmp world-
  // writable) as <state>/popup-<id>.json, 0644 (root daemon writes, the card-side adapter reads).
  std::string path = jihad::RuntimeResolvePath("1", (std::string("popup-") + id + ".json").c_str());
  if (path.empty()) {
    fprintf(stderr, "[jihad-bs] popupMenuShow: no state dir — dropping select popup\n");
    return;
  }
  FILE* f = fopen(path.c_str(), "wb");
  if (!f) { fprintf(stderr, "[jihad-bs] popupMenuShow: cannot write %s\n", path.c_str()); return; }
  // Fail CLOSED on a short write: the adapter forwards a truncated/empty file verbatim and
  // the card's createSelectPopup then throws inside JSON.parse mid-flow — AFTER showSpinner()
  // but before hideSpinner() — leaving a modal scrim over the card until relaunch (review #6).
  size_t wrote = fwrite(json.data(), 1, json.size(), f);
  int closed = fclose(f);
  if (wrote != json.size() || closed != 0) {
    fprintf(stderr, "[jihad-bs] popupMenuShow: short write %zu/%zu on %s — dropping popup\n",
            wrote, json.size(), path.c_str());
    unlink(path.c_str());
    return;
  }
  chmod(path.c_str(), 0644);
  fprintf(stderr, "[jihad-bs] popupMenuShow id=%s items->%s\n", id.c_str(), path.c_str());
  mSink.msgPopupMenuShow(id.c_str(), path.c_str());
}

// --- engine JS dialogs -> this page's card (browser-services R3) ------------------------------
//
// The engine raises a dialog on its own thread and BLOCKS the page until it has an answer. The
// frozen contract carries the question over YAP with the path of a FIFO, and the card's
// sendDialogResponse writes the answer back through it (BrowserAdapter::js_sendDialogResponse:
// a 4-byte big-endian length, then each argument NUL-terminated). Until now nothing in the daemon
// implemented DialogSink at all, so every engine dialog took its default and no card ever saw one.
//
// Blocking here is safe and is what the contract expects: the YAP message is written to the socket
// before we wait, and the answer comes back through the FIFO, not through the tick loop we are
// standing in. The TIMEOUT is the part that must not be skipped — a card that never answers (no
// dialog handler, a card that died mid-prompt) would otherwise hang the daemon forever.
// Create the reply FIFO for one dialog. Empty on failure (caller then takes the default).
std::string BrowserPageGoanna::makeDialogPipe() {
  static unsigned sSeq = 0;
  std::string path = jihad::RuntimeResolvePath("1",
      (std::string("dialog-") + std::to_string(++sSeq) + ".fifo").c_str());
  if (path.empty()) {
    fprintf(stderr, "[jihad-bs] dialog: no state dir for the reply pipe\n");
    return std::string();
  }
  unlink(path.c_str());
  if (mkfifo(path.c_str(), 0600) != 0) {
    fprintf(stderr, "[jihad-bs] dialog: mkfifo %s failed (%s)\n", path.c_str(), strerror(errno));
    return std::string();
  }
  return path;
}

// Block until the card answers through the FIFO, or the deadline passes. Fills accept/value
// from the adapter's wire format (4-byte big-endian length, then NUL-terminated args; arg0
// "1" accept / "0" cancel / "2" SSL trust-once, arg1 prompt text or username).
// The deadline is load-bearing: a card that never answers must not wedge the daemon.
bool BrowserPageGoanna::awaitDialogReply(const std::string& path, const char* what,
                                         bool* accept, std::string* value, int* answer,
                                         std::string* value2) {
  int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "[jihad-bs] dialog: cannot open reply pipe (%s)\n", strerror(errno));
    unlink(path.c_str());
    return false;
  }
  // ONE deadline, sized for a person.
  //
  // This used to be two: a short 5 s "has anyone picked up the phone" wait (detected by the
  // read-only FIFO's POLLHUP clearing) followed by a long wait for the answer. That reasoning
  // was wrong about how the card replies. BrowserAdapter::js_sendDialogResponse opens the
  // pipe ONLY at the moment the user answers (BrowserAdapter.cpp — fopen(gDialogResponsePipe)
  // inside the response handler); nothing opens the write end while the dialog is merely on
  // screen. So POLLHUP never cleared early, the 5 s deadline always won, and EVERY dialog a
  // human took more than five seconds to answer was silently defaulted — the daemon had
  // already unlinked the FIFO by the time they tapped.
  //
  // That is the "clicking the button to install the add-on doesn't do anything" report:
  // it worked in every harness, because a scripted answerer replies in ~300 ms.
  //
  // The short deadline existed to stop a daemon with no card — or a front-end with no handler
  // for this dialog kind — from wedging for a full minute. That cost is real but bounded, and
  // it is far better than answering for the user. Harnesses that WANT a fast default set
  // JIHAD_DIALOG_MS.
  long deadlineMs = 60000;
  if (const char* e = getenv("JIHAD_DIALOG_MS")) {
    long v = atol(e);
    if (v > 0) deadlineMs = v;
  }
  std::string data;
  int64_t start = jihadNowMs();
  bool got = false, writerSeen = false;
  for (;;) {
    int64_t elapsed = jihadNowMs() - start;
    if (elapsed >= deadlineMs) break;
    struct pollfd pfd; pfd.fd = fd; pfd.events = POLLIN; pfd.revents = 0;
    int pr = poll(&pfd, 1, 200);
    if (pr < 0) { if (errno == EINTR) continue; break; }
    if (pr == 0) continue;
    if (!(pfd.revents & POLLHUP) && !writerSeen) {
      // First moment the card has the write end open. Timed because "the dialog took a
      // while to appear" is otherwise unattributable: this line separates OUR side (engine
      // work to produce the dialog, already measured at ~2 ms on desktop) from the card's
      // side (YAP delivery + the front-end actually drawing something). Only the second
      // number can be large, and only the card can fix it.
      // (long) is not decoration: jihadNowMs() is int64_t and long is 32-bit on this ARM
      // target, so a bare %ld prints one word of a two-word vararg — a garbage number in the
      // one line that exists to be read as a measurement.
      fprintf(stderr, "[jihad-bs] dialog %s: card picked up after %ld ms\n",
              what, (long)(jihadNowMs() - start));
    }
    if (!(pfd.revents & POLLHUP)) writerSeen = true;   // someone has the write end open
    char buf[512];
    ssize_t n = read(fd, buf, sizeof buf);
    if (n > 0) { data.append(buf, (size_t)n); got = true; writerSeen = true; continue; }
    // A writer closing is NOT proof the answer is complete: the reply is a 4-byte length
    // followed by that many bytes, and a writer that opens/closes per write (or is simply
    // scheduled between the two) delivers the header alone. Breaking there truncated the
    // answer to nothing and every dialog read as "cancel" — believe the declared length,
    // not the close.
    if (n == 0 && got && data.size() >= 4) {
      const unsigned char* L = reinterpret_cast<const unsigned char*>(data.data());
      size_t declared = ((size_t)L[0] << 24) | ((size_t)L[1] << 16) |
                        ((size_t)L[2] << 8)  |  (size_t)L[3];
      if (data.size() >= 4 + declared) break;    // complete
    }
    if (n < 0 && errno != EAGAIN && errno != EINTR) break;
  }
  if (!writerSeen && !got) {
    fprintf(stderr, "[jihad-bs] dialog %s: nobody opened the reply pipe in %ld ms — "
                    "no card is answering, taking the default\n", what, deadlineMs);
  }
  close(fd);
  unlink(path.c_str());
  if (!got || data.size() < 4) {
    fprintf(stderr, "[jihad-bs] dialog %s: no answer from the card — taking the default\n", what);
    return false;
  }
  fprintf(stderr, "[jihad-bs] dialog %s: answered after %ld ms\n", what,
          (long)(jihadNowMs() - start));
  size_t pos = 4;
  std::vector<std::string> args;
  while (pos < data.size() && args.size() < 4) {
    size_t end = data.find('\0', pos);
    if (end == std::string::npos) { args.push_back(data.substr(pos)); break; }
    args.push_back(data.substr(pos, end - pos));
    pos = end + 1;
  }
  // The three-way answer is carried THROUGH, not just collapsed. arg0 is "0"
  // reject / "1" trust always / "2" trust once; every dialog but the SSL confirm
  // only cares about the collapse, and the SSL confirm cannot act correctly
  // without the distinction (permanent vs session trust, on both sides).
  int raw = 0;
  if (!args.empty()) raw = (args[0] == "1") ? 1 : (args[0] == "2") ? 2 : 0;
  if (accept) *accept = (raw != 0);
  if (answer) *answer = raw;
  if (args.size() > 1 && value) *value = args[1];
  if (args.size() > 2 && value2) *value2 = args[2];   // user-name/password dialog: the password
  fprintf(stderr, "[jihad-bs] dialog %s -> %s (answer=%d)\n", what,
          raw ? "ACCEPT" : "cancel", raw);
  return true;
}

void BrowserPageGoanna::OnDialog(DialogKind kind, const char* text, DialogReply* reply) {
  if (!reply) return;
  // Defaults are the safe answer for each kind, and stand if anything below fails.
  reply->accept = false;
  reply->promptValue.clear();

  // The engine's own connection-error alert for a certificate we are about to ASK about.
  // With no error pages in this embedding the docshell reports a failed load through an alert —
  // "An error occurred during a connection to <host>. … (Error code: SSL_ERROR_…)" — and for a
  // certificate failure it arrives first, so the user got an unexplained modal about a
  // "malformed Server Hello" and only then, after dismissing it, the trust question that
  // actually matters (device screenshot 2026-10-01; unattended it also cost the full dialog
  // deadline before the SSL confirm appeared). The SSL confirm is the report of that failure.
  // Matched narrowly — a captured certificate error AND an NSS error identifier in the text —
  // so a page's own alert() is never swallowed.
  if (kind == DialogKind::Alert && mPage && text) {
    std::string h; int c = 0;
    if (mPage->GetCertError(&h, &c) &&
        (strstr(text, "SSL_ERROR_") || strstr(text, "SEC_ERROR_") || strstr(text, "MOZILLA_PKIX_ERROR_"))) {
      fprintf(stderr, "[jihad-bs] dialog alert suppressed — the certificate error on %s is reported "
                      "by the SSL confirm instead\n", h.c_str());
      reply->accept = true;
      return;
    }
  }

  std::string path = makeDialogPipe();
  if (path.empty()) return;

  const char* kindName = (kind == DialogKind::Alert)   ? "alert"
                       : (kind == DialogKind::Confirm) ? "confirm"
                       : (kind == DialogKind::Prompt)  ? "prompt" : "auth";
  fprintf(stderr, "[jihad-bs] dialog %s -> card (pipe %s)\n", kindName, path.c_str());

  switch (kind) {
    case DialogKind::Alert:   mSink.msgDialogAlert(path.c_str(), text ? text : ""); break;
    case DialogKind::Confirm: mSink.msgDialogConfirm(path.c_str(), text ? text : ""); break;
    case DialogKind::Prompt:  mSink.msgDialogPrompt(path.c_str(), text ? text : "",
                                                    reply->promptValue.c_str()); break;
    default:                  mSink.msgDialogUserPassword(path.c_str(), text ? text : ""); break;
  }

  awaitDialogReply(path, kindName, &reply->accept, &reply->promptValue, nullptr,
                   kind == DialogKind::Auth ? &reply->password : nullptr);
}

bool BrowserPageGoanna::popupsOpen() const {
  return mPage ? mPage->PopupsOpen() : false;
}

void BrowserPageGoanna::popupMenuSelect(const char* identifier, int selectedIdx) {
  // The card's native <select> list returned a choice (asyncCmdPopupMenuSelect). Apply it to
  // the held element and fire input/change; a negative index is a dismissal (no change).
  if (!mPage) return;
  fprintf(stderr, "[jihad-bs] popupMenuSelect id=%s idx=%d\n", identifier ? identifier : "", selectedIdx);
  mPage->ApplySelectPopup(identifier, selectedIdx);
  mNeedsPaint = true;
}

void BrowserPageGoanna::pageCommand(const char* cmd, int queryNum) {
  if (!cmd) return;
  // A queue a card cannot grow without bound: sixteen is a great many menu taps per tick.
  if (!mPage || mPendingPageCmds.size() >= 16) {
    if (queryNum >= 0) mSink.msgCopySuccessResponse(queryNum, false);
    return;
  }
  PendingPageCmd c; c.cmd = cmd; c.queryNum = queryNum; c.navGen = mNavGen;
  mPendingPageCmds.push_back(c);
}

bool BrowserPageGoanna::runPageCommand(const char* cmd, const std::string& extraJson) {
  if (!mPage || !cmd) return false;
  // Copied text reaches the card as a file in the card's own application directory (the only
  // place a card may load a local file from — see the certificate details above for the same
  // constraint); the component is told where that is.
  std::string dir = jihad::RuntimeProfileDir();
  const size_t cut = dir.find_last_of('/');
  dir = (cut != std::string::npos && cut > 0) ? dir.substr(0, cut) + "/clip" : std::string();
  std::string json = std::string("{\"cmd\":\"") + cmd + "\",\"dir\":\"" + dir + "\"";
  if (!extraJson.empty()) { json += ","; json += extraJson; }
  json += "}";
  // The component reports a hand-over through the notification door, synchronously, so the
  // count moving across this call is the result.
  const unsigned before = jihad::PageCopiedCount();
  if (!mPage->NotifyChrome("jihad-page-command", json.c_str()))
    fprintf(stderr, "[jihad-bs] page command %s: no page window to act on\n", cmd);
  mNeedsPaint = true;
  return jihad::PageCopiedCount() != before;
}

void BrowserPageGoanna::insertStringAtCursor(const char* text) {
  // NB: never log `text` — it is user keystrokes (incl. passwords) and this stream is redirected
  // to a persistent, user-readable file on device (Jihad review F-163).
  if (mPage && text) { mPage->InsertText(text); mNeedsPaint = true; if (mNeedsPaint) maybePaint(); }
}

void BrowserPageGoanna::dragStart(int x, int y) {
  // Latch the finger position. Normally nothing needs it (deltas drive scroll), but while a
  // menu is open the drag is a menu drag-select and needs an absolute point to hit-test.
  mDragX = x; mDragY = y;
  if (!mPage) return;
  if (mPage->PopupsOpen()) {
    int cx = x, cy = y; docToViewport(&cx, &cy);
    mPage->PopupHover(cx, cy);
    mNeedsPaint = true;
  }
}

void BrowserPageGoanna::dragProcess(int deltaX, int deltaY) {
  if (!mPage) return;
  // While a menu is open the drag belongs to the menu, not the page: track the finger and
  // highlight the row under it. Scrolling the page out from under an open menu is never what
  // the user meant, and the menu would then be drawn over the wrong content anyway.
  mDragX += deltaX; mDragY += deltaY;
  if (mPage->PopupsOpen()) {
    int cx = mDragX, cy = mDragY; docToViewport(&cx, &cy);
    mPage->PopupHover(cx, cy);
    mNeedsPaint = true;
    return;
  }
  // Drag scrolls the content opposite the finger; surface deltas -> content px.
  double z = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
  int sx = 0, sy = 0; mPage->GetScrollXY(&sx, &sy);
  auto clamp = [](double v) -> int {
    if (v < -1000000.0) return -1000000; if (v > 1000000.0) return 1000000; return (int)v; };
  mPage->ScrollTo(clamp(sx - deltaX / z), clamp(sy - deltaY / z));
  mNeedsPaint = true;
}

void BrowserPageGoanna::dragEnd(int x, int y) {
  // Lifting over an open menu picks the highlighted row — the drag-select every desktop
  // menu does, and the natural touch equivalent of "press, slide, release".
  if (!mPage) return;
  if (mPage->PopupsOpen()) {
    int cx = (x || y) ? x : mDragX, cy = (x || y) ? y : mDragY;
    docToViewport(&cx, &cy);
    if (mPage->PopupActivate(cx, cy)) {
      fprintf(stderr, "[jihad-bs] drag ended over an open popup — picked that row\n");
    }
    mNeedsPaint = true;
  }
  /* otherwise: scroll already applied by dragProcess */
}

void BrowserPageGoanna::settingsJavaScriptEnabled(bool enable) {
  if (mPage) mPage->SetJavaScriptEnabled(enable);
}

// ---- double-activation suppressor (T-120, 2026-08-15) ---------------------------------------
//
// THE PROBLEM. With the adapter's JIHAD_TOUCH_EVENTS switch on, one finger reaches the page
// TWICE: once as DOM touch events (asyncCmdTouchEvent -> touchEvent above) and once as DOM mouse
// events (asyncCmdMouseEvent/clickAt -> queueInput). The second stream is not synthesized from
// the first — LunaSysMgr generates pen events for every finger whether or not the plugin also
// asked for touch — so Gecko's own touch-to-mouse suppression, which only ever suppresses mouse
// events it synthesized itself, cannot see the relationship. A page that handles touchstart and
// calls preventDefault (every carousel, map, and drawing surface) then gets its gesture AND a
// full mousedown/mouseup/click on top of it. That is the double-activation class this project
// treats as critical.
//
// THE ONLY AVAILABLE SIGNAL. sendTouchEventToWindow reports whether content called
// preventDefault. GoannaRenderPage::TouchEvent used to discard it; it now returns it, and this
// is what consumes it. preventDefault on touchstart is precisely the W3C signal for "I am
// handling this gesture, do not give me the compatibility mouse events".
//
// THE CORRELATION RULE, and why it is not by identity. The two streams share NO event id, no
// sequence number, and no timestamp — they are separate YAP commands. What they DO share is the
// one queue: touch and pen both land in mPendingMouse, so their relative ORDER is total and
// reliable (that is why the queue exists — see the PM_* comment in the header). So:
//
//   1. SEQUENCE STATE is the primary rule. From a touchstart that content consumed until the
//      matching touchend, every queued mouse/click/contextmenu event is dropped. No geometry is
//      involved; during a live consumed gesture there is nothing else the pen stream can be.
//   2. A TIME+DISTANCE WINDOW covers the tail. The pen-up and the synthesized click arrive just
//      AFTER the touchend, when rule 1 has already disarmed. For kTailMs after the touchend, a
//      mouse event within kSlopPx of the last touch point is dropped too.
//   3. A RUNAWAY CAP bounds the damage. If a touchend never arrives (a lost cancel, a card
//      switch or a navigation mid-gesture — a navigation clears the input queue but cannot know
//      a finger is still down), rule 1 would suppress ALL pointer input for the life of the page —
//      an unusable browser. Suppression from rule 1 therefore also expires kMaxSeqMs after the
//      touchstart. Nothing else in this file self-heals, so it is spelled out here.
//
// FAILURE MODES, stated rather than hidden. NONE of this has run on hardware; the ordering and
// the timing constants are reasoned from the code, not measured.
//   * A page that consumes touchMOVE but not touchSTART (a scroll-hijacking list) is NOT
//      suppressed: the rule keys on touchstart, matching the W3C compatibility-event rule. Such
//      a page gets the mouse stream as it does today, which is the current behaviour, not a
//      regression.
//   * A real second finger landing on a DIFFERENT element within kTailMs of a consumed gesture
//      ending, and within kSlopPx of where the first ended, loses its click. kSlopPx is small
//      (a finger's own jitter) to keep that window narrow.
//   * If the device delivers the pen-down BEFORE the touchstart, that one leading mousedown
//      escapes. Its mouseup/click still do not, so a click is not fabricated — but a page
//      watching mousedown alone would see one. Determining the real order needs a device.
//   * The window is wall-clock (gettimeofday, as everything else here is). A clock step during
//      a gesture mis-sizes one window; it cannot latch, because rule 1 does not use time except
//      as a cap.
static const int64_t kTouchTailMs   = 400;   // pen-up + click after a touchend
static const int64_t kTouchMaxSeqMs = 5000;  // runaway cap on an unterminated sequence
static const int     kTouchSlopPx   = 48;    // document px; a fingertip's own wander

bool BrowserPageGoanna::touchSuppressesMouse(int type, int docX, int docY) {
  if (type != PM_DOWN && type != PM_UP && type != PM_MOVE &&
      type != PM_CLICK && type != PM_CONTEXTMENU)
    return false;
  const int64_t now = jihadNowMs();
  if (mTouchSeqActive && mTouchSeqConsumed) {
    if (now - mTouchSeqStartMs <= kTouchMaxSeqMs) return true;
    // Rule 3: the sequence never ended. Disarm and let input through rather than wedge the page.
    fprintf(stderr, "[jihad-bs] touch suppressor: sequence ran %lldms with no touchend — disarming\n",
            (long long)(now - mTouchSeqStartMs));
    mTouchSeqActive = false; mTouchSeqConsumed = false; mTouchSuppressUntilMs = 0;
    return false;
  }
  if (now < mTouchSuppressUntilMs) {
    const int dx = docX - mTouchAnchorX, dy = docY - mTouchAnchorY;
    if (dx * dx + dy * dy <= kTouchSlopPx * kTouchSlopPx) return true;
  }
  return false;
}
void BrowserPageGoanna::touchEvent(int type, int /*count*/, int /*mods*/, const char* touchesJson) {
  if (!mPage) return;
  // Parse EVERY point in the touches payload, not just the first. The adapter builds
  // `[{"x":N,"y":N,"state":S}, …]` (BrowserAdapter::doTouchEvent) with one object per live
  // contact; keeping only the first made a two-finger gesture indistinguishable from a
  // one-finger one, so a pinch could not be expressed at all.
  //
  // COORDINATES ARE ALREADY IN DOCUMENT SPACE. The adapter divides by its zoom level and
  // subtracts the page offset before serializing (`touchDocPt`), unlike clickAt/mouseEvent,
  // which carry surface coordinates for us to map. This path used to call mapToContent on
  // them anyway — a second conversion of an already-converted point, which lands the touch
  // somewhere else entirely on any zoomed or scrolled page. The adapter is the contract, so
  // its coordinates are taken as given.
  std::vector<std::pair<int,int>> pts;
  if (touchesJson) {
    const char* c = touchesJson;
    while ((c = strstr(c, "\"x\"")) != nullptr) {
      int x = 0, y = 0;
      if (sscanf(c, "\"x\"%*[: ]%d", &x) != 1) break;
      const char* yk = strstr(c, "\"y\"");
      if (!yk || sscanf(yk, "\"y\"%*[: ]%d", &y) != 1) break;
      pts.push_back(std::make_pair(x, y));
      c = yk + 3;
    }
  }
  if (pts.empty()) return;
  // Queued like every other input (F-9/F5): TouchEvent -> SendTouchEventToWindow runs
  // touchstart/touchend page JS synchronously, so dispatching it from this YAP callback is the
  // same reboot-class crash door the queue was built to close.
  // type 3 is TouchCancelled (BrowserAdapter::handleTouchCancelled -> doTouchEvent(3, ...)). It used
  // to fall into the else and dispatch a phantom "touchmove" at the released coordinates, which is
  // worse than nothing: a page tracking a drag sees the finger STOP rather than leave. Ending the
  // sequence is the closest thing the current PM_ set can express; a real "touchcancel" needs a new
  // PM_ value, and the engine already accepts the string (GoannaRenderPage::TouchEvent passes the
  // type straight to nsIDOMWindowUtils::sendTouchEventToWindow). Reachable as of 2026-08-15 —
  // BrowserAdapter::doTouchEvent is compiled in — but only when LunaSysMgr is started with
  // JIHAD_TOUCH_EVENTS=1, and never yet exercised on a device.
  int t = (type == 0) ? PM_TOUCHSTART : (type == 2 || type == 3) ? PM_TOUCHEND : PM_TOUCHMOVE;
  queueInput(t, pts[0].first, pts[0].second, 0);
  // Assign in BOTH directions. queueInput coalesces a run of PM_TOUCHMOVE into the queue's last
  // entry, so a two-finger move followed by a one-finger move would otherwise keep the stale
  // two-point list and dispatch a contact the user has already lifted.
  if (pts.size() > 1) mPendingMouse.back().pts = pts;
  else                mPendingMouse.back().pts.clear();
  mNeedsPaint = true;
}

// Emit failure/cert + location + title/url for a settled load, and return the reported uri ("" if the
// load failed, so the caller skips writing history). Does NOT emit the load-stopped boundary — isis's
// pageLoadStopped writes the app history entry from the title/url this sets, so load-stopped must run
// AFTER this or history records stale data (Codex F-004/F-334).
std::string BrowserPageGoanna::emitLocationAndTitle() {
  // R5: an overridable certificate error surfaces as an SSL-confirm dialog rather than a generic
  // failed load. R3: other network failures -> failed.
  bool failed = false; int code = 0; std::string furl;
  mPage->GetLoadError(&failed, &code, &furl);
  std::string chost; int ccode = 0;
  bool certErr = mPage->GetCertError(&chost, &ccode);
  fprintf(stderr, "[jihad-bs] loaderr failed=%d code=0x%x certErr=%d chost=%s ccode=0x%x\n",
          (int)failed, (unsigned)code, (int)certErr, chost.c_str(), (unsigned)ccode);
  if (certErr) {
    // R5: ask the card whether to trust this certificate, and ACT on the answer. This used to
    // pass an empty reply-pipe path, so the card could show the prompt but never answer it and
    // nothing consumed a decision — AcceptCurrentCert() existed and had no caller outside the
    // test. Now it takes the same FIFO every other dialog uses: on accept we add the validity
    // override and RELOAD, which is what makes "trust this site" actually load the page; on
    // decline (or no answer) the failed load stands.
    // The card needs a REAL certificate file: it disables "View Certificate" on
    // every show and re-enables it only once com.palm.certificatemanager has
    // answered getcertificatedetails for this path, and CertificateDetail skips
    // that call entirely when certFile is empty. We passed "" here, so the
    // button was dead by construction on every prompt. The same PEM is what the
    // platform store installs, so it is produced once and used for both.
    std::string certFile;
    std::string der;
    if (mPage->GetCertDer(&der))
      certFile = jihad::certstore::WriteCertPem(der.data(), der.size());
    // …and the certificate's readable details, for the card to show. On webOS CE a card
    // cannot reach com.palm.certificatemanager at all ("Service does not exist" on the public
    // bus, measured 2026-10-02), and while this dialog is up the daemon is parked on the reply
    // pipe and can answer nothing over Luna either — so a file the card reads is the one
    // channel that works inside the window. Two constraints shape it, both measured:
    //   * WHERE: LunaSysMgr lets a card load local files only from its own application
    //     directory ("isAllowedFileAccess: NOT ALLOWING FILE ACCESS … target: /var/ssl/…"),
    //     so it goes in <appdir>/certinfo/, beside the profile this daemon already keeps there.
    //   * WHAT: a SCRIPT, one call carrying one JSON object. The card's WebKit refuses the
    //     same bytes over XMLHttpRequest ("Cross origin requests are only supported for HTTP").
    // The strings in it come from the server's certificate, so the JSON writer escapes
    // everything that could end a string or a line. Named after the PEM so the card can
    // derive it from the certFile it is handed; removed when the dialog resolves.
    std::string certInfoFile;
    if (!certFile.empty()) {
      std::string info;
      std::string appDir = jihad::RuntimeProfileDir();
      const size_t cut = appDir.find_last_of('/');
      const size_t base = certFile.find_last_of('/');
      if (cut != std::string::npos && cut > 0 && mPage->GetCertInfoJson(&info)) {
        const std::string dir = appDir.substr(0, cut) + "/certinfo";
        mkdir(dir.c_str(), 0755);
        const std::string side = dir + "/" + certFile.substr(base == std::string::npos ? 0 : base + 1) + ".js";
        certInfoFile = side;
        FILE* f = fopen(side.c_str(), "w");
        if (f) {
          fputs("if(window.jihadCertInfo){window.jihadCertInfo(", f);
          fwrite(info.data(), 1, info.size(), f);
          fputs(");}\n", f);
          fclose(f); chmod(side.c_str(), 0644);
        }
      }
    }

    // …and the error code has to be in the card's numbering space. mErrorStatus
    // is an nsresult; the card matches Palm's small ordinals, so an unmapped
    // value leaves the dialog body EMPTY under three unexplained buttons.
    jihad::certstore::Problem problem;
    mPage->GetCertProblem(&problem);
    const int ordinal = jihad::certstore::PalmSslErrorOrdinalFor(problem, ccode);

    std::string pipePath = makeDialogPipe();
    fprintf(stderr, "[jihad-bs] dialog ssl-confirm -> card (pipe %s) host=%s "
                    "status=0x%x flags=%d(untrusted=%d selfSigned=%d mismatch=%d badTime=%d "
                    "expired=%d) -> ordinal %d cert=%s\n",
            pipePath.empty() ? "(none)" : pipePath.c_str(), chost.c_str(), (unsigned)ccode,
            (int)problem.haveFlags, (int)problem.untrusted, (int)problem.selfSigned,
            (int)problem.domainMismatch, (int)problem.notValidNow, (int)problem.expired,
            ordinal, certFile.empty() ? "(none)" : certFile.c_str());
    mSink.msgSSLConfirm2(pipePath.c_str(), chost.c_str(), ordinal, certFile.c_str());
    if (!pipePath.empty()) {
      bool trust = false;
      int answer = 0;
      awaitDialogReply(pipePath, "ssl-confirm", &trust, nullptr, &answer);
      // "1" is Trust Always, "2" is Trust Once. These were collapsed into one
      // bool, and the override below was hardcoded temporary, so Trust Always
      // did not survive a daemon restart even in Goanna's own store.
      const bool permanent = (answer == 1);
      if (trust) {
        if (mPage->AcceptCurrentCert(permanent)) {
          // Write-through to the platform store so the rest of the device sees
          // the same decision. Absent or failing store = Goanna-only trust; it
          // must never turn an accepted certificate into a failed load.
          if (!certFile.empty()) {
            jihad::certstore::Install(certFile.c_str(),
                                      permanent ? jihad::certstore::kTrustPermanent
                                                : jihad::certstore::kTrustSession,
                                      this);
          }
          // Retry the load that FAILED, not the current document. CurrentUri() is the last
          // page that committed — when the certificate error came from a navigation away from
          // a page (the ordinary case: a link, a typed address), that is the page the user was
          // LEAVING, so "trust this site" reloaded the old page and the site never appeared
          // (device, 2026-10-01: trusted https://…:8443 from a file:// page, got the file://
          // page back). It only worked when the failed load was the card's first, where the
          // current document is still about:blank and the Reload() arm below is taken.
          std::string retry = (failed && !furl.empty()) ? furl : mPage->CurrentUri();
          fprintf(stderr, "[jihad-bs] ssl: %s override remembered for %s — reloading\n",
                  permanent ? "permanent" : "session", chost.c_str());
          // Reload through the normal load path so the retry reports load state as usual.
          if (!retry.empty() && retry != "about:blank") openUrl(retry.c_str());
          else mPage->Reload();
        } else {
          fprintf(stderr, "[jihad-bs] ssl: could not remember the override for %s\n", chost.c_str());
        }
      }
    }
    // Upstream unlinked the cert file once the dialog resolved, and so do we:
    // the card fetches its details at show time (setCertFile -> certFileChanged),
    // which is strictly inside the window we are blocked in, and the store has
    // already copied whatever it installed.
    jihad::certstore::ForgetCertPem(certFile);
    if (!certInfoFile.empty()) unlink(certInfoFile.c_str());
  }
  else if (failed) mSink.msgFailedLoad("Goanna", code, jihadBoundedUri(furl).c_str(), "Load failed");
  std::string uri = emitLocationAndTitleCore();
  return failed ? std::string() : uri;
}

// The location + title/url half, WITHOUT the failure/cert half above. Split out for the
// same-document path: GetLoadError/GetCertError report the state of the last real LOAD, so
// running them again for a fragment change would re-raise the previous load's failure dialog
// (or re-emit msgFailedLoad) on a page that never reloaded.
// A page title as it may go to the card: control characters become spaces and it is cut at
// 1024 bytes on a UTF-8 boundary. The YAP packet is a fixed 16 KiB; a page can make its title as
// long as it likes, and an oversized field was skipped while the fields after it were still
// written — a malformed message (Codex 2026-10-06). No browser shows a 1 KB title anyway.
static std::string jihadBoundedTitle(const std::string& in) {
  std::string t = in;
  for (char& c : t) if ((unsigned char)c < 0x20 || c == 0x7f) c = ' ';
  if (t.size() > 1024) {
    size_t n = 1024;
    while (n > 0 && ((unsigned char)t[n] & 0xC0) == 0x80) --n;   // not inside a UTF-8 sequence
    t.resize(n);
  }
  return t;
}

std::string BrowserPageGoanna::emitLocationAndTitleCore() {
  // For internal about: pages, report the typed about: URL, not the data: URL the engine loaded
  // (keeps the bar showing about:jihad and avoids a huge data: history entry). NB: never emit
  // msgUrlRedirected for an ordinary HTTP 3xx redirect — in isis the app binds onUrlRedirected ->
  // openResource -> the DEFAULT (stock) browser, so a normal redirect (google.com -> www.google.com)
  // would leave Jihad. A redirect is just a location change, reported here.
  std::string uri = mAliasUrl.empty() ? mPage->CurrentUri() : mAliasUrl;
  const std::string wireUri = jihadBoundedUri(uri);
  mSink.msgLocationChanged(wireUri.c_str(), mPage->CanGoBack(), mPage->CanGoForward());

  // The isis address bar updates on the title+url message (BasicWebView.titleURLChange ->
  // urlTitleChanged -> ActionBar.setUrl), NOT on msgLocationChanged. If the page has no <title>, fall
  // back to the URL so the bar still shows where we are (review #6 F-003).
  { std::string title = jihadBoundedTitle(mPage->GetTitle());
    if (title.empty()) title = jihadBoundedTitle(wireUri);
    mLastTitleSent = title; mLastTitleEmitMs = jihadNowMs();
    bool cb = mPage->CanGoBack(), cf = mPage->CanGoForward();
    fprintf(stderr, "[jihad-bs] titleAndUrl title=[%.300s] uri=%.300s back=%d fwd=%d\n", title.c_str(), wireUri.c_str(), (int)cb, (int)cf);
    mSink.msgTitleAndUrlChanged(title.c_str(), wireUri.c_str(), cb, cf); }
  return uri;
}

// Full completion boundary: progress, location/title, load-stopped, global history, geometry, repaint.
// emitProgress100 drives the bar to 100 (true for a normal completion and the watchdog's forced one;
// omit only if it is already at 100). Emits EXACTLY ONE load-stopped per load — the late-completion path
// deliberately does not call this (it only re-syncs the address bar), so pageLoadStopped/history run
// once (Codex F-361).
void BrowserPageGoanna::emitCompletion(bool emitProgress100) {
  if (emitProgress100) mSink.msgLoadProgress(100);
  std::string uri = emitLocationAndTitle();
  mSink.msgLoadStopped();   // records app history from the title/url just set (F-334)
  if (!uri.empty() && mAliasUrl.empty()) mSink.msgUpdateGlobalHistory(jihadBoundedUri(uri).c_str(), false);  // R6
  emitGeometry();      // R4: contents-size + meta-viewport once the page settled
  mNeedsPaint = true;  // paint the final frame once (dedup — Codex P2)
}

void BrowserPageGoanna::emitLoadAndLocation() {
  if (!mPage) return;
  // Same-document navigation: a fragment change (location.hash = ...), pushState or
  // replaceState. None of these start a load, so none of the branches below ever fire for
  // them and the card would keep reporting the pre-change url. This is the leg the settings
  // page needs: about:preferences publishes an edited payload by rewriting its own fragment,
  // and the shells adopt it off the committed url (ChromePrefs.adoptFromUrl and its Mochi/Mojo
  // twins). Emitted BEFORE the load branches so a same-document change that lands in the same
  // tick as a completion does not report the old url after the new one.
  if (mPage->TakeSameDocumentLocation()) {
    fprintf(stderr, "[jihad-bs] same-document location -> %s\n", mPage->CurrentUri().c_str());
    emitLocationAndTitleCore();
  }
  // Incremental load progress: feed the isis address-bar progress bar while the load is in flight so a
  // slow 512 MB-device page visibly advances instead of sitting at 0% — a frozen full-width bar reads
  // as a crashed "loading screen" (the user's #1 complaint). Only emit increases (isis ignores
  // decreases and clears its bar at 100, which the completion boundary sends).
  if (!mLoadWasDone) {
    int p = mPage->GetLoadProgress();
    // Progress also repaints (belt-and-braces under the 0012 dirty flag): bytes arrived, so the
    // incremental render likely changed — show the page building up instead of a stale frame.
    if (p > mLastProgress) { mLastProgress = p; mSink.msgLoadProgress(p); mNeedsPaint = true; }
  }
  bool stalled = (!mLoadWasDone && mLoadStartMs != 0 && (jihadNowMs() - mLoadStartMs) > 12000);
  if (mPage->LoadDone() && !mLoadWasDone) {
    // Normal completion — the one and only load-stopped boundary for this load.
    mLoadWasDone = true; mLoadStartMs = 0; mWatchdogDismissed = false;
    fprintf(stderr, "[jihad-bs] load done uri=%s\n", mPage->CurrentUri().c_str());
    emitCompletion(true);
    // New document settled: hang the engine focus/blur listener on it so script-driven
    // focus changes drive the VKB (Atlas IM-context port, device T4). No-op if same doc.
    mPage->RegisterEngineFocusListener();
  } else if (mWatchdogDismissed && mPage->LoadDone()) {
    // LATE completion: the engine finished a load the stall watchdog already gave a full completion
    // boundary. The lifecycle (load-started..load-stopped) is already balanced, so DON'T emit another
    // load-stopped/progress (that would double pageLoadStopped + unbalance the stream — Codex F-361).
    // Just re-sync the address bar to the FINAL url/title in case it changed since the watchdog fired
    // (e.g. a redirect that resolved late) and repaint the final frame.
    mWatchdogDismissed = false;
    fprintf(stderr, "[jihad-bs] late load done uri=%s\n", mPage->CurrentUri().c_str());
    emitLocationAndTitle();
    emitGeometry();
    mNeedsPaint = true;
    mPage->RegisterEngineFocusListener();   // the late-settled doc still needs the focus listener
  } else if (stalled) {
    // Stall watchdog: a load active too long without completing must NEVER pin the isis loading overlay
    // open — it covers the whole card and looks crashed. Force ONE full completion boundary (R3) so the
    // lifecycle is balanced even if the request never finishes (a permanently-stalled load isn't left
    // with load-started unmatched — Codex F-353). By 12s the doc has almost always COMMITTED (headers
    // received) so CurrentUri is the new page, not stale (F-360). This does NOT Stop() the engine: the
    // request keeps loading and, if it finishes, the late branch above re-syncs the final url + repaints
    // (F-288) — without a second load-stopped. ClearProgrammaticLoad lets a form/location.href/
    // meta-refresh nav the user triggers on the partial page still be detected and adopted/re-driven
    // instead of being misread as a command load and dropped (Codex F-333).
    fprintf(stderr, "[jihad-bs] load watchdog: forcing load-stopped after %ldms (engine still loading)\n",
            (long)(jihadNowMs() - mLoadStartMs));
    mLoadWasDone = true; mLoadStartMs = 0; mWatchdogDismissed = true;
    mPage->ClearProgrammaticLoad();
    emitCompletion(true);
    // The watchdog-completed page still needs the engine focus listener — by 12 s the document
    // has almost always committed, and this slow-load path is exactly the wedged-VKB scenario
    // the feature targets (inspector P2). Re-resolves when the load later finishes (late branch).
    mPage->RegisterEngineFocusListener();
  }
}

bool BrowserPageGoanna::emitGeometry() {
  if (!mPage) return false;
  int cw = 0, ch = 0;
  bool got = mPage->GetContentSize(&cw, &ch);
  // Every-call trace, off by default. The change-only line below is what a normal run wants
  // (an unconditional one filled the device's /var — see the note there), but "did it even
  // re-measure?" is exactly the question the scroll bug turned on, and a change-only log
  // cannot answer it. Set JIHAD_LOG_GEOMETRY=1 to see each call.
  { static const bool kTrace = getenv("JIHAD_LOG_GEOMETRY") != nullptr;
    if (kTrace) {
      fprintf(stderr, "[jihad-bs] emitGeometry(call) got=%d raw=%dx%d win=%dx%d last=%dx%d\n",
              (int)got, cw, ch, mPage->Width(), mPage->Height(), mLastContentW, mLastContentH);
    } }
  // NEVER emit a degenerate (0,0) content size. During a resize, GetContentSize (GetRootBounds)
  // momentarily returns (0,0) while the viewport reflow rebuilds the root frame — the Flush_Layout
  // can complete mid-reconstruction. The adapter treats contentsSizeChanged(0,0) as "new page":
  // it DROPS the live offscreen buffer (-> white card) and resets zoom-fit to 1.0 (-> the 1.0<->0.75
  // flicker). Guarding cw>0 && ch>0 kills the dominant path of BOTH the resize white-out and the
  // zoom oscillation (review #7 P1). Keeps the last good size until a real one arrives.
  // Never report content NARROWER than the window ("scrolling is janky", device 2026-08-02):
  // the adapter fit-zooms to mWindow.width/contentWidth, so a 764-wide body in a 768 window
  // produced mZoomLevel=1.0052 — and at inv=0.9948 EVERY blit is a nearest-neighbour
  // resample (row dup/skip shimmer, scroll-position rounding jitter, and the whole click
  // map wobbling by 0.5%). Fit-zoom exists to shrink WIDE desktop layouts; content that
  // already fits the layout viewport should composite at exactly 1.0 (identity blit). The
  // reported height is unchanged — only the width is floored at the window width.
  if (got && cw > 0 && cw < mPage->Width()) cw = mPage->Width();
  if (got && cw > 0 && ch > 0 && (cw != mLastContentW || ch != mLastContentH)) {
    // Log only on a REAL change. This used to log unconditionally, and emitGeometry is retried
    // from the pump on EVERY tick while mGeometryDirty is set — which made this line the single
    // biggest producer in the daemon log (1263 of the last 1400 lines on device) and the main
    // reason the three logs grew to 55 MB and filled the TouchPad's 62 MB /var, silently killing
    // all daemon logging (2026-08-06). A no-change retry carries no information; the change does.
    fprintf(stderr, "[jihad-bs] emitGeometry contentSize=%dx%d win=%dx%d mZoom=%.4f\n",
            cw, ch, mPage->Width(), mPage->Height(), mZoom);
    mLastContentW = cw; mLastContentH = ch;
    mSink.msgContentsSizeChanged(cw, ch);        // R4: contents-size-changed
  }
  // R4: meta-viewport — when the page declares one, and only when it CHANGES. This used to go
  // out on every call (the pump retries emitGeometry on every tick while geometry is dirty), and
  // a card that acts on the message — the Mojo WebView re-applies the initial scale each time —
  // was being told to reset its zoom several times a second.
  double is = 1.0, mn = 1.0, mx = 1.0; int vw = 0, vh = 0; bool us = true;
  if (mPage->GetViewport(&is, &mn, &mx, &vw, &vh, &us)) {
    // Again only when what the PAGE declares changes (width, limits, scalability) — not when the
    // computed initial scale or height moves with the window. In the Mojo shell the "window" is
    // the WebView's node, which the widget sizes to page x initial scale; a page that declares a
    // width (Wikipedia: 1120) gets an initial scale of window / width, so every re-send shrank the
    // node and the next scale with it — 768 -> 526 -> 360 -> ... -> 76 px, the page a stripe at the
    // left (device 2026-10-07, en.m.wikipedia.org; the owner's report "glitch out and jump around
    // and appear as a stripe at the left"). The stock server sends this once per page.
    if (!mViewportSent || mn != mVpMn || mx != mVpMx || vw != mVpW || us != mVpUs) {
      mViewportSent = true; mVpIs = is; mVpMn = mn; mVpMx = mx; mVpW = vw; mVpH = vh; mVpUs = us;
      fprintf(stderr, "[jihad-bs] metaViewport -> card: initial=%.4f min=%.4f max=%.4f %dx%d userScalable=%d\n",
              is, mn, mx, vw, vh, (int)us);
      mSink.msgMetaViewportSet(is, mn, mx, vw, vh, us);
    }
  } else {
    mViewportSent = false;   // a later page (or a tag added by script) reports afresh
  }
  return got && cw > 0 && ch > 0;   // a valid content size was available (deferred-emit can stop retrying)
}

void BrowserPageGoanna::pluginSpotlightStart(int x, int y, int w, int h) {
  mSpotlightPending = true;
  mSpotlightOn = true;
  // Document space in, absolute edges out — and note the two frozen messages disagree on
  // shape: the spotlight command carries origin + WIDTH/HEIGHT, while the interactive-rect
  // JSON carries absolute right/bottom. Normalising here keeps that asymmetry in one place.
  mSpotL = x; mSpotT = y; mSpotR = x + w; mSpotB = y + h;
}

void BrowserPageGoanna::pluginSpotlightEnd() {
  mSpotlightPending = true;
  mSpotlightOn = false;
  mSpotL = mSpotT = mSpotR = mSpotB = 0;
}

void BrowserPageGoanna::drainPluginSpotlight() {
  if (!mSpotlightPending || !mPage) return;
  mSpotlightPending = false;
  // The plugin thinks in the same viewport CSS px its own coordinates arrive in, so the rect
  // goes through docToViewport exactly like a pen event does.
  int l = mSpotL, t = mSpotT, r = mSpotR, b = mSpotB;
  if (mSpotlightOn) { docToViewport(&l, &t); docToViewport(&r, &b); }
  int n = mPage->SetPluginSpotlight(mSpotlightOn, l, t, r, b);
  // Logged even at zero. This command spent the whole port as a silent no-op; "no plugin was
  // listening" and "we never delivered it" have to stay distinguishable — and now so does
  // "we deliberately refused to deliver it", which is the default (see SetPluginSpotlight).
  if (n < 0) {
    fprintf(stderr, "[jihad-bs] pluginSpotlight %s %d,%d..%d,%d -> SUPPRESSED "
                    "(JIHAD_PLUGIN_SPOTLIGHT unset; delivering it kills Flash)\n",
            mSpotlightOn ? "start" : "end", l, t, r, b);
    return;
  }
  fprintf(stderr, "[jihad-bs] pluginSpotlight %s %d,%d..%d,%d -> %d instance(s)\n",
          mSpotlightOn ? "start" : "end", l, t, r, b, n);
}

void BrowserPageGoanna::clearPluginRects() {
  // The adapter has no expiry of its own: mFlashRects is only ever mutated by our two
  // messages, and nothing — navigation, load, resize, freeze — clears it. A rect we forget
  // to withdraw is a permanent dead zone in the card's gesture arbitration, on every page
  // the card visits afterwards. So teardown is not housekeeping, it is part of the contract.
  for (const auto& kv : mEmittedFlashRects) {
    char buf[64];
    snprintf(buf, sizeof buf, "{\"id\":%d,\"type\":1}", kv.first);
    mSink.msgRemoveFlashRects(buf);
    fprintf(stderr, "[jihad-bs] flashRect remove id=%d (teardown)\n", kv.first);
  }
  mEmittedFlashRects.clear();
  mPluginRectIds.clear();
}

void BrowserPageGoanna::syncPluginRects() {
  // Kill switch for bisecting: this is the only path that forces a layout flush on the tick.
  static const bool kOff = getenv("JIHAD_NO_PLUGIN_RECTS") != nullptr;
  if (kOff) return;
  if (!mPage) return;

  // Cost gate. This runs from the ~10 ms tick, so it is rate-limited. On a page with no
  // plugin — which is nearly every page — the cost stops at one querySelectorAll that matches
  // nothing. Nothing on this path flushes layout: CollectPluginRects deliberately reads the
  // bounds through getBoundsWithoutFlushing, for the reason documented there.
  // Rate limit, with an explicit "never run" flag rather than a 0 sentinel. jihadNowMs()
  // returns `long`, which is 32 BITS on this ARM target, so tv_sec*1000 overflows and the
  // value is a wrapped (often negative) counter. Differences between two wrapped stamps are
  // still correct, which is why every other user of this clock works — but comparing a
  // wrapped stamp against a literal 0 is not, and it silently made this function return on
  // EVERY tick. Measured: three "enter" probes, zero past this line.
  int64_t now = jihadNowMs();
  if (mRectSyncStarted && (now - mLastRectSyncMs) < 500) return;
  mRectSyncStarted = true;
  mLastRectSyncMs = now;

  std::vector<GoannaRenderPage::PluginRect> rects;
  if (!mPage->CollectPluginRects(&rects)) return;

  // COORDINATE SPACE — defined by the CONSUMER, not by inverting one of our own functions.
  //
  // getBoundingClientRect gives VIEWPORT CSS px. The adapter tests these rects with
  // flashRectContainsPoint(m_penDownDoc), and it builds that point as
  // `(mScrollPos + eventPt) / mZoomLevel` — unzoomed DOCUMENT CSS px. So the target space is
  // CSS document px: viewport CSS plus the scroll offset, with NO zoom factor applied.
  //
  // Written from the adapter's formula rather than as the inverse of docToViewport, on
  // purpose. docToViewport divides its input by the zoom a second time, and the rest of the
  // daemon disagrees with itself about what space that leaves (the popup path converts the
  // same value back with `x/Z + mPanX`, i.e. treating it as card px). That tangle is a real
  // pre-existing question and it is deliberately NOT resolved here — but a rect derived by
  // inverting the disputed function would inherit the dispute, whereas the adapter's own
  // formula is not in doubt. If docToViewport is later corrected, this needs no change.
  double offX = 0, offY = 0;
  if (mZoom < 0.99 || mZoom > 1.01) {
    // Zoomed: the engine scroll is parked and the visual pan carries the offset, in CSS px.
    mPage->GetRenderPan(&offX, &offY);
  } else {
    int ex = 0, ey = 0;
    if (mPage->GetScrollXY(&ex, &ey, /*flushLayout*/false)) { offX = ex; offY = ey; }
  }

  std::map<int, FlashRect> want;
  for (const auto& pr : rects) {
    int id;
    auto it = mPluginRectIds.find(pr.key);
    if (it != mPluginRectIds.end()) {
      id = it->second;
    } else {
      // PROCESS-GLOBAL counter, not per-page. The adapter files every rect into ONE map
      // keyed by id, and the card can hold two pages at once across a navigation — with a
      // per-page counter both pages mint id=1, the second add silently loses to the first
      // (the adapter's insert does not overwrite), and one page's remove withdraws the
      // other's rect. Observed: two adds for id=1 with different rects and no remove
      // between them. A global counter makes the collision unrepresentable.
      static int sNextFlashRectId = 1;
      id = sNextFlashRectId++;
      mPluginRectIds[pr.key] = id;
    }
    FlashRect fr;
    fr.l = (int)((pr.left + offX) + 0.5);
    fr.t = (int)((pr.top + offY) + 0.5);
    fr.r = (int)((pr.left + pr.width + offX) + 0.5);
    fr.b = (int)((pr.top + pr.height + offY) + 0.5);
    want[id] = fr;
  }

  // Withdraw first, then announce. The adapter's add path is a map::insert, which does NOT
  // overwrite an existing key — so a moved plugin whose add is re-sent without a remove
  // keeps its OLD rect forever, silently. That is the single most likely way this feature
  // half-works, which is why the change case sends both messages rather than just an add.
  for (auto it = mEmittedFlashRects.begin(); it != mEmittedFlashRects.end(); ) {
    auto w = want.find(it->first);
    bool moved = (w != want.end()) &&
                 (w->second.l != it->second.l || w->second.t != it->second.t ||
                  w->second.r != it->second.r || w->second.b != it->second.b);
    if (w == want.end() || moved) {
      char buf[64];
      snprintf(buf, sizeof buf, "{\"id\":%d,\"type\":1}", it->first);
      mSink.msgRemoveFlashRects(buf);
      fprintf(stderr, "[jihad-bs] flashRect remove id=%d (%s)\n", it->first,
              (w == want.end()) ? "gone" : "moved");
      it = mEmittedFlashRects.erase(it);
    } else {
      ++it;
    }
  }
  for (const auto& kv : want) {
    if (mEmittedFlashRects.count(kv.first)) continue;
    // One rect per message, in an array, and type 1 (InteractiveRectPlugin) — byte-identical
    // to what BrowserPage::addInteractiveWidgetRect emits. Note right/bottom are absolute
    // EDGES, not extents. Type 0 is isis's selection markers, which this port does not have.
    char buf[192];
    snprintf(buf, sizeof buf,
             "[{\"id\":%d,\"left\":%d,\"top\":%d,\"right\":%d,\"bottom\":%d,\"type\":1}]",
             kv.first, kv.second.l, kv.second.t, kv.second.r, kv.second.b);
    mSink.msgAddFlashRects(buf);
    mEmittedFlashRects[kv.first] = kv.second;
    fprintf(stderr, "[jihad-bs] flashRect add id=%d %d,%d..%d,%d\n", kv.first,
            kv.second.l, kv.second.t, kv.second.r, kv.second.b);
  }
}

void BrowserPageGoanna::emitScrollIfChanged() {
  if (!mPage) return;
  // When zoomed (>1) the authoritative visual scroll is the adapter's render-pan, not the engine
  // scroll (JihadRenderDocument ignores the engine scroll via RENDER_DOCUMENT_RELATIVE). Echoing
  // the engine scroll here would overwrite the adapter's horizontal pan and the vertical position
  // it is driving (Codex F2), so suppress the scrolled-to echo while zoomed.
  if (mZoom < 0.99 || mZoom > 1.01) return;
  // While the card is streaming positions (a drag, or a flick coasting: one every frame), nothing
  // is echoed: an echo resets the card's kinetic scroller, so a coast that ran past where the engine
  // stops (the page's end, or a page measured taller than it scrolls) would stop dead. Jihad's own
  // pages are drawn at zoom 1, the only zoom this echo runs at, and their flicks were reported to
  // stop dead (cavekit-chrome-look.md R3). The engine's position is not taken as seen meanwhile: once
  // the stream pauses (pump calls this every tick), a jump the page made in the window — a fragment,
  // a scrollIntoView, a focus scroll — is judged like any other (inspector 2026-10-12: taking it as
  // seen here lost it for good).
  if (jihadNowMs() - mLastScrollMs < 300) return;
  int sx = 0, sy = 0;
  if (mPage->GetScrollXY(&sx, &sy) &&
      (sx != mLastScrollX || sy != mLastScrollY)) {
    mLastScrollX = sx; mLastScrollY = sy;
    // Echo suppression ("scrolling is janky and skips around", device 2026-08-02): the
    // adapter's msgScrolledTo handler OVERWRITES its own pan position
    // (BrowserAdapter.cpp:4218 — mScrollPos + mScroller->scrollTo), and our engine scroll
    // CONVERGES on the adapter's setScrollPosition target 100-300 ms late (async
    // javascript: + pump cadence). Echoing that convergence yanked the view backwards to
    // where the finger was a beat ago, every beat. Only a scroll the PAGE initiated
    // (anchor jump, JS scrollTo, focus scroll) may drive the adapter — recognized as the
    // engine landing somewhere that is NOT near the adapter's own recent target. "Near"
    // is half a viewport: convergence chatter is always inside that; a real anchor jump
    // on any page worth jumping is not.
    double z = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
    long dy = (long)sy - (long)((double)mAdapterScrollY / z); if (dy < 0) dy = -dy;
    long dx = (long)sx - (long)((double)mAdapterScrollX / z); if (dx < 0) dx = -dx;
    bool adapterDriven = (jihadNowMs() - mLastScrollMs) < 1500 &&
                         dy < (long)(mPage->Height() / 2) && dx < (long)(mPage->Width() / 2);
    if (!adapterDriven) {
      fprintf(stderr, "[jihad-bs] scrolledTo -> card: %d,%d (content-initiated)\n", sx, sy);
      mSink.msgScrolledTo(sx, sy);   // R4: scrolled-to (content-initiated)
    }
  }
}

void BrowserPageGoanna::pump(int msBudget, bool drainOnly) {
  if (!mPage) return;
  // Fire the pending 'input' event for a keystroke edit FIRST — before the queued tap AND the Tab/Enter
  // queue below. It targets the element that was actually edited (mPendingInputEl). Flushing before the
  // tap matters when the user types then taps a submit/button or a link: a controlled/React handler must
  // observe the edited value before the click submits or navigates, and the event must dispatch on the
  // still-current document before openUrl swaps it out (Codex F-266/F-291). Runs here (guarded pump),
  // not in keyDown, since onChange/oninput run page JS.
  mPage->FlushPendingInputEvent();
  mPage->FlushPendingChangeEvent();
  { std::string nw; if (mPage->TakeNewWindowUrl(&nw)) mSink.msgNewPageRequested(jihadBoundedUri(nw).c_str()); }
  // A menu that opened or closed since the last pump changes the picture without damaging the
  // page: it is its own display root, composited over the frame at paint time, and XUL opens it
  // ASYNCHRONOUSLY — after the tap that asked for it has already been painted. Measured on the
  // device 2026-10-01: about:addons' tools menu was open (`popups=1`) and four seconds later
  // nothing had been painted (`0 composites`); it appeared only when the next event forced a
  // frame. A finger sends several events per tap, which is presumably why this went unseen.
  { bool open = mPage->PopupsOpen();
    if (open != mPopupWasOpen) { mPopupWasOpen = open; mNeedsPaint = true; mDamageForceFull = true; } }
  // Process a queued tap next — inside the tick's page-lifetime guard, and BEFORE
  // spending the pump budget so a link's load gets pumped this call (matters for
  // single-pump callers like link_test). ClickAt does the hit-test + activation +
  // mouse/DOMClick; for a link it records the href (TakeClickNav) instead of navigating.
  // Drain ALL queued input (F-9) in arrival order — raw mouse events and taps share one queue,
  // so a nested GLib loop entered by a page handler mid-drain can no longer invert a click ahead
  // of its own mouseup (see the PM_* comment in the header). Swap-and-clear, like
  // mPendingEditActions, so a handler that queues more input does not reenter this loop.
  if (!mPendingMouse.empty()) {
    std::vector<PendingMouse> evs;
    evs.swap(mPendingMouse);
    const unsigned gen = mNavGen;
    for (size_t i = 0; i < evs.size(); ++i) {
      // Stop on teardown OR on a navigation that happened during this drain: from here on the
      // remaining events belong to a document that is no longer current (F4). The queue itself
      // was already cleared by the navigation, but this batch is a local copy.
      if (!mPage || mNavGen != gen) break;
      PendingMouse e = evs[i];
      // Double activation: the same finger also produced this pen event on a separate YAP
      // command. Checked HERE, on the still-document coordinates, because the touch anchor it
      // compares against is in document space too (docToViewport below would move one and not
      // the other). See touchSuppressesMouse for the rule.
      if (touchSuppressesMouse(e.type, e.x, e.y)) {
        fprintf(stderr, "[jihad-bs] touch consumed gesture — dropping mouse type=%d at doc %d,%d\n",
                e.type, e.x, e.y);
        continue;
      }
      // The queue holds the adapter's DOCUMENT coords; the engine dispatch below
      // (SendMouseEvent / ElementFromPoint) expects VIEWPORT-relative CSS px. One mapping,
      // here, for every input kind — measured wrong on device 2026-08-02 (contextmenu at
      // doc y=1488 reached the page at client y=1488 while scrolled to 909; the target sat
      // at client y=579). This also explains the 2026-07-17 "link taps below the fold show
      // the overlay but never navigate": the tap resolved a screenful low.
      docToViewport(&e.x, &e.y);
      if (e.type == PM_CLICK) {
        fprintf(stderr, "[jihad-bs] clickAt %d,%d n=%d (doc %d,%d)\n", e.x, e.y, e.detail,
                evs[i].x, evs[i].y);
        mPage->ClickAt(e.x, e.y, e.detail);
        // VKB: the tap may have focused/blurred an editable field -> tell isis to raise/hide
        // the on-screen keyboard.
        bool efoc = false; int eft = 0, efa = 0;
        if (mPage->TakeEditorFocus(&efoc, &eft, &efa)) {
          fprintf(stderr, "[jihad-bs] editorFocused=%d fieldType=%d\n", (int)efoc, eft);
          mSink.msgEditorFocused(efoc, eft, efa);
        }
        std::string clickNav, clickRef; uint32_t clickPolicy = 0;
        if (mPage->TakeClickNav(&clickNav, &clickRef, &clickPolicy)) {
          // R6 (navigation-events): report the intercepted link activation, THEN navigate via
          // the load path. openUrl marks the load programmatic, so OnStateChange won't ALSO
          // flag it link-clicked — this msg is the single R6 notification for the tap.
          fprintf(stderr, "[jihad-bs] clickAt -> navigate %s\n", clickNav.c_str());
          // Logged because this MESSAGE, not the navigation it precedes, is what the
          // contract owes the card (navigation-events R6) — and it is otherwise invisible.
          fprintf(stderr, "[jihad-bs] linkClicked -> card: %s\n", clickNav.c_str());
          mSink.msgLinkClicked(jihadBoundedUri(clickNav).c_str());
          // The referrer the link's own rules give it (rel=noreferrer, its referrerpolicy, the
          // document's meta policy), as if the engine had followed it.
          openUrl(clickNav.c_str(), clickRef.c_str(), clickPolicy, /*fromPage*/true);   // bumps mNavGen -> the loop above stops next iteration
        }
        // A dropdown <select> tap queued a card-native popup instead of a click (Atlas model):
        // write the option data to a temp file and emit msgPopupMenuShow(id, file). The adapter
        // reads+unlinks the file and calls the card's showPopupMenu; the choice returns via
        // popupMenuSelect (asyncCmdPopupMenuSelect).
        emitSelectPopupIfPending();
        continue;
      }
      if (e.type == PM_TOUCHSTART || e.type == PM_TOUCHMOVE || e.type == PM_TOUCHEND) {
        const char* tt = e.type == PM_TOUCHSTART ? "touchstart"
                       : e.type == PM_TOUCHEND   ? "touchend" : "touchmove";
        bool consumed = false;
        if (e.pts.size() > 1) {
          std::vector<int> xs, ys;
          xs.reserve(e.pts.size()); ys.reserve(e.pts.size());
          for (auto& pt : e.pts) { xs.push_back(pt.first); ys.push_back(pt.second); }
          consumed = mPage->TouchEvent(tt, xs.data(), ys.data(), (int)xs.size());
        } else {
          consumed = mPage->TouchEvent(tt, e.x, e.y);
        }
        // Suppressor bookkeeping. The anchor is kept in DOCUMENT coords (evs[i], not the
        // viewport-mapped e) so it is comparable with the queued mouse events, which are
        // tested before their own mapping.
        const int64_t now = jihadNowMs();
        if (e.type == PM_TOUCHSTART) {
          mTouchSeqActive = true;
          mTouchSeqConsumed = consumed;
          mTouchSeqStartMs = now;
          mTouchAnchorX = evs[i].x; mTouchAnchorY = evs[i].y;
          if (consumed)
            fprintf(stderr, "[jihad-bs] touchstart consumed by page at doc %d,%d — suppressing "
                            "this gesture's mouse events\n", evs[i].x, evs[i].y);
        } else if (e.type == PM_TOUCHMOVE) {
          if (mTouchSeqActive) { mTouchAnchorX = evs[i].x; mTouchAnchorY = evs[i].y; }
        } else { // PM_TOUCHEND (also the adapter's TouchCancelled, type 3)
          if (mTouchSeqActive) { mTouchAnchorX = evs[i].x; mTouchAnchorY = evs[i].y; }
          // Rule 2: the pen-up and the synthesized click land just after this.
          mTouchSuppressUntilMs = (mTouchSeqActive && mTouchSeqConsumed) ? now + kTouchTailMs : 0;
          mTouchSeqActive = false;
          mTouchSeqConsumed = false;
        }
        continue;
      }
      // A finger moving over an OPEN menu highlights the row under it, exactly as a mouse
      // cursor would — and that highlight is also the selection indicator, so the user can
      // see what a lift would pick. The content document cannot hit-test a popup (separate
      // display root), so steer the move into the popup instead of dispatching it below.
      if (e.type == PM_MOVE && mPage->PopupHover(e.x, e.y)) {
        mNeedsPaint = true;
        continue;
      }
      const char* t = (e.type == PM_DOWN) ? "mousedown"
                    : (e.type == PM_UP)   ? "mouseup"
                    : (e.type == PM_MOVE) ? "mousemove" : "contextmenu";
      // The component listens for what the page does with its contextmenu event: a page
      // that claims it (preventDefault) has a menu of its own and gets no second one.
      if (e.type == PM_CONTEXTMENU) runPageCommand("holdArm");
      if (!mPage || mNavGen != gen) break;
      mPage->MouseEvent(t, e.x, e.y, e.detail);
      if (e.type == PM_CONTEXTMENU) {
        // The long-press, after the page has had its contextmenu event. The card raises the
        // menu; what it offers depends on what is under the finger, which only this side
        // knows — so say. (The card claims links and images itself and never sends holdAt
        // for those, so this is plain content or a field.)
        if (!mPage || mNavGen != gen) break;      // the page's handler navigated
        bool focused = false, readOnly = false;
        const bool editable = mPage->EditableAt(e.x, e.y, &focused, &readOnly);
        if (editable && !focused) {
          // As a right-click does on a desktop: the field takes the focus and the caret goes
          // where the press was, so Paste has a target and the keyboard comes up.
          mPage->ClickAt(e.x, e.y, 1);
          bool efoc = false; int eft = 0, efa = 0;
          if (mPage->TakeEditorFocus(&efoc, &eft, &efa)) mSink.msgEditorFocused(efoc, eft, efa);
          if (!mPage || mNavGen != gen) break;
        }
        char extra[128];
        snprintf(extra, sizeof extra, "\"x\":%d,\"y\":%d,\"editable\":%s,\"readOnly\":%s", e.x, e.y,
                 editable ? "true" : "false", readOnly ? "true" : "false");
        fprintf(stderr, "[jihad-bs] hold at %d,%d editable=%d wasFocused=%d\n", e.x, e.y,
                (int)editable, (int)focused);
        runPageCommand("hold", extra);
      }
    }
    mNeedsPaint = true;
  }
  // Process a queued editing key (Tab/Enter) in the SAME page-lifetime guard — it runs page JS
  // that may move focus or submit a form (navigate), which is unsafe in the keyDown YAP callback
  // (Codex F-219). A form submit navigates via the engine and is completed by the TakeLinkClicked
  // re-drive below, exactly like any content-initiated navigation.
  // The long-press menu's page commands (selectAll / copy / cut), in the same guard and for
  // the same reason: the component may run page script. A navigation in the middle ends the
  // batch; a copy the adapter is waiting on is answered either way.
  if (!mPendingPageCmds.empty()) {
    std::vector<PendingPageCmd> cmds;
    cmds.swap(mPendingPageCmds);
    for (const PendingPageCmd& c : cmds) {
      // Asked of one document, run on that document or not at all: every navigation — back,
      // forward and reload included — moves mNavGen, and a Copy or a Cut that was queued
      // before it must not land on the page that replaced it.
      bool ok = false;
      if (mPage && mNavGen == c.navGen) ok = runPageCommand(c.cmd.c_str());
      if (c.queryNum >= 0) mSink.msgCopySuccessResponse(c.queryNum, ok);
    }
  }
  if (!mPendingEditActions.empty()) {
    // Process every queued Tab/Enter in press order (a swap-and-clear so a handler that queues more
    // doesn't reenter this loop). Stop early once an editable is gone (a submit navigated away).
    std::vector<int> acts;
    acts.swap(mPendingEditActions);
    for (int act : acts) {
      if (!mPage->HasFocusedEditable()) break;   // a prior Enter submitted + navigated
      // A form-submit attempt ends the drain so a second queued Enter can't double-submit — regardless
      // of whether the nav commits synchronously (which also clears the edit target) or asynchronously
      // (cleared next tick by the adopt/re-drive's BeginLoad) — Codex F-323/F-325.
      if (act == PEA_ENTER)         { if (mPage->HandleEnter()) break; }
      else if (act == PEA_TAB)      mPage->HandleTab(false);
      else if (act == PEA_TAB_BACK) mPage->HandleTab(true);
    }
    mNeedsPaint = true;
    // A form submit navigates away / Tab moves fields — keep the VKB state in sync if it changed.
    bool efoc = false; int eft = 0, efa = 0;
    if (mPage->TakeEditorFocus(&efoc, &eft, &efa)) mSink.msgEditorFocused(efoc, eft, efa);
  }
  // R6 link-clicked: detect + re-drive/adopt a content-initiated navigation BEFORE PumpFor, so the
  // re-driven (GET) or adopted (POST) load runs to completion within THIS tick's PumpFor and is
  // reported by the emitLoadAndLocation below. Doing this AFTER PumpFor raced: a fast POST that fired
  // STATE_START+STATE_STOP inside PumpFor was already done, and adopting it then reset mDone=false
  // after its only STOP — no completion until the 12 s watchdog (Codex F-323).
  std::string linkUrl, linkRef; bool linkIsPost = false; uint32_t linkPolicy = 0;
  if (mPage->TakeLinkClicked(&linkUrl, &linkIsPost, &linkRef, &linkPolicy)) {
    fprintf(stderr, "[jihad-bs] linkClicked -> card: %s\n", linkUrl.c_str());
    mSink.msgLinkClicked(jihadBoundedUri(linkUrl).c_str());
    // Content-initiated navigation (JS `location.href`/`location.assign`, a form GET/POST,
    // meta-refresh, or a button onclick that sets location) STARTS but does NOT COMPLETE on its own
    // in this offscreen embedding (verified: it fires STATE_START for the target but never load-done).
    if (!linkIsPost) {
      // Re-drive a GET-class nav through the programmatic load path, which completes and supersedes
      // the stalled content load. openUrl marks the load programmatic so it won't re-trigger
      // TakeLinkClicked -> no loop.
      // With exactly the referrer the engine's own request carried (none if the page withheld
      // it): a plain openUrl sent none at all (device 2026-10-06), and a synthesised one would
      // ignore rel=noreferrer and the page's referrer policy (Codex 2026-10-06).
      fprintf(stderr, "[jihad-bs] content-nav re-drive GET -> %s (referrer %s)\n", linkUrl.c_str(),
              linkRef.empty() ? "none" : "kept");
      openUrl(linkUrl.c_str(), linkRef.c_str(), linkPolicy, /*fromPage*/true);
    } else {
      // A POST content-nav (form submit) is NOT re-driven: the original request has already reached
      // STATE_START — the body may already be on the wire — so re-issuing it would DOUBLE the POST
      // (double login/charge, Codex F-262). Leave the request to the ENGINE, but ADOPT it as the
      // tracked load so the daemon still: shows the overlay + progress, arms the stall watchdog, and
      // (crucially) emits the completion + new location and schedules a repaint when the POST response
      // arrives — otherwise the response silently stays on the old frame (Codex F-289).
      fprintf(stderr, "[jihad-bs] content-nav POST (engine-driven) %s\n", linkUrl.c_str());
      mPage->AdoptContentLoad();
      mWasSuppressed = false; mSuppressedSinceMs = 0;   // its own first-paint hold (see resetScrollForNavigation)
      // The card drops the old page's meta viewport at msgLoadStarted (below), so the result
      // page's must be sent afresh even when it declares the same one (inspector 2026-10-08).
      mViewportSent = false;
      mLoadWasDone = false;
      mNeedsPaint = false;
      mLastProgress = 0;
      mLoadStartMs = jihadNowMs();
      mSink.msgLoadStarted();
    }
  }
  if (drainOnly) mPage->PumpReady(msBudget); else mPage->PumpFor(msBudget);
  // Engine-driven repaint (UXP patch 0012): PumpFor just ran layout/JS/imagelib work; if any of it
  // invalidated content (incremental page render, SPA/JS DOM update, async image decode, animation),
  // repaint. This is the frame-delivery loop the stock QtWebKit server got from Qt paint events —
  // without it the shared buffer goes stale until user input forces a paint (device T3: "the old page
  // sticks around until you tap or drag"). RATE-LIMITED (inspector P2): a page with a persistent
  // animation (CSS spinner, video, SPA churn) otherwise re-dirties EVERY tick, forcing a full-document
  // software render + 3 MB buffer copy at ~30 Hz on the single-core ARMv7 — input latency + battery
  // drain. Dirty-driven paints are capped at one per 150 ms (~6 fps for pure animation; latency the
  // user can't perceive on a spinner); input/load-driven paints stay immediate via their own
  // mNeedsPaint sets. The pending flag is sticky so a dirty burst still paints when the window opens.
  // Pan-cadence refresh: while a pan is active, repaint every ~250 ms even when the pan is
  // fully covered — position:fixed content is baked at the band position of the LAST frame,
  // and the coverage skip alone would let it drift up to a whole overscan before snapping
  // ("the fixed banner flies around", device 2026-08-02). One ~66 ms paint per 250 ms keeps
  // the drift to a beat while preserving the coverage skip's win for fixed-free stretches.
  {
    int64_t pnow = jihadNowMs();
    if ((pnow - mLastScrollMs) < 400 && (pnow - mLastPaintDoneMs) >= 250) mNeedsPaint = true;
  }
  if (mPage->TakeDirty()) { mDirtyPending = true; gJihadDirtyTrue++; }
  {
    int dx = 0, dy = 0, dw = 0, dh = 0;
    if (mPage->TakeDirtyRect(&dx, &dy, &dw, &dh)) {
      // The engine reports damage in LAYOUT px; everything below — the margin, the per-buffer
      // boxes, the partial repaint — works in the buffer's px, which at any zoom but 1 are the
      // layout px scaled and shifted by the render pan (the engine scroll is parked at 0 then).
      // At zoom 1 the two are the same number, which is the only zoom the damage path had been
      // looked at with. Measured on the device 2026-10-01 at the 0.78 fit zoom of a page with
      // no viewport tag: a windowless plugin at layout 100,120 200x150 belongs at buffer
      // 78,94 157x118, was repainted through the UNSCALED box, and showed only the part of
      // itself inside it (x 101-234, y +120-+210 in the screenshot); the strip to its left and
      // above stayed whatever the last full frame had there. Any element away from the page
      // origin that changes by itself — a clock, a ticker, an animated image — is the same case.
      {
        const double zr = (mZoom >= 0.99 && mZoom <= 1.01) ? 1.0
                        : ((mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0);
        if (zr != 1.0) {
          double px = 0, py = 0; mPage->GetRenderPan(&px, &py);
          const double x0 = (dx - px) * zr, y0 = (dy - py) * zr;
          const double x1 = (dx + dw - px) * zr, y1 = (dy + dh - py) * zr;
          const int ix0 = (int)x0 - (x0 < 0 ? 1 : 0), iy0 = (int)y0 - (y0 < 0 ? 1 : 0);
          dx = ix0; dy = iy0;
          dw = (int)(x1 + 0.999) - ix0; dh = (int)(y1 + 0.999) - iy0;
        }
      }
      // GROW THE BOX BY A MARGIN before trusting it for a damage-only repaint (2026-10-01).
      // The engine's rect is the bounding box of what it invalidated, and that is not always
      // everything that changed on screen: decoration that overflows the frame being updated
      // can be left out. Device-measured on an <audio controls> element — the scrubber's time
      // bubble sits ABOVE the 28 px control bar, every damage rect the engine reported started
      // at the bar's top edge (y=53, never above), so each buffer kept the bubble exactly
      // where its last FULL paint had drawn it: "0:00" pinned at the far left while the thumb
      // moved on, which is the user's "glitchy 0:00 stuck at the left of the scrubber".
      // (Unioning the controls' overflow into nsVideoFrame's was tried and changed nothing.)
      // A damage-only repaint re-renders its rect from the document, so a superset is always
      // correct; the margin only costs pixels. Not applied while a plugin is on the page: there
      // the box is the plugin's own rect, exact by construction, and the 30 fps path is priced
      // per pixel.
      if (mEmittedFlashRects.empty()) {
        const double z = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
        const int m = (int)(kJihadDamageMarginCssPx * z + 0.5);
        dx -= m; dy -= m; dw += 2 * m; dh += 2 * m;   // clamped into the band by the partial path
      }
      // Union with anything not yet painted: several ticks of damage can accumulate behind
      // one paint, and repainting only the newest box would leave the older ones stale.
      if (mDamageValid) {
        int x0 = mDamageX < dx ? mDamageX : dx;
        int y0 = mDamageY < dy ? mDamageY : dy;
        int x1 = (mDamageX + mDamageW) > (dx + dw) ? (mDamageX + mDamageW) : (dx + dw);
        int y1 = (mDamageY + mDamageH) > (dy + dh) ? (mDamageY + mDamageH) : (dy + dh);
        mDamageX = x0; mDamageY = y0; mDamageW = x1 - x0; mDamageH = y1 - y0;
      } else {
        mDamageX = dx; mDamageY = dy; mDamageW = dw; mDamageH = dh;
        mDamageValid = true;
      }
      // AND into every buffer's own pending box — the global rect above is only the damage
      // since the LAST paint (any buffer); a buffer painted several frames ago needs the union
      // since ITS last paint or it repaints a stale frame minus one box (the scrubber-ghost bug).
      accumulateBufDamage(dx, dy, dw, dh);
    }
  }
  if (mDirtyPending) {
    int64_t dnow = jihadNowMs();
    // 33 ms, not 150. This cadence is what animated content actually runs at: every plugin
    // frame and every JS animation arrives as engine damage and is gated here, so 150 ms was a
    // hard 6.7 fps ceiling on the whole browser no matter how fast the source was. Measured
    // with a 30fps SWF: the plugin drew at 30fps and the parent received all of it, while the
    // card repainted 5 times in 25 seconds.
    // 33 ms is a FLOOR, not a promise — a paint that takes longer simply takes longer, and the
    // in-flight buffer guard above already refuses to start one while the adapter holds the
    // target. So this cannot spin: the cost of a frame throttles the next one.
    // A live plugin instance rides the paced grid (see jihadPluginFramePeriodMs); everything
    // else keeps the as-soon-as-possible floor. mEmittedFlashRects is the plugin-present
    // signal we already maintain for the adapter, so this costs one empty() test per tick.
    // Deferring here leaves mDirtyPending set, so the frame goes out on the first tick at or
    // after the deadline rather than being dropped.
    const int pacePeriod = mEmittedFlashRects.empty() ? 0 : jihadPluginFramePeriodMs();
    bool due;
    if (pacePeriod > 0) {
      // ONE CARD FRAME PER PLUGIN FRAME. The seq counter is the only signal in the pipeline
      // that distinguishes a new plugin frame from a repeat; the dirty flag cannot, because
      // the refresh driver and ordinary page damage set the same boolean. Gating on it makes
      // the publish rate EQUAL the plugin's delivery rate, which is what the stock host gets
      // for free by pulling the plugin inside its own page paint.
      //
      // Measured 2026-08-10, 30 fps SWF, and both alternatives are visibly wrong: publishing
      // on every dirty tick sent ~78 card frames for ~60 plugin frames (duplicates, and 22-44
      // deferred because the adapter could not drain that rate), while a fixed 33 ms grid sent
      // ~57 for ~68 and DROPPED frames, so the animation stepped two positions at once.
      const uint32_t seq = mPage->PluginFrameSeq();
      const bool haveSeq = (seq != 0);
      // The period is now a FLOOR, not a grid: it bounds how fast a pathological plugin can
      // drive the card, and it is what still paces things when the engine cannot report a seq.
      const bool floorOk = (dnow - mLastDirtyPaintMs) >= kJihadDirtyPaintMinMs;
      if (haveSeq) {
        // Damage on a plugin page is not necessarily the plugin's: a caret blink, an image
        // decode or chrome drawn over the embed all set the same flag, and gating those on a
        // plugin frame that may never come would freeze them for as long as the plugin is
        // idle. So a frame still goes out if damage has been waiting longer than the stale
        // bound, which is far above the plugin's own period and therefore never fires while
        // the plugin is animating.
        const bool stale = (dnow - mLastDirtyPaintMs) >= kJihadDirtyStaleMs;
        due = floorOk && ((seq != mLastPluginFrameSeq) || stale);
        if (due) mLastPluginFrameSeq = seq;
      } else {
        if (!mPaceDeadlineMs) mPaceDeadlineMs = dnow;
        due = dnow >= mPaceDeadlineMs;
        if (due) {
          mPaceDeadlineMs += pacePeriod;
          // Resync rather than chase: after a stall (a full repaint, a navigation, a frozen
          // card) the grid can be many periods in the past, and walking it forward one period
          // per frame would publish a burst at tick rate to "catch up" — the exact unevenness
          // this exists to remove.
          if (mPaceDeadlineMs <= dnow) mPaceDeadlineMs = dnow + pacePeriod;
        }
      }
    } else {
      mPaceDeadlineMs = 0;
      due = (dnow - mLastDirtyPaintMs) >= kJihadDirtyPaintMinMs;
    }
    if (due) {
      mNeedsPaint = true; mDirtyPending = false; mLastDirtyPaintMs = dnow;
      // A DOM change can change the SCROLLABLE SIZE, and until 2026-08-06 nothing re-measured
      // it: geometry was emitted only on load completion and on resize. So a page that grows
      // itself after load kept the adapter's original content size, and the adapter decides
      // whether there is anything to scroll from exactly that value — hence "I cant scroll
      // down in about:preferences", whose panes swap the whole content subtree with no load
      // and no resize (measured: Advanced overflows the viewport while the daemon still
      // reported contentSize == the window). Rides the dirty-paint cadence rather than the raw
      // tick because emitGeometry flushes layout; emitGeometry itself only emits on a real
      // change, so a page that dirties without resizing costs the flush and nothing more.
      // Re-measure the scrollable size — but NOT on every frame. This exists for pages that
      // grow themselves after load with no resize and no navigation (about:preferences swaps
      // its whole pane subtree), and emitGeometry FLUSHES LAYOUT. Setting it on every
      // damage-driven paint meant a full layout flush per animation frame, which on this
      // hardware costs more than the paint it was attached to: the animated-SWF pipeline sat
      // at 9 fps with 2 ms paints and no buffer contention, i.e. the flush WAS the frame time.
      // Twice a second catches a pane swap as promptly as anyone can notice.
      if (dnow - mLastGeomDirtyMs >= 500) { mGeometryDirty = true; mLastGeomDirtyMs = dnow; }
    }
  }
  // Engine-driven VKB sync (Atlas IM-context port, device T4): merge any focus/blur the engine
  // dispatched during PumpFor into the VKB state machine and emit the change. Script-driven focus
  // moves and blurs now raise/lower the keyboard without a tap; a wedged app-side state gets clean
  // false transitions to recover on.
  mPage->PollEngineFocus();
  { bool efoc = false; int eft = 0, efa = 0;
    if (mPage->TakeEditorFocus(&efoc, &eft, &efa)) {
      fprintf(stderr, "[jihad-bs] engine editorFocused=%d\n", (int)efoc);
      mSink.msgEditorFocused(efoc, eft, efa);
    } }
  // A lightweight theme ("persona") the page offers: the component asks the user and applies
  // it through the engine's LightweightThemeManager; the cards hear of the result on the
  // notification channel ("theme"). The JSON is content: escaped as one JSON string.
  if (!mThemePreviewDoc.empty()) {
    std::string now = mPage->CurrentUri();
    if (now.substr(0, now.find('#')) != mThemePreviewDoc) {
      mThemePreviewDoc.clear();
      fprintf(stderr, "[jihad-bs] theme: the previewing page is gone — preview reset\n");
      runPageCommand("theme", "\"kind\":\"reset\"");
    }
  }
  { std::string kind, tjson, origin, base;
    if (mPage->TakeThemeEvent(&kind, &tjson, &origin, &base)) {
      auto esc = [](const std::string& in) {
        std::string out; out.reserve(in.size() + 16);
        for (unsigned char c : in) {
          if (c == '"' || c == '\\') { out += '\\'; out += (char)c; }
          else if (c < 0x20) { char b[8]; snprintf(b, sizeof b, "\\u%04x", c); out += b; }
          else out += (char)c;
        }
        return out;
      };
      fprintf(stderr, "[jihad-bs] theme: page asks to %s a theme (%zu bytes)\n", kind.c_str(), tjson.size());
      // A preview belongs to the page that showed it: remembered here so that leaving that
      // page takes it off (below), as Firefox resets a preview when its page is hidden.
      mThemePreviewDoc = (kind == "preview") ? origin.substr(0, origin.find('#')) : std::string();
      runPageCommand("theme", "\"kind\":\"" + kind + "\",\"json\":\"" + esc(tjson) + "\",\"origin\":\"" + esc(origin) +
                     "\",\"base\":\"" + esc(base) + "\"");
    } }
  // A title the page changed after its load: the same message the load boundary sends, so the
  // card's title bar (and the tab strip to come) follows the page — not a location change,
  // nothing moved. History rows keep the title the page had when it finished loading. A page
  // can change its title on every frame (a ticker, a counter), so changes are coalesced: at
  // most one message per 500 ms, the latest title, nothing when it is the one last sent, and
  // a log line only when one is sent.
  if (mPage->TakeTitleChanged()) mTitleDirty = true;
  if (mTitleDirty && mLoadStartMs == 0 && jihadNowMs() - mLastTitleEmitMs >= 500) {   // during a load the boundary sends it
    mTitleDirty = false;
    std::string uri = jihadBoundedUri(mAliasUrl.empty() ? mPage->CurrentUri() : mAliasUrl);
    std::string title = jihadBoundedTitle(mPage->GetTitle());
    if (title.empty()) title = jihadBoundedTitle(uri);
    if (title != mLastTitleSent) {
      mLastTitleSent = title; mLastTitleEmitMs = jihadNowMs();
      fprintf(stderr, "[jihad-bs] title changed by the page: [%.120s]\n", title.c_str());
      mSink.msgTitleAndUrlChanged(title.c_str(), uri.c_str(), mPage->CanGoBack(), mPage->CanGoForward());
    }
  }
  // Emit deferred resize geometry now that PumpFor has let the reflow settle (review #7 P2).
  // emitGeometry itself guards against a still-degenerate 0x0 (P1), so a not-yet-settled
  // reflow just re-defers via the guard until a real size is available.
  // Retry until the reflow yields a valid size — but BOUNDED. emitGeometry() returns false on a
  // degenerate 0x0, which leaves the flag set, and it flushes layout on every call. That was
  // safe while only a RESIZE armed the flag (rare, and it settles); since a paint arms it too,
  // an unbounded retry would flush layout on every tick for as long as a page reports 0x0.
  // 20 ticks is far more than the handful a reflow needs, and the next paint re-arms it anyway.
  if (mGeometryDirty) {
    if (emitGeometry() || ++mGeometryTries >= 20) { mGeometryDirty = false; mGeometryTries = 0; }
  }
  emitLoadAndLocation();
  emitScrollIfChanged();
  syncPluginRects();
  drainPluginSpotlight();
  // Low-memory guardrail (512 MB Pre 3 floor): internally rate-limited /proc/meminfo poll; fires the
  // engine "memory-pressure" flush + malloc_trim when RAM runs short. Lives here (not the engine-
  // agnostic browserserver tick) so the daemon layer keeps zero engine includes.
  mHost.CheckMemoryPressure();
}

// Paint-pipeline counters, reported once every 2 s. Which of these three moves tells you where
// an animation is being lost: ticks that wanted a frame, frames refused because the adapter had
// not returned the buffer, and frames actually painted.
void BrowserPageGoanna::maybePaint() {
  if (mFrozen) return;                       // card backgrounded: don't paint
  // The engine's own first-paint gate (GoannaRenderPage::PaintingSuppressed). Our offscreen
  // render does not go through it, so a page could be drawn before its stylesheets applied —
  // and then nothing said it changed, so it stayed unstyled until a touch forced a paint
  // (device 2026-10-07). So: when the gate LIFTS, the whole page is repainted. And while it is
  // down, the last frame is held — but only for a second: in this embedding the engine lets
  // go only when the load ends (12 s on Wikipedia, at our load watchdog), and holding for
  // that long made every page slow to appear (owner 2026-10-08: "using the url bar to search
  // the web is slow"). After the second the page paints as it arrives, as before. Only once
  // a page has been shown (mHadContent): a first page has nothing behind it to hold.
  {
    const bool sup = mPage && mHadContent && mPage->PaintingSuppressed();
    const int64_t nowS = jihadNowMs();
    if (sup && !mWasSuppressed) mSuppressedSinceMs = nowS;
    if (mWasSuppressed && !sup) {
      fprintf(stderr, "[jihad-bs] first paint released — repainting the whole page\n");
      mNeedsPaint = true;
      mDamageValid = false; mDamageForceFull = true;
    }
    mWasSuppressed = sup;
    if (sup && nowS - mSuppressedSinceMs < 1000) return;   // mNeedsPaint stays set
  }
  gJihadTicks++;
  if (mNeedsPaint) {
    gJihadPaintWanted++;
    static int64_t sLast = 0;
    int64_t nowMs = jihadNowMs();
    if (!sLast) { sLast = nowMs; }
    else if (nowMs - sLast >= 2000) {
      fprintf(stderr, "[jihad-bs] paint pipeline: wanted=%u deferred=%u done=%u in %dms "
                      "(%.1f fps)\n",
              gJihadPaintWanted, gJihadPaintDeferInFlight, gJihadPaintDone,
              (int)(nowMs - sLast), gJihadPaintDone * 1000.0 / double(nowMs - sLast));
      fprintf(stderr, "[jihad-bs]   partial=%u (full: nodmg=%u force=%u slot=%u geom=%u small=%u) ticks=%u dirty=%u | frame gap ms "
                      "min=%u avg=%u max=%u\n",
              gJihadPaintPartial, gJihadPartialMiss[0], gJihadPartialMiss[1], gJihadPartialMiss[2],
              gJihadPartialMiss[3], gJihadPartialMiss[4], gJihadTicks, gJihadDirtyTrue,
              gJihadGapMin, gJihadGapN ? (uint32_t)(gJihadGapSum / gJihadGapN) : 0,
              gJihadGapMax);
      fprintf(stderr, "[jihad-bs]   gap hist ms %s=%u %s=%u %s=%u %s=%u %s=%u %s=%u\n",
              kGapLabels[0], gJihadGapHist[0], kGapLabels[1], gJihadGapHist[1],
              kGapLabels[2], gJihadGapHist[2], kGapLabels[3], gJihadGapHist[3],
              kGapLabels[4], gJihadGapHist[4], kGapLabels[5], gJihadGapHist[5]);
      gJihadTicks = gJihadDirtyTrue = 0;
      gJihadGapMin = gJihadGapMax = gJihadGapN = 0; gJihadGapSum = 0;
      for (int gi = 0; gi < kGapBuckets; ++gi) gJihadGapHist[gi] = 0;
      fflush(stderr);
      gJihadPaintPartial = 0;
      for (int mi = 0; mi < 5; ++mi) gJihadPartialMiss[mi] = 0;
      gJihadPaintWanted = gJihadPaintDeferInFlight = gJihadPaintDone = 0;
      sLast = nowMs;
    }
  }
  // The scroll-settle gate is GONE ("content doesn't stay consistently visible", device
  // 2026-08-02). It predates honest renderedX/Y: back then a mid-pan repaint mispositioned
  // the frame, so paints were held until the pan settled. Now a mid-pan frame is
  // positionally correct AND the coverage-aware skip means a paint request during a pan
  // exists precisely because the pan is about to run out of painted rows — holding it
  // until the fling ended guaranteed the white gap it was trying to avoid (every scroll
  // message reset the gate, so a continuous fling starved paints entirely).
  // MAINTENANCE FRAMES — asked for here, when nothing else wants a paint:
  //   grow   the region is smaller than it should be (it is built a step per frame);
  //   sweep  its off-screen rows may predate a change to the page; redraw the next few.
  // Growing waits only for the last frame to have been taken; sweeping waits for the page
  // to be loaded and still, and never runs during a pan — that is when the daemon's time
  // is worth most. A maintenance frame that could not run (the adapter still holds the
  // buffer) is simply asked for again on a later tick: mMaintenance still set means that.
  // A frame dump was asked for (<state>/dump-request, see the dump in paintToSharedBuffer):
  // paint one, so the dump is of the frame as it stands and not of whenever the page next
  // changes. Looked for twice a second, not every tick.
  if (!mNeedsPaint) {
    static int64_t sDumpLookMs = 0;
    const int64_t now = jihadNowMs();
    if (now - sDumpLookMs >= 500) {
      sDumpLookMs = now;
      static const std::string req = jihad::RuntimeStateDir().empty()
          ? std::string() : jihad::RuntimeStateDir() + "/dump-request";
      if (!req.empty() && access(req.c_str(), F_OK) == 0) mNeedsPaint = true;
    }
  }
  if (!mNeedsPaint && (mGrowOwed || mSweepLeft > 0)) {
    const int64_t now = jihadNowMs();
    int kind = 0;
    if (mGrowOwed && now - mLastPaintDoneMs >= 60) kind = 1;
    else if (mSweepLeft > 0 && mLoadWasDone && now >= mSweepNotBeforeMs &&
             now - mLastScrollMs >= 500 && now - mLastPaintDoneMs >= 250) kind = 2;
    if (kind) {
      mMaintenance = kind;
      mNeedsPaint = true;
      paintToSharedBuffer();
      if (mMaintenance) { mMaintenance = 0; mNeedsPaint = false; }
      return;
    }
  }
  if (mNeedsPaint) paintToSharedBuffer();    // only when there is a new frame
}

// ── PULL THE PLUGIN INSTEAD OF SAMPLING IT ───────────────────────────────────────────────
//
// Patch 0028 made delivery 1:1 (one card frame per plugin frame) but left TWO clocks running:
// the plugin child re-posts its own repaint task 16 ms after its work finishes, so its period
// is 16+work and drifts, while the daemon ticks on its own 16 ms timer. A frame that lands just
// after maybePaint() waits a whole tick, and because neither clock is locked to the other the
// wait varies frame to frame. Device-measured on the 30 fps SWF with 0028 alone: 1:1 delivery,
// deferred=0 and a healthy 29-32 fps average, but a gap spread of 9-62 ms around a 33 ms
// target — which is exactly the "smoother in the stock browser" report, and is invisible in
// the average.
//
// Stock has one clock: PluginViewPalm pulls the plugin inside the page paint. This is that,
// minus the blocking — the request is async because the synchronous shape out-of-process is
// intr NPP_HandleEvent, which would stall the daemon's tick and its YAP socket for an IPC
// round trip plus a Flash paint on every frame.
//
// ON THE TICK GRID, IN WHOLE TICKS — not on a wall-clock period, and not off the publish.
//
// A frame can only be PUBLISHED on a tick, so asking on any grid that is not a whole number of
// ticks guarantees a beat: with a 16 ms tick and a 33 ms request period the frame is ready
// either 2 or 3 ticks later depending on where in the tick the request fell, and the publish
// spacing alternates 32/48 ms. That is the same beat the 2026-08 note above records for a
// drifting floor, one layer up. Rounding the period to whole ticks removes it by construction:
// requests land ON ticks, the frame arrives a roughly constant time later, and the publish
// lands on a tick a fixed number of ticks after the request.
//
// NEVER STACK REQUESTS. If the previous request has not produced a frame yet, asking again
// would queue a draw the pipeline has no room for. Waiting for the seq to move instead is the
// backpressure the stock host gets from blocking the producer on the consumer — expressed here
// as "the consumer does not ask until the producer delivered". The child coalesces a stacked
// request anyway (mCurrentInvalidateTask), so this is belt and braces, but it also keeps the
// daemon from spending an IPC per tick on a plugin that is busy.
//
// The outstanding wait is BOUNDED. A plugin that dies mid-draw, or a frame lost to a torn-down
// actor, would otherwise mean we never ask again and the animation stops for good — silently,
// which is the failure mode PICKUP.md warns reads exactly like a pacing regression. After
// kJihadPluginReqTimeoutMs with no frame we ask again regardless.
static const int kJihadPluginReqTimeoutMs = 250;   // ~8 periods; cannot fire while animating

// The pull is a FALLBACK, not a clock (device 2026-10-06). Flash invalidates itself at its own
// content rate; asking for frames on top of that only adds draws of unchanged content that land
// between its own, which is exactly the beat the gap histogram showed (34 draws/s for a 30 fps
// SWF, spread from <16 to 55 ms; with the plugin alone, 28/s gathered at 32-39 ms). So the
// daemon asks only once the plugin has produced nothing for this long — a plugin that does
// not invalidate itself is still drawn (at about 1000/this per second), a still Flash movie
// costs a few draws a second, and a wedged plugin is re-driven exactly as before.
static int jihadPluginQuietMs() {
  static int s_ms = -1;
  if (s_ms < 0) {
    s_ms = 150;
    // A/B knob, like the ones above: 0 asks on every grid slot again (the pull as it was before
    // 2026-10-06), up to 1000.
    if (const char* e = getenv("JIHAD_PLUGIN_QUIET_MS")) {
      int v = atoi(e);
      if (v >= 0 && v <= 1000) s_ms = v;
      fprintf(stderr, "[jihad-bs] plugin pull after %d ms of plugin silence (JIHAD_PLUGIN_QUIET_MS)\n", s_ms);
    }
  }
  return s_ms;
}

// Disable the PULL ONLY, leaving the publish policy exactly as it is. $JIHAD_PLUGIN_PULL=0.
//
// This exists because the obvious way to A/B the pull — JIHAD_PLUGIN_FRAME_MS=0 — cannot answer
// the question. It stops the requests, but it ALSO drops the publish gate from "one card frame
// per plugin frame" to the bare 8 ms floor, so the run measures two changes at once and the
// histogram is unreadable. Worse, the child's lease then expires and its own timer takes over,
// which is patch 0026 behaviour: still two producers, just different ones.
//
// With this off and JIHAD_PLUGIN_FRAME_MS left at its normal value, the only producer left is the
// plugin invalidating ITSELF — measured at ~23/s against our ~26/s (see the R7 frame-rate
// criterion). That is the experiment that decides whether the host clock should exist at all for
// a self-invalidating plugin, or be deleted along with patch 0026's premise.
static bool jihadPluginPullEnabled() {
  static int s_on = -1;
  if (s_on < 0) {
    const char* e = getenv("JIHAD_PLUGIN_PULL");
    s_on = (e && *e && atoi(e) == 0) ? 0 : 1;   // default ON; only an explicit 0 disables
    if (!s_on) {
      fprintf(stderr, "[jihad-bs] plugin PULL disabled (JIHAD_PLUGIN_PULL=0) — the plugin's own "
                      "invalidations are the only frame source now\n");
      fflush(stderr);
    }
  }
  return s_on != 0;
}

bool BrowserPageGoanna::isBusy() const {
  if (!mPage) return false;
  // Engine media is playing somewhere in this process: its main-thread events (timeupdate,
  // buffering, the state machine's hops through the main thread) ride the pump.
  if (jihad_pulse_active_streams && jihad_pulse_active_streams() > 0) return true;
  // A backgrounded card paints nothing (maybePaint returns early), and mNeedsPaint can stay set
  // for as long as it is frozen — which must not hold the fast tick for a card nobody can see.
  if (mFrozen) return false;
  if (mNeedsPaint || mDirtyPending || mGeometryDirty || mBlankRetryAtMs) return true;
  // Maintenance frames (maybePaint). A sweep that is only waiting out its pause does not
  // need the fast tick; any tick after the pause finds it.
  if (mGrowOwed || (mSweepLeft > 0 && mLoadWasDone && jihadNowMs() >= mSweepNotBeforeMs)) return true;
  if (mLoadStartMs != 0 && !mLoadWasDone) return true;            // a load is running
  if (!mPendingMouse.empty() || !mPendingEditActions.empty() || !mPendingPageCmds.empty()) return true;
  if (!mEmittedFlashRects.empty()) return true;                   // plugin: the pull needs the grid
  const int64_t now = jihadNowMs();
  // Stay fast for a moment after the last paint or scroll: animation and panning arrive as a
  // stream of small events, and dropping to the quiet period between two of them would put
  // every other frame up to a quiet period late. 200 ms spans the gap between frames of
  // anything animating at 5 fps or more; a page that changes once a second is allowed to go
  // quiet in between.
  return (now - mLastPaintDoneMs) < 200 || (now - mLastScrollMs) < 200;
}

void BrowserPageGoanna::requestNextPluginFrame() {
  if (!mPage) return;
  if (!jihadPluginPullEnabled()) return;
  const int period = jihadPluginFramePeriodMs();
  if (period <= 0) return;            // JIHAD_PLUGIN_FRAME_MS=0 disables the pull with the pacer

  // DELIBERATELY NOT GATED ON mEmittedFlashRects. That map is the plugin-present signal used by
  // the publish policy, but it is refreshed only every 500 ms and — the reason this matters — it
  // is exactly what goes EMPTY when the plugin wedges: syncPluginRects logs "flashRect remove
  // id=N (gone)" and erases it. Gating here would stop asking at the one moment the plugin most
  // needs re-driving, turning a recoverable wedge into a permanent one. The engine-side call is
  // already the cheapest possible no-op with no plugin (a null weak symbol, or an empty list),
  // so paying it per grid slot on every page costs less than the gate would.
  const int tick = jihadPluginTickPeriodMs();       // the period the tick runs at with a plugin
  int ticksPerFrame = (period + tick / 2) / tick;   // whole ticks, nearest
  if (ticksPerFrame < 1) ticksPerFrame = 1;

  // Count EVERY tick, before any early return. Counting only the ticks that got as far as the
  // grid check would stall the counter for as long as a request is outstanding, and the request
  // period would then be (round trip + grid) instead of the grid — drifting with draw time,
  // which is the defect this replaces rather than a fix for it.
  ++mPluginReqTicks;

  const uint32_t seq = mPage->PluginFrameSeq();
  const int64_t now = jihadNowMs();
  if (seq != mLastRequestedSeq) {
    mPluginReqOutstanding = false;    // the frame we asked for arrived
    mLastRequestedSeq = seq;
    mPluginLastFrameMs = now;         // ...or the plugin drew on its own: it needs no asking
  }
  if (mPluginReqTicks < ticksPerFrame) return;
  if (now - mPluginLastFrameMs < jihadPluginQuietMs()) { mPluginReqTicks = 0; return; }

  if (mPluginReqOutstanding && (now - mPluginReqMs) < kJihadPluginReqTimeoutMs) {
    // The plugin still owes us a frame, so it is drawing slower than the grid. SKIP this slot
    // and stay on the grid (reset the counter) rather than firing the instant the frame lands:
    // sending off-grid is how an evenly-produced stream becomes unevenly delivered, and a plugin
    // that cannot hold 1/period should fall to a clean 1/2period, not to a drifting rate.
    mPluginReqTicks = 0;
    return;
  }
  mPluginReqTicks = 0;

  if (mPage->RequestPluginFrame() > 0) {
    mPluginReqOutstanding = true;
    mPluginReqMs = now;
  }
}

void BrowserPageGoanna::paintToSharedBuffer() {
  if (!mPage || !mActiveKey) return;
  // A frame just refused as blank-over-good is retried on a grid, not every tick (see the
  // blank-settle block below). mNeedsPaint stays set, so nothing is lost by waiting.
  if (mBlankRetryAtMs && jihadNowMs() < mBlankRetryAtMs) return;
  // Flow control (F-211): never overwrite a buffer the adapter still holds. If the target buffer
  // was msgPainted but not yet returned, defer this paint (mNeedsPaint stays set, so the next
  // pump tick / returnBuffer retries) — the typed text is already in the DOM, only the frame is
  // delayed. A timeout valve reclaims the buffer if a return was lost so painting can't stall.
  {
    int gs = slotForKey(mActiveKey);
    if (gs >= 0 && mInFlight[gs]) {
      // 2000 ms, was 250: with overscan the header GEOMETRY (renderedY) varies per frame,
      // so overwriting a buffer the adapter still holds no longer just tears pixels — a
      // blit straddling the write reads old geometry against new pixels and mispositions
      // the whole frame (review 2026-08-02 F7). A lost returnBuffer is rare; waiting 2 s
      // for it shrinks the torn-geometry window to near-nothing. The real fix is a frame
      // sequence number in the header + an adapter-side re-read guard — that needs an
      // adapter rebuild, queued for the next adapter change.
      if (jihadNowMs() - mPaintMs[gs] < 2000) { gJihadPaintDeferInFlight++; return; }  // adapter may still hold it
      mInFlight[gs] = false;                            // stale: assume the return was lost, reclaim
    }
  }
  int id = jihadShmResolve(mActiveKey);       // shmid (isis) or key->shmid (test adapter)
  if (id < 0) { perror("[BrowserPageGoanna] shm resolve"); return; }
  struct shmid_ds ds; size_t segSize = (size_t)mBufSize;
  if (shmctl(id, IPC_STAT, &ds) == 0 && ds.shm_segsz > 0) segSize = ds.shm_segsz;
  unsigned char* buf = attachShm(mActiveKey);   // cached attach — no per-paint shmat/shmdt (~400ms)
  if (!buf) return;

  // isis BrowserOffscreen shmem layout: [BrowserOffscreenInfo header][ARGB32 pixels].
  // The real BrowserAdapter reads header() at the base and rasterBuffer() at
  // base+sizeof(BrowserOffscreenInfo); it bails if renderedWidth<=0. So populate the
  // header and write pixels AFTER it (previously we wrote raw pixels at offset 0 with
  // no header -> the adapter read a garbage geometry and blitted nothing = white).
  int w = mPage->Width(), h = mPage->Height();
  const size_t hdr = sizeof(BrowserOffscreenInfo);
  if (w <= 0 || h <= 0 || segSize < hdr + (size_t)w * h * 4) { return; }
  // From here the paint runs. maybePaint reads mMaintenance == 0 as "it ran".
  const int maintenanceKind = mMaintenance; mMaintenance = 0;
  BrowserOffscreenInfo* oi = (BrowserOffscreenInfo*)buf;
  unsigned char* pixels = buf + hdr;

  // The EFFECTIVE scale the render runs at, so the adapter's blit (invScale =
  // contentZoom/mZoomLevel) AND its screen->content click mapping stay consistent with the
  // pixels. A near-1 fit-zoom (|mZoom-1|<=1%, the is-zoomed threshold) renders at 1x on the
  // engine-scroll path; reporting mZoom there desynced display from click coords (R5).
  double Zeff = (mZoom >= 0.99 && mZoom <= 1.01) ? 1.0
              : ((mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0);
  bool zoomed = (Zeff != 1.0);

  // --- Overscan geometry (scroll pan headroom, 2026-08-02). The adapter pans locally
  // inside the painted region between repaints; a viewport-exact paint has zero headroom,
  // so one pixel of pan exposed undrawn grey ("I scroll down into a grey area"). The shm
  // segment is 4x the screen (BrowserOffscreen::create), so paint viewport + overscan and
  // report the REAL painted geometry — the adapter's blit pans anywhere inside
  // renderedWidth/Height (baseY = srcTop - renderedY, FILL outside) and holds only on a
  // WIDTH mismatch, so a taller-than-window buffer is safe on both composite paths.
  long maxRows = (long)((segSize - hdr) / ((size_t)w * 4));
  // The whole segment is the budget. Until 2026-10-05 this was capped at 2048 rows, on the
  // worry (review 2026-08-02 F1) that the adapter's PGSurface wrap of the region might be a
  // GPU texture and the SGX540's limit is 2048. It is not a texture: JIHAD_REGION_ROWS_CAP
  // unset, a 4096-row portrait region wraps and blits on the device. The variable stays as
  // the way back (a number of rows) if some configuration ever disagrees.
  {
    static const long kRowsCap = getenv("JIHAD_REGION_ROWS_CAP") ? atol(getenv("JIHAD_REGION_ROWS_CAP")) : 0;
    if (kRowsCap >= (long)h && maxRows > kRowsCap) maxRows = kRowsCap;
  }

  // Where the viewport-relative band actually is, in ZOOMED px. At z~1 the engine scroll
  // is authoritative for Y (ScrollTo is an async javascript: — it can lag the adapter's
  // pan); when zoomed the band is the CLAMPED render pan. Stamping renderedY from the raw
  // adapter scroll while the frame was rendered elsewhere was itself a source of
  // mispositioned frames ("content moves without me touching it").
  // If the engine scroll is UNREADABLE at z~1, take the viewport-exact fallback instead of
  // stamping a geometry we cannot know (review F9: mixed coordinate spaces with an
  // honest-looking header). flushLayout=false — the renders below flush anyway (F4).
  // ── DAMAGE-ONLY REPAINT (the 30 fps path) ──────────────────────────────────────────────
  //
  // A whole-viewport software render costs ~40 ms on this hardware — measured p50 38 ms for
  // 1024x686 — which caps ANY animation at ~20 fps however fast its source runs. A plugin
  // animating inside a 320x240 box does not need the other 90% of the screen re-rasterised.
  //
  // The frozen adapter contract forbids simply reporting a smaller region: its blit FILLs
  // anything outside the painted rect, so a partial paint that ADMITTED to being partial
  // would blank the rest of the card. The way through is to repaint only the damaged rows
  // but keep reporting the geometry the buffer already holds — which is sound exactly when
  // this buffer already contains a full frame of that same geometry, and everything that has
  // changed since is inside the damage box. Both conditions are checked, and anything that
  // fails one takes the full path below.
  //
  // Buffers alternate, so "this buffer's last full frame" is two frames old. That is the
  // reason the eligibility test is per-buffer state rather than a single global flag.
  // ON by default; JIHAD_NO_PARTIAL_PAINT falls back to whole-frame repaints.
  //
  // The gate exists because this path is the one place that ships a frame the renderer did
  // not draw in full, so there has to be a way to take it out of the picture when diagnosing
  // anything that looks like a stale or torn frame. It is on by default because it is both
  // faster and STEADIER: a plugin frame costs ~2 ms here against 34-60 ms for a full repaint,
  // and it is that variance, not the mean, that shows up as uneven motion — measured, the
  // full-frame path delivered 12-25 fps swinging frame to frame while this one held 19 fps
  // flat.
  static const bool kPartialPaint = getenv("JIHAD_NO_PARTIAL_PAINT") == nullptr;
  // ── DID THE ENGINE SCROLL SINCE THE LAST FULL FRAME? ────────────────────────────────────
  // Asked HERE, not once per pump tick, and only when a damage-only repaint is actually on the
  // table — which is the only moment the answer can change a decision. Measured cost of getting
  // this wrong: probing every tick dropped the daemon tick from ~48/s to ~40/s, because at
  // fit-zoom (the normal state for a desktop-width page) emitScrollIfChanged used to return
  // before touching the engine at all.
  //
  // An engine scroll — anchor jump, window.scrollTo, scroll-into-view on focus, find-in-page,
  // or just the 100-300 ms convergence on the adapter's last target — changes NONE of the
  // things the eligibility test below checks: w, h, zoom and mAdapterScrollX/Y are all
  // identical across it. Meanwhile the damage box is relative to the NEW scroll and the buffer
  // holds the OLD one, so the partial branch would blit the plugin at the wrong rows forever.
  //
  // flushLayout=false is REQUIRED: the 2-arg default flushes, and a layout flush from the tick
  // reflows the plugin frame, reallocates its async surface under the child and kills it.
  // Its own last-seen pair, because mLastScrollX/Y belong to the adapter echo and are
  // deliberately not maintained while zoomed.
  if (kPartialPaint && mDamageValid && !mDamageForceFull && mPage) {
    int ex = 0, ey = 0;
    if (mPage->GetScrollXY(&ex, &ey, /*flushLayout*/false) &&
        (ex != mDamageScrollX || ey != mDamageScrollY)) {
      mDamageScrollX = ex; mDamageScrollY = ey;
      mDamageForceFull = true;
    }
  }
  if (kPartialPaint) {
    if (!mDamageValid) gJihadPartialMiss[0]++;
    else if (mDamageForceFull) gJihadPartialMiss[1]++;
    else {
      int ms = slotForKey(mActiveKey);
      if (ms < 0 || !mBufFull[ms].valid || !mBufFull[ms].dmgValid) gJihadPartialMiss[2]++;
      else if (mBufFull[ms].w != w || mBufFull[ms].h != h || mBufFull[ms].zeff != Zeff ||
               mBufFull[ms].stripGen != mStripGen || mBufFull[ms].scrollX != mAdapterScrollX ||
               mBufFull[ms].scrollY != mAdapterScrollY) gJihadPartialMiss[3]++;
    }
  }
  if (kPartialPaint && mDamageValid && !mDamageForceFull) {
    int slot = slotForKey(mActiveKey);
    if (slot >= 0 && mBufFull[slot].valid && mBufFull[slot].dmgValid &&
        mBufFull[slot].w == w && mBufFull[slot].h == h &&
        mBufFull[slot].zeff == Zeff &&
        mBufFull[slot].stripGen == mStripGen &&      // its off-screen rows are the newest there are
        mBufFull[slot].scrollX == mAdapterScrollX &&
        mBufFull[slot].scrollY == mAdapterScrollY) {
      const BufFrame& bf = mBufFull[slot];
      // The buffer's rows are pw px wide (the region's width: w, or more when zoomed in) and
      // the viewport starts bandCol px into each.
      const long pw = bf.paintedW > 0 ? (long)bf.paintedW : (long)w;
      // The damage box arrives in VIEWPORT device px; the buffer's rows are the painted
      // band, whose first visible row is bandRow. Clamp into the visible band and bail to
      // the full path on anything degenerate rather than guessing. Use THIS buffer's own
      // accumulated damage (bf.dmg*), not the global since-last-paint rect — see
      // accumulateBufDamage — so a buffer painted several frames ago repaints everything it
      // missed, not just the newest box.
      long dTop = (long)bf.bandRow + bf.dmgY;
      long dLeft = (long)bf.dmgX;
      long dW = bf.dmgW, dH = bf.dmgH;
      if (dLeft < 0) { dW += dLeft; dLeft = 0; }
      if (dTop < bf.bandRow) { dH -= (bf.bandRow - dTop); dTop = bf.bandRow; }
      if (dLeft + dW > w) dW = w - dLeft;
      if (dTop + dH > (long)bf.bandRow + h) dH = (long)bf.bandRow + h - dTop;
      // A damage box that covers essentially the whole viewport saves nothing over painting
      // the viewport...
      bool worthIt = dW > 0 && dH > 0 && (double)(dW * dH) < 0.6 * (double)(w * h);
      // ...but the region is usually far more than the viewport (the whole page, or every
      // row the segment holds), and re-rendering all of it because the SCREEN changed is
      // what made a page that animates, or one still loading, cost 100-250 ms a frame.
      // BAND-ONLY: redraw the visible rows, exactly as the full path draws them, and keep
      // the rest of this buffer's frame. Sound for the reason the small box is: the buffer
      // holds a complete frame of this geometry at this scroll. What it gives up is the
      // freshness of the rows nobody is looking at, which the idle sweep (mSweepLeft) pays
      // back once the page is still.
      const bool bandOnly = !worthIt && dW > 0 && dH > 0 && mThumbPath.empty() &&
                            bf.paintedH > h && bf.bandRow + (long)h <= (long)bf.paintedH &&
                            bf.bandCol + (long)w <= pw;
      if (!worthIt && !bandOnly) gJihadPartialMiss[4]++;
      if (worthIt || bandOnly) {
        int64_t pt0 = jihadNowMs();
        bool ok = false;
        if (worthIt) {
          ok = mPage->RenderRegion(pixels + ((size_t)dTop * pw + bf.bandCol + dLeft) * 4, (int)pw * 4,
                                   (int)dW, (int)dH,
                                   (double)(bf.paintedX + bf.bandCol + dLeft) / Zeff,
                                   (double)(bf.paintedTop + dTop) / Zeff, Zeff);
        } else {
          if (!zoomed) {
            // Viewport-relative, so position:fixed content and the caret are where the user
            // sees them. A blank read is a mid-reflow white: not published; the full path
            // below repaints the buffer and applies its own blank rules.
            ok = mPage->ReadPixels(pixels + (size_t)bf.bandRow * w * 4, (size_t)w * h * 4) > 0;
          } else {
            ok = mPage->RenderRegion(pixels + ((size_t)bf.bandRow * pw + bf.bandCol) * 4, (int)pw * 4,
                                     w, h, (double)(bf.paintedX + bf.bandCol) / Zeff,
                                     (double)(bf.paintedTop + bf.bandRow) / Zeff, Zeff);
          }
          if (ok) {
            mPage->CompositePopups(pixels, (int)pw * 4, (int)pw, bf.paintedH,
                                   (double)bf.paintedX / Zeff, (double)bf.paintedTop / Zeff, Zeff);
            dLeft = 0; dTop = bf.bandRow; dW = w; dH = h;
            // The off-screen rows now predate a change this big: sweep them when idle.
            mSweepLeft = bf.paintedH;
            // Enough changed to have changed the page's size too (see the full path).
            mGeometryDirty = true;
          }
        }
        if (ok) {
          // Re-stamp the SAME geometry the buffer was last fully painted with. The pixels
          // outside the damage box are that frame's and are still current, which is the
          // whole premise checked above.
          oi->bufferWidth = w; oi->bufferHeight = bf.paintedH;
          oi->contentZoom = Zeff;
          oi->renderedX = bf.paintedX; oi->renderedY = bf.paintedTop;
          oi->renderedWidth = (int)pw; oi->renderedHeight = bf.paintedH;
          mPaintedX = bf.paintedX; mPaintedW = (int)pw; mPaintedLo = bf.paintedTop;
          mPaintedHi = (long)bf.paintedTop + bf.paintedH; mPaintedZoom = Zeff;
          mLastPaintDoneMs = jihadNowMs();
          gJihadPaintDone++;
          gJihadPaintPartial++;
          JihadRecordFrameGap(mLastPaintDoneMs);
          mDamageValid = false; mDamageX = mDamageY = mDamageW = mDamageH = 0;
          // This buffer is now current through its accumulated damage; drop its pending box so
          // the next partial into it starts from the damage that arrives AFTER this frame.
          clearBufDamage(slot);
          {
            static int64_t sLastP = 0;
            int64_t nowP = jihadNowMs();
            if (nowP - sLastP >= 2000) {
              sLastP = nowP;
              fprintf(stderr, "[jihad-bs] %s paint %ldx%ld at %ld,%ld in %dms\n",
                      bandOnly ? "band-only" : "partial", dW, dH, dLeft, dTop, (int)(nowP - pt0));
              fflush(stderr);
            }
          }
          mLatestSlot = slot;
          mSink.msgPainted(mActiveKey);
          {
            int ps = slotForKey(mActiveKey);
            if (ps >= 0) { mInFlight[ps] = true; mPaintMs[ps] = jihadNowMs(); }
          }
          mActiveKey = nextBufferKey();
          mNeedsPaint = false;
          return;
        }
      }
    }
  }
  // Anything known to have changed on screen since the last frame? A maintenance paint
  // (maybePaint: growing the region, or sweeping its off-screen rows) is one that was asked
  // for when nothing had.
  const bool hadDamage = mDamageValid || mDamageForceFull;
  const int  maintKind = hadDamage ? 0 : maintenanceKind;
  // A change the engine could put a small box round (a caret, a spinner) is not a reason to
  // think the rest of the page moved; anything bigger, or of unknown extent, is.
  const bool smallDamage = mDamageValid && !mDamageForceFull &&
                           (double)mDamageW * (double)mDamageH < 0.6 * (double)w * (double)h;
  mDamageValid = false; mDamageForceFull = false;

  long bandX = mAdapterScrollX, bandY = mAdapterScrollY;
  bool bandKnown = true;
  if (zoomed) {
    double px = 0, py = 0; mPage->GetRenderPan(&px, &py);
    bandX = (long)(px * Zeff + 0.5); bandY = (long)(py * Zeff + 0.5);
  } else {
    int ex = 0, ey = 0;
    if (mPage->GetScrollXY(&ex, &ey, /*flushLayout*/false)) { bandY = ey; (void)ex; }
    else bandKnown = false;
  }
  // Horizontally the region is exactly one viewport wide (no x overscan), so keep the
  // pre-change identity: render AND stamp at the adapter's own x. Stamping the engine's
  // lagging x would turn every horizontal pan into a transient white column (review F6);
  // the transient CONTENT lag during a horizontal pan is the pre-existing behavior.
  // (When zoomed, bandX is the clamped pan — the render x — which is exact.)
  if (!zoomed) bandX = mAdapterScrollX;

  // Columns [loX, loX+regW). One viewport wide unless ZOOMED IN on content wider than the
  // screen — the one case where there is anything to the left or right. There the region
  // was one viewport wide too, so every sideways pan exposed an undrawn column and waited
  // for a repaint (owner, 2026-10-05: "if I scroll … left, or right, the page is grey").
  // Now it is the content's width, up to two viewports, centred on the view; the rows it can
  // have shrink in proportion, since the segment is what it is. The adapter is told the
  // viewport width in bufferWidth and the region's in renderedWidth (its rotation guard
  // compares the former; an adapter older than 2026-10-05 compares the latter and holds the
  // frame, so the two ship together).
  long regW = w, loX = bandX;
  if (zoomed) {
    long contentCols = (mLastContentW > 0) ? (long)((double)mLastContentW * Zeff) : 0;
    // The band was clamped to the content in css px and scaled; this is the same number
    // rounded the other way. At the content's right edge that came out one px short of the
    // band (1387 against a band ending at 1388, measured), and the band-inside-region test
    // below then threw the wide region away exactly where a sideways pan ends. The band is
    // inside the content by construction: let it say how wide the content is. The adapter's
    // own x may round the same way, by a pixel or two; past that it is a real overshoot, left
    // to the clamps below.
    if (contentCols > 0) {
      long edge = bandX + (long)w;
      const long adEdge = (long)mAdapterScrollX + (long)w;
      if (adEdge > edge && adEdge <= edge + 2) edge = adEdge;
      if (contentCols < edge) contentCols = edge;
    }
    if (contentCols > (long)w) {
      regW = contentCols < 2L * w ? contentCols : 2L * w;
      const long adX = (long)mAdapterScrollX;
      loX = bandX - (regW - (long)w) / 2;
      if (adX < loX) loX = adX;                                  // the adapter's own view…
      if (adX + (long)w > loX + regW) loX = adX + (long)w - regW;  // …stays inside
      if (loX + regW > contentCols) loX = contentCols - regW;
      if (loX < 0) loX = 0;
      if (bandX < loX || bandX + (long)w > loX + regW) { regW = w; loX = bandX; }
      const long rowsFor = (long)((segSize - hdr) / ((size_t)regW * 4));
      if (rowsFor < (long)h) { regW = w; loX = bandX; }          // no room: one viewport wide
      else if (rowsFor < maxRows) maxRows = rowsFor;
      // One line when the WIDTH decision changes (a zoom, or the content's width), with the
      // numbers behind it — the decision was invisible, and a wrong one looked like a slow
      // sweep. Not on every left edge: a sideways pan of content wider than two viewports
      // moves loX every frame, and a line per frame is how the log once filled /var.
      if (regW != mRegionColsLogged) {
        mRegionColsLogged = regW;
        fprintf(stderr, "[jihad-bs] region columns: content %ld px wide at zoom %.4f, band x=%ld, adapter x=%d -> [%ld, +%ld), rows up to %ld\n",
                contentCols, Zeff, bandX, (int)mAdapterScrollX, loX, regW, maxRows);
      }
    }
  }

  // Painted rows [lo, hi): cover the band AND the adapter's pan target, plus overscan
  // (half a viewport above, one below — scrolling down dominates), bounded by the segment
  // and the content height (mLastContentH is css px; unknown 0 = don't bound).
  long contentRows = (mLastContentH > 0) ? (long)((double)mLastContentH * Zeff) : 0;
  long adY = (long)mAdapterScrollY;
  // Bias the headroom toward the pan DIRECTION ("content blips in and out when I scroll",
  // device 2026-08-02): the strip behind the pan is dead weight; the strip ahead is what
  // the user is about to expose. Direction from the adapter scroll delta since last paint.
  //
  // HOW MUCH (owner, 2026-10-05: "pages only load when observed … the entire page should
  // stay rendered at once unless it's very long"). The headroom used to be a fraction of the
  // VIEWPORT (half above, one below), whatever the buffer could hold — so a page a screen
  // and a half long was painted as a window that followed the finger, repainted on the way,
  // and showed undrawn rows whenever the finger won. Now the whole row budget is spent:
  //   - a page that fits the budget is painted WHOLE, top to bottom. Nothing is ever
  //     uncovered and a scroll needs no repaint at all (setScrollPosition's coverage test
  //     finds every position covered) — the adapter just pans the frame it has;
  //   - a longer page gets the full budget around the viewport, most of it ahead of the pan.
  long slackRows = maxRows - (long)h; if (slackRows < 0) slackRows = 0;
  long ovAbove = slackRows / 3, ovBelow = slackRows - slackRows / 3;
  const int panDir = (adY > mPrevPaintScrollY + 8) ? 1 : (adY < mPrevPaintScrollY - 8) ? -1 : 0;
  if (panDir > 0)      { ovAbove = slackRows / 6; ovBelow = slackRows - ovAbove; }
  else if (panDir < 0) { ovBelow = slackRows / 6; ovAbove = slackRows - ovBelow; }
  mPrevPaintScrollY = adY;
  long lo = (bandY < adY ? bandY : adY) - ovAbove;
  long hi = (bandY > adY ? bandY : adY) + h + ovBelow;
  if (contentRows > 0 && contentRows <= maxRows) {
    lo = 0; hi = contentRows;                 // the whole page
  } else {
    // Headroom that falls off one end of the page is given to the other end.
    if (lo < 0) { hi -= lo; lo = 0; }
    if (contentRows > 0 && hi > contentRows) { lo -= (hi - contentRows); hi = contentRows; if (lo < 0) lo = 0; }
  }
  if (hi < lo + h) hi = lo + h;               // never less than one viewport
  if (hi - lo > maxRows) {                    // over budget: trim overscan…
    lo = bandY - (maxRows - (long)h) / 3;     // keep ~2/3 of the slack below the band
    if (lo < 0) lo = 0;
    hi = lo + maxRows;
    // …but the ADAPTER's viewport is what gets composited, so it must stay inside the
    // region (review F5: an engine parked at 0 by overflow:hidden while the adapter
    // flings to adY leaves the visible viewport past hi → a persistent white card).
    if (adY + h > hi) {
      hi = adY + h; lo = hi - maxRows; if (lo < 0) { lo = 0; hi = maxRows; }
    }
    // Re-apply the content bound after the re-anchors (review F10) — never below one
    // viewport though.
    if (contentRows > 0 && hi > contentRows) hi = contentRows;
    if (hi < lo + h) hi = lo + h;
  }
  // ── BUILD ON THE FRAME ALREADY PAINTED (2026-10-05) ─────────────────────────────────────
  //
  // [lo, hi) above is the region this page SHOULD have: all of it, or as many rows as the
  // segment holds. Rendering that whole region for every frame is what the paint used to
  // do, and it does not survive the region getting big: 4096 rows of a real page measured
  // 300-1400 ms on this hardware, with the daemon deaf to input for the duration, to redraw
  // rows that had not changed. So a frame is now COMPOSED:
  //
  //   - rows the most recent frame already holds (same document, zoom, width and columns)
  //     are COPIED from its buffer — a memcpy of a few MB, tens of ms at worst;
  //   - rows it does not hold are rendered, at most kGrowRows of them per frame, so the
  //     region reaches its full size over a few frames instead of in one long one
  //     (mGrowOwed asks maybePaint for the next step);
  //   - the visible band is drawn as it always was (viewport-relative at z~1);
  //   - at z~1 the band of the frame copied FROM carries viewport-relative content (a fixed
  //     header, the caret) that is wrong anywhere but under the viewport, so where that band
  //     is no longer this one's it is redrawn from the document.
  //
  // What a copy cannot know is whether the page changed somewhere nobody was looking — the
  // engine reports damage for the viewport only. So after any frame that reacted to a change
  // the off-screen rows are owed a SWEEP (mSweepLeft): redrawn kSweepRows at a time, by
  // maintenance frames, once the page has been still for a moment.
  const long kInitialRows = (long)h * 5 / 2;      // with nothing to build on: what a first frame is worth
  const long kGrowRows = 768;
  const long kSweepRows = mSweepStep;             // adapts to what a step costs on this page
  const int      xs = slotForKey(mActiveKey);
  const uint64_t docId = mPage->CurrentDocId();
  const BufFrame* src = nullptr; const unsigned char* srcPix = nullptr;
  if (bandKnown && docId != 0 && mLatestSlot >= 0 && mLatestSlot != xs) {
    const BufFrame& y = mBufFull[mLatestSlot];
    const int ykey = mLatestSlot == 0 ? mKey1 : (mLatestSlot == 1 ? mKey2 : mKey3);
    if (ykey && y.valid && !y.overlay && y.docId == docId && y.w == w && y.h == h &&
        y.zeff == Zeff && y.paintedW == (int)regW && y.paintedX == (int)loX && y.paintedH > 0) {
      if (unsigned char* yb = attachShm(ykey)) { src = &y; srcPix = yb + hdr; }
    }
  }
  bool growOwed = false;
  {
    const long mustLo = (bandY < adY ? bandY : adY);            // the band and the adapter's
    const long mustHi = (bandY > adY ? bandY : adY) + h;        // own viewport: never optional
    long cLo = lo, cHi = hi;
    if (src) {
      const long sLo = src->paintedTop, sHi = sLo + src->paintedH;
      const long needAbove = sLo > mustLo ? sLo - mustLo : 0;
      const long needBelow = mustHi > sHi ? mustHi - sHi : 0;
      // The viewport has left what that frame holds by more than a couple of steps (a jump
      // to an anchor, a long fling): bridging the gap would mean rendering all of it.
      if (needAbove + needBelow > 2 * kGrowRows || sHi <= lo || sLo >= hi) src = nullptr;
    }
    if (src) {
      const long sLo = src->paintedTop, sHi = sLo + src->paintedH;
      long newAbove = sLo > lo ? sLo - lo : 0, newBelow = hi > sHi ? hi - sHi : 0;
      long allowAbove, allowBelow;
      if (panDir < 0) {
        allowAbove = newAbove < kGrowRows ? newAbove : kGrowRows;
        allowBelow = newBelow < kGrowRows - allowAbove ? newBelow : kGrowRows - allowAbove;
      } else {
        allowBelow = newBelow < kGrowRows ? newBelow : kGrowRows;
        allowAbove = newAbove < kGrowRows - allowBelow ? newAbove : kGrowRows - allowBelow;
      }
      if (sLo - allowAbove > cLo) cLo = sLo - allowAbove;
      if (sHi + allowBelow < cHi) cHi = sHi + allowBelow;
    } else if (hi - lo > kInitialRows) {
      cLo = mustLo - (kInitialRows - (mustHi - mustLo)) / 3;
      if (cLo < lo) cLo = lo;
      cHi = cLo + kInitialRows;
      if (cHi > hi) { cHi = hi; cLo = cHi - kInitialRows; if (cLo < lo) cLo = lo; }
    }
    if (mustLo < cLo) cLo = mustLo < lo ? lo : mustLo;
    if (mustHi > cHi) cHi = mustHi > hi ? hi : mustHi;
    if (cHi < cLo + h) { cHi = cLo + h; if (cHi > hi) { cHi = hi; cLo = cHi - h; } }
    growOwed = (cLo > lo || cHi < hi);
    lo = cLo; hi = cHi;
  }
  int tallH = (int)(hi - lo);
  if (!bandKnown) tallH = h;                  // F9: unknown band → viewport-exact fallback

  int64_t paintT0 = jihadNowMs();
  // NO fling-mode fast path (tried 2026-08-02, removed the same day): skipping the band
  // overlay during flings painted position:fixed content at its DOCUMENT position in
  // those frames while full frames painted it at the viewport — two models alternating,
  // so a fixed banner "flew around" mid-scroll (user report). Fixed content bakes at the
  // band position each frame, swims with the pan between paints, snaps per paint — the
  // classic webOS-era behavior.
  long nb = -1;
  int paintedX = (int)bandX, paintedTop = (int)bandY, paintedH = h;
  int paintedW = w;                           // the buffer's row stride in px
  long bandCol = 0;                           // where the viewport starts in each row
  bool region = false;
  bool fresh = false;                         // every row of the region was rendered now
  bool stripsChanged = false;                 // rows outside the band differ from the source's
  long rowsRendered = 0, rowsCopied = 0;
  long bandRow = bandY - lo;
  if (bandRow < 0) bandRow = 0;
  if (bandRow > (long)tallH - h) bandRow = (long)tallH - h;
  if (tallH > h || regW > (long)w) {
    const size_t stride = (size_t)regW * 4;
    const double dX = (double)loX / Zeff;     // css px; loX == bandX unless zoomed and wide
    const long bandLo = bandRow, bandHi = bandRow + h;
    // Region rows [r0, r1), straight from the document (docY css px = zoomed px / Zeff).
    auto rows = [&](long r0, long r1) -> bool {
      if (r0 < 0) r0 = 0;
      if (r1 > (long)tallH) r1 = tallH;
      if (r1 <= r0) return true;
      rowsRendered += r1 - r0;
      return mPage->RenderRegion(pixels + (size_t)r0 * stride, (int)stride, (int)regW,
                                 (int)(r1 - r0), dX, (double)(lo + r0) / Zeff, Zeff);
    };
    // The same, less the visible band: at z~1 those rows come from the viewport-relative
    // overlay below, and rendering them here too is pure duplicate work.
    auto strips = [&](long r0, long r1) -> bool {
      if (zoomed) return rows(r0, r1);
      return rows(r0, r1 < bandLo ? r1 : bandLo) && rows(r0 > bandHi ? r0 : bandHi, r1);
    };
    bool ok = true;
    long a = 0, b = 0;                        // document rows [a, b) the source can give
    if (src) {
      const long sLo = src->paintedTop, sHi = sLo + src->paintedH;
      a = lo > sLo ? lo : sLo; b = hi < sHi ? hi : sHi;
      if (b <= a) src = nullptr;
    }
    if (!src) {
      fresh = true;
      ok = strips(0, tallH);
    } else {
      const long sLo = src->paintedTop;
      memcpy(pixels + (size_t)(a - lo) * stride, srcPix + (size_t)(a - sLo) * stride,
             (size_t)(b - a) * stride);
      rowsCopied = b - a;
      const long before = rowsRendered;
      ok = strips(0, a - lo) && strips(b - lo, tallH);          // rows it did not have
      if (ok && !zoomed) {
        // Its band, where that is not this frame's band (see the header comment).
        const long ob0 = (sLo + src->bandRow) - lo, ob1 = ob0 + h;
        if (ob0 != bandLo) {
          long c0 = ob0 > a - lo ? ob0 : a - lo, c1 = ob1 < b - lo ? ob1 : b - lo;
          if (c1 > c0) ok = strips(c0, c1);
        }
      }
      if (ok && maintKind == 2) {
        // One step of the sweep.
        long s0 = mSweepRow; if (s0 >= (long)tallH || s0 < 0) s0 = 0;
        long s1 = s0 + kSweepRows; if (s1 > (long)tallH) s1 = tallH;
        ok = strips(s0, s1);
        mSweepRow = (s1 >= (long)tallH) ? 0 : s1;
        mSweepLeft -= (s1 - s0);
        if (mSweepLeft < 0) mSweepLeft = 0;
      }
      stripsChanged = (rowsRendered != before);
    }
    region = ok;
    if (region) {
      paintedTop = (int)lo; paintedH = tallH;
      paintedX = (int)loX; paintedW = (int)regW; bandCol = bandX - loX;
    }
  }
  // The band a maintenance frame copied is still the screen: nothing has changed on it.
  const bool bandSame = region && src && !fresh && maintKind != 0 &&
                        (long)src->paintedTop + src->bandRow == lo + bandRow &&
                        (long)src->paintedX + src->bandCol == loX + bandCol &&
                        src->paintedTop <= lo + bandRow &&
                        (long)src->paintedTop + src->paintedH >= lo + bandRow + h;
  if (region && zoomed && !fresh && !bandSame) {
    // Zoomed: the copied rows under the viewport may predate what is on screen now.
    rowsRendered += h;
    if (!mPage->RenderRegion(pixels + ((size_t)bandRow * paintedW + bandCol) * 4, paintedW * 4,
                             w, h, (double)bandX / Zeff, (double)(lo + bandRow) / Zeff, Zeff)) {
      region = false; paintedX = (int)bandX; paintedW = w; bandCol = 0;
    }
  }
  if (region && !zoomed) {
    // Overlay the viewport-relative band over the visible rows so position:fixed/sticky
    // and the caret stay correct on screen; the doc-relative strips around it only show
    // transiently mid-pan. ReadPixels renders v2 (viewport-relative at z~1) + readback,
    // forces alpha and returns the nonblank count — same "did it render" signal as before.
    if (bandSame) nb = 1;
    else nb = mPage->ReadPixels(pixels + (size_t)bandRow * w * 4, (size_t)w * h * 4);
  } else if (region) {
    // Zoomed: the region render IS the frame (pure visual viewport; fixed content already
    // doc-positioned when zoomed). Count nonblank over the visible band only.
    nb = 0;
    for (long y = bandRow; y < bandRow + h && !bandSame; ++y) {
      unsigned char* row = pixels + ((size_t)y * paintedW + bandCol) * 4;
      for (int x = 0; x < w; ++x) {
        unsigned char* p = row + (size_t)x * 4;
        if (!(p[0] > 240 && p[1] > 240 && p[2] > 240)) ++nb;
      }
    }
    if (bandSame) nb = 1;
  } else {
    // Region render unavailable (old libxul: weak symbol absent / render failed / no room
    // for overscan): the previous viewport-exact paint, stamped at the band's true position.
    paintedTop = (int)bandY; paintedH = h;
    fresh = true; growOwed = false;
    nb = mPage->ReadPixels(pixels, segSize - hdr);
  }

  // A XUL popup (about:addons tools menu, a context menu) is a SEPARATE display root,
  // so nothing above can have drawn it — composite any open one over the frame we just
  // painted, at the same origin/zoom. Costs one popup-manager query when none is open,
  // which is the normal case. Must come AFTER the band overlay: that overlay rewrites
  // the visible rows and would erase a popup drawn before it.
  int popupsDrawn = 0;
  if (nb >= 0) {
    int popups = mPage->CompositePopups(pixels, paintedW * 4, paintedW, paintedH,
                                        (double)paintedX / Zeff, (double)paintedTop / Zeff,
                                        Zeff);
    popupsDrawn = popups;
    if (popups > 0) {
      fprintf(stderr, "[jihad-bs] composited %d popup(s) over the frame\n", popups);
      // A popup is content: an otherwise-blank frame carrying one must not be
      // suppressed as "blank over good" below.
      if (nb == 0) nb = 1;
    }
  }

  oi->bufferWidth = w; oi->bufferHeight = paintedH;     // bufferWidth: the VIEWPORT's width
  oi->contentZoom = Zeff;
  oi->renderedX = paintedX; oi->renderedY = paintedTop;
  oi->renderedWidth = paintedW; oi->renderedHeight = paintedH;
  // Record the painted region for the coverage-aware repaint skip (setScrollPosition).
  mPaintedX = paintedX; mPaintedW = paintedW;
  mPaintedLo = paintedTop; mPaintedHi = (long)paintedTop + paintedH;
  mPaintedZoom = Zeff;
  // NB: what this buffer now holds is recorded LOWER DOWN, past the blank-over-good
  // suppression — see the comment there. Recording it here marked a frame that is about to
  // be REJECTED as blank as "holds a complete frame", and the next tick's damage-only
  // repaint then blitted one live rectangle into that white buffer and published it,
  // laundering the exact white flash the suppression exists to prevent.
  mLastPaintDoneMs = jihadNowMs();   // pan-cadence refresh reference
  gJihadPaintDone++;
  JihadRecordFrameGap(mLastPaintDoneMs);
  if (nb >= 0) {
    fprintf(stderr, "[jihad-bs] painted shmid=0x%x bytes=%ld (%dx%d top=%d band=%ld left=%d) mZoom=%.4f contentZoom=%.4f ms=%ld"
                    " rows new=%ld copied=%ld%s\n",
            (unsigned)mActiveKey, nb, paintedW, paintedH, paintedTop, bandY, paintedX, mZoom,
            oi->contentZoom, (long)(jihadNowMs() - paintT0), rowsRendered, rowsCopied,
            maintKind == 1 ? " (grow)" : maintKind == 2 ? " (sweep)" : "");
    // Debug: dump each NON-EMPTY painted frame to a PPM so we can see exactly what
    // the engine rendered (text vs blank) independent of the adapter's blit. The
    // first paint after connect is empty (nb==0, blank buffer); guarding on nb>0
    // (not a once-only flag) makes frame.ppm hold the latest real content frame &mdash;
    // including repaints after in-page JS runs (e.g. navigator.userAgent). Env-gated.
    // $JIHAD_DUMP: unset = off (the default; a dump costs ~2.2 MB of fputc per
    // paint). "1" writes <state>/frame.ppm — /var/palm/jihad/$V/frame.ppm on the
    // device, NEVER the old /media/internal/jihad/frame.ppm (T-057, R8: the user's
    // vfat volume is not ours to write). Resolved once: getenv + the state-dir
    // derivation must not run on every paint.
    // A thumbnail someone asked for (requestThumbnail): the part of this frame the card is
    // showing — the viewport band, not the off-screen strips — scaled down and written as a
    // PNG. Taken from a real, non-empty frame, so it is what the user was looking at.
    if (!mThumbPath.empty() && nb > 0) {
      const long tr = region ? bandRow : 0;
      long rows = (long)paintedH - tr; if (rows > h) rows = h;
      const std::string out = mThumbPath; mThumbPath.clear();
      bool ok = false;
      if (rows > 0 && paintedW == w) {
        ok = writeThumbnail(pixels + (size_t)tr * w * 4, w, (int)rows, out);
      } else if (rows > 0) {
        // A region wider than the viewport: the thumbnail writer reads w-wide rows, so give
        // it the visible columns on their own.
        std::vector<unsigned char> band((size_t)w * rows * 4);
        for (long y = 0; y < rows; ++y)
          memcpy(&band[(size_t)y * w * 4], pixels + ((size_t)(tr + y) * paintedW + bandCol) * 4, (size_t)w * 4);
        ok = writeThumbnail(band.data(), w, (int)rows, out);
      }
      fprintf(stderr, "[jihad-bs] thumbnail %s -> %s\n", ok ? "written" : "FAILED", out.c_str());
    }
    static const std::string dumpPath = jihad::RuntimeResolvePath(getenv("JIHAD_DUMP"), "frame.ppm");
    // ...or ONE frame, on request: create <state>/dump-request and the next non-empty frame
    // is written to /tmp (RAM — a 4096-row frame is 9 MB and /var is a 62 MB partition) and
    // the request removed. This is how a composed frame's OFF-SCREEN rows are looked at: no
    // screenshot shows them, and the environment variable means editing a generated job on
    // the root filesystem. The request lives in the state directory, which only root writes.
    static const std::string dumpReq = jihad::RuntimeStateDir().empty()
        ? std::string() : jihad::RuntimeStateDir() + "/dump-request";
    bool dumpOnce = nb > 0 && !dumpReq.empty() && access(dumpReq.c_str(), F_OK) == 0;
    if (dumpOnce) {
      // /tmp is a 40 MB RAM disk the whole system uses (it was filled once by dumps left
      // lying there, and nothing on the device could write a temporary file until they
      // were removed). Replace the previous dump, and refuse unless what is free afterwards
      // is twice what this one needs.
      unlink(dumpReq.c_str());
      unlink("/tmp/jihad-frame.ppm");
      struct statvfs vfs;
      const unsigned long long need = (unsigned long long)paintedW * paintedH * 3 + 32;
      if (statvfs("/tmp", &vfs) != 0 || (unsigned long long)vfs.f_bavail * vfs.f_frsize < 2 * need) {
        fprintf(stderr, "[jihad-bs] frame dump refused: /tmp has too little room for %llu bytes\n", need);
        dumpOnce = false;
      }
    }
    const std::string outPath = dumpOnce ? std::string("/tmp/jihad-frame.ppm") : dumpPath;
    if (!outPath.empty() && nb > 0) {
      int dfd = open(outPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW, 0644);
      FILE* f = dfd >= 0 ? fdopen(dfd, "wb") : nullptr;
      if (!f && dfd >= 0) close(dfd);
      if (f) {
        fprintf(f, "P6\n%d %d\n255\n", paintedW, paintedH);     // buffer is BGRA -> write RGB
        std::vector<unsigned char> line((size_t)paintedW * 3);
        for (long y = 0; y < (long)paintedH; ++y) {
          const unsigned char* p = pixels + (size_t)y * paintedW * 4;
          for (int x = 0; x < paintedW; ++x) {
            line[(size_t)x * 3] = p[(size_t)x * 4 + 2];
            line[(size_t)x * 3 + 1] = p[(size_t)x * 4 + 1];
            line[(size_t)x * 3 + 2] = p[(size_t)x * 4];
          }
          fwrite(line.data(), 1, line.size(), f);
        }
        fclose(f);
        // Pin 0644 rather than inherit the daemon's umask (T-057 (d): root-owned
        // 0755 dirs / 0644 files — never a world-writable artifact).
        chmod(outPath.c_str(), 0644);
        fprintf(stderr, "[jihad-bs] dumped frame -> %s (%dx%d top=%d left=%d bandRow=%ld bandCol=%ld)\n",
                outPath.c_str(), paintedW, paintedH, paintedTop, paintedX, region ? bandRow : 0L,
                region ? bandCol : 0L);
      }
    }
  }
  // Every path that returns WITHOUT publishing must also drop this slot's frame record.
  // Otherwise a record from two frames ago keeps the slot eligible for a damage-only
  // repaint into a buffer whose current contents we did not write.
  if (nb < 0) { invalidateBufRecord(); return; }
  // Blank-over-good suppression (review #7 P1): a mid-reflow render fills the target white
  // (nb==0). Do NOT push that over a previously-good frame — keep the last valid frame on
  // screen and retry next tick (leave mNeedsPaint set, do NOT flip the active buffer). This
  // removes the resize/rotation/navigation white-flash. Only a genuinely-never-rendered page
  // (mHadContent==false) blits a blank frame.
  if (nb == 0 && region && mHadContent) {
    // The visible band is blank but the strips may carry real content (scrolled into a
    // white gap / white footer). Suppressing on the band alone would re-render the full
    // tall region every tick forever and publish none of them (review 2026-08-02 F8) —
    // sample the strips (every 8th row, every 4th px) and publish if anything is there.
    for (long y = 0; y < (long)paintedH && nb == 0; y += 8) {
      if (y >= bandRow && y < bandRow + h) continue;    // the band was already counted
      unsigned char* row = pixels + (size_t)y * paintedW * 4;
      for (int x = 0; x < paintedW; x += 4) {
        unsigned char* p = row + (size_t)x * 4;
        if (!(p[0] > 240 && p[1] > 240 && p[2] > 240)) { nb = 1; break; }
      }
    }
  }
  // A GENUINELY blank page must settle (cavekit-offscreen-rendering.md R8, 2026-10-01).
  //
  // The suppression above is right about a blank that is TRANSIENT — a mid-load or mid-reflow
  // render over a good frame — and it used to assume every blank was: it returned with
  // mNeedsPaint still set, unconditionally, so the next tick re-rendered the whole viewport and
  // refused it again. On a document that is blank on purpose that never ends: device-measured
  // on about:blank, 663 full ~35 ms paints in 30 s, forever, each logging a line (~210 MB/day
  // into a 62 MB /var). about:blank is an allowlisted home page, so a user setting selects it.
  //
  // Two changes, and the white-flash protection survives both:
  //  - A blank is transient only WHILE THE LOAD IS RUNNING. Once the load is done and the frame
  //    has stayed blank for kJihadBlankSettleMs, the blank IS the page: publish it once and
  //    stop. A blank arriving during a load is still never published over the good frame.
  //  - While it is still being suppressed, retry on a kJihadBlankRetryMs grid rather than every
  //    tick — a load stalled on a blank document must not cost a full render per tick either.
  if (nb > 0) {
    mHadContent = true;
    mBlankSinceMs = 0; mBlankRetryAtMs = 0;
  } else if (nb == 0 && mHadContent) {
    const int64_t now = jihadNowMs();
    if (!mLoadWasDone) mBlankSinceMs = 0;            // still loading: transient by definition
    else if (!mBlankSinceMs) mBlankSinceMs = now;
    if (!(mBlankSinceMs && now - mBlankSinceMs >= kJihadBlankSettleMs)) {
      // mNeedsPaint stays true -> retried until content lands or the blank settles. The record
      // must go too: this buffer holds white, and a damage-only repaint into it would publish
      // that white.
      invalidateBufRecord();
      mBlankRetryAtMs = now + kJihadBlankRetryMs;
      return;
    }
    fprintf(stderr, "[jihad-bs] blank frame settled after %lld ms — publishing it (the page is blank)\n",
            (long long)(now - mBlankSinceMs));
    mBlankSinceMs = 0; mBlankRetryAtMs = 0;
  }

  // Record what this buffer now holds, so a later damage-only repaint can prove it is
  // painting into a complete frame of the same geometry. Deliberately HERE — past every
  // early return above — so only a frame that is actually about to be published can become
  // the base for a partial paint.
  //
  // valid = (nb > 0), not (nb >= 0): the only nb==0 frames that reach this point are the
  // genuinely-never-rendered first one (mHadContent still false) and a settled blank page
  // (above). Both are blank buffers and must never be a partial-paint base.
  //
  // bandRow only means anything when the tall-region render ran. In the fallback the frame
  // was read to buffer row 0 (ReadPixels(pixels, ...)) while bandRow still holds the tall
  // computation, and recording that would offset every later damage rect by it.
  {
    int fs = slotForKey(mActiveKey);
    if (fs >= 0) {
      mBufFull[fs].valid = (nb > 0);
      mBufFull[fs].w = w; mBufFull[fs].h = h; mBufFull[fs].zeff = Zeff;
      mBufFull[fs].scrollX = mAdapterScrollX; mBufFull[fs].scrollY = mAdapterScrollY;
      mBufFull[fs].paintedX = paintedX; mBufFull[fs].paintedTop = paintedTop;
      mBufFull[fs].paintedH = paintedH; mBufFull[fs].bandRow = region ? bandRow : 0;
      mBufFull[fs].paintedW = paintedW; mBufFull[fs].bandCol = region ? bandCol : 0;
      mBufFull[fs].docId = docId;
      // A popup drawn over this frame is in its pixels, band and strips alike, and a later
      // frame must not copy it: such a frame is nobody's source.
      mBufFull[fs].overlay = popupsDrawn > 0;
      // Off-screen rows identical to the source's keep its generation, so three buffers
      // that were copied one from another all count as current (the fast path's test). A
      // frame with NO off-screen rows (the viewport only: a page no taller than the screen)
      // renders no strips, so it makes nobody's strips older — it must not bump the
      // generation. It did, on every frame, and so on a short page with a plugin every frame
      // took the full path: the other buffers always looked stale (device 2026-10-06,
      // jihad-anim.swf: partial=0 of 32 frames, paint ~25 ms, 15-16 fps).
      mBufFull[fs].stripGen = !region ? mStripGen
                            : (src && !fresh && !stripsChanged) ? src->stripGen : ++mStripGen;
      mLatestSlot = (nb > 0) ? fs : -1;
      // Another step is owed only if this frame can be built on — otherwise each "step"
      // would be a first frame again, for ever.
      mGrowOwed = growOwed && nb > 0 && docId != 0 && !mBufFull[fs].overlay &&
                  !(maintKind == 1 && rowsCopied == 0);
      if (fresh) { mSweepLeft = 0; mSweepRow = 0; }                 // nothing here is old
      // Owed a sweep: a frame that reacted to a change of unknown or large extent. Not one
      // that was painted only because the viewport moved — nothing changed for that.
      else if (maintKind == 0 && hadDamage && !smallDamage) mSweepLeft = paintedH;
      if (maintKind == 2) {
        // Keep a sweep step short: the daemon hears nothing while it renders. And once a
        // sweep has gone all the way round, leave the page alone for a while — one that
        // changes every few seconds must not be swept continuously.
        const long took = (long)(jihadNowMs() - paintT0);
        if (took > 200 && mSweepStep > 128) mSweepStep /= 2;
        else if (took < 70 && mSweepStep < 1024) mSweepStep *= 2;
        if (mSweepLeft <= 0) mSweepNotBeforeMs = jihadNowMs() + 8000;
      }
      // A full paint repainted the whole band: this buffer is completely current, so it has no
      // pending per-buffer damage. (dmgValid is cleared even when valid=false — a blank buffer
      // has nothing to catch up either.)
      clearBufDamage(fs);
    }
  }

  mNeedsPaint = false;
  // Anything that changed the pixels may have changed the SCROLLABLE SIZE too, and until
  // 2026-08-06 only a load completion or a resize ever re-measured it. A page that rebuilds
  // its own content after load — about:preferences swapping panes is the case that was
  // reported, but any script-driven show/hide does it — kept the adapter's original content
  // size, and the adapter decides from that whether there is anything to scroll. The dirty
  // flag is NOT a reliable trigger here (measured: the pane swap repaints without it), so
  // this hangs off the paint, which by definition happened. Drained on the next pump tick,
  // and emitGeometry only emits when the size really changed.
  mGeometryDirty = true;
  mSink.msgPainted(mActiveKey);
  // This buffer is now the adapter's until it returns it (F-211). Mark it in flight so a
  // subsequent paint won't overwrite it mid-blit, and record when for the timeout valve.
  {
    int ps = slotForKey(mActiveKey);
    if (ps >= 0) { mInFlight[ps] = true; mPaintMs[ps] = jihadNowMs(); }
  }
  // Double buffer: next paint targets the other segment. The adapter holds one buffer at a
  // time and returns the previous one on the next msgPainted, so alternating keeps a free
  // buffer available; the in-flight gate above blocks reuse until that return arrives.
  mActiveKey = nextBufferKey();
}

} // namespace jihad
