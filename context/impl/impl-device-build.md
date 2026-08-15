---
created: "2026-07-18"
last_edited: "2026-08-15"
---
# Implementation Tracking: device-build (cavekit loop)

Build site: context/plans/build-site.md

| Task | Status | Notes |
|------|--------|-------|
| T-011 | DONE | crosstool-NG GCC 9.4 / glibc 2.23 softfp; C++ ran on TouchPad (pre-loop, see impl-overview.md) |
| T-018 | DONE | headless ARM libxul (29 M stripped) + daemon cross-build; on-device round-trip PASS (pre-loop) |
| T-046 | DONE (build-side) — dual install device-gated | 2026-07-19: build/webos-oe/build-all-device.sh = single entry for all four artifact classes (engine reuse/--engine, daemon+bundle, adapter reuse/--adapter, both ipks; Enyo stage excludes stray repo docs). Validated: ALL ARTIFACTS PRESENT, daemon 33a1aaa0 == on-device deploy. Enyo-1 bundling = intentional deviation (system framework, matches upstream isis) |
| T-047 | DEVICE-GATED | Topaz: renders real pages (2026-07-07); T1–T5 retest + full nav/input matrix pending device reconnect; Opal: no hardware |
| T-048 | DEVICE-GATED | memory budget scenario needs device |
| T-054 | DONE (build-side) — Opal install/kernel [device-gated] | 2026-07-18, commit 41d49f0. Machine confs tenderloin+opal (`build/webos-oe/conf/machine/`), shared jihad-touchpad.inc (DEFAULTTUNE armv7a-neon softfp), COMPATIBLE_MACHINE on engine+daemon recipes, UI recipes allarch. Finding: both models one softfp binary set (same SoC family, same 1024x768; only DPI differs 132 vs 183). docs/DEVICE-BUILD.md Topaz-vs-Opal section. Opal install + kernel string pending hardware [human-review on device]. *(2026-08-15: the kernel-string half is no longer pending hardware — see T-122.)* |
| T-115 | DONE (declaration) — one `git add` + one device pull left | 2026-08-15. The two undeclared prebuilt inputs (Jessie sysroot, adapter-deps) now have checked-in, checksummed, re-obtainable manifests. See the 2026-08-15 section below. |
| T-122 | DONE — pinned, with a corrected value | 2026-08-15. Opal `KERNEL_VERSION_STRING` pinned `=` to `2.6.35-palm-shortloin`; the previous `?= "2.6.35-palm-opal"` was wrong, not merely unverified. Codex F-371 closed. See below. |

## 2026-08-15 — T-122 (Opal kernel string) + T-115 remainder (prebuilt-input manifests)

Both tasks were "declare a thing nobody had written down". Neither needed hardware, which in
one case was itself the finding.

### T-122 — the Opal kernel string was WRONG, not just unverified

`build/webos-oe/conf/machine/opal.conf` carried
`KERNEL_VERSION_STRING ?= "2.6.35-palm-opal"` with a [human-review on device] caveat, and
device-build R6 listed it as a thing to check when an Opal turns up. It never needed an Opal.

**There is no `opal_defconfig` and no `-opal` LOCALVERSION anywhere in HP's kernel tree.**
The TouchPad Go's BOARD is `shortloin`; `opal` is the product codename — the same split
tenderloin.conf already encodes as product `topaz` / board `tenderloin`.

Pinned to **`2.6.35-palm-shortloin`**, derived rather than pattern-matched. Linux computes
`KERNELRELEASE = VERSION.PATCHLEVEL.SUBLEVEL + EXTRAVERSION + CONFIG_LOCALVERSION`, and every
term is fixed by HP's own released Opal kernel drop (webos-internals/webos-linux-kernel branch
`opal`, head `683c4b8`, commit "opal-submission-65.3.7"), all four facts fetched and read
directly rather than taken on report:

- `Makefile` — `VERSION=2 PATCHLEVEL=6 SUBLEVEL=35 EXTRAVERSION=-palm`
- `arch/arm/configs/shortloin_defconfig` — `CONFIG_LOCALVERSION="-shortloin"`,
  `# CONFIG_LOCALVERSION_AUTO is not set`, `CONFIG_MACH_SHORTLOIN=y`
- that branch's `arch/arm/configs/` holds exactly three Palm board configs — `rump`,
  `shortloin`, `tenderloin` — so `shortloin_defconfig` IS the TouchPad Go config
- **control case**: the same tree's `tenderloin_defconfig` has
  `CONFIG_LOCALVERSION="-tenderloin"`, which reproduces `2.6.35-palm-tenderloin` — the string
  already verified on our own TouchPad. Same method, same tree, one output confirmed on
  hardware. That is what upgraded this from plausible to pinnable.

Corroborated by a third party who actually shipped Opal kernels: webOS Internals'
`support/kernel.mk` maps `DEVICE=opal` to `DEFCONFIG=shortloin_defconfig` and
`KERNEL_TYPE=2.6.35-palm-shortloin`, and uses that literal as the `/lib/modules/<KERNEL_TYPE>`
path — if it did not equal `uname -r` on real hardware their modules would not load. Board
identity is separately registered as ARM machine 3080 "Shortloin" (registered by Palm's
Dmitry Fink 82 seconds after 3079 "Tenderloin").

Also changed in that file: `MACHINEOVERRIDES =. "opal:shortloin:"` (was `"opal:"`), matching
tenderloin.conf's product:board pattern; and the comment asserting the Go "has no separate
product alias like topaz", which is exactly backwards, is corrected. `docs/DEVICE-BUILD.md`'s
Topaz-vs-Opal table had the stale `2.6.35-palm-opal (unverified ?=)` row — updated, plus a new
board/product row.

**Residual, stated rather than glossed.** HP's Opal source drop is webOS 3.0.3-era; no Opal
kernel source was ever published for 3.0.5 (a 3.0.5 Opal doctor did exist — no live mirror of
it survives). So the 3.0.3 → 3.0.5 hop is inference. It is sound inference — LOCALVERSION is
board-derived, not release-derived, and tenderloin's string was byte-identical across
3.0.0 → 3.0.5 — but it is not observation, and no `uname` from physical TouchPad Go hardware
exists publicly because the device was cancelled and only prototypes escaped. The one-line
settle is written into the machine conf: `novacom run file://bin/uname -- -r`.

**Lesson worth keeping:** "device-gated" was the wrong classification. The answer had been in
HP's own published kernel source since 2011, and the label stopped anyone looking.

### T-115 remainder — the last two undeclared prebuilt inputs

R3 requires every declared input to be obtainable from a clean clone. Two of the four had
neither a recipe nor a procedure. Both now have a generator script and a checked-in manifest;
both scripts are `bash -n` clean, deterministic (each generated twice and diffed
byte-identical), and self-verifying.

**Sysroot** — `build/webos-oe/gen-sysroot-manifest.sh` → `build/webos-oe/arm-sysroot-debs.manifest`.
Package, version, architecture, sha256, size, source package, on-disk filename and the
archive.debian.org pool URL for every `.deb`. Modes: default regenerate, `--check`, `--fetch`,
`--check-urls`. No dpkg needed — `ar` + `control.tar.gz`, the same trick `fetch-pdk.sh` uses.

- **187 packages, not 185.** Two ALSA debs (`libasound2`, `libasound2-dev` 1.0.28-1, source
  `alsa-lib`) landed in `arm-sysroot/debs/` from a concurrent task mid-generation. Regenerated
  to include them; `--check` is clean at 187/187.
- **Verified against the real archive, not just internally consistent.** Four packages
  downloaded from archive.debian.org and sha256-compared to the manifest — `libpthread-stubs0-dev`,
  `x11proto-xinerama-dev`, `x11proto-xf86vidmode-dev`, `libgcc1` — all byte-identical. Five
  deliberately awkward URL shapes (epoch, `lib`-prefixed source, binNMU, gir, alsa) all 200.
- **`--fetch` tested end-to-end, not just written**: a deb was moved aside, `--fetch` re-downloaded
  it, and the restored file was byte-identical to the original.
- **Two traps that silently 404 and are not derivable from the on-disk filename**, so both are
  recorded as columns: the pool filename DROPS the epoch that the apt-cache filename encodes as
  `%3a` (`libgcc1_1%3a4.9.2-…` → `libgcc1_4.9.2-…`), and the pool is keyed by SOURCE package
  (`libgcc1` lives under `pool/main/g/gcc-4.9/`, not `pool/main/l/libgcc1/`).

**adapter-deps** — `build/webos-oe/gen-adapter-deps-manifest.sh` → `build/webos-oe/adapter-deps.manifest`.
Per-part, because it is four different kinds of thing, and only ONE of them is device-gated.
This corrected the kit in three places:

1. **`qt4-extract/` is NOT an HP build off a TouchPad.** It is three stock Debian jessie armel
   packages — `libqt4-dev`, `libqtcore4`, `libqtgui4`, all
   `qt4-x11 4:4.8.6+git64-g5dc8b2b+dfsg-3+deb8u1` — unpacked. Proven, not inferred: all three
   downloaded from archive.debian.org, and `libQtCore.so.4.8.6` plus `QtCore/qglobal.h`
   extracted from them are BYTE-IDENTICAL to the tree. Now pinned by sha256, with
   `--check-qt4-debs` re-verifying against the archive (passes).
2. **The device `.so` set is 7 ELF files, not 10** — `libQt{Core,Gui,Network}.so.4.8.0`,
   `libpbnjson_{c,cpp}.so`, `libyajl.so.1`, `libpng12.so.0` — plus 10 development symlinks
   created here (the device rootfs ships no `-dev` symlinks and `-lQtGui` needs one), which is
   where the count of 10 came from. Provenance is now evidence: the binaries carry HP's own OE
   build paths, `/home/reviewdaemon/projects/nova/oe/BUILD-topaz/work/{qt4-4.8.0-83,pbnjson-1.0.1-38}`.
   These seven are the only genuinely device-gated part of the entire carve-out; they get a
   written `novacom get` procedure.
3. **4 of the 5 NPAPI headers are the pinned `npapi-headers` checkout** (byte-compared), so a
   clean clone regenerates them. Only `nppalmdefs.h` (341 bytes) is Palm-only and travels with
   the device drop.

Also recorded, because it is load-bearing and invites a wrong "cleanup": a three-way VERSION
SKEW. The headers this repo pins are pbnjson `submissions/10` (`c4b0611`) / yajl 1.0.12
(`17b1790`) / Qt 4.8.6; the libraries linked against are pbnjson submission-38 / yajl 1.0.7 /
Qt 4.8.0. `build-adapter-pdk.sh` chose `submissions/10` deliberately ("the device era") so gcc4
instantiates what the device `libpbnjson_cpp` exports. Bumping these to match each other is a
hardware retest, not a tidy-up.

**Bug found and fixed during validation:** the header-origin classifier took the first
same-named file in the `npapi-headers` checkout, which is a debhelper staging copy under
`debian/npapiheaders/usr/include/` that sorts ahead of the real header and does not match. It
misfiled `npapi.h` as `palm-only`. Now compares against every candidate.

**What is left on R3, and neither is a missing declaration:**
- the three toolchain recipe files (`toolchain/{Dockerfile,build-toolchain.sh,jihad.defconfig}`)
  are STILL untracked, so a clean clone still does not get them. Only a `git add` closes it —
  deliberately not done here, since this session commits nothing.
- the `novacom get` procedure for the seven device `.so`s is written but has never been run,
  and its `/usr/lib/...` paths are the standard webOS 3 locations rather than paths re-read off
  hardware (no device attached this session). [human-review on device]

**Where the manifests live and why.** `.gitignore` blanket-ignores
`/build/webos-oe/arm-sysroot/` and each `adapter-deps/` subdirectory, and git never descends
into an ignored directory — the same trap the 2026-08-10 toolchain work hit. Rather than add
more `!` re-includes, both manifests and both generators sit at `build/webos-oe/` top level,
which nothing ignores; verified with `git check-ignore` (all four trackable).
