/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — surface resize test (domain D / R5, setWindowSize).
 * A full-viewport green page (100vw x 100vh) fills the surface. After resizing
 * 1024x768 -> 640x480 the green area must shrink to the new surface, proving the
 * surface resized and the content reflowed to the new viewport.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

static long countGreen(unsigned char* buf, int n) {
  long c = 0;
  for (int i = 0; i < n; ++i) { unsigned char* p = buf + (size_t)i*4;
    int b=p[0], g=p[1], r=p[2];
    if (r<80 && g>=180 && b<80) ++c; }
  return c;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[resize] engine init FAIL\n"); return 1; }
  printf("[resize] engine up\n");

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) return 2;
    const char* url = "data:text/html,<body style='margin:0;width:100vw;height:100vh;"
                      "background:%2300ff00'></body>";
    page.LoadUrlAndWait(url, 20);
    page.PumpFor(1000);

    size_t cap = (size_t)1024 * 768 * 4;
    unsigned char* buf = (unsigned char*)malloc(cap);

    page.ReadPixels(buf, cap);
    long green0 = countGreen(buf, page.Width() * page.Height());
    printf("[resize] before: %dx%d green=%ld\n", page.Width(), page.Height(), green0);

    if (!page.Resize(640, 480)) { fprintf(stderr, "[resize] Resize FAIL\n"); free(buf); return 4; }
    page.PumpFor(1500);

    page.ReadPixels(buf, cap);
    long green1 = countGreen(buf, page.Width() * page.Height());
    printf("[resize] after:  %dx%d green=%ld\n", page.Width(), page.Height(), green1);
    free(buf);

    // Surface must now be 640x480 and green must fill ~that area (307200 px),
    // clearly smaller than the original ~786432.
    bool dims = page.Width() == 640 && page.Height() == 480;
    bool ok = dims && green0 > 700000 && green1 > 250000 && green1 < 360000;
    printf("[resize] %s\n", ok ? "RESIZE PASS" : "RESIZE FAIL");
    rc = ok ? 0 : 4;
  }
  host.Shutdown();
  return rc;
}
