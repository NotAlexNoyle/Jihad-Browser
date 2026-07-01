/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — settings test (domain G / R1: minFontSize, blockPopups,
 * acceptCookies). Behavioural check for popup blocking (window.open during load
 * returns null -> page paints green), plus engine-pref readback for the other
 * two (confirms the governing pref took effect).
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include "nsCOMPtr.h"
#include "nsIPrefBranch.h"
#include "nsServiceManagerUtils.h"
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

static long countGreen(unsigned char* buf, int n) {
  long c = 0;
  for (int i = 0; i < n; ++i) { unsigned char* p = buf + (size_t)i*4;
    if (p[2]<80 && p[1]>=180 && p[0]<80) ++c; }
  return c;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[set2] engine init FAIL\n"); return 1; }
  printf("[set2] engine up\n");

  jihad::SetMinFontSize(24);
  jihad::SetAcceptCookies(false);
  jihad::SetBlockPopups(true);

  // Pref readback.
  nsCOMPtr<nsIPrefBranch> pb = do_GetService("@mozilla.org/preferences-service;1");
  int32_t mf = 0, cb = -1; bool bp = false;
  if (pb) {
    pb->GetIntPref("font.minimum-size.x-western", &mf);
    pb->GetIntPref("network.cookie.cookieBehavior", &cb);
    pb->GetBoolPref("dom.disable_open_during_load", &bp);
  }
  printf("[set2] prefs: minFont=%d cookieBehavior=%d blockPopups=%d\n", mf, cb, bp);

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(800, 600)) return 2;
    // window.open during load is blocked -> returns null -> green.
    const char* url = "data:text/html,<body style='margin:0;width:100vw;height:100vh;"
                      "background:%23ff0000' onload=\"var w=window.open('about:blank');"
                      "document.body.style.background = w ? '%23ff0000' : '%2300ff00';\"></body>";
    page.LoadUrlAndWait(url, 20);
    page.PumpFor(1500);
    const int N = page.Width() * page.Height();
    unsigned char* buf = (unsigned char*)malloc((size_t)N*4);
    page.ReadPixels(buf, (size_t)N*4);
    long green = countGreen(buf, N);
    free(buf);
    printf("[set2] popup-blocked green=%ld\n", green);

    bool prefsOK = mf == 24 && cb == 2 && bp;
    bool popupOK = green > 300000;   // most of 800x600 turned green
    printf("[set2] prefsOK=%d popupOK=%d\n", prefsOK, popupOK);
    bool ok = prefsOK && popupOK;
    printf("[set2] %s\n", ok ? "SETTINGS2 PASS" : "SETTINGS2 FAIL");
    rc = ok ? 0 : 4;
  }
  host.Shutdown();
  return rc;
}
