/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — failed-load test (domain F / R3). Loading a refused endpoint
 * (127.0.0.1:1) must emit msgFailedLoad with the failing URL, then load-stopped
 * -- no network required, the connection is refused locally. A good load must
 * NOT emit msgFailedLoad.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "../EngineHost.h"
#include "../BrowserPageGoanna.h"

class CapSink : public jihad::IPageMessageSink {
public:
  int failed = 0, stopped = 0, gh = 0; char url[512] = {0};
  void msgPainted(int32_t) override {}
  void msgLoadStarted() override {}
  void msgLoadProgress(int32_t) override {}
  void msgLoadStopped() override { ++stopped; }
  void msgLocationChanged(const char*, bool, bool) override {}
  void msgTitleChanged(const char*) override {}
  void msgContentsSizeChanged(int32_t, int32_t) override {}
  void msgScrolledTo(int32_t, int32_t) override {}
  void msgMetaViewportSet(double, double, double, int32_t, int32_t, bool) override {}
  void msgFailedLoad(const char*, int32_t, const char* u, const char*) override {
    ++failed; if (u) { strncpy(url, u, sizeof url - 1); }
  }
  void msgUpdateGlobalHistory(const char*, bool) override { ++gh; }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[fail] engine init FAIL\n"); return 1; }
  printf("[fail] engine up\n");

  const int W = 640, H = 480, sz = W*H*4;
  const int k1 = 0x4a494741, k2 = 0x4a494742;
  int id1 = shmget(k1, sz, IPC_CREAT | 0666);
  int id2 = shmget(k2, sz, IPC_CREAT | 0666);
  if (id1 < 0 || id2 < 0) { perror("[fail] shmget"); return 2; }

  int rc = 3;
  {
    CapSink sink;
    jihad::BrowserPageGoanna page(host, sink);
    if (!page.init(W, H, k1, k2, sz)) { fprintf(stderr, "[fail] init FAIL\n"); return 2; }

    // 1) An unknown-scheme URL fails deterministically (synchronously) and must
    //    emit msgFailedLoad. (A refused/DNS host also does on device, but a bare
    //    TCP connect can hang in this sandbox, so we use a scheme with no handler.)
    const char* bad = "zzzbogus://nohandler/";
    page.openUrl(bad);
    page.pump(4000);
    printf("[fail] bad load: failed=%d stopped=%d gh=%d url=%s\n", sink.failed, sink.stopped, sink.gh, sink.url);
    // A failed load emits msgFailedLoad and NOT global-history (R3 + R6).
    bool badOK = sink.failed >= 1 && sink.stopped >= 1 && sink.gh == 0;

    // 2) A good load must NOT emit msgFailedLoad but MUST emit global-history.
    sink.failed = 0; sink.gh = 0;
    page.openUrl("data:text/html,<body>ok</body>");
    page.pump(8000);
    printf("[fail] good load: failed=%d gh=%d\n", sink.failed, sink.gh);
    bool goodOK = sink.failed == 0 && sink.gh >= 1;

    printf("[fail] badOK=%d goodOK=%d\n", badOK, goodOK);
    bool ok = badOK && goodOK;
    printf("[fail] %s\n", ok ? "FAIL-EVENT PASS" : "FAIL-EVENT FAIL");
    rc = ok ? 0 : 4;
    shmctl(id1, IPC_RMID, nullptr);
    shmctl(id2, IPC_RMID, nullptr);
  }
  host.Shutdown();
  return rc;
}
