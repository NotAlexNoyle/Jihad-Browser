/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — scroll test (domain D / R5, setScrollPosition).
 * Load a tall page, scroll to (0,500), and confirm the engine reports the new
 * scroll offset via GetScrollXY.
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
  if (!host.Init(greDir)) { fprintf(stderr, "[scroll] engine init FAIL\n"); return 1; }
  printf("[scroll] engine up\n");

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) return 2;
    // A page taller than the viewport so there is room to scroll.
    const char* url = "data:text/html,<body style='margin:0;width:100vw;height:3000px;"
                      "background:linear-gradient(%23ff0000,%230000ff)'></body>";
    page.LoadUrlAndWait(url, 20);
    page.PumpFor(1000);

    int x0 = -1, y0 = -1;
    page.GetScrollXY(&x0, &y0);
    printf("[scroll] before: (%d,%d)\n", x0, y0);

    page.ScrollTo(0, 500);
    page.PumpFor(1000);

    int x1 = -1, y1 = -1;
    page.GetScrollXY(&x1, &y1);
    printf("[scroll] after:  (%d,%d)\n", x1, y1);

    bool ok = x0 == 0 && y0 == 0 && x1 == 0 && y1 >= 480 && y1 <= 520;
    printf("[scroll] %s\n", ok ? "SCROLL PASS" : "SCROLL FAIL");
    rc = ok ? 0 : 4;
  }
  host.Shutdown();
  return rc;
}
