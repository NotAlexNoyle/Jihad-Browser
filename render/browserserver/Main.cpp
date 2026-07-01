/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — jihad-browserserver daemon entry point.
 * Brings up the Goanna engine, creates the JihadBrowserServer (real YAP socket
 * server), and pumps the engine + paint on the server's GLib main loop. A
 * BrowserAdapter connects over YAP exactly as it did to the QtWebKit daemon.
 */
#include <gtk/gtk.h>
#include <glib.h>
#include <cstdio>
#include <cstdlib>

#include "JihadBrowserServer.h"
#include "../goanna/EngineHost.h"

static JihadBrowserServer* g_server = nullptr;
static gboolean tick_cb(gpointer) { if (g_server) g_server->tick(); return TRUE; }

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* name = getenv("JIHAD_BS_NAME");
  if (!name || !*name) name = "jihad-browserserver";

  gtk_init(&argc, &argv);

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
