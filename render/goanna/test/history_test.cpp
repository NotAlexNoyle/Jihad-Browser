/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — clearHistory + history-state test (domain F R1 / R5).
 * Load A then B (canGoBack true), clearHistory, and confirm canGoBack is false.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[hist] engine init FAIL\n"); return 1; }
  printf("[hist] engine up\n");

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(640, 480)) return 2;
    page.LoadUrlAndWait("data:text/html,<title>A</title><body>A</body>", 20);
    page.PumpFor(300);
    page.LoadUrlAndWait("data:text/html,<title>B</title><body>B</body>", 20);
    page.PumpFor(300);

    bool backBefore = page.CanGoBack();
    printf("[hist] after A,B: canGoBack=%d\n", backBefore);

    page.ClearHistory();
    page.PumpFor(200);
    bool backAfter = page.CanGoBack();
    bool fwdAfter = page.CanGoForward();
    printf("[hist] after clearHistory: canGoBack=%d canGoForward=%d\n", backAfter, fwdAfter);

    bool ok = backBefore && !backAfter && !fwdAfter;
    printf("[hist] %s\n", ok ? "HISTORY PASS" : "HISTORY FAIL");
    rc = ok ? 0 : 4;
  }
  host.Shutdown();
  return rc;
}
