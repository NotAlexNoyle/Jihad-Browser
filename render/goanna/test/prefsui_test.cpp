/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — about:preferences desktop-harness verification.
 *
 * Covers two cavekit-gre-widgets.md criteria that were BUILT but never RUN:
 *
 *   T-139 (R5) — the GRE's notificationbox attaches inside the shipped chrome page and
 *                is dismissable. Judged from the DOM, never from pixels: desktop-harness
 *                pixel readback is a recorded dead end (context/impl/dead-ends.md), and
 *                "looks gone" is not "gone" for a bar whose default close path finishes in
 *                a transitionend handler that may never fire offscreen.
 *   T-111 (R7) — the ELEVEN row-backing prefs that used to be appended device-only now come
 *                from packaging/prefs/jihad-platform-prefs.js, which BOTH builds append. The
 *                criterion's remaining half was "neither build has been RUN, so this is an
 *                argument, not an observation". This is the observation.
 *
 * Everything asserted here is DOM- or text-level. The instruments are the daemon's own
 * debug channel (jihad::Debug*), which is what the kit's T-139 note already names as the
 * way in — a `javascript:` URL is NOT a reliable way into a chrome document here, so the
 * page is driven by real synthesized taps on its real controls wherever possible.
 *
 * Negative controls (JIHAD_PREFSUI_NEG), because an assert that cannot fail proves nothing:
 *   JIHAD_PREFSUI_NEG=139  look for a notification value the page never emits
 *   JIHAD_PREFSUI_NEG=111  add a pref this build does not ship to the eleven
 * Either must make the run FAIL. Run them once whenever this file changes.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

using namespace jihad;

static int gChecks = 0;
static int gFails = 0;

static void check(bool ok, const char* what, const std::string& got) {
  ++gChecks;
  if (!ok) ++gFails;
  printf("[prefsui] %-4s %-52s got=[%s]\n", ok ? "PASS" : "FAIL", what, got.c_str());
}

static bool has(const std::string& hay, const char* needle) {
  return hay.find(needle) != std::string::npos;
}

// Click an element by selector, bringing it into the viewport first. DebugClickElement
// clicks the element's centre in VIEWPORT css space, so a control below the fold is
// clicked at a coordinate the widget never delivers — the click lands on <null> and the
// run reads as "the button does nothing" rather than "the button was off screen". That
// happened on the first draft of this test: reset-pane sat at y=854 in a 768-high viewport.
//
// The viewport is GROWN rather than scrolled, and that is not a shortcut:
// GoannaRenderPage::ScrollTo drives window.scrollTo through a `javascript:` URL, and a
// `javascript:` URL does NOT execute in a chrome document here (measured again by this
// test, see the chrome-jsurl INFO line below). Resize() is C++ all the way down
// (nsIBaseWindow + ResizeReflowIgnoreOverride), so it works where ScrollTo cannot.
static bool clickInView(GoannaRenderPage& page, const char* sel) {
  std::string r = DebugElementRect(sel);
  int x = 0, y = 0, w = 0, h = 0;
  const char* sp = strstr(r.c_str(), "rect=");
  if (!sp || sscanf(sp + 5, "%d,%d %dx%d", &x, &y, &w, &h) != 4) {
    printf("[prefsui] clickInView: no rect for %s -> %s\n", sel, r.c_str());
    return false;
  }
  int cy = y + h / 2;
  if (cy < 0 || cy >= page.Height()) {
    printf("[prefsui] clickInView: %s centre y=%d outside %d-high viewport — growing it\n",
           sel, cy, page.Height());
    page.Resize(page.Width(), cy + h + 120);
    page.PumpFor(800);
  }
  bool ok = DebugClickElement(sel, 1);
  page.PumpFor(1500);
  return ok;
}

// The eleven prefs that back an about:preferences row AND used to be appended only by
// build/webos-oe/make-device-bundle.sh. Intersection of that heredoc with the `pref:`
// keys in packaging/prefsui/content/preferences.js; the values are the ones now carried
// by packaging/prefs/jihad-platform-prefs.js. Three of them are the ones the kit names
// as SILENTLY STOCK on desktop before the split (frame_rate -1, disk capacity 256000,
// prefetch-next true) — those are the interesting ones, and they are marked.
struct PrefRow {
  const char* name;
  char type;          // DebugGetPref type letter
  const char* expect; // shared-file value
  const char* pane;   // about:preferences pane that renders the row
  bool wasSilentlyStock;
};

static std::vector<PrefRow> ElevenRows() {
  std::vector<PrefRow> v = {
    {"general.smoothScroll",                  'b', "false", "general",  false},
    {"browser.sessionhistory.max_entries",     'i', "20",    "general",  false},
    {"layout.frame_rate",                      'i', "30",    "general",  true},
    {"browser.cache.disk.enable",              'b', "true",  "advanced", false},
    {"browser.cache.disk.capacity",            'i', "51200", "advanced", true},
    {"browser.cache.disk.smart_size.enabled",  'b', "false", "advanced", false},
    {"browser.cache.memory.capacity",          'i', "16384", "advanced", false},
    {"network.http.max-connections",           'i', "32",    "advanced", false},
    {"network.dns.disablePrefetch",            'b', "true",  "advanced", false},
    {"network.prefetch-next",                  'b', "false", "advanced", true},
    {"image.animation_mode",                   's', "once",  "advanced", false},
  };
  const char* neg = getenv("JIHAD_PREFSUI_NEG");
  if (neg && !strcmp(neg, "111")) {
    // Deliberate-failure control: a pref no build ships. If the run still passes, every
    // other row in this table proves nothing either.
    v.push_back({"jihad.no.such.pref", 'i', "42", "general", false});
  }
  return v;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* neg = getenv("JIHAD_PREFSUI_NEG");
  gtk_init(&argc, &argv);

  printf("[prefsui] START gre=%s neg=%s\n", greDir, neg ? neg : "(none)");

  EngineHost host;
  if (!host.Init(greDir)) { printf("[prefsui] engine init FAIL\n"); return 1; }

  int rc = 3;
  {
    // 1024x1200, not 1024x768: the Browser pane's footer button sits at y~854, and the only
    // way to reach a control below the fold in a CHROME document here is a taller viewport
    // (see clickInView). The device card is smaller; that is the device half of the criterion.
    GoannaRenderPage page(host);
    if (!page.Create(1024, 1200)) { printf("[prefsui] page create FAIL\n"); host.Shutdown(); return 2; }

    // ---- phase 0: the real chrome page loads ------------------------------------------
    // about:preferences is resolved by render/goanna/components/jihadAboutPreferences.js to
    // chrome://jihad-prefs/content/preferences.html. If that component or the prefsui package
    // is missing from the dist this is where it shows, and nothing below would mean anything.
    bool loaded = page.LoadUrlAndWait("about:preferences", 30);
    page.PumpFor(2500);
    printf("[prefsui] load ok=%d title=[%s]\n", (int)loaded, page.GetTitle().c_str());

    std::string pageTitle = DebugElementText("#page-title", 80);
    check(pageTitle == "Preferences", "about:preferences renders the chrome page", pageTitle);
    if (pageTitle != "Preferences") {
      printf("[prefsui] PREMISE BROKEN: the page did not load; skipping every dependent assert\n");
      printf("[prefsui] checks=%d fails=%d\n", gChecks, gFails);
      printf("[prefsui] PREFSUI FAIL\n");
      host.Shutdown();
      return 5;
    }

    // =====================================================================================
    // T-139 — cavekit-gre-widgets.md R5: a notificationbox renders and is dismissable
    // =====================================================================================

    // (a1) the element exists: initNotifyBox() created it with createElementNS(XUL_NS, ...)
    // inside an HTML document, which had never been done in this build.
    std::string boxRect = DebugElementRect("sel:#msgbar notificationbox");
    check(!has(boxRect, "(no element)") && has(boxRect, "rect="),
          "T139a1 initNotifyBox created the XUL notificationbox", boxRect);

    // (a2) the BINDING attached. NAC/XBL readback, not a guess: an unbound <notificationbox>
    // is a styled box with no implementation at all, and xul.css is only pulled into an HTML
    // document on demand when the first non-minimal XUL element is bound.
    std::string boxAnon = DebugAnonNodes("#msgbar notificationbox");
    int xblCount = -1;
    const char* xp = strstr(boxAnon.c_str(), "xbl=");
    if (xp) sscanf(xp + 4, "%d", &xblCount);
    check(xblCount >= 1, "T139a2 XBL binding attached to the notificationbox", boxAnon);

    // INFO, not a check. The kit's T-139 hazard 3 says a `javascript:` URL does not execute
    // in a chrome document, measured 2026-08-04 against about:addons — a XUL one. This run
    // measures the same thing against a chrome-privileged HTML document, which had never been
    // tried, so the finding is not accidentally read as XUL-specific. It also rules the probe
    // out as an instrument for the asserts below, which is why they are all real taps.
    {
      std::string before = page.GetTitle();
      DebugRunChromeJs("javascript:void(document.title='chrome-jsurl-ran')");
      page.PumpFor(600);
      std::string after = page.GetTitle();
      printf("[prefsui] INFO chrome-jsurl in a chrome HTML document: %s (title before=[%s] after=[%s])\n",
             after == "chrome-jsurl-ran" ? "EXECUTED" : "did NOT execute", before.c_str(), after.c_str());
    }

    // (b) a REAL TAP renders a notification. resetPane() on the Browser pane (the default
    // pane) announces unconditionally, so this is the page's own consumer, not an injected
    // call: click "Restore this pane's defaults" and the bar must appear.
    bool tapped = clickInView(page, "reset-pane");
    printf("[prefsui] tap reset-pane ok=%d\n", (int)tapped);

    const char* wantValue = (neg && !strcmp(neg, "139")) ? "no-such-notification" : "prefs-reset";
    const char* kResetMsg = "Home page and start page links restored to their defaults.";

    std::string byValue =
      DebugElementRect((std::string("sel:#msgbar notification[value=\"") + wantValue + "\"]").c_str());
    // (a3) AND (b) in one element. This <notification> can only exist if announce() got past
    // notifyBox(), whose whole body is `typeof gNotifyBox.appendNotification === "function"` —
    // i.e. the page's own method test, which is the test the criterion asks for. There is no
    // other way into the DOM for this element: the page never creates one itself, and the
    // `javascript:` inject channel is ruled out above.
    check(!has(byValue, "(no element)"),
          "T139a3 the page's typeof-appendNotification gate passed + rendered a bar", byValue);

    std::string byLabel =
      DebugElementRect((std::string("sel:#msgbar notification[label=\"") + kResetMsg + "\"]").c_str());
    check(!has(byLabel, "(no element)"), "T139b  the notification carries the expected label", byLabel);

    // Hazard 2's answer, asserted rather than assumed: the platform's own X routes through
    // dismiss() -> close() -> removeNotification(item) with NO skipAnimation, i.e. the
    // transitionend-on-margin-top path that may never complete offscreen. announce() hides it
    // and ships a Dismiss button instead. Both facts are DOM-visible.
    std::string hideclose = DebugElementRect("sel:#msgbar notification[hideclose=\"true\"]");
    check(!has(hideclose, "(no element)"),
          "T139b  the animated platform close button is suppressed", hideclose);
    std::string dismissBtn = DebugElementRect("sel:#msgbar notification button.notification-button");
    check(!has(dismissBtn, "(no element)"), "T139b  the Dismiss button is rendered", dismissBtn);

    // (d) the #status footer mirror carries the message too — announce() calls status(label)
    // unconditionally, so a build where the binding never attached is no worse off.
    std::string statusText = DebugElementText("#status", 300);
    check(statusText == kResetMsg, "T139d  #status footer mirrors the message", statusText);

    // (c) dismissal, cycle 1: the SHIPPED affordance — a real tap on the Dismiss button. Its
    // callback returns TRUE, which suppresses the binding's own animated close(), and calls
    // dismissNotification() -> removeNotification(item, true), the skip-animation form that
    // reaches _removeNotificationElement() synchronously.
    //
    // The bar is judged GONE FROM THE DOM, which is the whole point: the default close path
    // only sets margin-top/opacity and finishes in a transitionend handler, so a bar that has
    // merely stopped being visible is not a dismissed bar.
    bool dismissTapped = clickInView(page, "sel:#msgbar notification button.notification-button");
    printf("[prefsui] tap Dismiss ok=%d\n", (int)dismissTapped);
    std::string afterTap = DebugElementRect("sel:#msgbar notification");
    check(has(afterTap, "(no element)"),
          "T139c1 Dismiss leaves NO <notification> in the DOM", afterTap);

    std::string statusAfter = DebugElementText("#status", 300);
    check(statusAfter == kResetMsg, "T139d  #status still carries it after dismissal", statusAfter);

    // (c) cycle 2: raise it again, then dismiss by tapping the BAR itself (the plain-DOM click
    // listener announce() adds, the touch affordance). A second append succeeding at all is
    // itself worth asserting — it is the state the kit warns can be corrupted by letting the
    // animated close path run and sweeping the element out behind it.
    clickInView(page, "reset-pane");
    std::string reAppeared = DebugElementRect("sel:#msgbar notification[value=\"prefs-reset\"]");
    check(!has(reAppeared, "(no element)"),
          "T139c2 the bar can be raised again after a dismissal", reAppeared);

    bool barTapped = clickInView(page, "sel:#msgbar notification");
    printf("[prefsui] tap bar ok=%d\n", (int)barTapped);
    std::string finalDom = DebugElementRect("sel:#msgbar notification");
    check(has(finalDom, "(no element)"),
          "T139c2 tapping the bar leaves NO <notification> in the DOM", finalDom);

    // =====================================================================================
    // T-111 — cavekit-gre-widgets.md R7: the eleven row-backing prefs come from the file
    // BOTH builds append, so no row is disabled and none silently shows the stock value.
    // =====================================================================================
    std::vector<PrefRow> rows = ElevenRows();

    // Engine-side readback first: this is the value the page's own Services.prefs read sees.
    for (const PrefRow& r : rows) {
      std::string got = DebugGetPref(r.name, r.type);
      std::string what = std::string("T111p  ") + r.name + (r.wasSilentlyStock ? " (was stock)" : "");
      check(got == r.expect, what.c_str(), got + " expected=" + r.expect);
    }

    // Then the PAGE's own rendering of the same rows, per pane. The page reports its own
    // missing set: a pref this build lacks gets class="row unavailable" and the help line
    // "Not available in this build.", so `.row.unavailable .row-pref` names the first one.
    const char* panes[] = { "general", "advanced" };
    for (const char* pane : panes) {
      std::string sel = std::string("sel:#panes button[data-pane=\"") + pane + "\"]";
      bool ok = clickInView(page, sel.c_str());
      page.PumpFor(800);
      printf("[prefsui] pane %s switch ok=%d\n", pane, (int)ok);

      std::string content = DebugElementText("#content", 30000);
      check(content.size() > 200, (std::string("T111r  pane ") + pane + " rendered rows").c_str(),
            std::to_string(content.size()) + " chars");

      std::string missing = DebugElementText(".row.unavailable .row-pref", 300);
      check(missing == "(no element)",
            (std::string("T111u  pane ") + pane + ": no row reads \"not available\"").c_str(), missing);

      for (const PrefRow& r : rows) {
        if (strcmp(r.pane, pane) != 0) continue;
        std::string what = std::string("T111d  ") + pane + " renders a row for " + r.name;
        check(has(content, r.name), what.c_str(), has(content, r.name) ? "present" : "ABSENT");
      }
    }

    printf("[prefsui] checks=%d fails=%d\n", gChecks, gFails);
    printf("[prefsui] %s\n", gFails == 0 ? "PREFSUI PASS" : "PREFSUI FAIL");
    rc = (gFails == 0) ? 0 : 4;
  }
  host.Shutdown();
  return rc;
}
