/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — XUL/chrome input test (cavekit-input-bridging R6, T-067/T-068).
 *
 * WHY THIS EXISTS
 * Synthesized mouse input on a XUL document SIGSEGV'd the daemon on-device, and the
 * shipped mitigation was to SKIP the click, leaving about:config/about:addons inert.
 * The device round-trip is ~5 minutes; this harness reproduces the same path on the
 * desktop engine in seconds so the fault can be diagnosed in a local loop.
 *
 * The last device-only bug in this project turned out to be an ENVIRONMENT difference
 * ($HOME set by the container, unset by upstart), so this test deliberately drives the
 * exact same GoannaRenderPage entry points the daemon does, under the same
 * JIHAD_OFFSCREEN PuppetWidget path, rather than a bespoke XPCOM sequence.
 *
 * Phases:
 *   A  chrome:// reachability — can the content docShell load a chrome XUL document at
 *      all? (about:config and about:addons are both chrome XUL, so a shared setup defect
 *      would show up here first.)
 *   B  about:config renders.
 *   C  a synthesized click on the XUL document — the crash under investigation.
 *   D  XUL default action: does the warning button's oncommand actually run (the prefs
 *      list replaces the warning screen)?
 *   E  keyboard into the XUL filter box.
 *   F  repeated taps across the surface (R6 "under repeated taps across a full session").
 *   G  HTML control sanity in the SAME process (is any of this XUL-specific?).
 *   H  which of mousedown/mouseup/click actually arrives.
 *   I  an HTML checkbox toggles exactly once through ClickAt alone.
 *   J  about:addons (informational — known broken for a separate, named reason).
 *   K  ONE TAP = ONE CLICK SEQUENCE: the adapter's pen path (mouseEvent down/up) AND the
 *      single-tap gesture (clickAt) both fire on the same tap; the DOM must still see one
 *      activation (F-1).
 *   L  a page that DECLINES a click (preventDefault, onsubmit returning false, a disabled
 *      button) must not have it re-run by the daemon's fallbacks (F-2 / F-3).
 *
 * F-5 — THIS HARNESS MUST BE ABLE TO FAIL. It previously initialised `rc = 0` and never
 * assigned it, and the runner script printed the exit status as its last command, so a
 * SIGSEGV became script-exit 0. Every phase below that has a pass condition now ORs a
 * distinct bit into `rc`, the bits are printed on exit, and build-xul-test.sh propagates
 * the status. THE FAILURE PATH IS DEMONSTRATED, three ways (2026-08-01):
 *   - against the UNFIXED daemon, phase K printed "double-toggled back to UNCHECKED",
 *     rc=128, script exit 128 — a real defect, found by this harness before the fix;
 *   - with the F-2/F-3 delivery gates deliberately removed, L1/L2/L3 all tripped, rc=384;
 *   - with a deliberate null dereference after phase A, the runner exited 139 (SIGSEGV),
 *     which is the case the old runner reported as success.
 */
// JIHAD_OFFSCREEN_ONLY is the DEVICE build flag (build-daemon-arm.sh): GTK-free, offscreen
// PuppetWidget only. Honouring it here is what makes this test a real device proxy — with GTK
// linked in, gtk_init() needs a DISPLAY, and running under Xvfb would put an X server back into
// a configuration whose entire point is not having one.
#ifndef JIHAD_OFFSCREEN_ONLY
#include <gtk/gtk.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"
#include "../JihadCrashReport.h"

static long countNonWhite(unsigned char* buf, int n) {
  long c = 0;
  for (int i = 0; i < n; ++i) {
    unsigned char* p = buf + (size_t)i * 4;   // B,G,R,A
    if (p[0] < 240 || p[1] < 240 || p[2] < 240) ++c;
  }
  return c;
}

// Dump the framebuffer so the click target can be chosen from the real render instead
// of guessed at (the XUL warning box is centered and its size depends on the DTD text).
static void dumpPpm(const char* path, unsigned char* buf, int w, int h) {
  FILE* f = fopen(path, "wb");
  if (!f) return;
  fprintf(f, "P6\n%d %d\n255\n", w, h);
  for (int i = 0; i < w * h; ++i) {
    unsigned char* p = buf + (size_t)i * 4;
    unsigned char rgb[3] = { p[2], p[1], p[0] };
    fwrite(rgb, 1, 3, f);
  }
  fclose(f);
  printf("[xul] wrote %s\n", path);
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
#ifndef JIHAD_OFFSCREEN_ONLY
  gtk_init(&argc, &argv);
#endif
  jihad::JihadInstallCrashHandler();          // pre-engine: covers bring-up
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[xul] engine init FAIL\n"); return 1; }
  jihad::JihadInstallCrashHandler();          // MANDATORY re-arm: SpiderMonkey replaced ours
  printf("[xul] engine up\n");

  // F-5: one bit per phase, ORed in on that phase's FAILURE condition, printed on exit and
  // returned as the process status. Bits (not a count) so a run says WHICH phases failed.
  enum { F_A = 1, F_B = 2, F_D = 4, F_E = 8, F_G = 16, F_H = 32, F_I = 64, F_K = 128,
         F_L = 256 };
  int rc = 0;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) { fprintf(stderr, "[xul] create FAIL\n"); return 2; }
    const int N = page.Width() * page.Height();
    size_t sz = (size_t)N * 4;
    unsigned char* buf = (unsigned char*)malloc(sz);

    // ---- Phase A: can a content docShell reach chrome:// at all? -----------------
    printf("[xul] --- A: chrome:// reachability ---\n");
    bool okChrome = page.LoadUrlAndWait("chrome://global/content/config.xul", 30);
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    long nwA = countNonWhite(buf, N);
    printf("[xul] A: load=%d uri=%s title=%s nonwhite=%ld\n",
           (int)okChrome, page.CurrentUri().c_str(), page.GetTitle().c_str(), nwA);
    // A chrome XUL document that loads but paints nothing is a setup defect too, so both
    // halves gate: the load has to succeed AND put ink on the page (F-5).
    if (!okChrome || nwA < 1000) { rc |= F_A; printf("[xul] A: FAIL (rc|=%d)\n", F_A); }

    // ---- Phase B: about:config renders -------------------------------------------
    printf("[xul] --- B: about:config ---\n");
    bool okCfg = page.LoadUrlAndWait("about:config", 30);
    page.PumpFor(2000);
    page.ReadPixels(buf, sz);
    long nw = countNonWhite(buf, N);
    printf("[xul] B: load=%d uri=%s title=%s nonwhite=%ld\n",
           (int)okCfg, page.CurrentUri().c_str(), page.GetTitle().c_str(), nw);
    dumpPpm(getenv("JIHAD_XUL_PPM") ? getenv("JIHAD_XUL_PPM") : "/out/xul-config.ppm",
            buf, page.Width(), page.Height());
    if (!okCfg || nw < 1000 || page.CurrentUri() != "about:config") {
      rc |= F_B; printf("[xul] B: FAIL (rc|=%d)\n", F_B);
    }

    // ---- Phase C: the click that crashes ------------------------------------------
    // Coordinates come from JIHAD_XUL_CLICK="x,y" so the same binary can probe the
    // warning button, the checkbox, and empty chrome without a rebuild.
    // F-5: the DEFAULT is the measured centre of about:config's "I promise to be careful!"
    // button in this 1024x768 render (read off /out/xul-config.ppm). It used to be the
    // viewport centre (512,384) — empty chrome — so phases D and E could never observe the
    // XUL default action or the filter box, and the "about:config is operable" claim rested
    // on a hand-passed JIHAD_XUL_CLICK that the default run did not reproduce. If the DTD
    // text or font metrics ever move the button, D fails loudly and the ppm dump says where
    // it went; that is the intended behaviour, not a reason to soften the check.
    int cx = 236, cy = 394;
    if (const char* cs = getenv("JIHAD_XUL_CLICK")) sscanf(cs, "%d,%d", &cx, &cy);
    printf("[xul] --- C: click at %d,%d (this is the crash under test) ---\n", cx, cy);
    fflush(stdout);
    page.ClickAt(cx, cy, 1);
    printf("[xul] C: SURVIVED the click\n");
    page.PumpFor(2500);

    // ---- Phase D: did the XUL default action run? ----------------------------------
    // ShowPrefs() swaps the <deck> from the warning screen to the prefs tree, which is a
    // massive visual change — a pixel delta is the crash-proof way to observe it.
    page.ReadPixels(buf, sz);
    long nw2 = countNonWhite(buf, N);
    printf("[xul] D: nonwhite before=%ld after=%ld delta=%ld\n", nw, nw2, nw2 - nw);
    dumpPpm(getenv("JIHAD_XUL_PPM2") ? getenv("JIHAD_XUL_PPM2") : "/out/xul-config-after.ppm",
            buf, page.Width(), page.Height());
    // The warning deck (a few lines of text, ~24k ink) is replaced by the full prefs tree
    // (~113k ink), so the threshold is nowhere near either state — it cannot be tripped by
    // an anti-aliasing difference, and it cannot pass on a page that did not change.
    if ((nw2 - nw) < 20000) { rc |= F_D; printf("[xul] D: FAIL — XUL oncommand did not run (rc|=%d)\n", F_D); }

    // ---- Phase E: keyboard into the XUL filter box (R6 AC6) --------------------------
    // Only meaningful once the prefs deck is showing, i.e. after the warning button was
    // activated in phase C. Tap the search box first — that is what gives the anonymous
    // html:input inside the XUL <textbox> focus — then type. Filtering to a rare substring
    // collapses a ~3000-row tree to a handful, which is an unmistakable pixel change.
    printf("[xul] --- E: keyboard into the XUL filter box ---\n");
    page.ClickAt(400, 20, 1);
    page.PumpFor(600);
    page.InsertText("zzzznosuchpref");
    page.PumpFor(2000);
    page.ReadPixels(buf, sz);
    long eAfter = countNonWhite(buf, N);
    printf("[xul] E: nonwhite=%ld (was %ld) -> filter %s\n",
           eAfter, nw2, (nw2 - eAfter) > 20000 ? "APPLIED" : "did NOT apply");
    dumpPpm(getenv("JIHAD_XUL_PPM3") ? getenv("JIHAD_XUL_PPM3") : "/out/xul-config-typed.ppm",
            buf, page.Width(), page.Height());
    // NB what this does and does not prove (R6 AC6, F-6): the text got in via InsertText —
    // the ENGINE EDITOR path (SetValue/SetSelectionRange), the same one HTML fields use.
    // It is NOT proof that key events reach XUL: KeyEvent still goes through
    // nsDOMWindowUtils::SendKeyEvent -> the widget -> PuppetWidget drops it.
    if ((nw2 - eAfter) <= 20000) { rc |= F_E; printf("[xul] E: FAIL (rc|=%d)\n", F_E); }

    // ---- Phase F: repeated taps (R6 "under repeated taps across a full session") -----
    printf("[xul] --- F: 12 repeated taps across the surface ---\n");
    for (int i = 0; i < 12; ++i) {
      page.ClickAt(80 + (i * 71) % 900, 60 + (i * 53) % 650, 1);
      page.PumpFor(120);
    }
    printf("[xul] F: survived 12 taps\n");

    // ---- Phase G: HTML control, SAME binary and SAME engine configuration -------------
    // The discriminator. If a synthesized click fires onclick on HTML here but nothing fires
    // on XUL above, the defect is XUL-specific. If HTML is dead too, the defect is in the
    // synthesis and XUL was never special. Guessing between those two costs a whole session,
    // so the test answers it in the same process rather than across two harnesses.
    printf("[xul] --- G: HTML control sanity (same engine) ---\n");
    page.LoadUrlAndWait(
      "data:text/html,<body onclick=\"document.body.style.background='%2300ff00'\" "
      "style='margin:0;width:100vw;height:100vh;background:%230000ff'></body>", 20);
    page.PumpFor(1200);
    page.ReadPixels(buf, sz);
    long gBefore = countNonWhite(buf, N);
    page.ClickAt(512, 384, 1);
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    // blue -> green is a colour change, not a coverage change, so count green explicitly.
    long green = 0;
    for (int i = 0; i < N; ++i) {
      unsigned char* p = buf + (size_t)i * 4;
      if (p[2] < 80 && p[1] > 180 && p[0] < 80) ++green;
    }
    printf("[xul] G: nonwhite=%ld green_after_click=%ld -> HTML onclick %s\n",
           gBefore, green, green > 100000 ? "FIRED" : "DID NOT FIRE");
    if (green <= 100000) { rc |= F_G; printf("[xul] G: FAIL (rc|=%d)\n", F_G); }

    // ---- Phase H: WHICH of mousedown / mouseup / click is actually delivered? ----------
    // Three stacked bands, each repainted by one capture-phase listener. One click, and the
    // colours say exactly how far the synthesis gets: no red = the mousedown never arrived;
    // red+green but no blue = the events arrive but the engine never synthesises the click
    // (which is the difference between "input is broken" and "the click default action is
    // broken", and those need completely different fixes).
    printf("[xul] --- H: which events arrive? ---\n");
    // NB: every '#' is written as %23. SetHtml/LoadUrl paste the body into a data: URL with no
    // escaping, so a literal '#' starts the URL FRAGMENT and silently truncates the document
    // there — which is exactly how the first version of this phase reported "no events" on a
    // build where events demonstrably worked. A probe that can fail open is worse than none.
    page.LoadUrlAndWait(
      "data:text/html,<body style='margin:0'>"
      "<div id='d1' style='height:200px;background:%23888888'></div>"
      "<div id='d2' style='height:200px;background:%23888888'></div>"
      "<div id='d3' style='height:200px;background:%23888888'></div>"
      "<script>"
      "document.addEventListener('mousedown',function(){d1.style.background='%23ff0000'},true);"
      "document.addEventListener('mouseup',  function(){d2.style.background='%2300ff00'},true);"
      "document.addEventListener('click',    function(){d3.style.background='%230000ff'},true);"
      "</script></body>", 20);
    page.PumpFor(1500);
    page.ClickAt(300, 300, 1);       // inside d2, well away from the band edges
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    long r = 0, g = 0, b = 0;
    for (int i = 0; i < N; ++i) {
      unsigned char* p = buf + (size_t)i * 4;
      if (p[2] > 180 && p[1] < 80 && p[0] < 80) ++r;
      if (p[2] < 80 && p[1] > 180 && p[0] < 80) ++g;
      if (p[2] < 80 && p[1] < 80 && p[0] > 180) ++b;
    }
    printf("[xul] H: mousedown=%s mouseup=%s click=%s  (px r=%ld g=%ld b=%ld)\n",
           r > 5000 ? "YES" : "no", g > 5000 ? "YES" : "no", b > 5000 ? "YES" : "no", r, g, b);
    // All three, or the T-067 dropped-event defect (or a partial regression of it) is back.
    if (r <= 5000 || g <= 5000 || b <= 5000) { rc |= F_H; printf("[xul] H: FAIL (rc|=%d)\n", F_H); }

    // ---- Phase I: HTML checkbox must toggle EXACTLY ONCE (R6 AC7, no HTML regression) ----
    // The real hazard of making the engine's click default action work: the daemon ALSO
    // hand-flips checkboxes (R1/F-001), so a working default action would toggle twice and
    // land back where it started — silently converting the R1 fix into the bug it cured.
    printf("[xul] --- I: HTML checkbox toggles exactly once ---\n");
    page.LoadUrlAndWait(
      "data:text/html,<body style='margin:0;background:%23ffffff'>"
      "<input id='c' type='checkbox' onchange=\"document.body.style.background="
      "this.checked?'%2300ff00':'%23ff0000'\" style='width:200px;height:200px'>"
      "</body>", 20);
    page.PumpFor(1200);
    page.ClickAt(100, 100, 1);
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    long onGreen = 0, onRed = 0;
    for (int i = 0; i < N; ++i) {
      unsigned char* p = buf + (size_t)i * 4;
      if (p[2] < 80 && p[1] > 180 && p[0] < 80) ++onGreen;
      if (p[2] > 180 && p[1] < 80 && p[0] < 80) ++onRed;
    }
    printf("[xul] I: after one tap green=%ld red=%ld -> %s\n", onGreen, onRed,
           onGreen > 100000 ? "CHECKED once (correct)"
           : (onRed > 100000 ? "UNCHECKED (double-toggled - REGRESSION)"
                             : "no change event at all"));
    if (onGreen <= 100000) { rc |= F_I; printf("[xul] I: FAIL (rc|=%d)\n", F_I); }

    // ---- Phase K: ONE TAP = ONE CLICK SEQUENCE (F-1) -----------------------------------
    // Phase I drives clickAt ALONE, which is not what a device tap looks like. The adapter
    // forwards its pen path — asyncCmdMouseEvent(down) then (up) — whenever
    // shouldPassInputEvents() is true, and that is true for every page whose content fits the
    // viewport with a non-scalable meta-viewport, i.e. most mobile pages, NOT only when the tap
    // hits a known interactive rect. Jihad then ALSO sends asyncCmdClickAt from the single-tap
    // gesture. Before T-067 the raw pair was discarded by the un-attached PuppetWidget so only
    // clickAt acted; now both act, and one tap delivered two click sequences (checkbox toggles
    // twice = net zero + two `change` events; forms double-submit; links race two loads).
    // Replay the real wire order: down, up, a pump (the daemon defers clickAt to the tick), click.
    printf("[xul] --- K: one tap must deliver ONE click sequence ---\n");
    const char* kCheckboxPage =
      "data:text/html,<body style='margin:0;background:%23ffffff'>"
      "<input id='c' type='checkbox' onchange=\"document.body.style.background="
      "this.checked?'%2300ff00':'%23ff0000'\" style='width:200px;height:200px'>"
      "</body>";
    // K1: the raw pen pair ALONE must activate the control. If this fails, deduplicating in
    // favour of the raw path would leave the control dead, and the fix belongs elsewhere.
    page.LoadUrlAndWait(kCheckboxPage, 20);
    page.PumpFor(1200);
    page.MouseEvent("mousedown", 100, 100, 0);
    page.MouseEvent("mouseup",   100, 100, 0);
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    long k1Green = 0, k1Red = 0;
    for (int i = 0; i < N; ++i) {
      unsigned char* p = buf + (size_t)i * 4;
      if (p[2] < 80 && p[1] > 180 && p[0] < 80) ++k1Green;
      if (p[2] > 180 && p[1] < 80 && p[0] < 80) ++k1Red;
    }
    printf("[xul] K1: raw mouse pair only -> green=%ld red=%ld (%s)\n", k1Green, k1Red,
           k1Green > 100000 ? "CHECKED" : "not checked");
    // K2: the real tap — raw pair AND clickAt. Exactly one net toggle.
    page.LoadUrlAndWait(kCheckboxPage, 20);
    page.PumpFor(1200);
    page.MouseEvent("mousedown", 100, 100, 0);
    page.MouseEvent("mouseup",   100, 100, 0);
    page.PumpFor(200);
    page.ClickAt(100, 100, 1);
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    long k2Green = 0, k2Red = 0;
    for (int i = 0; i < N; ++i) {
      unsigned char* p = buf + (size_t)i * 4;
      if (p[2] < 80 && p[1] > 180 && p[0] < 80) ++k2Green;
      if (p[2] > 180 && p[1] < 80 && p[0] < 80) ++k2Red;
    }
    printf("[xul] K2: pen pair + clickAt -> green=%ld red=%ld -> %s\n", k2Green, k2Red,
           k2Green > 100000 ? "CHECKED once (correct)"
           : (k2Red > 100000 ? "double-toggled back to UNCHECKED (F-1)"
                             : "no change event at all"));
    if (k1Green <= 100000 || k2Green <= 100000) {
      rc |= F_K; printf("[xul] K: FAIL (rc|=%d)\n", F_K);
    }
    // K3 (MEASUREMENT, not a gate): does the engine's own click default action navigate an
    // anchor off the raw pen pair alone? That is what decides whether clickAt's direct-navigate
    // may be skipped when the pair already delivered a click, or whether skipping it would make
    // links dead on every shouldPassInputEvents() page. Local files, because data: cannot link
    // to anything the loader will follow and the harness has no HTTP server.
    {
      FILE* fa = fopen("/out/xul-link-a.html", "w");
      if (fa) { fputs("<body style='margin:0'><a href='xul-link-b.html' "
                      "style='display:block;width:100vw;height:100vh;background:#4444ff'>go</a></body>", fa);
               fclose(fa); }
      FILE* fb = fopen("/out/xul-link-b.html", "w");
      if (fb) { fputs("<body style='margin:0;background:#00aa00'>B</body>", fb); fclose(fb); }
      page.LoadUrlAndWait("file:///out/xul-link-a.html", 20);
      page.PumpFor(1000);
      page.MouseEvent("mousedown", 400, 300, 0);
      page.MouseEvent("mouseup",   400, 300, 0);
      page.PumpFor(3000);
      std::string after = page.CurrentUri();
      printf("[xul] K3: raw mouse pair on an <a href> -> uri=%s -> engine %s\n",
             after.c_str(), after.find("xul-link-b") != std::string::npos
                            ? "NAVIGATED (clickAt must not navigate again)"
                            : "did NOT navigate (clickAt keeps its direct navigate)");
    }

    // ---- Phase L: the fallbacks must not fire when the page DECLINED (F-2 / F-3) --------
    // Both fallbacks used to infer "the engine did not act" from state — unchanged `checked`,
    // nothing loading — and both conditions are also what a deliberate refusal looks like. UXP
    // even REVERTS its pre-toggle on eConsumeNoDefault (HTMLInputElement.cpp:4483-4505), so a
    // preventDefault()ed click is byte-identical to a dropped one. Each sub-phase below is a page
    // that refuses, and each one is repainted BY THE HANDLER THAT MUST NOT RUN TWICE, so the
    // failure is visible in pixels rather than inferred.
    printf("[xul] --- L: cancelled clicks must not be re-run by the fallbacks ---\n");
    long lRed = 0, lGreen = 0;
    // L1: a checkbox whose click handler returns false. The control must stay unchecked and NO
    // change event may fire — the old fallback hand-flipped it and dispatched input+change.
    page.LoadUrlAndWait(
      "data:text/html,<body style='margin:0;background:%23ffffff'>"
      "<input id='c' type='checkbox' onclick='return false' "
      "onchange=\"document.body.style.background='%23ff0000'\" "
      "style='width:200px;height:200px'></body>", 20);
    page.PumpFor(1200);
    page.ClickAt(100, 100, 1);
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    lRed = 0;
    for (int i = 0; i < N; ++i) {
      unsigned char* p = buf + (size_t)i * 4;
      if (p[2] > 180 && p[1] < 80 && p[0] < 80) ++lRed;
    }
    printf("[xul] L1: preventDefault()ed checkbox -> phantom change %s (red=%ld)\n",
           lRed > 1000 ? "FIRED (F-2 REGRESSION)" : "not fired (correct)", lRed);
    if (lRed > 1000) { rc |= F_L; printf("[xul] L1: FAIL (rc|=%d)\n", F_L); }
    // L2: onsubmit returns false. The handler paints green the first time and red the second, so
    // the double-submit the unconditional fallback caused is a colour, not an inference.
    page.LoadUrlAndWait(
      "data:text/html,<body style='margin:0;background:%23ffffff'>"
      "<form onsubmit=\"window.n=(window.n||0)+1;document.body.style.background="
      "window.n==1?'%2300ff00':'%23ff0000';return false\">"
      "<button type='submit' style='width:100vw;height:100vh'>go</button></form></body>", 20);
    page.PumpFor(1200);
    page.ClickAt(400, 300, 1);
    page.PumpFor(2000);
    page.ReadPixels(buf, sz);
    lRed = 0; lGreen = 0;
    for (int i = 0; i < N; ++i) {
      unsigned char* p = buf + (size_t)i * 4;
      if (p[2] > 180 && p[1] < 80 && p[0] < 80) ++lRed;
      if (p[2] < 80 && p[1] > 180 && p[0] < 80) ++lGreen;
    }
    printf("[xul] L2: onsubmit=false -> green=%ld red=%ld -> handler ran %s\n", lGreen, lRed,
           lRed > 1000 ? "TWICE (F-3 REGRESSION)" : (lGreen > 1000 ? "once (correct)" : "not at all"));
    if (lRed > 1000 || lGreen <= 1000) { rc |= F_L; printf("[xul] L2: FAIL (rc|=%d)\n", F_L); }
    // L3: a DISABLED submit button. It receives no click at all, which is why "no click arrived"
    // alone cannot license the fallback — the classifier has to ask GetDisabled.
    page.LoadUrlAndWait(
      "data:text/html,<body style='margin:0;background:%2300ff00'>"
      "<form onsubmit=\"document.body.style.background='%23ff0000';return false\">"
      "<button type='submit' disabled style='width:100vw;height:100vh'>go</button></form></body>", 20);
    page.PumpFor(1200);
    page.ClickAt(400, 300, 1);
    page.PumpFor(2000);
    page.ReadPixels(buf, sz);
    lRed = 0;
    for (int i = 0; i < N; ++i) {
      unsigned char* p = buf + (size_t)i * 4;
      if (p[2] > 180 && p[1] < 80 && p[0] < 80) ++lRed;
    }
    printf("[xul] L3: disabled submit button -> form %s (red=%ld)\n",
           lRed > 1000 ? "SUBMITTED (F-3 REGRESSION)" : "not submitted (correct)", lRed);
    if (lRed > 1000) { rc |= F_L; printf("[xul] L3: FAIL (rc|=%d)\n", F_L); }

    // ---- Phase J: about:addons (R6 AC4 / cavekit-addons-extensions R2) -----------------
    // Recorded 2026-07-20 as rendering a <parsererror> in the stripped build, cause unknown.
    // With the chrome-JS console bridged (see EngineHost.cpp) the engine now says why, instead
    // of us inferring it from a screenshot.
    printf("[xul] --- J: about:addons ---\n");
    bool okAddons = page.LoadUrlAndWait("about:addons", 30);
    page.PumpFor(2500);
    page.ReadPixels(buf, sz);
    printf("[xul] J: load=%d uri=%s title=%s nonwhite=%ld\n", (int)okAddons,
           page.CurrentUri().c_str(), page.GetTitle().c_str(), countNonWhite(buf, N));
    dumpPpm("/out/xul-addons.ppm", buf, page.Width(), page.Height());
    page.ClickAt(200, 200, 1);
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    printf("[xul] J: after tap nonwhite=%ld (survived)\n", countNonWhite(buf, N));
    dumpPpm("/out/xul-addons-after.ppm", buf, page.Width(), page.Height());
    // J is deliberately NOT gated: about:addons renders a <parsererror> for a separate, named
    // reason (`No chrome package registered for chrome://branding/locale/brand.dtd` — the
    // branding strip, not input), so gating it here would make this harness fail for something
    // it does not test. Surviving the tap is still checked — by the process not dying.

    free(buf);
  }
  host.Shutdown();
  // F-5: the bitmask is the DIAGNOSIS and it is printed; the exit STATUS is only pass/fail.
  // Returning the mask itself would lie: a process status is 8 bits, so the first demo run of this
  // harness printed rc=384 (K|L) and the script reported exit 128 — bit 8 silently gone, and 128+
  // collides with the shell's own 128+signal encoding for a crash. Small status, full mask in the
  // log. Phases C/F/J have no pass condition: a crash there is reported by the process dying (the
  // runner propagates that status), not by rc.
  printf("[xul] done rc=%d %s\n", rc, rc ? "FAIL" : "PASS");
  return rc ? 1 : 0;
}
