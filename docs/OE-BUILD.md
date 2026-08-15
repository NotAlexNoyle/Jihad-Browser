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

- Downloads Ubuntu's official **`ubuntu-base-14.04`** rootfs tarball once (just `curl`+`tar`+`gpg`
  — `gpg` pins the GCC PPA's signing key, see below; no `debootstrap`, no distro packages) and
  enters it with plain POSIX **`chroot`**.
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

## apt authentication in the chroot (the GCC 9 PPA)

UXP's *host* tools need GCC ≥ 9.1 while the dylan OE natives need trusty's stock 4.8, so the
rootfs installs `gcc-9`/`g++-9` from the **ubuntu-toolchain-r/test** PPA alongside 4.8. That repo
is **authenticated** — apt verification is never disabled (review #2):

- The PPA signing key is pinned by **full fingerprint**
  `60C317803A41BA51845E371A1E9377A2BA9EF27F` ("Launchpad Toolchain builds").
- `oe-env.sh` installs it host-side into its own keyring, `/etc/apt/trusted.gpg.d/ubuntu-toolchain-r.gpg`
  inside the rootfs. (trusty ships apt 1.0.1, which predates the `[signed-by=…]` sources.list
  option added in apt 1.1, so a dedicated `trusted.gpg.d` keyring is the tightest scoping this
  apt supports.)
- Key source: the **pre-seeded** copy at `build/webos-oe/keys/ubuntu-toolchain-r.asc` if present
  (works offline), otherwise fetched over HTTPS from `keyserver.ubuntu.com`.
- It **fails closed**: a fingerprint mismatch, unreadable key material, or a missing keyring
  aborts provisioning — it never falls back to `[trusted=yes]`. A rootfs provisioned by an older
  `oe-env.sh` (which did use `[trusted=yes]`) is repaired in place on the next `provision`.

To re-seed the in-repo key (e.g. after a key rotation), or to seed it on a machine that will
build offline:

```bash
gpg --keyserver hkps://keyserver.ubuntu.com --recv-keys 60C317803A41BA51845E371A1E9377A2BA9EF27F
gpg --armor --export 60C317803A41BA51845E371A1E9377A2BA9EF27F \
  > build/webos-oe/keys/ubuntu-toolchain-r.asc
```

`PPA_KEY_SEED=<path>` and `PPA_KEYSERVER=<url>` override the seed path and the keyserver.

## Task signatures and `${JIHAD_REPO}` inputs

The Jihad recipes read several build-control files and prebuilt trees **straight from the bound
repo** (`${JIHAD_REPO}`) instead of through `SRC_URI` — the goanna patch queue and ARM mozconfig,
the deviceroot bundler + upstart job, the PDK adapter build script and its sources, the
crosstool-NG toolchain, the Jessie sysroot, the Palm PDK, `adapter-deps/`, the Mochi frameworks,
and the LICENSE/NOTICE payload. The fetcher never sees those, so nothing would invalidate sstate
when they change (review #8).

Each task that reads such an input now declares it in its **own signature** with bitbake's
per-task `file-checksums` varflag (supported in the pinned bitbake 1.18.0: `lib/bb/cache.py:135`
collects the flag, `lib/bb/siggen.py:189-193` folds every listed file's md5 into the task hash).
The two large prebuilt trees are represented by small identity sets rather than whole-tree walks —
`JIHAD_TC_SIG` (gcc drivers + crosstool-NG build log) and `JIHAD_SYS_SIG` (the sysroot's
pkg-config manifest, i.e. exactly what `PKG_CONFIG_LIBDIR` points at). See
`build/webos-oe/recipes-jihad/jihad-common.inc` for the full rationale and the glob rules the
1.18 implementation imposes.

Because the declarations change every affected task hash, the **first build after this change
rebuilds** those recipes once instead of restoring from sstate.

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

Tracked as one acceptance criterion — `context/kits/cavekit-device-build.md` R3 — not restated
here. The short version a reader of this file needs: **the OE environment parses and is runnable,
but the `do_compile`s do not yet cross-build**, so the direct cross-build scripts in
`docs/DEVICE-BUILD.md` remain the verified pipeline and are what every current binary was built
with.

Two fixes are worth keeping here because they are properties of the metadata rather than of the
requirement:

- `recipes-jihad/jihad-common.inc` points `FILESEXTRAPATHS` at the repo root (`/oe/Jihad-Browser`)
  so `file://` sources resolve, and supplies the real `LIC_FILES_CHKSUM` (repo `LICENSE`) + the
  webOS app dir.
- UI recipes drop the nonexistent `webos-app` class → `inherit allarch`; the modern `:`-override in
  goanna → dylan underscore; the adapter `SRC_URI` → the vendored `render/adapter/`.
