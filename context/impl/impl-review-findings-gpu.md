---
created: "2026-10-19"
last_edited: "2026-10-19"
---

# Review — the GPU path's T-511 and T-517 work (2026-10-19)

Build site: context/plans/build-site.md

`ck:inspector`, read-only, on the main-repo commits `bf26d622` (YapProxy locks), `8518970c` (T-511:
WebGL, the column read), `e0416cbc` (no cache clean on write-through segments) and `4b8a595b` (frame
windows), with the engine commits they pin (`6ea0ae9f54`, `8e117bab07`, `a563423cac`). An interim
review ahead of T-513 (Tier 87's review, Codex by direct invocation with the inspector as backup):
the frame windows reach into the adapter (LunaSysMgr's process) and change how both frame lanes
keep their records, so they were reviewed before more work went on top.

| # | Severity | Where | Finding | Disposition |
|---|---|---|---|---|
| 1 | P1 wrong pixels, a race | `BrowserPageGoanna.cpp` `xStart` (the express lane's own-slot path, its slot choice) | The express lane built on its slot's own frame while a main-lane job was composed but not yet polled (its change not yet in any slot's record), and could pick the slot a main-lane job was still being read into (the slot still marked valid from its older publish; a recentring move there would race the read) | Fixed: the own path only with no main-lane job in flight; the express lane never takes a slot a main-lane job holds; a slot is marked invalid when a job (either lane) is posted into it, and its publish records it again |
| 2 | P1 wrong pixels | engine `CompositorOGL.cpp` `JihadShiftRows`, the window-moved branch | Kept rects copied one at a time (saved, then written, before the next was read): stacked sticky bars got each other's rows | Fixed (engine 0054): every rect saved first, then all written |
| 3 | P1/P2 likely wrong pixels | own-slot bases, both lanes | The engine judges kept rects and fixed columns against the previous composite; an own-slot base is older, so a sticky layer that stuck between them (no tree change) could leave a ghost | Fixed by narrowing: an own-slot base after a scroll only when neither the latest composite nor the slot's own frame had fixed or stuck layers (`mGpuSlotFixed`). Pages with a fixed header go back to the copy from the newest slot (the long test page's median frame gap 26 -> 30 ms) |
| 4 | P2 wrong pixels | main lane, a job that fails after composing | Only the active slot was invalidated; the other slots' records missed the composite's change | Fixed: `gpuSlotsLost()` on any failed main-lane job |
| 5 | P2 | recentring before a post that fails | The rows moved, the slot stayed valid with its old window | Fixed: the slot is marked invalid before the move (both lanes) |
| 6 | P2 | WebGL after the daemon gives the GPU path up | `JIHAD_GPU_COMPOSITOR` stays set, so WebGL went on making EGL contexts on an abandoned GPU | Fixed (engine 0054): `jihad_offscreen_gpu_given_up()` from the daemon's fallback; WebGL's EGL choice and the headless GfxInfo ask `jihad_gpu_compositor_on()` |
| 7 | trivial | adapter `BrowserOffscreen::windowBytes()` | The trailer's window read twice | Fixed: read once (the compiled adapter came out the same) |

Found correct by the review: the YapProxy lock pairing and tag counter; the header at each segment's
start; no frame rows reaching the trailer on any budget (software path, overscan, express lane);
the adapter's bounds and fallback to window 0; no software repaint on a windowed GPU frame; the
recentring moves inside the segment; the window-moved shift's source rows; the column read's stale
columns; the express publish's records; the stale bookkeeping's failure paths; WebGL's share root,
context release and pbuffer readback.

Validation after the fixes (rounds 339-340): the default checks of Enyo, Mochi and Mojo pass;
verify mode on a main-lane Wikipedia pan, 30 frames after the pan began, 0 structurally wrong;
Wikipedia's express pan median frame gap 43 ms (unchanged).
