/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — mismatched-targetApplication XPI install, desktop harness.
 *
 * cavekit-addons-extensions.md R3, last criterion: "An extension whose targetApplication
 * does not match this app's ID/version is rejected with a CLEAR REASON rather than
 * installed and silently inert."
 *
 * The refusal itself was verified 2026-08-04. What was never run is the half landed for
 * T-103 on 2026-08-10: components/jihadInstallPrompt.js observes "addon-install-failed",
 * filters to installs whose addon.appDisabled is true, and raises a card alert through
 * @mozilla.org/prompter;1 -> JihadPrompter::Alert -> DialogSink -> msgDialogAlert. That
 * component was written, force-instantiated from DialogService.cpp, and never compiled or
 * executed. This test executes it.
 *
 * What is asserted, all text-level:
 *   1. the page callback still gets -210 (USER_CANCELLED) — UNCHANGED BY DECISION, because
 *      AddonInstall.cancel() fires onDownloadCancelled synchronously one step BEFORE the
 *      notification that carries the reason (see jihadInstallPrompt.js for the full trace);
 *   2. NO confirm dialog is raised — the refusal precedes any prompt, which is correct:
 *      there is nothing to ask about;
 *   3. exactly one NOTIFICATION is raised, and it NAMES THE ADD-ON and says it is
 *      incompatible. That text, captured off the NotificationSink, is the proof of the
 *      "clear reason".
 *
 * CHANGED 2026-08-15 (T-148, cavekit-gre-widgets.md R5). Assertion 3 used to read "exactly one
 * ALERT ... captured off the DialogSink", and the message used to travel as msgDialogAlert —
 * a MODAL that stops the daemon in awaitDialogReply until the card answers. It is a statement
 * of fact with no possible answer, so it moved to the non-blocking channel. THE TEST NOW
 * ASSERTS BOTH HALVES: the notification arrives AND no dialog is raised at all. Asserting only
 * the first would let a regression that raises both pass.
 *
 * Two controls, because a positive assertion nothing can falsify proves nothing:
 *   JIHAD_XPI_GOOD=1    the MATCHING add-on, DENIED at the confirm. Must raise a CONFIRM (a
 *                       real question, correctly still modal) and NO notification at all —
 *                       otherwise the observer fires on every failed install and its
 *                       appDisabled filter means nothing.
 *   JIHAD_XPI_ACCEPT=1  (with GOOD) the matching add-on ACCEPTED. Exercises the other new
 *                       observer, "addon-install-complete": exactly one notification saying
 *                       the add-on installed, and still no alert.
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

using namespace jihad;

static int gChecks = 0;
static int gFails = 0;

static void check(bool ok, const char* what, const std::string& got) {
  ++gChecks;
  if (!ok) ++gFails;
  printf("[xpi] %-4s %-50s got=[%s]\n", ok ? "PASS" : "FAIL", what, got.c_str());
}

struct Rec { DialogKind kind; std::string text; };

struct RecSink : DialogSink {
  std::vector<Rec> recs;
  bool acceptConfirm = false;
  void OnDialog(DialogKind k, const char* text, DialogReply* reply) override {
    std::string t = text ? text : "";
    recs.push_back({k, t});
    printf("[xpi] DIALOG %s text=[%s]\n",
           k == DialogKind::Alert ? "ALERT" : (k == DialogKind::Confirm ? "CONFIRM" : "PROMPT"),
           t.c_str());
    if (reply) reply->accept = acceptConfirm;   // deny by default, as an unattended card would
  }
  int count(DialogKind k) const {
    int n = 0;
    for (const Rec& r : recs) if (r.kind == k) ++n;
    return n;
  }
  std::string firstOf(DialogKind k) const {
    for (const Rec& r : recs) if (r.kind == k) return r.text;
    return std::string();
  }
};

// The non-blocking channel (T-148). On device this sink is the Luna subscription that pushes
// a toast to the card; here it records, so the message can be asserted as text exactly the way
// the alert used to be.
struct NoteSink : NotificationSink {
  std::vector<std::string> cats, texts;
  void OnNotification(const char* category, const char* text) override {
    cats.push_back(category ? category : "");
    texts.push_back(text ? text : "");
    printf("[xpi] NOTIFY category=[%s] text=[%s]\n", category ? category : "", text ? text : "");
  }
  std::string first() const { return texts.empty() ? std::string() : texts[0]; }
};

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* dir = getenv("JIHAD_XPI_DIR") ? getenv("JIHAD_XPI_DIR") : "/out";
  const bool useGood = getenv("JIHAD_XPI_GOOD") != nullptr;
  // Accept the confirm instead of declining it — only meaningful with the GOOD add-on, and the
  // only way to reach "addon-install-complete" at all.
  const bool accept = getenv("JIHAD_XPI_ACCEPT") != nullptr;
  // Deliberate-failure knob: apply the MISMATCH assertions to whichever add-on was installed.
  // Combined with JIHAD_XPI_GOOD=1 it runs the positive asserts against a premise that is
  // false by construction, which is the only way to know they can fail at all.
  const bool forceMismatchAsserts = getenv("JIHAD_XPI_ASSERT_MISMATCH") != nullptr;
  gtk_init(&argc, &argv);

  // The two add-ons differ in ONE field: install.rdf's <em:targetApplication><em:id>.
  // The mismatch names {ffffffff-...}, which is neither JIHAD_APP_ID nor the AppCompat GUID
  // nor the toolkit ID, so AddonInternal.matchingTargetApplication is null, isCompatibleWith
  // returns false and isUsableAddon sets appDisabled — regardless of strictCompatibility.
  const char* xpi  = useGood ? "jihad-t103-good.xpi" : "jihad-t103-mismatch.xpi";
  const char* name = useGood ? "Jihad T103 Good"     : "Jihad T103 Mismatch";

  printf("[xpi] START gre=%s dir=%s xpi=%s mode=%s\n", greDir, dir, xpi,
         !useGood ? "mismatched targetApplication"
                  : (accept ? "matching add-on, ACCEPTED at the confirm (install-complete)"
                            : "NEGATIVE-CONTROL (matching add-on, denied at the confirm)"));

  // The driver page. file:// installing a file:// XPI is what the 2026-08-03 run used, and
  // xpinstall.whitelist.{required,fileRequest} in packaging/prefs/jihad-addon-prefs.js are
  // what make it reachable at all.
  std::string pagePath = std::string(dir) + "/xpi-t103-page.html";
  {
    FILE* f = fopen(pagePath.c_str(), "w");
    if (!f) { printf("[xpi] cannot write %s\n", pagePath.c_str()); return 1; }
    fprintf(f,
      "<!DOCTYPE html><html><body style=\"margin:0;font-family:sans-serif\">\n"
      "<h1>T-103 install probe</h1>\n"
      "<div id=\"s\">idle</div>\n"
      "<script>\n"
      "function go() {\n"
      "  if (!window.InstallTrigger) { document.title = 'XPI:no-InstallTrigger'; return; }\n"
      "  try {\n"
      "    InstallTrigger.install({\"%s\": \"%s\"}, function (url, status) {\n"
      "      document.title = 'XPI:status=' + status;\n"
      "      document.getElementById('s').textContent = 'status=' + status;\n"
      "    });\n"
      "    if (document.title.indexOf('XPI:status=') !== 0) { document.title = 'XPI:pending'; }\n"
      "  } catch (e) {\n"
      "    document.title = 'XPI:threw ' + e;\n"
      "    document.getElementById('s').textContent = 'threw ' + e;\n"
      "  }\n"
      "}\n"
      "window.addEventListener('load', function () { setTimeout(go, 300); }, false);\n"
      "</script></body></html>\n", name, xpi);
    fclose(f);
  }

  EngineHost host;
  if (!host.Init(greDir)) { printf("[xpi] engine init FAIL\n"); return 1; }

  // This is what force-instantiates components/jihadInstallPrompt.js: the incompatible case
  // is refused BEFORE any confirm prompt, so nothing else would construct the component in
  // time to have registered the addon-install-failed observer.
  if (!InstallDialogService()) { printf("[xpi] InstallDialogService FAIL\n"); host.Shutdown(); return 1; }
  RecSink sink;
  sink.acceptConfirm = accept;
  SetDialogSink(&sink);
  NoteSink notes;
  SetNotificationSink(&notes);

  int rc = 3;
  {
    GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) { printf("[xpi] page create FAIL\n"); SetNotificationSink(nullptr); SetDialogSink(nullptr); host.Shutdown(); return 2; }

    std::string url = "file://" + pagePath;
    page.LoadUrlAndWait(url.c_str(), 30);

    // Pump until the page's install callback has reported, or the budget runs out. The
    // download is asynchronous even for a file:// source.
    std::string title;
    for (int i = 0; i < 40; ++i) {
      page.PumpFor(500);
      title = page.GetTitle();
      if (title.find("XPI:status=") == 0 || title.find("XPI:threw") == 0 ||
          title.find("XPI:no-InstallTrigger") == 0) break;
    }
    page.PumpFor(2000);   // let any observer notification that follows the callback land

    std::string statusText = DebugElementText("#s", 120);
    printf("[xpi] title=[%s] page-status=[%s] dialogs=%d notifications=%d\n",
           title.c_str(), statusText.c_str(), (int)sink.recs.size(), (int)notes.texts.size());

    check(title.find("XPI:threw") != 0 && title.find("XPI:no-InstallTrigger") != 0,
          "the install flow ran at all (InstallTrigger reachable)", title);

    // 1. the page-facing status. -210 = legacy XPInstall USER_CANCELLED. Unchanged BY
    //    DECISION for the mismatch case, and the same number the good add-on gets when the
    //    card declines — which is exactly why the message below is the "clear reason", not
    //    this. An ACCEPTED install reports 0 instead, which is the whole point of that mode.
    check(statusText == (accept ? "status=0" : "status=-210"),
          accept ? "page callback status is 0 (installed)"
                 : "page callback status is -210 (unchanged)", statusText);

    int confirms = sink.count(DialogKind::Confirm);
    int alerts   = sink.count(DialogKind::Alert);
    int notified = (int)notes.texts.size();

    if (!useGood || forceMismatchAsserts) {
      // 2. refused BEFORE any confirm prompt.
      check(confirms == 0, "refused before any confirm prompt was raised",
            std::to_string(confirms) + " confirms");

      // 3. THE OBSERVER FIRED, and its text names the add-on and the reason — on the
      //    NON-BLOCKING channel (T-148). The alert count is asserted too: this message must
      //    not be able to stall the daemon on a card that never answers.
      check(alerts == 0, "no BLOCKING alert was raised (it is a toast now)",
            std::to_string(alerts) + " alerts");
      check(notified == 1, "exactly one notification was posted",
            std::to_string(notified) + " notifications");
      std::string note = notes.first();
      std::string want = std::string(name) +
        " could not be installed because it is not compatible with Jihad Browser 1.0.";
      check(note == want, "the notification is the incompatibility-specific message", note);
      check(note.find(name) != std::string::npos, "the notification names the add-on", note);
      check(!notes.cats.empty() && notes.cats[0] == "addon",
            "it is categorised \"addon\"", notes.cats.empty() ? "(none)" : notes.cats[0]);
    } else if (accept) {
      // The OTHER new observer: "addon-install-complete". The install really ran, so the
      // message is the success notice cavekit-gre-widgets.md R5 names first and which had no
      // emitter at all before T-148.
      check(confirms == 1, "the matching add-on reached the confirm prompt",
            std::to_string(confirms) + " confirms");
      check(alerts == 0, "no blocking alert on the success path",
            std::to_string(alerts) + " alerts");
      check(notified == 1, "exactly one notification was posted",
            std::to_string(notified) + " notifications");
      std::string want = std::string(name) + " installed.";
      check(notes.first() == want, "the notification says the add-on installed", notes.first());
    } else {
      // Negative control: a MATCHING add-on reaches the confirm, and declining it must NOT
      // produce the incompatibility message. If it does, the observer's appDisabled filter is
      // not doing anything and the positive result above means nothing. It must not produce
      // the INSTALLED message either — nothing was installed.
      check(confirms == 1, "the matching add-on reached the confirm prompt",
            std::to_string(confirms) + " confirms: " + sink.firstOf(DialogKind::Confirm));
      check(alerts == 0, "no incompatibility alert for a merely-declined install",
            std::to_string(alerts) + " alerts: " + sink.firstOf(DialogKind::Alert));
      check(notified == 0, "no notification for a merely-declined install",
            std::to_string(notified) + " notifications: " + notes.first());
    }

    printf("[xpi] checks=%d fails=%d\n", gChecks, gFails);
    printf("[xpi] %s\n", gFails == 0 ? "XPI-MISMATCH PASS" : "XPI-MISMATCH FAIL");
    rc = (gFails == 0) ? 0 : 4;
  }
  SetNotificationSink(nullptr);
  SetDialogSink(nullptr);
  host.Shutdown();
  return rc;
}
