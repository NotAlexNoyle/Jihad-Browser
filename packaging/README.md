# `packaging/` — three independent, good-citizen on-device packages

Jihad Browser ships as **three fully standalone browsers** — Enyo 1.0, Enyo 2/Mochi, and Mojo.
Each has its own NPAPI MIME type, adapter shim, adapter impl, YAP service name, socket, upstart
job, daemon process, and runtime state directory. **No file on the device is owned by more than
one of them**, so installing, upgrading, or removing any one has no effect on the others and no
refcount logic exists anywhere (cavekit-device-build.md **R7**).

Each package also behaves the way webOS expects a third-party app to behave (**R8**): the engine
runs **in place** from the app's own bundle, no *app internals* are written to the user's storage,
no stock file is touched, and every rootfs write is reversed exactly on removal. The authoritative
name table is `context/plans/plan-variant-identity.md`.

> **Install via Preware or WebOS Quick Install.** Those are the supported paths, and they DO run
> `postinst`/`prerm` — the same constraint the Atlas browser ships under. `palm-install` (the SDK
> dev shortcut) runs as a non-root user and does **not** execute control scripts, and a raw
> `ipkg -o <root>` defers and then deletes them; under either one the shim, impl and upstart job
> are never laid down and, on removal, never taken away. Every footprint claim below is a claim
> about the Preware / WQI path.

## Identity

| | **Enyo 1.0** | **Enyo 2 / Mochi** | **Mojo** | Stock (untouched) |
|---|---|---|---|---|
| variant token | `enyo` | `mochi` | `mojo` | — |
| app id | `net.riverstonerelay.jihad-browser` | `…jihad-browser-mochi` | `…jihad-browser-mojo` | — |
| NPAPI MIME | `application/x-jihad-browser` | `application/x-jihad-browser-mochi` | `application/x-jihad-browser-mojo` | `application/x-palm-browser` |
| adapter shim | `BrowserAdapterJihad.so` | `BrowserAdapterJihadMochi.so` | `BrowserAdapterJihadMojo.so` | `BrowserAdapter.so` |
| YAP name | `jihad-browser` | `jihad-browser-mochi` | `jihad-browser-mojo` | `browser` |
| socket | `/tmp/yapserver.jihad-browser` | `/tmp/yapserver.jihad-browser-mochi` | `/tmp/yapserver.jihad-browser-mojo` | `/tmp/yapserver.browser` |
| upstart job | `/etc/event.d/jihad` | `/etc/event.d/jihad-mochi` | `/etc/event.d/jihad-mojo` | `browserserver` |

`$APP` below = `/media/cryptofs/apps/usr/palm/applications/<app id>`.

**The suffixed app ids use a HYPHEN, never a dot.** `ipkg` keeps package metadata as
`info/<pkgid>.{control,list,prerm}` and removes a package by globbing `<pkgid>.*` — which also
matches `<pkgid>.child.control`. Dotted ids made Mochi and Mojo *dot-children* of the Enyo
package, so removing Enyo deleted their control scripts and file lists outright: they became
un-uninstallable and their shim, impl and upstart job permanent residue (P1 against R7 and R8).
Proven on-device 2026-08-01 with a dot pair and a hyphen pair —
`../context/impl/impl-ipkg-prefix-collision.md`. Any future variant id follows the same rule.

## Install footprint — exactly what each package puts OUTSIDE its own app directory

**R8** requires this enumeration to exist. These lists are complete: `postinst` creates exactly
these paths and nothing else, and the matching `prerm` removes exactly these paths and nothing
else. Read them straight off `packaging/<variant>/postinst` and `packaging/<variant>/prerm` —
both scripts declare the same set at the top of the file.

### Enyo 1.0 — `net.riverstonerelay.jihad-browser`

| Path | Created by `postinst` | Removed by `prerm` |
|---|---|---|
| `/usr/lib/BrowserPlugins/BrowserAdapterJihad.so` | yes, root:root 0755 | yes |
| `/usr/lib/jihad/enyo/BrowserAdapterImpl.so` | yes, root:root 0644 | yes |
| `/usr/lib/jihad/enyo/` | yes, root:root 0755 (mode-pinned) | yes (`rmdir`) |
| `/usr/lib/jihad/` | yes (`mkdir -p`), root:root 0755 (mode-pinned) | `rmdir` only — succeeds only when the last variant is gone |
| `/etc/event.d/jihad` | yes, root:root **0644** | yes |
| `/var/palm/jihad/enyo/` (daemon log + debug channels ONLY) | yes, root:root 0755 | yes (`rm -rf`) |
| `$APP/profile/` (engine `ProfD`: cookies, prefs, permissions) | no — the **daemon** creates it at runtime | yes (`rm -rf`, real removal only) |
| `$APP/cache/` (engine `ProfLD`: `cache2`, `startupCache`) | no — the **daemon** creates it at runtime | yes (`rm -rf`, real removal only) |
| `/var/palm/jihad/` | yes (`mkdir -p`), root:root 0755 | `rmdir` only — succeeds only when the last variant is gone |
| `/tmp/yapserver.jihad-browser` | created by the daemon at runtime | yes (exact path) |

### Enyo 2 / Mochi — `net.riverstonerelay.jihad-browser-mochi`

| Path | Created by `postinst` | Removed by `prerm` |
|---|---|---|
| `/usr/lib/BrowserPlugins/BrowserAdapterJihadMochi.so` | yes, root:root 0755 | yes |
| `/usr/lib/jihad/mochi/BrowserAdapterImpl.so` | yes, root:root 0644 | yes |
| `/usr/lib/jihad/mochi/` | yes, root:root 0755 (mode-pinned) | yes (`rmdir`) |
| `/usr/lib/jihad/` | yes (`mkdir -p`), root:root 0755 (mode-pinned) | `rmdir` only |
| `/etc/event.d/jihad-mochi` | yes, root:root **0644** | yes |
| `/var/palm/jihad/mochi/` (daemon log + debug channels ONLY) | yes, root:root 0755 | yes (`rm -rf`) |
| `$APP/profile/` (engine `ProfD`: cookies, prefs, permissions) | no — the **daemon** creates it at runtime | yes (`rm -rf`, real removal only) |
| `$APP/cache/` (engine `ProfLD`: `cache2`, `startupCache`) | no — the **daemon** creates it at runtime | yes (`rm -rf`, real removal only) |
| `/var/palm/jihad/` | yes (`mkdir -p`), root:root 0755 | `rmdir` only |
| `/tmp/yapserver.jihad-browser-mochi` | created by the daemon at runtime | yes (exact path) |

### Mojo — `net.riverstonerelay.jihad-browser-mojo`

| Path | Created by `postinst` | Removed by `prerm` |
|---|---|---|
| `/usr/lib/BrowserPlugins/BrowserAdapterJihadMojo.so` | yes, root:root 0755 | yes |
| `/usr/lib/jihad/mojo/BrowserAdapterImpl.so` | yes, root:root 0644 | yes |
| `/usr/lib/jihad/mojo/` | yes, root:root 0755 (mode-pinned) | yes (`rmdir`) |
| `/usr/lib/jihad/` | yes (`mkdir -p`), root:root 0755 (mode-pinned) | `rmdir` only |
| `/etc/event.d/jihad-mojo` | yes, root:root **0644** | yes |
| `/var/palm/jihad/mojo/` (daemon log + debug channels ONLY) | yes, root:root 0755 | yes (`rm -rf`) |
| `$APP/profile/` (engine `ProfD`: cookies, prefs, permissions) | no — the **daemon** creates it at runtime | yes (`rm -rf`, real removal only) |
| `$APP/cache/` (engine `ProfLD`: `cache2`, `startupCache`) | no — the **daemon** creates it at runtime | yes (`rm -rf`, real removal only) |
| `/var/palm/jihad/` | yes (`mkdir -p`), root:root 0755 | `rmdir` only |
| `/tmp/yapserver.jihad-browser-mojo` | created by the daemon at runtime | yes (exact path) |

### What is deliberately NOT written

- **Nothing under `/media/internal`.** That vfat volume is the user's USB mass-storage
  partition. The daemon, `libxul.so`, the bundled glibc-2.23 loader and the GRE resources are
  **executed in place** from `$APP/deviceroot/hl` on cryptofs, which the installer removes with
  the app directory. (Measured live 2026-07-31: `/media/cryptofs` is `fuse.cryptofs
  (rw,nosuid,nodev)` and is **not** `noexec`; a binary copied into an app directory was proven
  to execute.) This is the model the Atlas browser uses.
- **No stock file** is modified, replaced, moved, or backed-up-and-swapped. In particular
  `/usr/lib/BrowserPlugins/BrowserAdapter.so`, `…/BrowserAdapterMojo.so` and
  `/etc/event.d/browserserver` keep their stock bytes; the stock `BrowserServer` keeps serving
  `/tmp/yapserver.browser` for every other app.
- **No file owned by another variant.** Every path above carries its variant's token. There is
  no glob anywhere that could span variants — a stray `rm -f /tmp/yapserver.jihad-browser*`
  would delete the Mochi and Mojo sockets, and `killall jihad-browserserver` would kill their
  daemons (all three ship a binary with that name), so `prerm` matches its own daemon by the
  exact app-bundle path in the process's argv instead.
- **No `killall LunaSysMgr`** (review F-12). It tears down every card of every app, including the
  other variants' — R7 isolation broken from the packaging side — in exchange for a plugin
  re-scan it does not perform. `postinst` prints a reboot instruction instead.

### Where the engine's own data lives

| | Path | Why |
|---|---|---|
| daemon log + debug channels | `/var/palm/jihad/<variant>/` | Kilobytes of operational state for a root daemon; correctly on ext3, root-owned. **Nothing else** goes here — `/var` has 49.6 MB free and is shared with system state. |
| engine profile (`ProfD`) | `$APP/profile/` | cookies.sqlite, prefs.js, permissions, cert overrides. |
| engine local profile (`ProfLD`) | `$APP/cache/` | `cache2`, `startupCache`. Capped at **50 MB** (`browser.cache.disk.capacity`). |
| finished downloads | `/media/internal/downloads` | USER data — see below. |

Both profile halves are on **cryptofs**, in the app's own directory, because that is where both
prior implementations of this browser on this device put them:

- **isis**, our own upstream — still in the fork at `render/browserserver/Src/Settings.cpp:65-74`
  (dead Qt code now, but it is the shipped configuration of the browser we forked):
  `CachePath=/media/cryptofs/.browser/cache`, `CookieJarPath=/media/cryptofs/.browser/cookies`,
  `CacheMaxSize="50M"`, `DownloadPath=/media/internal/downloads`.
- **Atlas** routes `netdata`, `netcache` and `cookies.db` to its app `deviceroot` on cryptofs, and
  says why: `/media/internal` is VFAT — no hard links, no real file locking — which breaks the
  network cache *and* SQLite-backed storage. That also explains our own 2026-07-20 device failure,
  where `cookies.sqlite` was never created at all.

Tradeoff, stated deliberately: cryptofs is FUSE, so `cache2`'s many-small-file I/O is slower than
ext3 would be. Both predecessors accepted that; the alternative is a browser cache filling a 62 MB
system partition. The 50 MB cap and `smart_size.enabled=false` are what keep it bounded — cache2
sizes itself from *free space* otherwise, and there are 10.2 GB there.

**R8 note:** the daemon creates `profile/` and `cache/` at runtime, so `ipkg` never tracks them and
would leave both trees (and hence the app directory) behind on removal. Each variant's `prerm`
removes them explicitly, and `device-citizen-audit.sh` snapshots them so residue is visible.

### The one deliberate exception: `/media/internal/downloads`

**Finished downloads are USER data and go to the user's volume**, at
`/media/internal/downloads` — webOS's own convention, and still the `DownloadPath` default in
`render/browserserver/Src/Settings.cpp`. R8 exists to keep *app internals* off that volume; a
file the user explicitly asked the browser to save is the opposite case, and it has to be
reachable from other apps and from USB-drive mode, has to survive an uninstall, and has to fit
(the rootfs is 559 MB, the user volume is multi-GB).

This is exactly **one** destination, spelled once as `RuntimeUserDownloadDir()` in
`render/goanna/JihadRuntimePaths.h`. Nothing else changes: the R8 guard still refuses every
*other* path on that volume, so a stale `$JIHAD_DUMP`, `$JIHAD_STATE_DIR` or `$JIHAD_PROFILE_DIR`
cannot re-colonise it. **Neither `postinst` nor `prerm` touches it** — the packages still write
nothing to `/media/internal`, and an uninstall deliberately leaves the user's downloads alone.

### db8 kinds are per-variant too

Each variant that has a data layer declares, **ships and owns** its own db8 kinds, registered by
the appinstaller from that package's own `db/{kinds,permissions}/`:

| Variant | Kinds | Declared in |
|---|---|---|
| Enyo 1.0 | `net.riverstonerelay.jihad-browser.{history,bookmarks,preferences}:1` | `app/db/` |
| Mochi | `net.riverstonerelay.jihad-browser-mochi.{history,bookmarks,preferences}:1` | `app-mochi/db/` |
| Mojo | *none* — ships no history/bookmarks/preferences UI and makes no `com.palm.db` call | — |

They used to be **co-owned** (review F-1): all three kinds were declared in the Enyo package with
`owner: net.riverstonerelay.jihad-browser`, and `app/db/permissions/*` merely granted the
Mochi/Mojo app ids CRUD on them. Installing Mochi alone therefore registered no kinds at
all and its whole data layer failed; removing the Enyo package destroyed Mochi's data with it —
a direct R7 violation. A db8 kind's `owner` must equal the app id that registers it, so
independence means **separate namespaces, not broader grants**. There are deliberately no
cross-variant permission grants; do not add any. History and bookmarks are consequently
**per variant**, exactly like each variant's engine profile under `/var/palm/jihad/<variant>/`.

### Upgrading from a pre-2026-07-31 build

Older Jihad builds copied the runtime to `/media/internal/jihad/hl` and logged to
`/media/internal/jihad/upstart-daemon.log`. Nothing installs or removes that path any more, and
this package will not delete a directory it did not create in this install. Clear the leftovers
by hand once: `rm -rf /media/internal/jihad`.

## Files here

- **`gen-variant-scripts.sh`** — the template + identity table that GENERATES the nine artifacts
  below **plus the OE mirror**
  `build/webos-oe/recipes-jihad/jihad-ui/jihad-app-scripts.inc`, so the direct-build and
  bitbake shipping paths cannot drift (review F-4). It is the only place a variant-specific string is written, which is what makes a
  cross-variant reference impossible to introduce by accident. `--check` fails if any generated
  file has drifted from the template.
- **`<variant>/postinst`** — GENERATED. Verifies the payload, makes the in-place daemon
  executable, then opens ONE rootfs read-write window to install the shim + impl + upstart job +
  state dir, verifies they landed, and restarts that variant's job. `set -e` throughout, and a
  `trap … EXIT` restores `/` to read-only on **every** exit path including failure.
- **`<variant>/prerm`** — GENERATED. Stops that variant's job, kills only that variant's daemon,
  and — **on a real removal only** — removes exactly the paths enumerated above, then verifies
  they are gone and fails loudly if they are not ("believe the filesystem, not `$?`"). On an
  **upgrade** (`prerm upgrade …`) it removes nothing, so a version bump never destroys the user's
  profile, cookies or downloads (review F-6/F-9). No refcount, no globs.
- **`event.d/<job>`** — GENERATED. That variant's upstart job. It runs the daemon in place from
  `$APP/deviceroot/hl` via the bundled loader, sets `JIHAD_BS_NAME` to that variant's YAP name
  and `JIHAD_STATE_DIR` to `/var/palm/jihad/<variant>`, and logs to
  `/var/palm/jihad/<variant>/daemon.log`. The engine profile and cache are NOT in the state dir
  — they are in `$APP/{profile,cache}` on cryptofs (see above).

> **`LD_LIBRARY_PATH` is set only via `env` on the daemon's `exec` line, never exported into the
> job's shell.** The bundled gcc9 glibc-2.23 is incompatible with the device's 2.8 userland, so
> an exported `LD_LIBRARY_PATH` makes `/bin/rm`, `mkdir`, `touch` — every command in the job —
> load the wrong libc and segfault before the daemon ever execs. This was expensive to find; do
> not "simplify" it.

## Building the packages

```sh
build/webos-oe/build-variant-ipk.sh              # all three .ipks -> build/webos-oe/out-ipk/
build/webos-oe/build-variant-ipk.sh enyo mochi   # or just these
```

`palm-package` alone is **not** enough: it emits a UI-only package whose `control.tar.gz`
contains nothing but `./control`, so none of the install footprint above would ever be created.
`build-variant-ipk.sh` adds the `deviceroot/` runtime and repacks the `.ipk` to inject that
variant's `postinst`/`prerm` (the repack pattern from `webos://knowledge/postinst-packaging`),
verifying the result with `ar t` and `tar -tzf <(ar p … control.tar.gz)` at build time.

The OE/bitbake path produces the same payload from
`build/webos-oe/recipes-jihad/jihad-ui/jihad-app.inc` plus `…/jihad-deviceroot/`, driven by the
same identity table in `build/webos-oe/recipes-jihad/jihad-variants.inc`. Its `pkg_postinst`/
`pkg_prerm` are not written by hand at all: `jihad-app.inc` `require`s the GENERATED
`jihad-app-scripts.inc`, emitted from the same templates as `packaging/<variant>/*`, so the two
paths are byte-identical apart from the human-readable variant label.

**Install via Preware or WebOS Quick Install — not `palm-install`.** `palm-install` runs as a
non-root user and does not execute control scripts, so the shim/impl/upstart would never be
installed.

## Build inputs (from the git-excluded `build/` tree)

- Adapter (×3): `build/webos-oe/build-adapter-pdk.sh` (gcc 4.3.3 PDK) → the shim
  `build-pdk/<shim>.so` and the impl `build-pdk/<variant>/BrowserAdapterImpl.so`. The MIME, YAP
  name, impl path and state dir are compile-time constants, verified by reading the strings back
  out of both binaries.
- Engine + daemon bundle: `build/webos-oe/make-device-bundle.sh` → `device-bundle/` (daemon,
  `libxul.so`, the `.so` closure, the bundled glibc-2.23 loader, NSS modules, GRE resources,
  low-RAM `goanna.js` prefs). Variant-agnostic; each `.ipk` ships its own copy and runs it in
  place from its own app directory.
- Mochi's bundled Enyo 2 + layout + Mochi trees: staged by `build/webos-oe/build-mochi-ipk.sh`
  (`--stage-only` mode, reused by `build-variant-ipk.sh`).

## Two deploy gotchas (must-do)

1. **Reboot after installing a variant's adapter for the first time.** webOS WebKit builds its
   NPAPI MIME/plugin database at BOOT by scanning `/usr/lib/BrowserPlugins`; `killall LunaSysMgr`
   does **not** re-scan (which is why `postinst` no longer runs it — F-12), so the new MIME stays
   unregistered until the next boot. Confirm with
   `grep -l BrowserAdapterJihad /proc/*/maps` after a launch.
2. **App JS changes need a `.ipk` reinstall**, not a loose-file `novacom put` — a push into the
   installed app dir does not bust WebKit's `file://` resource cache, so `JihadEngineOverride.js`
   silently won't run.

## Attribution

The in-place-from-cryptofs install model — leaving the engine in the app's own `deviceroot`
rather than copying it to `/media/internal`, doing every rootfs write in one `remount,rw` window,
and a teardown that leaves the user's storage alone because nothing was written there — is
adapted from the **Atlas browser**, `atlas-browser-app/packaging/{ipk-postinst.sh,ipk-prerm.sh}`
(Apache-2.0, Herman van Hazendonk / Herrie82). See `NOTICE`.
