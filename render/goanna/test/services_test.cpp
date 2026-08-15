/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — browser-services test (domain G).
 * With JavaScript DISABLED, the onclick handler that turns the page green must
 * NOT run — the page stays blue. Also exercises setUserAgent/clearCache/
 * clearCookies (must run without crashing). Contrast with input_test (JS on).
 *
 * ALSO, since 2026-08-15 (T-148, cavekit-gre-widgets.md R5): the two privacy actions
 * ACKNOWLEDGE themselves, and they do it WITHOUT a modal. Both sinks are installed and both
 * are asserted, because either half alone would pass for the wrong reason —
 *   * a notification with no dialog check would not notice a regression that raises BOTH;
 *   * a "no dialog" check with no notification check is satisfied by the old silent code.
 * The dialog half matters beyond tidiness: OnDialog writes a FIFO and POLLS it until the card
 * answers or the deadline expires, so a msgDialog* on this path would stall the daemon for
 * seconds every time a user cleared their cookies.
 *
 * JIHAD_SVC_NEG=148 is the deliberate-failure control: it expects a notification text no
 * build emits, so the assertions below are proved able to fail.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"
#include "../DialogService.h"

static long countColor(unsigned char* buf, int n,
                       int rmn,int rmx,int gmn,int gmx,int bmn,int bmx) {
  long c = 0;
  for (int i = 0; i < n; ++i) { unsigned char* p = buf + (size_t)i*4;
    int b=p[0], g=p[1], r=p[2];
    if (r>=rmn&&r<=rmx && g>=gmn&&g<=gmx && b>=bmn&&b<=bmx) ++c; }
  return c;
}

static int gChecks = 0;
static int gFails  = 0;
static void check(bool ok, const char* what, const std::string& got) {
  ++gChecks;
  if (!ok) ++gFails;
  printf("[svc] %-4s %-52s got=[%s]\n", ok ? "PASS" : "FAIL", what, got.c_str());
}

// The BLOCKING channel. Nothing on the privacy path may reach it.
struct RecDialogSink : jihad::DialogSink {
  std::vector<std::string> texts;
  void OnDialog(jihad::DialogKind, const char* text, jihad::DialogReply* reply) override {
    texts.push_back(text ? text : "");
    printf("[svc] DIALOG (unexpected here) text=[%s]\n", text ? text : "");
    if (reply) reply->accept = false;
  }
};

// The NON-blocking channel. On device this is the Luna subscription; here it just records.
struct RecNotifySink : jihad::NotificationSink {
  std::vector<std::string> cats, texts;
  void OnNotification(const char* category, const char* text) override {
    cats.push_back(category ? category : "");
    texts.push_back(text ? text : "");
    printf("[svc] NOTIFY category=[%s] text=[%s]\n", category ? category : "", text ? text : "");
  }
  bool has(const std::string& t) const {
    for (const std::string& s : texts) if (s == t) return true;
    return false;
  }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const bool neg = getenv("JIHAD_SVC_NEG") != nullptr;
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[svc] engine init FAIL\n"); return 1; }
  printf("[svc] engine up\n");

  // Both channels, installed BEFORE the services run. The dialog sink is deliberately a
  // recorder rather than absent: with no sink at all "no dialog was raised" and "a dialog was
  // raised and dropped on the floor" are the same observation.
  RecDialogSink dialogs;
  RecNotifySink notes;
  jihad::SetDialogSink(&dialogs);
  jihad::SetNotificationSink(&notes);

  // Global services should not crash.
  jihad::SetUserAgentOverride("JihadBrowser/1.0 (webOS 3; Goanna)");
  jihad::ClearCache();
  jihad::ClearCookies();
  printf("[svc] setUserAgent + clearCache + clearCookies ran\n");

  // T-148: the flow COMPLETED (both calls returned — we are here), it said so, and it said so
  // without blocking.
  const char* wantCache   = neg ? "Cache emptied."   : "Cache cleared.";
  const char* wantCookies = neg ? "Cookies emptied." : "Cookies cleared.";
  check(dialogs.texts.empty(), "clearCache/clearCookies raised NO blocking dialog",
        std::to_string(dialogs.texts.size()) + " dialogs");
  check(notes.texts.size() == 2, "exactly two notifications were posted",
        std::to_string(notes.texts.size()) + " notifications");
  check(notes.has(wantCache),   "the cache clear acknowledged itself",   wantCache);
  check(notes.has(wantCookies), "the cookie clear acknowledged itself",  wantCookies);
  bool privacyCat = !notes.cats.empty();
  for (const std::string& c : notes.cats) if (c != "privacy") privacyCat = false;
  check(privacyCat, "both are categorised \"privacy\"",
        notes.cats.empty() ? "(none)" : notes.cats[0]);

  // THE DEGRADE PATH, exercised in the same run rather than argued. With NO sink at all — the
  // desktop case, and equally a device whose liblunaservice.so turns out to have no
  // LSSubscriptionAdd — a post must be a no-op that logs one line and returns. What is being
  // ruled out is the two ways this could go wrong quietly: falling back to the blocking dialog
  // channel, or taking the process with it. The log line itself is on stderr
  // ("[jihad-bs] notify (no channel): …") and is checked by reading the run output.
  jihad::SetNotificationSink(nullptr);
  size_t dialogsBefore = dialogs.texts.size();
  size_t notesBefore   = notes.texts.size();
  jihad::ClearCookies();
  check(dialogs.texts.size() == dialogsBefore,
        "with NO channel the post is still not a dialog",
        std::to_string(dialogs.texts.size()) + " dialogs");
  check(notes.texts.size() == notesBefore,
        "with NO channel nothing reaches the detached sink",
        std::to_string(notes.texts.size()) + " notifications");
  jihad::SetNotificationSink(&notes);

  int rc = 3;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) { jihad::SetDialogSink(nullptr);
                                   jihad::SetNotificationSink(nullptr); return 2; }
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
    check(ok, "JS toggle blocked the onclick (page stayed blue)",
          "green=" + std::to_string(green) + " blue=" + std::to_string(blue));
    printf("[svc] checks=%d fails=%d\n", gChecks, gFails);
    printf("[svc] %s\n", gFails == 0 ? "SERVICES PASS" : "SERVICES FAIL");
    rc = (gFails == 0) ? 0 : 4;
    free(buf);
  }
  jihad::SetNotificationSink(nullptr);
  jihad::SetDialogSink(nullptr);
  host.Shutdown();
  return rc;
}
