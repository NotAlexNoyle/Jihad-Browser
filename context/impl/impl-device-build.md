---
created: "2026-07-18"
last_edited: "2026-07-18"
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
| T-054 | DONE (build-side) — Opal install/kernel [device-gated] | 2026-07-18, commit 41d49f0. Machine confs tenderloin+opal (`build/webos-oe/conf/machine/`), shared jihad-touchpad.inc (DEFAULTTUNE armv7a-neon softfp), COMPATIBLE_MACHINE on engine+daemon recipes, UI recipes allarch. Finding: both models one softfp binary set (same SoC family, same 1024x768; only DPI differs 132 vs 183). docs/DEVICE-BUILD.md Topaz-vs-Opal section. Opal install + kernel string pending hardware [human-review on device] |
