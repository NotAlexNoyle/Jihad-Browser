# `packaging/` — three independent, good-citizen on-device packages

Jihad Browser ships as **three fully standalone browsers** — Enyo 1.0, Enyo 2/Mochi, and Mojo.
Each has its own NPAPI MIME type, adapter shim, adapter impl, YAP service name, socket, upstart
job, daemon process, and runtime state directory. **No file on the device is owned by more than
one of them**, so installing, upgrading, or removing any one has no effect on the others and no
refcount logic exists anywhere (cavekit-device-build.md **R7**).

Each package also behaves the way webOS expects a third-party app to behave (**R8**): the engine
runs **in place** from the app's own bundle, nothing is written to the user's storage, no stock
file is touched, and every rootfs write is reversed exactly on removal. The authoritative name
table is `context/plans/plan-variant-identity.md`.

## Identity

| | **Enyo 1.0** | **Enyo 2 / Mochi** | **Mojo** | Stock (untouched) |
|---|---|---|---|---|
| variant token | `enyo` | `mochi` | `mojo` | — |
| app id | `net.riverstonerelay.jihad-browser` | `…jihad-browser.mochi` | `…jihad-browser.mojo` | — |
| NPAPI MIME | `application/x-jihad-browser` | `application/x-jihad-browser-mochi` | `application/x-jihad-browser-mojo` | `application/x-palm-browser` |
| adapter shim | `BrowserAdapterJihad.so` | `BrowserAdapterJihadMochi.so` | `BrowserAdapterJihadMojo.so` | `BrowserAdapter.so` |
| YAP name | `jihad-browser` | `jihad-browser-mochi` | `jihad-browser-mojo` | `browser` |
| socket | `/tmp/yapserver.jihad-browser` | `/tmp/yapserver.jihad-browser-mochi` | `/tmp/yapserver.jihad-browser-mojo` | `/tmp/yapserver.browser` |
| upstart job | `/etc/event.d/jihad` | `/etc/event.d/jihad-mochi` | `/etc/event.d/jihad-mojo` | `browserserver` |

`$APP` below = `/media/cryptofs/apps/usr/palm/applications/<app id>`.

## Install footprint — exactly what each package puts OUTSIDE its own app directory

**R8** requires this enumeration to exist. These lists are complete: `postinst` creates exactly
these paths and nothing else, and the matching `prerm` removes exactly these paths and nothing
else. Read them straight off `packaging/<variant>/postinst` and `packaging/<variant>/prerm` —
both scripts declare the same set at the top of the file.

### Enyo 1.0 — `net.riverstonerelay.jihad-browser`

| Path | Created by `postinst` | Removed by `prerm` |
|---|---|---|
| `/usr/lib/BrowserPlugins/BrowserAdapterJihad.so` | yes, 0755 | yes |
| `/usr/lib/jihad/enyo/BrowserAdapterImpl.so` | yes, root:root 0644 | yes |
| `/usr/lib/jihad/enyo/` | yes | yes (`rmdir`) |
| `/usr/lib/jihad/` | yes (`mkdir -p`) | `rmdir` only — succeeds only when the last variant is gone |
| `/etc/event.d/jihad` | yes, 0755 | yes |
| `/var/palm/jihad/enyo/` (holds `daemon.log`) | yes, root:root 0755 | yes (`rm -rf`) |
| `/var/palm/jihad/` | yes (`mkdir -p`), root:root 0755 | `rmdir` only — succeeds only when the last variant is gone |
| `/tmp/yapserver.jihad-browser` | created by the daemon at runtime | yes (exact path) |

### Enyo 2 / Mochi — `net.riverstonerelay.jihad-browser.mochi`

| Path | Created by `postinst` | Removed by `prerm` |
|---|---|---|
| `/usr/lib/BrowserPlugins/BrowserAdapterJihadMochi.so` | yes, 0755 | yes |
| `/usr/lib/jihad/mochi/BrowserAdapterImpl.so` | yes, root:root 0644 | yes |
| `/usr/lib/jihad/mochi/` | yes | yes (`rmdir`) |
| `/usr/lib/jihad/` | yes (`mkdir -p`) | `rmdir` only |
| `/etc/event.d/jihad-mochi` | yes, 0755 | yes |
| `/var/palm/jihad/mochi/` (holds `daemon.log`) | yes, root:root 0755 | yes (`rm -rf`) |
| `/var/palm/jihad/` | yes (`mkdir -p`), root:root 0755 | `rmdir` only |
| `/tmp/yapserver.jihad-browser-mochi` | created by the daemon at runtime | yes (exact path) |

### Mojo — `net.riverstonerelay.jihad-browser.mojo`

| Path | Created by `postinst` | Removed by `prerm` |
|---|---|---|
| `/usr/lib/BrowserPlugins/BrowserAdapterJihadMojo.so` | yes, 0755 | yes |
| `/usr/lib/jihad/mojo/BrowserAdapterImpl.so` | yes, root:root 0644 | yes |
| `/usr/lib/jihad/mojo/` | yes | yes (`rmdir`) |
| `/usr/lib/jihad/` | yes (`mkdir -p`) | `rmdir` only |
| `/etc/event.d/jihad-mojo` | yes, 0755 | yes |
| `/var/palm/jihad/mojo/` (holds `daemon.log`) | yes, root:root 0755 | yes (`rm -rf`) |
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

### Upgrading from a pre-2026-07-31 build

Older Jihad builds copied the runtime to `/media/internal/jihad/hl` and logged to
`/media/internal/jihad/upstart-daemon.log`. Nothing installs or removes that path any more, and
this package will not delete a directory it did not create in this install. Clear the leftovers
by hand once: `rm -rf /media/internal/jihad`.

## Files here

- **`gen-variant-scripts.sh`** — the template + identity table that GENERATES the nine artifacts
  below. It is the only place a variant-specific string is written, which is what makes a
  cross-variant reference impossible to introduce by accident. `--check` fails if any generated
  file has drifted from the template.
- **`<variant>/postinst`** — GENERATED. Verifies the payload, makes the in-place daemon
  executable, then opens ONE rootfs read-write window to install the shim + impl + upstart job +
  state dir, verifies they landed, and restarts that variant's job. `set -e` throughout, and a
  `trap … EXIT` restores `/` to read-only on **every** exit path including failure.
- **`<variant>/prerm`** — GENERATED. Stops that variant's job, kills only that variant's daemon,
  and removes exactly the paths enumerated above. No refcount, no globs.
- **`event.d/<job>`** — GENERATED. That variant's upstart job. It runs the daemon in place from
  `$APP/deviceroot/hl` via the bundled loader, sets `JIHAD_BS_NAME` to that variant's YAP name
  and `JIHAD_STATE_DIR` to `/var/palm/jihad/<variant>`, and logs to
  `/var/palm/jihad/<variant>/daemon.log`.

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
`build/webos-oe/recipes-jihad/jihad-ui/jihad-app.inc` (`pkg_postinst`/`pkg_prerm` mirror
`packaging/<variant>/`) plus `…/jihad-deviceroot/`, driven by the same identity table in
`build/webos-oe/recipes-jihad/jihad-variants.inc`.

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
   does **not** re-scan, so the new MIME stays unregistered until the next boot. Confirm with
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
