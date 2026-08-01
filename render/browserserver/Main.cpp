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

#include "JihadBrowserServer.h"
#include "../goanna/EngineHost.h"
#include "../goanna/JihadRuntimePaths.h"   // the ONE runtime-state dir (T-057 / R8)
#include "../goanna/JihadCrashReport.h"    // self-reporting fatal-signal dump

static JihadBrowserServer* g_server = nullptr;
static gboolean tick_cb(gpointer) { if (g_server) g_server->tick(); return TRUE; }

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

  JihadBrowserServer server(name, host);
  g_server = &server;

  // Pump Goanna's event queue + paint on the server's GLib context (~60 Hz).
  GMainContext* ctx = server.mainLoop() ? g_main_loop_get_context(server.mainLoop()) : nullptr;
  GSource* src = g_timeout_source_new(16);
  g_source_set_callback(src, (GSourceFunc)tick_cb, nullptr, nullptr);
  g_source_attach(src, ctx);
  g_source_unref(src);

  printf("[jihad-bs] engine up; serving YAP '%s' (waiting for BrowserAdapter)\n", name);
  server.run();   // GLib main loop (patched YapServer::run)
  printf("[jihad-bs] exited\n");
  return 0;
}
