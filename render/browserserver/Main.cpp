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

static gboolean tick_cb(gpointer) {
  if (g_termSignal) {
    if (g_server && g_server->mainLoop()) g_main_loop_quit(g_server->mainLoop());
    return FALSE;
  }
  if (g_server) g_server->tick();
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
  printf("[jihad-bs] tick period %d ms\n", tickMs);
  GSource* src = g_timeout_source_new(tickMs);
  g_source_set_callback(src, (GSourceFunc)tick_cb, nullptr, nullptr);
  g_source_attach(src, ctx);
  g_source_unref(src);

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
