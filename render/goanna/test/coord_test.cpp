/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — coordinate mapping under zoom (domain E / R5). A small target
 * at a known content position turns the page green when clicked. We probe which
 * coordinate space clickAt uses at zoom 1x and 2x to validate/derive the mapping.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

static long green(jihad::GoannaRenderPage& p) {
  int N = p.Width() * p.Height();
  unsigned char* b = (unsigned char*)malloc((size_t)N*4);
  p.ReadPixels(b, (size_t)N*4);
  long c = 0;
  for (int i = 0; i < N; ++i) { unsigned char* q = b + (size_t)i*4;
    if (q[2] < 80 && q[1] >= 180 && q[0] < 80) ++c; }
  free(b);
  return c;
}

// Target: 40x40 box at content (100,100); onclick paints the body green.
static const char* kPage =
  "data:text/html,<body style='margin:0;background:%230000ff'>"
  "<div onclick=\"document.body.style.background='%2300ff00'\" "
  "style='position:absolute;left:100px;top:100px;width:40px;height:40px;"
  "background:%23ff0000'></div></body>";

static long trial(jihad::EngineHost& host, double zoom, int cx, int cy, const char* label) {
  jihad::GoannaRenderPage page(host);
  page.Create(800, 600);
  page.LoadUrlAndWait(kPage, 20);
  page.PumpFor(700);
  if (zoom != 1.0) { page.SetZoom(zoom); page.PumpFor(700); }
  page.ClickAt(cx, cy, 1);
  page.PumpFor(1000);
  long g = green(page);
  printf("[coord] %s zoom=%.1f click=(%d,%d) green=%ld hit=%d\n",
         label, zoom, cx, cy, g, g > 50000);
  return g;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[coord] engine init FAIL\n"); return 1; }
  printf("[coord] engine up\n");

  // Baseline: at 1x a click inside the content box (120,120) must hit.
  long base = trial(host, 1.0, 120, 120, "baseline-content");
  // At 2x the box is displayed at surface (200..280). Does a surface-space click
  // (240,240) hit, or does clickAt use content space (120,120)?
  long surf = trial(host, 2.0, 240, 240, "zoom2-surface");
  long cont = trial(host, 2.0, 120, 120, "zoom2-content");

  bool baseOK = base > 50000;
  printf("[coord] baseline_hit=%d zoom2_surface_hit=%d zoom2_content_hit=%d\n",
         baseOK, surf > 50000, cont > 50000);
  host.Shutdown();
  return baseOK ? 0 : 4;
}
