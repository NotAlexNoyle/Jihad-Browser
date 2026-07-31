/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — cookie PERSISTENCE test (domain G / R2).
 *
 * Run twice against the same $JIHAD_PROFILE_DIR (that second run IS the
 * "daemon restart"):
 *   set   — load /set (Set-Cookie with a future Max-Age, i.e. a PERSISTENT
 *           cookie; session cookies are not supposed to survive), then /echo to
 *           prove the cookie is live in-session.
 *   check — load /echo only. The cookie can only come back if the engine read
 *           it from cookies.sqlite in the profile.
 * Both modes report whether cookies.sqlite exists after a clean shutdown.
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include "../GoannaRenderPage.h"
#include "../EngineHost.h"

static const char* kCookie = "jihadpersist=1";

static bool fileExists(const std::string& p) {
  struct stat st;
  return stat(p.c_str(), &st) == 0;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const char* mode   = (argc > 2) ? argv[2] : "set";
  const char* base   = (argc > 3) ? argv[3] : "http://127.0.0.1:8138";
  const bool setMode = strcmp(mode, "set") == 0;

  gtk_init(&argc, &argv);
  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[cookie] engine init FAIL\n"); return 1; }
  printf("[cookie] engine up (mode=%s)\n", mode);

  bool ok = false;
  {
    jihad::GoannaRenderPage page(host);
    if (!page.Create(640, 480)) { host.Shutdown(); return 2; }
    if (setMode) {
      page.LoadUrlAndWait((std::string(base) + "/set").c_str(), 25);
      page.PumpFor(1500);
    }
    page.LoadUrlAndWait((std::string(base) + "/echo").c_str(), 25);
    page.PumpFor(1500);
    std::string title = page.GetTitle();
    printf("[cookie] %s-run echo title='%s'\n", mode, title.c_str());
    ok = title.find(kCookie) != std::string::npos;
  }
  host.Shutdown();   // clean teardown: this is what flushes/closes cookies.sqlite

  const char* prof = getenv("JIHAD_PROFILE_DIR");
  std::string db = std::string(prof ? prof : "") + "/cookies.sqlite";
  bool dbOK = prof && *prof && fileExists(db);
  printf("[cookie] cookies.sqlite (%s): %s\n", db.c_str(), dbOK ? "PRESENT" : "MISSING");

  bool pass = ok && dbOK;
  printf("[cookie] %s %s\n", setMode ? "COOKIE-SET" : "COOKIE-PERSIST",
         pass ? "PASS" : "FAIL");
  return pass ? 0 : 4;
}
