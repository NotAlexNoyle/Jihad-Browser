---
created: "2026-10-18"
last_edited: "2026-10-18"
---

# Cavekit: GPU Rendering (the pick: 2 + 4)

## Scope
The page is composited on the TouchPad's GPU (the Adreno 220, through EGL) by the engine's own
OpenGL compositor running inside the headless daemon, and the finished frame is read back into the
YAP shared buffer the card already shows (option 4). WebGL then runs on the same GPU through EGL
(option 2). The BrowserAdapter↔BrowserServer contract stays byte-identical: the card receives the
same buffers and messages as from the software path, only faster. All three interfaces (Enyo,
Mochi, Mojo) get it; the software path stays as the fallback.

**User decisions:** 2026-10-17, the rendering report cut down to the survivors and *"the pick: 2 +
4"* (GPU compositor with readback, then WebGL through EGL), a one-week spike first (T-502); video
sometimes, 24 fps video acceptable, styled video matters (fades, rounded corners, video under a
modal), so the video overlay (5) is held back. 2026-10-17: *"dont treat the chart speed predictions
as the theoretical maximum, but the bare minimum goal, and keep the performance acceleration going
as far as you can"*. 2026-10-18, the owner's finger test on the GPU path: *"areas turn grey and zoom
rendering of page content lags behind actual page zoom state"*. 2026-10-18: *"continue using
cavekit 3.1 to finish the implementation"*.

**The chart's rows for 2 + 4 (the floor, not the target):** animation and in-band scroll 42 ms
(24 fps); new content 106 ms (9 fps); WebGL 42 ms (24 fps); video decode plus a 42 ms frame. The
whole account, the measurements and the traps: `docs/RENDERING-OPTIONS.md` ("T-502").

**What exists at the kit's creation (T-502, rounds 1-141):** engine patches 0040 (the compositor),
0041 (the Adreno 220's GL) and 0042 (compositor-side scroll, band reads, the band surface,
displayport, fixed/sticky rows, the composition fix); the daemon's job pipeline
(`render/goanna/BrowserPageGoanna.cpp` gpuJobStart/poll, `GoannaRenderPage.cpp` ComposeStart/Poll,
the wake pipe in `render/browserserver/Main.cpp`); behind `JIHAD_GPU_COMPOSITOR=1`, Enyo only,
started by hand (`tools/device/t502/`). Measured: the test pan 27-34 fps, the animation page 26-31,
Hacker News 36-37, desktop Wikipedia 13-14 (software 9-12.4).

## Requirements

### R1: The GPU path is the default, with the software path as its fallback
**Description:** The daemons of all three interfaces composite on the GPU unless it is unavailable.
**Acceptance Criteria:**
- [ ] `packaging/event.d/{jihad,jihad-mochi,jihad-mojo}` (or the daemon's own default) start the GPU path; `JIHAD_GPU_COMPOSITOR=0` (and a pref the bundle reads) forces the software path
- [ ] A daemon whose EGL setup fails (simulated by an env that makes the context creation fail) renders every page by the software path, and the startup report names the path taken
- [ ] No GPU wait is unbounded: a stuck job is abandoned after a bound and the page falls back to the software path for that daemon's life; LunaSysMgr never loses EGL because of the daemon (memory `gpu-wedge-needs-a-reboot`)
- [ ] Enyo, Mochi and Mojo each load, pan, zoom and close a page on the GPU path on the bench (CE 3.1.0)
**Dependencies:** offscreen-rendering R1-R6, device-build R4

### R2: Every frame is right
**Description:** What the card shows on the GPU path equals a whole composite of the page at the frame's scroll and zoom.
**Acceptance Criteria:**
- [ ] `JIHAD_GPU_VERIFY=1` reports 0 differing rows over a scripted pan on `jihad-long-static.html` (fixed header), `jihad-anim-css.html`, `jihad-scroll-js.html`, a page with a fixed header AND a fixed footer, desktop en.wikipedia.org (stuck sticky sidebars) and news.ycombinator.com
- [ ] The mid-pan frame dumps of each are looked at and match the page (memory `verify-device-frames-by-eye`)
- [ ] A page laid out wider than the card (fit zoom below 1) shows its right side and bottom
**Dependencies:** R1

### R3: Pans meet the chart, on real pages too
**Description:** A drag-scroll moves the content by compositing the painted displayport, not by repainting.
**Acceptance Criteria:**
- [ ] The scripted pan (0 → 3000 card px in 10 s, `tools/device/t502/pan-run.sh`) runs at 24 fps or better on `jihad-long-static.html` and on desktop en.wikipedia.org after its load
- [ ] On every page measured, the GPU path's pan is at least as fast as the software path's on the same pan (`NOGPU=1`), and the table in `docs/RENDERING-OPTIONS.md` says so per page
- [ ] Frame gaps of 56 ms or more during a pan are accounted for (traced to their source) and the avoidable ones removed
**Dependencies:** R2

### R4: Fixed and sticky content costs little
**Description:** Fixed headers and footers and stuck sticky elements stay put and are not re-read whole every pan step.
**Acceptance Criteria:**
- [ ] An opaque fixed or stuck layer that held still keeps its pixels from the previous frame; only the rows the shift uncovers beside it are read
- [ ] A non-opaque one is read as its columns, not as full rows
- [ ] A page with a fixed header and a fixed footer reads two bands, not the whole frame
- [ ] R2's verify passes on each case
**Dependencies:** R2

### R5: Animation and new content meet the chart
**Acceptance Criteria:**
- [ ] `jihad-anim-css.html` animates at 24 fps or better (only the changed rows composed and read)
- [ ] A content change (a DOM edit on a test page, typing into a field) reaches the card within 106 ms
- [ ] A page that scrolls itself (`jihad-scroll-js.html`) is followed at 9 fps or better
**Dependencies:** R2

### R6: Zoom keeps up (the owner's finger test, 2026-10-18)
**Description:** A pinch or a double-tap zoom shows the page at the new zoom promptly and never leaves part of the card grey.
**Acceptance Criteria:**
- [x] A scripted pinch in to about 1.7 and back out to the fit zoom (and a double-tap zoom) on desktop en.wikipedia.org and on `jihad-long-static.html` is reproduced on the bench with the frame timeline logged *(2026-10-18: the card's own gesture calls through its test hook, `g` pinches and `t` double taps; Wikipedia 0.69 → 1.23 → 2.20 → 1.51 → 1.03, double tap 1.26 and back to 0.69; the test page 0.79 → 1.41 → 2.51 → 1.72 → 1.18 and its double taps; each zoom's "the first frame at the new zoom is out" line)*
- [ ] The first frame at the new zoom reaches the card within 250 ms of the zoom command, and the frames after it keep the pan rate of R3 *(2026-10-18, PARTIAL: on a settled page 52-217 ms for pinches, 61-124 ms for a double tap in and 182-257 ms back out, one double tap in at 384 and 414 ms in two runs of ten; a pan at 1.23 runs 18.5-28 fps, the fit zoom's rate. While the page's own script holds the main thread (desktop Wikipedia's modules for ~10 s after its load) the daemon cannot even read the zoom command: 0.8-1.7 s. That part waits on frames made off the main thread, with T-505.)*
- [x] No area of the card is left grey or blank at rest: within 500 ms of the gesture's end every row of the card has content at the new zoom (dumps looked at) *(2026-10-18: `JIHAD_GPU_ZOOM_DUMP` keeps the frame the card shows at rest; the last frame of each zoom came 344-612 ms after it and covers the card: Wikipedia at 1.23, 2.20, 1.51, a double tap at 1.26, the test page at 2.51 and 1.72. Zoomed in, the settle had published frames at the engine's x under a card at another x, a grey strip down the right; fixed)*
- [x] Tile uploads do not each wait behind queued GPU work (the Adreno 220 serialises an upload with pending GPU work: 12-63 ms per 256x256 tile seen) *(2026-10-18: the wait was the driver converting a texture on its first write after its first draw, ~18 ms per 256x256, once per texture (`gpu-bench.c` sections 11-16); engine patch 0044 converts each new tile texture at 1x1 (1.1-2 ms) and pools textures: re-uploads slower than 8 ms from 55-59 a Wikipedia zoom run to one)*
- [x] Zoomed in, the page is painted at the zoom (text as sharp as the software path's), not a 1x paint magnified *(added 2026-10-18 from the owner's "zoom rendering of page content lags behind actual page zoom state": above a zoom of 1.1 the pres shell's resolution follows the zoom; Wikipedia at 2.2 compared with the 1x paint, crisp and aligned to the pixel)*
**Dependencies:** R2

### R7: Video and plugins on the GPU path
**Acceptance Criteria:**
- [ ] `<video>` (VP8, Theora, H.264) plays to the end on the GPU path with its frames composited (the ImageBridge), at the video's own rate up to 24 fps, with no frame the software path would not show
- [ ] Styled video works: rounded corners, an opacity fade, video under a modal (`render/goanna/test/` pages)
- [ ] Flash (NPAPI windowless) draws, animates and takes input on the GPU path at its own bounds
**Dependencies:** R2, gre-widgets (media), addons-extensions R7

### R8: WebGL through EGL (option 2)
**Description:** WebGL 1 contexts are created on the device's GPU and composited without a CPU round trip per WebGL frame.
**Acceptance Criteria:**
- [ ] On the GPU path `canvas.getContext("webgl")` succeeds on the bench; on the software path WebGL stays off
- [ ] A WebGL test page (a spinning textured cube, `render/goanna/test/`) renders correctly and runs at 24 fps or better
- [ ] Context loss (the card closed, the GPU reset) is handled without a daemon crash; a WebGL page cannot wedge the GPU path (R1's bound)
- [ ] WebGL conformance spot checks (the basic ones a page needs: textures, framebuffers, shaders) pass, and what does not is listed
**Dependencies:** R1

### R9: Everything else the card does still works on the GPU path
**Acceptance Criteria:**
- [ ] Rotation (portrait ↔ landscape) and a resize redraw the page right
- [ ] Select popups, the text caret, link highlights, find-in-page highlights and the card thumbnail show as on the software path
- [ ] Loading a new page, going back and forward, and reloading show the new content with no stale frame
**Dependencies:** R2

### R10: It fits the device
**Acceptance Criteria:**
- [ ] The daemon's resident memory and the GPU memory with a page on the GPU path are measured against the software path and recorded; the three daemons on the GPU path at once run without running the device out of memory or GPU memory
- [ ] An idle page on the GPU path uses no more CPU than on the software path (no compositing while nothing changes)
**Dependencies:** R1

### R11: The code is fit to keep
**Acceptance Criteria:**
- [ ] The `[jihad-gpu]` diagnostics left are bounded and off by default; the verify mode and the frame dumps stay (they are the test); every `JIHAD_GPU_*` knob is listed in `tools/device/t502/README.md`
- [ ] The engine work is folded into the patch queue (`check-engine-pins.sh` and `patch-queue.sh --check` agree) and `docs/ENGINE-SOURCE.md` names each patch
- [ ] A review (Codex by direct invocation, the inspector as backup) of the engine and daemon changes, every finding dispositioned
- [ ] `docs/RENDERING-OPTIONS.md`, `docs/PICKUP.md` and the README say what the GPU path does and what it measured
**Dependencies:** R1-R10

### R12: The owner's finger round
**Acceptance Criteria:**
- [ ] The owner pans, flicks, pinches and double-taps real pages on the GPU path in all three interfaces (`docs/FINGER-TESTS.md`) and finds it smooth
**Dependencies:** R1-R10 (BLOCKED-EXTERNAL: a person at the device)

## Non-goals (this version)
- The video overlay (option 5) and a LunaSysMgr change (option 6): held back by the owner's pick.
- WebGL 2.
- The TouchPad Go (opal): no unit on the bench; the GPU path is untested there.
- Stock webOS 3.0.5: the bench runs CE 3.1.0 only (ask the owner before stock-only work).

## References
- `docs/RENDERING-OPTIONS.md` — the options, the chart, the pick, T-502's rounds and traps
- `docs/PICKUP.md` — 2026-10-17 delta
- `tools/device/t502/README.md` — the device scripts and knobs
- memories: `compositor-scroll-traps`, `gpu-wedge-needs-a-reboot`, `verify-pans-by-dumps`, `verify-device-frames-by-eye`, `owner-tests-clash-with-pan-runs`

## Changelog
- 2026-10-18: created from T-502 (rounds 1-141) and the owner's finger test; R1-R12.
- 2026-10-18: R6 gains the sharpness criterion (the zoom's resolution) after T-504's rounds 162-186.
