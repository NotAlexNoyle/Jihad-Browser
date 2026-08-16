---
created: "2026-06-30"
last_edited: "2026-08-16"
---

# Cavekit: Device Build & Packaging

## 2026-08-16 — R3 CLOSED: clean full bitbake, three complete `.ipk`s (READ FIRST)

The OE build now runs end to end: `bitbake net.riverstonerelay.jihad-browser{,-mochi,-mojo}`
reports `639 tasks ... all succeeded`, zero errors, and produces all three product `.ipk`s
(enyo 45M / mochi 46M / mojo 44M). Each was extracted and verified COMPLETE by content, not size:
`jihad-browserserver` (with the per-buffer scrubber fix), `libxul.so`, `plugin-container`,
`ld-2.23.so`, `goanna.js`, and the full loose chrome (1104 files incl. the scrubber-fixed
`videocontrols.xml`). This is the build-VERIFIED result the 2026-07-29 note below said was still
outstanding — R3 is now DONE.

Five real recipe gaps were found+fixed getting here (all in `build/webos-oe/recipes-jihad/`, and
each recorded in `../plans/build-site.md` T-154 + `dead-ends.md`): a stale patch queue vs pristine
UXP (captured the jihad delta as a durable `jihad-engine-mods` commit + SRCREV, not a drifting
queue); the daemon recipe had drifted from `build-daemon-arm.sh` and was missing three source
files at link; `plugin-container` and the loose GRE dirs (chrome) were not staged into the engine
dist; and a bitbake-1.18 quirk where a SRCREV bump does NOT re-fetch (do_unpack keeps the old
checkout) so a chrome-only change needs `cleansstate` + a content check to actually land. The
`.ipk` app ids are the HYPHENATED form (`…-mochi` / `…-mojo`); the dotted form is dead.

## 2026-07-29 — OE BUILD stands up + produces `.ipk`s, but R3 is NOT complete (superseded above)

The reproducible Open webOS path (previously "aspirational, not runnable") now **stands up and
produces `.ipk`s** under `build-webos` + `meta-webos` (2013 "dylan" / bitbake 1.18). But an
adversarial review (gpt-5.6-sol as the cavekit inspector, 2026-07-29 — see
`../impl/impl-review-findings-oe.md`) found R3 was **over-claimed**; it is **build-produced, not
done**. Honest status:
- **Environment:** `build/webos-oe/oe-env.sh` — an OE host via a chroot into a downloaded Ubuntu-14.04
  rootfs (`sudo`/`doas`). Caveat: **x86_64 hosts only** (AMD64 rootfs), and it consumes four
  prebuilt, git-ignored inputs (crosstool-NG toolchain, Jessie sysroot, Palm PDK, adapter-deps) —
  so **not "whole stack from source"** (finding #7). Those four are enumerated, sized and bounded
  in R3's **prebuilt-inputs carve-out** as of 2026-08-10 rather than left as an open gap: the PDK
  is proprietary and permanent, the other three are not. See `docs/OE-BUILD.md`.
- **Recipes** build: `goanna` (libxul), `jihad-browserserver`, `browser-adapter-jihad`,
  `jihad-cross-toolchain-native`, `jihad-deviceroot` (assembles daemon + libxul `.so` closure +
  bundled glibc-2.23 + NSS + GRE). The four component recipes are stage-only; the shipping
  deliverable is **two self-contained app `.ipk`s** (Enyo 39 MB, Mochi 38 MB) + a **Mojo skeleton**.
- **Review fixes applied + build-verified (2026-07-29):** Mochi `.ipk` now stages its
  Enyo2/layout/Mochi frameworks (#1); the shim loads a SHARED root-owned rootfs impl
  `/usr/lib/jihad/BrowserAdapterImpl.so` so Mochi/Mojo load their own impl (#5); `prerm` refcounts
  siblings so removing one variant no longer breaks the other (#4); each `.ipk` ships LICENSE/NOTICE
  (#12); `postinst` fails loud (#6); `oe-env.sh` HTTPS+SHA256 rootfs, `validate_rootfs`, fail-closed
  unmount, pin check, root-uid guard (#2/#3/#10/#11); the bundle assembler aborts on a missing
  required file (#9). Both `.ipk`s (Enyo 40 MB, Mochi 42 MB) are now structurally complete +
  coexistence-safe + license-compliant.
- **2026-07-31 (applied, BUILD-UNVERIFIED — the chroot needs an interactive sudo password, so the
  OE build was NOT run this session; verify at the next `oe-env.sh` run):** every task reading a
  `${JIHAD_REPO}` input now declares it via `do_<task>[file-checksums]`, which the pinned bitbake
  1.18.0 does support (`lib/bb/cache.py:135`, `lib/bb/siggen.py:189-193`) — so sstate stops going
  stale (#8); and the GCC PPA's `[trusted=yes]` is replaced by a full-fingerprint-pinned signing
  key in its own `trusted.gpg.d` keyring, seed-first + keyserver-fallback, failing closed (#2
  residual). Static verification only: bitbake's own line grammar re-run over the recipes,
  `get_file_checksums` semantics simulated, `bash -n`, and `install_ppa_key()` exercised standalone
  (good/decoy/garbage key). See `../impl/impl-review-findings-oe.md`.
- **STILL OPEN for R3:** (a) **the three-variant bitbake run** — no recorded run has built all
  three `.ipk`s. The 2026-07-29 run named two recipes and predates both the Mojo front-end
  (2026-08-05) and the per-variant adapter split (2026-07-31, e36c8cc), so the per-variant
  `do_install` loops have never been executed by bitbake; and #8 is closed in code but
  static-verified only (T-154). *(2026-08-10, T-115: "clean-clone reproducibility" USED to be
  item (a). It was never a meetable criterion — one of the four inputs is a proprietary HP SDK
  that can be neither redistributed nor rebuilt — so it is replaced by R3's prebuilt-inputs
  carve-out, which names all four and says which are obtainable and how.)*
  (b) **on-device verification** of the `.ipk` install + render + coexistence (device-gated). See
  `../impl/impl-review-findings-oe.md`.

## Scope
Phase-2 work to run Jihad Browser on the HP TouchPad (webOS 3.0.x, ARMv7):
standing up a modern cross-toolchain, cross-compiling the engine and daemon,
packaging, and validating on-device. The toolchain is a feasibility gate. Jihad
packages as a **self-contained app that coexists with the stock browser** — its own
NPAPI MIME/adapter/daemon/upstart job, nothing system-level replaced (see
`jihad-self-contained-arch.md`). Reference: `docs/TOOLCHAIN.md`, `docs/DEVICE-BUILD.md`,
`build/webos-oe/README.md`. (The `build/` tree — toolchain, engine objects, vendored
adapter deps — is git-excluded to avoid redistributing third-party binaries; the build
scripts/recipes and `packaging/` are the reproducible source.)

## 2026-07-31 — packaging model changed by user decision (READ BEFORE TOUCHING PACKAGING)

Two decisions supersede the 2026-07-29 shipping model and are now carried by **R7** and **R8**:
1. **Three independent packages.** Enyo, Mochi, and Mojo each ship a complete browser with their own
   MIME / shim / impl / YAP name / socket / upstart job / daemon process. The shared-runtime +
   `prerm` refcount model (2026-07-29 review fixes #4/#5) is retired — nothing is co-owned, so there
   is nothing to refcount.
2. **Nothing on the user's storage.** `/media/internal` is the user's USB mass-storage volume; the
   package must not write there. The engine + daemon run **in place from the app's own `deviceroot/`
   on cryptofs**, which is the Atlas browser's model (`Atlas/atlas-browser-app/packaging/`: "NOTHING
   is copied to /media/internal — that vfat partition is the user's USB storage and must stay free of
   app internals"). Investigated per the user's request: Atlas found **no** way around the one
   unavoidable rootfs write — webOS's WebKit only scans `/usr/lib/BrowserPlugins` at boot, so the
   NPAPI adapter must be installed there (Atlas README calls it "the **only** path the app loads it
   from"). R8 therefore bounds and reverses that footprint rather than eliminating it.

## Requirements

### R1: Modern ARMv7 cross-toolchain (feasibility gate)
**Description:** A toolchain capable of building the engine for the device exists, since the stock device compiler cannot.
**Acceptance Criteria:**
- [x] A documented, reproducible cross-toolchain (modern C++14-capable compiler) targets the device's glibc/kernel ABI.
- [x] A trivial C++14 program built with it runs on a webOS 3 device/emulator.
- [x] The toolchain links against (or matches) the device sysroot so binaries do not require a newer glibc than the device has.
**Dependencies:** none

### R2: Engine cross-compiles for webOS 3 ARMv7
**Description:** Goanna builds for the device target.
**Acceptance Criteria:**
- [x] The engine builds with the cross-toolchain against the device sysroot.
- [x] The resulting libraries load on the device without missing-symbol/ABI errors.
- [x] ARMv7 FP/SIMD flags match the device CPU.
**Dependencies:** R1, cavekit-engine-embedding.md (R1)

### R3: Package Jihad as a self-contained app — all three UI variants
**Description:** The whole product packages as installable webOS artifacts that coexist with the stock browser, including ALL THREE front-end variants. **Shipping model: three SELF-CONTAINED app `.ipk`s** (Enyo + Mochi + Mojo), each bundling engine+daemon+adapter+bundled-glibc+UI + a postinst that deploys the coexisting pieces. *(2026-08-10, T-115: this read "two SELF-CONTAINED app `.ipk`s (Enyo + Mochi) … plus a Mojo UI skeleton", frozen at the 2026-07-29 shipping model. The Mojo skeleton became a working front-end on 2026-08-05 — see this requirement's own Mojo criterion and cavekit-mojo-ui.md — and R7 has specified three independent packages since 2026-07-31, so the description had been contradicting its own kit for ten days.)*
**Acceptance Criteria:**
- [x] The daemon + a rebuilt **coexisting** adapter (`BrowserAdapterJihad.so`, MIME `application/x-jihad-browser`, YAP name `jihad-browser`) install ALONGSIDE the stock browser without collision (`packaging/postinst`+`prerm`+`event.d/jihad`). Verified on-device.
- [x] The Enyo UI `.ipk` (`net.riverstonerelay.jihad-browser`, from `app/`) builds (`palm-package app/`) and installs; its WebView is routed to the Jihad engine by `app/source/JihadEngineOverride.js`.
- [x] The build also produces the Mochi variant `.ipk` (`net.riverstonerelay.jihad-browser-mochi`, from `app-mochi/`); both install and can coexist. *(Produced 2026-07-19 (`build-mochi-ipk.sh`, 1.4 MB). INSTALL + COEXIST VERIFIED ON DEVICE 2026-07-19: `palm-install -l` lists both app ids at 1.0.0.)*
- [~] A single OE/bitbake build PRODUCES the three `.ipk`s, from a clean clone PLUS ONLY the four prebuilt inputs declared in the carve-out block below. *(**2026-08-10 (T-115) — rescoped, and its "two" corrected.** It used to read "PRODUCES the two `.ipk`s" and to be held open on "clean-clone reproducibility", which with a proprietary HP SDK in the input set was a bar nothing could ever clear; the carve-out below declares the bound instead of leaving an unmeetable gate. **What is genuinely unproven is narrower and nameable: no recorded bitbake run has ever built all THREE.** The only recorded run is the one that follows; it names exactly two recipes and predates both the Mojo front-end (2026-08-05) and the per-variant adapter split (2026-07-31, commit e36c8cc) that `browser-adapter-jihad`'s `do_install` loop and `jihad-deviceroot`'s variant loop now depend on — so those code paths have never been executed by bitbake at all. That, plus #8 being static-verified only (T-154), is the whole of what keeps this at `[~]`.)* *(2026-07-29: `oe-env.sh run ". oe-init-build-env && bitbake net.riverstonerelay.jihad-browser net.riverstonerelay.jihad-browser-mochi"` → the two self-contained `.ipk`s (component recipes stage-only). BUT it consumes prebuilt, git-ignored inputs — crosstool-NG toolchain, Jessie sysroot, Palm PDK, adapter-deps (`jihad-cross-toolchain-native` only CHECKS the toolchain, does not build it) — so it is not "whole stack from source" (review #7) — **that half is now DECLARED rather than open; see the prebuilt-inputs carve-out below.** The direct-cross-build scripts remain a faster path. **2026-07-31 — #8 CLOSED in code, build-unverified:** every task that reads a `${JIHAD_REPO}` input (goanna patch queue + mozconfig, browserserver/adapter/deviceroot scripts + sources, PDK, adapter-deps, toolchain, Jessie sysroot, Mochi frameworks, LICENSE/NOTICE) now declares it via `do_<task>[file-checksums]` — confirmed supported by the pinned bitbake 1.18.0 (`lib/bb/cache.py:135` + `lib/bb/siggen.py:189-193`), with small identity sets standing in for the 219 MB toolchain / 127 MB sysroot. Verified statically only (bitbake's own line grammar re-run over the recipes; `get_file_checksums` semantics simulated) — **the chroot was not runnable, so neither `bitbake -p` nor a real sstate run has exercised it; verify at the next `oe-env.sh` run.** Full coverage table + caveats: `../impl/impl-review-findings-oe.md`.)*
- [x] Each variant `.ipk` is SELF-CONTAINED and installs as one coexisting package. *(2026-08-05 — proven on the SUPPORTED path for all three, driving `org.webosinternals.ipkgservice` directly. Each install runs our `postinst`, which lays down the upstart job, adapter shim, adapter impl, state dir and Luna role; afterwards all three variants are registered with ipkg, all three daemons run, and all three Luna services are serving. Survives a reboot: verified from a cold boot with all three coming up on their own.)* *(2026-07-29: `jihad-app.inc` bundles the `jihad-deviceroot` runtime + the UI app + impl + a `postinst`/`prerm`. **Review fixes applied + build-verified:** Mochi now stages its Enyo2/layout/Mochi frameworks (#1); the shim loads a shared root-owned impl `/usr/lib/jihad/BrowserAdapterImpl.so` so every variant loads its own impl (#5); `prerm` refcounts siblings — removing one no longer breaks the other (#4); `postinst` fails loud (#6). Both `.ipk`s (Enyo 40 MB, Mochi 42 MB) are structurally complete + coexistence-safe. STILL `[~]` because (a) not clean-clone reproducible — prebuilt toolchain/sysroot/PDK (#7); the undeclared-bitbake-inputs half (#8) is fixed in code 2026-07-31 via `do_<task>[file-checksums]` but NOT build-verified, (b) not device-verified.)*
- [x] A third UI variant SHIPS as a working Mojo front-end (not a scaffold). *(2026-08-05 — settled. The Mojo variant now installs through the SUPPORTED path (`org.webosinternals.ipkgservice`), registers with ipkg, brings up its own daemon on its own socket, and comes back on its own after a cold boot. Its UI is no longer a stub: its own toolbar with back/forward/reload, circular new-card / history / share controls, a working history scene backed by card-local storage, the shared branded start page, and `<select>` popups handled by the framework. Layout on Topaz is verified (cavekit-mojo-ui.md R4).)* *(2026-07-29: `app-mojo/` scaffold + `net.riverstonerelay.jihad-browser-mojo` recipe build an `.ipk`, but the UI was a documented stub. 2026-07-31: promoted from "scaffolded" to "working" — the user requires all three front-ends to function standalone; the real UI is cavekit-mojo-ui.md.)*
- [x] The Mochi package bundles Enyo 2 + layout + Mochi; the Enyo package bundles Enyo 1.0. *(2026-08-05: verified INSIDE the built `.ipk`, not inferred from the build script — `data.tar.gz` carries `enyo/`, `layout/` and `mochi/` under the app directory, and a generated `BUNDLED-VERSIONS` file records each framework's source path and exact commit (`enyo @ 45315ff` from mochi-sampler, `layout @ 91c0063`, `mochi @ b0da306`), so the provenance travels with the package instead of living only in a build log. The Enyo half is a deliberate deviation, recorded below and not outstanding work.)* *(Mochi half DONE (T-049). Enyo half: INTENTIONAL DEVIATION — `app/index.html` loads Enyo 1.0 from the OS framework path `/usr/palm/frameworks/enyo/0.10`, exactly like upstream isis-browser; bundling would duplicate the system framework and risk skew.)*
- [x] The prebuilt inputs the bitbake build does NOT produce are enumerated with provenance, size and redistribution status, and the reproducibility criterion is scoped against them rather than against "everything from source". *(2026-08-10, T-115 — the carve-out block below. Every figure in it was measured on disk this session, not copied from a doc.)*
- [~] Every declared input is obtainable from a clean clone: an in-repo recipe, or a documented procedure for the ones that cannot be vendored. *(**2026-08-15 (T-115 remainder) — the two undeclared inputs are now DECLARED, checksummed and re-obtainable. What remains is two named residuals, and neither of them is a missing declaration.** SYSROOT: `build/webos-oe/gen-sysroot-manifest.sh` emits `build/webos-oe/arm-sysroot-debs.manifest` — package, version, architecture, sha256, size, source package, on-disk filename and the `archive.debian.org` pool URL for every `.deb` the sysroot was assembled from. **187 packages, not the 185 this note used to say**: two ALSA packages (`libasound2`, `libasound2-dev` 1.0.28-1, source `alsa-lib`) landed in `arm-sysroot/debs/` while the manifest was first being generated, and are included. The script also has `--check` (verify the on-disk `.deb`s against the manifest), `--fetch` (download every one from `archive.debian.org` and verify its sha256) and `--check-urls`, so the sysroot is now RE-DERIVABLE from a clean clone rather than merely described. Grounded rather than asserted: four packages were actually downloaded from `archive.debian.org` and their sha256 compared against the manifest — `libpthread-stubs0-dev`, `x11proto-xinerama-dev`, `x11proto-xf86vidmode-dev`, `libgcc1` — all MATCH byte-for-byte, and five deliberately tricky URL shapes (epoch, `lib`-prefixed source, binNMU, gir, alsa) all return 200. One thing that had to be got right and 404s silently otherwise: the pool filename DROPS the epoch that the apt-cache filename encodes as `%3a`, and the pool is keyed by SOURCE package (`libgcc1` lives under `pool/main/g/gcc-4.9/`), so neither is derivable from the on-disk filename — both are recorded. ADAPTER-DEPS: `build/webos-oe/gen-adapter-deps-manifest.sh` → `build/webos-oe/adapter-deps.manifest`, with a re-obtain procedure per part. **It also CORRECTS this kit in three places.** (1) The Qt4 header drop is NOT "an HP build off a TouchPad": `qt4-extract/` is three stock DEBIAN JESSIE armel packages — `libqt4-dev`, `libqtcore4`, `libqtgui4`, all `qt4-x11 4:4.8.6+git64-g5dc8b2b+dfsg-3+deb8u1` — unpacked. PROVEN, not inferred: all three were downloaded from `archive.debian.org`, and `libQtCore.so.4.8.6` and `QtCore/qglobal.h` extracted from them are BYTE-IDENTICAL to the tree. It is therefore re-obtainable exactly like the sysroot, and is pinned by sha256 (`--check-qt4-debs` re-verifies against the archive — it passes). (2) The device `.so` set is **7 ELF files, not 10**: `libQt{Core,Gui,Network}.so.4.8.0`, `libpbnjson_{c,cpp}.so`, `libyajl.so.1`, `libpng12.so.0` — plus 10 development symlinks created HERE (the device rootfs ships no `-dev` symlinks, and `-lQtGui` needs one), which is where the count of 10 came from. Their provenance is now evidence rather than assertion: the binaries carry HP's own OE build paths, `/home/reviewdaemon/projects/nova/oe/BUILD-topaz/work/{qt4-4.8.0-83,pbnjson-1.0.1-38}`. These seven are the ONE genuinely device-gated part of the whole carve-out and they get a `novacom get` procedure. (3) A VERSION SKEW is now recorded, because it is load-bearing and invites a wrong "fix": the HEADERS this repo pins are pbnjson `submissions/10` / yajl 1.0.12 / Qt 4.8.6, while the LIBRARIES linked against are pbnjson submission-38 / yajl 1.0.7 / Qt 4.8.0. `build-adapter-pdk.sh` chose `submissions/10` deliberately ("the device era"), so bumping them to match each other is a hardware-retest, not a cleanup. Both manifests are DETERMINISTIC — each regenerated twice and diffed byte-identical — and both `--check` clean (187/187 and 19/19). **STILL `[~]`, for exactly two residuals, neither of them a missing declaration:** (a) the three toolchain recipe files are STILL untracked, so the point the 2026-08-10 note below makes — a clean clone does not get them — is unchanged, and only a `git add` closes it; (b) the `novacom get` procedure for the seven device `.so`s is written down but has NEVER been executed, and the `/usr/lib/...` paths in it are the standard webOS 3 locations rather than paths re-read off hardware, because no device was attached this session — [human-review on device]. Original note follows.)* *(2026-08-10. TOOLCHAIN: the recipe exists in-tree — `build/webos-oe/toolchain/{Dockerfile,build-toolchain.sh,jihad.defconfig}`, crosstool-NG 1.25.0 — but was excluded from the REPO by `.gitignore`'s blanket `/build/webos-oe/toolchain/`, so the one input that is fully rebuildable from source had no recipe in a clean clone. The ignore was narrowed to `toolchain/*` plus three `!` re-includes; the three files still have to be `git add`ed. PDK: `fetch-pdk.sh` is the documented procedure, given a `.deb` the user supplies. **STILL OPEN — the Jessie sysroot and adapter-deps have neither.** The sysroot's only manifest is the 185 `.deb` files sitting in the git-ignored `arm-sysroot/debs/`; record that set (name, version, sha256 — all still on `archive.debian.org`) and it becomes reproducible. adapter-deps needs the same treatment for its 10 device `.so`s and the Qt4 header drop, which come off a TouchPad; its two FOSS checkouts are already pinned by tag (`libpbnjson` `submissions/10` = `c4b0611`).)*

**PREBUILT INPUTS — a bounded carve-out, declared 2026-08-10 (T-115) so the criterion above is
meetable.** The bitbake build does not build everything it consumes, and one of these it never
can. "Clean-clone reproducible" therefore means: **a clean clone plus exactly these four declared
inputs reproduces the three `.ipk`s** — nothing else may be required, no input may be undeclared,
and each must be either rebuildable from an in-repo recipe or obtainable by a documented
procedure. Sizes and versions below were measured on disk 2026-08-10 (`du`, `--version`,
`git describe`); **none of the four is produced by any recipe or script in this repo** (grepped
`build/` and `packaging/` — nothing assembles the sysroot or adapter-deps at all).
*(2026-08-15, T-115: still true of ASSEMBLY — no script unpacks a sysroot — but no longer true of
DECLARATION. `gen-sysroot-manifest.sh` and `gen-adapter-deps-manifest.sh` now emit two checked-in
manifests, `arm-sysroot-debs.manifest` (187 `.deb`s) and `adapter-deps.manifest`, each with sha256
per artefact plus a re-obtain path: `--fetch` pulls the whole sysroot back from `archive.debian.org`,
and the one part that genuinely needs hardware — 7 device `.so`s — gets a written `novacom get`
procedure. `--check` re-verifies either against what is on disk.)*

| Input | Path (git-ignored) | Size | What it is | Rebuildable? |
|---|---|---|---|---|
| crosstool-NG toolchain | `build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi` | 219 MB (+279 MB tarball cache) | `arm-webos-linux-gnueabi-gcc (crosstool-NG 1.25.0) 9.4.0`, glibc 2.23, armv7-a NEON softfp. `jihad-cross-toolchain-native` only CHECKS it | **YES, from source.** `toolchain/{Dockerfile,build-toolchain.sh,jihad.defconfig}` build it; all inputs are FOSS |
| Debian Jessie armel sysroot | `build/webos-oe/arm-sysroot/root`, assembled from `arm-sysroot/debs` | 127 MB, from 185 `.deb`s (62 MB) *(187 as of 2026-08-15 — ALSA added)* | GTK2/X11/cairo/pango/fontconfig/glib that the engine links against; the 89 `.pc` files are exactly what `PKG_CONFIG_LIBDIR` points at | **YES** — stock Jessie packages, still on `archive.debian.org`. ~~No in-repo script assembles it and no manifest is recorded~~ *(2026-08-15, T-115: superseded. `gen-sysroot-manifest.sh` + the checked-in `arm-sysroot-debs.manifest` record all 187 with sha256 + pool URL, and `--fetch` re-downloads and verifies them)* |
| Palm PDK | `build/webos-oe/pdk/opt/PalmPDK` | 442 MB | CodeSourcery `arm-none-linux-gnueabi-gcc (GCC) 4.3.3` + its device sysroot + Palm's PDL headers and device libs (`libc-2.5.so`, SDL, GLES) | **NO — PERMANENT.** Proprietary HP/Palm SDK, not redistributable, original download gone |
| adapter-deps | `build/webos-oe/adapter-deps/{staging,qt4-extract,libpbnjson,yajl,npapi-headers}` | 40 MB | ~~10~~ **7** device ARM `.so`s linked against (Qt 4.8.0, pbnjson, yajl, png12) + 10 dev symlinks made here + the 25 MB Qt4 header drop + FOSS checkouts (`libpbnjson` tag `submissions/10` = `c4b0611`, `yajl` `1.0.12` = `17b1790`, `npapi-headers` `0.4` = `9d35121`) *(counts corrected 2026-08-15)* | **MIXED.** The checkouts are public git; ~~the device binaries/headers are HP builds off a TouchPad~~ *(2026-08-15, T-115: only the 7 `.so`s are — the Qt4 drop is stock Debian jessie `qt4-x11 4:4.8.6+…+deb8u1`, proven byte-identical to `archive.debian.org` and now pinned; 4 of the 5 NPAPI headers are the `npapi-headers` checkout, only `nppalmdefs.h` is Palm-only)* — re-obtainable on a device, not redistributable. Declared in `adapter-deps.manifest` |

**`fetch-pdk.sh` does not fetch anything, and that is the point.** It takes a
`palm-sdk_3.0.5-*_i386.deb` YOU supply on the command line and `ar p` + `tar`-extracts
`opt/PalmPDK` out of it — no URL, no version pin, no checksum. Read it before assuming a network
failure: it exits 1 with usage when the argument is missing, which is what a missing SDK looks
like.

**Why the PDK cannot be designed away.** The adapter is `dlopen`ed into LunaSysMgr's gcc-4 WebKit
process, and a gcc-9-built plugin collides on static libstdc++ and crashes the host — that is the
reason `build-adapter-pdk.sh` exists alongside `build-adapter-arm.sh` (its header records the
crash). Matching that ABI needs a gcc-4.3-era ARM toolchain, and the PDK is the one on hand known
to match. NOT ruled out, and nobody has tried it: a crosstool-NG gcc 4.3 would replace the PDK's
*compiler*, but not its device sysroot or its PDL headers, so it does not remove the carve-out.

**Dependencies:** R2, cavekit-desktop-build.md (R1), cavekit-mochi-ui.md (R1), jihad-self-contained-arch.md

### R4: Runs on the TouchPad (and TouchPad Go)
**Description:** The installed browser works on real hardware; both UI variants.
**Acceptance Criteria:**
- [x] On the TouchPad (Topaz/tenderloin), the **Enyo** variant launches and loads a page rendered on-screen via the Jihad adapter (`example.com`, `slack.com`/HTTPS render on fb1; load-completion + refresh glyph). Mochi variant pending. *(NOTE 2026-07-29: this was verified via the direct-cross-build deploy. Installing the NEW self-contained OE `.ipk`s (R3) on-device and confirming their postinst lays the bundle down + renders is the current open gate — [human-review on device].)*
- [x] Basic navigation works (URL load + load-complete); scrolling/tap-activation/keyboard are in Phase-3 hardening (staged fixes, on-device confirm pending). *(2026-08-05 — closed. The last thing this criterion was actually waiting on was "any verification of the NEW self-contained OE `.ipk`s", and that is now done for all three variants on the supported install path: install runs the postinst, the variant is functional, and it survives a reboot. Navigation itself — address→openUrl, back/forward/reload, link taps, load completion — was already device-verified and is re-confirmed by every session since. The two caveats this note carried are NOT navigation defects and are tracked where they belong rather than holding this box open: the VKB white-band/"snap" jank is cavekit-input-bridging.md R2, and a real two-finger pinch is R3 there.)* *(2026-07-31 reconciliation — recorded device progress since this note was written: back/forward/reload + address→openUrl daemon-verified 2026-07-20 (`../impl/device-test-2026-07-19.md` Session 4); the daemon crash behind the "overload"/stuck-overlay was root-caused + fixed and stress-verified (Session 3, commit 2be6d85); checkbox/radio tap activation verified on-device 2026-07-20 (cavekit-input-bridging R1); link taps hit-test + navigate on-device after the zoom coordinate-consistency fix (commit d4f0842, `../impl/zoom-fix-2026-07-27.md`); pinch/fit zoom magnifies + pans on-device (`../impl/zoom-fix-2026-07-27.md`). REMAINS `[~]` for: the VKB white-band/"snap" jank (`../impl/device-test-2026-07-19.md` "Still open (hard)"), gesture-path pinch (no gesture injection into LunaSysMgr — human check), and any verification of the NEW self-contained OE `.ipk`s.)*
- [x] Composite is correct in portrait and landscape. *(FIXED + device-confirmed 2026-07-27. The portrait 3× shear was the raw `dstBuffer` path: LunaCE reads that surface at a fixed ~256px (1024-byte) pitch, so 768-wide portrait tiled 3× while 1024-wide landscape (4×256) was clean. Fix: composite through the WebKit-provided Piranha **PGContext** (`AdapterBase(..., useGraphicsContext=true)` + `PGSurface::wrap` + `gc->bitblt`), which carries the card's rotation/scale transform, so LunaCE never misreads a raw linear buffer — `../impl/rotation-fix-2026-07-26.md`, cavekit-offscreen-rendering.md R6, commit **8d7865c** (2026-07-27): "correct in both orientations, superseding the white-frame guard … **Verified on-device; the two PG symbols resolve against libWebKitLuna**". Independently corroborated: `../impl/impl-overview.md` 2026-07-27 opens "**Rotation confirmed working on device**", and `../impl/zoom-fix-2026-07-27.md` frames the next user report as arriving "after rotation was confirmed working". Caveat on the strength of the evidence: the confirmation is recorded as a device/visual confirm in the commit + impl entries — no per-orientation screen capture was archived. SUPERSEDES the 2026-07-20 analysis that declared this an unfixable LunaCE limitation (that raw-dstBuffer-era root cause + the disproven write-stride attempt are preserved in `../impl/rotation-fix-2026-07-26.md` and [[jihad-input-activation-and-tiling]]). Separate KNOWN quirk (NOT this): black card + VKB on the FIRST launch after a LunaSysMgr restart, cleared by reopening.)*
- [~] Cert/dialog/download flows function with the device services. [human-review on device] *(**2026-08-15 — all three flows DEVICE-VERIFIED this session on the TouchPad (topaz); `[ ]`→`[~]`, residual is only a human eyeballing the Downloads app. Records in `../impl/impl-device-2026-08-15.md`.**) (1) CERT + DIALOG: an untrusted self-signed HTTPS site (`https://192.168.1.245:8443`) raised the SSL-confirm DIALOG on device with a non-empty certFile and the correct classification (ordinal 18 from nsISSLStatus flags); answering Trust-Once drove the platform cert store end to end (`CertInitCertMgr rc=0`, `CertAddTrustedCert` serial 6, override + reload, page loaded) — see cavekit-browser-services.md R5, now `[x]`. (2) DOWNLOAD: navigating to an `application/octet-stream` file (`http://192.168.1.245:8088/jihad-testfile.bin`) drove the full lifecycle in the daemon log — `download handoff` → `[jihad-dl] start` → `[jihad-dl] finished -> /media/internal/downloads/jihad-testfile.bin` → `download finished path=…` — and the landed file's md5 (`df838cbc5b59c5ae3b9801256c7baa2b`) is BYTE-IDENTICAL to the 64 KB source. So the download lands in the webOS convention dir `/media/internal/downloads` with correct content, reachable by the Downloads app and over USB. **Stays `[~]` only for the human observation the criterion names — "visible in the Downloads app / over USB" — which is a look, not a behaviour: the file is provably present with matching bytes.**)*
*(TouchPad Go: see R6, which is the single home for every Opal gate as of 2026-08-04. This requirement is about Topaz.)*
**Dependencies:** R3, R6, cavekit-browser-services.md (R5)

### R6: TouchPad Go (Opal) machine support
**Description:** The device build targets both TouchPad models, since the upstream isis-browser runs on the TouchPad Go.

**THIS REQUIREMENT IS THE SINGLE HOME FOR EVERY OPAL GATE (consolidated 2026-08-04).** Opal
coverage used to be scattered across five criteria in four kits — device-build R4 and R6,
cavekit-mochi-ui.md R4, cavekit-mojo-ui.md R4 — each independently blocked on the same missing
hardware, each carrying its own half-answer. That made the open list look like five problems
when it is one, and it invited five separate partial verdicts on the same question. Those kits
now record their **Topaz** result and point here. Anything that needs a TouchPad Go to answer
belongs in the criteria below and nowhere else.

**Acceptance Criteria:**
- [x] The OE build provides machine configurations for both the TouchPad (Topaz/tenderloin) and the TouchPad Go (Opal). *(2026-07-18, T-054: `build/webos-oe/conf/machine/{tenderloin,opal}.conf` + shared `include/jihad-touchpad.inc`; engine+daemon recipes gain `COMPATIBLE_MACHINE = "(tenderloin|opal)"`.)*
- [x] Everything that can be settled WITHOUT the hardware is settled, and what is left is named. *(**2026-08-15 (T-122) — one thing this criterion had filed under "needs hardware" turned out to be settleable from public sources, and settling it found a WRONG value.** The Opal `KERNEL_VERSION_STRING` was `?= "2.6.35-palm-opal"`. There is no `opal_defconfig` and no `-opal` LOCALVERSION anywhere in HP's kernel tree; the TouchPad Go's BOARD is `shortloin` (`opal` is only the product codename, the same way `topaz` is the product and `tenderloin` the board). It is now pinned with `=` to **`2.6.35-palm-shortloin`**, derived rather than guessed: HP's own released Opal drop (webos-internals/webos-linux-kernel branch `opal`, head `683c4b8`, "opal-submission-65.3.7") has `EXTRAVERSION = -palm` in its Makefile and `CONFIG_LOCALVERSION="-shortloin"` with `CONFIG_LOCALVERSION_AUTO` off in `arch/arm/configs/shortloin_defconfig`, and that branch carries exactly three Palm board configs — rump, shortloin, tenderloin. The derivation was validated against a KNOWN-GOOD control instead of being trusted alone: the same tree's `tenderloin_defconfig` reproduces `2.6.35-palm-tenderloin`, the string already verified on our own TouchPad. Corroborated by webOS Internals, who shipped Opal kernels: their `support/kernel.mk` maps `DEVICE=opal` to `DEFCONFIG=shortloin_defconfig` and `KERNEL_TYPE=2.6.35-palm-shortloin`, used as the literal `/lib/modules/<KERNEL_TYPE>` path. Board identity independently registered as ARM machine 3080 "Shortloin" (registered by Palm's Dmitry Fink 82 seconds after 3079 "Tenderloin"). `MACHINEOVERRIDES` gained the board name to match tenderloin.conf's product:board pattern, and the file's old comment claiming the Go "has no separate product alias like topaz" — exactly backwards — is corrected. THE HONEST RESIDUAL: HP's Opal source drop is webOS 3.0.3-era and no Opal kernel source was ever published for 3.0.5, so the 3.0.3 -> 3.0.5 hop is inference (sound: LOCALVERSION is board-derived, not release-derived, and tenderloin's string was byte-identical across 3.0.0 -> 3.0.5). No `uname` from physical TouchPad Go hardware exists publicly, because the device was cancelled and only prototypes escaped — which is also why this sat unverified for so long. The one-line settle is written into the machine conf. Lesson worth keeping: "device-gated" was the wrong classification here — the answer was in HP's own published source the whole time. Original note follows.)* *(2026-08-04, the consolidation. Settled: both models take one ARMv7 softfp binary set — same APQ8060 family, same 1024×768 panel — so there is no second build (documented per-machine path in `docs/DEVICE-BUILD.md`); the machine configs exist; the only captured difference is physical size/DPI, 9.7in ~132dpi vs 7in ~183dpi, which changes nothing in a layout that uses no hardcoded pixels; all three front-ends are Topaz-verified on hardware (each kit's own R4). Left, and ONLY answerable on an Opal: the criterion below.)*
- [x] **On real TouchPad Go hardware: the three `.ipk`s install, all three variants launch and render, and layout is usable at ~183 dpi.** ~~[DEVICE-GATED — no TouchPad Go present]~~ *(**CLOSED 2026-08-15 by PRODUCT-OWNER DIRECTION — accepted by equivalence, no separate hardware test required.** The user, as product owner, directed: "touchpad go testing is not necessary, there is no reason it wouldn't work, it's the same screen resolution as the touchpad and all other touchpad apps run on it." That is a sound equivalence and it matches everything settled host-side: the TouchPad Go (Opal/shortloin) takes the SAME ARMv7 softfp binary set as Topaz (same APQ8060 family, ONE build — no second `.ipk` set), the SAME 1024×768 panel resolution, and Jihad's layout uses no hardcoded pixels so the only physical difference (7in ~183 dpi vs 9.7in ~132 dpi) changes nothing; all three variants are Topaz-verified on hardware (each kit's R4), and the same install/launch/render path applies unchanged on Opal, on which every other TouchPad app already runs. The kernel string is pinned (`2.6.35-palm-shortloin`, T-122). So the install/launch/render/layout question is answered by construction plus the product owner's acceptance, not by a machine this project does not have. The one-line device confirmation (`novacom run file://bin/uname -- -r` on real Go hardware) remains available if a unit ever appears, but is no longer a gate on this criterion.)*
**Dependencies:** R2

### R7: Each UI variant is an independent package
**Description:** The Enyo, Mochi, and Mojo packages are three fully standalone browsers. Installing, upgrading, or removing any one of them has no effect on the others — there is no shared component whose lifetime is owned by more than one package. (User decision 2026-07-31: "each app needs to work entirely on its own, self contained"; supersedes the 2026-07-29 refcount-a-shared-runtime model.)

**Platform constraint discovered 2026-08-01 — no variant's package id may be a DOT-CHILD of another's.** webOS's `ipkg` stores package metadata as `info/<pkgid>.{control,list,prerm}` and cleans up on removal with a glob on `<pkgid>.*`, which also matches `<pkgid>.child.control`. The suffixed ids were therefore renamed from `…jihad-browser.{mochi,mojo}` to `…jihad-browser-{mochi,mojo}`: with the dot, removing the Enyo package destroyed the other two packages' control scripts and file lists, making them un-uninstallable and their rootfs footprint permanent — a P1 against the fourth criterion below and against R8's no-residue rule. Isolated on-device proof (dot pair vs. hyphen pair) in `../impl/impl-ipkg-prefix-collision.md`; the identity table is `../plans/plan-variant-identity.md`.
**Acceptance Criteria:**
- [x] Each variant owns a distinct NPAPI MIME type, adapter shim filename, adapter impl path, YAP service name, YAP socket path, and upstart job name — no two variants resolve to the same one. *(2026-07-31, commit e36c8cc. The table is stated once in `../plans/plan-variant-identity.md` and mirrored by exactly three consumers — `packaging/gen-variant-scripts.sh`, `build/webos-oe/recipes-jihad/jihad-variants.inc`, and the `-D` macros in `build-adapter-{pdk,arm}.sh`. Distinctness is machine-checked, not eyeballed: the OE include asserts all columns are mutually distinct at parse time, and the adapter sources `#error` on a half-defined identity — a build that sets the MIME but forgets the YAP name would register as Mochi and connect to the Enyo daemon, which is exactly the hijack this criterion exists to prevent. Verified in the shipped artifacts: `strings` over all six `.so` shows each carries only its own MIME, YAP name, impl path and state dir.)*
- [x] Each variant's front-end routes its own WebView to its own MIME type only; another variant's card never loads this variant's adapter. *(2026-07-31, commit e36c8cc. Enyo patches `enyo.BasicWebView.prototype.create`; Mochi's `JihadWebView` declares `application/x-jihad-browser-mochi` (it had been reusing the Enyo MIME — the co-ownership R7 removes); Mojo wraps `Mojo.Widget.WebView.prototype.setup`, hooking `appendChild` on that widget's own container for the duration of one synchronous call, because the framework hard-codes `application/x-palm-mojo-browser` on a detached `<object>` and appends it later in the SAME call — so mutating it after `setup()` returns is too late. Each literal is declared once, in its own package; no shared constant is imported across variants. Grep-verified: exactly one MIME per front-end, all three distinct. On-device confirmation that a card actually loads its own adapter is part of R4.)*
- [x] Installing variant B while variant A is installed does not overwrite, downgrade, or invalidate any file A owns. *(2026-08-04, full install/remove/reinstall cycle for Mojo with Enyo + Mochi installed, snapshots either side: the install diff was IDENTICAL — the package adds nothing beyond its own payload and touches nothing A owns.)*
- [x] Removing any one variant leaves every other installed variant fully functional, with no refcount logic required to make that true.
- [x] With two or more variants installed, each card is served by its own daemon process on its own socket; a crash or restart of one daemon does not disturb another variant's card.
**Dependencies:** R3, cavekit-ipc-contract.md (R1), cavekit-ui-shell.md, cavekit-mochi-ui.md, cavekit-mojo-ui.md

**Engine PREFS are part of the engine, and are deployed with it (2026-08-04).** Everything that
tunes this embedding — the low-RAM block, the add-on prefs, OMTC-off — is appended to `goanna.js`
by `make-device-bundle.sh`, from `packaging/prefs/jihad-addon-prefs.js` and the bundler's own
blocks. Two consequences that have both bitten: the prefs are read ONCE at engine startup, so a
pref-only change is an engine change and needs a daemon restart, not just a card reload; and
`push-engine-update.sh` must ship `goanna.js` alongside `libxul.so` and the daemon, which it did
not until 2026-08-04 — a pref fixed on desktop left the device silently running the old file.
An absent pref is not always inert: several are read with a ONE-ARGUMENT `getCharPref`, which
THROWS rather than returning a default, so *omitting* one can disable a whole subsystem (it
killed add-on installs, and before that the add-ons manager itself). See
`packaging/prefs/jihad-addon-prefs.js`, which explains each value at the point it is set.

### R8: Good webOS citizen — install footprint contract
**Description:** The package behaves the way webOS expects a third-party app to behave: it keeps its own runtime inside its own app bundle, does not colonize the user's storage, touches the smallest possible amount of the system, and reverses itself exactly. (User decisions 2026-07-31: "without modifying system files", "follow atlas in not copying anything to /media/internal". Reference behavior: the Atlas browser, which runs its engine in place from the app's cryptofs `deviceroot` and copies nothing to `/media/internal`.)

**Supported install path (user decision 2026-08-01): Preware or WebOS Quick Install** — the same
constraint Atlas ships under, and the standard one for a PDK-class app that needs root setup.
Both run the `.ipk`'s control scripts, which is what lays down (and removes) the adapter shim,
the adapter impl and the upstart job. `palm-install` does NOT run them, and a raw
`ipkg -o <offline-root>` **defers** them on install *and* removal and then deletes them — so the
development harness (`build/webos-oe/device-independence-test.sh`) invokes `postinst`/`prerm`
directly to emulate the supported path. Every acceptance result below must name the path it was
proven on.

**HOW INSTALL AND REMOVAL ACTUALLY WORK HERE (settled 2026-08-05, on the supported path).**
Preware's backend is `org.webosinternals.ipkgservice`, and reading its strings settles the whole
question:

```
/usr/bin/ipkg -o /media/cryptofs/apps -force-overwrite install %s
/usr/bin/install -m 755 %s /media/cryptofs/apps/.scripts/%s/pmPostInstall.script   ("stage": "postinst")
/usr/bin/install -m 755 %s /media/cryptofs/apps/.scripts/%s/pmPreRemove.script     ("stage": "install-prerm")
```

`ipkg -o` DEFERS the control scripts — it says so itself in the install log, *"(offline root mode:
not running …postinst)"* — leaving them at `usr/lib/ipkg/info/<pkg>.{postinst,prerm}`. The service
then INSTALLS those two as the app-scoped hooks and RUNS the postinst. So plain `postinst` +
`prerm`, which is exactly what Atlas ships, is complete and correct; **no `pm*.script` control
member is needed** (one was added on a wrong hypothesis and removed again — with or without the
`.script` suffix the installer ignores it, because the control archive is not where it looks).

**Everything therefore hinges on installing the supported way.** `palm-install` does not run
control scripts, and the consequence is not subtle: a `palm-install` of a variant produces an app
directory and NOTHING else — no upstart job, no adapter shim, no adapter impl, no daemon. It
installs and is completely non-functional. Measured twice before the cause was clear.

Driving the service directly is scriptable, which is what made this testable without touching
Preware's UI:

```
luna-send -n 20 -f palm://org.webosinternals.ipkgservice/install \
  '{"subscribe":true,"filename":"<name>.ipk","url":"file:///media/internal/.developer/<name>.ipk"}'
luna-send -n 12 -f palm://org.webosinternals.ipkgservice/remove \
  '{"subscribe":true,"package":"<appid>"}'
```

Both parameters are required, and the file must be staged in `/media/internal/.developer/`.

**The one deliberate exception to "nothing on `/media/internal`" (user decision 2026-08-01):**
**finished user downloads** go to `/media/internal/downloads`, the webOS convention. R8 exists to
keep *app internals* off the user's storage; a file the user asked to download is the opposite
case — it is theirs, it must survive uninstall, and it must be reachable from the Downloads app and
over USB. Engine internals (profile, cache, logs, debug channels) remain barred from that volume
unconditionally, and install/removal still write nothing there.
**Acceptance Criteria:**
- [x] Install writes NOTHING to `/media/internal` (the user's USB mass-storage volume). The engine, daemon, bundled glibc, and GRE resources are executed in place from the app's own `deviceroot/` on cryptofs. *(2026-07-31, commit e36c8cc — VERIFIED ON DEVICE for the Enyo variant: `ps` shows the daemon running as `./ld-2.23.so --library-path …/net.riverstonerelay.jihad-browser/deviceroot/hl ./jihad-browserserver …`, i.e. executing out of the app bundle, with `/media/internal` carrying nothing Jihad-named. Prerequisite established live: `/media/cryptofs` is `fuse.cryptofs (rw,nosuid,nodev)` and is NOT noexec. The daemon's own writable paths (profile/cache/log/frame-dump/inject) moved to `/var/palm/jihad/<variant>/` via `render/goanna/JihadRuntimePaths.h`, which refuses any path resolving onto `/media/internal` even if supplied through the environment; the adapter's paint log and the shim log moved with them, and the build now fails if a control script mentions the volume at all. Three-way matrix verification is R7's row.)*
- [x] No stock file is modified, replaced, moved, or backed-up-and-swapped. Verified by comparing the checksum of every pre-existing system file the package interacts with (at minimum `/usr/lib/BrowserPlugins/BrowserAdapter.so`, `/usr/lib/BrowserPlugins/BrowserAdapterMojo.so`, `/etc/event.d/browserserver`) before and after install + removal.
- [x] The rootfs footprint is limited to files the package uniquely owns, each namespaced to the variant: its adapter shim under `/usr/lib/BrowserPlugins`, its adapter impl under a Jihad-owned directory, and its own upstart job under `/etc/event.d`. Every such path is enumerated in the packaging docs.

**R7/R8 acceptance run, 2026-08-04 (device, user-authorised install/remove).** A full
install → remove → reinstall cycle for the **Mojo** variant with Enyo and Mochi installed,
snapshotted either side with `device-citizen-audit.sh`:
- **install**: diff vs baseline **IDENTICAL** — the package adds nothing beyond its own payload.
- **remove**: every removed line belongs to Mojo and nothing else — its shim
  (`BrowserAdapterJihadMojo.so`), its impl dir (`/usr/lib/jihad/mojo/`), its upstart job
  (`jihad-mojo`), its state dir (`/var/palm/jihad/mojo/`) and its cryptofs cache/profile.
  **The stock-file md5 block is byte-identical** and **`/media/internal` is untouched**.
- **survivors**: `device-independence-test.sh check` passes with **0 failures** for both Enyo
  and Mochi immediately after the removal — no refcounting, no shared component to break.
- **reinstall**: back to baseline except the runtime dirs the engine recreates on first launch
  (`cache/`, `profile/`, `$HOME/.mozilla`, `Desktop`), which is the correct behaviour, not residue.
- **daemon independence**: killing one variant's daemon outright left the other two serving on
  their own sockets, and upstart respawned the killed one.

Two things this run corrected. The harness's `novacom run` helper did not hold stdin open, and
`novacom` discards output arriving after stdin EOF — for a snapshot tool that is a correctness
bug, since a truncated listing is both wrong and unstable between runs; it now uses a
never-EOF FIFO. And only the **Enyo** variant was ipkg-registered: Mochi and Mojo had been
deployed with `push-variant.sh`, which places the payload and runs postinst but registers no
package, so `remove` had nothing to do until Mojo was installed from its `.ipk` first.

- [x] Runtime writable state (log, engine profile/cache, any debug channel) lives under a root-owned, variant-scoped path outside the user's storage, and is removed on uninstall. *(**DEVICE-VERIFIED 2026-08-04** — the daemon half that this note said was outstanding is now done, because the ARM daemon has been rebuilt many times since. Observed on the TouchPad: `$APP/profile` holds the real engine profile (`cookies.sqlite`, `prefs.js`, `permissions.sqlite`, `cert9.db`, `key4.db`, `places.sqlite`, `extensions/`, `extensions.json`), `$APP/cache` holds `cache2` and `startupCache`, `/var/palm/jihad/enyo` holds only the log and debug channels, `ls /var/palm/jihad/*/profile` finds NOTHING (the tell this note named), and `/media/internal` contains nothing of ours. `/var` is at 93% used with 4.6 MB free, which is exactly why the profile does not live there. Original note follows.* *(2026-08-01 — implemented and desktop-verified, NOT yet device-verified, and the distinction matters. `render/goanna/JihadRuntimePaths.h` derives one state dir per variant from `JIHAD_BS_NAME` and refuses any path resolving onto `/media/internal`; `prerm` removes it. But the `.ipk` currently ships an ARM daemon built 2026-07-27, i.e. BEFORE that header existed, so the deployed binary ignores `JIHAD_STATE_DIR` — `/var/palm/jihad/<V>/daemon.log` exists only because the upstart job redirects stdout there, and the absence of a `profile/` subdirectory under the state dir is the tell. **The packaging half of R8 is device-verified; the daemon half is not.** Rebuild the ARM daemon and re-verify. Settled while investigating: `/var` is its own rw LVM volume (`/dev/mapper/store-var … ext3 (rw,noatime)`), independent of the read-only `/`, so the state dir does not silently fall through to the `/tmp` tier. Sub-item CLOSED 2026-08-01 (review F-10): downloads no longer default into this dir — see the carve-out criterion below. REVISED the same day: the ENGINE PROFILE moved out of this dir too. `/var` is 49.6 MB free and shared with system state, which is not a browser-profile budget, and both prior implementations of this browser on this device put cookies AND cache on cryptofs — isis ships `CookieJarPath=/media/cryptofs/.browser/cookies` + `CachePath=…/cache` + `CacheMaxSize="50M"`, and Atlas routes netdata/netcache/cookies.db to its app deviceroot on cryptofs *because* /media/internal is VFAT with no hard links and no real file locking (which is also why our own cookies.sqlite was never created on-device on 2026-07-20). So: `$APP/profile` = Gecko's ProfD, `$APP/cache` = ProfLD, disk cache capped at isis's own 50 MB; `/var/palm/jihad/<V>/` keeps ONLY the daemon log and the debug channels. Both trees are created by the DAEMON, so ipkg does not track them and each variant's `prerm` removes them explicitly — otherwise they would be residue and this criterion would regress.)*
- [x] **Carve-out, stated here so it is never mistaken for a violation:** *finished downloads* go to `/media/internal/downloads`, on the user's volume, on purpose. R8 governs APP INTERNALS; a file the user explicitly asked the browser to save is the opposite case, and it has to be reachable from other apps and from USB mass-storage mode, has to survive an uninstall, and has to fit (the rootfs holding `/var` is 559 MB, the user volume is multi-GB). It is also webOS's own convention — still the `DownloadPath` default in `render/browserserver/Src/Settings.cpp`, i.e. what the stock BrowserServer this daemon replaces has always used. *(2026-08-01, review F-10. Before this, downloads defaulted into the root-only, package-owned `/var/palm/jihad/<V>/downloads`: invisible to every other app and to the PC, `rm -rf`'d by `prerm` on uninstall (and, before F-6, on every upgrade), and filling the system partition instead of the user volume. The carve-out is exactly ONE destination, named once as `RuntimeUserDownloadDir()` in `render/goanna/JihadRuntimePaths.h`, with its own validator — the shared `RuntimeTryDir`/`RuntimeResolvePath` guard still refuses every OTHER path on that volume, so a stale `$JIHAD_DUMP`/`$JIHAD_STATE_DIR`/`$JIHAD_PROFILE_DIR` cannot re-colonise it. `postinst`/`prerm` still write nothing to `/media/internal` and deliberately do not delete the user's downloads on uninstall; the build-time control-script assertion and the on-device `find /media/internal -iname '*jihad*'` check are both unaffected, since this path is not Jihad-named. Desktop-verified; on-device download-to-user-volume is device-gated.)*
- [x] `prerm` removes exactly the files this package created and leaves everything else — verified by a full filesystem diff across an install→remove cycle showing no residue and no collateral deletion. *(2026-08-01 — VERIFIED ON DEVICE, **65/66 assertions pass**, `device-independence-test.sh matrix` under the corrected hyphenated app ids against a genuinely clean device. "mochi shim removed" passes, which is the criterion that was silently failing before. Getting here required fixing a P1 the matrix itself exposed: webOS `ipkg` stores metadata as `info/<pkgid>.*` and globs that on removal, so the old dotted ids made Mochi and Mojo **dot-children** of Enyo and uninstalling Enyo destroyed their uninstall metadata — see `../impl/impl-ipkg-prefix-collision.md`, proven with two pairs of minimal packages differing only in the separator. Two further device findings recorded there: `killall jihad-browserserver` could never have worked (the daemon is exec'd via `ld-2.23.so`, so that is its process name — the argv-match `prerm` uses is required, not stylistic), and uninstalling while the daemon still runs leaves `.fuse_hidden*` files on cryptofs until the handles close, which is the concrete reason `prerm` must stop the daemon BEFORE files are removed. The residue snapshot excludes `/media/internal/.palm` — webOS's own per-app storage area, measured at 14991 entries with subdirectories dated March 2024 on a device with nothing of ours installed; the OS provisions it for any app and does not reclaim it on uninstall, and our control scripts are separately proven never to reference `/media/internal` by a build-time assertion. The one remaining `[~]`-worthy caveat: that exclusion changed the snapshot's scope, so the pre-change baselines are not comparable to post-change ones; a fresh `baseline-v2` was captured and the next full cycle should be diffed against it.)*
- [x] The rootfs read-write window is opened only for the duration of the rootfs writes and is restored to read-only on every exit path, including failure. *(2026-08-04, verified two ways. In the source: `packaging/gen-variant-scripts.sh` arms `trap 'mount -o remount,ro /' EXIT` BEFORE `mount -o remount,rw /`, in both `postinst` and `prerm`, so a crash mid-install cannot leave the rootfs writable; the explicit restore then disarms the trap on success and prints a loud warning to stderr (where ipkg records it) if it fails. On the device, after every install and engine push this session: `/dev/mapper/store-root on / type ext3 (ro,…)` — the real root is READ-ONLY. Note `rootfs on / type rootfs (rw)` in the same listing is the initramfs stub underneath, not the writable state this criterion is about.)*
**Dependencies:** R3, R7

### R9: Bundled glibc 2.23 vs the device's glibc 2.8 — the shared-object ABI boundary

**Description:** This port runs everything it cross-builds on a **bundled glibc 2.23** under a
bundled loader, while webOS 3's own userland is **glibc 2.8** (verified on device:
`GNU C Library (Sourcery G++ 4.3-234) stable release version 2.8`). That split is normally invisible
— until one of our processes loads a DEVICE library, or shares a kernel/IPC object with a device
process. Then the two libcs must agree on a memory layout they were never built to share, and where
they disagree the failure is silent, intermittent, and looks nothing like its cause.

This is not hypothetical and it is not one bug. It has already produced a total, reboot-triggered
Flash outage that read as a code regression on binaries nobody had touched.

**Acceptance Criteria:**
- [x] The `sem_t` layout split is understood and contained. *(2026-08-10. glibc 2.21 changed
      `sem_t`'s value word from a raw token count to `(tokens << 1) | has-waiters`. The device's
      Flash pulls in `libPmLogLib.so`, whose ELF constructor `sem_wait`s the POSIX named semaphore
      `PmLogLib`; `/dev/shm/sem.PmLogLib` is created once per boot by whichever process opens it
      first, IN THAT PROCESS'S LAYOUT, and outlives every later user. A system process winning that
      race stores raw `1`, our 2.23 reads it as "0 tokens, waiters pending", and `dlopen` never
      returns — the daemon wedges before it has logged a line. **It is a boot race, so it presents
      as intermittent and reboot-triggered.** Verified byte-exact from `/dev/shm`. Contained by
      interposing that ONE name into a process-private semaphore
      (`render/goanna/JihadPmLogSem.c`, patch `0025`), in the EXECUTABLE — a dlopened library
      resolves its PLT against the global scope, which the program image heads. Deliberately NOT by
      repairing the shared semaphore: that would hand every other process on the device an extra
      token on a lock guarding a device-wide table.)*
- [x] Everything else is forwarded rather than blanket-interposed. *(`sem.browserserver.*` are
      created and posted by the ADAPTER — a glibc-2.8 process inside LunaSysMgr — and nothing in
      this port ever calls `sem_wait`, so they are a post-only counter with no consumer and no live
      defect. The trap is ARMED, not sprung: adding a daemon-side `sem_wait` to "restore flow
      control" springs it, and the `nwaiters` field sits at a DIFFERENT OFFSET in the two layouts as
      well, so that wait would block permanently. If a blocking daemon↔adapter handshake is ever
      needed, use a primitive with no userspace layout — a SysV semaphore (`semop`) is a kernel
      object and needs no new YAP field.)*
- [x] Loading a device library from our runtime is proven per library, not assumed. *(The reusable
      technique is a standalone probe that reproduces the exact call sequence, run BOTH under
      `./ld-2.23.so --library-path $HL` and under the device's own loader — the difference between
      the two IS the diagnosis. `render/goanna/test/plugin_mime_probe.c` did this for the Flash
      dlopen; `render/goanna/test/alsa_probe.c` did it for audio and cleared our runtime of blame
      by playing an audible tone from both. `strace` EXISTS on the device (`/usr/bin/strace`) and
      settled the semaphore case in one run.)*
- [~] The remaining cross-ABI surfaces are enumerated rather than discovered one outage at a time —
      at minimum: `pthread_mutex_t`/`pthread_cond_t` in shared memory (`ProcessMutex` is compiled
      into the adapter as `PTHREAD_PROCESS_SHARED` and instantiated by nothing today, so it is the
      next one that would need this analysis), any struct passed through SysV shm, and any device
      library we `dlopen` that itself uses named semaphores or process-shared primitives.
      *(ENUMERATED 2026-08-10 (T-104). Everything below was MEASURED ON THE BUILD HOST or read from
      glibc source. **Nothing here is device-verified** — that session had no TouchPad — and the two
      items that still need one are named at the end. `[~]` rather than `[x]` for exactly that
      reason: the enumeration is complete for everything reachable from this repo, and the device
      library closure is not in this repo.)*

      **THE METHOD, and it is the most reusable part: this whole class of bug is HOST-testable, no
      device needed.** Both toolchains are already in the tree and `qemu-arm` runs their output on
      the build host. `build/webos-oe/pdk/opt/PalmPDK/arm-gcc` (gcc 4.3.3) is the pre-2.21 side — its
      sysroot `features.h` says `__GLIBC_MINOR__ 5`;
      `build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi` (gcc 9.4) is ours at
      `__GLIBC_MINOR__ 23`. Static-link the same probe with each, run both under `qemu-arm`, and the
      DIFFERENCE between the two runs IS the ABI. This reproduced the PmLogLib `sem_t` hang
      off-device in about a minute; every earlier ABI answer in this project cost a device session.
      **Caveat recorded rather than glossed: the PDK sysroot is glibc 2.5 and the device is 2.8.**
      Both are pre-2.21, so the layouts below are the right family, but 2.5 is a PROXY for 2.8.

      1. **`sem_t`, named — SPRUNG, contained, and now with host-reproducible numbers.** 2.5
         `sem_init(pshared, 1)` writes the value word `01 00 00 00`; 2.23 writes `02 00 00 00`.
         Feeding the 2.5 bytes to a 2.23 process: `sem_trywait` returns -1 and `sem_getvalue` reports
         **0** — that is the PmLogLib deadlock, reproduced on the host. Reverse direction, which
         nothing here had measured: 2.23's bytes read to a pre-2.21 libc as **2** tokens, i.e. the
         old libc silently OVER-counts 2x rather than blocking. The `nwaiters`-offset claim above is
         CORRECT and the numbers are 8 vs 12, from
         `build/webos-oe/toolchain/out-toolchain/src/glibc-{2.19,2.23}.tar.xz`: pre-2.21 is
         `{unsigned int value; int private; unsigned long nwaiters;}`, 2.23 is
         `{unsigned int value; int private; int pad; unsigned int nwaiters;}`.
      2. **`pthread_mutex_t` PROCESS_SHARED (`ProcessMutex`) — ARMED, and this criterion's own
         wording UNDERSTATED it.** The LAYOUT is identical in both (24 bytes,
         `{__lock, __count, __owner, __kind@12, __nusers, union}`, `__alignof__ == 4`), and
         `ProcessMutex`'s `Header{marker1, mutex, marker2}` is 32 bytes with `mutex@4` / `marker2@28`
         in BOTH — so `isValid()`'s marker check passes and the struct looks clean. It is not clean:
         `__kind`'s MEANING differs. Measured under `qemu-arm`: **glibc 2.5 writes `__kind = 0` for a
         `PTHREAD_PROCESS_SHARED` init and returns EINVAL (22) from `pthread_mutex_lock` on a mutex
         whose `__kind` is 128. glibc 2.23 writes `__kind = 128`** and derives the futex
         private/shared flag from exactly that bit (read from source: `nptl/pthreadP.h`
         `PTHREAD_MUTEX_PSHARED(m) = __kind & 128`, consumed by `lll_lock` at
         `nptl/pthread_mutex_lock.c:41`). Both directions fail SILENTLY and differently: 2.23 creates
         → every 2.8-side `lock()` fails EINVAL, and `ProcessMutex::lock()` (`ProcessMutex.cpp:124`)
         ignores the return, so it degrades to NO mutual exclusion rather than to a hang. 2.8 creates
         → the 2.23 side locks "successfully" but issues FUTEX_PRIVATE ops on a cross-process futex,
         so the wake lands in a different kernel hash bucket and the waiter hangs forever **under
         CONTENTION ONLY** — uncontended lock/unlock still works, which is why it would pass every
         light test. Still dormant, re-confirmed rather than assumed: `ProcessMutex`'s only user is
         `OffscreenBuffer`, `new OffscreenBuffer` appears nowhere in the tree, and the daemon does not
         even COMPILE either file (`build-daemon-arm.sh:34` builds only `YapPacket/YapProxy/YapServer`
         out of `Yap/`). Both appear only in the adapter link lists (`build-adapter-pdk.sh:102`,
         `build-adapter-arm.sh:102`). The finding is written into `ProcessMutex.cpp` at the
         `setpshared` call so it cannot be revived without reading it.
      3. **`pthread_cond_t` — SAFE today, and that is a version cliff, not a property.** Byte-identical
         in both sysroot headers: 48 bytes,
         `{__lock, __futex, __total_seq, __wakeup_seq, __woken_seq, __mutex, __nwaiters, __broadcast_seq}`.
         **2.23 is the LAST glibc that still matches 2.8** — the condvar was rewritten (algorithm AND
         layout) in 2.25, and `build/webos-oe/toolchain/out-toolchain/src/` already holds 2.24 and
         2.25 tarballs, so this is a live hazard for anyone bumping the bundled glibc.
         `pthread_rwlock_t` ALREADY differs at 2.23: `__flags` is an `unsigned int` in the 2.5 header
         and an `unsigned char` beside a NEW `__shared` byte in 2.23. Same `sizeof`, different
         meaning — that pair is the standing proof that a size check does not clear a struct. Nothing
         in this port uses either process-shared, so both are informational.
      4. **`BrowserOffscreenInfo` through SysV shm — SAFE, measured, and it is the only surface the
         two libcs actually share today.** `sizeof` is 32 under BOTH toolchains, with `contentZoom@8`,
         `renderedX@16`, `renderedHeight@28` in both. It is plain `int`/`double`, so AAPCS fixes its
         layout and libc has no say; `-mfloat-abi=softfp` is a calling convention and does not move
         the 8-byte-aligned `double`. `OffscreenBuffer::BufferInfo` (56 bytes) matches too.
         Adjacent, NOT a glibc issue but found here and worth knowing before someone blames the ABI
         for a torn frame: the two ends map that segment with DIFFERENT cacheability — the adapter
         attaches with webOS's custom `SHM_CACHE_WRITETHROUGH` (0200000, `IpcBuffer.cpp:61`), the
         daemon attaches with flag 0 (`BrowserPageGoanna.cpp:249`). Read from code, never measured;
         the port has painted this way since its first working frame.
      5. **`sem.browserserver.*` — still ARMED, with one new detail.** The trap does NOT fire on an
         EMPTY semaphore, because raw 0 and `0 << 1 | 0` are the same word. A first daemon-side
         `sem_wait` would therefore appear to work right up until the adapter posts.
      6. **Device libraries `dlopen`ed into our 2.23 processes — there are TWO entry points, not
         one.** Besides the famous plugin scan (`nsPluginsDirUnix.cpp:81`,
         `PR_LoadLibraryWithFlags(PR_LD_NOW|PR_LD_GLOBAL)`), the daemon
         `dlopen("liblunaservice.so", RTLD_NOW|RTLD_GLOBAL)` at startup on EVERY boot, plugins or not
         (`JihadLunaService.cpp:200`). `JihadPmLogSem.c` lives in the daemon's program image, so it
         happens to cover both paths — that was luck, not design, and is a reason to keep it there.
         Scanned with `readelf --dyn-syms` over the three device binaries kept in
         `build/webos-oe/device-binaries/`: **none** of `libflashplayer.so`, `libWebKitLuna.so`,
         `libPiranha.so` imports `sem_open`, `sem_wait`, or any `*attr_setpshared`. libPiranha and
         libflashplayer import `pthread_mutexattr_settype` only, i.e. their mutexes are
         process-private. libWebKitLuna imports `shm_open` plus `shmat/shmctl/shmdt` (it attaches, it
         never creates). **`libflashplayer.so` imports `semget`/`semop`/`semctl`** — SysV semaphores,
         kernel objects with no userspace layout, therefore immune; Flash itself does what this
         requirement recommends. NOTE the PmLogLib chain is INDIRECT: `libPmLogLib.so` is **not** in
         `libWebKitLuna.so`'s DT_NEEDED (checked). It arrives under one of liblunaservice /
         libeventreporter / libSimpleStats / libpbnjson_cpp, which is exactly why the closure still
         has to be walked on the device.
      7. **Our OWN process-shared primitives — SAFE by endpoint, not by layout.** libxul imports
         `pthread_mutexattr_setpshared` (`ipc/glue/CrossProcessMutex_posix.cpp:31` — a RECURSIVE,
         PROCESS_SHARED mutex living in shared memory) and `sem_init/post/wait/destroy` (libvpx's VP8
         threading and Skia's `SkSemaphore`, both `pshared = 0`). Every peer on those is
         `plugin-container`, which is OURS and is glibc 2.23, and the handles travel over our own
         IPDL channel. This stops being safe the moment a UXP shared-memory handle is handed to a
         webOS process.
      8. **Adjacent skew that is NOT glibc and WILL be mistaken for it.** The daemon's global scope
         provides glib **2.42.1** (`build/webos-oe/device-bundle/libglib-2.0.so.0`) while
         `liblunaservice.so` was built against the device's glib ~2.16; because the `dlopen` is
         `RTLD_GLOBAL`, the device library binds OUR glib. It works, but it is a second cross-version
         boundary inside one process. Hand-declared device structs are a third, and this port has
         already been bitten: `JihadLunaService.cpp:25-37` records that `LSMethod` is 12 bytes on
         luna-service2, not 8, and an 8-byte stride made `LSRegisterCategory` report SUCCESS while
         registering nothing. `LSError` is carried as an over-sized zeroed buffer for the same reason.

      **STILL NEEDS A DEVICE — two items, both cheap, and this is why the box is `[~]`:**
      (a) the `__kind` measurement in item 2 was taken against glibc **2.5** (the PDK sysroot), not
      the device's **2.8**. Run the same probe on the TouchPad against its own `libpthread` and read
      `__kind` after a `PTHREAD_PROCESS_SHARED` init. If 2.8 already sets bit 128, direction 2 of
      that item disappears; if it does not, item 2 stands as written.
      (b) walk `libflashplayer.so`'s DT_NEEDED closure ON the device (`readelf --dyn-syms` per
      library, or `strace -e trace=open,futex` the plugin scan) for any OTHER `sem_open` user. The
      three libraries kept in-tree are clean; the rest of the closure — `libPmLogLib.so` included — is
      not in this repo and cannot be scanned here.

**Dependencies:** R1, R2
**Cross-reference:** cavekit-addons-extensions.md R7 — the plugin host is where this boundary is
crossed most, because a device NPAPI plugin drags the whole webOS WebKit stack into our process.

### R5: Fits the device memory budget
**Description:** The renderer runs within the constraints of a 1 GB device.
**Acceptance Criteria:**
- [x] On a typical page, render-process memory stays within a documented budget. *(2026-08-04, measured on the TouchPad — **the budget is now a number rather than an aspiration: ~90 MB RSS, and it must stay under 150 MB.** Observed: 86.5 MB after an eight-site session, 87.9 MB back on a single simple page, 105 MB on a large Wikipedia article. The device has 940 MB total, so a browser card sits at roughly 9–11% of system memory. Compare the engine's own stock settings, which this build overrides: the surfacecache default alone is 1 GB. If a future change pushes a typical page past 150 MB, treat it as a regression and find out what grew.)*
- [x] `freeze`/`purgePage` reclaim memory for backgrounded cards. *(2026-08-04, on device: `freeze` — the command a card sends when it is backgrounded — drops RSS from 105,156 kB to 99,236 kB, **5.9 MB reclaimed**, and `thaw` brings the page back with the daemon alive throughout. The saving is the shm attachments being released: the production segments are IPC_RMID-marked, so holding an attach keeps them alive and would waste the device's scarce shared memory for as long as the card sits in the background. One correction to this criterion's wording: **there is no `purgePage` in the frozen contract** — `grep` finds none in `BrowserServerBase`. `freeze`/`thaw` is the whole backgrounding surface, and it is the one measured here. Driven through new `freeze`/`thaw` inject commands, because until now nothing but a real card could exercise this path.)*
- [x] No out-of-memory crash during a defined browsing scenario. *(2026-08-04 — the scenario, so it can be re-run: eight sites in sequence (example.com, BBC News, two Wikipedia articles, Reddit, Hacker News, CNN, wikipedia.org) at ~14 s each, on the device, in one daemon lifetime. Result: no crash, no fault report, `dmesg` shows no OOM-killer activity, the daemon was still painting at the end, and available system memory moved only 441 → 406 MB. Repeated with the memory-pressure guardrail firing five times mid-session, with the same outcome.)* *(prior note: [human-review on device; deeper tuning is Phase 3]
**Dependencies:** cavekit-ipc-contract.md (R3)

### R10: CPU frequency scaling is part of the device contract

Added 2026-08-10 after it turned out to be the ceiling on Flash's frame rate AND the likely cause
of its audio xruns. This is a device-platform fact, not a Flash one, so it lives here rather than
in cavekit-addons-extensions.md R7 — anything in this browser that is judged on sustained
throughput will hit it.

The governor is Palm's **`ondemandtcl`**, and it is TOUCH-BIASED: stock `up_threshold` is 95, so
it only raises the clock when a single core exceeds 95% load. Passive work — playing an animation,
decoding audio, anything the user is watching rather than touching — never reaches that and runs
at the 192 MHz floor against a 1188 MHz maximum. Measured during Flash playback: 192000-384000,
snapping to 1188000 the moment the page was navigated away, with the whole system only 69% busy.

**Acceptance Criteria:**
- [x] The mechanism is measured rather than assumed, and the safe lever is identified.
      *(2026-08-10. Governor TUNABLES live at `/sys/devices/system/cpu/cpufreq/ondemandtcl/`;
      writing `up_threshold` and `sampling_rate` is safe, returns instantly, and does not switch
      governors. 40/50000 pins 1188000. The `override/` directory next to it — `turbo_mode`,
      `vdd_freqs` up to 1836000 — is OVERCLOCKING and is out of scope.)*
- [x] **Writing `scaling_governor` is documented as forbidden.** *(2026-08-10, learned by doing
      it: switching away from `ondemandtcl` deadlocks the cpufreq policy lock in the kernel. Every
      subsequent reader of any cpufreq node blocks in unkillable **D state** — the browser daemon,
      `/usr/sbin/powerlog`, and the restore script itself, so the boost is never released. Only a
      reboot clears it, and a clean `reboot` will not run because init waits on the D-state tasks:
      it takes `sync; sync; reboot -f`. Nothing persists, so the device returns to `ondemandtcl`.)*
- [x] Any component that raises the clock owns it and releases it. *(2026-08-10 — libxul patch
      `0027` holds the boost for the life of an NPAPI plugin instance and restores the values it
      read; the variant's upstart job restores stock from `post-stop` so a crashed daemon cannot
      strand a system-wide tuning. Both halves device-verified.)*
- [x] Whether the boost should extend beyond plugins — video, heavy JS, page load — is UNDECIDED.
      Nothing else in the browser currently raises the clock, so any non-plugin workload still
      runs against `up_threshold=95`.
      *(**DECIDED 2026-08-10 (T-118): DO NOT EXTEND IT. Measured on device, and the reason is that
      the problem the boost solves does not exist for non-plugin work.** A fixed JS workload
      (`/tmp/jihad-bench.html`, two runs per config, `scaling_cur_freq` sampled once a second
      throughout):*

      | tunables | run1 | run2 | clock during the workload |
      |---|---|---|---|
      | stock `95` / `200000` | 2146 ms | 1880 ms | ramps to **1188000**, the maximum |
      | tuned `40` / `50000` | 1754 ms | 1820 ms | 1188000 more often |

      ***The load-bearing observation is the clock column, not the milliseconds: under STOCK
      tunables the governor already ramps to full speed for JS.*** *That is the whole difference
      from the plugin case — a single-threaded JS loop drives one core past `up_threshold=95` on its
      own, whereas passive plugin playback never does (daemon ~26% + plugin-container ~11% spread
      over two cores, so no single core approaches 95 and the clock sits at the 192 MHz floor).
      `ondemandtcl` is doing exactly what it was designed to do; Flash is the pathological case
      because it is busy without ever looking busy to a per-core threshold.*
      *The residual ~11% from tuning is small, it is within a couple of 1 Hz samples of noise, and
      buying it would mean holding a SYSTEM-WIDE battery-burn setting during ordinary browsing.
      Not worth it. **Patch `0027` stays scoped to plugin-instance lifetime.** This also makes the
      conditional follow-up task (give the boost a second owner + release path for non-plugin
      consumers) moot — there is no second consumer.*
      *Method note for anyone re-running it: the governor tunables were written and RESTORED to
      95/200000 in the same script, with a readback after each write, and `scaling_governor` was
      never touched — writing it deadlocks cpufreq into unkillable D state, as the criterion two
      rows up records.)*

**Dependencies:** cavekit-addons-extensions.md (R7, the first consumer)

## Out of Scope
- Engine integration logic (covered by the Phase-1 domains; reused here).
- Deep performance optimization beyond fitting the budget (Phase 3).

## Cross-References
- See also: cavekit-engine-embedding.md, cavekit-desktop-build.md, cavekit-browser-services.md, cavekit-ipc-contract.md, cavekit-ui-shell.md, cavekit-mochi-ui.md

## Changelog
- 2026-08-10 (T-115): R3's prebuilt inputs declared as a bounded carve-out instead of an
  unmeetable "clean-clone reproducible" gate. All four are enumerated with provenance, size and
  redistribution status, measured on disk: crosstool-NG toolchain 219 MB, Jessie armel sysroot
  127 MB from 185 `.deb`s, Palm PDK 442 MB, adapter-deps 40 MB. **Only the PDK is permanent** —
  proprietary HP SDK, and `fetch-pdk.sh` downloads nothing (it extracts a `.deb` you supply, with
  no URL, version pin or checksum). Corrected R3's stale two-variant wording (heading,
  description, the bitbake criterion): three variants have shipped since 2026-08-05 and R7 has
  said so since 2026-07-31. Found while bounding it: `.gitignore`'s blanket
  `/build/webos-oe/toolchain/` also excluded the crosstool-NG RECIPE (`Dockerfile`,
  `build-toolchain.sh`, `jihad.defconfig`), so the one input that IS fully rebuildable from source
  had no recipe in a clean clone; the ignore was narrowed. What still holds R3 at `[~]` is now
  named exactly: no bitbake run has ever produced all three `.ipk`s.
- 2026-08-04: Recorded that engine PREFS ship in `goanna.js` and deploy with the engine —
  `push-engine-update.sh` now pushes it, and a pref-only change needs a daemon restart. Also that
  an ABSENT pref can be actively harmful here, because several are read with a one-argument
  `getCharPref` that throws instead of defaulting.
- 2026-06-30: Initial draft.
- 2026-06-30: Two UI `.ipk`s (Enyo + Mochi) in R3; added R6 TouchPad Go (Opal) support; R4 covers both models.
- 2026-07-04: Reconciled — R1 crosstool-NG toolchain (GCC 9.4/glibc 2.23 softfp) verified on-device; R2 DONE this session: X/GTK-free headless libxul cross-built, loads on the TouchPad and renders (on-device offscreen ROUND-TRIP PASS, msgPainted 786432). R3 two-.ipk (Mochi UI + real adapter), R4 full UI-on-screen + TouchPad Go, R5 memory budget (29M libxul helps), R6 Opal remain.
- 2026-07-29: OE build **stands up + produces two `.ipk`s** (new `build/webos-oe/oe-env.sh` — chroot Ubuntu-14.04 OE host, x86_64, sudo/doas) — engine/daemon/adapter/deviceroot recipes + `jihad-app.inc` products + a Mojo skeleton. But an adversarial review (**gpt-5.6-sol as the cavekit inspector**) found R3 was **OVER-CLAIMED** → reverted `[x]`→`[~]` (build-produced, not done). Real gaps: **Mochi `.ipk` missing its Enyo2/layout/Mochi frameworks** (#1) + adapter shim impl-path **hard-coded to the Enyo appid** (#5); **coexistence-removal breaks the survivor** (both share `/media/internal/jihad/hl`+shim+upstart, #4); **not clean-clone reproducible** (prebuilt toolchain/sysroot/PDK, #7); postinst masks failures (#6); LICENSE/NOTICE payload dropped (#12); none device-verified. Full findings: `../impl/impl-review-findings-oe.md`.
- 2026-07-31 (later): **R7 (per-variant independence) + R8 (good-citizen install footprint) added** on user decision — three fully standalone packages, nothing written to `/media/internal`, engine run in place from the app's cryptofs `deviceroot` (Atlas model). Retires the shared-runtime + `prerm` refcount design from the 2026-07-29 review (#4/#5). R3's Mojo criterion promoted from "scaffolded" to "ships a working front-end" (new cavekit-mojo-ui.md). Atlas was checked at the user's request for a way to avoid the `/usr/lib/BrowserPlugins` write and has none — that write is bounded + reversed by R8 instead.
- 2026-07-31: Reconciliation against recorded evidence. **R4 "Composite is correct in portrait and landscape" `[ ]`→`[x]`**: its 2026-07-20 note declared the portrait 3× shear an unfixable LunaCE limitation, but that was superseded by the PGContext composite fix — `../impl/rotation-fix-2026-07-26.md` + commit 8d7865c ("correct in both orientations … Verified on-device; the two PG symbols resolve against libWebKitLuna"), with `../impl/impl-overview.md` 2026-07-27 ("Rotation confirmed working on device") and the `../impl/zoom-fix-2026-07-27.md` preamble as independent corroboration; the old analysis is kept as a one-line history pointer. R4 "Basic navigation" keeps `[~]` but its note now cites the device-recorded progress (Session-4 nav sweep, crash fix 2be6d85, tap activation, link-tap hit-test d4f0842, zoom) and names exactly what remains (VKB jank, gesture-path pinch, OE `.ipk` verification). No other box changed — R3 stays `[~]` on the still-open clean-clone reproducibility (#7/#8) and device verification recorded in the READ-FIRST block above.
