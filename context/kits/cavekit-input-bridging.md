---
created: "2026-06-30"
last_edited: "2026-07-15"
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
- [x] Typing into a focused text field inserts the expected characters (letters, digits, symbols, and non-ASCII), at the caret, with no per-keystroke navigation flash.
- [x] `insertStringAtCursor` inserts text at the caret.
- [x] A visible text caret is shown for the focused editable, and typing/editing happens at it (not append-only).
- [ ] Tapping an editable raises the VKB reliably on EVERY page in a session, and focusing a field does not scroll the page content off-screen. *(REPORTED broken on-device 2026-07-17 — T1: on google, focusing the search box pushes the whole page up off the screen (the F-247 one-shot scroll-restore doesn't hold once the VKB resize/reflow lands). T4: after visiting google + ddg in one card, tapping the html.duckduckgo.com field raises no keyboard until the browser restarts — editorFocused state desync across navigations. See context/impl/device-test-2026-07-17.md.)*
**Dependencies:** none

### R2a: Full editable navigation and editing
**Description:** The on-screen-keyboard editing keys drive the focused `<input>`/`<textarea>` correctly.
**Acceptance Criteria:**
- [x] Backspace deletes the code point before the caret; forward-Delete the one after; both are surrogate-pair aware.
- [x] Left/Right move the caret by a full code point; Home/End jump to the field start/end; Up/Down move by line in a `<textarea>` (and degrade to Home/End in a single-line input) without landing between a surrogate pair.
- [x] Typing or deleting while a range is selected replaces/removes the selection (e.g. an `onfocus=this.select()` search box).
- [ ] Enter inserts a newline in a `<textarea>` and submits the form for a single-line `<input>`. Tab inserts a tab character in a `<textarea>` but moves focus to the next/prev text field in a single-line `<input>` (Shift+Tab goes back) — inserting a control char into a search/login value is wrong. *(REPORTED broken on-device 2026-07-17, T2: on google/duckduckgo full sites Enter shows the loading overlay and the search never visibly executes. May be the stale-frame paint root (offscreen-rendering R3) rather than the submit path itself. See context/impl/device-test-2026-07-17.md.)*
- [x] Enter submission runs the form's constraint validation first: an INVALID form is not submitted and keeps focus + the keyboard so the user can correct it (never discards the edit target before knowing submission succeeded). *(2026-07 Codex F-290.)*
- [x] When a page focus/blur handler moves focus off the tapped/next field to a NON-text control (button/checkbox), the keystroke target is cleared rather than left on the old field — typing must never silently mutate a field that is no longer focused. Retargeting only follows focus to another text control. *(2026-07 Codex F-270/F-292.)*
- [x] After a programmatic value edit, a bubbling DOM `input` event is dispatched (from the guarded loop) so controlled/framework (React) inputs keep the edit instead of reverting it.
- [x] A held Backspace accelerates to whole-word deletion after it auto-repeats for a short interval.
- [x] `<input type=number>`/`email` etc. (no text-selection API) still accept typed characters.
- [x] Focus/blur handlers that navigate or submit run in the page-lifetime-protected pump/tick context, never synchronously in the YAP key callback (same crash class as R1).
- [x] No keystroke, character, or field value is ever written to the daemon log (F-163).
**Dependencies:** cavekit-offscreen-rendering.md (RENDER_CARET — a visible caret requires the offscreen render to draw it)

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
- [~] With the page zoomed and scrolled, a click at a given content coordinate hits the element actually shown at that location. *(REPORTED ISSUE 2026-07-15: on a real site — example.com — a tap on the sole link hit-tested to `<HTML>` (missed), while a tap on duckduckgo's search box did hit the `<input>`. Under investigation: whether the adapter-vs-daemon zoom disagree, or a render/blit vertical offset makes small targets miss. `ElementFromPoint` receives content coords directly; `m_headerHeight` is 0 in both this adapter and the reference isis adapter, so the header offset is not the divergence.)*
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
- 2026-07-15: R2 field typing DONE + verified on-device — the engine editor works headless after UXP patch 0010, so typing runs through the real engine caret (SetValue + SetSelectionRange), not the old append-only content-attribute path. Added R2a (full editable navigation/editing): arrows, Home/End, Up/Down, forward-Delete, selection-replace, Enter (newline in textarea / submit in input, deferred to pump per R1's crash rule), Tab-inserts-tab, held-Backspace word acceleration, `<input type=number>` fallback, surrogate-pair safety — all verified on-device via the VKB, reviewed by codex gpt-5.6-sol (F-219..F-225) then the cavekit inspector while codex is rate-limited. Visible caret required a UXP change (patch 0011: offscreen RenderDocument must pass `nsIPresShell::RENDER_CARET`). webOS VKB arrow keycodes measured on-device: Down=0xE0A0, Up=0xE0A1, Left=0xE0A2, Right=0xE0A3 (a private-use block, not Apple/Qt ranges). R5 has a REPORTED link-hit-test miss on real sites (see R5) — open.
- 2026-07-17: R2a hardened (Codex gpt-5.6-sol F-264..F-292): Enter submission is constraint-validation-gated (invalid form keeps focus + VKB, F-290); the keystroke target is cleared when a focus handler moves focus to a non-text control and only follows focus to another text control (F-270/F-292); the pending `input` event flushes before a queued tap so a typed edit is observed before a submit/link click (F-291); the deferred `input` event targets the actually-edited element (F-266/F-267).
