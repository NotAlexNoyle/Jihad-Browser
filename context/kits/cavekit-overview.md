---
created: "2026-06-30"
last_edited: "2026-06-30"
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
| Domain | Cavekit File | Requirements | Status | Description |
|--------|--------------|--------------|--------|-------------|
| UI Shell | cavekit-ui-shell.md | 4 | DRAFT | Forked/rebranded Enyo app using the unchanged adapter contract |
| IPC Contract Preservation | cavekit-ipc-contract.md | 5 | DRAFT | Frozen YAP interface, shmem framebuffer, daemon, LunaService |
| Engine Embedding & Build | cavekit-engine-embedding.md | 4 | DRAFT | Out-of-tree Goanna build, embedding runtime, event-loop integration |
| Offscreen Rendering | cavekit-offscreen-rendering.md | 5 | DRAFT | Headless render → shared buffer → paint protocol + geometry events |
| Input Bridging | cavekit-input-bridging.md | 5 | DRAFT | webOS pointer/key/touch/gesture → DOM events |
| Navigation, Loading & Events | cavekit-navigation-events.md | 6 | DRAFT | Nav commands + load/location/title/history message stream |
| Browser Services | cavekit-browser-services.md | 5 | DRAFT | Settings, cookies/cache, JS dialogs, downloads, TLS |
| Desktop Build & PoC Harness | cavekit-desktop-build.md | 4 | DRAFT | Phase-1 x86_64 build + YAP test client + end-to-end gate |
| Device Build & Packaging | cavekit-device-build.md | 5 | DRAFT | Phase-2 ARM cross-toolchain, OE recipes, .ipk, on-device |
| Licensing & Branding | cavekit-licensing-branding.md | 5 | DRAFT | Apache+MPL headers, NOTICE, trademark stripping (cross-cut) |

Totals: 10 domains, 48 requirements.

## Cross-Reference Map
| Domain A | Interacts With | Interaction Type |
|----------|----------------|------------------|
| UI Shell | IPC Contract | uses contract (client) |
| UI Shell | Navigation/Events | drives navigation, observes events |
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
  Device Build & Packaging     (needs: Tier 2/3 + cross-toolchain gate)
```

Notes:
- The graph is acyclic. Tier-2 domains are tightly coupled (the paint loop needs
  both the engine event loop and the buffer protocol) so they are co-developed,
  but each depends only on Tier-0/Tier-1 — bidirectional links between them are
  conceptual (see each kit's Cross-References), not build-order dependencies.
- The cross-toolchain (Device Build R1) has no engine dependency and can be
  stood up in parallel with Phase-1 integration to de-risk Phase 2.
