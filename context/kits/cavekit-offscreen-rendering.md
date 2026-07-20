---
created: "2026-06-30"
last_edited: "2026-07-04"
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
- [x] `setZoomAndScroll` changes rendered scale and position; subsequent input maps to the new transform.
**Dependencies:** none (input coordinate mapping consumes this transform — see cavekit-input-bridging.md R5 and Cross-References)

## Out of Scope
- Synthesizing input (cavekit-input-bridging.md).
- GPU/WebGL/video compositor path — first target is CPU/basic-layers readback; accelerated paths are deferred (note in impl, revisit in Phase 3).

## Cross-References
- See also: cavekit-ipc-contract.md, cavekit-engine-embedding.md, cavekit-input-bridging.md, cavekit-navigation-events.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-07-04: Status reconciled to implementation — all R1–R5 verified on desktop AND on-device (HP TouchPad): true windowless PuppetWidget render (now via MOZ_WIDGET_TOOLKIT=headless — no gtk window at all), ARGB32 correct, msgPainted double-buffer, geometry/viewport events, resize/zoom/scroll.
