/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#ifndef JIHAD_CRASH_GUARD_H
#define JIHAD_CRASH_GUARD_H

// A page that kills the daemon must not be reopened forever.
//
// WHAT HAPPENED (2026-10-05). Two pages crashed the daemon while loading (a null dereference
// in the plugin code, since fixed). When the daemon dies the card reconnects to the new one
// and opens its address again — which is right for an ordinary restart, and here meant: load,
// crash, restart, load, crash … for as long as the card stayed open. About forty crash
// reports in a few minutes, and nothing on screen but a spinner.
//
// THE GUARD. Every top-level load that starts is noted in a small file in the state
// directory, with how many daemons in a row have died while that same address was the last
// one started. A clean shutdown removes the file. So a file found at start means the previous
// daemon did NOT exit cleanly, and names what it was loading. When the card then asks for an
// address that has already taken two daemons down within the last ten minutes, the daemon
// does not load it: it shows a short page saying so, with a link to try again. The refusal
// is used up by being shown — the link, a reload or a typed address loads the page for real.
//
// What it is not: a judgement about the page. One death is allowed to be coincidence (the
// second attempt is automatic); two in a row with the same address is a pattern.

#include "JihadRuntimePaths.h"
#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <unistd.h>

namespace jihad {

struct CrashGuardState {
  bool loaded = false;
  std::string prevUrl;      // what the previous daemon was loading when it died uncleanly
  int prevCount = 0;        // how many daemons in a row died on it
  long prevTime = 0;
  bool refused = false;     // the one refusal has been shown
};

inline CrashGuardState& CrashGuardGet() {
  static CrashGuardState s;
  if (!s.loaded) {
    s.loaded = true;
    const std::string path = RuntimeStatePath("last-load");
    if (!path.empty()) {
      if (FILE* f = fopen(path.c_str(), "r")) {
        char head[64] = "", url[2048] = "";
        if (fgets(head, sizeof head, f) && fgets(url, sizeof url, f)) {
          url[strcspn(url, "\r\n")] = 0;
          int n = 0; long t = 0;
          if (sscanf(head, "%d %ld", &n, &t) == 2 && n > 0 && url[0]) {
            s.prevUrl = url; s.prevCount = n; s.prevTime = t;
            fprintf(stderr, "[jihad-bs] crash guard: the previous daemon did not exit cleanly; "
                            "it was loading a page (%d in a row on that address)\n", n);
          }
        }
        fclose(f);
      }
    }
  }
  return s;
}

// A top-level load of `url` is starting. Records it as "what this daemon dies with, if it
// dies now". Never the fragment: the settings page rewrites its own.
inline void CrashGuardNoteLoad(const char* url) {
  if (!url || !*url) return;
  CrashGuardState& s = CrashGuardGet();
  const std::string path = RuntimeStatePath("last-load");
  if (path.empty()) return;
  std::string u(url);
  const size_t hash = u.find('#');
  if (hash != std::string::npos) u.erase(hash);
  if (u.size() > 2000) u.resize(2000);
  const int n = (u == s.prevUrl) ? s.prevCount + 1 : 1;
  if (FILE* f = fopen(path.c_str(), "w")) {
    fprintf(f, "%d %ld\n%s\n", n, (long)time(nullptr), u.c_str());
    fclose(f);
  }
}

// Should the card's request for `url` be refused this once?
inline bool CrashGuardShouldRefuse(const char* url) {
  if (!url || !*url) return false;
  CrashGuardState& s = CrashGuardGet();
  if (s.refused || s.prevCount < 2) return false;
  std::string u(url);
  const size_t hash = u.find('#');
  if (hash != std::string::npos) u.erase(hash);
  if (u != s.prevUrl) return false;
  if ((long)time(nullptr) - s.prevTime > 600) return false;
  s.refused = true;
  fprintf(stderr, "[jihad-bs] crash guard: NOT reopening a page that stopped the browser %d times in a row\n",
          s.prevCount);
  return true;
}

// The daemon is exiting on purpose: nothing it had open is to blame.
inline void CrashGuardCleanExit() {
  const std::string path = RuntimeStatePath("last-load");
  if (!path.empty()) unlink(path.c_str());
}

}  // namespace jihad

#endif  // JIHAD_CRASH_GUARD_H
