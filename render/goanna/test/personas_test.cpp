/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — desktop Personas test (cavekit-engine-update.md R6, T-387).
 *
 * Personas and the about:addons pane were covered on the device only (docs/ENGINE.md §7). On a
 * fresh profile, through the real pages and real taps (the gallery's buttons have no ids, so by
 * selector), this asserts that the component and the engine's LightweightThemeManager (reached
 * through JihadEngine.jsm, R4b) wear a bundled Persona:
 *   - about:personas offers the bundled set;
 *   - a tap on the first one's Wear says "Wearing …", and the theme manager's prefs and the
 *     cards' jihad.theme.current name it;
 *   - about:addons opened on the theme view is the Personas pane: the category is called
 *     Personas and the pane's own box (the gallery link) is there.
 * JIHAD_PERSONAS_NEG=1 looks for a pane text that is never shown: the run must FAIL.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

using namespace jihad;

static int gChecks = 0, gFails = 0;
static void check(bool ok, const char* what, const std::string& got) {
  ++gChecks; if (!ok) ++gFails;
  printf("[personas] %-4s %-56s got=[%s]\n", ok ? "PASS" : "FAIL", what, got.c_str());
}
static bool has(const std::string& hay, const char* needle) { return hay.find(needle) != std::string::npos; }

// a tap on an element by selector, the viewport grown when it sits below the fold (as prefsui_test)
static bool clickInView(GoannaRenderPage& page, const char* sel) {
  std::string r = DebugElementRect(sel);
  int x = 0, y = 0, w = 0, h = 0;
  const char* sp = strstr(r.c_str(), "rect=");
  if (!sp || sscanf(sp + 5, "%d,%d %dx%d", &x, &y, &w, &h) != 4) {
    printf("[personas] clickInView: no rect for %s -> %s\n", sel, r.c_str());
    return false;
  }
  int cy = y + h / 2;
  if (cy < 0 || cy >= page.Height()) { page.Resize(page.Width(), cy + h + 120); page.PumpFor(800); }
  bool ok = DebugClickElement(sel, 1);
  page.PumpFor(2000);
  return ok;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const bool neg = getenv("JIHAD_PERSONAS_NEG") != nullptr;
  gtk_init(&argc, &argv);
  printf("[personas] START gre=%s neg=%d\n", greDir, (int)neg);
  EngineHost host;
  if (!host.Init(greDir)) { printf("[personas] engine init FAIL\n"); return 1; }
  int rc = 3;
  {
    GoannaRenderPage page(host);
    if (!page.Create(1024, 1200)) { printf("[personas] page create FAIL\n"); host.Shutdown(); return 2; }

    // the gallery: a fresh profile has the bundled Personas installed at its first start (the
    // component offers them once), none worn, so "Your Personas" lists them with Wear
    bool loaded = page.LoadUrlAndWait("about:personas", 30);
    page.PumpFor(3000);
    printf("[personas] about:personas load ok=%d title=[%s]\n", (int)loaded, page.GetTitle().c_str());
    std::string mine = DebugElementText("#mine", 3000);
    check(has(mine, "Default look") && has(mine, "Wear"), "about:personas lists the installed Personas to wear", mine.substr(0, 160));

    // wear the first one (the default look's button is the disabled "In use" while none is worn)
    bool tapped = clickInView(page, "sel:#mine button:not([disabled])");
    page.PumpFor(2000);
    std::string said = DebugElementText("#status-text", 200);
    check(tapped && has(said, "Wearing"), "a tap on Wear says it is worn", said);
    // this engine's LightweightThemeManager records the worn theme as isThemeSelected + usedThemes[0]
    std::string sel = DebugGetPref("lightweightThemes.isThemeSelected", 'b');
    check(sel == "true", "the theme manager wears it (isThemeSelected)", sel);
    std::string used = DebugGetPref("lightweightThemes.usedThemes", 's');
    check(used.size() > 2 && used[0] == '[' && has(used, "\"id\""), "it heads the theme manager's usedThemes", used.substr(0, 120));
    std::string current = DebugGetPref("jihad.theme.current", 's');
    check(!current.empty(), "the cards' jihad.theme.current names it", current.substr(0, 120));

    // about:addons on the theme view: the Personas pane
    DebugSetPref("extensions.ui.lastCategory", 's', "addons://list/theme");
    loaded = page.LoadUrlAndWait("about:addons", 30);
    page.PumpFor(3000);
    printf("[personas] about:addons load ok=%d title=[%s]\n", (int)loaded, page.GetTitle().c_str());
    std::string cat = DebugElementRect("sel:#category-theme[name=\"Personas\"]");
    check(!has(cat, "(no element)") && !has(cat, "?"), "the theme category is called Personas", cat);
    std::string box = DebugElementText("#jihad-themes-box", 500);
    const char* want = neg ? "no such pane text" : "In the gallery";
    check(has(box, want), "the pane's own box (the gallery link) is there", box.substr(0, 160));

    printf("[personas] checks=%d fails=%d\n", gChecks, gFails);
    printf("[personas] %s\n", gFails == 0 ? "PERSONAS PASS" : "PERSONAS FAIL");
    rc = gFails == 0 ? 0 : 4;
  }
  host.Shutdown();
  return rc;
}
