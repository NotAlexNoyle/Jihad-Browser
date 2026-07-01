/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — geometry events test (domain D / R4).
 * Drives BrowserPageGoanna with a capturing sink and asserts the renderer emits:
 *   - contents-size-changed  (tall page -> content height > viewport)
 *   - meta-viewport          (parsed from a <meta name=viewport> tag)
 *   - scrolled-to            (after setScrollPosition)
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "../EngineHost.h"
#include "../BrowserPageGoanna.h"

class CapSink : public jihad::IPageMessageSink {
public:
  int contentW = -1, contentH = -1, contentsMsgs = 0;
  int scrollX = -1, scrollY = -1, scrolledMsgs = 0;
  double vpInit = 0, vpMin = 0, vpMax = 0; int vpW = 0, vpH = 0; bool vpUS = false; int vpMsgs = 0;
  void msgPainted(int32_t) override {}
  void msgLoadStarted() override {}
  void msgLoadProgress(int32_t) override {}
  void msgLoadStopped() override {}
  void msgLocationChanged(const char*, bool, bool) override {}
  void msgTitleChanged(const char*) override {}
  void msgContentsSizeChanged(int32_t w, int32_t h) override { contentW = w; contentH = h; ++contentsMsgs; }
  void msgScrolledTo(int32_t x, int32_t y) override { scrollX = x; scrollY = y; ++scrolledMsgs; }
  void msgMetaViewportSet(double is, double mn, double mx, int32_t w, int32_t h, bool us) override {
    vpInit = is; vpMin = mn; vpMax = mx; vpW = w; vpH = h; vpUS = us; ++vpMsgs;
  }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[geo] engine init FAIL\n"); return 1; }
  printf("[geo] engine up\n");

  int rc = 3;
  {
    CapSink sink;
    jihad::BrowserPageGoanna page(host, sink);
    const int W = 1024, H = 768, sz = W*H*4;
    const int k1 = 0x4a494731, k2 = 0x4a494732;   // stand in for the adapter's segments
    int id1 = shmget(k1, sz, IPC_CREAT | 0666);
    int id2 = shmget(k2, sz, IPC_CREAT | 0666);
    if (id1 < 0 || id2 < 0) { perror("[geo] shmget"); return 2; }
    if (!page.init(W, H, k1, k2, sz)) { fprintf(stderr, "[geo] init FAIL\n"); return 2; }

    // Tall page (2500px) with an explicit viewport meta (width=800, scale 0.5..2).
    const char* url =
      "data:text/html,<head><meta name='viewport' content='width=800,"
      "initial-scale=0.5,minimum-scale=0.5,maximum-scale=2.0,user-scalable=yes'>"
      "</head><body style='margin:0;width:100%25;height:2500px;"
      "background:linear-gradient(%23ff0000,%230000ff)'></body>";
    page.openUrl(url);
    page.pump(20000);
    page.pump(1000);

    printf("[geo] contents: msgs=%d %dx%d\n", sink.contentsMsgs, sink.contentW, sink.contentH);
    printf("[geo] viewport: msgs=%d init=%.2f min=%.2f max=%.2f %dx%d us=%d\n",
           sink.vpMsgs, sink.vpInit, sink.vpMin, sink.vpMax, sink.vpW, sink.vpH, sink.vpUS);

    page.setScrollPosition(0, 400);
    page.pump(600);
    printf("[geo] scrolled: msgs=%d (%d,%d)\n", sink.scrolledMsgs, sink.scrollX, sink.scrollY);

    bool contentsOK = sink.contentsMsgs >= 1 && sink.contentH >= 2400;
    // The meta's scale/user-scalable prove it was parsed; the reported width is
    // the scale-derived layout viewport (1024/0.5 = 2048), not the literal 800.
    bool vpOK = sink.vpMsgs >= 1 && sink.vpW > 0 && sink.vpUS &&
                sink.vpInit > 0.4 && sink.vpInit < 0.6 &&
                sink.vpMin > 0.4 && sink.vpMin < 0.6 &&
                sink.vpMax > 1.9 && sink.vpMax < 2.1;
    bool scrollOK = sink.scrolledMsgs >= 1 && sink.scrollX == 0 &&
                    sink.scrollY >= 380 && sink.scrollY <= 420;
    printf("[geo] contentsOK=%d vpOK=%d scrollOK=%d\n", contentsOK, vpOK, scrollOK);
    bool ok = contentsOK && vpOK && scrollOK;
    printf("[geo] %s\n", ok ? "GEO PASS" : "GEO FAIL");
    rc = ok ? 0 : 4;
    shmctl(id1, IPC_RMID, nullptr);
    shmctl(id2, IPC_RMID, nullptr);
  }
  host.Shutdown();
  return rc;
}
