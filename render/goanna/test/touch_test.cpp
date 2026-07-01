/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — touch synthesis test (domain E / R3).
 * ontouchstart turns the blue background green; synthesize a touchstart at the
 * center and confirm the render went blue -> green. Requires
 * dom.w3c_touch_events.enabled (set by the build script). Run under Xvfb.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

static long countColor(unsigned char* buf, int n,
                       int rmn,int rmx,int gmn,int gmx,int bmn,int bmx) {
  long c = 0;
  for (int i = 0; i < n; ++i) { unsigned char* p = buf + (size_t)i*4;
    int b=p[0], g=p[1], r=p[2];
    if (r>=rmn&&r<=rmx && g>=gmn&&g<=gmx && b>=bmn&&b<=bmx) ++c; }
  return c;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[touch] engine init FAIL\n"); return 1; }
  printf("[touch] engine up\n");

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) return 2;
    const char* url = "data:text/html,<body ontouchstart=\"document.body.style.background='%2300ff00'\" "
                      "style='margin:0;width:100vw;height:100vh;background:%230000ff'></body>";
    page.LoadUrlAndWait(url, 20);
    page.PumpFor(1000);

    const int N = page.Width() * page.Height();
    size_t sz = (size_t)N * 4;
    unsigned char* buf = (unsigned char*)malloc(sz);

    printf("[touch] touchstart at (512,384)\n");
    page.TouchEvent("touchstart", 512, 384);
    page.TouchEvent("touchend", 512, 384);
    page.PumpFor(2000);

    page.ReadPixels(buf, sz);
    long green = countColor(buf, N, 0,80, 180,255, 0,80);
    printf("[touch] after touch: green=%ld\n", green);
    bool ok = green > 100000;
    printf("[touch] %s (touch %s reach the DOM)\n", ok ? "TOUCH PASS" : "TOUCH FAIL", ok ? "DID" : "did NOT");
    rc = ok ? 0 : 4;
    free(buf);
  }
  host.Shutdown();
  return rc;
}
