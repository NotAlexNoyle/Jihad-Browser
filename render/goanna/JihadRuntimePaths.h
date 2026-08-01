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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace jihad {

// ── the user's volume: never ours ───────────────────────────────────────────
// R8: nothing this package writes may land here. Kept as a predicate (not just a
// comment) so an inherited environment variable cannot smuggle a path back onto
// the user's storage — see RuntimeResolvePath().
inline const char* RuntimeUserVolume() { return "/media/internal"; }

inline bool RuntimeOnUserVolume(const std::string& p) {
  const char* v = RuntimeUserVolume();
  const size_t n = strlen(v);
  return p.size() >= n && p.compare(0, n, v) == 0 &&
         (p.size() == n || p[n] == '/');
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
//   "/abs/path"                           -> exactly that, EXCEPT under
//                                            /media/internal, which R8 forbids:
//                                            redirected to <state>/<defaultLeaf>
//                                            with a loud line, so a stale env in
//                                            an upstart job or an old helper
//                                            script cannot re-colonize the user's
//                                            volume behind our back.
// ".." anywhere in a relative value is refused rather than silently escaping the
// state dir.
inline std::string RuntimeResolvePath(const char* env, const char* defaultLeaf) {
  if (!env || !*env) return std::string();
  if (!strcmp(env, "0") || !strcmp(env, "off") ||
      !strcmp(env, "no") || !strcmp(env, "false")) return std::string();

  const bool boolish = !strcmp(env, "1") || !strcmp(env, "on") ||
                       !strcmp(env, "yes") || !strcmp(env, "true");
  if (boolish) return RuntimeStatePath(defaultLeaf);

  if (env[0] == '/') {
    std::string p(env);
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

}  // namespace jihad

#endif  // JIHAD_RUNTIME_PATHS_H
