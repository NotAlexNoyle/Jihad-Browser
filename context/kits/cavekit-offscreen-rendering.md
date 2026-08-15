---
created: "2026-06-30"
last_edited: "2026-08-03"
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

### R7: Engine popups reach the user
**Description:** A popup the engine opens (XUL `<menupopup>`: the `about:addons` tools menu,
context menus; and the native `<select>` combobox) is a **separate display root** —
`nsLayoutUtils::GetDisplayRootFrame` returns the popup frame itself — so the main-document
render never contains it. Every such popup must still reach the user somehow.
**Added 2026-08-03**, splitting a cross-cutting defect out of the UI kits: the diagnosis and both
solution shapes are shared, only the presentation differs per popup type.
**Acceptance Criteria:**
- [x] **`<select>` comboboxes: solved by handing them to the card.** The engine cannot paint the
  combobox, so the daemon serializes the options and emits the inherited `msgPopupMenuShow`
  contract; the card renders the list and replies with the index. This is also what isis and Atlas
  do — neither ever rendered a `<select>` dropdown in the engine. Device-verified on all three
  variants 2026-08-03 (`../impl/impl-select-popup-2026-08-03.md`); the serialized shape is the
  isis one (`items[].text` / `items[].isEnabled` + `selectedIdx`), plus a Jihad-additive `rect`
  so the card can anchor the list under the tapped control.
- [x] **XUL `<menupopup>`: composited.** After the main paint the daemon enumerates
  `nsXULPopupManager::GetInstance()->GetVisiblePopups()` and draws each into the shared buffer at
  its own offset, clipped to its box, innermost last, enforcing the alpha==255 invariant the
  adapter's raw blit needs; a page with no popup pays one query.
  **The 2026-08-02 diagnosis that a popup "is created 0x0 and never shown" was wrong**: it was
  measured on a plain `<select>`, which is the COMBOBOX path and never enters the popup manager.
  A real `<menupopup>` sizes and shows itself correctly (594,52 347x117 on desktop), so no
  size-and-show fix was needed — only the compositing, and then the input routing below.
- [x] The `about:addons` tools menu opens, is readable, can be dismissed, and its items ACTIVATE.
  Desktop-verified end to end: all items, separators and the checkmark render; a tap elsewhere
  dismisses; clicking "Update Add-ons Automatically" drives the real XUL command path (on reopen
  the checkmark is gone and the last item reads "…to Update Manually"). Device-verified: the menu
  opens and composites over the page, and a double-delivered tap no longer closes it.
  Page context menus are NOT this path — a long-press is reported to the CARD over the frozen
  contract (isis model), so the engine never opens a context popup here.
- [x] An open popup is INTERACTIVE, not just visible. It is a separate display root, so the
  content document's hit-testing cannot see it: a tap on the tools menu's first row resolved to
  the `<vbox>` underneath. Taps, moves and drags are routed into the popup, the row under the
  finger highlights (an AGENT sheet draws it — this build has no native theme to paint
  `_moz-menuactive`), and a drag that ends over the menu picks the highlighted row.
**Dependencies:** R1, R3, cavekit-input-bridging.md, cavekit-addons-extensions.md (R2)
**Iteration loop:** `build/desktop/build-menupopup-probe.sh` — it drives **about:addons**, because
XUL parsing is chrome-only and a `file://` .xul document is refused outright, and because
`build-popup-probe.sh`'s plain `<select>` exercises the combobox path instead of the popup manager
(that conflation is what produced the wrong 2026-08-02 diagnosis). The desktop dist needs the
`chrome://branding/` package or about:addons parses to a `<parsererror>` and every probe silently
tests an error page — `build-goanna.sh` now installs it, as the device bundle already did.

### R8: The damage-only repaint may never publish a frame it cannot prove

**Description:** A whole-viewport software render costs ~38 ms on this device; the damage box of an
animating plugin costs ~2 ms, so damage-only repaint is what makes animated content viable at all
(cavekit-addons-extensions.md R7). It is also the single easiest way to publish a WRONG frame,
because it copies forward a buffer it did not write this tick. Every one of its premises must be
proven per buffer, per frame — geometry alone is not enough, because the common failures leave
width, height, zoom and the adapter's scroll position all identical while invalidating the mapping.

**Acceptance Criteria:**
- [x] A buffer is eligible as a partial-paint base only if it provably holds a COMPLETE, NON-BLANK
      frame of the current geometry. *(2026-08-10. `mBufFull[fs].valid` was recorded BEFORE the
      blank-over-good suppression and before the active-buffer flip, so a frame rejected as blank
      still advertised its slot as complete; the next tick then blitted one live rectangle into that
      white buffer and published it — laundering the exact white flash the suppression exists to
      prevent. The record now happens past every early return, on the path that will definitely
      publish, and is `nb > 0` rather than `nb >= 0`. Both non-publishing returns explicitly drop the
      record via `invalidateBufRecord()`.)*
- [x] An ENGINE-initiated scroll forces one full frame. *(2026-08-10. `mDamageForceFull` existed,
      was read once, and was NEVER assigned true anywhere in the tree — the protection its own
      comment described did not exist. Adapter-driven scroll/zoom/resize/nav were covered by
      accident, because they change `mAdapterScrollX/Y` or drop the buffer records; an anchor jump,
      `window.scrollTo`, scroll-into-view on focus, find-in-page, or the 100-300 ms convergence on
      the adapter's target change NONE of those, while the damage box from PuppetWidget is relative
      to the NEW scroll. With a plugin animating, damage is present on essentially every tick, so
      the partial branch would win every tick and the card would composite pre-scroll rows
      indefinitely with the plugin drawn at the wrong offset. **The check must sit ABOVE the
      zoom guard in `emitScrollIfChanged`** — that guard suppresses the adapter ECHO while zoomed,
      and fit-zoom ~0.78 is the normal state for a desktop-width page, so gating on it left the bug
      live in the common case (measured: a `window.scrollTo` at fit-zoom never reached the flag).
      It reads the scroll with `flushLayout=false` and its own last-seen pair: the 2-arg default
      flushes layout, and a layout flush from the tick reflows the plugin frame, reallocates its
      async surface under the child and kills it.)*
- [x] `thaw()` invalidates both buffer records. *(2026-08-10. Its own comment said "the buffers may
      be new segments" and it called `detachShm()`, but left `mBufFull[0]/[1]` valid — so the first
      paint after a card returns to the foreground passed the eligibility test and rendered only the
      damage box into a segment we had never written, publishing whatever its previous owner left
      there. With a plugin animating this was the guaranteed first frame, not a rare one.)*
- [x] `bandRow` is only recorded when the tall-region render actually ran. *(2026-08-10. The
      fallback path reads the frame to buffer row 0 while `bandRow` still holds the tall-region
      value, which would offset every later damage rect by it.)*
- [x] The partial path's own output is bounded the way the full path's is — it publishes with no
      non-blank count and no blank check of any kind, so anything that corrupts a partial frame is
      invisible to the daemon. Decide whether that is acceptable or add a cheap guard.
      *(**2026-08-10 — analysed, and the obvious guard is the WRONG one. Do not ship it.** The
      full path's guard is a non-blank pixel count over the visible band, and it is sound there
      because a whole viewport that renders to nothing is always a failure
      (`BrowserPageGoanna.cpp:2422/2428/2444` — `nb < 0`, `nb == 0 && region && mHadContent`,
      `nb == 0 && mHadContent`). Copying that test onto the damage rect
      (`RenderRegion`, `:2196-2199`) inverts its meaning: a SMALL rect that is legitimately
      all-white is completely ordinary — a caret blinking off, text being erased, a white area
      scrolling in — and the full path's own test treats `p[0]>240 && p[1]>240 && p[2]>240` as
      blank. A per-damage-rect blank check would therefore suppress a large class of correct
      frames, and on a mostly-white page it would suppress most of them. That is a worse defect
      than the one this criterion is about.*
      *What the partial path DOES already have is `RenderRegion`'s boolean return, which is a
      "did the render run" signal but not a "did it produce sane pixels" one.*
      *So this criterion needs a guard whose failure mode is not "white content", and the two
      candidates worth measuring are (a) a checksum/／sentinel over the destination rect before and
      after the render — corruption changes it, legitimate white does too, so this only detects a
      render that wrote NOTHING, and (b) bounding the rect against `renderedWidth/Height` and the
      segment size, which catches the geometry class of corruption without looking at pixels at
      all. (b) is cheap, has no false positives, and is probably the whole answer; (a) is nearly
      free given `RenderRegion` already returns bool. Kept `[~]` rather than closed because
      neither is implemented yet.)*
      *(**2026-08-10, SECOND PASS — DECIDED: no blank check, and the path is already bounded. This
      criterion offers two ways to close and this takes the first one, "justify that in writing".**
      Reading the partial branch properly (`BrowserPageGoanna.cpp:2173-2199`) changes the picture
      the criterion was written from — it is not unguarded, it is guarded by construction rather
      than by a pixel test:*
      1. ***The damage box is CLAMPED, not trusted.*** *`:2184-2190` clips `dLeft/dTop/dW/dH` into
         the visible band on all four sides, and `worthIt` (`:2193`) additionally requires
         `dW > 0 && dH > 0`. The destination write `pixels + (dTop*w + dLeft)*4` with stride `w*4`
         and `dH` rows therefore cannot leave the band, and the band itself is capped by `maxRows`
         from the real segment size. The geometry class of corruption — the one that actually
         crashes or tears — has no way in.*
      2. ***It only runs on an EXACT buffer-state match.*** `:2175-2179` *requires the cached full
         frame to match on `w`, `h`, `zeff`, `scrollX` AND `scrollY`; anything else falls through
         to the full path. So a partial paint cannot land on a buffer whose geometry moved.*
      3. ***`RenderRegion` returns bool*** *(`:2196`) and a false return skips the publish entirely,
         which covers "the render did not run".*
      4. ***And the decisive one: a partial paint only ever rewrites a SUB-RECT of a frame that
         already passed the full path's non-blank check.*** *The surrounding pixels are the last
         full paint. So the failure this criterion imagines — "publishes a corrupt frame invisibly"
         — cannot produce a blank or wholly-corrupt card frame; the worst case is a stale-or-wrong
         rectangle inside an otherwise valid one, bounded by `0.6 * w * h`.*
      *The residual risk is `RenderRegion` returning true having written wrong pixels, and there is
      no cheap detector for that which does not fire on legitimate white (see the previous note).
      `JIHAD_NO_PARTIAL_PAINT=1` remains the kill switch if a real corruption is ever observed.
      Closing `[x]` on the written justification, NOT on an added guard.)*

**Dependencies:** R1, R2, cavekit-addons-extensions.md (R7)
**Kill switch:** `JIHAD_NO_PARTIAL_PAINT=1` disables the damage-only branch. NOTE it gates exactly
one block — it is NOT a general "disable this session's frame work" switch, and a 2026-08-09 bisect
that assumed otherwise reached a right conclusion for the wrong reason.

## Out of Scope
- Synthesizing input (cavekit-input-bridging.md).
- GPU/WebGL/video compositor path — first target is CPU/basic-layers readback; accelerated paths are deferred (note in impl, revisit in Phase 3).

## Cross-References
- See also: cavekit-ipc-contract.md, cavekit-engine-embedding.md, cavekit-input-bridging.md, cavekit-navigation-events.md

## Changelog
- 2026-08-03: Added **R7** (engine popups reach the user), which collects the "popup is a separate
  display root" defect that had been tracked only in impl notes and referenced loosely from the UI
  kits. Its `<select>` AC is met on all three variants (card-side list, isis JSON shape, anchored
  from a daemon-supplied rect); the `<menupopup>` overlay-composite ACs are diagnosed and open,
  and are the project's current top priority.
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
