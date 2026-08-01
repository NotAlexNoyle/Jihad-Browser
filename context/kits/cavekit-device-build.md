---
created: "2026-06-30"
last_edited: "2026-07-31"
---

# Cavekit: Device Build & Packaging

## 2026-07-29 — OE BUILD stands up + produces `.ipk`s, but R3 is NOT complete (READ FIRST)

The reproducible Open webOS path (previously "aspirational, not runnable") now **stands up and
produces `.ipk`s** under `build-webos` + `meta-webos` (2013 "dylan" / bitbake 1.18). But an
adversarial review (gpt-5.6-sol as the cavekit inspector, 2026-07-29 — see
`../impl/impl-review-findings-oe.md`) found R3 was **over-claimed**; it is **build-produced, not
done**. Honest status:
- **Environment:** `build/webos-oe/oe-env.sh` — an OE host via a chroot into a downloaded Ubuntu-14.04
  rootfs (`sudo`/`doas`). Caveat: **x86_64 hosts only** (AMD64 rootfs), and it consumes prebuilt,
  git-ignored inputs (crosstool-NG toolchain, Jessie sysroot, Palm PDK, adapter-deps) — so **NOT
  "whole stack from source" / not clean-clone reproducible** (finding #7). See `docs/OE-BUILD.md`.
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
- **STILL OPEN for R3:** (a) **clean-clone reproducibility** — the build consumes prebuilt,
  git-ignored inputs (crosstool-NG toolchain, Jessie sysroot, Palm PDK, adapter-deps) (#7); the PDK
  is proprietary so full "from source" is bounded. (#8 is closed in code but not build-verified.)
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

### R3: Package Jihad as a self-contained app — both UI variants
**Description:** The whole product packages as installable webOS artifacts that coexist with the stock browser, including BOTH front-end variants. **Shipping model (2026-07-29): two SELF-CONTAINED app `.ipk`s** (Enyo + Mochi), each bundling engine+daemon+adapter+bundled-glibc+UI + a postinst that deploys the coexisting pieces — plus a Mojo UI skeleton.
**Acceptance Criteria:**
- [x] The daemon + a rebuilt **coexisting** adapter (`BrowserAdapterJihad.so`, MIME `application/x-jihad-browser`, YAP name `jihad-browser`) install ALONGSIDE the stock browser without collision (`packaging/postinst`+`prerm`+`event.d/jihad`). Verified on-device.
- [x] The Enyo UI `.ipk` (`net.riverstonerelay.jihad-browser`, from `app/`) builds (`palm-package app/`) and installs; its WebView is routed to the Jihad engine by `app/source/JihadEngineOverride.js`.
- [x] The build also produces the Mochi variant `.ipk` (`net.riverstonerelay.jihad-browser.mochi`, from `app-mochi/`); both install and can coexist. *(Produced 2026-07-19 (`build-mochi-ipk.sh`, 1.4 MB). INSTALL + COEXIST VERIFIED ON DEVICE 2026-07-19: `palm-install -l` lists both app ids at 1.0.0.)*
- [~] A single OE/bitbake build PRODUCES the two `.ipk`s. *(2026-07-29: `oe-env.sh run ". oe-init-build-env && bitbake net.riverstonerelay.jihad-browser net.riverstonerelay.jihad-browser.mochi"` → the two self-contained `.ipk`s (component recipes stage-only). BUT it consumes prebuilt, git-ignored inputs — crosstool-NG toolchain, Jessie sysroot, Palm PDK, adapter-deps (`jihad-cross-toolchain-native` only CHECKS the toolchain, does not build it) — so it is NOT clean-clone reproducible and NOT "whole stack from source" (review #7). The direct-cross-build scripts remain a faster path. **2026-07-31 — #8 CLOSED in code, build-unverified:** every task that reads a `${JIHAD_REPO}` input (goanna patch queue + mozconfig, browserserver/adapter/deviceroot scripts + sources, PDK, adapter-deps, toolchain, Jessie sysroot, Mochi frameworks, LICENSE/NOTICE) now declares it via `do_<task>[file-checksums]` — confirmed supported by the pinned bitbake 1.18.0 (`lib/bb/cache.py:135` + `lib/bb/siggen.py:189-193`), with small identity sets standing in for the 219 MB toolchain / 127 MB sysroot. Verified statically only (bitbake's own line grammar re-run over the recipes; `get_file_checksums` semantics simulated) — **the chroot was not runnable, so neither `bitbake -p` nor a real sstate run has exercised it; verify at the next `oe-env.sh` run.** Full coverage table + caveats: `../impl/impl-review-findings-oe.md`.)*
- [~] Each variant `.ipk` is SELF-CONTAINED and installs as one coexisting package. *(2026-07-29: `jihad-app.inc` bundles the `jihad-deviceroot` runtime + the UI app + impl + a `postinst`/`prerm`. **Review fixes applied + build-verified:** Mochi now stages its Enyo2/layout/Mochi frameworks (#1); the shim loads a shared root-owned impl `/usr/lib/jihad/BrowserAdapterImpl.so` so every variant loads its own impl (#5); `prerm` refcounts siblings — removing one no longer breaks the other (#4); `postinst` fails loud (#6). Both `.ipk`s (Enyo 40 MB, Mochi 42 MB) are structurally complete + coexistence-safe. STILL `[~]` because (a) not clean-clone reproducible — prebuilt toolchain/sysroot/PDK (#7); the undeclared-bitbake-inputs half (#8) is fixed in code 2026-07-31 via `do_<task>[file-checksums]` but NOT build-verified, (b) not device-verified.)*
- [~] A third UI variant SHIPS as a working Mojo front-end (not a scaffold). *(2026-07-29: `app-mojo/` scaffold + `net.riverstonerelay.jihad-browser.mojo` recipe build an `.ipk`, but the UI was a documented stub. 2026-07-31: promoted from "scaffolded" to "working" — the user requires all three front-ends to function standalone; the real UI is cavekit-mojo-ui.md.)*
- [~] The Mochi package bundles Enyo 2 + layout + Mochi; the Enyo package bundles Enyo 1.0. *(Mochi half DONE (T-049). Enyo half: INTENTIONAL DEVIATION — `app/index.html` loads Enyo 1.0 from the OS framework path `/usr/palm/frameworks/enyo/0.10`, exactly like upstream isis-browser; bundling would duplicate the system framework and risk skew.)*
**Dependencies:** R2, cavekit-desktop-build.md (R1), cavekit-mochi-ui.md (R1), jihad-self-contained-arch.md

### R4: Runs on the TouchPad (and TouchPad Go)
**Description:** The installed browser works on real hardware; both UI variants.
**Acceptance Criteria:**
- [x] On the TouchPad (Topaz/tenderloin), the **Enyo** variant launches and loads a page rendered on-screen via the Jihad adapter (`example.com`, `slack.com`/HTTPS render on fb1; load-completion + refresh glyph). Mochi variant pending. *(NOTE 2026-07-29: this was verified via the direct-cross-build deploy. Installing the NEW self-contained OE `.ipk`s (R3) on-device and confirming their postinst lays the bundle down + renders is the current open gate — [human-review on device].)*
- [~] Basic navigation works (URL load + load-complete); scrolling/tap-activation/keyboard are in Phase-3 hardening (staged fixes, on-device confirm pending). *(2026-07-31 reconciliation — recorded device progress since this note was written: back/forward/reload + address→openUrl daemon-verified 2026-07-20 (`../impl/device-test-2026-07-19.md` Session 4); the daemon crash behind the "overload"/stuck-overlay was root-caused + fixed and stress-verified (Session 3, commit 2be6d85); checkbox/radio tap activation verified on-device 2026-07-20 (cavekit-input-bridging R1); link taps hit-test + navigate on-device after the zoom coordinate-consistency fix (commit d4f0842, `../impl/zoom-fix-2026-07-27.md`); pinch/fit zoom magnifies + pans on-device (`../impl/zoom-fix-2026-07-27.md`). REMAINS `[~]` for: the VKB white-band/"snap" jank (`../impl/device-test-2026-07-19.md` "Still open (hard)"), gesture-path pinch (no gesture injection into LunaSysMgr — human check), and any verification of the NEW self-contained OE `.ipk`s.)*
- [x] Composite is correct in portrait and landscape. *(FIXED + device-confirmed 2026-07-27. The portrait 3× shear was the raw `dstBuffer` path: LunaCE reads that surface at a fixed ~256px (1024-byte) pitch, so 768-wide portrait tiled 3× while 1024-wide landscape (4×256) was clean. Fix: composite through the WebKit-provided Piranha **PGContext** (`AdapterBase(..., useGraphicsContext=true)` + `PGSurface::wrap` + `gc->bitblt`), which carries the card's rotation/scale transform, so LunaCE never misreads a raw linear buffer — `../impl/rotation-fix-2026-07-26.md`, cavekit-offscreen-rendering.md R6, commit **8d7865c** (2026-07-27): "correct in both orientations, superseding the white-frame guard … **Verified on-device; the two PG symbols resolve against libWebKitLuna**". Independently corroborated: `../impl/impl-overview.md` 2026-07-27 opens "**Rotation confirmed working on device**", and `../impl/zoom-fix-2026-07-27.md` frames the next user report as arriving "after rotation was confirmed working". Caveat on the strength of the evidence: the confirmation is recorded as a device/visual confirm in the commit + impl entries — no per-orientation screen capture was archived. SUPERSEDES the 2026-07-20 analysis that declared this an unfixable LunaCE limitation (that raw-dstBuffer-era root cause + the disproven write-stride attempt are preserved in `../impl/rotation-fix-2026-07-26.md` and [[jihad-input-activation-and-tiling]]). Separate KNOWN quirk (NOT this): black card + VKB on the FIRST launch after a LunaSysMgr restart, cleared by reopening.)*
- [ ] Cert/dialog/download flows function with the device services. [human-review on device]
- [ ] The same is verified on the TouchPad Go (Opal). [human-review on device]
**Dependencies:** R3, R6, cavekit-browser-services.md (R5)

### R6: TouchPad Go (Opal) machine support
**Description:** The device build targets both TouchPad models, since the upstream isis-browser runs on the TouchPad Go.
**Acceptance Criteria:**
- [x] The OE build provides machine configurations for both the TouchPad (Topaz/tenderloin) and the TouchPad Go (Opal). *(2026-07-18, T-054: `build/webos-oe/conf/machine/{tenderloin,opal}.conf` + shared `include/jihad-touchpad.inc`; engine+daemon recipes gain `COMPATIBLE_MACHINE = "(tenderloin|opal)"`.)*
- [~] The daemon, adapter, and both UI `.ipk`s build for, and install on, both machines (ARMv7 webOS 3). *(Build side satisfied 2026-07-18: both models share one ARMv7 softfp binary set — same APQ8060 family, same 1024x768 — documented per-machine build path in docs/DEVICE-BUILD.md. Install-on-Opal DEVICE-GATED: no TouchPad Go hardware present.)*
- [~] Any model-specific differences (screen geometry, machine config) are captured rather than assumed identical. [human-review on device] *(Paper capture done 2026-07-18: only physical size/DPI differ — 9.7in ~132dpi vs 7in ~183dpi; opal kernel string left `?=` unverified. Stays [~] until the on-device human review on real Opal hardware confirms (codex F-371).)*
**Dependencies:** R2

### R7: Each UI variant is an independent package
**Description:** The Enyo, Mochi, and Mojo packages are three fully standalone browsers. Installing, upgrading, or removing any one of them has no effect on the others — there is no shared component whose lifetime is owned by more than one package. (User decision 2026-07-31: "each app needs to work entirely on its own, self contained"; supersedes the 2026-07-29 refcount-a-shared-runtime model.)
**Acceptance Criteria:**
- [ ] Each variant owns a distinct NPAPI MIME type, adapter shim filename, adapter impl path, YAP service name, YAP socket path, and upstart job name — no two variants resolve to the same one.
- [ ] Each variant's front-end routes its own WebView to its own MIME type only; another variant's card never loads this variant's adapter.
- [ ] Installing variant B while variant A is installed does not overwrite, downgrade, or invalidate any file A owns.
- [ ] Removing any one variant leaves every other installed variant fully functional (launch + load a page), with no refcount logic required to make that true.
- [ ] With two or more variants installed, each card is served by its own daemon process on its own socket; a crash or restart of one daemon does not disturb another variant's card.
**Dependencies:** R3, cavekit-ipc-contract.md (R1), cavekit-ui-shell.md, cavekit-mochi-ui.md, cavekit-mojo-ui.md

### R8: Good webOS citizen — install footprint contract
**Description:** The package behaves the way webOS expects a third-party app to behave: it keeps its own runtime inside its own app bundle, does not colonize the user's storage, touches the smallest possible amount of the system, and reverses itself exactly. (User decisions 2026-07-31: "without modifying system files", "follow atlas in not copying anything to /media/internal". Reference behavior: the Atlas browser, which runs its engine in place from the app's cryptofs `deviceroot` and copies nothing to `/media/internal`.)
**Acceptance Criteria:**
- [ ] Install writes NOTHING to `/media/internal` (the user's USB mass-storage volume). The engine, daemon, bundled glibc, and GRE resources are executed in place from the app's own `deviceroot/` on cryptofs.
- [ ] No stock file is modified, replaced, moved, or backed-up-and-swapped. Verified by comparing the checksum of every pre-existing system file the package interacts with (at minimum `/usr/lib/BrowserPlugins/BrowserAdapter.so`, `/usr/lib/BrowserPlugins/BrowserAdapterMojo.so`, `/etc/event.d/browserserver`) before and after install + removal.
- [ ] The rootfs footprint is limited to files the package uniquely owns, each namespaced to the variant: its adapter shim under `/usr/lib/BrowserPlugins`, its adapter impl under a Jihad-owned directory, and its own upstart job under `/etc/event.d`. Every such path is enumerated in the packaging docs.
- [ ] Runtime writable state (log, engine profile/cache, any debug channel) lives under a root-owned, variant-scoped path outside the user's storage, and is removed on uninstall.
- [ ] `prerm` removes exactly the files this package created and leaves everything else — verified by a full filesystem diff across an install→remove cycle showing no residue and no collateral deletion.
- [ ] The rootfs read-write window is opened only for the duration of the rootfs writes and is restored to read-only on every exit path, including failure.
**Dependencies:** R3, R7

### R5: Fits the device memory budget
**Description:** The renderer runs within the constraints of a 1 GB device.
**Acceptance Criteria:**
- [ ] On a typical page, render-process memory stays within a documented budget.
- [ ] `freeze`/`purgePage` reclaim memory for backgrounded cards.
- [ ] No out-of-memory crash during a defined browsing scenario. [human-review on device; deeper tuning is Phase 3]
**Dependencies:** cavekit-ipc-contract.md (R3)

## Out of Scope
- Engine integration logic (covered by the Phase-1 domains; reused here).
- Deep performance optimization beyond fitting the budget (Phase 3).

## Cross-References
- See also: cavekit-engine-embedding.md, cavekit-desktop-build.md, cavekit-browser-services.md, cavekit-ipc-contract.md, cavekit-ui-shell.md, cavekit-mochi-ui.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-06-30: Two UI `.ipk`s (Enyo + Mochi) in R3; added R6 TouchPad Go (Opal) support; R4 covers both models.
- 2026-07-04: Reconciled — R1 crosstool-NG toolchain (GCC 9.4/glibc 2.23 softfp) verified on-device; R2 DONE this session: X/GTK-free headless libxul cross-built, loads on the TouchPad and renders (on-device offscreen ROUND-TRIP PASS, msgPainted 786432). R3 two-.ipk (Mochi UI + real adapter), R4 full UI-on-screen + TouchPad Go, R5 memory budget (29M libxul helps), R6 Opal remain.
- 2026-07-29: OE build **stands up + produces two `.ipk`s** (new `build/webos-oe/oe-env.sh` — chroot Ubuntu-14.04 OE host, x86_64, sudo/doas) — engine/daemon/adapter/deviceroot recipes + `jihad-app.inc` products + a Mojo skeleton. But an adversarial review (**gpt-5.6-sol as the cavekit inspector**) found R3 was **OVER-CLAIMED** → reverted `[x]`→`[~]` (build-produced, not done). Real gaps: **Mochi `.ipk` missing its Enyo2/layout/Mochi frameworks** (#1) + adapter shim impl-path **hard-coded to the Enyo appid** (#5); **coexistence-removal breaks the survivor** (both share `/media/internal/jihad/hl`+shim+upstart, #4); **not clean-clone reproducible** (prebuilt toolchain/sysroot/PDK, #7); postinst masks failures (#6); LICENSE/NOTICE payload dropped (#12); none device-verified. Full findings: `../impl/impl-review-findings-oe.md`.
- 2026-07-31 (later): **R7 (per-variant independence) + R8 (good-citizen install footprint) added** on user decision — three fully standalone packages, nothing written to `/media/internal`, engine run in place from the app's cryptofs `deviceroot` (Atlas model). Retires the shared-runtime + `prerm` refcount design from the 2026-07-29 review (#4/#5). R3's Mojo criterion promoted from "scaffolded" to "ships a working front-end" (new cavekit-mojo-ui.md). Atlas was checked at the user's request for a way to avoid the `/usr/lib/BrowserPlugins` write and has none — that write is bounded + reversed by R8 instead.
- 2026-07-31: Reconciliation against recorded evidence. **R4 "Composite is correct in portrait and landscape" `[ ]`→`[x]`**: its 2026-07-20 note declared the portrait 3× shear an unfixable LunaCE limitation, but that was superseded by the PGContext composite fix — `../impl/rotation-fix-2026-07-26.md` + commit 8d7865c ("correct in both orientations … Verified on-device; the two PG symbols resolve against libWebKitLuna"), with `../impl/impl-overview.md` 2026-07-27 ("Rotation confirmed working on device") and the `../impl/zoom-fix-2026-07-27.md` preamble as independent corroboration; the old analysis is kept as a one-line history pointer. R4 "Basic navigation" keeps `[~]` but its note now cites the device-recorded progress (Session-4 nav sweep, crash fix 2be6d85, tap activation, link-tap hit-test d4f0842, zoom) and names exactly what remains (VKB jank, gesture-path pinch, OE `.ipk` verification). No other box changed — R3 stays `[~]` on the still-open clean-clone reproducibility (#7/#8) and device verification recorded in the READ-FIRST block above.
