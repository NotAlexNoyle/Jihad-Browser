/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — input synthesis test (domain E / T-026).
 * Loads a page whose onclick turns the blue background green, renders, clicks
 * the center via nsIDOMWindowUtils, re-renders, and checks the pixels went
 * blue -> green. Proves synthesized input reaches the DOM. Run under Xvfb.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

static long countColor(unsigned char* buf, int n,
                       int rmn,int rmx,int gmn,int gmx,int bmn,int bmx) {
  long c = 0;
  for (int i = 0; i < n; ++i) { unsigned char* p = buf + (size_t)i*4;   // B,G,R,A
    int b=p[0], g=p[1], r=p[2];
    if (r>=rmn&&r<=rmx && g>=gmn&&g<=gmx && b>=bmn&&b<=bmx) ++c; }
  return c;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[input] engine init FAIL\n"); return 1; }
  printf("[input] engine up\n");

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) { fprintf(stderr, "[input] create FAIL\n"); return 2; }
    // onclick turns the blue (#0000ff) body green (#00ff00).
    const char* url = "data:text/html,<body onclick=\"document.body.style.background='%2300ff00'\" "
                      "style='margin:0;width:100vw;height:100vh;background:%230000ff'></body>";
    page.LoadUrlAndWait(url, 20);
    page.PumpFor(1200);

    const int N = page.Width() * page.Height();
    size_t sz = (size_t)N * 4;
    unsigned char* buf = (unsigned char*)malloc(sz);

    page.ReadPixels(buf, sz);
    long blueBefore  = countColor(buf, N, 0,80, 0,80, 180,255);
    long greenBefore = countColor(buf, N, 0,80, 180,255, 0,80);
    printf("[input] before click: blue=%ld green=%ld\n", blueBefore, greenBefore);

    printf("[input] clicking center (512,384)\n");
    page.ClickAt(512, 384, 1);
    page.PumpFor(2000);

    page.ReadPixels(buf, sz);
    long greenAfter = countColor(buf, N, 0,80, 180,255, 0,80);
    printf("[input] after click:  green=%ld\n", greenAfter);

    bool ok = blueBefore > 100000 && greenBefore < 1000 && greenAfter > 100000;
    printf("[input] %s (click %s change the page)\n",
           ok ? "INPUT PASS" : "INPUT FAIL", ok ? "DID" : "did NOT");
    rc = ok ? 0 : 4;
    free(buf);
  }
  host.Shutdown();
  return rc;
}
