# Review findings — T-505 the parts on the CPU (2026-10-26, evening)

Two reviews of daemon `2bd3e6f1` and engine `d14fbe3f68` (0070) / `ffb0cb8ae1` (0071), by the ck:inspector (Codex
was out of credits until 14:59; the inspector is Cavekit 3.1's backup): a code review, and the adversarial design
review the owner asked for on the stale-picture trade ("adversarial review with cavekit 3.1 backup inspector to
address this"). Verdict of both: REVISE. The owner had chosen CPU parts on by default once the findings are fixed;
until they are, the default stays off.

## The trade (design review)

Neither option is exact: without the switch a frame posted before a change but composed after it is still dressed
with the old picture (no check at the poll), and "at most 600 ms" is not bounded (a picture that arrives stale
restarts the clock). Recommended third option (**E2**): the parts' picture composed and read **inside the frame's
own compositor job** whenever the parts changed since the picture the daemon holds — the parts alone, cleared
transparent, only the parts' rects read (packed as chunks, one read) — so every frame is dressed from the same tree
state; the daemon keeps the last 2-3 pictures keyed by generation and dresses each frame with its own. Exact, and
expected at least as fast as the stale switch (the separate picture job, 55-250 ms in the queue, and the express
lane's lost base after it both go). Prerequisite: a change signal that covers more than paints (a parts signature).
Rejected: cheap kind changes (a kind-0 band frame on Wikipedia costs about a whole read), a per-layer hide set (high
effort, E2 with per-part signatures does it cheaper), dropping frames newer than the picture (stalls).

## Findings

| # | sev | where | finding | fix | status |
|---|---|---|---|---|---|
| C1 | high | daemon xStart (express second frame) | with parts on, the express lane's second frame copies the first frame's slot while the express thread dresses it: a ghost strip of a part | no second frame on the first without parts (as the main lane) | |
| C2/D1 | medium | daemon cpuPartsApply | the picture is checked at the post, used at the poll: another zoom's picture, or one older than the frame's generation, can dress it | stamp the job with the picture's serial; at the poll a frame whose generation is not the picture's is not shown (not counted a miss) | |
| D3 | medium | daemon partsGenSeen | the generation can move backwards (an older poll after a newer one) and mark a stale picture current | ignore an older generation | |
| D2 | medium | daemon partsGenSeen / cpuPartsStep | the stale bound restarts with each stale picture | (moot with E2; the switch goes) | |
| D4 | high | engine JihadColumnRead; daemon restore | a column read in a frame without its parts: the restore of what lay beneath treats the band as read whole rows; old part pixels kept (a part blended twice, a ghost strip later) | no column reads without the parts | |
| D5 | medium | daemon cpuPartsStep | the picture's post sets mXLastPostToken: the express lane loses its base after every picture | (moot with E2) | |
| C3/D6 | medium | engine CompositableTransactionParent | the generation moves only on paints: opacity/transform/colour/clip set in a transaction, compositor animations, structure changes inside a part are missed (a fading button stays as pictured) | a parts signature computed in the composite (E2) | |
| C4 | medium | engine | content stacked above a part (a popup over a fixed bar) is drawn under the CPU-drawn part | the engine flags a non-part leaf above and over a part; such a frame is composed with the engine's parts | |
| C5 | medium | engine stuck block | in a frame without its parts the card's hold set (T-544) is every sticky layer, stuck or not, merged: a not-yet-stuck one held still between frames | report the hold set (fixed + stuck) apart from the parts | |
| C6 | medium | engine 0071 | in frames WITH the engine's parts and fixed layers, an own-slot frame's diff-first plan keeps rects judged against the last composite, not the older base | the compensation only without parts or with no fixed/stuck rects | |
| D7 | medium | daemon match | parts matched by columns, height and nearest y: same-size sticky headers in one column (GitHub's file headers) can take each other's pixels | (E2: the frame's own rects and picture come from one composite) | |
| C7 | low | daemon main poll | the slot's kind recorded at the publish, not with the record: the blank-suppression return leaves a valid slot with the old kind and beneath | record the kind at adoption | |
| C8 | low | daemon cpuPartsApply | a gone base record (after a forget or a loss) counted as a miss: old-document jobs can turn parts off for a new page | a separate outcome, not counted | |
| C9 | low | daemon cpuPartsStep | the clip flags read cr.stuck with the compacted index | the original index | |
| D8 | low | daemon cpuPartsApply | a part cut at both frame edges takes the wrong rows | (E2) | |
| C10/D9 | low | daemon main poll | the main lane dresses before its out-of-page white fill (the express lane after) | the same order | |
| — | — | daemon cpuPartsApply | (found while waiting) a frame's part 2 px off the picture's read past the part's block or before its row | rows and columns both have only | fixed 2026-10-26 |

Checked and fine (code review): the restore offset for every kind of base; the scratch's graves; `ownNotWhole`
(speed only); the poll's thirtieth int and flag 128 only with an engine that takes them; the main lane's round-130
exclusion; dropped, missed and adopted jobs' records; 0071's compensation for content (diff in current coordinates,
the verify guard's state, a page-made scroll is whole). No path publishes a frame without its parts undressed.

Proposed criterion (cavekit-gpu-rendering.md): every published frame's CPU-drawn parts come from a composite of the
same tree state as the frame; the change signal covers content, attribute, structure and animation changes to a
part; `JIHAD_GPU_VERIFY=1` with the parts on shows zero wrong rows over a scripted pan on Wikipedia and the long page.
