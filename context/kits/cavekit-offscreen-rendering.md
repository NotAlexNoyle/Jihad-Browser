---
created: "2026-06-30"
last_edited: "2026-06-30"
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
- [ ] A page can be rendered with no native window created or shown.
- [ ] The render surface size tracks the page/window size.
**Dependencies:** cavekit-engine-embedding.md (R2)

### R2: Frames delivered in the contract pixel format
**Description:** Rendered output is written into the shared buffer in the exact format/stride the adapter expects.
**Acceptance Criteria:**
- [ ] After loading a known page, the shared buffer holds a non-blank image of the correct dimensions.
- [ ] Pixel format and stride match the upstream offscreen contract (32-bit), verifiable via `renderToFile` output or a pixel checksum against a reference.
- [ ] Colors/coordinates are correct (no channel swap, no vertical flip).
**Dependencies:** cavekit-ipc-contract.md (R2)

### R3: Paint notification protocol honored
**Description:** Paint-ready notifications follow the double-buffer discipline.
**Acceptance Criteria:**
- [ ] Each completed frame yields exactly one paint-ready notification naming the buffer that was filled.
- [ ] A buffer is not reused for the next frame until the client returns it.
- [ ] Invalidations are coalesced so transient changes do not produce redundant full repaints.
**Dependencies:** cavekit-ipc-contract.md (R2), cavekit-engine-embedding.md (R3)

### R4: Geometry and viewport events emitted
**Description:** The renderer reports size, scroll, and viewport changes.
**Acceptance Criteria:**
- [ ] Content-size changes emit a contents-size-changed message.
- [ ] Scroll position changes emit a scrolled-to message.
- [ ] Parsing a viewport meta tag emits a meta-viewport message with the parsed scale/size values.
**Dependencies:** none (emitted by the renderer; consumed alongside cavekit-navigation-events.md — see Cross-References)

### R5: Resize, zoom, and scroll commands affect output
**Description:** Surface/viewport commands change what is rendered.
**Acceptance Criteria:**
- [ ] `setWindowSize`/`setVirtualWindowSize` resize the surface and content viewport respectively.
- [ ] `setScrollPosition` and `scrollLayer` move the rendered content.
- [ ] `setZoomAndScroll` changes rendered scale and position; subsequent input maps to the new transform.
**Dependencies:** none (input coordinate mapping consumes this transform — see cavekit-input-bridging.md R5 and Cross-References)

## Out of Scope
- Synthesizing input (cavekit-input-bridging.md).
- GPU/WebGL/video compositor path — first target is CPU/basic-layers readback; accelerated paths are deferred (note in impl, revisit in Phase 3).

## Cross-References
- See also: cavekit-ipc-contract.md, cavekit-engine-embedding.md, cavekit-input-bridging.md, cavekit-navigation-events.md

## Changelog
- 2026-06-30: Initial draft.
