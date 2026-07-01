/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — URL redirect rules test (domain F / R6, addUrlRedirect).
 * A redirect rule for "^tel:" must hand a tel: URL to the client
 * (msgUrlRedirected with its userData) and NOT load it; an ordinary URL with no
 * matching rule loads normally.
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
  int started = 0, redir = 0; std::string ru, rud;
  void msgPainted(int32_t) override {}
  void msgLoadStarted() override { ++started; }
  void msgLoadProgress(int32_t) override {}
  void msgLoadStopped() override {}
  void msgLocationChanged(const char*, bool, bool) override {}
  void msgTitleChanged(const char*) override {}
  void msgContentsSizeChanged(int32_t, int32_t) override {}
  void msgScrolledTo(int32_t, int32_t) override {}
  void msgMetaViewportSet(double, double, double, int32_t, int32_t, bool) override {}
  void msgFailedLoad(const char*, int32_t, const char*, const char*) override {}
  void msgUrlRedirected(const char* u, const char* ud) override { ++redir; ru = u?u:""; rud = ud?ud:""; }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[rules] engine init FAIL\n"); return 1; }
  printf("[rules] engine up\n");

  const int W = 640, H = 480, sz = W*H*4;
  const int k1 = 0x4a495231, k2 = 0x4a495232;
  int id1 = shmget(k1, sz, IPC_CREAT | 0666);
  int id2 = shmget(k2, sz, IPC_CREAT | 0666);
  if (id1 < 0 || id2 < 0) { perror("[rules] shmget"); return 2; }

  int rc = 3;
  {
    CapSink sink;
    jihad::BrowserPageGoanna page(host, sink);
    if (!page.init(W, H, k1, k2, sz)) { fprintf(stderr, "[rules] init FAIL\n"); return 2; }

    page.addUrlRedirect("^tel:", 0, /*redirect*/true, "dial");

    // 1) tel: matches the redirect rule -> handed to client, not loaded.
    page.openUrl("tel:12345");
    page.pump(500);
    printf("[rules] tel: redir=%d ru=%s rud=%s started=%d\n", sink.redir, sink.ru.c_str(), sink.rud.c_str(), sink.started);
    bool redirOK = sink.redir == 1 && sink.ru == "tel:12345" && sink.rud == "dial" && sink.started == 0;

    // 2) an ordinary URL with no matching rule loads normally.
    sink.redir = 0; sink.started = 0;
    page.openUrl("data:text/html,<body>ok</body>");
    page.pump(3000);
    printf("[rules] normal: redir=%d started=%d\n", sink.redir, sink.started);
    bool normalOK = sink.redir == 0 && sink.started == 1;

    printf("[rules] redirOK=%d normalOK=%d\n", redirOK, normalOK);
    bool ok = redirOK && normalOK;
    printf("[rules] %s\n", ok ? "RULES PASS" : "RULES FAIL");
    rc = ok ? 0 : 4;
    shmctl(id1, IPC_RMID, nullptr);
    shmctl(id2, IPC_RMID, nullptr);
  }
  host.Shutdown();
  return rc;
}
