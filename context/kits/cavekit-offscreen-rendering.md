---
created: "2026-06-30"
last_edited: "2026-11-07"
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
- [x] Invalidations are coalesced so transient changes do not produce redundant full repaints. *(Downgraded and re-closed 2026-10-01 on webOS CE 3.1.0: `about:blank` was repainted ~22 times a second forever (663 paints in 30 s); fixed, 0 paints in 30 s on a settled blank page — R8's last criterion.)*
- [x] **A content change produces a repaint without user input.** *(UXP patch 0012 adds a sticky `PuppetWidget::Invalidate()` dirty flag drained by `jihad_offscreen_take_dirty`; the daemon polls it each tick after PumpFor and repaints on dirty, and a progress increase repaints during loads. Device 2026-07-19: example.com renders with zero input, alternating double-buffer `painted` frames. context/impl/device-test-2026-07-19.md.)*
**Dependencies:** cavekit-ipc-contract.md (R2), cavekit-engine-embedding.md (R3)

### R4: Geometry and viewport events emitted
**Description:** The renderer reports size, scroll, and viewport changes.
**Acceptance Criteria:**
- [x] Content-size changes emit a contents-size-changed message. *(2026-10-01, CE: `size 600 500` → `emitGeometry contentSize=600x500 win=600x500`, frame painted 600 wide, and back. Colour/orientation, ticker and engine-scroll results of the same sweep: cavekit-device-build.md R4, offscreen sweep box.)*
- [x] Scroll position changes emit a scrolled-to message.
- [x] Parsing a viewport meta tag emits a meta-viewport message with the parsed scale/size values. *(2026-10-01, CE: it was also sent for pages with no viewport tag, with the engine's clamped "unset" scale 0.1, and the Mojo WebView drew those pages as a 76x99 thumbnail. Now sent only when the document declares a viewport and the value changes; a tag with a width but no initial scale reports "fit that width". After: Mojo fills the card width (`../impl/evidence-2026-10-01/mojo-no-viewport-page-after-fix.jpg`), Enyo unchanged (980 layout at 0.7837).)*
**Dependencies:** none (emitted by the renderer; consumed alongside cavekit-navigation-events.md — see Cross-References)

### R5: Resize, zoom, and scroll commands affect output
**Description:** Surface/viewport commands change what is rendered.
**Acceptance Criteria:**
- [x] `setWindowSize`/`setVirtualWindowSize` resize the surface and content viewport respectively.
- [x] `setScrollPosition` and `scrollLayer` move the rendered content. *(2026-10-01, CE: `ScrollTo` sets `scrollLeft/scrollTop` on the viewport's scrolling element (the root, or `<body>` in quirks mode) instead of running a `javascript:` URL, so it works on chrome pages and with script disabled. Device: standards page, quirks page and `javascript.enabled=false` — `scroll 0 1100` moved a link from y=1500 to y=400 and a tap at its position navigated; `about:preferences` scrolled and a button at document y=1335 was hit.)*
- [x] `setZoomAndScroll` changes rendered scale and position; subsequent input maps to the new transform. *(Rebuilt and device-verified 2026-07-27, commit 8d7865c: `SetFullZoom` does not magnify an offscreen `RenderDocument` capture, so `JihadRenderDocument` pre-scales the gfxContext by Z and renders an absolute document rect (`RENDER_DOCUMENT_RELATIVE`), with the layout viewport left at device width (no reflow, so the landscape fit-zoom loop cannot re-arm). Device via the inject channel + frame.ppm: `zoom 3.0` magnifies 3×, pans reach the page edges, `zoom 1.0` unchanged. `../impl/zoom-fix-2026-07-27.md`; input under the transform: cavekit-input-bridging.md R5.)*
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
  filling the card with correct geometry — verified by eye on the TouchPad. *(Device-confirmed 2026-07-27, commit 8d7865c ("correct in both orientations … Verified on-device"); `../impl/impl-overview.md` 2026-07-27. A visual confirm; no per-orientation capture was archived. Replaces the white-frame rotation guard.)*
- [x] The load-time symbols `PGContext::bitblt` and `PGSurface::wrap` resolve against the device
  `libWebKitLuna.so` (the link is intentionally not `--no-undefined`); `releaseRef`/`addRef` are
  inherited INLINE from the `PGShared` base (no exported symbol — modeling them as PGSurface
  members would emit an unresolvable UND that blocks the RTLD_NOW load). Verifiable off-device by
  demangling the adapter's UND symbols (exactly two PG entries), and on-device by `dlopen_probe`.
  *(Off-device demangle 2026-07-26 (`../impl/rotation-fix-2026-07-26.md` "Verification"): exactly the
  two PG UNDs, no vtable/typeinfo orphans. On device 2026-07-27, commit 8d7865c: both resolve
  against libWebKitLuna.)*
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
- [x] The `about:addons` tools menu opens, is readable, can be dismissed, and its items ACTIVATE. *(2026-10-01, CE, device: two defects that only a fit zoom shows, fixed — the popup path divided an already-converted tap coordinate by the zoom again, so a tap hit another row; and XUL opens the menu asynchronously after the tap's frame, so it did not appear until something forced a frame. Now the gear opens it with no further input (`composited 1 popup(s) over the frame`), Escape closes it (`popups=0`), and "View Recent Updates" closes it and switches the view. Desktop: all items, separators and the checkmark render; "Update Add-ons Automatically" runs the real XUL command path. 2026-10-25, by finger on the GPU path: a menu stayed drawn after its item ran (its pixels were in the frame slots); the daemon now voids every slot when a popup closes (`BrowserPageGoanna.cpp`). `#utils-menu` is styled in addons.css.)*
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
- [x] Damage is applied in the BUFFER's pixels: at any zoom but 1 the engine's rectangle is scaled by the zoom and shifted by the render pan before it is used. *(2026-10-01, CE: damage was used in layout px, right only at zoom 1, and every page without a viewport tag is fit-zoomed (0.78 in portrait). After the conversion the control plugin's box is exact, `render/goanna/test/jihad-zoom-damage.html` ends uniformly in its final colour, and Flash's paint fell from 11 ms to 4 ms per frame (33.3 fps).)*
- [x] A buffer is eligible as a partial-paint base only if it provably holds a COMPLETE, NON-BLANK
      frame of the current geometry. *(2026-08-10: `mBufFull[fs].valid` was
      recorded before the blank-over-good suppression, so a frame rejected as blank still advertised
      its slot and the next partial paint published one rectangle over white. The record now happens
      only on the publishing path, with `nb > 0`; both non-publishing returns call
      `invalidateBufRecord()`.)*
- [x] An ENGINE-initiated scroll forces one full frame. *(2026-08-10: `mDamageForceFull` was never set
      anywhere. An anchor jump, `window.scrollTo`, scroll-into-view, find-in-page or the adapter's
      scroll convergence changes none of what the buffer records check, while the damage box is
      relative to the new scroll, so with a plugin animating the card showed pre-scroll rows. The
      check sits ABOVE the zoom guard in `emitScrollIfChanged` (fit zoom ~0.78 is the normal state)
      and reads the scroll with `flushLayout=false` and its own last-seen pair: a layout flush from
      the tick reflows the plugin frame and kills its async surface.)*
- [x] `thaw()` invalidates both buffer records. *(2026-08-10: `thaw()` called `detachShm()` but
      left `mBufFull[0]/[1]` valid, so the first paint after a card returned rendered only the
      damage box into a segment never written.)*
- [x] `bandRow` is only recorded when the tall-region render actually ran. *(2026-08-10. The
      fallback path reads the frame to buffer row 0 while `bandRow` still holds the tall-region
      value, which would offset every later damage rect by it.)*
- [x] The partial path's own output is bounded the way the full path's is — it publishes with no
      non-blank count and no blank check of any kind, so anything that corrupts a partial frame is
      invisible to the daemon. Decide whether that is acceptable or add a cheap guard.
      *(Decided 2026-08-10: no blank check on the partial path, and it is bounded by construction.
      A small all-white damage rect is ordinary (a caret blinking off, erased text), so the full
      path's non-blank test applied per rect would suppress correct frames. The partial branch in
      `BrowserPageGoanna.cpp`: clamps the damage box into the visible band (and needs `dW > 0 &&
      dH > 0`), the band itself capped by the segment size; runs only when the cached full frame
      matches `w`, `h`, `zeff`, `scrollX` and `scrollY`; skips the publish when `RenderRegion`
      returns false; and only rewrites a sub-rect of a frame that passed the full path's non-blank
      check, so the worst case is a wrong rectangle bounded by `0.6 * w * h`. Kill switch:
      `JIHAD_NO_PARTIAL_PAINT=1`.)*
- [x] **A genuinely blank page SETTLES: a frame the full path will not publish must not be
      retried every tick forever.** *(Met 2026-10-01, webOS CE 3.1.0, daemon
      `3e29f045…`, Enyo card connected, log growth asserted. A blank-over-good frame is transient
      only while the load runs; 500 ms after load-done a persisting blank is published once, and a
      still-refused blank is retried on a 100 ms grid instead of every tick. (1) `about:blank`
      (an allowlisted home page): settled at 558 ms, then 0 paints in 30 s (was 663, ~2.4 KB/s of
      log). (2) blank → real page: first non-blank paint 0.78 s after the navigation. (3) a page
      whose body arrives 2.5 s after its headers, loaded over a good frame: nothing blank
      published, content painted on arrival. (4) a page blank at load that fills from a timer 3 s
      later: settled at 515 ms, then the content painted.)*

**Dependencies:** R1, R2, cavekit-addons-extensions.md (R7)
**Kill switch:** `JIHAD_NO_PARTIAL_PAINT=1` disables the damage-only branch. NOTE it gates exactly
one block — it is NOT a general "disable this session's frame work" switch.

## Out of Scope
- Synthesizing input (cavekit-input-bridging.md).
- GPU/WebGL/video compositor path — first target is CPU/basic-layers readback; accelerated paths are deferred (note in impl, revisit in Phase 3). **Revisited 2026-10-17/18:** the owner's pick 2 + 4 (GPU compositor with readback, then WebGL through EGL) is its own kit, `cavekit-gpu-rendering.md`.

## Cross-References
- See also: cavekit-ipc-contract.md, cavekit-engine-embedding.md, cavekit-input-bridging.md, cavekit-navigation-events.md

### R9: The page stays painted while it is panned and zoomed
**Description:** A pan or a zoom shows the page, not grey. The painted region is as much of the page as the shared segment holds — the whole page when it fits — and keeping it painted must not cost a whole-region render per frame on this CPU.
**Added 2026-10-05 (owner: "pages only load when observed. if I scroll up, down, left, or right, the page is grey for a substantial amount of time while it re renders. the entire page should stay rendered at once unless its very long so that navigation and zooming is smooth").**
**Acceptance Criteria:**
- [x] The region uses the whole segment (4096 rows in portrait, 3072 in landscape); a page that fits is painted whole. The 2048-row cap is gone: a 4096-row region wraps and blits on the device (the texture-limit worry behind the cap did not hold). `JIHAD_REGION_ROWS_CAP` is the way back. *(Device, 2026-10-05.)*
- [x] Frames are COMPOSED: rows the newest frame holds are copied from its buffer (same document, zoom, width, columns), rows it lacks are rendered — at most 768 a frame, the region grown by maintenance frames — the visible band is drawn as before, and at z~1 the copied frame's band is redrawn from the document where it is no longer the band. A frame on a heavy page costs 110-180 ms instead of 300-1400 ms. *(Device, Wikipedia, 2026-10-05.)*
- [x] Off-screen rows that may predate a change are swept a few hundred rows at a time once the page is loaded and still, with a pause between sweeps; rows owed a sweep do not count as covering a pan. A large change at the same scroll repaints the band only. *(Device, 2026-10-05: sweep frames 40-250 ms; the step adapts.)*
- [x] Every row of a composed frame holds what the page has there: on a colour-coded 16000-row page with a fixed header, after jumps by anchor links, dumped whole frames (`<state>/dump-request`) show no wrong row, the header only at the top of the visible band, and no stray header. *(Device, 2026-10-05, `render/goanna/test` has no checker yet — the script lived in the session's scratch; see the loop log.)*
- [x] Zoomed in, the region is as wide as the content up to two viewports, centred on the view, so a sideways pan has something to show; the adapter's rotation guard compares the viewport width (`bufferWidth`) and not the region's. Rows shrink in proportion. *(Device 2026-10-06, through the adapter's pinch entry points (`gestureStart/Change/End`, hook step `g`): Wikipedia at zoom 1.2434 paints a 1388x2266 region and the card composites it; a pinch back to 0.8447 gives 943x3335 with sweeps. Fixed on the way: a one-pixel rounding made the wide region drop at the content's right edge; the content is now at least as wide as the band.)*
- [ ] A real pan and a real pinch, judged by eye: scrolling fast through a long page, zooming in and panning sideways. Script-made pointer events do not reach the plugin and the adapter's flick entry needs a drag in progress, so this is the owner's. [human at the device]
- [x] Wide (zoomed-in) regions on the device. *(2026-10-06: through the pinch hook, see criterion 5 (`setZoomAndScroll zoom=1.2434 (was 0.6882) scroll=658,686`). A sideways pan has no script: scrolling is the adapter's drag.)*
**Dependencies:** cavekit-ipc-contract.md (R1 — the frozen header layout), cavekit-device-build.md (R5 — the segment is the memory budget)

## Changelog
- 2026-11-07: Sweep: evidence notes compacted to date, task, proof and the deciding numbers; superseded notes and session narrative moved verbatim to `archive/offscreen-rendering-history.md`. No box changed state.
- 2026-10-06 (later): R9 "wide regions on the device" `[ ]`→`[x]` — the pinch hook works through the adapter's gesture entry points; the wide region was being dropped at the content's right edge (one-px rounding) and is fixed. "A real pan and a real pinch, judged by eye" stays the owner's.
- 2026-10-06: Added R9 (the page stays painted: full budget, composed frames, sweeps, wide when zoomed) for the 2026-10-05 work. R8's partial path gained the band-only repaint and a strip-generation test, within its own rules.
- 2026-10-01 (later): R8 "a blank page settles" `[ ]`→`[x]` and R3 "invalidations are coalesced"
  back to `[x]` — device-verified: about:blank settles in ~0.5 s then 0 paints in 30 s; a blank
  frame mid-load is still never published.
- 2026-10-01: **Target platform is now webOS CE 3.1.0.** R8 gained an open criterion: a blank
  page must settle (the `about:blank` repaint storm — ~22 unpublishable full paints a second,
  forever); R3's "invalidations are coalesced" `[x]`→`[~]` for the same measurement. Every device-verified box in this kit is 3.0.5 evidence and is listed in
  cavekit-device-build.md R4's re-verification sweep.
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
