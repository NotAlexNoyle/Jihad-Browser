---
created: "2026-08-02"
last_edited: "2026-08-02"
status: IMPLEMENTED — desktop verify in progress; device deploy after ARM rebuild
---

# Scroll pan headroom — overscan paint (fix for impl-scroll-glitch-open.md)

The root defect (painting exactly the viewport → zero pan headroom → grey strips on every pan)
is fixed by painting **viewport + overscan** and reporting the REAL painted geometry. The shm
segment always had 4x-screen room (`BrowserOffscreen::create`,
`kOffscreenSizeAsScreenSizeMultiplier = 4.0`); the daemon just never used it.

## The pieces

1. **libxul export `jihad_offscreen_render_region`** (PuppetWidget.cpp, patch 0005 regenerated):
   renders an ABSOLUTE document region (`RENDER_DOCUMENT_RELATIVE |
   RENDER_IGNORE_VIEWPORT_SCROLLING`) straight into caller memory via
   `Factory::CreateDrawTargetForData(CAIRO, …)` — NOT bounded by the widget DrawTarget, so the
   region can be taller than the viewport. Opaque-white base keeps alpha 255 (adapter blits raw
   words). WEAK in the daemon: a pre-overscan libxul degrades to the old viewport-exact paint
   instead of failing at load (new-symbol-weak is safe; the F1 rename rule is for CHANGED
   signatures).

2. **`BrowserPageGoanna::paintToSharedBuffer`** now:
   - computes the painted rows `[lo, hi)`: covers the viewport band AND the adapter's pan
     target, plus overscan (½ viewport above, 1 below), bounded by `segSize` and the content
     height;
   - renders the whole region doc-relative into the shm pixels, then at z≈1 **overlays the
     viewport-relative band** (`ReadPixels` into `pixels + bandRow*stride`) so
     `position:fixed`/`sticky`/caret stay correct on the visible rows — the doc-relative strips
     only show transiently mid-pan (fixed content is NOT at the scroll position in strips, by
     construction);
   - stamps **honest geometry**: `renderedY = lo`, `renderedHeight = tallH`, and the band
     position from the ENGINE's actual state (`GetScrollXY` at z≈1 — the `javascript:` ScrollTo
     is async and can lag the adapter; when zoomed, the CLAMPED `GetRenderPan`), not the raw
     adapter scroll. The old stamping (raw `mAdapterScrollX/Y` while the frame rendered
     elsewhere) was itself a source of "content moves without me touching it".
   - Zoomed path is a single doc-relative region render (pure visual viewport, same semantics
     as before, just taller).

3. **Adapter: NO changes needed.** Verified both composite paths pan anywhere inside
   `renderedWidth/Height` (`baseY = srcTop - renderedY`, FILL outside) and hold only on a
   WIDTH mismatch — so tall buffers are safe in portrait AND landscape (the guard is width-only;
   a tall landscape buffer does not trip it). `bufferWidth/Height` are log-only + a never-called
   `copyFrom` guard.

4. **Test**: `adapter_client.cpp` now allocates the real 4x segment, loads a 4-band
   taller-than-viewport page by default, verifies against the header geometry, counts strip
   (beyond-first-viewport) content, and enforces `OVERSCAN PASS` under `JIHAD_EXPECT_OVERSCAN=1`
   (set in both round-trip scripts).

## Desktop verification — PASS (2026-08-02)

- Xvfb round-trip: `ROUND-TRIP PASS` + `OVERSCAN PASS` — painted **1024x1536** (viewport 768 + a
  full extra viewport), `renderedY=0`, 739k non-white px in the strip rows, band-2 colour probed
  at the right rows in the PPM (the ~40 px band offset is h1 margin-collapse, not a defect).
- Headless (no X, the device configuration): same PASS after fixing a PRE-EXISTING init crash —
  the T-067 widget probe `do_GetService`s the GTK drag service, whose constructor builds GTK
  widgets against a null display → `gdk_window_new` assert → SIGSEGV. The no-X desktop round-trip
  had not run since T-067 landed. Probe now skipped on the GTK build when DISPLAY is unset
  (device headless-toolkit build unaffected).
- Note: the desktop objdir had been cleaned — this required a full desktop rebuild.

## Opus adversarial review (2026-08-02, replaces Codex per user direction) — all 16 findings dispositioned

Verdict was **BLOCKERS: 3**; every blocker + P2 fixed and re-verified same day (Xvfb AND
headless: `ROUND-TRIP PASS` + `OVERSCAN PASS` + **`SCROLL-OVERSCAN PASS`** + `alphaBad=0`,
scrolled frame `renderedY=816`, painted 1024x1920, band-3 colour probed at the computed row).

- **F1 (blocker)** portrait tallH 2355 would exceed the SGX540's 2048 max texture on the
  closed-source `PGSurface::wrap` path → region rows capped at 2048 until a device run proves
  taller blits.
- **F2 (blocker)** tests only ever exercised offset 0 → adapter_client now runs a scrolled
  phase: `SetScrollPosition(0,1200)`, requires `renderedY > 0` AND the band-3 colour at
  `(700, 2000-renderedY)` — the bandRow/renderedY arithmetic proven at a real offset.
- **F3 (blocker)** alpha==255 was assumed, adapter raw-blits words → enforced at the source
  (`jihad_offscreen_render_region` post-pass) + unconditional `alphaBad` gate in the test.
- **F5** over-budget branch could drop the ADAPTER's viewport from the region (engine parked
  by `overflow:hidden` + fling → persistent white card) → re-anchor on `adY` after the budget
  clamp; content clamp re-applied after (F10).
- **F6** horizontal: no x overscan, so stamping the engine's lagging x = white column on every
  horizontal pan → x stays the adapter's own scroll (pre-change identity blit).
- **F7** header geometry now varies per frame, so the 250 ms in-flight reclaim valve made
  torn-geometry blits reachable → valve widened to 2000 ms; REAL fix (header frame-seq +
  adapter-side re-read guard) queued for the next adapter rebuild.
- **F8** blank-over-good suppression judged only the band → strips sampled (every 8th row)
  before suppressing, so a white visible band over full strips still publishes.
- **F9** `GetScrollXY` failure at z≈1 → viewport-exact fallback instead of stamping unknowable
  geometry. **F4** its forced reflow removed (`flushLayout=false`; the render flushes anyway).
- **F12** install-prompt text: content-controlled add-on names could forge the origin line →
  control chars stripped, hard budgets, HOST leads the text. **F13** observer removed on
  shutdown. **F11** the XPI component stays deliberately UNWIRED (no manifest) until the card
  confirm-reply path is verified — a blocking dialog with no card answer would hang the daemon.
- **F14** push scripts: postinst md5-verified before root-exec; engine-update restarts the
  variant's job on any failure path. **F15** probe guard checks the OPENED gdk display, not
  just $DISPLAY. **F16** start-page visual check must cover portrait + landscape + VKB-up
  (failure mode of the flex chain is a BLANK page, not a mis-centred one).

## DEVICE VERIFIED (2026-08-02, post-review deploy + cold reboot)

- Engine (libxul + daemon, all review fixes) pushed to ALL THREE variants
  (`push-engine-update.sh`, md5-verified, atomic mv); device rebooted.
- **Cold boot: all three daemons auto-start** on their own sockets, ~27 MB RSS each idle,
  swap untouched — the R7 daemon layer measured with all three variants real for the first
  time (`device-independence-test.sh check` 24/24 PASS as well).
- **Enyo card end-to-end:** example.com paints viewport-exact (short page — correctly no
  overscan waste); `holdtest.html` (2876 content rows) paints viewport + overscan through
  the REAL adapter; fb1 shows the card composited scrolled with the page's fixed header
  correctly pinned (band overlay), no grey strips, `wrap FAILED` count 0 (≤2048 PGSurface
  regions blit fine).
- **User feel (live): "scrolling is now more reliable"** — the grey-strip repro is gone.

## Follow-up fixes shipped the same session (user live-testing)

1. **Long-press was never delivered — daemon `asyncCmdHitTest` was a stub.** The adapter
   GATES mousehold on a hit-test round-trip (`mouseHoldTimeoutCb → asyncCmdHitTest`, sends
   `asyncCmdHoldAt` only after `msgHitTestResponse` + card decline). Implemented a real
   hit test (`GoannaRenderPage::HitTestAt`: ElementFromPoint + anchor/img ancestor walk →
   the stock `/etc/palm/browser/HitTest.schema` JSON, escaped + UTF-8-safe truncation).
   This also unlocks card-side link/image context menus later.
2. **Input coords were DOCUMENT space dispatched as VIEWPORT space.** Measured live: the
   contextmenu reached the page at `client=(388,1488)` while `scrollY=909` (target at
   client y≈579). One `docToViewport` mapping at the input drain now covers
   click/mouse/contextmenu/touch + the hit test (z≈1: subtract engine scroll; zoomed:
   subtract the clamped render pan; never flushes layout in an input path). The earlier
   "no mapping" pass had fixed a double-ADD by overshooting to zero mapping. This likely
   also explains 2026-07-17's "link taps below the fold never navigate".
3. **Scroll blips ("content blips in and out", user live):** overscan now biased toward the
   pan direction (⅛ viewport behind, 1.5 ahead), settle gate 220 → 100 ms. Both feel-tunable.

## Live-tuning rounds with the user (same day)

**User: "long press works" — the test page banner went green** (`PASS hit=target
client=(470,875) page=(470,1352) scrollY=477`): hit-test round-trip + doc→viewport mapping
verified end-to-end by a human finger.

**Round 2 ("scrolling is janky and skips around"):** three more root causes, all daemon-side:
1. `msgScrolledTo` echo fight — the adapter OVERWRITES its own pan from the echo
   (`BrowserAdapter.cpp:4218`), and our engine echo lags the finger by 100-300 ms → periodic
   backwards yanks. Echoes that merely confirm adapter-commanded scrolls are now suppressed
   (recent `setScrollPosition` + engine within half a viewport of its target); genuine
   page-initiated jumps still emit.
2. Fit-zoom 1.0052 — content 764 css px in a 768 window made every blit a 0.9948
   nearest-neighbour resample (row dup/skip shimmer). `emitGeometry` now floors the reported
   content width at the window width → fit exactly 1.0 → identity blits. Verified live:
   `mZoom=1.0000` where it was `1.0052`.
3. Repaint starvation — every `setScrollPosition` forced a 2-viewport repaint even with the
   target fully inside the painted region. Coverage-aware skip: repaint only within h/2 of a
   painted edge (page top/bottom edges exempt from slack).

**Round 3 ("smoother but content doesn't stay consistently visible"):**
- The scroll-settle gate REMOVED — it predates honest geometry, and with coverage-aware
  requests it was inverted: a paint request mid-fling exists precisely because painted rows
  are running out, and the gate (reset by every scroll msg) held that paint until the fling
  ended — guaranteeing the white gap.
- FLING MODE: while scroll msgs stream (<250 ms), paints skip the band overlay (single
  doc-relative pass, sampled nonblank) — ~40% cheaper (desktop: 7 ms vs 18 ms; device
  ~10-20x). `position:fixed` rides at document position during the fling only; pump()
  ships one full-fidelity frame ≥300 ms after the last scroll (settle repaint).
- Paint duration now in the `painted` log line (`ms=`).

**App display names (user request): "Jihad Enyo" / "Jihad Mochi" / "Jihad Mojo"** — appinfo
titles + each UI's self-branding; deployed via file push + LunaSysMgr restart.

**Round 4 ("the fixed banner flies around during scrolling; other parts behave"):**
- FLING MODE REVERTED same-day: skipping the band overlay painted `position:fixed` at its
  DOCUMENT position in fling frames while full frames painted it at the viewport — two
  alternating models made fixed content jump erratically. A full region+band paint measured
  **66 ms on device** (`ms=` now in the painted log line), cheap enough to be every frame.
- Pan-cadence refresh added: during an active pan, repaint every ~250 ms even when the pan
  is fully covered — otherwise the coverage skip let the baked-in fixed banner drift up to a
  whole overscan before snapping. Fixed content now swims ≤ a beat and snaps per frame (the
  classic webOS-era compromise); everything else stays identity-stable.
- Card-lifecycle note: every `push-engine-update.sh` kills the live card's adapter; the
  relaunch after a LunaSysMgr restart can take minutes to ACK (app manager slow to return) —
  the `until …grep returnValue` retry loop is the working pattern.

## Still open on this item

- User confirmation of the long-press after fix 2 (the coordinate mapping) — watching the
  daemon log + the test page's own banner (it reports hit coords + scrollY on-page).
- Paint cost: region ≤ 2048 rows + band per paint; 150 ms dirty rate limit unchanged. If
  typing/animation latency regresses, trim the ahead-of-pan overscan first.
- The F7 torn-geometry REAL fix (header frame-seq + adapter re-read guard) — next adapter
  rebuild.

# Variant deploy (R7) — same session — ALL THREE CARDS NOW RUN THEIR OWN ENGINE

Post-reboot card launches (the final R7 routing evidence, all measured):
- **Mochi card end-to-end for the FIRST time via its own daemon**: example.com loaded +
  painted through `yapserver.jihad-browser-mochi` (its own shm keys).
- **Mojo card — the variant that had NEVER run**: its UI on fb1 (start view + nav chrome)
  with paints through `yapserver.jihad-browser-mojo`; the enyo daemon's log untouched
  during the launch (cross-variant isolation observed live).
- Enyo card unaffected throughout. Three cards, three shims, three daemons, three sockets.

# Variant deploy (R7) — same session

`build/webos-oe/push-variant.sh` (new): pushes a staged variant payload over novacom as ONE
md5-verified tarball, extracts into the app's cryptofs dir, and runs the variant's REAL
`packaging/<V>/postinst` on-device as root — the user-approved autonomous route.

- **mochi: DEPLOYED + daemon RUNNING** (`yapserver.jihad-browser-mochi` bound, job
  `jihad-mochi (start) running`). Card→adapter still needs the REBOOT (WebKit scans
  BrowserPlugins at boot) — do it once mojo + the overscan libxul re-push land.
- mojo: push in progress.
- Postinst nit fixed + scripts regenerated: this device's upstart `status` prints
  `"(start) running"`, not `"start/running"`, so the old pattern always warned "did not start"
  for a running daemon and the teardown wait always burned its 10 s.
- Start-page centring (queue item 4) implemented in `app/` (flex + height:100% instead of the
  fixed 1024px box); mochi/mojo start pages already tracked card height. Device visual check
  pending.
