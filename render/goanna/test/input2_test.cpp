/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — input extras (domain E): holdAt (R1 long-press/context menu),
 * dragProcess (R4 drag scrolling), insertStringAtCursor (R2). Driven through the
 * bridge. PASS requires holdAt + drag; insertStringAtCursor is reported.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "../EngineHost.h"
#include "../BrowserPageGoanna.h"

class Sink : public jihad::IPageMessageSink {
public:
  int lastKey = 0, scrolledMsgs = 0, sy = -1;
  void msgPainted(int32_t k) override { lastKey = k; }
  void msgLoadStarted() override {}
  void msgLoadProgress(int32_t) override {}
  void msgLoadStopped() override {}
  void msgLocationChanged(const char*, bool, bool) override {}
  void msgTitleChanged(const char*) override {}
  void msgContentsSizeChanged(int32_t, int32_t) override {}
  void msgScrolledTo(int32_t, int32_t y) override { ++scrolledMsgs; sy = y; }
  void msgMetaViewportSet(double, double, double, int32_t, int32_t, bool) override {}
  void msgFailedLoad(const char*, int32_t, const char*, const char*) override {}
};

static long greenInShm(int key, int n) {
  int id = shmget(key, n*4, 0); if (id < 0) return -1;
  unsigned char* b = (unsigned char*)shmat(id, nullptr, SHM_RDONLY);
  if (b == (unsigned char*)-1) return -1;
  long c = 0;
  for (int i = 0; i < n; ++i) { unsigned char* q = b + (size_t)i*4;
    if (q[2] < 80 && q[1] >= 180 && q[0] < 80) ++c; }
  shmdt(b); return c;
}

static const int W = 800, H = 600, KSZ = 800*600*4;
static int gk1 = 0x4a493231, gk2 = 0x4a493232;

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[in2] engine init FAIL\n"); return 1; }
  printf("[in2] engine up\n");
  int id1 = shmget(gk1, KSZ, IPC_CREAT | 0666), id2 = shmget(gk2, KSZ, IPC_CREAT | 0666);
  if (id1 < 0 || id2 < 0) { perror("[in2] shmget"); return 2; }

  bool holdOK = false, dragOK = false, insertOK = false;

  // --- holdAt: oncontextmenu paints the body green ---
  { Sink s; jihad::BrowserPageGoanna p(host, s); p.init(W, H, gk1, gk2, KSZ);
    p.openUrl("data:text/html,<body oncontextmenu=\"document.body.style.background='%2300ff00';return false\" "
              "style='margin:0;width:100vw;height:100vh;background:%230000ff'></body>");
    p.pump(20000); p.holdAt(400, 300); p.pump(1500); p.paintToSharedBuffer();
    long g = greenInShm(s.lastKey, W*H); holdOK = g > 100000;
    printf("[in2] holdAt green=%ld hit=%d\n", g, holdOK);
  }

  // --- dragProcess: drag up scrolls the tall page down ~200 ---
  { Sink s; jihad::BrowserPageGoanna p(host, s); p.init(W, H, gk1, gk2, KSZ);
    p.openUrl("data:text/html,<body style='margin:0;width:100vw;height:3000px;"
              "background:linear-gradient(%23ff0000,%230000ff)'></body>");
    p.pump(20000); p.dragStart(400, 300); p.dragProcess(0, -200); p.dragEnd(400, 100);
    p.pump(1000);
    dragOK = s.scrolledMsgs >= 1 && s.sy >= 150 && s.sy <= 250;
    printf("[in2] drag scrolledMsgs=%d sy=%d ok=%d\n", s.scrolledMsgs, s.sy, dragOK);
  }

  // --- insertStringAtCursor (best-effort): autofocus input, oninput -> green ---
  { Sink s; jihad::BrowserPageGoanna p(host, s); p.init(W, H, gk1, gk2, KSZ);
    p.openUrl("data:text/html,<body style='margin:0;background:%230000ff'>"
              "<input autofocus oninput=\"if(this.value=='hi')"
              "document.body.style.background='%2300ff00'\"></body>");
    p.pump(20000);
    p.clickAt(20, 12, 1);   // focus the input first
    p.pump(500);
    p.insertStringAtCursor("hi"); p.pump(1500); p.paintToSharedBuffer();
    long g = greenInShm(s.lastKey, W*H); insertOK = g > 50000;
    printf("[in2] insertStringAtCursor green=%ld inserted=%d (best-effort)\n", g, insertOK);
  }

  bool ok = holdOK && dragOK;   // insertStringAtCursor reported, not gating
  printf("[in2] holdOK=%d dragOK=%d insertOK=%d\n", holdOK, dragOK, insertOK);
  printf("[in2] %s\n", ok ? "INPUT2 PASS" : "INPUT2 FAIL");
  shmctl(id1, IPC_RMID, nullptr); shmctl(id2, IPC_RMID, nullptr);
  host.Shutdown();
  return ok ? 0 : 4;
}
