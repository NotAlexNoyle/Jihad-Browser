---
created: "2026-06-30"
last_edited: "2026-07-07"
---

# Cavekit: Device Build & Packaging

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
**Description:** The whole product packages as installable webOS artifacts that coexist with the stock browser, including BOTH front-end variants.
**Acceptance Criteria:**
- [x] The daemon + a rebuilt **coexisting** adapter (`BrowserAdapterJihad.so`, MIME `application/x-jihad-browser`, YAP name `jihad-browser`) install ALONGSIDE the stock browser without collision (`packaging/postinst`+`prerm`+`event.d/jihad`). Verified on-device.
- [x] The Enyo UI `.ipk` (`net.riverstonerelay.jihad-browser`, from `app/`) builds (`palm-package app/`) and installs; its WebView is routed to the Jihad engine by `app/source/JihadEngineOverride.js`.
- [ ] The build also produces the Mochi variant `.ipk` (`net.riverstonerelay.jihad-browser.mochi`, from `app-mochi/`); both install and can coexist.
- [ ] A single OE/repeatable build produces the daemon + adapter + both UI `.ipk`s (currently the daemon/adapter are built by `build/webos-oe/*.sh` and the Enyo `.ipk` by `palm-package`).
- [ ] The Mochi package bundles Enyo 2 + layout + Mochi; the Enyo package bundles Enyo 1.0.
**Dependencies:** R2, cavekit-desktop-build.md (R1), cavekit-mochi-ui.md (R1), jihad-self-contained-arch.md

### R4: Runs on the TouchPad (and TouchPad Go)
**Description:** The installed browser works on real hardware; both UI variants.
**Acceptance Criteria:**
- [x] On the TouchPad (Topaz/tenderloin), the **Enyo** variant launches and loads a page rendered on-screen via the Jihad adapter (`example.com`, `slack.com`/HTTPS render on fb1; load-completion + refresh glyph). Mochi variant pending.
- [~] Basic navigation works (URL load + load-complete); scrolling/tap-activation/keyboard are in Phase-3 hardening (staged fixes, on-device confirm pending).
- [ ] Cert/dialog/download flows function with the device services. [human-review on device]
- [ ] The same is verified on the TouchPad Go (Opal). [human-review on device]
**Dependencies:** R3, R6, cavekit-browser-services.md (R5)

### R6: TouchPad Go (Opal) machine support
**Description:** The device build targets both TouchPad models, since the upstream isis-browser runs on the TouchPad Go.
**Acceptance Criteria:**
- [x] The OE build provides machine configurations for both the TouchPad (Topaz/tenderloin) and the TouchPad Go (Opal). *(2026-07-18, T-054: `build/webos-oe/conf/machine/{tenderloin,opal}.conf` + shared `include/jihad-touchpad.inc`; engine+daemon recipes gain `COMPATIBLE_MACHINE = "(tenderloin|opal)"`.)*
- [~] The daemon, adapter, and both UI `.ipk`s build for, and install on, both machines (ARMv7 webOS 3). *(Build side satisfied 2026-07-18: both models share one ARMv7 softfp binary set — same APQ8060 family, same 1024x768 — documented per-machine build path in docs/DEVICE-BUILD.md. Install-on-Opal DEVICE-GATED: no TouchPad Go hardware present.)*
- [x] Any model-specific differences (screen geometry, machine config) are captured rather than assumed identical. [human-review on device] *(Captured 2026-07-18: only physical size/DPI differ — 9.7in ~132dpi vs 7in ~183dpi; opal kernel string left `?=` unverified pending hardware. docs/DEVICE-BUILD.md Topaz-vs-Opal section.)*
**Dependencies:** R2

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
