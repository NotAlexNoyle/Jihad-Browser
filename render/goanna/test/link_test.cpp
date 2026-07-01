/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — link-clicked test (domain F / R6). Loads /link (a full-
 * viewport anchor to /b), clicks the centre, and confirms the content-initiated
 * navigation is reported via msgLinkClicked (target /b). Base via $JIHAD_REDIR_BASE.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "../EngineHost.h"
#include "../BrowserPageGoanna.h"

class CapSink : public jihad::IPageMessageSink {
public:
  int links = 0; std::string linkUrl;
  void msgPainted(int32_t) override {}
  void msgLoadStarted() override {}
  void msgLoadProgress(int32_t) override {}
  void msgLoadStopped() override {}
  void msgLocationChanged(const char*, bool, bool) override {}
  void msgTitleChanged(const char*) override {}
  void msgContentsSizeChanged(int32_t, int32_t) override {}
  void msgScrolledTo(int32_t, int32_t) override {}
  void msgMetaViewportSet(double, double, double, int32_t, int32_t, bool) override {}
  void msgFailedLoad(const char*, int32_t, const char*, const char*) override {}
  void msgLinkClicked(const char* u) override { ++links; linkUrl = u ? u : ""; }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* base = getenv("JIHAD_REDIR_BASE");
  if (!base || !*base) base = "http://127.0.0.1:18080";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[link] engine init FAIL\n"); return 1; }
  printf("[link] engine up (base=%s)\n", base);

  const int W = 800, H = 600, sz = W*H*4;
  const int k1 = 0x4a494c31, k2 = 0x4a494c32;
  int id1 = shmget(k1, sz, IPC_CREAT | 0666);
  int id2 = shmget(k2, sz, IPC_CREAT | 0666);
  if (id1 < 0 || id2 < 0) { perror("[link] shmget"); return 2; }

  int rc = 3;
  {
    CapSink sink;
    jihad::BrowserPageGoanna page(host, sink);
    if (!page.init(W, H, k1, k2, sz)) { fprintf(stderr, "[link] init FAIL\n"); return 2; }

    std::string url = std::string(base) + "/link";
    page.openUrl(url.c_str());
    page.pump(15000);   // load /link (command-initiated, not a link)

    // Click the centre: the anchor fills the viewport -> navigates to /b.
    page.clickAt(W/2, H/2, 1);
    page.pump(8000);    // link navigation happens + is reported via pump

    printf("[link] links=%d linkUrl=%s\n", sink.links, sink.linkUrl.c_str());
    bool ok = sink.links >= 1 && sink.linkUrl.find("/b") != std::string::npos;
    printf("[link] %s\n", ok ? "LINK PASS" : "LINK FAIL");
    rc = ok ? 0 : 4;
    shmctl(id1, IPC_RMID, nullptr);
    shmctl(id2, IPC_RMID, nullptr);
  }
  host.Shutdown();
  return rc;
}
