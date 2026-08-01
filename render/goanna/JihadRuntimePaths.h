/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
/*
 * Jihad Browser — the daemon's ONE runtime-state directory (T-057).
 *
 * WHY THIS FILE EXISTS
 * Every writable runtime path the daemon owns — the engine profile/cache, the
 * frame dump, the self-drive inject channel, and (via the upstart job) the log —
 * used to land in /media/internal/jihad/. That is the user's **vfat USB
 * mass-storage volume**: it has no real permissions (every file reads as 777),
 * it vanishes when the device is mounted as a USB drive on a PC, and it is the
 * user's storage, not ours. cavekit-device-build.md **R8** ("good webOS citizen —
 * install footprint contract") therefore requires that the package write NOTHING
 * there, and plan-variant-identity.md pins the replacement:
 *
 *     runtime state dir = /var/palm/jihad/<variant>/     (root:root 0755, ext3 rootfs)
 *     variant           = enyo | mochi | mojo
 *
 * Per R7 the three UI variants are fully independent packages, so their state
 * must not collide either — hence the <variant> level, derived here from the YAP
 * service name the daemon already receives in $JIHAD_BS_NAME. Deriving it in ONE
 * place is the point of this header: nothing else in the tree may spell a runtime
 * path itself.
 *
 * Header-only on purpose: the daemon's sources are compiled by a dozen explicit
 * per-file build scripts under build/desktop plus the OE recipe, so adding a .cpp
 * would mean touching every one of them. All entry points are `inline` with
 * function-local statics, so the derivation runs once per process.
 */
#ifndef JIHAD_RUNTIME_PATHS_H
#define JIHAD_RUNTIME_PATHS_H

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace jihad {

// ── path canonicalisation ───────────────────────────────────────────────────
// The R8 guard below is a PREFIX test, and a prefix test on a raw string is not
// a statement about where a path lands: "/tmp/../media/internal/x" and
// "/media//internal/x" both resolve onto the user's volume while failing to
// start with the literal prefix (review F-5). Canonicalise first, then test.
//
// Done in two steps because realpath(3) fails on a path that does not exist yet
// — which is the normal case here, since these helpers are what CREATE the
// directories:
//   1. lexical: collapse "//", drop ".", pop ".." against the accumulated
//      components. Purely textual, always available.
//   2. physical: realpath() the longest EXISTING ancestor (so a symlinked
//      component cannot hide the destination) and re-attach the rest.
// A path that cannot be canonicalised at all falls back to its lexical form,
// which is still strictly better than the raw string.
inline std::string RuntimeLexicalCanon(const std::string& in) {
  if (in.empty() || in[0] != '/') return in;      // callers only pass absolutes
  std::vector<std::string> parts;
  size_t i = 0;
  while (i < in.size()) {
    while (i < in.size() && in[i] == '/') i++;
    size_t j = i;
    while (j < in.size() && in[j] != '/') j++;
    if (j > i) {
      const std::string c = in.substr(i, j - i);
      if (c == ".") {
        // no-op
      } else if (c == "..") {
        if (!parts.empty()) parts.pop_back();     // ".." at "/" stays at "/"
      } else {
        parts.push_back(c);
      }
    }
    i = j;
  }
  std::string out;
  for (size_t k = 0; k < parts.size(); k++) { out += "/"; out += parts[k]; }
  return out.empty() ? std::string("/") : out;
}

inline std::string RuntimeCanon(const std::string& in) {
  if (in.empty() || in[0] != '/') return in;
  const std::string lex = RuntimeLexicalCanon(in);
  std::string head = lex, tail;
  for (;;) {
    char rp[PATH_MAX];
    if (realpath(head.c_str(), rp)) {
      std::string out(rp);
      if (!tail.empty()) {
        if (out != "/") out += "/";
        out += tail;
      }
      return RuntimeLexicalCanon(out);
    }
    const size_t slash = head.find_last_of('/');
    if (slash == std::string::npos || head == "/") break;   // realpath("/") cannot fail
    const std::string comp = head.substr(slash + 1);
    tail = tail.empty() ? comp : comp + "/" + tail;
    head = (slash == 0) ? std::string("/") : head.substr(0, slash);
  }
  return lex;
}

// ── the user's volume: never ours ───────────────────────────────────────────
// R8: nothing this package writes may land here. Kept as a predicate (not just a
// comment) so an inherited environment variable cannot smuggle a path back onto
// the user's storage — see RuntimeResolvePath().
inline const char* RuntimeUserVolume() { return "/media/internal"; }

inline bool RuntimeOnUserVolume(const std::string& p) {
  if (p.empty() || p[0] != '/') return false;     // relative: not a claim we can make
  // The volume itself is canonicalised too (and cached): on a desktop/container
  // host it does not exist at all, where realpath() fails and the lexical form
  // "/media/internal" is exactly right.
  static const std::string v = RuntimeCanon(RuntimeUserVolume());
  const std::string c = RuntimeCanon(p);
  const size_t n = v.size();
  return c.size() >= n && c.compare(0, n, v) == 0 &&
         (c.size() == n || c[n] == '/');
}

// ── the ONE deliberate carve-out: where the USER's downloads go ─────────────
// R8 is about keeping APP INTERNALS off the user's USB mass-storage volume: the
// engine profile, the cache, the log, the debug channels — things the user never
// asked for, cannot see, and would be baffled to find on the drive they mount on
// a PC. A file the user explicitly asked the browser to download is the exact
// opposite case (review F-10). It is THEIR data; it has to be reachable from
// every other app and from the USB mass-storage mode; it must survive an upgrade
// and an uninstall of this package; and on a 559 MB rootfs vs a multi-GB user
// volume, it is also the only place a large download fits.
//
// /media/internal/downloads is webOS's own convention for exactly this — it is
// still the `DownloadPath` default in render/browserserver/Src/Settings.cpp, i.e.
// what the stock BrowserServer this daemon replaces has always used, and what
// com.palm.downloadmanager hands out.
//
// This is ONE destination, named here as a constant, and nothing else about the
// R8 guard changes: RuntimeTryDir()/RuntimeResolvePath() still refuse every other
// path on that volume, so an inherited $JIHAD_DUMP or a stale $JIHAD_STATE_DIR
// cannot re-colonise it. Do not generalise this into "paths under /media are
// allowed" — the whole value of the carve-out is that it is a single, auditable,
// user-data-only exception.
inline const char* RuntimeUserDownloadDir() { return "/media/internal/downloads"; }

inline bool RuntimeIsUserDownloadDir(const std::string& p) {
  if (p.empty() || p[0] != '/') return false;
  static const std::string d = RuntimeCanon(RuntimeUserDownloadDir());
  return RuntimeCanon(p) == d;
}

// ── identity table: YAP service name -> variant token ───────────────────────
// context/plans/plan-variant-identity.md is authoritative; these three names are
// the ones the three upstart jobs export as JIHAD_BS_NAME. Anything else is NOT
// a packaged install (a desktop harness run, an ad-hoc novacom run, a future
// variant nobody has registered here), and gets the non-packaged token below.
inline const char* const kRuntimeVariantUnpackaged = "default";

inline const char* RuntimeVariantForName(const char* yapName) {
  if (!yapName || !*yapName) return kRuntimeVariantUnpackaged;
  if (!strcmp(yapName, "jihad-browser"))       return "enyo";
  if (!strcmp(yapName, "jihad-browser-mochi"))  return "mochi";
  if (!strcmp(yapName, "jihad-browser-mojo"))   return "mojo";
  return kRuntimeVariantUnpackaged;
}

// The same table's app-id column. Needed because the DISPOSABLE half of the
// engine profile lives in the app's own directory on cryptofs (see
// RuntimeCacheDir below), and that path is keyed by app id, not by variant
// token. Empty for a non-packaged run — there is no app directory then.
inline const char* RuntimeAppIdForVariant(const char* v) {
  if (!v) return "";
  if (!strcmp(v, "enyo"))  return "net.riverstonerelay.jihad-browser";
  if (!strcmp(v, "mochi")) return "net.riverstonerelay.jihad-browser.mochi";
  if (!strcmp(v, "mojo"))  return "net.riverstonerelay.jihad-browser.mojo";
  return "";
}

// The variant token for THIS process. Read once — JIHAD_BS_NAME is fixed at exec
// (Main.cpp passes the same value to the YapServer), so caching is safe.
inline const char* RuntimeVariant() {
  static const char* const v = RuntimeVariantForName(getenv("JIHAD_BS_NAME"));
  return v;
}

// True when $JIHAD_BS_NAME named one of the three packaged variants. Only those
// may create state under /var/palm/jihad/: that root is owned by the packages,
// and each variant's `prerm` removes exactly its own /var/palm/jihad/$V/ (rule 2
// of the identity plan). A directory created there under any other name would be
// residue no prerm ever deletes — which is precisely what R8's "no residue"
// acceptance criterion forbids. So an unknown/unset name falls through to the
// per-user fallbacks below instead.
inline bool RuntimeIsPackagedVariant() {
  return strcmp(RuntimeVariant(), kRuntimeVariantUnpackaged) != 0;
}

// ── directory helpers ───────────────────────────────────────────────────────
// mkdir -p with an explicit mode. chmod() only on components WE created, so a
// pre-existing system directory (/var, /var/palm — root:root 0755 on the device)
// is never re-permissioned by us. The chmod defeats the inherited umask: upstart
// runs the daemon with whatever umask init had, and the identity table pins these
// modes rather than leaving them to chance.
inline bool RuntimeMakeDirs(const std::string& path, mode_t mode) {
  if (path.empty() || path[0] != '/') return false;
  for (size_t i = 1; i <= path.size(); i++) {
    if (i < path.size() && path[i] != '/') continue;
    if (path[i - 1] == '/') continue;                 // collapse "//"
    const std::string acc = path.substr(0, i);
    if (mkdir(acc.c_str(), mode) == 0) {
      if (chmod(acc.c_str(), mode) != 0) { /* best effort */ }
    } else if (errno != EEXIST) {
      return false;
    }
  }
  return true;
}

// A state dir is usable only if it is a REAL directory (lstat, so a symlink
// planted in its place fails), owned by us, not group/world-writable, and
// writable. The mode check is requirement (d) of T-057 turned into an assertion:
// the daemon runs as root on-device, and a root-owned world-writable state dir
// would hand every process on the box the inject channel.
inline bool RuntimeDirUsable(const std::string& path) {
  struct stat st;
  if (lstat(path.c_str(), &st) != 0) return false;
  if (!S_ISDIR(st.st_mode)) return false;
  if (st.st_uid != geteuid()) return false;
  if (st.st_mode & (S_IWGRP | S_IWOTH)) return false;
  return access(path.c_str(), W_OK | X_OK) == 0;
}

inline bool RuntimeTryDir(const std::string& path, mode_t mode, std::string& out) {
  if (path.empty() || path[0] != '/') return false;
  if (RuntimeOnUserVolume(path)) return false;        // R8, unconditional
  if (!RuntimeMakeDirs(path, mode)) return false;
  if (!RuntimeDirUsable(path)) return false;
  out = path;
  return true;
}

// ── the weaker validator, for filesystems that HAVE no permissions ──────────
// Same shape as RuntimeTryDir minus RuntimeDirUsable's ownership/mode
// assertions, because on the two filesystems this is used for those assertions
// cannot be satisfied by ANY directory:
//   * /media/internal is VFAT — no ownership, no mode bits. The kernel
//     synthesises one uid/mode for every entry from the mount options (0777
//     root-owned in practice).
//   * /media/cryptofs is FUSE and reports EVERY file as 0777 (the same fact that
//     forces the adapter impl onto the rootfs — plan-variant-identity.md rule 5).
// Demanding "owned by us and not group/world-writable" there would reject the
// correct destination 100% of the time.
//
// What is STILL checked, and why it is enough: the path must resolve to a real
// DIRECTORY (lstat, not stat — a symlink planted at the path cannot redirect us)
// and must be writable. Nothing written through this helper is ever trusted for
// integrity: it is either the user's own downloaded file, or a disposable HTTP
// cache whose entries Gecko checksums (a corrupt entry is a cache miss, not a
// vulnerability). Anything privileged, or anything whose integrity we rely on,
// must keep using RuntimeTryDir.
inline bool RuntimeTryDirUnowned(const std::string& path, mode_t mode, std::string& out) {
  if (path.empty() || path[0] != '/') return false;
  if (!RuntimeMakeDirs(path, mode)) return false;
  struct stat st;
  if (lstat(path.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) return false;
  if (access(path.c_str(), W_OK | X_OK) != 0) return false;
  out = path;
  return true;
}

// The download carve-out's constructor (F-10). This is the ONE caller that also
// skips RuntimeOnUserVolume — because it IS the exception. Spelled out here
// rather than by weakening the shared guard, so the exception stays greppable.
inline bool RuntimeTryUserDownloadDir(std::string& out) {
  return RuntimeTryDirUnowned(RuntimeUserDownloadDir(), 0755, out);
}

// ── the app partition ───────────────────────────────────────────────────────
// /media/cryptofs is where an installed webOS app lives, and where our engine
// already executes from. It is FUSE and reports EVERY entry as 0777, so
// RuntimeDirUsable's ownership/mode assertions can never hold there (the same
// fact that forces the adapter impl onto the rootfs — plan-variant-identity.md
// rule 5).
inline const char* RuntimeAppPartition() { return "/media/cryptofs"; }

inline bool RuntimeOnAppPartition(const std::string& p) {
  if (p.empty() || p[0] != '/') return false;
  static const std::string v = RuntimeCanon(RuntimeAppPartition());
  const std::string c = RuntimeCanon(p);
  const size_t n = v.size();
  return c.size() >= n && c.compare(0, n, v) == 0 &&
         (c.size() == n || c[n] == '/');
}

// One entry point for "make this directory and check it", picking the strongest
// validator the destination's filesystem can actually satisfy:
//   /media/internal  -> REFUSED outright (R8). No exceptions here; the single
//                       download carve-out calls RuntimeTryDirUnowned directly.
//   /media/cryptofs  -> RuntimeTryDirUnowned (0777-reporting FUSE; see above)
//   anything else    -> RuntimeTryDir, full lstat + ownership + mode assertions
// Choosing by FILESYSTEM rather than by caller is what keeps this honest: the
// weaker check applies exactly where the stronger one is impossible, never
// because a caller found it inconvenient.
inline bool RuntimeTryDirAuto(const std::string& path, mode_t mode, std::string& out) {
  if (path.empty() || path[0] != '/') return false;
  if (RuntimeOnUserVolume(path)) return false;          // R8, unconditional
  if (RuntimeOnAppPartition(path)) return RuntimeTryDirUnowned(path, mode, out);
  return RuntimeTryDir(path, mode, out);
}

// ── the state directory ─────────────────────────────────────────────────────
// Resolution order (first usable wins):
//   1. $JIHAD_STATE_DIR            — explicit override (absolute; never on the
//                                    user volume). Escape hatch for harnesses.
//   2. /var/palm/jihad/<variant>   — the packaged device location, 0755, only
//                                    for a packaged variant (see above).
//   3. $HOME/.jihad/<variant>      — desktop / non-root: /var/palm is root-owned
//                                    and unwritable there, and the container test
//                                    harness runs with HOME=/out (a persistent
//                                    volume), so cookies still survive a restart.
//   4. /tmp/jihad-<euid>/<variant> — last resort. uid-scoped so two users cannot
//                                    fight over one name; 0700 because /tmp is
//                                    shared (the 0755 of the table is right for a
//                                    root-owned rootfs dir, wrong for /tmp).
// Empty return = no writable state at all; every caller degrades instead of
// aborting (the desktop harness must keep running — T-057 constraint).
inline const std::string& RuntimeStateDir() {
  static const std::string dir = []() -> std::string {
    const std::string v = RuntimeVariant();
    std::string out;

    if (const char* e = getenv("JIHAD_STATE_DIR")) {
      if (*e) {
        if (RuntimeTryDir(e, 0755, out)) return out;
        fprintf(stderr, "[jihad-bs] JIHAD_STATE_DIR=%s unusable — falling back\n", e);
      }
    }
    if (RuntimeIsPackagedVariant()) {
      if (RuntimeTryDir("/var/palm/jihad/" + v, 0755, out)) return out;
      // Notable, not fatal: a packaged variant that cannot reach its own state
      // dir is either running non-root or on a read-only rootfs.
      fprintf(stderr, "[jihad-bs] /var/palm/jihad/%s unavailable (last errno: %s) — using a fallback\n",
              v.c_str(), strerror(errno));
    }
    if (const char* h = getenv("HOME")) {
      if (*h == '/' && RuntimeTryDir(std::string(h) + "/.jihad/" + v, 0700, out)) return out;
    }
    {
      char b[64];
      snprintf(b, sizeof b, "/tmp/jihad-%lu/", (unsigned long)geteuid());
      if (RuntimeTryDir(std::string(b) + v, 0700, out)) return out;
    }
    fprintf(stderr, "[jihad-bs] no writable runtime state dir — profile/debug channels disabled\n");
    return std::string();
  }();
  return dir;
}

// <state>/<leaf>, or "" when there is no state dir.
inline std::string RuntimeStatePath(const char* leaf) {
  const std::string& d = RuntimeStateDir();
  if (d.empty() || !leaf || !*leaf) return std::string();
  return d + "/" + leaf;
}

// ── env value -> runtime path ───────────────────────────────────────────────
// Shared by the debug channels (JIHAD_DUMP, JIHAD_INJECT) so they cannot drift
// apart. Accepted forms:
//   unset, "", "0", "off", "no", "false"  -> DISABLED (returns "")
//   "1", "on", "yes", "true"              -> <state>/<defaultLeaf>
//   "name.ext"                            -> <state>/name.ext
//   "/abs/path"                           -> that path, CANONICALISED, EXCEPT
//                                            when it resolves onto
//                                            /media/internal, which R8 forbids:
//                                            redirected to <state>/<defaultLeaf>
//                                            with a loud line, so a stale env in
//                                            an upstart job or an old helper
//                                            script cannot re-colonize the user's
//                                            volume behind our back.
// ".." anywhere in a relative value is refused rather than silently escaping the
// state dir. In an ABSOLUTE value ".." is not refused but resolved (F-5) —
// "/tmp/../media/internal/x" used to slip past the guard because the check was a
// raw-string prefix test; it is now a canonicalised one, and the canonical form is
// what gets returned, so the caller cannot re-introduce the escape either.
inline std::string RuntimeResolvePath(const char* env, const char* defaultLeaf) {
  if (!env || !*env) return std::string();
  if (!strcmp(env, "0") || !strcmp(env, "off") ||
      !strcmp(env, "no") || !strcmp(env, "false")) return std::string();

  const bool boolish = !strcmp(env, "1") || !strcmp(env, "on") ||
                       !strcmp(env, "yes") || !strcmp(env, "true");
  if (boolish) return RuntimeStatePath(defaultLeaf);

  if (env[0] == '/') {
    const std::string p = RuntimeCanon(env);
    if (!RuntimeOnUserVolume(p)) return p;
    std::string redirected = RuntimeStatePath(defaultLeaf);
    fprintf(stderr, "[jihad-bs] refusing to write %s on the user's volume (R8) -> %s\n",
            env, redirected.empty() ? "(disabled)" : redirected.c_str());
    return redirected;
  }
  if (strstr(env, "..")) {
    fprintf(stderr, "[jihad-bs] refusing runtime path with '..': %s\n", env);
    return std::string();
  }
  return RuntimeStatePath(env);
}

// ── the engine profile: BOTH halves live in the app's own directory ─────────
//
// PRIOR ART, and it is unanimous. Both previous implementations of this browser
// on this device put the disk cache AND the cookies on cryptofs:
//
//   isis (our own upstream — render/browserserver/Src/Settings.cpp:65-74, dead
//   Qt code now, but it is the shipped configuration of the browser we forked):
//       CachePath                         /media/cryptofs/.browser/cache
//       CookieJarPath                     /media/cryptofs/.browser/cookies
//       WebSettings/PersistentStoragePath /media/cryptofs/.browser
//       CacheMaxSize                      "50M"
//       DownloadPath                      /media/internal/downloads
//
//   Atlas (atlas-wpe-backend/BrowserPageWPE.cpp:730), which also gives the hard
//   reason: "Network session data (IndexedDB / localStorage / ServiceWorkers) +
//   disk cache + cookies must NOT live on /media/internal: it is VFAT (no hard
//   links, no real file locking). That breaks WebKit's NetworkCache … AND,
//   critically, the SQLite-backed website storage that heavy apps need… Put them
//   on cryptofs (create/rename/lock work; PROVEN…)". It routes netdata, netcache
//   and cookies.db to its app deviceroot on cryptofs.
//
// That VFAT sentence also explains OUR OWN 2026-07-20 device failure, where
// cookies.sqlite was never created at all: the profile was on /media/internal,
// and SQLite had no real locking there. Moving to ext3 fixed it — but cryptofs
// fixes it equally and is where both predecessors put it, so this converges on
// the platform's answer rather than inventing a third one.
//
// Measured free space, 2026-08-01:
//     /var             49.6 MB  ext3, SHARED with system state
//     /media/cryptofs  10.2 GB  the app partition, where the engine already runs
// So /var/palm/jihad/<variant>/ keeps ONLY the daemon log and the debug channels
// — kilobytes of operational state for a root daemon, correctly on ext3. The
// engine profile does not go there: 49.6 MB is not a browser profile budget.
//
// Gecko's own split is honoured rather than a new mechanism invented (EngineHost
// registers both keys):
//     ProfD  (NS_APP_USER_PROFILE_50_DIR)        $APP/profile  durable
//     ProfLD (NS_APP_USER_PROFILE_LOCAL_50_DIR)  $APP/cache    disposable
//
// TRADEOFF, written down: cryptofs is FUSE, so cache2's many-small-file I/O is
// slower than ext3 would be. Both predecessors accepted that, and the
// alternative is a cache that fills a 62 MB system partition and takes the rest
// of webOS with it. The disk cache is also CAPPED (browser.cache.disk.capacity =
// 50 MB, isis's own CacheMaxSize; set in make-device-bundle.sh's goanna.js).
//
// R8 consequence, handled in packaging: both trees are created at RUNTIME inside
// the app directory, so ipkg never tracks them and would leave them behind on
// removal. Each variant's prerm removes them explicitly.
//
// NOT /var/file-cache and NOT /var/db: those are the filecache service's and
// mojodb's managed stores, with their own accounting and GC. Squatting in
// another service's store is exactly the bad-citizen behaviour R8 exists to
// prevent, whatever the free-space number says.

// $APP for this variant, or "" when this is not a packaged run.
inline std::string RuntimeAppDir() {
  const char* appId = RuntimeAppIdForVariant(RuntimeVariant());
  if (!*appId) return std::string();
  return std::string("/media/cryptofs/apps/usr/palm/applications/") + appId;
}

// Shared resolution for the two profile halves: explicit env override first
// (through the R8 guard + canonicalisation), then $APP/<leaf> on cryptofs, then
// <state>/<leaf> for a desktop/harness run where there is no app directory.
// Never returns a path that has not been created and validated.
inline std::string RuntimeAppScopedDir(const char* env, const char* leaf, mode_t mode) {
  std::string out;
  if (const char* e = env) {
    if (*e) {
      const std::string want = RuntimeResolvePath(e, leaf);
      if (want.empty()) return std::string();          // "0"/"off" — explicitly disabled
      if (RuntimeTryDirAuto(want, mode, out)) return out;
      fprintf(stderr, "[jihad-bs] %s unusable — falling back\n", e);
    }
  }
  const std::string app = RuntimeAppDir();
  if (!app.empty()) {
    if (RuntimeTryDirAuto(app + "/" + leaf, mode, out)) return out;
    fprintf(stderr, "[jihad-bs] %s/%s unavailable (%s) — falling back to the state dir\n",
            app.c_str(), leaf, strerror(errno));
  }
  // Desktop/harness (no app dir), or a device where it is not writable. On the
  // desktop the state dir is $HOME/.jihad/<v>, with no 49.6 MB ceiling.
  if (RuntimeTryDirAuto(RuntimeStatePath(leaf), mode, out)) return out;
  return std::string();
}

// ProfD — durable: cookies.sqlite, prefs.js, permissions, cert overrides.
// 0700 on a real filesystem; on cryptofs the mode is advisory (it reports 0777
// for everything), which is precisely why nothing privileged reads this tree.
inline const std::string& RuntimeProfileDir() {
  static const std::string dir = []() -> std::string {
    const std::string d = RuntimeAppScopedDir(getenv("JIHAD_PROFILE_DIR"), "profile", 0700);
    if (d.empty())
      fprintf(stderr, "[jihad-bs] no writable profile dir — cookies will be memory-only\n");
    return d;
  }();
  return dir;
}

// ProfLD — disposable: cache2, startupCache.
inline const std::string& RuntimeCacheDir() {
  static const std::string dir = []() -> std::string {
    const std::string d = RuntimeAppScopedDir(getenv("JIHAD_CACHE_DIR"), "cache", 0700);
    if (d.empty())
      fprintf(stderr, "[jihad-bs] no writable cache dir — HTTP disk cache disabled\n");
    return d;
  }();
  return dir;
}

}  // namespace jihad

#endif  // JIHAD_RUNTIME_PATHS_H
