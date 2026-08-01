---
created: "2026-08-01"
last_edited: "2026-08-01"
---

# Adversarial review — per-variant independence + citizen footprint (commit e36c8cc)

**Reviewer: fable.** Read-only review against cavekit-device-build.md R7/R8 and
`../plans/plan-variant-identity.md`.

## Verdict on the headline claims

The naming/identity machinery **holds**. The reviewer extracted the identity table from all seven
consumers and every row agrees on all six columns; `gen-variant-scripts.sh --check` passes; the
compile-time `#error` on a half-set identity and the `strings` readback probes are real checks. It
specifically tried and failed to break app-id **prefix confusion** (`net.riverstonerelay.jihad-browser`
is a prefix of the other two ids) — the `_` in the `.ipk` glob, the `/` in the deviceroot match, and
the whole-argument `[ "$arg" = "$HL" ]` in `prerm` all block it. No socket/job/state globs, no
`killall`, no wildcard `rm` survive in the generated scripts.

**The `novacom` exit-status question the reviewer raised as UNPROVEN — and correctly flagged as
capable of making every harness `PASS` vacuous — was settled immediately: `novacom run` DOES
propagate the remote command's status (missing path → 1, present → 0). The device evidence stands.**

| # | Sev | Where | Defect | Status |
|---|-----|-------|--------|--------|
| F-1 | P1 | `app/db/kinds/*`, `app-mochi/source/JihadServices.js:33-35` | **The three variants co-own the db8 kinds.** Mochi's history/bookmarks/preferences kinds are `"owner": net.riverstonerelay.jihad-browser` and ship **only** inside the Enyo package. Install Mochi alone → the kinds were never registered, so its whole data layer fails. Remove Enyo → Mochi's data layer dies with a package it does not own. A direct R7 violation, and `packaging/README.md` currently asserts the opposite. | OPEN |
| F-2 | P1 | `device-independence-test.sh:119` | The R8 "`/media/internal` clean" assertion is `… \| grep -qi` under `pipefail` — the **sixth** instance of this class — and it **fails OPEN**: a real violation reports PASS. | FIXED |
| F-3 | P1 | `packaging/README.md` vs measured behaviour | `ipkg -o <root>` **defers control scripts on both install and removal**, then deletes them — so on the only install path we document (Preware / appinstaller), `prerm` never runs and the rootfs footprint is never reversed: a stale shim keeps registering an NPAPI MIME at boot and a stale upstart job respawns against a missing directory. R8's "exact reversal" is unmet in the field; only the test harness works around it, by hand. | OPEN |
| F-4 | P2 | `jihad-app.inc:202` | The OE `pkg_prerm` still uses `grep -qxF` — the exact BusyBox-1.17.3 trap the direct-build `prerm` was rewritten to avoid (that grep has no `-x`, so it prints usage and returns non-zero for every process, silently disabling the fallback kill). The two shipping paths disagree. | OPEN |
| F-5 | P2 | `EngineHost.cpp:236-238` | `JIHAD_PROFILE_DIR` is the one path env var that **bypasses the R8 guard entirely** — no `RuntimeOnUserVolume` check, no ownership/mode validation. Also: `RuntimeOnUserVolume` is an un-canonicalized prefix test, so `/tmp/../media/internal/x` slips through, and the last-resort fallback now lands in the 0777 cryptofs app bundle. | OPEN |
| F-6 | P2 | `packaging/*/prerm:81` | `prerm` ignores its ipkg argument, so an **upgrade** runs `rm -rf /var/palm/jihad/<V>` — destroying the user's profile, cookies and downloads — and if the new `postinst` then aborts or is deferred, the install is left bricked with the data already gone. | OPEN |
| F-7 | P2 | `packaging/*/postinst:85,93` | `cp -a` from the untrusted 0777 cryptofs bundle **preserves source ownership** for the shim and the upstart job; only the impl is `chown 0:0`'d. The shim is dlopened into privileged LunaSysMgr with no trust check of its own, and the job is executed by init as root. (Exploitability depends on the uid cryptofs reports — UNPROVEN — but the asymmetry with the impl is not.) | OPEN |
| F-8 | P2 | `packaging/*/postinst:81` | `mkdir -p "$IMPLDIR"` leaves `/usr/lib/jihad{,/<V>}` at the inherited umask while the adjacent state dir is explicitly mode-pinned. With `umask 0` they are 0777, so any local uid can unlink or hardlink-swap the impl — and the shim validates the *file*, never its *directory*. | OPEN |
| F-9 | P2 | `packaging/*/prerm:75-89` | `prerm` performs its rootfs removals unchecked and unconditionally reports success: a failed `remount,rw` makes removal a silent no-op while the installer records a clean uninstall. `postinst` added exactly this verification ("believe the filesystem, not `$?`"); the reverse operation never got it. | OPEN |
| F-10 | P2 | `DownloadService.cpp:464-480` | Downloaded files — **user data** — now default into the package-owned, root-only `/var/palm/jihad/<V>/downloads`, invisible to every other app and `rm -rf`'d by `prerm`/upgrade. R8 is about keeping *app internals* off the user's volume; a file the user asked to download is the opposite case. Also fills the 559 MB system partition instead of the multi-GB user volume. | OPEN |
| F-11 | P2 | `device-purge-legacy.sh:37,59-60,66` | The legacy-purge script deletes the **current** Enyo variant's production shim, upstart job and socket (the paths are deliberately identical) and `killall`s every variant's daemon — the two operations the rest of the commit exists to never do. | OPEN |
| F-12 | P3 | `packaging/*/postinst:116`, `prerm:87` | `killall LunaSysMgr` on every install/removal tears down every card of every variant — breaking R7 AC5 from the packaging side — for a plugin re-scan the script's own comment admits it does not achieve (only a reboot does). | OPEN |
| F-13 | P3 | `build-mochi-ipk.sh:127` | Another `\| grep -q` under `pipefail`: the bundled-framework provenance stamp records a **dirty tree as clean**. | OPEN |
| F-14 | P3 | `device-citizen-audit.sh:50,56,59` | The R8 evidence snapshot has blind spots: `/media/internal` is listed top-level only (anything written into an existing subdirectory is invisible), `/usr/lib/jihad` is `-type f` (a leftover empty dir is not residue), `/var/palm/jihad` is `-type d` (residual files invisible). | OPEN |
| F-15 | P3 | `build-variant-ipk.sh:199` | The build-time R8 assertion ends in `\|\| true`, so any pipeline failure makes it pass vacuously. | OPEN |

## Found on-device while triaging this review (not from the review itself)

- **F-16 (P2, FIXED) — `postinst`'s `stop`/`start` races upstart.** `stop` returns before the job
  has finished tearing down, and a `start` issued into that window is dropped ("Job not changed")
  leaving the job STOPPED with no error anywhere. That, not a daemon bug, is why the first matrix
  run showed Mochi and Mojo with no socket and no process while Enyo happened to win the race —
  started by hand each came up cleanly (`engine up; serving YAP 'jihad-browser-mochi'`). Now:
  wait for `stop/waiting`, start, confirm `start/running`, retry up to 3×, and log a warning
  (never a hard failure — a reboot brings it up regardless).
- **F-17 (P1, OPEN) — the deployed ARM daemon predates the R8 path work.** The `.ipk` ships
  `out-arm/jihad-browserserver-arm` built 2026-07-27, i.e. *before* `JihadRuntimePaths.h` existed.
  It ignores `JIHAD_STATE_DIR` entirely, so the daemon's OWN writable paths (engine profile, cache,
  frame dump, inject channel) are **not** yet verified on device — the only reason
  `/var/palm/jihad/<V>/daemon.log` exists is that the upstart job redirects stdout there, and the
  absence of a `profile/` subdirectory under the state dir is the tell. **The packaging half of R8
  is device-verified; the daemon half is not.** Rebuild the ARM daemon and re-verify before
  claiming R8 whole.

## F-18 (P0, OPEN) — the REBUILT ARM daemon crash-loops on device

Rebuilding the ARM daemon (to fix F-17) surfaced a regression in today's daemon changes. Clean A/B,
same engine bundle, same upstart job, same variant — only the daemon binary differs:

| daemon binary | result on device |
|---|---|
| `jihad-browserserver-arm` built **2026-07-27** (pre-R8-paths) | reaches `engine up; serving YAP 'jihad-browser-mochi'`, binds its socket, stays up |
| `jihad-browserserver-arm` built **2026-08-01** (T-057 runtime paths + the F-1…F-9 download fixes) | prints `variant=mochi (JIHAD_BS_NAME=jihad-browser-mochi) state=/var/palm/jihad/mochi`, then dies ~2.6 s later. **117 respawns**, never reaches `engine up`, never binds the socket. |

Evidence: `/var/palm/jihad/mochi/daemon.log`, 9101 lines, 117 × the `variant=…state=` line, exactly
1 line containing `engine`, and **0 `ABORT` lines** — so this is not a `NS_RUNTIMEABORT`; the
process is dying silently (signal, or an early `exit`). The only other repeated line is a
pre-existing, benign `addons.manager … NS_ERROR_ILLEGAL_VALUE [nsIPrefBranch.setCharPref]` that the
old daemon also emitted.

What this does and does not invalidate:
- **Confirmed by the new binary before it dies:** the daemon's own variant derivation works on
  device — `state=/var/palm/jihad/mochi` is printed by `JihadRuntimePaths.h`, and the engine got far
  enough on the FIRST run to create `/var/palm/jihad/mochi/profile/` containing `cookies.sqlite`
  (98304 bytes, root 0644), `prefs.js`, `cache2/` and `startupCache/`. So R8's daemon half and the
  long-open R2 on-device cookie gap are both *demonstrated in principle* — `cookies.sqlite` had
  never once appeared on this device before (it failed on VFAT; ext3 fixed it).
- **Not yet true:** that the daemon is usable. Until this is fixed the shipping `.ipk`s must carry
  the 2026-07-27 daemon, which is what the Enyo package on the device still has (and it runs).

Suspects, in the order worth checking: the `@mozilla.org/transfer;1` factory registration and
`ShutdownDownloadService()` added to `EngineHost` (new XPCOM work at init/shutdown), the profile
dir provider changes, and `RuntimeDirUsable()`'s `access()`/`lstat()` on a path whose parent lives
on a different mount. Reproduce with the daemon alone — no adapter needed, it dies before serving.

## Settled while triaging (both were flagged UNPROVEN by the reviewer)

- **`novacom run` propagates the remote exit status** — `ls` of a missing path → 1, of a real path
  → 0. So the harness's `&& pass || fail` assertions are real and the matrix results are not
  vacuous. This was the single most important thing to check: had it gone the other way, every
  device result in this session would have been worthless.
- **`/var` is its own rw LVM volume** (`/dev/mapper/store-var on /var type ext3 (rw,noatime,…)`),
  independent of `/` (currently `ro`). So `RuntimeDirUsable()`'s write check succeeds regardless of
  the rootfs remount state, and the `/var/palm/jihad/<V>` state dir does **not** silently fall
  through to `/tmp`.

## The two structural lessons

1. **`… | grep -q` under `set -o pipefail` is endemic to this repo.** Six instances found across two
   reviews, in the adapter build scripts, the `.ipk` verifier, the Mochi payload check, the Mochi
   provenance stamp, the device harness's daemon check, and the harness's R8 assertion. Two of them
   failed **open** on the exact assertions R8 exists to enforce. This should be a standing lint.
2. **`ipkg -o` defers control scripts** (F-3). Every acceptance claim that depends on `postinst`/
   `prerm` running needs to state which install path it was verified on, because the documented user
   path does not run them.
