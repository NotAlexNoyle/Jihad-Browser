/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — input coordinate mapping under zoom (domain E / R5). Through
 * the bridge (which maps adapter surface coords -> content coords), a click at
 * the SURFACE position of a target under 2x zoom must hit it. Without the
 * mapping the same surface click would miss (see coord_test).
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
  int lastKey = 0;
  void msgPainted(int32_t k) override { lastKey = k; }
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

// Read green pixel count directly from the shared buffer the daemon paints.
static long greenInShm(int key, int n) {
  int id = shmget(key, n*4, 0);
  if (id < 0) return -1;
  unsigned char* b = (unsigned char*)shmat(id, nullptr, SHM_RDONLY);
  if (b == (unsigned char*)-1) return -1;
  long c = 0;
  for (int i = 0; i < n; ++i) { unsigned char* q = b + (size_t)i*4;
    if (q[2] < 80 && q[1] >= 180 && q[0] < 80) ++c; }
  shmdt(b);
  return c;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[cmap] engine init FAIL\n"); return 1; }
  printf("[cmap] engine up\n");

  const int W = 800, H = 600, sz = W*H*4;
  const int k1 = 0x4a434d31, k2 = 0x4a434d32;
  int id1 = shmget(k1, sz, IPC_CREAT | 0666);
  int id2 = shmget(k2, sz, IPC_CREAT | 0666);
  if (id1 < 0 || id2 < 0) { perror("[cmap] shmget"); return 2; }

  int rc = 3;
  {
    Sink sink;
    jihad::BrowserPageGoanna page(host, sink);
    if (!page.init(W, H, k1, k2, sz)) { fprintf(stderr, "[cmap] init FAIL\n"); return 2; }

    // Box at content (100,100) 40x40; clicking it paints the body green.
    page.openUrl("data:text/html,<body style='margin:0;background:%230000ff'>"
                 "<div onclick=\"document.body.style.background='%2300ff00'\" "
                 "style='position:absolute;left:100px;top:100px;width:40px;height:40px;"
                 "background:%23ff0000'></div></body>");
    page.pump(20000);

    // Zoom 2x, no scroll. The box is displayed at surface (200..280).
    page.setZoomAndScroll(2.0, 0, 0);
    page.pump(1500);

    // A SURFACE click at (240,240) must map to content (120,120) and hit the box.
    page.clickAt(240, 240, 1);
    page.pump(1500);
    page.paintToSharedBuffer();               // paints into a buffer, emits msgPainted(key)
    long g = greenInShm(sink.lastKey, W*H);
    printf("[cmap] surface-click(240,240)@2x green=%ld hit=%d\n", g, g > 50000);

    bool ok = g > 50000;
    printf("[cmap] %s\n", ok ? "COORDMAP PASS" : "COORDMAP FAIL");
    rc = ok ? 0 : 4;
    shmctl(id1, IPC_RMID, nullptr);
    shmctl(id2, IPC_RMID, nullptr);
  }
  host.Shutdown();
  return rc;
}
