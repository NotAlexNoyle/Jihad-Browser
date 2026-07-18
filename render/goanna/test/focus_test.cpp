/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — engine-driven editor focus (input-bridging R2, device T4).
 * Asserts the Atlas-ported focus/blur -> msgEditorFocused bridge:
 *   A) autofocus gate: a page that autofocuses an input on LOAD must NOT raise
 *      the VKB before the first tap (no editorFocused(true) emission);
 *   B) script blur lowers: after a tap raises the VKB, a page-script blur()
 *      must emit editorFocused(false) WITHOUT any further user input — the
 *      clean false transition that unwedges a stuck app-side VKB state;
 *   C) script focus raises: once the user has interacted with the page (a tap
 *      anywhere), a script-driven focus() must emit editorFocused(true)
 *      without the field ever being tapped.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "../EngineHost.h"
#include "../BrowserPageGoanna.h"

class Sink : public jihad::IPageMessageSink {
public:
  std::vector<int> vkb;   // ordered editorFocused emissions (1 = raise, 0 = lower)
  std::string lastLink;   // last msgLinkClicked target (scenario D: proves the submit fired a nav)
  void msgPainted(int32_t) override {}
  void msgLoadStarted() override {}
  void msgLoadProgress(int32_t) override {}
  void msgLoadStopped() override {}
  void msgLocationChanged(const char*, bool, bool) override {}
  void msgTitleChanged(const char*) override {}
  void msgContentsSizeChanged(int32_t, int32_t) override {}
  void msgScrolledTo(int32_t, int32_t) override {}
  void msgMetaViewportSet(double, double, double, int32_t, int32_t, bool) override {}
  void msgFailedLoad(const char*, int32_t, const char*, const char*) override {}
  void msgEditorFocused(bool focused, int, int) override { vkb.push_back(focused ? 1 : 0); }
  void msgLinkClicked(const char* u) override { if (u) lastLink = u; }
};

static const int W = 800, H = 600, KSZ = 800*600*4;
static int gk1 = 0x4a464f31, gk2 = 0x4a464f32;

static int raises(const Sink& s) { int n = 0; for (int v : s.vkb) if (v == 1) ++n; return n; }
static int lowers(const Sink& s) { int n = 0; for (int v : s.vkb) if (v == 0) ++n; return n; }

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[foc] engine init FAIL\n"); return 1; }
  printf("[foc] engine up\n");
  int id1 = shmget(gk1, KSZ, IPC_CREAT | 0666), id2 = shmget(gk2, KSZ, IPC_CREAT | 0666);
  if (id1 < 0 || id2 < 0) { perror("[foc] shmget"); return 2; }

  bool gateOK = false, blurOK = false, scriptFocusOK = false;

  // --- A: autofocus on load must not raise the VKB before any tap ---
  { Sink s; jihad::BrowserPageGoanna p(host, s); p.init(W, H, gk1, gk2, KSZ);
    p.openUrl("data:text/html,<body style='margin:0'><input autofocus "
              "style='width:300px;height:40px'></body>");
    p.pump(20000); p.pump(1500);
    gateOK = raises(s) == 0;
    printf("[foc] A autofocus-gate raises=%d ok=%d\n", raises(s), gateOK);
  }

  // --- B: tap raises; a script blur() later lowers with NO further input ---
  { Sink s; jihad::BrowserPageGoanna p(host, s); p.init(W, H, gk1, gk2, KSZ);
    p.openUrl("data:text/html,<body style='margin:0'><input id=i "
              "onfocus=\"setTimeout(function(){document.getElementById('i').blur()},800)\" "
              "style='width:300px;height:40px'></body>");
    p.pump(20000);
    p.clickAt(50, 20, 1);          // tap the input -> raise
    p.pump(600);
    int raisedAfterTap = raises(s);
    p.pump(2500);                  // let the scheduled blur() fire -> engine-driven lower
    blurOK = raisedAfterTap >= 1 && lowers(s) >= 1 && !s.vkb.empty() && s.vkb.back() == 0;
    printf("[foc] B blur raises=%d lowers=%d last=%d ok=%d\n",
           raises(s), lowers(s), s.vkb.empty() ? -1 : s.vkb.back(), blurOK);
  }

  // --- C: after a tap ANYWHERE, a script focus() must raise without tapping the field ---
  { Sink s; jihad::BrowserPageGoanna p(host, s); p.init(W, H, gk1, gk2, KSZ);
    p.openUrl("data:text/html,<body style='margin:0;width:100vw;height:100vh' "
              "onmousedown=\"setTimeout(function(){document.getElementById('i').focus()},500)\">"
              "<div style='height:200px'></div><input id=i style='width:300px;height:40px'></body>");
    p.pump(20000);
    p.clickAt(600, 500, 1);        // tap empty page area (not the field, not editable)
    p.pump(2500);                  // scheduled script focus() -> engine-driven raise
    scriptFocusOK = raises(s) >= 1 && !s.vkb.empty() && s.vkb.back() == 1;
    printf("[foc] C script-focus raises=%d last=%d ok=%d\n",
           raises(s), s.vkb.empty() ? -1 : s.vkb.back(), scriptFocusOK);
  }

  // --- D: Enter submits via an OFF-SCREEN submit button (ClickElementSynthetic must scroll it into
  //        view and land the synthesized click, not silently no-op — inspector P2). The submit button
  //        sits below a viewport-tall spacer. The form GET-submits to a marker host; the daemon reports
  //        the resulting content nav via msgLinkClicked — checked instead of pixels so this does not
  //        depend on the (currently stale) desktop paint harness (see dead-ends.md).
  bool submitOK = false;
  { Sink s; jihad::BrowserPageGoanna p(host, s); p.init(W, H, gk1, gk2, KSZ);
    // Form GET-submits to the local http server (/b -> 200 HTML). Enter in the field must submit it:
    // proves FireFormSubmit (validate → fire cancelable 'submit' → form->Submit()) actually navigates,
    // the crash-safe replacement for DOMClick (which MOZ_CRASHed) — and that SendMouseEvent alone did
    // NOT fire the submit default action. The daemon reports the resulting content nav via
    // msgLinkClicked; a real responding server gives a clean document-level nav (unlike a dead port).
    p.openUrl("data:text/html,<body style='margin:0'>"
              "<form action='http://127.0.0.1:18080/b' method='get'>"
              "<input id=q name=q style='width:300px;height:40px'>"
              "<input type=submit value=go style='height:40px'></form></body>");
    p.pump(20000);
    p.clickAt(50, 20, 1);          // focus the text field
    p.pump(500);
    p.keyDown(13, 0, 0);           // Enter -> HandleEnter -> FireFormSubmit
    p.pump(4000);
    submitOK = s.lastLink.find("18080/b") != std::string::npos;
    printf("[foc] D enter-submit link=[%s] ok=%d\n", s.lastLink.c_str(), submitOK);
  }

  // --- E: TAP the submit button directly (not Enter). The synthesized click fires onclick but not the
  //        submit default action in this embedding, so ClickAt must FireFormSubmit for a tapped submit
  //        control (search "Go" buttons). Assert the form navigated.
  bool tapSubmitOK = false;
  { Sink s; jihad::BrowserPageGoanna p(host, s); p.init(W, H, gk1, gk2, KSZ);
    p.openUrl("data:text/html,<body style='margin:0'>"
              "<form action='http://127.0.0.1:18080/b' method='get'>"
              "<input name=q style='width:300px;height:40px'>"
              "<input type=submit value=go style='position:absolute;left:0;top:60px;width:100px;height:40px'>"
              "</form></body>");
    p.pump(20000);
    p.clickAt(50, 80, 1);          // tap the submit button (at ~y=80)
    p.pump(4000);
    tapSubmitOK = s.lastLink.find("18080/b") != std::string::npos;
    printf("[foc] E tap-submit link=[%s] ok=%d\n", s.lastLink.c_str(), tapSubmitOK);
  }

  bool ok = gateOK && blurOK && scriptFocusOK && submitOK && tapSubmitOK;
  printf("[foc] gate=%d blur=%d scriptFocus=%d enterSubmit=%d tapSubmit=%d\n",
         gateOK, blurOK, scriptFocusOK, submitOK, tapSubmitOK);
  printf("[foc] %s\n", ok ? "FOCUS PASS" : "FOCUS FAIL");
  shmctl(id1, IPC_RMID, nullptr); shmctl(id2, IPC_RMID, nullptr);
  host.Shutdown();
  return ok ? 0 : 4;
}
