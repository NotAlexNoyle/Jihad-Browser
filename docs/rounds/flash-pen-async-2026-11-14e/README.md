# Flash pen events without the wait (engine 0112) — rounds a-e, 2026-11-14

Why: under a drag in Raft Wars the screen changed 0.5 times a second (9-14 untouched). The daemon's main thread sat in
the intr NPP_HandleEvent of every pen move until the movie's frame ended (80-150 ms), so nothing was published.

| round | build | Flash finger tests failed (of 45) | what it showed |
|---|---|---|---|
| a `flash-pen-async-2026-11-14` | down/move/up async, delivered on receipt | 12 | releases lost (n-up 0 in tap and lock): delivered inside the movie's own call to the host |
| b `…-14b` | async off; the daemon capped moves (newest only, 25 ms) | 4 | R5 move rate 11.6-14.2/s: cap reverted |
| c `…-14c` | async, queued while an event call is open | 6 → 3 after reruns | releases kept; R5 25.5-28.6/s; Enyo's drag after a hold lost (0 moves from the card) |
| d `…-14d` | + the movie's real answer per kind (JihadPenHandled) | 5 (of 42) | Enyo drag back (27/s); a tap's down lost, its up counted (Mochi tap, Enyo tags) |
| e `…-14e` | + delivered only from a loop task (as DoAsyncSetWindow) | 3 → 1 after reruns | committed as engine 0112 (6331ef3f87) |

The fixed answer of round c was wrong: this AVM2 movie answers 1 to a down and an up (the child's `palm event 0x1 …
handled=1`), and a 0 let the page's default for the press run. The test `flash-input-hold` waited for "up" after its
drag, which the hold's own "up 201 150" already matched: it now waits for "up 3" (the drag ends at stage x 360).

Left: `flash-input-cover` in Mochi (above-keyboard + flash-at), which failed the same way before this work (11-12 full
round, round b). Round e's Enyo flash-input-kernel (card-scroll-min) and Mojo flash-fullscreen (fs never fullScreen
after the second tap) passed on rerun.
