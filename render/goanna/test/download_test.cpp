/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — download / MIME-handoff test (domain G / R4).
 * Navigating to an application/octet-stream data: URL is not renderable, so
 * Goanna hands it to the helper-app service. Our override must capture the
 * handoff (source URL + MIME) instead of opening a chrome window.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"
#include "../DownloadService.h"

struct RecSink : jihad::DownloadSink {
  int n = 0; std::string url, mime, name; long long len = -2;
  jihad::DownloadOrigin origin = nullptr;   // which page it came from (F-1)
  void OnDownload(jihad::DownloadOrigin o, const char* u, const char* m,
                  const char* nm, int64_t l) override {
    ++n; origin = o; url = u ? u : ""; mime = m ? m : ""; name = nm ? nm : ""; len = l;
  }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[dl] engine init FAIL\n"); return 1; }
  RecSink sink;
  jihad::SetDownloadSink(&sink);
  printf("[dl] engine up + download service installed\n");

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(640, 480)) return 2;
    // Unsupported MIME -> helper-app handoff.
    page.LoadUrlAndWait("data:application/octet-stream,JihadDownloadPayload", 20);
    page.PumpFor(2500);

    printf("[dl] captured n=%d mime='%s' url-prefix='%.32s'\n",
           sink.n, sink.mime.c_str(), sink.url.c_str());
    // Diagnostic only (F-1): does the reported origin identify THIS page? Not
    // asserted here — this test has a single page, so it cannot distinguish a
    // correct attribution from the fallback; the two-scenario daemon harness
    // (build-download2-test.sh) logs the same comparison against a real card.
    printf("[dl] origin=%p page=%p (%s)\n", sink.origin, page.DocShellKey(),
           (sink.origin && sink.origin == page.DocShellKey()) ? "matches this page"
                                                              : "unresolved/fallback");
    bool ok = sink.n >= 1 && strstr(sink.mime.c_str(), "octet-stream") &&
              sink.url.rfind("data:application/octet-stream", 0) == 0;
    printf("[dl] %s\n", ok ? "DOWNLOAD PASS" : "DOWNLOAD FAIL");
    rc = ok ? 0 : 4;
  }
  jihad::SetDownloadSink(nullptr);
  host.Shutdown();
  return rc;
}
