/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — desktop media and video-controls test (cavekit-engine-update.md R6, T-387).
 *
 * The media paths and the video controls were covered on the device only (docs/ENGINE.md §7). For
 * each clip (VP8, Theora, VP9) render/goanna/test/jihad-media-test.html plays it muted with its
 * controls and reports through its title; this asserts:
 *   - the controls' XBL binding attached (videocontrols, which our engine branch changes),
 *   - playback starts, a tap on the video (its click-to-pause area) pauses it and another resumes it,
 *   - it plays to the end: currentTime reaches the duration and frames were decoded.
 * An audio-only clip (Vorbis) is played too when the container has a sound server.
 * JIHAD_MEDIA_NEG=1 asks for a clip that does not exist: the run must FAIL.
 */
#include <gtk/gtk.h>
#include <cmath>
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
  printf("[media] %-4s %-56s got=[%s]\n", ok ? "PASS" : "FAIL", what, got.c_str());
}
static bool starts(const std::string& s, const char* p) { return s.compare(0, strlen(p), p) == 0; }

// pump until the title starts with one of the prefixes (or the time runs out); the last title
static std::string waitTitle(GoannaRenderPage& page, const char* const* prefixes, int seconds) {
  std::string t;
  for (int i = 0; i < seconds * 4; i++) {
    page.PumpFor(250);
    t = page.GetTitle();
    for (const char* const* p = prefixes; *p; ++p) if (starts(t, *p)) return t;
  }
  return t;
}

static void playClip(GoannaRenderPage& page, const char* testDir, const char* clip, const char* codec) {
  std::string url = std::string("file://") + testDir + "/jihad-media-test.html?src=" + clip;
  bool loaded = page.LoadUrlAndWait(url.c_str(), 20);
  printf("[media] %s: load ok=%d\n", codec, (int)loaded);
  const char* started[] = { "playing", "error", "play refused", nullptr };
  std::string t = waitTitle(page, started, 10);
  check(starts(t, "playing"), (std::string(codec) + ": playback starts").c_str(), t);
  if (!starts(t, "playing")) return;
  std::string anon = DebugAnonNodes("#v");
  check(anon.find("videocontrols[") != std::string::npos && anon.find("videocontrols[0]") == std::string::npos,
        (std::string(codec) + ": the controls' binding attached").c_str(), anon);
  // the play/pause button: the left end of the control bar along the video's bottom edge
  std::string r = DebugElementRect("v");
  int x = 0, y = 0, w = 0, h = 0;
  const char* sp = strstr(r.c_str(), "rect=");
  if (sp) sscanf(sp + 5, "%d,%d %dx%d", &x, &y, &w, &h);
  // A tap on the video, through the pres shell like a finger's (a move, a press, a release at its
  // centre): videocontrols' click-to-pause listeners sit on its anonymous parts (the spacer above
  // the bar, the play button), which an element-targeted click never reaches
  auto tap = [&]() {
    page.MouseEvent("mousemove", x + w / 2, y + h / 2, 0);
    page.PumpFor(300);
    page.MouseEvent("mousedown", x + w / 2, y + h / 2, 0);
    page.PumpFor(60);
    page.MouseEvent("mouseup", x + w / 2, y + h / 2, 0);
  };
  tap();
  const char* paused[] = { "paused", "ended", "error", nullptr };
  t = waitTitle(page, paused, 4);
  check(starts(t, "paused"), (std::string(codec) + ": a tap on the video pauses").c_str(), t);
  page.PumpFor(1000);
  tap();
  const char* resumed[] = { "resumed", "ended", "error", nullptr };
  t = waitTitle(page, resumed, 4);
  check(starts(t, "resumed"), (std::string(codec) + ": another tap resumes").c_str(), t);
  const char* ended[] = { "ended", "error", nullptr };
  t = waitTitle(page, ended, 25);
  float ct = -1, dur = -2; int decoded = -1;
  if (starts(t, "ended")) sscanf(t.c_str(), "ended t=%f dur=%f decoded=%d", &ct, &dur, &decoded);
  check(starts(t, "ended") && std::fabs(ct - dur) < 0.6f, (std::string(codec) + ": plays to the end").c_str(), t);
  check(decoded > 100, (std::string(codec) + ": frames decoded").c_str(), std::to_string(decoded));
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* testDir = (argc > 2) ? argv[2] : "/jihad/render/goanna/test";
  const bool neg = getenv("JIHAD_MEDIA_NEG") != nullptr;
  const bool sound = getenv("JIHAD_MEDIA_SOUND") != nullptr;
  gtk_init(&argc, &argv);
  printf("[media] START gre=%s tests=%s neg=%d sound=%d\n", greDir, testDir, (int)neg, (int)sound);
  EngineHost host;
  if (!host.Init(greDir)) { printf("[media] engine init FAIL\n"); return 1; }
  int rc = 3;
  {
    GoannaRenderPage page(host);
    if (!page.Create(1024, 768)) { printf("[media] page create FAIL\n"); host.Shutdown(); return 2; }
    if (neg) {
      playClip(page, testDir, "no-such-clip.webm", "NEG");
    } else {
      playClip(page, testDir, "clip10v.webm", "VP8");
      playClip(page, testDir, "clip10.ogv", "Theora");
      playClip(page, testDir, "clip10vp9.webm", "VP9");
      if (sound) {
        std::string url = std::string("file://") + testDir + "/jihad-media-test.html?audio=1&src=tone15.ogg";
        page.LoadUrlAndWait(url.c_str(), 20);
        const char* ended[] = { "ended", "error", nullptr };
        std::string t = waitTitle(page, ended, 25);
        check(starts(t, "ended"), "Vorbis audio plays to the end", t);
      } else {
        printf("[media] SKIP audio-only: no sound server in this container (JIHAD_MEDIA_SOUND unset)\n");
      }
    }
    printf("[media] checks=%d fails=%d\n", gChecks, gFails);
    printf("[media] %s\n", gFails == 0 && gChecks > 0 ? "MEDIA PASS" : "MEDIA FAIL");
    rc = (gFails == 0 && gChecks > 0) ? 0 : 4;
  }
  host.Shutdown();
  return rc;
}
