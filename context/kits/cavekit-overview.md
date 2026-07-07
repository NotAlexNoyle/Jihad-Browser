---
created: "2026-06-30"
last_edited: "2026-07-04"
---

# Cavekit Overview

## Project
Jihad Browser — port the UXP/Goanna web engine into the isis-browser webOS 3
(HP TouchPad) shell, replacing QtWebKit while keeping the
BrowserAdapter↔BrowserServer YAP IPC contract byte-identical. Phase 1 brings the
engine up on desktop x86_64; Phase 2 cross-compiles for the device.

Grounding: `context/refs/refs-overview.md`, `docs/IPC-CONTRACT.md`,
`render/goanna/PORT-MAP.md`.

## Domain Index
Status legend: ✅ complete · 🟢 mostly (device/edge items remain) · 🟡 partial · ⬜ not started.
Verified on desktop x86_64 AND cross-built + rendering on the HP TouchPad unless noted.

| Domain | Cavekit File | Requirements | Status | Description |
|--------|--------------|--------------|--------|-------------|
| UI Shell (Enyo) | cavekit-ui-shell.md | 4 | 🟢 R1–R3 ✓; R4 vs Jihad daemon on-device | Forked/rebranded Enyo-1.0 app (`app/`) using the unchanged adapter contract |
| Mochi UI Variant | cavekit-mochi-ui.md | 5 | ⬜ 0/5 — `app-mochi/` skeleton only | Second front-end on Enyo-2/Mochi (`app-mochi/`), same contract, separate .ipk |
| IPC Contract Preservation | cavekit-ipc-contract.md | 5 | 🟢 R1–R3 ✓; R4/R5 device integration | Frozen YAP interface, shmem framebuffer, daemon, LunaService |
| Engine Embedding & Build | cavekit-engine-embedding.md | 4 | ✅ 4/4 (+ ARM cross-build) | Out-of-tree Goanna build, embedding runtime, event-loop integration |
| Offscreen Rendering | cavekit-offscreen-rendering.md | 5 | ✅ 5/5 desktop + on-device | Headless render → shared buffer → paint protocol + geometry events |
| Input Bridging | cavekit-input-bridging.md | 5 | 🟢 R1/R4/R5 ✓; R2/R3 on-device | webOS pointer/key/touch/gesture → DOM events |
| Navigation, Loading & Events | cavekit-navigation-events.md | 6 | ✅ 6/6 | Nav commands + load/location/title/history message stream |
| Browser Services | cavekit-browser-services.md | 5 | 🟢 R1–R3 ✓; R4/R5 partial (device) | Settings, cookies/cache, JS dialogs, downloads, TLS |
| Desktop Build & PoC Harness | cavekit-desktop-build.md | 4 | ✅ R1–R3; R4 [human-review] | Phase-1 x86_64 build + YAP test client + end-to-end gate |
| Device Build & Packaging | cavekit-device-build.md | 6 | 🟡 R1/R2 ✓ (engine renders on device); R3–R6 pending | Phase-2 ARM cross-toolchain, OE recipes, two .ipks, TouchPad + TouchPad Go |
| Licensing & Branding | cavekit-licensing-branding.md | 5 | ✅ 5/5 | Apache+MPL headers, NOTICE, trademark stripping (cross-cut) |

Totals: 11 domains, 54 requirements — **~38 met/verified, ~16 remaining** (Mochi UI, device UI-integration: LunaService daemon + real BrowserAdapter, on-device input/gestures, TouchPad Go).

### Milestone (2026-07-04): headless engine, no X/GTK, renders on the TouchPad
`MOZ_WIDGET_TOOLKIT=headless` — libxul links **zero** gtk/gdk/pango/cairo/X libs
(freetype+fontconfig only); ARM cross-build (29 M libxul) + GTK-free daemon → lean
device bundle (**28 .so vs 68**) → **on-device offscreen ROUND-TRIP PASS** (msgPainted
786432). This closes Engine-Embedding R2, Offscreen Rendering on-device, and
Device-Build R2, and advances Device-Build R4/R5. Detail in
`context/impl/impl-overview.md` + auto-memory `jihad-headless-toolkit.md`.

## Cross-Reference Map
| Domain A | Interacts With | Interaction Type |
|----------|----------------|------------------|
| UI Shell (Enyo) | IPC Contract | uses contract (client) |
| UI Shell (Enyo) | Navigation/Events | drives navigation, observes events |
| Mochi UI Variant | IPC Contract / UI Shell | same contract; parity with Enyo UI |
| Mochi UI Variant | Device Build | second .ipk packaged + on both TouchPad models |
| IPC Contract | Offscreen Rendering | framebuffer + paint protocol |
| IPC Contract | Engine Embedding | page lifecycle / page manager |
| IPC Contract | Browser Services | LunaService surface |
| Engine Embedding | Offscreen Rendering | provides instance + event loop for painting |
| Engine Embedding | Navigation/Events | provides event loop for load progress |
| Offscreen Rendering | Input Bridging | shared transform (zoom/scroll ↔ coordinate mapping) |
| Offscreen Rendering | Navigation/Events | geometry/viewport events |
| Navigation/Events | Browser Services | redirects, MIME handoff, downloads |
| Browser Services | Device Build | device cert store / Luna integration |
| Licensing & Branding | UI / IPC / Engine / Services | cross-cutting compliance |
| Desktop Build | Engine/Render/Nav/Input | integrates them into the Phase-1 PoC |
| Device Build | Engine Embedding / Desktop Build | reuses integration, cross-compiles + packages |

## Dependency Graph
Implementation order (earlier enables later):

```
Tier 0 (foundations, parallel):
  IPC Contract Preservation
  Licensing & Branding (cross-cutting; applied throughout)

Tier 1:
  Engine Embedding & Build     (needs: build host per docs/TOOLCHAIN.md)

Tier 2 (the integration core, co-developed on Engine Embedding + IPC):
  Offscreen Rendering          (needs: IPC R2, Engine Embedding R2/R3)
  Navigation, Loading & Events (needs: IPC R1, Engine Embedding R3)
  Input Bridging               (consumes Offscreen transform)
  Browser Services             (needs: IPC R4, Engine Embedding R3, Navigation R6)

Tier 3:
  Desktop Build & PoC Harness  (Phase-1 acceptance: needs IPC + Engine + Tier 2)

Tier 4 (Phase 2):
  Device Build & Packaging     (needs: Tier 2/3 + cross-toolchain gate;
                                produces TWO UI .ipks for TouchPad + TouchPad Go)

Parallel UI track (depends only on the contract; packaged by Device Build):
  UI Shell (Enyo)   — forked/rebranded, mostly done
  Mochi UI Variant  — Enyo-2/Mochi rewrite to parity (uses Navigation/Services
                      contracts as its behavioral reference)
```

Notes:
- The graph is acyclic. Tier-2 domains are tightly coupled (the paint loop needs
  both the engine event loop and the buffer protocol) so they are co-developed,
  but each depends only on Tier-0/Tier-1 — bidirectional links between them are
  conceptual (see each kit's Cross-References), not build-order dependencies.
- The cross-toolchain (Device Build R1) has no engine dependency and can be
  stood up in parallel with Phase-1 integration to de-risk Phase 2.
- Both UI variants share the BrowserAdapter contract and can be developed in
  parallel with the engine work; only their on-device verification needs the
  working daemon. The Mochi rewrite is the larger of the two.
