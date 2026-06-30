/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — embedding smoke test (T-013 acceptance).
 *
 * Brings the Goanna engine up via EngineHost, creates and destroys an
 * nsIWebBrowser instance, and tears the runtime down. Prints a clear PASS/FAIL.
 * This proves the embedding runtime + per-page instance lifecycle works against
 * the built libxul, independent of any rendering wiring.
 *
 * Usage: embed_smoke <greDir>   (greDir = directory containing libxul.so)
 */
#include <cstdio>
#include "../EngineHost.h"
#include "nsIWebBrowser.h"
#include "nsCOMPtr.h"

int main(int argc, char** argv)
{
  const char* greDir = (argc > 1) ? argv[1] : ".";
  printf("[embed_smoke] greDir=%s\n", greDir);

  jihad::EngineHost host;
  if (!host.Init(greDir)) {
    fprintf(stderr, "[embed_smoke] FAIL: EngineHost.Init (XRE_InitEmbedding2)\n");
    return 1;
  }
  printf("[embed_smoke] engine runtime up\n");

  {
    nsCOMPtr<nsIWebBrowser> wb = host.CreateBrowser();
    if (!wb) {
      fprintf(stderr, "[embed_smoke] FAIL: CreateBrowser (nsIWebBrowser)\n");
      host.Shutdown();
      return 2;
    }
    printf("[embed_smoke] created nsIWebBrowser @ %p\n", (void*)wb.get());
    // Destroyed here as wb goes out of scope (per-page instance lifecycle).
  }
  printf("[embed_smoke] destroyed nsIWebBrowser\n");

  host.Shutdown();
  printf("[embed_smoke] PASS: runtime up + instance create/destroy + shutdown\n");
  return 0;
}
