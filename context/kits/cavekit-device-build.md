---
created: "2026-06-30"
last_edited: "2026-06-30"
---

# Cavekit: Device Build & Packaging

## Scope
Phase-2 work to run Jihad Browser on the HP TouchPad (webOS 3.0.x, ARMv7):
standing up a modern cross-toolchain, cross-compiling the engine and daemon,
packaging via OpenEmbedded, and validating on-device. The toolchain is a
feasibility gate. Reference: `docs/TOOLCHAIN.md`, `build/webos-oe/README.md`.

## Requirements

### R1: Modern ARMv7 cross-toolchain (feasibility gate)
**Description:** A toolchain capable of building the engine for the device exists, since the stock device compiler cannot.
**Acceptance Criteria:**
- [ ] A documented, reproducible cross-toolchain (modern C++14-capable compiler) targets the device's glibc/kernel ABI.
- [ ] A trivial C++14 program built with it runs on a webOS 3 device/emulator.
- [ ] The toolchain links against (or matches) the device sysroot so binaries do not require a newer glibc than the device has.
**Dependencies:** none

### R2: Engine cross-compiles for webOS 3 ARMv7
**Description:** Goanna builds for the device target.
**Acceptance Criteria:**
- [ ] The engine builds with the cross-toolchain against the device sysroot.
- [ ] The resulting libraries load on the device without missing-symbol/ABI errors.
- [ ] ARMv7 FP/SIMD flags match the device CPU.
**Dependencies:** R1, cavekit-engine-embedding.md (R1)

### R3: OE recipes build and package Jihad
**Description:** The whole product packages as installable webOS artifacts.
**Acceptance Criteria:**
- [ ] Recipes build the daemon (with Goanna backend), the rebuilt BrowserAdapter, and the app package.
- [ ] The build produces `.ipk` package(s).
- [ ] The packages install on a webOS 3 device/emulator.
**Dependencies:** R2, cavekit-desktop-build.md (R1)

### R4: Runs on the TouchPad
**Description:** The installed browser works on real hardware.
**Acceptance Criteria:**
- [ ] The app launches and loads a page rendered on-screen via the BrowserAdapter.
- [ ] Basic navigation, scrolling, and tap input work.
- [ ] Cert/dialog/download flows function with the device services. [human-review on device]
**Dependencies:** R3, cavekit-browser-services.md (R5)

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
- See also: cavekit-engine-embedding.md, cavekit-desktop-build.md, cavekit-browser-services.md, cavekit-ipc-contract.md

## Changelog
- 2026-06-30: Initial draft.
