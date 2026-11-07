# cavekit-offscreen-rendering.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## R3, criterion 3 (invalidations coalesced) (removed 2026-11-07)

 *(**RE-CLOSED `[~]`→`[x]` 2026-10-01 (later): the blank-page repaint storm this was downgraded for is fixed and device-verified — 0 paints in 30 s on a settled `about:blank`, see R8's last criterion.**)* *(**DOWNGRADED `[x]`→`[~]` 2026-10-01 (earlier).** True of a page with content — a static page is quiet, 16 log lines in 30 s — and FALSE of a blank one: on `about:blank` the daemon performs ~22 full repaints a second forever (663 in 30 s, measured on webOS CE 3.1.0), every one redundant, because a frame it refuses to publish is retried on the next tick. That is not a coalescing failure in the invalidation path itself, but it is exactly the outcome this criterion exists to rule out, so the box cannot stay checked while it happens. Closes with R8's "a genuinely blank page settles" criterion.)*

## R3, criterion 4 (content change repaints without input) (removed 2026-11-07)

 *(FIXED: UXP patch 0012 adds a sticky PuppetWidget::Invalidate() dirty flag drained by `jihad_offscreen_take_dirty`; the daemon polls it each tick after PumpFor and repaints on dirty — the engine-driven frame delivery the stock QtWebKit server got from Qt paint events; plus progress-increase→repaint during loads. Verified on device 2026-07-19: example.com renders with zero input, alternating double-buffer `painted` frames in the log. The earlier "old page sticks around / overload" that masked this was the daemon crash — fixed separately (see cavekit-navigation-events R6). See context/impl/device-test-2026-07-19.md.)*

## R4, criterion 1 (contents-size-changed) (removed 2026-11-07)

 *(2026-10-01 (third session), CE sweep: `size 600 500` → `emitGeometry contentSize=600x500 win=600x500`, frame painted 600 wide, and back; colours and orientation checked by pixel on `jihad-colors.html` — red top-left (255,0,0), green top-right, blue below-left, yellow below-right; an 8 s ticker painted 8 frames through the three rotating buffers with no input; PageDown (an engine-initiated scroll) produced `scrolledTo -> card: 0,908 (content-initiated)` and full frames, no partial.)*

## R4, criterion 3 (meta-viewport) (removed 2026-11-07)

 *(**2026-10-01 (third session) — it was ALSO being emitted for pages with no tag, with a value that is not a scale, and that broke the Mojo shell; fixed and device-verified.** The engine answers its viewport query for every document; with no tag the "initial scale" is "unset" clamped to its minimum, 0.1, and the daemon forwarded it on every geometry retry. The Mojo WebView applies the message literally: measured `metaViewport is=0.1 … 980x1271`, the widget sized its node to 76x99 and every page without a viewport tag drew as a thumbnail in the corner of the card. Now sent only when the document declares a viewport, only when it changes, and a tag with a width but no initial scale reports "fit that width". After: the same page fills the card width in Mojo (`../impl/evidence-2026-10-01/mojo-no-viewport-page-after-fix.jpg`), 0 viewport messages for it, and Enyo is unchanged (980 layout at 0.7837).)*

## R5, criterion 2 (setScrollPosition / scrollLayer) (removed 2026-11-07)

 *(**2026-10-01 (third session) — the ENGINE scroll behind it did not happen on chrome pages or with script disabled; fixed and device-verified.** `ScrollTo` was a `javascript:` URL. It now sets `scrollLeft/scrollTop` on the viewport's scrolling element (the root, or `<body>` in quirks mode). Device: standards page, quirks page, and the same page with `javascript.enabled=false` — `scroll 0 1100` moved a link from y=1500 to y=400 and a tap at its document position hit `<A>` and navigated; a drag moved it by the drag; `about:preferences` scrolled and a button at document y=1335 was hit.)*

## R5, criterion 3 (setZoomAndScroll) (removed 2026-11-07)

 *(The 2026-07-04 desktop ZOOM PASS rested on `SetFullZoom`, which `../impl/zoom-fix-2026-07-27.md` later proved does NOT magnify an offscreen `RenderDocument` capture — its internal AppUnitsPerDevPixel/AppUnitsPerCSSPixel scale cancels the engine zoom, so on device the content shrank into a `1/Z` quadrant ("things get cut off"). REBUILT + device-verified 2026-07-27 (commit 8d7865c): `JihadRenderDocument` pre-scales the gfxContext by Z and renders an absolute document rect (`RENDER_DOCUMENT_RELATIVE`), leaving the layout viewport at device width (no reflow → cannot re-arm the landscape fit-zoom loop → R6 stays safe). Verified on device via the daemon inject channel + frame.ppm: `zoom 3.0` magnifies 3×, horizontal and vertical pan reach the page edges/bottom, `zoom 1.0` unregressed. Input consistency under the new transform is cavekit-input-bridging.md R5 (also device-verified, commit d4f0842).)*

## R6, criterion 2 (rotate renders correctly) (removed 2026-11-07)

 *(CONFIRMED ON
  DEVICE 2026-07-27, commit 8d7865c: "correct in both orientations, superseding the white-frame
  guard … Verified on-device"; `../impl/impl-overview.md` 2026-07-27 opens "Rotation confirmed
  working on device" and `../impl/zoom-fix-2026-07-27.md` dates its next user report "after
  rotation was confirmed working". This box was checked on 2026-07-26 while the check was still
  device-gated — the note said so; the 2026-07-27 evidence is what actually backs it. The
  confirmation is a device/visual confirm recorded in the commit + impl entries; no per-orientation
  capture was archived. The old white-frame "rotation guard" band-aid is removed by this path.)*

## R6, criterion 3 (PG symbols resolve) (removed 2026-11-07)


  *(Off-device demangle done 2026-07-26 (`../impl/rotation-fix-2026-07-26.md` "Verification": exactly
  the two PG UNDs, no vtable/typeinfo orphans, `releaseRef` resolved inline via the modeled
  `PGShared` base — Codex F1). On-device resolution recorded 2026-07-27, commit 8d7865c: "the two PG
  symbols resolve against libWebKitLuna" — necessarily so, since the PG-composited adapter loaded and
  rendered the rotate confirmed above.)*

## R7, criterion 2 (XUL menupopup composited) (removed 2026-11-07)


  **The 2026-08-02 diagnosis that a popup "is created 0x0 and never shown" was wrong**: it was
  measured on a plain `<select>`, which is the COMBOBOX path and never enters the popup manager.
  A real `<menupopup>` sizes and shows itself correctly (594,52 347x117 on desktop), so no
  size-and-show fix was needed — only the compositing, and then the input routing below.

## R7, criterion 3 (about:addons tools menu) (removed 2026-11-07)

 *(**2026-10-01 (third session) — re-run on CE 3.1.0, device: two defects that only a device at a fit zoom shows, both fixed.** `about:addons` is laid out 980 wide and fit-zoomed to 0.78 in portrait. (1) A tap on a menu row landed on a different row: the popup path divided an already-converted coordinate by the zoom a second time (`clickAt (601,73)` reached the popup as `doc=766.9,93.2 -> popup-local 330,52`). (2) The menu did not appear until something else forced a frame: XUL opens it asynchronously, after the tap was painted, and a popup damages nothing in the page (`popups=1`, `0 composites` four seconds later). Now: the tap on the gear opens the menu and it is on screen with no further input (`composited 1 popup(s) over the frame`), Escape closes it (`popups=0`), and a tap on "View Recent Updates" closes it and switches the view (screenshots). The activation half of this box had only ever been desktop-verified.)*
  Desktop-verified end to end: all items, separators and the checkmark render; a tap elsewhere
  dismisses; clicking "Update Add-ons Automatically" drives the real XUL command path (on reopen
  the checkmark is gone and the last item reads "…to Update Manually"). Device-verified: the menu
  opens and composites over the page, and a double-delivered tap no longer closes it.
  Page context menus are NOT this path — a long-press is reported to the CARD over the frozen
  contract (isis model), so the engine never opens a context popup here. **2026-10-25, by finger on the GPU path: after an item RAN (Check for Updates) the menu stayed drawn — its pixels were in the frame slots and the GPU frame rebuilt on them; the daemon now voids every slot when a popup closes (`BrowserPageGoanna.cpp`, the popup transition), verified gone after the item and after a dismiss. The menu's look: `#utils-menu` styled in addons.css (it was pale on pale).*

## R8, criterion 1 (damage in buffer pixels) (removed 2026-11-07)

 *(Added and met 2026-10-01 (third session). The engine reports damage in layout px and the repaint works in buffer px; they were used as-is, which is only right at zoom 1 — and every page without a viewport tag is fit-zoomed (0.78 in portrait). Measured: the control plugin at layout 100,120 200x150 belongs at buffer 78,94 157x118 and showed only the part of itself inside the unscaled box (screenshot bbox x 101–234, y 202–292 where 78–235, 176–294 was due). After the conversion its bbox is 80–234 × 177–292, a self-changing block at layout 700,800 (`render/goanna/test/jihad-zoom-damage.html`) ends uniformly in its final colour, and Flash's own box became exact at that zoom too: paint 11 ms → 4 ms per frame, 33.3 fps.)*

## R8, criterion 2 (partial-paint base must be complete and non-blank) (removed 2026-11-07)

 *(2026-08-10. `mBufFull[fs].valid` was recorded BEFORE the
      blank-over-good suppression and before the active-buffer flip, so a frame rejected as blank
      still advertised its slot as complete; the next tick then blitted one live rectangle into that
      white buffer and published it — laundering the exact white flash the suppression exists to
      prevent. The record now happens past every early return, on the path that will definitely
      publish, and is `nb > 0` rather than `nb >= 0`. Both non-publishing returns explicitly drop the
      record via `invalidateBufRecord()`.)*

## R8, criterion 3 (engine-initiated scroll forces a full frame) (removed 2026-11-07)

 *(2026-08-10. `mDamageForceFull` existed,
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

## R8, criterion 4 (thaw invalidates buffer records) (removed 2026-11-07)

 *(2026-08-10. Its own comment said "the buffers may
      be new segments" and it called `detachShm()`, but left `mBufFull[0]/[1]` valid — so the first
      paint after a card returns to the foreground passed the eligibility test and rendered only the
      damage box into a segment we had never written, publishing whatever its previous owner left
      there. With a plugin animating this was the guaranteed first frame, not a rare one.)*

## R8, criterion 6 (partial path bounded) (removed 2026-11-07)


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

## R8, criterion 7 (a blank page settles) (removed 2026-11-07)

 *(**MET 2026-10-01 — fixed and DEVICE-VERIFIED on webOS CE
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

## R8, kill-switch note (removed 2026-11-07)

, and a 2026-08-09 bisect
that assumed otherwise reached a right conclusion for the wrong reason.

## R9, criterion 5 (wide region when zoomed in) (removed 2026-11-07)

 *(Device 2026-10-06, through the adapter's own pinch entry points (`gestureStart/Change/End`, hook step `g`): Wikipedia at zoom 1.2434 paints a 1388x2266 region (`region columns: content 1388 px wide ... -> [0, +1388), rows up to 2266`), the card composites it right, and a pinch back to 0.8447 gives 943x3335 with sweeps. The first run painted one viewport wide: the content width scaled to 1387 while the band ended at 1388 (two roundings of the same clamp), and the band-inside-region check threw the wide region away exactly at the content's right edge, where a sideways pan ends. Fixed: the content is at least as wide as the band.)*

## R9, criterion 7 (wide regions on the device) (removed 2026-11-07)

 *(2026-10-06: the pinch hook works — the adapter's gesture entry points do what a pinch does, zoom and scroll included (`setZoomAndScroll zoom=1.2434 (was 0.6882) scroll=658,686`), so the wide region was painted and composited on the device; see the criterion above. The sideways pan itself still has no script: scrolling is the adapter's drag, which a script cannot start.)*

