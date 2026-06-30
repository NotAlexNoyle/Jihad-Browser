---
created: "2026-06-30"
last_edited: "2026-06-30"
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
- [ ] `clickAt(x,y,numClicks)` dispatches pointer/mouse events at the content coordinate; the element at that point receives them (verified via a page that reports the event target, or by a navigation that results from clicking a link).
- [ ] Click count (single/double) is preserved.
- [ ] `holdAt` produces the long-press/context behavior.
**Dependencies:** cavekit-offscreen-rendering.md (R5)

### R2: Keyboard input
**Description:** Key commands produce correct DOM keyboard events and text entry.
**Acceptance Criteria:**
- [ ] `keyDown`/`keyUp` with a webOS key code, modifiers, and character produce DOM key events with the correct key and modifier state.
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
- [ ] `dragStart`/`dragProcess`/`dragEnd` scroll the page and CSS-overflow elements.
- [ ] Scrolling emits scrolled-to updates consistent with the rendered output.
**Dependencies:** cavekit-offscreen-rendering.md (R4)

### R5: Coordinate mapping under zoom/scroll
**Description:** Content coordinates resolve to the correct page location regardless of transform.
**Acceptance Criteria:**
- [ ] With the page zoomed and scrolled, a click at a given content coordinate hits the element actually shown at that location.
- [ ] Mapping is consistent between input and the geometry the renderer reports.
**Dependencies:** cavekit-offscreen-rendering.md (R5)

## Out of Scope
- Producing frames (cavekit-offscreen-rendering.md).
- IME/virtual-keyboard UI (webOS sysmgr concern; only the resulting key/text commands are handled here).

## Cross-References
- See also: cavekit-offscreen-rendering.md, cavekit-navigation-events.md

## Changelog
- 2026-06-30: Initial draft.
