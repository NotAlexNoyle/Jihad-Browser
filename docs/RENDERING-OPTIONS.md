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

## The options (the report's table, in short)

1. Software tuning — NEON blend/scale, fewer copies: 40–45 ms frames, 8–12 fps scroll; days; all builds.
2. WebGL through EGL with readback — 15–20 fps for a full-screen GLES 2.0 canvas; 1–2 weeks.
3. Skia GPU canvas on 2's context — days more; readback per canvas.
4. OMTC on the GPU with readback (Atlas's shape) — 25–30 fps scroll/animation, 4–8 weeks, high risk.
5. Video through the MDP overlay (Atlas's `gst-mdpoverlay`) — decode-speed playback, 2–3 weeks.
6. Zero-copy GPU frames to the card — 40–60 fps ceiling, LunaSysMgr change, webOS CE only, on top of 4.
