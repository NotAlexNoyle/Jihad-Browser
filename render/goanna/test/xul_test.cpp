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
    printf("[xul] A: load=%d uri=%s title=%s nonwhite=%ld\n",
           (int)okChrome, page.CurrentUri().c_str(), page.GetTitle().c_str(),
           countNonWhite(buf, N));

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

    // ---- Phase C: the click that crashes ------------------------------------------
    // Coordinates come from JIHAD_XUL_CLICK="x,y" so the same binary can probe the
    // warning button, the checkbox, and empty chrome without a rebuild.
    int cx = 512, cy = 384;
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

    // ---- Phase E: keyboard into the XUL filter box ----------------------------------
    printf("[xul] --- E: keyboard ---\n");
    page.InsertText("javascript");
    page.PumpFor(1500);
    page.ReadPixels(buf, sz);
    printf("[xul] E: nonwhite=%ld\n", countNonWhite(buf, N));
    dumpPpm(getenv("JIHAD_XUL_PPM3") ? getenv("JIHAD_XUL_PPM3") : "/out/xul-config-typed.ppm",
            buf, page.Width(), page.Height());

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

    // ---- Phase H: WHICH of mousedown / mouseup / click is actually delivered? ----------
    // Three stacked bands, each repainted by one capture-phase listener. One click, and the
    // colours say exactly how far the synthesis gets: no red = the mousedown never arrived;
    // red+green but no blue = the events arrive but the engine never synthesises the click
    // (which is the difference between "input is broken" and "the click default action is
    // broken", and those need completely different fixes).
    printf("[xul] --- H: which events arrive? ---\n");
    page.SetHtml(
      "<body style='margin:0'>"
      "<div id='d1' style='height:200px;background:#888'></div>"
      "<div id='d2' style='height:200px;background:#888'></div>"
      "<div id='d3' style='height:200px;background:#888'></div>"
      "<script>"
      "document.addEventListener('mousedown',function(){d1.style.background='#ff0000'},true);"
      "document.addEventListener('mouseup',  function(){d2.style.background='#00ff00'},true);"
      "document.addEventListener('click',    function(){d3.style.background='#0000ff'},true);"
      "</script></body>");
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

    free(buf);
  }
  host.Shutdown();
  printf("[xul] done rc=%d\n", rc);
  return rc;
}
