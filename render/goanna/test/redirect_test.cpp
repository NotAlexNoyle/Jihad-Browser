/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — redirect event test (domain F / R4). Loads /a on a local
 * server that 302-redirects to /b, and confirms the engine emits
 * msgUrlRedirected and the final location is /b. Requires redirect_server.py
 * (started by the build script). Base URL via $JIHAD_REDIR_BASE.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "../EngineHost.h"
#include "../BrowserPageGoanna.h"

class CapSink : public jihad::IPageMessageSink {
public:
  int redirected = 0, stopped = 0; std::string loc, redirUrl;
  void msgPainted(int32_t) override {}
  void msgLoadStarted() override {}
  void msgLoadProgress(int32_t) override {}
  void msgLoadStopped() override { ++stopped; }
  void msgLocationChanged(const char* u, bool, bool) override { loc = u ? u : ""; }
  void msgTitleChanged(const char*) override {}
  void msgContentsSizeChanged(int32_t, int32_t) override {}
  void msgScrolledTo(int32_t, int32_t) override {}
  void msgMetaViewportSet(double, double, double, int32_t, int32_t, bool) override {}
  void msgFailedLoad(const char*, int32_t, const char*, const char*) override {}
  void msgUrlRedirected(const char* u, const char*) override { ++redirected; redirUrl = u ? u : ""; }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* base = getenv("JIHAD_REDIR_BASE");
  if (!base || !*base) base = "http://127.0.0.1:18080";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[redir] engine init FAIL\n"); return 1; }
  printf("[redir] engine up (base=%s)\n", base);

  const int W = 640, H = 480, sz = W*H*4;
  const int k1 = 0x4a494751, k2 = 0x4a494752;
  int id1 = shmget(k1, sz, IPC_CREAT | 0666);
  int id2 = shmget(k2, sz, IPC_CREAT | 0666);
  if (id1 < 0 || id2 < 0) { perror("[redir] shmget"); return 2; }

  int rc = 3;
  {
    CapSink sink;
    jihad::BrowserPageGoanna page(host, sink);
    if (!page.init(W, H, k1, k2, sz)) { fprintf(stderr, "[redir] init FAIL\n"); return 2; }

    std::string url = std::string(base) + "/a";
    page.openUrl(url.c_str());
    page.pump(15000);

    printf("[redir] redirected=%d redirUrl=%s finalLoc=%s stopped=%d\n",
           sink.redirected, sink.redirUrl.c_str(), sink.loc.c_str(), sink.stopped);
    bool ok = sink.redirected >= 1 && sink.stopped >= 1 &&
              sink.loc.rfind("/b") != std::string::npos;
    printf("[redir] %s\n", ok ? "REDIRECT PASS" : "REDIRECT FAIL");
    rc = ok ? 0 : 4;
    shmctl(id1, IPC_RMID, nullptr);
    shmctl(id2, IPC_RMID, nullptr);
  }
  host.Shutdown();
  return rc;
}
