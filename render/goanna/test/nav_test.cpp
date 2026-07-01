/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — navigation history test (domain F).
 * Loads A then B, checks canGoBack/canGoForward, goes Back (verifies URI==A and
 * canGoForward), goes Forward (URI==B), then setHtml (inline). Proves session
 * history + canGo* + setHtml against the real engine. Run under Xvfb.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

static bool uriHas(jihad::GoannaRenderPage& p, const char* needle, int waitMs) {
  for (int t = 0; t < waitMs; t += 100) {
    if (strstr(p.CurrentUri().c_str(), needle)) return true;
    p.PumpFor(100);
  }
  return strstr(p.CurrentUri().c_str(), needle) != nullptr;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[nav] engine init FAIL\n"); return 1; }
  printf("[nav] engine up\n");

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) return 2;

    page.LoadUrlAndWait("data:text/html,<title>PAGE_A</title>A", 15);
    page.LoadUrlAndWait("data:text/html,<title>PAGE_B</title>B", 15);
    bool cb = page.CanGoBack(), cf = page.CanGoForward();
    printf("[nav] after A,B: canGoBack=%d canGoForward=%d (want 1,0)\n", cb, cf);

    page.GoBack();
    bool backOk = uriHas(page, "PAGE_A", 8000);
    bool cf2 = page.CanGoForward();
    printf("[nav] after Back: uri=%s canGoForward=%d (want PAGE_A,1)\n", page.CurrentUri().c_str(), cf2);

    page.GoForward();
    bool fwdOk = uriHas(page, "PAGE_B", 8000);
    printf("[nav] after Forward: uri=%s (want PAGE_B)\n", page.CurrentUri().c_str());

    page.SetHtml("<title>INLINE_OK</title><h1>hello</h1>");
    bool htmlOk = uriHas(page, "INLINE_OK", 8000);
    printf("[nav] after setHtml: uri contains INLINE_OK=%d\n", htmlOk);

    bool ok = cb && !cf && backOk && cf2 && fwdOk && htmlOk;
    printf("[nav] %s\n", ok ? "NAV PASS" : "NAV FAIL");
    rc = ok ? 0 : 4;
  }
  host.Shutdown();
  return rc;
}
