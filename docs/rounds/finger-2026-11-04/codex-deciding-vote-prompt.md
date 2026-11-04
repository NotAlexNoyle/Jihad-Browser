# Deciding Vote: the next step after T-505 item 4 (Jihad Browser, GPU compositor on the HP TouchPad)

You are the peer reviewer in a cavekit "Deciding Vote". Four options have been put to the project's owner.
The builder (Claude) recommends option 1. Your job is to evaluate the four critically against the evidence
in ./evidence/ and pick ONE, with explicit reasoning. Do not rubber-stamp the builder's recommendation;
challenge its premises where the evidence allows. The owner has the final say; your pick informs it.

Read everything in ./evidence/ before answering. Do not modify any file. Do not write code.

## Context

Jihad Browser: a webOS 3 browser (HP TouchPad, ARMv7, Adreno 220 GPU, ~1 GHz dual core, libc 2.8) with the
UXP/Goanna engine. A daemon drives the engine; the engine's compositor composes frames into a shared
slot that the card shows. The "band path": for a scroll-only frame (or one whose change fits bands) the
compositor composes only the rows asked (the bands) and reads them back with glReadPixels (~90 us/row on
this GPU), after shifting the kept rows on the CPU. The kit's R3 target is 24 fps pans on real pages
(desktop Wikipedia) and on the long test page; measured by finger with a framebuffer stream sampler
(16 drags per round), see evidence/round-notes-2026-11-04.txt for the full table and methodology.

T-505 item 4 was just built behind two switches, both OFF by default:
- engine `JIHAD_GPU_CPU_BANDS=1`: the band frames composed on the CPU from the tiles' shared-memory
  buffers (memcpy / premultiplied over), only when every leaf under the bands is a plain tiled painted
  layer at resolution 1 (or a colour layer) at an integer-translation scale-1 2D transform; otherwise the
  frame stays on the GPU. Also under this switch the tiles' read lock is kept after the immediate upload
  (so the content side double-buffers rather than repainting the buffer the compositor copies).
- daemon `JIHAD_GPU_RES_ZOOM=1`: the layers painted at the zoom's resolution also below 1.1 (today the
  rule only applies above 1.1), so that a zoomed-out Wikipedia (zoom 0.6882 after load) has tiles at
  scale 1 relative to the screen and the CPU band path applies. This is the owner's "scale choice (b)".
  (The alternative "scale choice (a)", NOT built: keep painting at resolution 1 and downscale the tiles
  on the CPU into the bands, bilinear, NEON; estimated ~80k output pixels a frame.)

Measured results (evidence/round-notes-2026-11-04.txt has the full table; headline numbers):

| round | fps flowing | band frame on the compositor thread | express made median / p90 |
|---|---|---|---|
| long page (scale 1), control | 18.2 | GPU 22.0 ms (13.6 of it the read) | 31 / 57 ms |
| long page, CPU bands | 19.5 | CPU 4.7 ms | 14 / 41 ms |
| Wikipedia (0.6882), control | 9.6 | GPU 31.0 ms | 69 / 187 ms |
| Wikipedia, resolution switch only | 10.4 | GPU 20.9 ms | 62 / 203 ms |
| Wikipedia, both switches | 9.8 | CPU 20.5 ms (105 of 160 band frames; 15 ms of it the job's entry/prep) | 65 / 285 ms |

Long page under CPU bands: verify 0 wrong rows of 121 frames; worst lag 294 -> 131 ms; stream 0 stale bands.
Wikipedia with both: verify 1 wrong of 66 (outside the bands); but the main thread's paints at 0.6882
sum 5.1-5.3 s per round against 3.2-3.9 s at resolution 1 (median 102-114 vs 77-84 ms, p90 508-605 vs
227-246), the express lane waits for a slot behind main-lane jobs ("no slot" 166-232 vs 78-105 ticks),
the grey band at the screen's bottom in 21-35 samples against 14. Fps flowing on Wikipedia across the
four variants (9.6 / 9.8 / 10.4 / 10.2) is noise around 10. The builder's premise before building, "painting
at 0.6882 halves the paint pixels", was wrong in effect: the paints got dearer, not cheaper (the notes
discuss why: more of the page per tile, text/image rasterisation at a non-integer scale, more tiles
touched per scroll). An earlier round (T-504, round 183) had found the same loss.

A Codex diff review already ran on the code (evidence/impl-review-findings-t505-cpubands.md): ten findings,
seven fixed, F5/F6 open (both in the daemon's resolution switch: a pinch repaints at every step; T-527's
zoom-back gates below 1 do not know the new rule); each finding's status is in the file.

## The four options (the owner's numbering; keep it in your answer)

1. Engine switch ON by default, daemon switch OFF; then BUILD scale choice (a): tiles painted at
   resolution 1, downscaled on the CPU into the bands (bilinear, NEON), paints untouched. Rationale:
   Wikipedia's band frames leave the GPU (the ~13-20 ms read) without the paints' price. The builder's
   recommendation. Cost: new code in the compositor's band path (a scaler, the non-integer transform
   accepted, sub-pixel phase per tile), another build/measure/review cycle; risk: the scaler's quality
   (text at 0.69 bilinear vs the GPU's sampling), its CPU time on a ~1 GHz ARM (80k px * 4 taps), and
   whether the compositor thread is even Wikipedia's bottleneck when fps flowing sits at ~10 regardless.
2. Engine switch ON, daemon OFF; no scaler. The long page (and any page at scale 1) gains; Wikipedia's
   bands stay on the GPU. Move on to the next tasks (T-563.. in evidence/build-site-rows.md).
3. Keep scale choice (b) (daemon switch ON too): fix the pinch settle (F5) and the zoom-back gates (F6),
   accept the dearer paints.
4. Both switches OFF; move on.

## Evaluation criteria

- Correctness and risk: which leaves the fewest failure modes on a 2011 device with one test bench?
  Consider the tile read-lock change under the engine switch (content side double-buffers: memory, any
  stall it could add) and what "inert elsewhere" really means.
- Performance against the kit (evidence/kit-gpu-rendering-R3-R6-R12.md): R3 wants 24 fps pans on Wikipedia.
  Which option moves Wikipedia towards it, and is the compositor thread actually the limiter there? Use
  the round notes' numbers (express made, no-slot waits, paint sums, the bottom grey band).
- Simplicity and maintenance (the kit's R11: the code is fit to keep).
- Opportunity cost: the remaining work queue (build-site rows) and the owner's usage budget.

## Output

1. Your pick: one number (1-4), on the first line as "PICK: N".
2. Why, with the evidence cited (file and line or table row). Challenge the builder's claims where the
   numbers do not support them.
3. The strongest case against your pick, and what measurement would break the tie if you are unsure.
4. If you pick 1, say what you would require of the scaler (quality and cost gates) before it ships ON.
5. Your second choice.

Keep it under ~700 words. Plain prose and short tables; no code.
