# Rendering options for Jihad — what the TouchPad measured (2026-10-17)

The report the owner reads is the Claude doc "Jihad rendering options: speed comparison"; this page
keeps the measurements in the repository with the tools that took them. Every number is from the
TouchPad (webOS 3.0.5, APQ8060, Adreno 220).

## The software pipeline as it runs (the daemons' logs, 1958 paints since a boot)

| Stage | Cost per 1024×768 frame |
|---|---|
| Paint the document into the headless widget's buffer (cairo/skia on the CPU) | 62–73 ms per megapixel: screen-sized band median 53 ms (p90 182), full-page band median 120 ms (p90 274), worst 2.4–4.4 s |
| Copy into the YAP shared memory (3 MB) | 4–5 ms |
| The card's blit with the nearest-neighbour fit scale | 8 ms scaled, 2–4 ms at 1:1 |
| LunaSysMgr's composite (GPU) | ~4 ms |

Frame gaps while painting: 35% under 16 ms, 1% 16–23, 6% 24–31, 16% 32–39, 17% 40–55, 24% over 56.
The caps in `BrowserPageGoanna.cpp`: one dirty repaint per 150 ms, one pan repaint per 250 ms.

## The GPU (`tools/device/gpu-bench.c`, vendor EGL 1.4 / OpenGL ES 2.0, two runs)

| Stage | run 1 | run 2 |
|---|---|---|
| EGL + pbuffer + shader setup | 135 ms | 82 ms |
| Upload a 1024×768 RGBA texture (3 MB) | 12.1 ms | 8.8 ms |
| Composite one opaque full-screen layer | 3.8 ms | 3.7 ms |
| Composite four layers, three blended | 13.5 ms | 12.9 ms |
| Twelve 256×256 `glTexSubImage2D` updates (3 MB) + composite | 169 ms | 158 ms |
| `glReadPixels` 1024×768 RGBA | 14.4 ms | 11.3 ms |
| `glReadPixels` 640×480 / 320×240 (a Flash movie's rectangle; a third run, 2026-10-17) | 4.9 / 1.9 ms | 6.3 / 2.3 ms |
| Upload a 640×480 / 320×240 RGBA texture (a movie's own frame; the same third run) | 1.85 / 0.59 ms | 1.82 / 0.53 ms |
| Whole readback frame (12 tile updates + 2 layers + read) | 45 ms | 64 ms |
| CPU `memcpy` 3 MB | 5.0 ms | 4.2 ms |
| CPU blend of one 1024×768 ARGB layer (plain C) | 22.5 ms | 19.7 ms |
| CPU nearest-neighbour scale 1000→768 wide | 8.2 ms | 8.1 ms |

`GL_EXT_texture_format_BGRA8888` present, `GL_EXT_read_format_bgra` absent. The one pathological
path is the sub-image tile update; whole-texture uploads run at 250–340 MB/s.

## What the other UXP browsers do (the platform source, `third_party/uxp`)

`gfxPlatform::AccelerateLayersByDefault()` (`gfx/thebes/gfxPlatform.cpp:2126`) is true on Windows,
macOS and GTK: Pale Moon and Basilisk composite on the GPU by default (Direct3D 11 / OpenGL / OpenGL
over GLX), rasterize content with Direct2D (Windows), CoreGraphics (macOS) or cairo (Linux), draw
canvas with Direct2D, Skia-on-GL (`gfx.canvas.azure.accelerated`, macOS) or Skia on the CPU
(Linux), and have WebGL on through WGL/ANGLE, CGL or GLX. The headless toolkit Jihad builds gets
`gfxPlatformHeadless` (cairo, software layers) and the `Null` GL provider. The application pref
files on repo.palemoon.org sit behind Anubis and were not read.

## The shortlist, fastest first (the report's chart)

Every combination that adds options without adding speed in any column is cut (rows with 3, rows
with 1 on top of 4), and so are the options advised against: 2 without 4 (WebGL at 11–13 fps through
a readback and a software blend, below a page's WebGL-off path) and 6 (a patched LunaSysMgr, so
webOS CE 3.1.0 only; a frame path beside the YAP delivery this project keeps byte-identical; 5 weeks
on top of 4's 6 with the daemon-to-LunaSysMgr buffer share unmeasured; animation only, 42 to 17 ms,
new content stays at the 80 ms raster). Sorted by the columns left to right. The 44 rows behind the
cut follow. Flash games: 27 fps is a 30 fps 320×240 movie and 22 fps a 640×480 one, Flash's own
rate on this CPU (the plugin child; the daemon's paints keep up at 4–5 ms, 30–34 fps composite,
2026-10-07); no option touches it. On the 4 rows a whole-frame readback (12.3 ms) would cap 320×240
at 23 fps; the movie's rectangle read back (1.9–6.3 ms measured) keeps 27. The report colours the
cells (green ≥ 24 fps, yellow 16–23, red ≤ 13; weeks 0–1 green, 2.5–3.5 yellow, 6–7.5 orange, 8.5+ red).

| # | Solution (option numbers) | Animation and in-band scroll | New content | WebGL games | Flash games | Video | Weeks | Like Pale Moon / Basilisk |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 2 + 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | 27 / 22 fps with the movie's rect read back; 23 / 22 whole-frame | decode only (overlay) | 10 | their compositor + a readback, their WebGL |
| 2 | 2 + 4 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | 27 / 22 fps with the movie's rect read back; 23 / 22 whole-frame | decode + a 42 ms frame | 7.5 | their compositor + a readback, their WebGL |
| 3 | 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | none | 27 / 22 fps with the movie's rect read back; 23 / 22 whole-frame | decode only (overlay) | 8.5 | their compositor + a readback |
| 4 | 4 | 42 ms, 24 fps | 106 ms, 9 fps | none | 27 / 22 fps with the movie's rect read back; 23 / 22 whole-frame | decode + a 42 ms frame | 6 | their compositor + a readback |
| 5 | 1 + 5 | 63 ms, 16 fps | 116 ms, 9 fps | none | 27 / 22 fps, Flash's own rate | decode only (overlay) | 3.5 | no equivalent |
| 6 | 1 | 63 ms, 16 fps | 116 ms, 9 fps | none | 27 / 22 fps, Flash's own rate | decode + a 63 ms frame | 1 | no equivalent |
| 7 | 5 | 80 ms, 13 fps | 132 ms, 8 fps | none | 27 / 22 fps, Flash's own rate | decode only (overlay) | 2.5 | no equivalent |
| 8 | today | 80 ms, 13 fps | 132 ms, 8 fps | none | 27 / 22 fps, Flash's own rate | decode + a 80 ms frame | 0 | their software fallback |

## Every combination, fastest first (the data behind the shortlist)

The frame modeled is four full-screen layers, three blended; the CPU pays three blends (20.8 ms plain,
15.4 ms NEON each), the GPU one composite (12.9 ms); "new content" adds the 53 ms CPU raster and, for
the GPU rows, the 10.3 ms upload; a readback row adds 12.3 ms, the copy 5.1 ms, the card's scale 8.2 ms,
LunaSysMgr's composite 3.8 ms. 3 needs 2 or 4; 6 needs 4 (webOS CE only). Weeks add per option.

| # | Solution (option numbers) | Animation and in-band scroll | New content | WebGL | Video | Weeks | Builds | Like Pale Moon / Basilisk |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 4 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | none | decode + a 17 ms frame | 11 | CE 3.1.0 only | their compositor, their presentation |
| 2 | 3 + 4 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | none | decode + a 17 ms frame | 11.5 | CE 3.1.0 only | their compositor, their canvas (macOS), their presentation |
| 3 | 1 + 4 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | none | decode + a 17 ms frame | 12 | CE 3.1.0 only | their compositor, their presentation |
| 4 | 2 + 4 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | 17 ms, 60 fps | decode + a 17 ms frame | 12.5 | CE 3.1.0 only | their compositor, their WebGL, their presentation |
| 5 | 1 + 3 + 4 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | none | decode + a 17 ms frame | 12.5 | CE 3.1.0 only | their compositor, their canvas (macOS), their presentation |
| 6 | 2 + 3 + 4 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | 17 ms, 60 fps | decode + a 17 ms frame | 13 | CE 3.1.0 only | their compositor, their WebGL, their canvas (macOS), their presentation |
| 7 | 4 + 5 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | none | decode only (overlay) | 13.5 | CE 3.1.0 only | their compositor, their presentation |
| 8 | 1 + 2 + 4 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | 17 ms, 60 fps | decode + a 17 ms frame | 13.5 | CE 3.1.0 only | their compositor, their WebGL, their presentation |
| 9 | 3 + 4 + 5 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | none | decode only (overlay) | 14 | CE 3.1.0 only | their compositor, their canvas (macOS), their presentation |
| 10 | 1 + 2 + 3 + 4 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | 17 ms, 60 fps | decode + a 17 ms frame | 14 | CE 3.1.0 only | their compositor, their WebGL, their canvas (macOS), their presentation |
| 11 | 1 + 4 + 5 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | none | decode only (overlay) | 14.5 | CE 3.1.0 only | their compositor, their presentation |
| 12 | 2 + 4 + 5 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | 17 ms, 60 fps | decode only (overlay) | 15 | CE 3.1.0 only | their compositor, their WebGL, their presentation |
| 13 | 1 + 3 + 4 + 5 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | none | decode only (overlay) | 15 | CE 3.1.0 only | their compositor, their canvas (macOS), their presentation |
| 14 | 2 + 3 + 4 + 5 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | 17 ms, 60 fps | decode only (overlay) | 15.5 | CE 3.1.0 only | their compositor, their WebGL, their canvas (macOS), their presentation |
| 15 | 1 + 2 + 4 + 5 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | 17 ms, 60 fps | decode only (overlay) | 16 | CE 3.1.0 only | their compositor, their WebGL, their presentation |
| 16 | 1 + 2 + 3 + 4 + 5 + 6 | 17 ms, 60 fps | 80 ms, 12 fps | 17 ms, 60 fps | decode only (overlay) | 16.5 | CE 3.1.0 only | their compositor, their WebGL, their canvas (macOS), their presentation |
| 17 | 4 | 42 ms, 24 fps | 106 ms, 9 fps | none | decode + a 42 ms frame | 6 | all | their compositor + a readback |
| 18 | 3 + 4 | 42 ms, 24 fps | 106 ms, 9 fps | none | decode + a 42 ms frame | 6.5 | all | their compositor + a readback, their canvas (macOS) |
| 19 | 1 + 4 | 42 ms, 24 fps | 106 ms, 9 fps | none | decode + a 42 ms frame | 7 | all | their compositor + a readback |
| 20 | 2 + 4 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | decode + a 42 ms frame | 7.5 | all | their compositor + a readback, their WebGL |
| 21 | 1 + 3 + 4 | 42 ms, 24 fps | 106 ms, 9 fps | none | decode + a 42 ms frame | 7.5 | all | their compositor + a readback, their canvas (macOS) |
| 22 | 2 + 3 + 4 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | decode + a 42 ms frame | 8 | all | their compositor + a readback, their WebGL, their canvas (macOS) |
| 23 | 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | none | decode only (overlay) | 8.5 | all | their compositor + a readback |
| 24 | 1 + 2 + 4 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | decode + a 42 ms frame | 8.5 | all | their compositor + a readback, their WebGL |
| 25 | 3 + 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | none | decode only (overlay) | 9 | all | their compositor + a readback, their canvas (macOS) |
| 26 | 1 + 2 + 3 + 4 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | decode + a 42 ms frame | 9 | all | their compositor + a readback, their WebGL, their canvas (macOS) |
| 27 | 1 + 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | none | decode only (overlay) | 9.5 | all | their compositor + a readback |
| 28 | 2 + 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | decode only (overlay) | 10 | all | their compositor + a readback, their WebGL |
| 29 | 1 + 3 + 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | none | decode only (overlay) | 10 | all | their compositor + a readback, their canvas (macOS) |
| 30 | 2 + 3 + 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | decode only (overlay) | 10.5 | all | their compositor + a readback, their WebGL, their canvas (macOS) |
| 31 | 1 + 2 + 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | decode only (overlay) | 11 | all | their compositor + a readback, their WebGL |
| 32 | 1 + 2 + 3 + 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | decode only (overlay) | 11.5 | all | their compositor + a readback, their WebGL, their canvas (macOS) |
| 33 | 1 | 63 ms, 16 fps | 116 ms, 9 fps | none | decode + a 63 ms frame | 1 | all | no equivalent |
| 34 | 1 + 2 | 63 ms, 16 fps | 116 ms, 9 fps | 76 ms, 13 fps | decode + a 63 ms frame | 2.5 | all | their WebGL |
| 35 | 1 + 2 + 3 | 63 ms, 16 fps | 116 ms, 9 fps | 76 ms, 13 fps | decode + a 63 ms frame | 3 | all | their WebGL, their canvas (macOS) |
| 36 | 1 + 5 | 63 ms, 16 fps | 116 ms, 9 fps | none | decode only (overlay) | 3.5 | all | no equivalent |
| 37 | 1 + 2 + 5 | 63 ms, 16 fps | 116 ms, 9 fps | 76 ms, 13 fps | decode only (overlay) | 5 | all | their WebGL |
| 38 | 1 + 2 + 3 + 5 | 63 ms, 16 fps | 116 ms, 9 fps | 76 ms, 13 fps | decode only (overlay) | 5.5 | all | their WebGL, their canvas (macOS) |
| 39 | today | 80 ms, 13 fps | 132 ms, 8 fps | none | decode + a 80 ms frame | 0 | all | their software fallback |
| 40 | 2 | 80 ms, 13 fps | 132 ms, 8 fps | 92 ms, 11 fps | decode + a 80 ms frame | 1.5 | all | their WebGL |
| 41 | 2 + 3 | 80 ms, 13 fps | 132 ms, 8 fps | 92 ms, 11 fps | decode + a 80 ms frame | 2 | all | their WebGL, their canvas (macOS) |
| 42 | 5 | 80 ms, 13 fps | 132 ms, 8 fps | none | decode only (overlay) | 2.5 | all | no equivalent |
| 43 | 2 + 5 | 80 ms, 13 fps | 132 ms, 8 fps | 92 ms, 11 fps | decode only (overlay) | 4 | all | their WebGL |
| 44 | 2 + 3 + 5 | 80 ms, 13 fps | 132 ms, 8 fps | 92 ms, 11 fps | decode only (overlay) | 4.5 | all | their WebGL, their canvas (macOS) |

## The options, in short

1. Software tuning — NEON blend/scale, fewer copies: 40–45 ms frames, 8–12 fps scroll; days; all builds.
2. WebGL through EGL with readback — 15–20 fps for a full-screen GLES 2.0 canvas; 1–2 weeks.
3. Skia GPU canvas on 2's context — days more; readback per canvas.
4. OMTC on the GPU with readback (Atlas's shape) — 25–30 fps scroll/animation, 4–8 weeks, high risk.
5. Video through the MDP overlay (Atlas's `gst-mdpoverlay`) — decode-speed playback, 2–3 weeks.
6. Zero-copy GPU frames to the card — 40–60 fps ceiling, LunaSysMgr change, webOS CE only, on top of 4.
