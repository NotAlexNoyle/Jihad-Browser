/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — browser-services test (domain G).
 * With JavaScript DISABLED, the onclick handler that turns the page green must
 * NOT run — the page stays blue. Also exercises setUserAgent/clearCache/
 * clearCookies (must run without crashing). Contrast with input_test (JS on).
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
  if (!host.Init(greDir)) { fprintf(stderr, "[svc] engine init FAIL\n"); return 1; }
  printf("[svc] engine up\n");

  // Global services should not crash.
  jihad::SetUserAgentOverride("JihadBrowser/1.0 (webOS 3; Goanna)");
  jihad::ClearCache();
  jihad::ClearCookies();
  printf("[svc] setUserAgent + clearCache + clearCookies ran\n");

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) return 2;
    page.SetJavaScriptEnabled(false);          // <-- disable JS before load
    const char* url = "data:text/html,<body onclick=\"document.body.style.background='%2300ff00'\" "
                      "style='margin:0;width:100vw;height:100vh;background:%230000ff'></body>";
    page.LoadUrlAndWait(url, 20);
    page.PumpFor(1200);

    const int N = page.Width() * page.Height();
    size_t sz = (size_t)N * 4;
    unsigned char* buf = (unsigned char*)malloc(sz);

    page.ClickAt(512, 384, 1);
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    long green = countColor(buf, N, 0,80, 180,255, 0,80);
    long blue  = countColor(buf, N, 0,80, 0,80, 180,255);
    printf("[svc] JS-disabled click: green=%ld blue=%ld\n", green, blue);

    bool ok = green < 1000 && blue > 100000;   // stayed blue => JS was blocked
    printf("[svc] %s (JS toggle %s block the onclick)\n",
           ok ? "SERVICES PASS" : "SERVICES FAIL", ok ? "DID" : "did NOT");
    rc = ok ? 0 : 4;
    free(buf);
  }
  host.Shutdown();
  return rc;
}
