# Full-OE build — `oe-env.sh` (OpenEmbedded "dylan" host, any Linux)

The reproducible-from-source device build: `bitbake` the Jihad recipes under the same
2013 **OpenEmbedded "dylan" / BitBake 1.18** stack that the openwebos `meta-webos` layer
(correct for the HP TouchPad) targets. This is the alternative to the direct cross-build
scripts — chosen so the whole stack builds from source with no prebuilt device libraries.

## Why a rootfs at all

OpenEmbedded/BitBake is a **Linux-host-only** build system by design: its `pseudo`
fakeroot is `LD_PRELOAD`-based and the recipes/host-tools assume GNU/Linux. It has no
native macOS/*BSD port. On top of that, the *dylan* stack is **Python-2 / Ubuntu-14.04-era**,
so it will not run on a modern host userland (Void, Arch, current Ubuntu, …). The build
therefore needs an old GNU/Linux userland regardless of what you run on.

`oe-env.sh` supplies exactly that userland — **one clean, self-contained solution that spans
Linux** — without a container runtime:

- Downloads Ubuntu's official **`ubuntu-base-14.04`** rootfs tarball once (just `curl`+`tar`;
  no `debootstrap`, no distro packages) and enters it with plain POSIX **`chroot`**.
- `chroot` exists on **every** Linux and **every** init system — runit / OpenRC / s6 /
  systemd, **no systemd dependency**. The rootfs carries its own glibc, so it runs identically
  on a glibc host or a **musl** host (Alpine, musl-Void). Native speed (no `proot`/ptrace).
- Privilege for `chroot`/`mount` is taken via **`sudo`** or **`doas`** (whichever the host
  has), or run directly if already root. Override with `OE_ROOTCMD=...`.

(There is no macOS/*BSD path: the *target* is Linux/ARM/glibc, so a Linux build host is
unavoidable — on a non-Linux Unix, run this inside a Linux VM.)

## Quick start

```bash
# 1. Build the OE host rootfs (once). Prompts for sudo/doas; downloads ~65 MB + apt.
build/webos-oe/oe-env.sh provision

# 2. Bring up the OE tree: clone+pin the dylan layers, fetch, wire the Jihad layer.
build/webos-oe/oe-env.sh bringup tenderloin          # or: opal

# 3. Build (inside the chroot). A full image needs ~40 GB free.
build/webos-oe/oe-env.sh make webos-image
#   …or a single recipe while iterating:
build/webos-oe/oe-env.sh run "bitbake goanna"
```

Everything after `provision` runs inside the rootfs as a `builder` user that shares your
host UID/GID, so bind-mounted files stay owned by you.

## Commands

| Command | What it does |
|---------|--------------|
| `provision` | Fetch + extract the rootfs, install the OE host tools (idempotent). |
| `bringup [machine]` | `provision`, then `./mcf -p 0 -b 0 <machine>` (clone+pin layers, fetch), then wire the Jihad layer into `BBLAYERS`. Default machine `tenderloin`. |
| `mcf <args…>` | Run `./mcf` inside build-webos with your args. |
| `wire` | (Re)add the Jihad layer to `conf/bblayers.conf`, idempotent. |
| `make <args…>` | `make` inside build-webos (e.g. `make webos-image`). |
| `run <cmd…>` | Run any command as `builder` in the build-webos dir (e.g. `run "bitbake goanna"`). |
| `shell` | Interactive shell inside the chroot. |
| `root <cmd…>` | Run as root inside (debug/provisioning). |
| `clean` | Remove the rootfs (keeps `oe-downloads/` + `oe-sstate/`). |

## Layout

Inside the chroot:

| Path | Bind |
|------|------|
| `/oe/Jihad-Browser` | this repo |
| `/oe/build-webos` | the build-webos orchestrator (cloned + pinned by `oe-env.sh`) |
| `/oe/downloads` | persistent `DL_DIR` (host `build/webos-oe/oe-downloads/`, survives `clean`) |
| `/oe/sstate` | persistent sstate (host `build/webos-oe/oe-sstate/`, survives `clean`) |

Host-side, all generated state is git-ignored: `oe-rootfs/`, `oe-cache/`, `oe-downloads/`,
`oe-sstate/`.

## How the Jihad layer is wired

`oe-env.sh` vendors **`build/webos-oe/weboslayers.py`** (a copy of build-webos's, pinning the
dylan-era layers: bitbake 1.18, oe-core/meta-oe dylan, the legacy openwebos meta-webos) and
installs it over build-webos's before running mcf — so the layer pins live in *this* repo and
reproduce from a clean clone. Its only change vs upstream is adding **`tenderloin`** and
**`opal`** to the `Machines` list so `./mcf tenderloin` is accepted.

The Jihad layer itself (`build/webos-oe/`, which already has `conf/layer.conf` +
`recipes-jihad/` + the machine confs) is **not** added as a `weboslayers.py` tuple — that would
put it under mcf's git layer-management. Instead `oe-env.sh` appends it to `conf/bblayers.conf`
**after** mcf, pointing at the in-chroot path `/oe/Jihad-Browser/build/webos-oe`. Idempotent.

## Status

The **environment is runnable and proven**: `oe-env.sh provision` + `bringup tenderloin` build
the Ubuntu-14.04 host, clone+pin the dylan layers, and wire the Jihad layer; `bitbake -p` then
**parses the full metadata (1419 recipes) with 0 errors**, including all five Jihad recipes, and
every in-repo `SRC_URI` (`render/`, `app/`, `packaging/`, …) resolves via the repo bind. The
early blockers are fixed:

- `recipes-jihad/jihad-common.inc` (new) points `FILESEXTRAPATHS` at the repo root (`/oe/Jihad-Browser`)
  so `file://` sources resolve, and supplies the real `LIC_FILES_CHKSUM` (repo `LICENSE`) + the
  webOS app dir.
- UI recipes drop the nonexistent `webos-app` class → `inherit allarch`; the modern `:`-override
  in goanna → dylan underscore; the adapter `SRC_URI` → the vendored `render/adapter/`.

**Remaining (Phase B — the real compiles):** the `do_compile`s still need to actually cross-build:
`goanna` (libxul via the ARM mozconfig + patch queue — and a `jihad-cross-toolchain-native`
provider, since stock dylan gcc can't build UXP), then `jihad-browserserver` and
`browser-adapter-jihad` (the latter against the Palm PDK, `fetch-pdk.sh`). This is iterated against
live `bitbake` inside `oe-env.sh`. Until those land, the direct cross-build scripts
(`docs/DEVICE-BUILD.md`) remain the verified pipeline.
