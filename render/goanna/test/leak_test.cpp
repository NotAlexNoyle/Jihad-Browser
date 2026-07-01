/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — repeated create/destroy cycles (domain engine-embedding R2).
 * The embedding runtime is initialized once; a page (nsIWebBrowser + offscreen
 * widget) is created, loaded, rendered, and destroyed N times. Completing all
 * cycles and rendering each frame proves per-page create/destroy is clean (no
 * crash) across repetition.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  int cycles = (argc > 2) ? atoi(argv[2]) : 20;
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[leak] engine init FAIL\n"); return 1; }
  printf("[leak] engine up; running %d create/destroy cycles\n", cycles);

  int rendered = 0;
  for (int i = 0; i < cycles; ++i) {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(640, 480)) { fprintf(stderr, "[leak] Create FAIL at cycle %d\n", i); return 2; }
    char url[128];
    snprintf(url, sizeof url, "data:text/html,<body style='background:%%23%06x'>cycle %d</body>",
             (i * 0x112233) & 0xffffff, i);
    page.LoadUrlAndWait(url, 15);
    page.PumpFor(120);
    int N = page.Width() * page.Height();
    unsigned char* b = (unsigned char*)malloc((size_t)N*4);
    long nb = page.ReadPixels(b, (size_t)N*4);
    free(b);
    if (nb > 0) ++rendered;
    if ((i % 5) == 0) printf("[leak] cycle %d ok (non-white px=%ld)\n", i, nb);
    // page destructor runs here: ordered teardown of browser + widget.
  }

  printf("[leak] completed %d/%d cycles; rendered %d\n", cycles, cycles, rendered);
  bool ok = rendered == cycles;
  printf("[leak] %s\n", ok ? "LEAK-CYCLE PASS" : "LEAK-CYCLE FAIL");
  host.Shutdown();
  return ok ? 0 : 4;
}
