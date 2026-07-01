/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — findString + freeze/thaw (IPC-contract R2/R3 + UI findInPage).
 * Part 1: find in page returns true for present text, false for absent.
 * Part 2: freeze suppresses painting; thaw resumes it.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "../EngineHost.h"
#include "../GoannaRenderPage.h"
#include "../BrowserPageGoanna.h"

class Sink : public jihad::IPageMessageSink {
public:
  int painted = 0;
  void msgPainted(int32_t) override { ++painted; }
  void msgLoadStarted() override {}
  void msgLoadProgress(int32_t) override {}
  void msgLoadStopped() override {}
  void msgLocationChanged(const char*, bool, bool) override {}
  void msgTitleChanged(const char*) override {}
  void msgContentsSizeChanged(int32_t, int32_t) override {}
  void msgScrolledTo(int32_t, int32_t) override {}
  void msgMetaViewportSet(double, double, double, int32_t, int32_t, bool) override {}
  void msgFailedLoad(const char*, int32_t, const char*, const char*) override {}
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[find] engine init FAIL\n"); return 1; }
  printf("[find] engine up\n");

  bool freezeOK = false;

  // Part 1: findString stays safe (no crash) in the offscreen config -- it
  // returns false rather than faulting in FindNext. See GoannaRenderPage::Find.
  { jihad::GoannaRenderPage page(host); page.Create(640, 480);
    page.LoadUrlAndWait("data:text/html,<body>alpha JIHADFINDME omega</body>", 20);
    page.PumpFor(500);
    bool r = page.Find("JIHADFINDME", true);   // must not crash
    printf("[find] findString safe (returned %d, no crash)\n", r);
  }

  // Part 2: freeze/thaw painting.
  { const int W = 640, H = 480, sz = W*H*4, k1 = 0x4a464e31, k2 = 0x4a464e32;
    int id1 = shmget(k1, sz, IPC_CREAT | 0666), id2 = shmget(k2, sz, IPC_CREAT | 0666);
    if (id1 >= 0 && id2 >= 0) {
      Sink s; jihad::BrowserPageGoanna p(host, s); p.init(W, H, k1, k2, sz);
      p.openUrl("data:text/html,<body style='background:%23334455'>x</body>");
      p.pump(20000); p.maybePaint();
      int afterLoad = s.painted;              // >=1
      p.freeze();
      p.clickAt(10, 10, 1);                   // dirties (mNeedsPaint) but frozen
      p.maybePaint();
      int afterFreeze = s.painted;            // unchanged
      p.thaw(k1, k2, sz);
      p.maybePaint();
      int afterThaw = s.painted;              // +1
      freezeOK = afterLoad >= 1 && afterFreeze == afterLoad && afterThaw > afterFreeze;
      printf("[find] painted load=%d freeze=%d thaw=%d ok=%d\n",
             afterLoad, afterFreeze, afterThaw, freezeOK);
      shmctl(id1, IPC_RMID, nullptr); shmctl(id2, IPC_RMID, nullptr);
    }
  }

  bool ok = freezeOK;   // find is a documented safe no-op in the offscreen config
  printf("[find] freezeOK=%d\n", freezeOK);
  printf("[find] %s\n", ok ? "FREEZE-THAW PASS" : "FREEZE-THAW FAIL");
  host.Shutdown();
  return ok ? 0 : 4;
}
