---
created: "2026-06-30"
last_edited: "2026-07-04"
---

# Cavekit: Input Bridging

## Scope
Translating webOS input commands (delivered over YAP) into DOM input events the
engine understands: pointer/click, keyboard, touch, gestures, and drag-scroll,
with correct coordinate mapping under zoom and scroll. Reference:
`render/goanna/PORT-MAP.md` (input rows), `docs/IPC-CONTRACT.md` (input commands).

## Requirements

### R1: Pointer/click input
**Description:** Click and hold commands hit the right element.
**Acceptance Criteria:**
- [x] `clickAt(x,y,numClicks)` dispatches pointer/mouse events at the content coordinate; the element at that point receives them (verified via a page that reports the event target, or by a navigation that results from clicking a link).
- [x] Click count (single/double) is preserved.
- [x] `holdAt` produces the long-press/context behavior.
- [x] Input handlers do NOT perform navigation or other engine teardown synchronously inside the YAP socket callback; teardown-capable work (hit-test/activation/click/navigation) runs in the page-lifetime-protected pump/tick context. *(review #5 H-1: a synchronous link-navigation from the clickAt callback re-entered and crash-rebooted the device.)*
**Dependencies:** cavekit-offscreen-rendering.md (R5)

### R2: Keyboard input
**Description:** Key commands produce correct DOM keyboard events and text entry.
**Acceptance Criteria:**
- [x] `keyDown`/`keyUp` with a webOS key code, modifiers, and character produce DOM key events with the correct key and modifier state.
- [ ] Typing into a focused text field inserts the expected characters.
- [ ] `insertStringAtCursor` inserts text at the caret.
**Dependencies:** none

### R3: Touch and gesture input
**Description:** Touch and gesture commands map to DOM touch/zoom behavior.
**Acceptance Criteria:**
- [ ] `touchEvent` (with the touches payload) produces DOM touch events with matching touch points.
- [ ] A pinch gesture changes page zoom; a tap maps to a click.
- [ ] Multi-touch point count and coordinates are preserved.
**Dependencies:** cavekit-offscreen-rendering.md (R5)

### R4: Drag and flick scrolling
**Description:** Drag commands scroll content and overflow regions.
**Acceptance Criteria:**
- [x] `dragStart`/`dragProcess`/`dragEnd` scroll the page and CSS-overflow elements.
- [x] Scrolling emits scrolled-to updates consistent with the rendered output.
**Dependencies:** cavekit-offscreen-rendering.md (R4)

### R5: Coordinate mapping under zoom/scroll
**Description:** Content coordinates resolve to the correct page location regardless of transform.
**Acceptance Criteria:**
- [x] With the page zoomed and scrolled, a click at a given content coordinate hits the element actually shown at that location.
- [x] Mapping is consistent between input and the geometry the renderer reports.
**Dependencies:** cavekit-offscreen-rendering.md (R5)

## Out of Scope
- Producing frames (cavekit-offscreen-rendering.md).
- IME/virtual-keyboard UI (webOS sysmgr concern; only the resulting key/text commands are handled here).

## Cross-References
- See also: cavekit-offscreen-rendering.md, cavekit-navigation-events.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-07-04: Reconciled — R1 click/hold, R4 drag-scroll, R5 coord-mapping-under-zoom all verified (INPUT/INPUT2/COORDMAP PASS); R2 keyDown/keyUp verified. Pending on-device: R2 field typing + insertStringAtCursor, all of R3 touch/pinch/tap (offscreen widget does not route synth touch on desktop).
