/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — jihad-browserserver daemon entry point.
 * Brings up the Goanna engine, creates the JihadBrowserServer (real YAP socket
 * server), and pumps the engine + paint on the server's GLib main loop. A
 * BrowserAdapter connects over YAP exactly as it did to the QtWebKit daemon.
 */
#ifndef JIHAD_OFFSCREEN_ONLY
#include <gtk/gtk.h>
#endif
#include <glib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <unistd.h>              // write/_exit/alarm in the teardown bailout
#include <sys/stat.h>            // fstat for the runtime log cap
#include <fcntl.h>
#include <string>

#include "JihadBrowserServer.h"
#include "../goanna/EngineHost.h"
#include "../goanna/JihadRuntimePaths.h"   // the ONE runtime-state dir (T-057 / R8)
#include "../goanna/JihadCrashReport.h"    // self-reporting fatal-signal dump
#include "JihadLunaService.h"              // this variant's own palm:// service (IPC R4)

static JihadBrowserServer* g_server = nullptr;

// ── clean termination ───────────────────────────────────────────────────────
// SIGTERM must NOT be left on its default action. Engine state is written by
// DEFERRED savers — the add-on database (XPIDatabase/DeferredSave), prefs.js
// (savePrefFile), permissions — which only flush when XPCOM shuts down. Default
// SIGTERM kills the process outright, so XRE_TermEmbedding never runs, nothing
// flushes, and the last changes are silently lost.
//
// That is not theoretical: measured 2026-08-04. Disabling an extension from
// about:addons ran its shutdown() (the UI half worked), the daemon was then
// killed and restarted, and the add-on came back ENABLED — userDisabled had
// never reached extensions.json. On device this is the NORMAL path, not an edge
// case: `stop jihad-browserserver-<variant>` is how upstart stops the job, and
// it sends SIGTERM.
//
// The handler only sets a flag. g_main_loop_quit is not async-signal-safe, so
// the 16 ms tick — already running — observes the flag and quits the loop; run()
// returns and the ordinary shutdown path (EngineHost dtor -> XRE_TermEmbedding)
// flushes everything.
static volatile sig_atomic_t g_termSignal = 0;
static void term_handler(int sig) { g_termSignal = sig; }

// Seconds the engine teardown gets before we stop waiting for it. Generous enough for a
// real flush (the add-on database and prefs are small), short enough that a wedged one
// never holds up an OS shutdown.
static const unsigned kTeardownDeadlineSec = 8;

// Installed ONLY around the teardown — see the long comment at the call site. Async-signal
// safe: one write, then _exit. Not exit(): no atexit handlers, no further I/O against a
// filesystem that is in the middle of going away.
static void teardown_bailout(int sig) {
  const char* m = (sig == SIGALRM)
    ? "[jihad-bs] teardown exceeded its deadline — leaving the rest to the on-disk journals\n"
    : "[jihad-bs] storage went away mid-teardown (expected at system shutdown) — exiting\n";
  ssize_t n = write(2, m, strlen(m)); (void)n;
  _exit(0);
}

// ── AN IDLE DAEMON IS IDLE (cavekit-device-build.md R8, 2026-10-01) ─────────────────────────
// The tick exists to pump and paint PAGES. With no card connected there is no page, yet the
// timer kept firing at the frame period: device-measured, ~120 wakeups a second and ~2.4% of a
// core per variant, for as long as the device was awake — three installed variants cost the
// battery ~7% of a core whether or not a browser had been opened since boot.
//
// So the period follows the state: the frame period while any card has a page, kJihadIdleTickMs
// while none does. The idle tick is not zero because two things still ride it — the SIGTERM
// check below (so `stop` is honoured within a second) and the development inject channel, which
// has to be able to answer "no page". Going idle is decided here, after the tick that saw the
// last page go; waking is NOT left to the next idle tick — the server calls jihadWakeTick() from
// the card's connect, so the first frame is not held up to a second behind the old timer.
static const int kJihadIdleTickMs = 1000;

// ── AND A CONNECTED CARD WITH NOTHING TO DO IS QUIET (cavekit-engine-embedding.md R3) ─────────
// Measured 2026-10-01 with a card open on a static page and no input: 12.3% of a core, all of it
// this tick — ~125 pump-and-bookkeeping passes a second at ~1 ms each on a CPU sitting at its
// idle clock. A browser left open on a page was costing an eighth of a core for as long as the
// card existed.
//
// So there is a third period between "frame" and "idle": after kJihadQuietAfterTicks consecutive
// ticks in which no page was busy and no card sent a command, the tick drops to the quiet
// period. It goes back to the frame period the moment either changes — a command wakes it
// directly from the server (the same hook a connect uses), page-side activity (a timer that
// touches the DOM, a load, media, a plugin) is seen by the next quiet tick, i.e. at most one
// quiet period late, and from then on it is fast again. Neither transition is logged: a page
// with a one-second clock would flip twice a second for ever.
//
// 100 ms, not 50: measured on the device, 50 ms left a static page at 3.5% of a core (each pass
// costs ~1.75 ms at the idle clock) and 100 ms brings it under 2%. The price is that a page-side
// event with no command behind it — a timer firing — is noticed up to 100 ms late, once, before
// the tick is fast again; a command is never late.
// $JIHAD_QUIET_TICK_MS overrides the quiet period (20-500); 0 disables the quiet tick.
static int jihadQuietTickMs() {
  static const int v = [] {
    int q = 100;
    if (const char* e = getenv("JIHAD_QUIET_TICK_MS")) {
      int n = atoi(e);
      if (n == 0) q = 0; else if (n >= 20 && n <= 500) q = n;
    }
    return q;
  }();
  return v;
}
static const int kJihadQuietAfterTicks = 25;   // ~0.2 s of the frame tick with nothing to do
static int g_quietTicks = 0;
static GMainContext* g_tickCtx = nullptr;
static GSource*      g_tickSrc = nullptr;
static int           g_tickFastMs = 8;
static int           g_tickArmedMs = 0;
static gboolean tick_cb(gpointer);

static void arm_tick(int ms) {
  if (g_tickSrc) { g_source_destroy(g_tickSrc); g_source_unref(g_tickSrc); }
  g_tickSrc = g_timeout_source_new(ms);
  g_source_set_callback(g_tickSrc, (GSourceFunc)tick_cb, nullptr, nullptr);
  g_source_attach(g_tickSrc, g_tickCtx);
  g_tickArmedMs = ms;
}

static void jihadWakeTick() {
  g_quietTicks = 0;
  // Already on a frame-class period (the page tick or the plugin tick): nothing to wake from.
  // Without the second half every card command would re-arm the page tick over the plugin tick
  // and the next tick would put it back — twice per frame, for every frame of a Flash page.
  if (g_tickArmedMs == g_tickFastMs || g_tickArmedMs == jihad::jihadPluginTickPeriodMs()) return;
  if (g_tickArmedMs == kJihadIdleTickMs)
    printf("[jihad-bs] tick: card connected — %d ms\n", g_tickFastMs);
  arm_tick(g_tickFastMs);
}

static gboolean tick_cb(gpointer) {
  if (g_termSignal) {
    if (g_server && g_server->mainLoop()) g_main_loop_quit(g_server->mainLoop());
    return FALSE;
  }
  if (g_server) g_server->tick();
  if (!g_server) return TRUE;
  if (!g_server->hasPages()) {
    if (g_tickArmedMs != kJihadIdleTickMs) {
      printf("[jihad-bs] tick: no card — idling at %d ms\n", kJihadIdleTickMs);
      arm_tick(kJihadIdleTickMs);   // destroys the source this callback is running in
      return FALSE;
    }
    return TRUE;
  }
  // The frame-class period for THIS state: the plugin tick while a plugin is on a page (see
  // jihadPluginTickPeriodMs — a plugin is paced on a grid of whole ticks and 8 ms is the wrong
  // grid), the page tick otherwise. A plugin page is always busy, so it never goes quiet.
  const int frameMs = g_server->tickHadPlugin() ? jihad::jihadPluginTickPeriodMs() : g_tickFastMs;
  if (g_server->tickWasBusy()) {
    g_quietTicks = 0;
    if (g_tickArmedMs != frameMs) { arm_tick(frameMs); return FALSE; }
  } else if ((g_tickArmedMs == g_tickFastMs || g_tickArmedMs == jihad::jihadPluginTickPeriodMs()) &&
             jihadQuietTickMs() > 0 &&
             ++g_quietTicks >= kJihadQuietAfterTicks) {
    arm_tick(jihadQuietTickMs());
    return FALSE;
  }
  return TRUE;
}

// ── THE LOG IS BOUNDED WHILE THE DAEMON RUNS (cavekit-device-build.md R8, 2026-10-01) ───────
// The upstart job caps daemon.log at 2 MB — but only at job START, and a daemon stays up for
// days. Device-measured: two idle daemons had written 10 MB each in 3.6 days on a 62 MB /var
// shared with db8 and the Luna prefs. A full /var makes every write fail with ENOSPC silently,
// the log freezes, and a frozen log reads exactly like a live one.
//
// Copy-then-truncate, not rename-and-reopen: stdout/stderr were opened by the job's shell with
// `>>` (O_APPEND) and are INHERITED by plugin-container, so a renamed file would keep growing
// under its new name through every descriptor we do not own. Truncating the one file every
// writer shares moves them all back to offset 0. One kept generation, so the worst case is
// 2 x cap per variant — 12 MB across three with the default cap, as the job's comment promises.
// $JIHAD_LOG_MAX (bytes, 64 KB-16 MB) lowers the cap so the rotation can be exercised on a device
// in a minute instead of a day.
static off_t jihadLogCap() {
  long v = 2 * 1024 * 1024;
  if (const char* e = getenv("JIHAD_LOG_MAX")) {
    long n = atol(e);
    if (n >= 64 * 1024 && n <= 16 * 1024 * 1024) v = n;
  }
  return (off_t)v;
}

static gboolean log_cap_cb(gpointer) {
  static const off_t cap = jihadLogCap();
  struct stat st;
  if (fstat(2, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size < cap) return TRUE;

  char path[512];
  ssize_t n = readlink("/proc/self/fd/2", path, sizeof path - 1);
  bool kept = false;
  if (n > 0) {
    path[n] = 0;
    const std::string old = std::string(path) + ".old";
    unlink(old.c_str());                       // free the previous generation before copying
    int in = open(path, O_RDONLY);
    int out = open(old.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (in >= 0 && out >= 0) {
      char buf[65536];
      off_t left = st.st_size;
      kept = true;
      while (left > 0) {
        ssize_t r = read(in, buf, left < (off_t)sizeof buf ? (size_t)left : sizeof buf);
        if (r <= 0) break;
        if (write(out, buf, (size_t)r) != r) { kept = false; break; }   // ENOSPC: drop the copy
        left -= r;
      }
    }
    if (in >= 0) close(in);
    if (out >= 0) close(out);
    if (!kept) unlink(old.c_str());            // never leave a partial generation eating /var
  }
  // Bounded matters more than kept: truncate even if the copy failed.
  if (ftruncate(2, 0) == 0 && !(fcntl(2, F_GETFL) & O_APPEND)) lseek(2, 0, SEEK_SET);
  fprintf(stderr, "[jihad-bs] log reached %ld bytes (cap %ld) — rotated in place%s\n",
          (long)st.st_size, (long)cap, kept ? ", previous kept as .old" : ", previous NOT kept");
  return TRUE;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  // Before anything else: a daemon that dies during bring-up must say where.
  // Re-armed after engine init (EngineHost::Init) because SpiderMonkey replaces
  // SIGSEGV during startup — see JihadCrashReport.h.
  jihad::JihadInstallCrashHandler();
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* name = getenv("JIHAD_BS_NAME");
  if (!name || !*name) name = "jihad-browserserver";

  // Resolve (and create) the variant's runtime state dir BEFORE the engine comes
  // up — EngineHost::Init puts the profile inside it. One line of provenance in
  // the log so a device session can see which variant this process believes it is
  // and where its state actually landed; the upstart job's `exec >> …` log belongs
  // in that same directory (plan-variant-identity.md: /var/palm/jihad/$V/daemon.log)
  // and NEVER on /media/internal (R8). Empty = no writable state (degraded, not fatal).
  const std::string& state = jihad::RuntimeStateDir();
  printf("[jihad-bs] variant=%s (JIHAD_BS_NAME=%s) state=%s\n",
         jihad::RuntimeVariant(), name, state.empty() ? "(none)" : state.c_str());

  // ── $HOME MUST BE SET BEFORE THE ENGINE STARTS ──────────────────────────────
  // This is not hygiene, it is a hard crash fix. UXP reads $HOME through
  // `nsDependentCString(PR_GetEnv("HOME"))` in at least two places with NO null
  // check — xpcom/io/SpecialSystemDirectory.cpp:189 (GetUnixHomeDir) and
  // xpcom/io/nsAppFileLocationProvider.cpp:318 — so an unset HOME is
  // `strlen(NULL)` and an instant SIGSEGV at address 0.
  //
  // An upstart job inherits init's environment, which on webOS 3 has no HOME. The
  // container harness always runs with `-e HOME=/out`, which is exactly why this
  // reproduced 100% on device and never once on the desktop. Diagnosed 2026-08-01
  // from the daemon's own fault report: faultaddr=0x0, lr in
  // nsDependentCString(char const*), pc outside every libxul segment (i.e. inside
  // libc's strlen).
  //
  // Point it at the variant's runtime state dir: root-owned 0755, variant-scoped,
  // and removed by that variant's prerm — so anything Gecko puts under $HOME
  // (~/.cache and friends) stays inside our own R8 footprint instead of landing
  // somewhere nobody cleans up. `setenv(..., 0)` never overwrites a HOME that was
  // deliberately provided (the desktop harness, an ad-hoc novacom run).
  if (!getenv("HOME")) {
    const char* home = state.empty() ? "/tmp" : state.c_str();
    setenv("HOME", home, 0);
    printf("[jihad-bs] HOME was unset — set to %s (UXP dereferences it unguarded)\n", home);
  }

  // JIHAD_OFFSCREEN (on-device / headless): there is no X server. gtk_init() calls
  // exit(1) if it cannot open a display; gtk_init_check() returns FALSE instead.
  // The daemon drives the engine on a GLib main loop (not GTK), and rendering goes
  // through the PuppetWidget offscreen path — no GTK widgets/X needed.
#ifndef JIHAD_OFFSCREEN_ONLY
  if (getenv("JIHAD_OFFSCREEN")) {
    gtk_init_check(&argc, &argv);
  } else {
    gtk_init(&argc, &argv);
  }
#endif

  static jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[jihad-bs] engine init FAILED\n"); return 1; }

  // Scoped: the server owns every BrowserPage (and therefore every nsIWebBrowser), and
  // XRE_TermEmbedding tears down the process-wide runtime — running it while a page is
  // still live is the hazard EngineHost::Shutdown warns about. Letting `server` go out of
  // scope here destroys the pages FIRST. Not hypothetical: with a card still connected,
  // shutting down in the other order SIGSEGV'd inside the engine teardown (device,
  // 2026-08-04, faultaddr=0x150) — and a stop with the browser open is the ordinary case,
  // not an edge one.
  {
  JihadBrowserServer server(name, host);
  g_server = &server;

  // Pump Goanna's event queue + paint on the server's GLib context (~60 Hz).
  GMainContext* ctx = server.mainLoop() ? g_main_loop_get_context(server.mainLoop()) : nullptr;
  // TICK PERIOD. A frame can only be published on a tick, so this is the quantiser on every
  // frame gap: a plugin frame that arrives just after maybePaint() waits a whole period, which
  // is why an evenly-produced 26 ms stream still reached the card uneven. 16 ms was chosen when
  // the tick cost 12 ms of pump per iteration; PumpReady removed that (device-measured: work
  // avg 12 ms -> 1 ms idle), so the budget for a shorter period now exists. 8 ms halves the
  // worst-case wait. Env-tunable because the right value depends on what else the device is
  // doing, and clamped so a typo cannot spin the daemon or stall it.
  // One definition, in BrowserPageGoanna.cpp: the plugin pull expresses its request period in
  // whole ticks, so a second env read here could drift from the one the pacer uses.
  int tickMs = jihad::jihadTickPeriodMs();
  printf("[jihad-bs] tick period %d ms with a card, %d ms idle\n", tickMs, kJihadIdleTickMs);
  g_tickCtx = ctx; g_tickFastMs = tickMs;
  server.setWakeHook(jihadWakeTick);
  arm_tick(tickMs);       // starts fast; the first tick drops to idle if no card has connected

  // Log cap: checked every 30 s, on the same loop. At the worst rate ever measured (2.4 KB/s)
  // that is an overshoot of ~70 KB past the cap.
  GSource* capSrc = g_timeout_source_new_seconds(30);
  g_source_set_callback(capSrc, log_cap_cb, nullptr, nullptr);
  g_source_attach(capSrc, ctx);
  g_source_unref(capSrc);

  // Installed AFTER engine init: EngineHost::Init re-arms the fatal-signal
  // handlers, and this must sit on top of whatever it leaves behind. SA_RESTART
  // so a signal never turns a socket read into a spurious EINTR failure.
  struct sigaction sa;
  memset(&sa, 0, sizeof sa);
  sa.sa_handler = term_handler;
  sa.sa_flags = SA_RESTART;
  sigaction(SIGTERM, &sa, nullptr);
  sigaction(SIGINT, &sa, nullptr);
  sigaction(SIGHUP, &sa, nullptr);

  // This variant's OWN Luna service. Started AFTER the server exists (it needs the same
  // GLib loop) and treated as optional: no bus, no library, or no role file means no service
  // and a daemon that still runs. See JihadLunaService.h for why the name is not
  // com.palm.browserServer.
  // The YAP name is the app id's last component in every variant (jihad-browser,
  // jihad-browser-mochi, jihad-browser-mojo), so the card's app id follows from it.
  jihad::LunaServiceSetOwnAppId(std::string("net.riverstonerelay.") + name);
  static JihadBrowserServer* sThumbServer = nullptr;
  sThumbServer = &server;
  jihad::LunaServiceSetThumbnailFn([](const std::string& path) -> bool {
    return sThumbServer && sThumbServer->thumbnailForActivePage(path);
  });
  jihad::LunaServiceStart(jihad::LunaServiceNameFor(name), server.mainLoop());

  printf("[jihad-bs] engine up; serving YAP '%s' (waiting for BrowserAdapter)\n", name);
  server.run();   // GLib main loop (patched YapServer::run)
  if (g_termSignal)
    printf("[jihad-bs] signal %d — shutting the engine down cleanly\n", (int)g_termSignal);
  jihad::LunaServiceStop();
  g_server = nullptr;
  }   // every page destroyed here, before the runtime goes away
  // Explicit, not left to static destruction order: this is what flushes the
  // add-on database and prefs. `host` is function-static, so its dtor would run
  // eventually, but Shutdown() is idempotent (mInited) and doing it here keeps
  // the flush inside the part of the program that can still report a problem.
  //
  // ── BOUNDED, AND SURVIVABLE. This is the shutdown that bricked the device. ──
  // Our SIGTERM handling is right for `stop jihad`, where flushing is the whole
  // point. It is DANGEROUS at system shutdown, and the job cannot tell the two
  // apart: `stop on started start_update` is what every cryptofs-dependent job
  // on this platform uses, the stock browserserver included — so at reboot we
  // are killed by the late generic sweep, at the same moment cryptofs is being
  // torn down. The engine profile LIVES on cryptofs, so XRE_TermEmbedding is
  // writing and mmap-reading files whose backing store is disappearing under it.
  //
  // Measured 2026-08-05, from the device's own log, as the system went down:
  //     minicore_launch: CRASH! ld-2.23.so(1947) received 7
  //     minicore_launch: CRASH! ld-2.23.so(1950) received 7
  // Signal 7 is SIGBUS — the classic result of touching an mmap'd region whose
  // file went away. Two of the three daemons took it, each then writing a
  // minicore dump to /var while /var was being unmounted. The next boot found
  // /var with an unrecovered journal and never came up: no syslog, no
  // LunaSysMgr, novacom silent. Recovery needed bootie plus a doctor image.
  // The stock BrowserServer never had this exposure because it simply dies.
  //
  // So: keep the flush, bound it, and never let it turn into a fatal signal.
  // SIGBUS/SIGSEGV during teardown means the storage went away mid-flush, which
  // at this point is EXPECTED rather than exceptional — the profile's own stores
  // (sqlite, prefs.js via an atomic rename) are crash-safe by design, so the
  // right answer is to stop touching them and leave quietly. _exit avoids
  // atexit handlers and any further I/O.
  {
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = teardown_bailout;
    sigaction(SIGBUS,  &sa, nullptr);
    sigaction(SIGSEGV, &sa, nullptr);
    signal(SIGALRM, teardown_bailout);
    alarm(kTeardownDeadlineSec);   // a flush that cannot finish must not hold up the OS
  }
  host.Shutdown();
  alarm(0);
  printf("[jihad-bs] exited\n");
  return 0;
}
