/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser &mdash; BrowserPageGoanna implementation. See BrowserPageGoanna.h.
 * Bridges the YAP BrowserPage command surface to GoannaRenderPage and emits the
 * server->adapter messages via IPageMessageSink.
 */
#include "BrowserPageGoanna.h"
#include "EngineHost.h"
#include "GoannaRenderPage.h"
#include "BrowserOffscreenInfo.h"   // isis shmem header (from render/browserserver/Src)
#include "JihadLogo.h"              // JIHAD_LOGO_B64 (app icon for about: pages)
#include "JihadRuntimePaths.h"      // the ONE runtime-state dir (T-057 / R8)

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
#include <poll.h>       // wait for the card's answer with a deadline
#include <unistd.h>
#include <cerrno>
#include <cstring>

namespace { }  // (UrlRule defined below in the jihad namespace)

namespace jihad {

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

BrowserPageGoanna::BrowserPageGoanna(EngineHost& host, IPageMessageSink& sink)
  : mHost(host), mSink(sink), mPage(nullptr),
    mKey1(0), mKey2(0), mBufSize(0), mActiveKey(0),
    mLoadWasDone(false), mNeedsPaint(false),
    mLastContentW(-1), mLastContentH(-1),
    mLastScrollX(-1), mLastScrollY(-1), mZoom(1.0),
    mAdapterScrollX(0), mAdapterScrollY(0), mFrozen(false), mHadContent(false), mGeometryDirty(false),
    mNavGen(0), mLastScrollMs(0) {
  mShmBuf[0] = mShmBuf[1] = nullptr; mShmId[0] = mShmId[1] = -1;
  mInFlight[0] = mInFlight[1] = false; mPaintMs[0] = mPaintMs[1] = 0;
  mLastBackspaceMs = 0; mBackspaceRun = 0;
  mLoadStartMs = 0;
  mLastProgress = 0;
  mWatchdogDismissed = false;
  mDirtyPending = false; mLastDirtyPaintMs = 0;
  // Engine dialogs are process-wide (one prompter service), so the LAST page created owns
  // them — the same single-page-embedding assumption DebugRunChromeJs already makes. Without
  // this nothing implements DialogSink and every dialog silently takes its default.
  SetDialogSink(this);
}

// Deferred editing keys (run in pump(), not the keyDown YAP callback — Codex F-219). Enter may
// submit a form and navigate; Tab in an <input> focuses another field (fires focus/blur JS). Both
// must run in the page-lifetime-protected pump loop. (Tab in a <textarea> just inserts a tab, but
// HandleTab decides that, so all Tabs defer for uniformity.)
enum { PEA_NONE = 0, PEA_TAB = 1, PEA_TAB_BACK = 2, PEA_ENTER = 3 };

// Monotonic-enough wall clock in ms for the buffer flow-control timeout valve.
static long jihadNowMs() {
  struct timeval tv; gettimeofday(&tv, NULL);
  return (long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int BrowserPageGoanna::slotForKey(int key) const {
  if (key && key == mKey1) return 0;
  if (key && key == mKey2) return 1;
  return -1;
}

static int jihadShmResolve(int keyOrId);   // defined below

// Resolve a key/shmid and return a cached attachment (attaching once, then reusing). shmat/shmdt
// per paint cost ~400ms/keystroke on device; the two alternating segments are attached at most once.
unsigned char* BrowserPageGoanna::attachShm(int keyOrId) {
  int id = jihadShmResolve(keyOrId);
  if (id < 0) return nullptr;
  for (int i = 0; i < 2; ++i) if (mShmId[i] == id && mShmBuf[i]) return mShmBuf[i];
  void* p = shmat(id, nullptr, 0);
  if (p == (void*)-1) { perror("[BrowserPageGoanna] shmat"); return nullptr; }
  int slot = (mShmBuf[0] == nullptr) ? 0 : 1;   // 2 buffers total; fill an empty slot
  if (mShmBuf[slot]) { shmdt(mShmBuf[slot]); }
  mShmBuf[slot] = (unsigned char*)p; mShmId[slot] = id;
  return mShmBuf[slot];
}

void BrowserPageGoanna::detachShm() {
  for (int i = 0; i < 2; ++i) { if (mShmBuf[i]) shmdt(mShmBuf[i]); mShmBuf[i] = nullptr; mShmId[i] = -1; }
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

BrowserPageGoanna::~BrowserPageGoanna() {
  // Never leave the process-wide sink pointing at a destroyed page.
  SetDialogSink(nullptr);
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
      mSink.msgUrlRedirected(url, r->userData.c_str());   // R6: notify the client
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
  mBufSize = sharedBufferSize;
  mActiveKey = mKey1;
  mInFlight[0] = mInFlight[1] = false;   // fresh segments: nothing in flight (F-211)

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
  if (shrink && hadEditable) {
    int sx1 = 0, sy1 = 0; mPage->GetScrollXY(&sx1, &sy1);
    // Only counter an engine-induced downward push (field-into-view), and only when
    // the tapped field's top stays within the shrunken viewport if we restore — i.e.
    // its content-y is above the pre-resize scroll + the new (smaller) height. That
    // keeps a field the VKB would otherwise cover lifted, but stops the common case
    // (top-of-page search box) from being shoved off the top.
    if (sy1 > sy0) mPage->ScrollTo(sx0, sy0);
  }
  mLastWinH = (int)height;
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
  mInFlight[0] = mInFlight[1] = false;   // reattached (possibly new) segments: clear in flight (F-211)
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
    long slack = h2 / 2;   // trigger EARLY: the repaint takes real time on this CPU and the
                           // fling keeps consuming rows while it renders ("content doesn't
                           // stay consistently visible" — h/4 triggered too late)
    double Zc = (z >= 0.99 && z <= 1.01) ? 1.0 : z;   // the zoom the paint path would use
    long contentRows = (mLastContentH > 0) ? (long)((double)mLastContentH * Zc) : 0;
    long loBound = (mPaintedLo <= 0) ? 0 : mPaintedLo + slack;             // page top: no slack
    long hiBound = (contentRows > 0 && mPaintedHi >= contentRows)
                       ? mPaintedHi : mPaintedHi - slack;                   // page end: no slack
    bool covered = mPaintedHi > mPaintedLo &&
                   x == mPaintedX &&
                   (mPaintedZoom > Zc - 0.001 && mPaintedZoom < Zc + 0.001) &&
                   (long)y >= loBound && (long)y + h2 <= hiBound;
    if (!covered) mNeedsPaint = true;   // covered: keep whatever was already pending
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
  mNeedsPaint = true;
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
    "&mdash; webOS browser integration (Apache License 2.0)</div>";
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

void BrowserPageGoanna::openUrl(const char* url) {
  if (!mPage || !url) return;
  // A newer explicit navigation supersedes ALL queued input aimed at the old document, and the
  // generation bump stops an in-flight drain from continuing into the superseded page.
  mPendingMouse.clear(); ++mNavGen;
  mPendingEditActions.clear();   // queued Tab/Enter for the old page are stale after a navigation
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
  // R6: a matching redirect rule hands the URL to the client (msgUrlRedirected)
  // and, if it is a redirect, is not loaded in the browser at all.
  if (applyRedirectRules(url)) return;
  mLoadWasDone = false;
  mNeedsPaint = false;
  mLastProgress = 0;
  mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted();
  if (!mPage->LoadUrl(url)) {
    // Synchronous rejection (bad/unknown-scheme URL): report it as a failed load
    // and don't leave the adapter permanently "loading" (Codex P2 + R3).
    mSink.msgFailedLoad("Goanna", 0, url, "Load failed");
    mSink.msgLoadProgress(100);
    mSink.msgLoadStopped();
    mLoadWasDone = true;
  }
}

void BrowserPageGoanna::setHTML(const char* /*url*/, const char* body) {
  if (!mPage || !body) return;
  mLoadWasDone = false; mNeedsPaint = false; mLastProgress = 0;
  mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted();
  if (!mPage->SetHtml(body)) { mSink.msgLoadStopped(); mLoadWasDone = true; }
}

// Nav commands restart the load lifecycle so completion re-emits load+location.
// back/forward/reload also clear editor focus so the VKB lowers over the new page (review #7 P2).
void BrowserPageGoanna::pageBackward() { if (mPage) { mPendingMouse.clear(); ++mNavGen; if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false,0,0); mLoadWasDone=false; mNeedsPaint=false; mLastProgress=0; mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted(); mPage->GoBack(); } }
void BrowserPageGoanna::pageForward() { if (mPage) { mPendingMouse.clear(); ++mNavGen; if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false,0,0); mLoadWasDone=false; mNeedsPaint=false; mLastProgress=0; mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted(); mPage->GoForward(); } }
void BrowserPageGoanna::pageReload()  { if (mPage) { mPendingMouse.clear(); ++mNavGen; if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false,0,0); mLoadWasDone=false; mNeedsPaint=false; mLastProgress=0; mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted(); mPage->Reload(); } }
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
  // Never log key/chr — those are user keystrokes (F-163). When a tapped editable is the type
  // target, the webOS VKB delivers each character (and the editing keys) as a keyDown; route it
  // into the field via the engine editor (caret-aware value edits + SetSelectionRange), and render
  // IMMEDIATELY (don't wait for the ~16ms tick) so typing feels responsive. Non-editable focus
  // falls through to normal engine key dispatch.
  if (mPage->HasFocusedEditable()) {
    // webOS delivers editing keys to the plugin as: ASCII control codes where they exist
    // (ESC=27, Backspace=8, Tab=9, Enter=13) and the Apple/NSEvent function-key Unicode range
    // (0xF700+) for arrows / Home / End / forward-Delete — the WebKit webOS derives from puts
    // those code points in the key field. (Qt keycodes 0x0100001x are matched too, as a fallback.)
    // The code arrives in `key` with `chr`==0, but we accept it from either field. A printable
    // character inserts at the caret; the editing keys drive the engine caret.
    typedef jihad::GoannaRenderPage GRP;
    const int kBackspace = 8, kTab = 9, kEnter = 13, kDelete = 127;
    const int kQtTab = 0x01000001, kQtBacktab = 0x01000002, kQtReturn = 0x01000004,
              kQtEnter = 0x01000005, kQtDelete = 0x01000007, kQtHome = 0x01000010,
              kQtEnd = 0x01000011, kQtLeft = 0x01000012, kQtUp = 0x01000013,
              kQtRight = 0x01000014, kQtDown = 0x01000015;
    const int kMacUp = 0xF700, kMacDown = 0xF701, kMacLeft = 0xF702, kMacRight = 0xF703,
              kMacDelete = 0xF728, kMacHome = 0xF729, kMacEnd = 0xF72B;
    // webOS's own special-key block (measured on-device via the keyprobe): the VKB keypad's arrow
    // keys arrive here, NOT in the Apple/Qt ranges. Down/Up/Left/Right are 0xE0A0..0xE0A3.
    const int kWebDown = 0xE0A0, kWebUp = 0xE0A1, kWebLeft = 0xE0A2, kWebRight = 0xE0A3;
    const bool shift = (modifiers & (1 << 2)) != 0;   // npPalmShiftKeyModifier
    auto km = [key, chr](int v) { return key == v || chr == v; };
    if (!km(kBackspace)) mBackspaceRun = 0;   // any non-Backspace key breaks the accelerate run
    if (km(kBackspace)) {
      // Accelerate a held Backspace: consecutive presses within the auto-repeat window build a run;
      // once sustained (~0.5s of holding), delete a whole word per repeat so long text clears fast.
      long now = jihadNowMs();
      mBackspaceRun = (now - mLastBackspaceMs <= 250) ? (mBackspaceRun + 1) : 1;
      mLastBackspaceMs = now;
      if (mBackspaceRun >= 12) mPage->DeleteBackwardWord();
      else                     mPage->DeleteBackward();
      mNeedsPaint = true;
    } else if (km(kDelete) || km(kQtDelete) || km(kMacDelete)) {
      mPage->EditKey(GRP::EK_DELETE); mNeedsPaint = true;
    } else if (km(kTab) || km(kQtTab) || km(kQtBacktab)) {
      // Tab: <textarea> inserts a tab, single-line <input> moves to the next field (fires focus JS)
      // — defer to pump() (F-219). Shift+Tab / the Qt backtab code go backward.
      mPendingEditActions.push_back((shift || km(kQtBacktab)) ? PEA_TAB_BACK : PEA_TAB); mNeedsPaint = true;
    } else if (km(kEnter) || km(kQtReturn) || km(kQtEnter)) {
      // Enter may submit a form and navigate — defer to pump() (never navigate in the key callback).
      mPendingEditActions.push_back(PEA_ENTER); mNeedsPaint = true;
    } else if (km(kQtLeft)  || km(kMacLeft)  || km(kWebLeft))  { mPage->EditKey(GRP::EK_LEFT);  mNeedsPaint = true; }
    else if (km(kQtRight) || km(kMacRight) || km(kWebRight)) { mPage->EditKey(GRP::EK_RIGHT); mNeedsPaint = true; }
    else if (km(kQtUp)    || km(kMacUp)    || km(kWebUp))    { mPage->EditKey(GRP::EK_UP);    mNeedsPaint = true; }
    else if (km(kQtDown)  || km(kMacDown)  || km(kWebDown))  { mPage->EditKey(GRP::EK_DOWN);  mNeedsPaint = true; }
    else if (km(kQtHome)  || km(kMacHome))  { mPage->EditKey(GRP::EK_HOME);  mNeedsPaint = true; }
    else if (km(kQtEnd)   || km(kMacEnd))   { mPage->EditKey(GRP::EK_END);   mNeedsPaint = true; }
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
      // Function/nav keys arrive as Private-Use code points, NOT text: webOS's own special-key
      // block 0xE0A0-0xE0BF (arrows measured at 0xE0A0-0xE0A3; Home/End/PageUp/Down likely adjacent)
      // and the Apple/NSEvent function-key range 0xF700-0xF8FF. The ones we map are consumed above;
      // swallow the rest here so an unmapped nav key never inserts an invisible PUA glyph (which
      // read as "arrows type characters" + repaint lag + Backspace eating the invisible chars).
      else if (c >= 0xE0A0 && c <= 0xE0BF) c = 0u;
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
  if (mPage) { mPage->KeyEvent("keyup", key, chr, modifiers); mNeedsPaint = true; }
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
  double cx = *x / z, cy = *y / z;              // zoomed doc px -> CSS doc px
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
                                         bool* accept, std::string* value) {
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
  long start = jihadNowMs();
  bool got = false, writerSeen = false;
  for (;;) {
    long elapsed = jihadNowMs() - start;
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
      fprintf(stderr, "[jihad-bs] dialog %s: card picked up after %ld ms\n",
              what, jihadNowMs() - start);
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
  fprintf(stderr, "[jihad-bs] dialog %s: answered after %ld ms\n", what, jihadNowMs() - start);
  size_t pos = 4;
  std::vector<std::string> args;
  while (pos < data.size() && args.size() < 4) {
    size_t end = data.find('\0', pos);
    if (end == std::string::npos) { args.push_back(data.substr(pos)); break; }
    args.push_back(data.substr(pos, end - pos));
    pos = end + 1;
  }
  if (!args.empty() && accept) *accept = (args[0] == "1" || args[0] == "2");
  if (args.size() > 1 && value) *value = args[1];
  fprintf(stderr, "[jihad-bs] dialog %s -> %s\n", what,
          (accept && *accept) ? "ACCEPT" : "cancel");
  return true;
}

void BrowserPageGoanna::OnDialog(DialogKind kind, const char* text, DialogReply* reply) {
  if (!reply) return;
  // Defaults are the safe answer for each kind, and stand if anything below fails.
  reply->accept = false;
  reply->promptValue.clear();

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

  awaitDialogReply(path, kindName, &reply->accept, &reply->promptValue);
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
void BrowserPageGoanna::touchEvent(int type, int /*count*/, int /*mods*/, const char* touchesJson) {
  if (!mPage) return;
  // Minimal parse of the first touch point's x/y from the touches JSON.
  int x = 0, y = 0;
  if (touchesJson) {
    const char* px = strstr(touchesJson, "\"x\"");
    const char* py = strstr(touchesJson, "\"y\"");
    if (px) sscanf(px, "\"x\"%*[: ]%d", &x);
    if (py) sscanf(py, "\"y\"%*[: ]%d", &y);
  }
  // Queued like every other input (F-9/F5): TouchEvent -> SendTouchEventToWindow runs
  // touchstart/touchend page JS synchronously, so dispatching it from this YAP callback is the
  // same reboot-class crash door the queue was built to close. Currently unreachable from the
  // device (the adapter's doTouchEvent body is #ifdef QT_FIXME'd out) but the daemon command IS
  // wired, so this must already be safe the day touch forwarding is turned on.
  int t = (type == 0) ? PM_TOUCHSTART : (type == 2) ? PM_TOUCHEND : PM_TOUCHMOVE;
  int cx, cy; mapToContent(x, y, &cx, &cy);   // R5: surface -> content, at QUEUE time
  queueInput(t, cx, cy, 0);
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
    std::string pipePath = makeDialogPipe();
    fprintf(stderr, "[jihad-bs] dialog ssl-confirm -> card (pipe %s) host=%s\n",
            pipePath.empty() ? "(none)" : pipePath.c_str(), chost.c_str());
    mSink.msgSSLConfirm2(pipePath.c_str(), chost.c_str(), ccode, "");
    if (!pipePath.empty()) {
      bool trust = false;
      awaitDialogReply(pipePath, "ssl-confirm", &trust, nullptr);
      if (trust) {
        if (mPage->AcceptCurrentCert()) {
          std::string retry = mPage->CurrentUri();
          fprintf(stderr, "[jihad-bs] ssl: override remembered for %s — reloading\n", chost.c_str());
          // Reload through the normal load path so the retry reports load state as usual.
          if (!retry.empty() && retry != "about:blank") openUrl(retry.c_str());
          else mPage->Reload();
        } else {
          fprintf(stderr, "[jihad-bs] ssl: could not remember the override for %s\n", chost.c_str());
        }
      }
    }
  }
  else if (failed) mSink.msgFailedLoad("Goanna", code, furl.c_str(), "Load failed");
  // For internal about: pages, report the typed about: URL, not the data: URL the engine loaded
  // (keeps the bar showing about:jihad and avoids a huge data: history entry). NB: never emit
  // msgUrlRedirected for an ordinary HTTP 3xx redirect — in isis the app binds onUrlRedirected ->
  // openResource -> the DEFAULT (stock) browser, so a normal redirect (google.com -> www.google.com)
  // would leave Jihad. A redirect is just a location change, reported here.
  std::string uri = mAliasUrl.empty() ? mPage->CurrentUri() : mAliasUrl;
  mSink.msgLocationChanged(uri.c_str(), mPage->CanGoBack(), mPage->CanGoForward());
  // The isis address bar updates on the title+url message (BasicWebView.titleURLChange ->
  // urlTitleChanged -> ActionBar.setUrl), NOT on msgLocationChanged. If the page has no <title>, fall
  // back to the URL so the bar still shows where we are (review #6 F-003).
  { std::string title = mPage->GetTitle();
    if (title.empty()) title = uri;
    bool cb = mPage->CanGoBack(), cf = mPage->CanGoForward();
    fprintf(stderr, "[jihad-bs] titleAndUrl title=[%s] uri=%s back=%d fwd=%d\n", title.c_str(), uri.c_str(), (int)cb, (int)cf);
    mSink.msgTitleAndUrlChanged(title.c_str(), uri.c_str(), cb, cf); }
  return failed ? std::string() : uri;
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
  if (!uri.empty() && mAliasUrl.empty()) mSink.msgUpdateGlobalHistory(uri.c_str(), false);  // R6
  emitGeometry();      // R4: contents-size + meta-viewport once the page settled
  mNeedsPaint = true;  // paint the final frame once (dedup — Codex P2)
}

void BrowserPageGoanna::emitLoadAndLocation() {
  if (!mPage) return;
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
            jihadNowMs() - mLoadStartMs);
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
  fprintf(stderr, "[jihad-bs] emitGeometry contentSize=%dx%d win=%dx%d mZoom=%.4f (reporting=%d)\n",
          cw, ch, mPage->Width(), mPage->Height(), mZoom,
          (int)(got && cw > 0 && ch > 0 && (cw != mLastContentW || ch != mLastContentH)));
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
    mLastContentW = cw; mLastContentH = ch;
    mSink.msgContentsSizeChanged(cw, ch);        // R4: contents-size-changed
  }
  double is = 1.0, mn = 1.0, mx = 1.0; int vw = 0, vh = 0; bool us = true;
  if (mPage->GetViewport(&is, &mn, &mx, &vw, &vh, &us))
    mSink.msgMetaViewportSet(is, mn, mx, vw, vh, us);   // R4: meta-viewport
  return got && cw > 0 && ch > 0;   // a valid content size was available (deferred-emit can stop retrying)
}

void BrowserPageGoanna::emitScrollIfChanged() {
  if (!mPage) return;
  // When zoomed (>1) the authoritative visual scroll is the adapter's render-pan, not the engine
  // scroll (JihadRenderDocument ignores the engine scroll via RENDER_DOCUMENT_RELATIVE). Echoing
  // the engine scroll here would overwrite the adapter's horizontal pan and the vertical position
  // it is driving (Codex F2), so suppress the scrolled-to echo while zoomed.
  if (mZoom < 0.99 || mZoom > 1.01) return;
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
    if (!adapterDriven) mSink.msgScrolledTo(sx, sy);   // R4: scrolled-to (content-initiated)
  }
}

void BrowserPageGoanna::pump(int msBudget) {
  if (!mPage) return;
  // Fire the pending 'input' event for a keystroke edit FIRST — before the queued tap AND the Tab/Enter
  // queue below. It targets the element that was actually edited (mPendingInputEl). Flushing before the
  // tap matters when the user types then taps a submit/button or a link: a controlled/React handler must
  // observe the edited value before the click submits or navigates, and the event must dispatch on the
  // still-current document before openUrl swaps it out (Codex F-266/F-291). Runs here (guarded pump),
  // not in keyDown, since onChange/oninput run page JS.
  mPage->FlushPendingInputEvent();
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
        std::string clickNav;
        if (mPage->TakeClickNav(&clickNav)) {
          // R6 (navigation-events): report the intercepted link activation, THEN navigate via
          // the load path. openUrl marks the load programmatic, so OnStateChange won't ALSO
          // flag it link-clicked — this msg is the single R6 notification for the tap.
          fprintf(stderr, "[jihad-bs] clickAt -> navigate %s\n", clickNav.c_str());
          mSink.msgLinkClicked(clickNav.c_str());
          openUrl(clickNav.c_str());   // bumps mNavGen -> the loop above stops next iteration
        }
        // A dropdown <select> tap queued a card-native popup instead of a click (Atlas model):
        // write the option data to a temp file and emit msgPopupMenuShow(id, file). The adapter
        // reads+unlinks the file and calls the card's showPopupMenu; the choice returns via
        // popupMenuSelect (asyncCmdPopupMenuSelect).
        emitSelectPopupIfPending();
        continue;
      }
      if (e.type == PM_TOUCHSTART || e.type == PM_TOUCHMOVE || e.type == PM_TOUCHEND) {
        mPage->TouchEvent(e.type == PM_TOUCHSTART ? "touchstart"
                        : e.type == PM_TOUCHEND   ? "touchend" : "touchmove", e.x, e.y);
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
      mPage->MouseEvent(t, e.x, e.y, e.detail);
    }
    mNeedsPaint = true;
  }
  // Process a queued editing key (Tab/Enter) in the SAME page-lifetime guard — it runs page JS
  // that may move focus or submit a form (navigate), which is unsafe in the keyDown YAP callback
  // (Codex F-219). A form submit navigates via the engine and is completed by the TakeLinkClicked
  // re-drive below, exactly like any content-initiated navigation.
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
  std::string linkUrl; bool linkIsPost = false;
  if (mPage->TakeLinkClicked(&linkUrl, &linkIsPost)) {
    mSink.msgLinkClicked(linkUrl.c_str());
    // Content-initiated navigation (JS `location.href`/`location.assign`, a form GET/POST,
    // meta-refresh, or a button onclick that sets location) STARTS but does NOT COMPLETE on its own
    // in this offscreen embedding (verified: it fires STATE_START for the target but never load-done).
    if (!linkIsPost) {
      // Re-drive a GET-class nav through the programmatic load path, which completes and supersedes
      // the stalled content load. openUrl marks the load programmatic so it won't re-trigger
      // TakeLinkClicked -> no loop.
      fprintf(stderr, "[jihad-bs] content-nav re-drive GET -> %s\n", linkUrl.c_str());
      openUrl(linkUrl.c_str());
    } else {
      // A POST content-nav (form submit) is NOT re-driven: the original request has already reached
      // STATE_START — the body may already be on the wire — so re-issuing it would DOUBLE the POST
      // (double login/charge, Codex F-262). Leave the request to the ENGINE, but ADOPT it as the
      // tracked load so the daemon still: shows the overlay + progress, arms the stall watchdog, and
      // (crucially) emits the completion + new location and schedules a repaint when the POST response
      // arrives — otherwise the response silently stays on the old frame (Codex F-289).
      fprintf(stderr, "[jihad-bs] content-nav POST (engine-driven) %s\n", linkUrl.c_str());
      mPage->AdoptContentLoad();
      mLoadWasDone = false;
      mNeedsPaint = false;
      mLastProgress = 0;
      mLoadStartMs = jihadNowMs();
      mSink.msgLoadStarted();
    }
  }
  mPage->PumpFor(msBudget);
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
    long pnow = jihadNowMs();
    if ((pnow - mLastScrollMs) < 400 && (pnow - mLastPaintDoneMs) >= 250) mNeedsPaint = true;
  }
  if (mPage->TakeDirty()) mDirtyPending = true;
  if (mDirtyPending) {
    long dnow = jihadNowMs();
    if (dnow - mLastDirtyPaintMs >= 150) { mNeedsPaint = true; mDirtyPending = false; mLastDirtyPaintMs = dnow; }
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
  // Emit deferred resize geometry now that PumpFor has let the reflow settle (review #7 P2).
  // emitGeometry itself guards against a still-degenerate 0x0 (P1), so a not-yet-settled
  // reflow just re-defers via the guard until a real size is available.
  if (mGeometryDirty && emitGeometry()) mGeometryDirty = false;   // retry until reflow yields a valid size
  emitLoadAndLocation();
  emitScrollIfChanged();
  // Low-memory guardrail (512 MB Pre 3 floor): internally rate-limited /proc/meminfo poll; fires the
  // engine "memory-pressure" flush + malloc_trim when RAM runs short. Lives here (not the engine-
  // agnostic browserserver tick) so the daemon layer keeps zero engine includes.
  mHost.CheckMemoryPressure();
}

void BrowserPageGoanna::maybePaint() {
  if (mFrozen) return;                       // card backgrounded: don't paint
  // The scroll-settle gate is GONE ("content doesn't stay consistently visible", device
  // 2026-08-02). It predates honest renderedX/Y: back then a mid-pan repaint mispositioned
  // the frame, so paints were held until the pan settled. Now a mid-pan frame is
  // positionally correct AND the coverage-aware skip means a paint request during a pan
  // exists precisely because the pan is about to run out of painted rows — holding it
  // until the fling ended guaranteed the white gap it was trying to avoid (every scroll
  // message reset the gate, so a continuous fling starved paints entirely).
  if (mNeedsPaint) paintToSharedBuffer();    // only when there is a new frame
}

void BrowserPageGoanna::paintToSharedBuffer() {
  if (!mPage || !mActiveKey) return;
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
      if (jihadNowMs() - mPaintMs[gs] < 2000) return;  // adapter may still hold it — wait
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
  // SGX540/Piranha cap (review 2026-08-02 F1): the adapter's PRIMARY composite path wraps
  // the WHOLE painted region as one PGSurface and bitblts it through the GPU compositor.
  // Piranha is closed source and the SGX540's max texture dimension is 2048 — a portrait
  // region of 2355 rows would be the only configuration to cross it, and it is exactly the
  // configuration the desktop round-trip cannot exercise. Cap until a device run proves a
  // taller PGSurface blits.
  if (maxRows > 2048) maxRows = 2048;

  // Where the viewport-relative band actually is, in ZOOMED px. At z~1 the engine scroll
  // is authoritative for Y (ScrollTo is an async javascript: — it can lag the adapter's
  // pan); when zoomed the band is the CLAMPED render pan. Stamping renderedY from the raw
  // adapter scroll while the frame was rendered elsewhere was itself a source of
  // mispositioned frames ("content moves without me touching it").
  // If the engine scroll is UNREADABLE at z~1, take the viewport-exact fallback instead of
  // stamping a geometry we cannot know (review F9: mixed coordinate spaces with an
  // honest-looking header). flushLayout=false — the renders below flush anyway (F4).
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

  // Painted rows [lo, hi): cover the band AND the adapter's pan target, plus overscan
  // (half a viewport above, one below — scrolling down dominates), bounded by the segment
  // and the content height (mLastContentH is css px; unknown 0 = don't bound).
  long contentRows = (mLastContentH > 0) ? (long)((double)mLastContentH * Zeff) : 0;
  long adY = (long)mAdapterScrollY;
  // Bias the headroom toward the pan DIRECTION ("content blips in and out when I scroll",
  // device 2026-08-02): the strip behind the pan is dead weight; the strip ahead is what
  // the user is about to expose. Direction from the adapter scroll delta since last paint.
  long ovAbove = h / 2, ovBelow = h;
  if (adY > mPrevPaintScrollY + 8)      { ovAbove = h / 8; ovBelow = h + h / 2; }
  else if (adY < mPrevPaintScrollY - 8) { ovAbove = h + h / 2; ovBelow = h / 8; }
  mPrevPaintScrollY = adY;
  long lo = (bandY < adY ? bandY : adY) - ovAbove;
  long hi = (bandY > adY ? bandY : adY) + h + ovBelow;
  if (lo < 0) lo = 0;
  if (contentRows > 0 && hi > contentRows) hi = contentRows;
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
  int tallH = (int)(hi - lo);
  if (!bandKnown) tallH = h;                  // F9: unknown band → viewport-exact fallback

  long paintT0 = jihadNowMs();
  // NO fling-mode fast path (tried 2026-08-02, removed the same day): skipping the band
  // overlay during flings painted position:fixed content at its DOCUMENT position in
  // those frames while full frames painted it at the viewport — two models alternating,
  // so a fixed banner "flew around" mid-scroll (user report). A full region+band paint
  // measured 66 ms on device, cheap enough to be every frame's model: fixed content bakes
  // at the band position each frame, swims with the pan between paints, snaps per paint —
  // the classic webOS-era behavior.
  long nb = -1;
  int paintedX = (int)bandX, paintedTop = (int)bandY, paintedH = h;
  bool region = false;
  long bandRow = bandY - lo;
  if (bandRow < 0) bandRow = 0;
  if (bandRow > (long)tallH - h) bandRow = (long)tallH - h;
  if (tallH > h) {
    // Absolute document rows straight into the shared buffer (not bounded by the widget
    // DrawTarget). docX/docY are css px = zoomed px / Zeff.
    if (!zoomed) {
      // Render ONLY the strips doc-relative — the band rows come from the viewport-relative
      // overlay below, so rendering them here too was pure duplicate work (~30% of the
      // frame: 2048-row frame = 2990 rows rendered, now 2048 — "trim the paint cost",
      // device 2026-08-02, measured 185 ms before this change at the region cap).
      bool okAbove = true, okBelow = true;
      if (bandRow > 0) {
        okAbove = mPage->RenderRegion(pixels, w * 4, w, (int)bandRow,
                                      (double)bandX / Zeff, (double)lo / Zeff, Zeff);
      }
      long belowRows = (long)tallH - h - bandRow;
      if (belowRows > 0) {
        okBelow = mPage->RenderRegion(pixels + (size_t)(bandRow + h) * w * 4, w * 4,
                                      w, (int)belowRows, (double)bandX / Zeff,
                                      (double)(lo + bandRow + h) / Zeff, Zeff);
      }
      region = okAbove && okBelow;
    } else {
      // Zoomed: single-pass region (there is no separate band pass at zoom).
      region = mPage->RenderRegion(pixels, w * 4, w, tallH,
                                   (double)bandX / Zeff, (double)lo / Zeff, Zeff);
    }
    if (region) { paintedTop = (int)lo; paintedH = tallH; }
  }
  if (region && !zoomed) {
    // Overlay the viewport-relative band over the visible rows so position:fixed/sticky
    // and the caret stay correct on screen; the doc-relative strips around it only show
    // transiently mid-pan. ReadPixels renders v2 (viewport-relative at z~1) + readback,
    // forces alpha and returns the nonblank count — same "did it render" signal as before.
    nb = mPage->ReadPixels(pixels + (size_t)bandRow * w * 4, (size_t)w * h * 4);
  } else if (region) {
    // Zoomed: the region render IS the frame (pure visual viewport, same semantics as the
    // old single-pass zoomed paint; fixed content already doc-positioned when zoomed).
    // Count nonblank over the visible band rows only — same signal, 1/tallH the cost.
    nb = 0;
    for (long y = bandRow; y < bandRow + h; ++y) {
      unsigned char* row = pixels + (size_t)y * w * 4;
      for (int x = 0; x < w; ++x) {
        unsigned char* p = row + (size_t)x * 4;
        if (!(p[0] > 240 && p[1] > 240 && p[2] > 240)) ++nb;
      }
    }
  } else {
    // Region render unavailable (old libxul: weak symbol absent / render failed / no room
    // for overscan): the previous viewport-exact paint, stamped at the band's true position.
    paintedTop = (int)bandY; paintedH = h;
    nb = mPage->ReadPixels(pixels, segSize - hdr);
  }

  // A XUL popup (about:addons tools menu, a context menu) is a SEPARATE display root,
  // so nothing above can have drawn it — composite any open one over the frame we just
  // painted, at the same origin/zoom. Costs one popup-manager query when none is open,
  // which is the normal case. Must come AFTER the band overlay: that overlay rewrites
  // the visible rows and would erase a popup drawn before it.
  if (nb >= 0) {
    int popups = mPage->CompositePopups(pixels, w * 4, w, paintedH,
                                        (double)paintedX / Zeff, (double)paintedTop / Zeff,
                                        Zeff);
    if (popups > 0) {
      fprintf(stderr, "[jihad-bs] composited %d popup(s) over the frame\n", popups);
      // A popup is content: an otherwise-blank frame carrying one must not be
      // suppressed as "blank over good" below.
      if (nb == 0) nb = 1;
    }
  }

  oi->bufferWidth = w; oi->bufferHeight = paintedH;
  oi->contentZoom = Zeff;
  oi->renderedX = paintedX; oi->renderedY = paintedTop;
  oi->renderedWidth = w; oi->renderedHeight = paintedH;
  // Record the painted region for the coverage-aware repaint skip (setScrollPosition).
  mPaintedX = paintedX; mPaintedLo = paintedTop; mPaintedHi = (long)paintedTop + paintedH;
  mPaintedZoom = Zeff;
  mLastPaintDoneMs = jihadNowMs();   // pan-cadence refresh reference
  if (nb >= 0) {
    fprintf(stderr, "[jihad-bs] painted shmid=0x%x bytes=%ld (%dx%d top=%d band=%ld) mZoom=%.4f contentZoom=%.4f ms=%ld\n",
            (unsigned)mActiveKey, nb, w, paintedH, paintedTop, bandY, mZoom, oi->contentZoom,
            jihadNowMs() - paintT0);
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
    static const std::string dumpPath = jihad::RuntimeResolvePath(getenv("JIHAD_DUMP"), "frame.ppm");
    if (!dumpPath.empty() && nb > 0) {
      FILE* f = fopen(dumpPath.c_str(), "wb");
      if (f) {
        fprintf(f, "P6\n%d %d\n255\n", w, paintedH);     // buffer is BGRA -> write RGB
        for (long i = 0, px = (long)w * paintedH; i < px; i++) {
          unsigned char* p = pixels + i * 4; fputc(p[2], f); fputc(p[1], f); fputc(p[0], f);
        }
        fclose(f);
        // Pin 0644 rather than inherit the daemon's umask (T-057 (d): root-owned
        // 0755 dirs / 0644 files — never a world-writable artifact).
        chmod(dumpPath.c_str(), 0644);
        fprintf(stderr, "[jihad-bs] dumped frame -> %s\n", dumpPath.c_str());
      }
    }
  }
  if (nb < 0) return;
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
      unsigned char* row = pixels + (size_t)y * w * 4;
      for (int x = 0; x < w; x += 4) {
        unsigned char* p = row + (size_t)x * 4;
        if (!(p[0] > 240 && p[1] > 240 && p[2] > 240)) { nb = 1; break; }
      }
    }
  }
  if (nb == 0 && mHadContent) return;   // mNeedsPaint stays true -> retried until content lands
  if (nb > 0) mHadContent = true;

  mNeedsPaint = false;
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
  mActiveKey = (mActiveKey == mKey1 && mKey2) ? mKey2 : mKey1;
}

} // namespace jihad
