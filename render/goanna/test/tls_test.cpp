/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — TLS certificate handling test (domain G / R5).
 * Loads a local self-signed HTTPS endpoint. The invalid cert must be detected
 * and surfaced (host + security error code) as an SSL-confirm, and the load must
 * NOT proceed by default (reject aborts). Accepting (validity override) requires
 * the untrusted cert object, which this headless embedding does not expose
 * (nsIBadCertListener2 isn't consulted; SSL status/failed-chain are null) -- that
 * plus the webOS cert store are [human-review on device]. Base via $JIHAD_TLS_BASE.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <string>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* base = getenv("JIHAD_TLS_BASE");
  if (!base || !*base) base = "https://127.0.0.1:18443";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[tls] engine init FAIL\n"); return 1; }
  printf("[tls] engine up (base=%s)\n", base);

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(640, 480)) return 2;

    std::string url = std::string(base) + "/";
    page.LoadUrlAndWait(url.c_str(), 20);
    page.PumpFor(500);

    std::string chost; int ccode = 0;
    bool certErr = page.GetCertError(&chost, &ccode);
    // nsresult module field == NS_ERROR_MODULE_SECURITY(21) + base offset(0x45).
    bool isSecErr = ((((uint32_t)ccode) >> 16) & 0x7fff) == (21u + 0x45u);
    // Reject is the default: the untrusted page must not have loaded its content.
    std::string loc = page.CurrentUri();
    bool aborted = loc.find("tls-ok") == std::string::npos;
    printf("[tls] certError=%d host=%s code=0x%x isSecErr=%d rejectAborted=%d loc=%s\n",
           certErr, chost.c_str(), (unsigned)ccode, isSecErr, aborted, loc.c_str());

    // Accept path is device-gated (no cert object headless) -- attempt + report.
    bool accepted = page.AcceptCurrentCert();
    printf("[tls] accept override attempted=%d (accept-proceeds + cert store are on-device)\n", accepted);

    bool ok = certErr && isSecErr &&
              chost.find("127.0.0.1") != std::string::npos && aborted;
    printf("[tls] %s\n", ok ? "TLS PASS" : "TLS FAIL");
    rc = ok ? 0 : 4;
  }
  host.Shutdown();
  return rc;
}
