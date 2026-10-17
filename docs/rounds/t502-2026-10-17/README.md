# T-502 — the GPU compositor spike on the TouchPad (2026-10-17)

The question: does the OpenGL compositor run in the headless daemon on the Adreno 220 through the
vendor EGL, and what does a composited frame cost on the device, against the 42 ms model of
`docs/RENDERING-OPTIONS.md`? Engine patch 0040 (`JIHAD_GPU_COMPOSITOR`), daemon export
`jihad_offscreen_compose`, Enyo variant, daemon started by hand with the env.

| Run | What | Result |
|---|---|---|
| 1 | first build | the bundle's `layers.offmainthreadcomposition.force-disabled=true` kept OMTC off; no compositor |
| 2 (`spike-run2-daemon-gpu.log`) | env wins over the pref | compositor up, backend 2 = OpenGL on an EGL pbuffer; frames 224–390 ms; SIGSEGV when a child widget destroyed the shared ClientLayerManager |
| 3 (`spike-run3-*`) | crash fixed, timings per stage | `glReadPixels(+convert)` 240–490 ms of it; frame upside down and cropped (the generic copy-to-target flips, the T-499 layout viewport was 980×1202) |
| gpu-bench | readback surfaces | glReadPixels from a TEXTURE-backed FBO (what the offscreen GLScreenBuffer is) 368–643 ms; from the pbuffer 12–43 ms; from a RENDERBUFFER FBO 12–28 ms; `GL_BGRA_EXT` reads refused (0x500); `EGL_KHR_lock_surface` lock+memcpy+unlock 26 ms (a copy inside, not a mapping) |
| 4 (`spike-run4a-*`, `spike-run4b-*`) | plain pbuffer context, direct read into the target, no flip | colours page upright and whole; frames 62–84 ms: glReadPixels 26–37, swizzle (plain C) 12–17, composite 1–5, snapshot road 12–18 |
| 5 (`spike-run5-*`) | NEON swizzle, a frame every tick | the four-layer animation page: the compositor moves the box with the main thread painting 0.1–2.8 ms a frame; frames 54–96 ms; the daemon's idle tick (100 ms) capped the pipeline at 7–9 fps |

Screenshots: `spike-run4a-colors.jpg` (upright, all four blocks), `spike-run4b-anim.jpg` and
`spike-run5-anim.jpg` (the fixed header, the half-transparent overlay, the box mid-slide, the rAF
counter). The spike's scripts are in the session scratchpad; the device is left with the Enyo job
running the round-5 engine (libxul `ad33ee4f`) and daemon (`ac4a08c5`) on the software path (no
env), T-499's pref line removed from the profile again.

## Rounds 31-32 (2026-10-17, end of session): the frames DO reach the adapter

- An adapter blit-time probe (`[hp-px]` in `<state>/adapter.log`, gated by `<state>/adapterlog`;
  the probing adapter is installed on the rootfs at `/usr/lib/jihad/enyo/BrowserAdapterImpl.so`,
  the stock one saved beside it as `.orig`) reads the daemon's content from the shared segment at
  blit time: red at (100,100) for the colours page, the animation page's pixels for that page
  (`spike-run31-*`). `blit=1`, `frozen=0`. The "third process sees white" result of the earlier
  rounds was a timing artefact: the zoom-1.0 frames are empty (`bytes=0`) and the content frames
  only start after the fit zoom to 0.7837; a `shmdump` of all three segments after that point
  showed the page in every one.
- The screenshots are still white because `gpu-spike-start.sh` shoots 14 s after the launch, which
  is BEFORE the first content frame on this path (the zoom change comes late). Next: shoot after
  the daemon log shows three `bytes>0` frames, and read the whole `adapter.log` for that run.
- The adapter's samples alternate between two layouts for the same page: "red at (100,100)" and
  "white at (100,100), green at (700,900)". The second is a zoom-1.0 frame either y-flipped or
  translated down by about 742 px; three samples cannot tell which. The daemon's first-content dump
  (`spike-run31-colors-frame.jpg`) is upright, so the flip or shift comes in a LATER frame. Dump a
  run of consecutive content frames (not only the first) to see which, then look at
  `CompositorOGL::PrepareViewport` (`IsOffscreen()` picks the un-flipped view matrix; the
  intermediate-surface copy at the "Drawing is always flipped" comments adds a flip that assumes
  the window draw was flipped) and at the world-transform offset.
- The engine branch head now also lets the borrowed renderbuffer FBO count as the window target
  (`CompositingRenderTargetOGL::IsWindow()`); bundle `1cb9d1dd` is on the Enyo card. Still a
  temporary commit on top of 0040/0041, not yet folded into the patch queue (pins disagree).
- With `JIHAD_GPU_NO_WORLD=1` (run 32) the adapter saw only white frames; unexplained, one run.
