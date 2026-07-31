---
created: "2026-06-30"
last_edited: "2026-07-31"
---

# Cavekit Overview

## Project
Jihad Browser — port the UXP/Goanna web engine into the isis-browser webOS 3
(HP TouchPad) shell, replacing QtWebKit while keeping the
BrowserAdapter↔BrowserServer YAP IPC command/message contract byte-identical.
Phase 1 brings the engine up on desktop x86_64; Phase 2 cross-compiles for the
device. Jihad ships **self-contained, coexisting with the stock browser** — its
own MIME/adapter/daemon/upstart job, nothing system-level replaced (Atlas model;
see `jihad-self-contained-arch.md`). The adapter carries a two-line rebrand (MIME
string + YAP server name) for coexistence; the YAP command/message interface it
speaks is unchanged.

Grounding: `context/refs/refs-overview.md`, `docs/IPC-CONTRACT.md`,
`render/goanna/PORT-MAP.md`.

## Domain Index
Status legend: ✅ complete · 🟢 mostly (device/edge items remain) · 🟡 partial · ⬜ not started.
Verified on desktop x86_64 AND cross-built + rendering on the HP TouchPad unless noted.

| Domain | Cavekit File | Requirements | Status | Description |
|--------|--------------|--------------|--------|-------------|
| UI Shell (Enyo) | cavekit-ui-shell.md | 4 | 🟢 R1–R3 ✓; R4 3/4 ACs device-verified 2026-07-20 (launch, address→openUrl, back/forward/reload — device-test-2026-07-19 Session 4); findInPage untested | Forked/rebranded Enyo-1.0 app (`app/`) using the unchanged adapter contract |
| Mochi UI Variant | cavekit-mochi-ui.md | 5 | 🟡 R1 ✓ (dual-install VERIFIED ON DEVICE 2026-07-19), R3 ✓, R5 ✓; R2/R4 [~] — full parity views + dialogs built and PARITY.md complete, on-device functional verification pending | Second front-end on Enyo-2/Mochi (`app-mochi/`), same contract, separate .ipk |
| IPC Contract Preservation | cavekit-ipc-contract.md | 5 | 🟢 R1–R3 ✓; R5 ✓ on-device (adapter drives daemon; +2-line coexistence rebrand); R4 device LunaService | Frozen YAP command/message interface, shmem framebuffer, daemon, LunaService |
| Engine Embedding & Build | cavekit-engine-embedding.md | 4 | ✅ 4/4 (+ ARM cross-build) | Out-of-tree Goanna build, embedding runtime, event-loop integration |
| Offscreen Rendering | cavekit-offscreen-rendering.md | 6 | ✅ 6/6 desktop + on-device (R6 rotation composite + the R5 zoom rework device-confirmed 2026-07-27) | Headless render → shared buffer → paint protocol + geometry events + orientation-correct composite |
| Input Bridging | cavekit-input-bridging.md | 5 | 🟢 R1 ✓ (XUL-button activation deferred — crashes headless), R4 ✓, R5 ✓ (link hit-test fixed + device-verified 2026-07-27); R2 VKB jank / R3 gestures on-device | webOS pointer/key/touch/gesture → DOM events |
| Navigation, Loading & Events | cavekit-navigation-events.md | 6 | 🟢 6/6 (R6's link-clicked AC still [~] — on-device link-tap navigation confirmed 2026-07-27, the message-emission re-test is open) | Nav commands + load/location/title/history message stream |
| Browser Services | cavekit-browser-services.md | 5 | 🟢 R1–R3 ✓; R4/R5 partial (device) | Settings, cookies/cache, JS dialogs, downloads, TLS |
| Desktop Build & PoC Harness | cavekit-desktop-build.md | 4 | ✅ R1–R3; R4 [human-review] | Phase-1 x86_64 build + YAP test client + end-to-end gate |
| Device Build & Packaging | cavekit-device-build.md | 6 | 🟡 R1/R2 ✓; **R3 BUILD-PRODUCED, not done** — the OE build (`oe-env.sh`) emits two `.ipk`s + a Mojo skeleton, and the 2026-07-29 review items #1/#4/#5/#6/#9/#12 (Mochi frameworks, prerm refcount, shared shim impl path, loud postinst, bundle manifest, LICENSE/NOTICE) are **fixed + build-verified** (commits 9413d16, b1c0112) — the two open gaps are **clean-clone reproducibility** (prebuilt toolchain/sysroot/PDK + undeclared bitbake inputs, #7/#8) and **on-device install verification**; R4 rotation composite ✓ (2026-07-27) with cert/download flows + Opal open; R5/R6 device-gated | Phase-2 ARM cross-toolchain; self-contained packaging via bitbake; TouchPad + TouchPad Go |
| Licensing & Branding | cavekit-licensing-branding.md | 5 | ✅ 5/5 | Apache+MPL headers, NOTICE, trademark stripping (cross-cut) |

Totals: 11 domains, **55 requirements** (offscreen R6 added 2026-07-26) — **~43 met/verified, ~12
open**, and every open item is either device-gated or a named debug lead. Current priorities
(reprioritized 2026-07-31 against the recorded evidence; the 2026-07-29 review items #1/#4/#5/#12
that used to lead this list are fixed + build-verified — see cavekit-device-build.md and commit
b1c0112):
(1) **Install the two self-contained OE `.ipk`s on device** and confirm the postinst lays the
bundle down, both variants render, and coexistence + removal are safe — the single gate blocking
device-build R3's "device-verified" and R4;
(2) **Mochi on-device functional verification** (mochi-ui R2/R4) — the parity views, dialogs, and
PARITY.md are complete; nothing but hardware time is missing;
(3) **cookie/cache persistence GAP** (browser-services R2) — no `cookies.sqlite` is created on
device despite a correct profile provider + prefs (device-test-2026-07-19 Session 4); the only
open item here with a concrete debug lead rather than a hardware gate;
(4) device LunaService methods (IPC R4, browser-services R2/R4 device half);
(5) remaining on-device input work — VKB white-band/"snap" jank (input R2) and the real
pinch/touch gesture path (input R3; the daemon-side zoom itself is device-verified);
(6) ui-shell R4 findInPage focused test — the smallest open item on the board;
(7) clean-clone reproducibility of the OE build (device-build R3, review #7/#8 — a larger refactor;
the PDK is proprietary so "from source" is bounded);
(8) TouchPad Go (Opal) hardware (device-build R6) and memory-budget measurement (R5).
Full OE-review findings: `../impl/impl-review-findings-oe.md`.

### Milestone (2026-07-27): rotation and zoom work on the device
The portrait↔landscape render-break and the "things get cut off" zoom are both fixed and
confirmed on hardware (commit 8d7865c; `../impl/rotation-fix-2026-07-26.md`,
`../impl/zoom-fix-2026-07-27.md`, `../impl/impl-overview.md` 2026-07-27 "Rotation confirmed
working on device"). Rotation: the adapter now composites through the WebKit Piranha
**PGContext** (transform-aware) instead of a raw `dstBuffer` blit that LunaCE read at a fixed
~256px pitch — that pitch, not any daemon bug, was the portrait 3× shear. Zoom: `RenderDocument`'s
internal scale cancelled the engine zoom, so the daemon now pre-scales the gfxContext and renders
an absolute document rect (magnify + full-page pan in both axes, layout viewport untouched so
rotation stays safe). The same change made the daemon report the EFFECTIVE render scale, which
closed the long-open small-link tap miss (input-bridging R5, commit d4f0842). Closes offscreen
R6, closes device-build R4's composite AC, and supersedes the 2026-07-20 "LunaCE limitation, NOT
fixable" analysis.

### Milestone (2026-07-07): self-contained app renders real pages on the TouchPad
Re-architected from *replacing* the system browser to a **self-contained app that
coexists** with it (Atlas model: own MIME `application/x-jihad-browser` →
`BrowserAdapterJihad.so` → `/tmp/yapserver.jihad-browser` → upstart job `jihad`; the
app's WebView routed there by `app/source/JihadEngineOverride.js`). On-device the card
loads the Jihad adapter → the Jihad daemon, `example.com` + `slack.com`/HTTPS load and
render (fb1), load-completion fires (address-bar refresh glyph), and the stock browser
keeps working for all other apps. This meets IPC-Contract R5 (rebuilt adapter drives the
daemon through a full load+paint cycle) and advances Device-Build R4. Deploy gotchas
(reboot registers the plugin; `.ipk` reinstall busts the WebKit cache; content is on fb1)
in auto-memory `jihad-self-contained-arch.md` + `jihad-device-gotchas.md`.

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
  Mochi UI Variant  — Enyo-2/Mochi rewrite to parity, built (uses Navigation/
                      Services contracts as its behavioral reference)
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
  working daemon. The Mochi rewrite was the larger of the two; as of 2026-07-20
  its shell, parity views, dialogs, and `app-mochi/PARITY.md` are built and it
  dual-installs on device (2026-07-19) — what is left is on-device functional
  verification, not construction.

## Changelog
- 2026-07-31: Reconciliation against recorded evidence. The Mochi row read "⬜ 0/5 — skeleton
  only" while cavekit-mochi-ui.md records R1 (dual-install VERIFIED ON DEVICE 2026-07-19), R3 and
  R5 as met → now 🟡 R1/R3/R5 ✓, R2/R4 [~]. The UI-Shell row now reflects R4's three
  device-verified ACs (device-test-2026-07-19 Session 4). The Offscreen row goes 5→6 requirements
  (R6, added 2026-07-26) and 6/6 (rotation composite + zoom rework device-confirmed 2026-07-27).
  The Device-Build row no longer describes the Mochi `.ipk` as broken — review items #1/#4/#5/#6/
  #9/#12 are fixed + build-verified (commits 9413d16, b1c0112); the two real gaps (clean-clone
  reproducibility, on-device install) are named instead. Totals corrected to 55 requirements
  (~43 met, ~12 open) and "Current priorities" reordered to what is actually open — the list still
  led with those fixed review items. Added the 2026-07-27 rotation/zoom milestone.
