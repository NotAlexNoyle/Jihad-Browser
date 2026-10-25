---
created: "2026-06-30"
last_edited: "2026-10-06"
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
- [x] Invalidations are coalesced so transient changes do not produce redundant full repaints. *(**RE-CLOSED `[~]`→`[x]` 2026-10-01 (later): the blank-page repaint storm this was downgraded for is fixed and device-verified — 0 paints in 30 s on a settled `about:blank`, see R8's last criterion.**)* *(**DOWNGRADED `[x]`→`[~]` 2026-10-01 (earlier).** True of a page with content — a static page is quiet, 16 log lines in 30 s — and FALSE of a blank one: on `about:blank` the daemon performs ~22 full repaints a second forever (663 in 30 s, measured on webOS CE 3.1.0), every one redundant, because a frame it refuses to publish is retried on the next tick. That is not a coalescing failure in the invalidation path itself, but it is exactly the outcome this criterion exists to rule out, so the box cannot stay checked while it happens. Closes with R8's "a genuinely blank page settles" criterion.)*
- [x] **A content change produces a repaint without user input.** *(FIXED: UXP patch 0012 adds a sticky PuppetWidget::Invalidate() dirty flag drained by `jihad_offscreen_take_dirty`; the daemon polls it each tick after PumpFor and repaints on dirty — the engine-driven frame delivery the stock QtWebKit server got from Qt paint events; plus progress-increase→repaint during loads. Verified on device 2026-07-19: example.com renders with zero input, alternating double-buffer `painted` frames in the log. The earlier "old page sticks around / overload" that masked this was the daemon crash — fixed separately (see cavekit-navigation-events R6). See context/impl/device-test-2026-07-19.md.)*
**Dependencies:** cavekit-ipc-contract.md (R2), cavekit-engine-embedding.md (R3)

### R4: Geometry and viewport events emitted
**Description:** The renderer reports size, scroll, and viewport changes.
**Acceptance Criteria:**
- [x] Content-size changes emit a contents-size-changed message. *(2026-10-01 (third session), CE sweep: `size 600 500` → `emitGeometry contentSize=600x500 win=600x500`, frame painted 600 wide, and back; colours and orientation checked by pixel on `jihad-colors.html` — red top-left (255,0,0), green top-right, blue below-left, yellow below-right; an 8 s ticker painted 8 frames through the three rotating buffers with no input; PageDown (an engine-initiated scroll) produced `scrolledTo -> card: 0,908 (content-initiated)` and full frames, no partial.)*
- [x] Scroll position changes emit a scrolled-to message.
- [x] Parsing a viewport meta tag emits a meta-viewport message with the parsed scale/size values. *(**2026-10-01 (third session) — it was ALSO being emitted for pages with no tag, with a value that is not a scale, and that broke the Mojo shell; fixed and device-verified.** The engine answers its viewport query for every document; with no tag the "initial scale" is "unset" clamped to its minimum, 0.1, and the daemon forwarded it on every geometry retry. The Mojo WebView applies the message literally: measured `metaViewport is=0.1 … 980x1271`, the widget sized its node to 76x99 and every page without a viewport tag drew as a thumbnail in the corner of the card. Now sent only when the document declares a viewport, only when it changes, and a tag with a width but no initial scale reports "fit that width". After: the same page fills the card width in Mojo (`../impl/evidence-2026-10-01/mojo-no-viewport-page-after-fix.jpg`), 0 viewport messages for it, and Enyo is unchanged (980 layout at 0.7837).)*
**Dependencies:** none (emitted by the renderer; consumed alongside cavekit-navigation-events.md — see Cross-References)

### R5: Resize, zoom, and scroll commands affect output
**Description:** Surface/viewport commands change what is rendered.
**Acceptance Criteria:**
- [x] `setWindowSize`/`setVirtualWindowSize` resize the surface and content viewport respectively.
- [x] `setScrollPosition` and `scrollLayer` move the rendered content. *(**2026-10-01 (third session) — the ENGINE scroll behind it did not happen on chrome pages or with script disabled; fixed and device-verified.** `ScrollTo` was a `javascript:` URL. It now sets `scrollLeft/scrollTop` on the viewport's scrolling element (the root, or `<body>` in quirks mode). Device: standards page, quirks page, and the same page with `javascript.enabled=false` — `scroll 0 1100` moved a link from y=1500 to y=400 and a tap at its document position hit `<A>` and navigated; a drag moved it by the drag; `about:preferences` scrolled and a button at document y=1335 was hit.)*
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
- [x] The `about:addons` tools menu opens, is readable, can be dismissed, and its items ACTIVATE. *(**2026-10-01 (third session) — re-run on CE 3.1.0, device: two defects that only a device at a fit zoom shows, both fixed.** `about:addons` is laid out 980 wide and fit-zoomed to 0.78 in portrait. (1) A tap on a menu row landed on a different row: the popup path divided an already-converted coordinate by the zoom a second time (`clickAt (601,73)` reached the popup as `doc=766.9,93.2 -> popup-local 330,52`). (2) The menu did not appear until something else forced a frame: XUL opens it asynchronously, after the tap was painted, and a popup damages nothing in the page (`popups=1`, `0 composites` four seconds later). Now: the tap on the gear opens the menu and it is on screen with no further input (`composited 1 popup(s) over the frame`), Escape closes it (`popups=0`), and a tap on "View Recent Updates" closes it and switches the view (screenshots). The activation half of this box had only ever been desktop-verified.)*
  Desktop-verified end to end: all items, separators and the checkmark render; a tap elsewhere
  dismisses; clicking "Update Add-ons Automatically" drives the real XUL command path (on reopen
  the checkmark is gone and the last item reads "…to Update Manually"). Device-verified: the menu
  opens and composites over the page, and a double-delivered tap no longer closes it.
  Page context menus are NOT this path — a long-press is reported to the CARD over the frozen
  contract (isis model), so the engine never opens a context popup here. **2026-10-25, by finger on the GPU path: after an item RAN (Check for Updates) the menu stayed drawn — its pixels were in the frame slots and the GPU frame rebuilt on them; the daemon now voids every slot when a popup closes (`BrowserPageGoanna.cpp`, the popup transition), verified gone after the item and after a dismiss. The menu's look: `#utils-menu` styled in addons.css (it was pale on pale).*
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
- [x] Damage is applied in the BUFFER's pixels: at any zoom but 1 the engine's rectangle is scaled by the zoom and shifted by the render pan before it is used. *(Added and met 2026-10-01 (third session). The engine reports damage in layout px and the repaint works in buffer px; they were used as-is, which is only right at zoom 1 — and every page without a viewport tag is fit-zoomed (0.78 in portrait). Measured: the control plugin at layout 100,120 200x150 belongs at buffer 78,94 157x118 and showed only the part of itself inside the unscaled box (screenshot bbox x 101–234, y 202–292 where 78–235, 176–294 was due). After the conversion its bbox is 80–234 × 177–292, a self-changing block at layout 700,800 (`render/goanna/test/jihad-zoom-damage.html`) ends uniformly in its final colour, and Flash's own box became exact at that zoom too: paint 11 ms → 4 ms per frame, 33.3 fps.)*
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
- [x] **A genuinely blank page SETTLES: a frame the full path will not publish must not be
      retried every tick forever.** *(**MET 2026-10-01 — fixed and DEVICE-VERIFIED on webOS CE
      3.1.0, daemon `3e29f045…`, Enyo card connected, `inject: no page` = 0, log growth asserted.**
      A blank-over-good frame is now treated as transient only while the load is running; 500 ms
      after the load is done a blank that persists is published once, and a blank that is still
      being refused is re-rendered on a 100 ms grid instead of every tick. Four runs:*
      *(1) `about:blank`: 4 blank renders, then `blank frame settled after 558 ms — publishing
      it`, then **0 paints in the next 30 s** (was 663).*
      *(2) blank → real page: first non-blank `painted` 0.78 s after the navigation was injected,
      one blank render before it — the settle did not become "never paints again".*
      *(3) WHITE-FLASH PROTECTION, a page whose body arrives 2.5 s after its headers, loaded over
      a good frame: 12 blank renders while the load ran (≈5 a second, where it used to be ≈22),
      **zero** `blank frame settled` lines — nothing blank was published over the previous page —
      and the content painted once it arrived.*
      *(4) a page that is blank when its load completes and writes its content from a timer 3 s
      later: the blank settles and is published at 515 ms, and the late content then paints and
      reads back. So a document that is blank at load and fills in later is not stuck either.)*
      *(Original criterion follows.)* **A genuinely blank page SETTLES.** *(Added 2026-10-01 — defect measured on the device, webOS CE
      3.1.0, 2026-09-27/10-01. On `about:blank` the daemon logs `painted shmid=… bytes=0 … ms=35`
      about 22 times a second indefinitely: 663 full paints in 30 s, 2.4 KB/s of log (~210 MB/day
      against a 62 MB `/var`), a full ~35 ms software render each time. The full path produces a
      frame with zero non-blank bytes, the blank-over-good suppression above refuses to publish
      it, nothing records that the refusal was FINAL for this content, and so the next tick tries
      again. The suppression is right about a blank frame that follows real content (a white
      flash mid-load); it is wrong about a document that is blank on purpose. `about:blank` is an
      allowlisted home page (`safeUrl`, all three shells), so an ordinary user setting selects
      this state — it is not a debug-only corner. A static page with real content is quiet (16
      log lines / 30 s), which is the control.*
      *To close, on the device, card connected, log growth asserted: with `about:blank` loaded
      and nothing animating, full paints over a 30 s window after load fall to a bounded handful
      (same order as the static-content control, not hundreds), the per-paint log line stops, and
      a subsequent navigation to a real page still paints its first frame promptly — i.e. the
      settle must not become "never paints again". The white-flash protection this requirement's
      first criterion exists for must survive: a blank frame arriving DURING a load of a page
      that goes on to have content must still not be published over the previous good frame.)*

**Dependencies:** R1, R2, cavekit-addons-extensions.md (R7)
**Kill switch:** `JIHAD_NO_PARTIAL_PAINT=1` disables the damage-only branch. NOTE it gates exactly
one block — it is NOT a general "disable this session's frame work" switch, and a 2026-08-09 bisect
that assumed otherwise reached a right conclusion for the wrong reason.

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
- [x] Zoomed in, the region is as wide as the content up to two viewports, centred on the view, so a sideways pan has something to show; the adapter's rotation guard compares the viewport width (`bufferWidth`) and not the region's. Rows shrink in proportion. *(Device 2026-10-06, through the adapter's own pinch entry points (`gestureStart/Change/End`, hook step `g`): Wikipedia at zoom 1.2434 paints a 1388x2266 region (`region columns: content 1388 px wide ... -> [0, +1388), rows up to 2266`), the card composites it right, and a pinch back to 0.8447 gives 943x3335 with sweeps. The first run painted one viewport wide: the content width scaled to 1387 while the band ended at 1388 (two roundings of the same clamp), and the band-inside-region check threw the wide region away exactly at the content's right edge, where a sideways pan ends. Fixed: the content is at least as wide as the band.)*
- [ ] A real pan and a real pinch, judged by eye: scrolling fast through a long page, zooming in and panning sideways. Script-made pointer events do not reach the plugin and the adapter's flick entry needs a drag in progress, so this is the owner's. [human at the device]
- [x] Wide (zoomed-in) regions on the device. *(2026-10-06: the pinch hook works — the adapter's gesture entry points do what a pinch does, zoom and scroll included (`setZoomAndScroll zoom=1.2434 (was 0.6882) scroll=658,686`), so the wide region was painted and composited on the device; see the criterion above. The sideways pan itself still has no script: scrolling is the adapter's drag, which a script cannot start.)*
**Dependencies:** cavekit-ipc-contract.md (R1 — the frozen header layout), cavekit-device-build.md (R5 — the segment is the memory budget)

## Changelog
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
