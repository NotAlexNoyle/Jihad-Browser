---
created: "2026-06-30"
last_edited: "2026-07-31"
---

# Cavekit: Offscreen Rendering

## Scope
Rendering page content headlessly into the shared-memory framebuffer the
BrowserAdapter expects, and emitting the paint and geometry messages that drive
the client's display. This is the load-bearing, highest-risk part of the port.
Reference: `docs/IPC-CONTRACT.md` (framebuffer model), `render/goanna/PORT-MAP.md`
(paint loop).

## Requirements

### R1: Headless, windowless rendering surface
**Description:** Page content renders without any on-screen native window.
**Acceptance Criteria:**
- [x] A page can be rendered with no native window created or shown.
- [x] The render surface size tracks the page/window size.
**Dependencies:** cavekit-engine-embedding.md (R2)

### R2: Frames delivered in the contract pixel format
**Description:** Rendered output is written into the shared buffer in the exact format/stride the adapter expects.
**Acceptance Criteria:**
- [x] After loading a known page, the shared buffer holds a non-blank image of the correct dimensions.
- [x] Pixel format and stride match the upstream offscreen contract (32-bit), verifiable via `renderToFile` output or a pixel checksum against a reference.
- [x] Colors/coordinates are correct (no channel swap, no vertical flip).
**Dependencies:** cavekit-ipc-contract.md (R2)

### R3: Paint notification protocol honored
**Description:** Paint-ready notifications follow the double-buffer discipline.
**Acceptance Criteria:**
- [x] Each completed frame yields exactly one paint-ready notification naming the buffer that was filled.
- [x] A buffer is not reused for the next frame until the client returns it.
- [x] Invalidations are coalesced so transient changes do not produce redundant full repaints.
- [x] **A content change produces a repaint without user input.** *(FIXED: UXP patch 0012 adds a sticky PuppetWidget::Invalidate() dirty flag drained by `jihad_offscreen_take_dirty`; the daemon polls it each tick after PumpFor and repaints on dirty — the engine-driven frame delivery the stock QtWebKit server got from Qt paint events; plus progress-increase→repaint during loads. Verified on device 2026-07-19: example.com renders with zero input, alternating double-buffer `painted` frames in the log. The earlier "old page sticks around / overload" that masked this was the daemon crash — fixed separately (see cavekit-navigation-events R6). See context/impl/device-test-2026-07-19.md.)*
**Dependencies:** cavekit-ipc-contract.md (R2), cavekit-engine-embedding.md (R3)

### R4: Geometry and viewport events emitted
**Description:** The renderer reports size, scroll, and viewport changes.
**Acceptance Criteria:**
- [x] Content-size changes emit a contents-size-changed message.
- [x] Scroll position changes emit a scrolled-to message.
- [x] Parsing a viewport meta tag emits a meta-viewport message with the parsed scale/size values.
**Dependencies:** none (emitted by the renderer; consumed alongside cavekit-navigation-events.md — see Cross-References)

### R5: Resize, zoom, and scroll commands affect output
**Description:** Surface/viewport commands change what is rendered.
**Acceptance Criteria:**
- [x] `setWindowSize`/`setVirtualWindowSize` resize the surface and content viewport respectively.
- [x] `setScrollPosition` and `scrollLayer` move the rendered content.
- [x] `setZoomAndScroll` changes rendered scale and position; subsequent input maps to the new transform. *(The 2026-07-04 desktop ZOOM PASS rested on `SetFullZoom`, which `../impl/zoom-fix-2026-07-27.md` later proved does NOT magnify an offscreen `RenderDocument` capture — its internal AppUnitsPerDevPixel/AppUnitsPerCSSPixel scale cancels the engine zoom, so on device the content shrank into a `1/Z` quadrant ("things get cut off"). REBUILT + device-verified 2026-07-27 (commit 8d7865c): `JihadRenderDocument` pre-scales the gfxContext by Z and renders an absolute document rect (`RENDER_DOCUMENT_RELATIVE`), leaving the layout viewport at device width (no reflow → cannot re-arm the landscape fit-zoom loop → R6 stays safe). Verified on device via the daemon inject channel + frame.ppm: `zoom 3.0` magnifies 3×, horizontal and vertical pan reach the page edges/bottom, `zoom 1.0` unregressed. Input consistency under the new transform is cavekit-input-bridging.md R5 (also device-verified, commit d4f0842).)*
**Dependencies:** none (input coordinate mapping consumes this transform — see cavekit-input-bridging.md R5 and Cross-References)

### R6: Orientation-correct on-device composite (portrait ↔ landscape)
**Description:** When the card rotates between portrait and landscape, the composited
on-screen frame stays correct — no shear, tiling, scanlines, or blank page. The daemon
already re-renders at the new viewport (R5); this requirement covers the BrowserAdapter's
composite of the shared buffer to the rotated card.
**Acceptance Criteria:**
- [x] The adapter composites the offscreen through the WebKit-provided **PGContext**
  (Piranha graphics context), which carries the card's rotation/scale transform, rather
  than a raw row-major `dstBuffer` blit that is fixed to the card's logical orientation.
  *(Enabled via `AdapterBase(..., useGraphicsContext=true)`; the raw dstBuffer blit remains
  a no-context fallback for desktop/Ubuntu builds.)*
- [x] After a portrait→landscape (and landscape→portrait) rotate, the page renders
  filling the card with correct geometry — verified by eye on the TouchPad. *(CONFIRMED ON
  DEVICE 2026-07-27, commit 8d7865c: "correct in both orientations, superseding the white-frame
  guard … Verified on-device"; `../impl/impl-overview.md` 2026-07-27 opens "Rotation confirmed
  working on device" and `../impl/zoom-fix-2026-07-27.md` dates its next user report "after
  rotation was confirmed working". This box was checked on 2026-07-26 while the check was still
  device-gated — the note said so; the 2026-07-27 evidence is what actually backs it. The
  confirmation is a device/visual confirm recorded in the commit + impl entries; no per-orientation
  capture was archived. The old white-frame "rotation guard" band-aid is removed by this path.)*
- [x] The load-time symbols `PGContext::bitblt` and `PGSurface::wrap` resolve against the device
  `libWebKitLuna.so` (the link is intentionally not `--no-undefined`); `releaseRef`/`addRef` are
  inherited INLINE from the `PGShared` base (no exported symbol — modeling them as PGSurface
  members would emit an unresolvable UND that blocks the RTLD_NOW load). Verifiable off-device by
  demangling the adapter's UND symbols (exactly two PG entries), and on-device by `dlopen_probe`.
  *(Off-device demangle done 2026-07-26 (`../impl/rotation-fix-2026-07-26.md` "Verification": exactly
  the two PG UNDs, no vtable/typeinfo orphans, `releaseRef` resolved inline via the modeled
  `PGShared` base — Codex F1). On-device resolution recorded 2026-07-27, commit 8d7865c: "the two PG
  symbols resolve against libWebKitLuna" — necessarily so, since the PG-composited adapter loaded and
  rendered the rotate confirmed above.)*
**Dependencies:** cavekit-offscreen-rendering.md (R5), cavekit-device-build.md
**Reference solution:** Atlas Browser (Herrie82) BrowserAdapter graphics-context paint path
(Apache-2.0; see NOTICE). Ported to Goanna's viewport-sized buffer contract (renderedX/Y =
adapter scroll, contentZoom = engine zoom).

## Out of Scope
- Synthesizing input (cavekit-input-bridging.md).
- GPU/WebGL/video compositor path — first target is CPU/basic-layers readback; accelerated paths are deferred (note in impl, revisit in Phase 3).

## Cross-References
- See also: cavekit-ipc-contract.md, cavekit-engine-embedding.md, cavekit-input-bridging.md, cavekit-navigation-events.md

## Changelog
- 2026-07-31: Reconciliation against recorded evidence. R6's second AC carried a "(Device-gated
  visual check)" note while already checked — the gate closed on 2026-07-27 (commit 8d7865c
  "Verified on-device"; `../impl/impl-overview.md` 2026-07-27 "Rotation confirmed working on
  device"), so the note now cites that instead of claiming a pending check. R6's third AC note
  now separates the off-device demangle (2026-07-26) from the on-device symbol resolution
  (2026-07-27). R5's zoom AC gained the correction that the desktop `SetFullZoom` PASS did not
  hold offscreen and was rebuilt + device-verified 2026-07-27 (`../impl/zoom-fix-2026-07-27.md`).
  No box changed state.
- 2026-07-26: Added R6 (orientation-correct on-device composite). Root cause of the
  portrait↔landscape render-break traced to the adapter's raw dstBuffer blit ignoring the
  card rotation transform; fixed by compositing through the PGContext (Atlas reference).
- 2026-06-30: Initial draft.
- 2026-07-04: Status reconciled to implementation — all R1–R5 verified on desktop AND on-device (HP TouchPad): true windowless PuppetWidget render (now via MOZ_WIDGET_TOOLKIT=headless — no gtk window at all), ARGB32 correct, msgPainted double-buffer, geometry/viewport events, resize/zoom/scroll.
