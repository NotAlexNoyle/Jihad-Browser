/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — full-page zoom test (domain D / R5, setZoomAndScroll).
 * A fixed 100x100 red box on white. At zoom 1.0 it covers ~10000 px; at zoom
 * 3.0 it must cover ~9x that, proving full-page zoom scales the rendering.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

static long countRed(unsigned char* buf, int n) {
  long c = 0;
  for (int i = 0; i < n; ++i) { unsigned char* p = buf + (size_t)i*4;
    int b=p[0], g=p[1], r=p[2];
    if (r>=180 && g<80 && b<80) ++c; }
  return c;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[zoom] engine init FAIL\n"); return 1; }
  printf("[zoom] engine up\n");

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) return 2;
    const char* url = "data:text/html,<body style='margin:0;background:white'>"
                      "<div style='width:100px;height:100px;background:%23ff0000'></div></body>";
    page.LoadUrlAndWait(url, 20);
    page.PumpFor(1000);

    const int N = page.Width() * page.Height();
    size_t sz = (size_t)N * 4;
    unsigned char* buf = (unsigned char*)malloc(sz);

    page.ReadPixels(buf, sz);
    long red1 = countRed(buf, N);
    printf("[zoom] zoom=1.0 red=%ld\n", red1);

    page.SetZoom(3.0);
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    long red3 = countRed(buf, N);
    printf("[zoom] zoom=3.0 red=%ld\n", red3);
    free(buf);

    // ~100x100=10000 at 1x; ~300x300=90000 at 3x. Allow generous tolerance.
    bool ok = red1 > 6000 && red1 < 15000 && red3 > 60000 && red3 < 120000;
    printf("[zoom] %s\n", ok ? "ZOOM PASS" : "ZOOM FAIL");
    rc = ok ? 0 : 4;
  }
  host.Shutdown();
  return rc;
}
