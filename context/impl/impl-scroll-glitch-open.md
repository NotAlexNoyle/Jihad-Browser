---
created: "2026-08-02"
last_edited: "2026-08-02"
status: OPEN — P1, user-visible, suspected regression from adc06f5f
---

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
