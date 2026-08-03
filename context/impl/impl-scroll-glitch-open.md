---
created: "2026-08-02"
last_edited: "2026-08-03"
status: RESOLVED 2026-08-03 — see impl-scroll-overscan-2026-08-02.md. User signed off ("scrolling feels good now").
---

> **RESOLVED.** The overscan paint (viewport + direction-biased headroom, honest per-frame
> geometry, ≤2048-row SGX cap) fixed this end to end; the settle gate discussed below was
> ultimately REMOVED. Kept for the root-cause history. Authoritative record + the follow-up F7
> (header frame-seq) is **`impl-scroll-overscan-2026-08-02.md`**.


# Scrolling is glitchy: content moves untouched, regions blank out

**Reported by the user on device, 2026-08-02**, while scrolling a tall test page
(`/var/palm/jihad/enyo/holdtest.html`): *"scrolling is very glitchy. it moves and changes without
me touching it and things disappear as I scroll."*

## Prime suspect: my own dirty-loop fix (adc06f5f)

Before that commit the engine-driven repaint loop was **inert** — `jihad_offscreen_take_dirty()`
could never return true, so after load NOTHING repainted. Scrolling was therefore smooth by
accident: the adapter simply panned inside the last good buffer using
`BrowserOffscreenInfo::renderedX/renderedY`, and no new frame ever arrived to disturb it.

After the fix the daemon repaints on every invalidation. Each frame is rendered at the DAEMON's
scroll position (`mAdapterScrollX/mAdapterScrollY`, written from `setScrollPosition`), which during
a fling lags the adapter's own pan. The adapter then blits a buffer whose `renderedX/Y` disagree
with where it is currently panned — which is exactly "content jumps without input" and "regions
disappear" (the out-of-buffer area is left undrawn / white-filled by the clamp in
`BrowserAdapter::handlePaint`).

So this is very likely **latency/ordering between the adapter's pan and the daemon's repaint**, made
reachable for the first time by restoring repaints. The fix itself is correct and must NOT simply be
reverted — without it no page ever updates after load (SPA content, animations, late images, and the
about:config/XUL cases). The scroll path needs to cope with frames now actually arriving.

## Decisive A/B available on device, cheaply

The pre-fix engine is still on the device next to the current one:

```
.../deviceroot/hl/libxul.so                 <- current (dirty-loop fix)
.../deviceroot/hl/libxul.so.pre-dirty-fix   <- previous
```

Swap, restart the daemon (`stop jihad; start jihad`), scroll the same page, and compare. That
attributes the glitch definitively rather than by argument. Do this BEFORE writing any fix.

## Directions to investigate (unverified)

1. **Scroll feedback loop.** The adapter sends `setScrollPosition`; the daemon emits `msgScrolledTo`.
   If the adapter applies that echo back to its own pan while the user is still dragging, position
   oscillates. Check whether `msgScrolledTo` is now emitted far more often (it is gated on the same
   pump that the dirty flag now drives).
2. **Repaint rate during a fling.** `BrowserPageGoanna` rate-limits paints (~150 ms); the flow-control
   gate (`mInFlight`, 250 ms reclaim) may interact badly with a fast pan.
3. **Suppress engine-driven repaints while a drag/fling is in flight**, and resume on settle — the
   adapter already knows it is panning.
4. Confirm whether `renderedX/renderedY` are being written from the CURRENT adapter scroll at paint
   time, or from a stale value captured earlier.

## Note

The `holdAt` long-press verification is blocked behind this: the test needs a large scroll offset,
and scrolling is currently too unstable to reach the target reliably.


## Device feedback after the scroll-settle gate (2026-08-02)

User: *"scrolling is more stable than before but my long press isnt being registered and the page is
still glitchy. sometimes I scroll down into a grey area instead of the next zone."*

So the gate reduced the backwards-jumping (stale `renderedX/Y` being blitted), but it is **at best
half a fix**, and the grey areas expose the real defect.

### The buffer has no pan headroom — this is the root problem

`paintToSharedBuffer` paints exactly `w x h = mPage->Width() x mPage->Height()`, i.e. the WINDOW
size (768x942 on device), and stamps `renderedWidth/Height` to match. The adapter pans inside that
buffer using `renderedX/Y`. With the painted region exactly viewport-sized there is **zero
headroom**: the moment the user pans by one pixel they are outside the painted area, and the
adapter's own out-of-buffer clamp leaves that strip undrawn — the grey.

Deferring paints during the pan (the settle gate) therefore trades jumping for grey: nothing
repaints the newly exposed strip until the pan settles.

**There IS room to fix this.** The shm segment is far larger than one viewport: the connect line
reports `connect 960x1400 keys=… sz=12582944`, i.e. 12.58 MB = ~3.15 M pixels at 4 bytes, against a
768x942 = 723 k pixel viewport — roughly 4x headroom. The isis/Atlas design this port inherits
expects exactly that (see the adapter's "tall-buffer pan" lineage, commit dbc897c referenced in
`BrowserAdapter::handlePaint`). So the daemon should paint a region TALLER than the viewport
(viewport + overscan, bounded by `segSize`) and report it in `renderedWidth/Height`, giving the
adapter real content to pan into.

That is the actual fix. The settle gate should be re-evaluated once there is headroom — with a
taller buffer, repainting during a pan may be harmless, and the gate may become unnecessary or want
a much shorter window.

### Also unresolved: long-press never registers

The `holdAt` path did not fire at all during this session's testing — no `contextmenu` reached the
page and no long-press line appeared in the daemon log. Unknown whether the adapter's `mousehold`
gesture is being consumed as a scroll (the gesture starts with a pen-down that the pan path also
claims), or whether `asyncCmdHoldAt` is not being sent. **This blocks verifying the `holdAt`
coordinate fix**, which is otherwise ready and desktop-reasoned.

Next step there: instrument the daemon's `holdAt` entry (it currently logs nothing) so "not sent" and
"sent but mis-resolved" stop being indistinguishable — the same instrument-the-boundary lesson this
project keeps relearning.
