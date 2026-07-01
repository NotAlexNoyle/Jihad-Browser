/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — driver exercising the daemon-side BrowserPageGoanna bridge
 * (which drives GoannaRenderPage). Simulates the YAP command/message flow with
 * a printing IPageMessageSink: connect(init) -> openUrl -> pump -> paint.
 * This is exactly the sequence the real BrowserServer dispatch performs.
 *
 * Usage: xvfb-run page_driver <greDir>   ($JIHAD_URL overrides the page)
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>

#include "../EngineHost.h"
#include "../BrowserPageGoanna.h"

// Stand-in for the BrowserServer YAP forwarder: print each server->adapter msg.
class PrintingSink : public jihad::IPageMessageSink {
public:
  int painted = 0;
  void msgPainted(int32_t key) override { ++painted; printf("  -> msgPainted(key=0x%x)\n", (unsigned)key); }
  void msgLoadStarted() override { printf("  -> msgLoadStarted\n"); }
  void msgLoadProgress(int32_t p) override { printf("  -> msgLoadProgress(%d)\n", p); }
  void msgLoadStopped() override { printf("  -> msgLoadStopped\n"); }
  void msgLocationChanged(const char* uri, bool b, bool f) override {
    printf("  -> msgLocationChanged(%s, back=%d fwd=%d)\n", uri, b, f);
  }
  void msgTitleChanged(const char* t) override { printf("  -> msgTitleChanged(%s)\n", t ? t : ""); }
  void msgContentsSizeChanged(int32_t w, int32_t h) override { printf("  -> msgContentsSizeChanged(%d,%d)\n", w, h); }
  void msgScrolledTo(int32_t x, int32_t y) override { printf("  -> msgScrolledTo(%d,%d)\n", x, y); }
  void msgMetaViewportSet(double is, double mn, double mx, int32_t w, int32_t h, bool us) override {
    printf("  -> msgMetaViewportSet(is=%.2f min=%.2f max=%.2f %dx%d us=%d)\n", is, mn, mx, w, h, us);
  }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* url = getenv("JIHAD_URL");
  if (!url || !*url) url = "data:text/html,<title>Jihad</title>"
    "<body style='background:%23224488;color:white;font:48px sans-serif'>"
    "<h1>BrowserPageGoanna OK</h1></body>";

  gtk_init(&argc, &argv);

  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[driver] FAIL engine init\n"); return 1; }
  printf("[driver] engine up\n");

  {
    PrintingSink sink;
    jihad::BrowserPageGoanna page(host, sink);

    // YAP: connect(pageW,pageH, shmKey1, shmKey2, shmSize). Adapter-provided
    // keys are simulated here with two fixed SysV keys.
    const int W = 1024, H = 768, sz = W * H * 4;
    if (!page.init(W, H, /*key1*/0x4a494831, /*key2*/0x4a494832, sz)) {
      fprintf(stderr, "[driver] FAIL page.init\n"); return 2;
    }
    printf("[driver] page.init %dx%d (2 shared buffers)\n", W, H);

    printf("[driver] openUrl: %s\n", url);
    page.openUrl(url);                 // YAP: openUrl -> msgLoadStarted
    page.pump(20000);                  // event-loop ticks -> msgLoadStopped/Location
    page.pump(1500);                   // let it paint
    page.paintToSharedBuffer();        // YAP paint -> msgPainted(key)
    page.paintToSharedBuffer();        // second frame targets the other buffer

    printf("[driver] frames painted: %d\n", sink.painted);
    printf("[driver] %s\n", sink.painted >= 2 ? "DAEMON-BRIDGE PASS" : "DAEMON-BRIDGE FAIL");
    // page destructor tears down render page + shared buffers before engine shutdown.
  }

  host.Shutdown();
  printf("[driver] done\n");
  return 0;
}
