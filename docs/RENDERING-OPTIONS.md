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
`gfxPlatformHeadless` (cairo, software layers); its `GLContextProvider` is `GLContextProviderEGL`
(`gfx/gl/GLContextProvider.h`: the default when neither `MOZ_X11` nor `MOZ_GL_PROVIDER` is set),
loading `libEGL.so` / `libGLESv2.so` by name, but `AccelerateLayersByDefault()` is false without
`MOZ_GL_PROVIDER` and the daemon runs with `JIHAD_DISABLE_OMTC=1`, so no GL is ever created. The application pref
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
cells (green ≥ 24 fps, yellow 16–23, red ≤ 13; weeks 0–1 green, 2.5–3.5 yellow, 6–7.5 orange, 8.5+ red). Every row runs on webOS 3.0.5 and webOS CE 3.1.0: the one CE-only option, 6, is cut. Rows that scroll
slower than 24 fps (1 + 5, 1, 5, today) are cut too, and then
the rows without 2 (4, 4 + 5).

| # | Solution (option numbers) | Animation and in-band scroll | New content | WebGL games | Flash games | Video | Weeks | webOS | Like Pale Moon / Basilisk |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 2 + 4 + 5 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | 27 / 22 fps with the movie's rect read back; 23 / 22 whole-frame | decode only (overlay) | 10 | 3.0.5 and CE 3.1.0 | their compositor + a readback, their WebGL |
| 2 | 2 + 4 | 42 ms, 24 fps | 106 ms, 9 fps | 42 ms, 24 fps | 27 / 22 fps with the movie's rect read back; 23 / 22 whole-frame | decode + a 42 ms frame | 7.5 | 3.0.5 and CE 3.1.0 | their compositor + a readback, their WebGL |

### What tells them apart

Both share option 4's frame and option 2's WebGL: 24 fps scroll, animation and WebGL, 9 fps new
content, Flash at its own rate; they differ only in video and 2.5 weeks.

| Solution | Adds | Costs | Risk | Pick it when |
| --- | --- | --- | --- | --- |
| 2 + 4 | the GPU compositor (CSS animation, fixed headers, in-band scroll at 24 fps; 13 today, capped to 4–6.7 by the repaint limits) and WebGL at the same 24 fps as one more layer (nothing today); video at 24 fps, a 42 ms frame per video frame | 7.5 weeks | high: `CompositorOGL` over `GLContextProviderEGL` has never run in the headless toolkit or on an Adreno 220, every frame is read back, and the Adreno 220's GLES 2.0 driver meets WebGL 1 (Atlas turned WebGL off after ANGLE whited out its compositor, a different stack) | video is occasional |
| 2 + 4 + 5 | video at decode speed through the MDP overlay (a 30 fps clip at 30), the compositor and CPU freed of it | 2.5 weeks more (10) | the above, plus an overlay plane: no CSS transform, opacity or clip over the video, the card cuts a hole and moves the rectangle as the page scrolls; Atlas ships the sink (`gst-mdpoverlay`) | YouTube and embedded clips matter |

### The pick (2026-10-17) and the engine patches it touches

Picked: **2 + 4**, a one-week spike first (the OpenGL compositor up in the headless daemon on the
device, one real frame read back into the YAP buffer and measured), then 4, then 2. Video stays a
compositor layer at 24 fps so styled video keeps working; 5 is held back as a full-screen-only
overlay if 24 fps full-screen displeases by eye.

What 2 + 4 is in the engine: off-main-thread compositing on (`gfxPlatform::UsesOffMainThreadCompositing`),
the offscreen widget handing out a `ClientLayerManager` and creating an in-process compositor
(`nsBaseWidget::CreateCompositor` → `GPUProcessManager::CreateTopLevelCompositor`), `CompositorOGL`
on an EGL pbuffer context (`MOZ_LAYERS_PREFER_OFFSCREEN` → `GLContextProvider::CreateOffscreen`),
and the frame fetched with `CompositorBridgeParent::ForceComposeToTarget` into a `DrawTarget` over
the shared buffer (`CompositorOGL::EndFrame` → `CopyToTarget` = the measured `glReadPixels`). All
of that is upstream code; what it does to our 39 patches (`docs/ENGINE-SOURCE.md`):

| Patch | Under 2 + 4 | Why |
|---|---|---|
| 0003 gfx init and OMTC | half retired | the `JIHAD_DISABLE_OMTC` override in `gfxPlatform::UsesOffMainThreadCompositing` is dead, 4 is OMTC on; the `gfxPlatformHeadless` selection and `gfxPlatform::GetPlatform()` in `XRE_InitEmbedding2` stay |
| 0005 PuppetWidget offscreen | paint half retired | `GetLayerManager`'s BasicLayerManager branch and `jihad_offscreen_render_region` (`RenderDocument` into the buffer) give way to the ClientLayerManager and `ForceComposeToTarget` into the same buffer; create/resize/release, the dirty flag and rect, and the popup composite stay (popups are their own display roots on either path) |
| 0006 headless toolkit | changes | `gfxPlatformHeadless` stays but must accept accelerated layers: `AccelerateLayersByDefault()` is false without `MOZ_GL_PROVIDER`, so either `--with-gl-provider=EGL` at configure or `layers.acceleration.force-enabled`; its `GfxInfo` answers `FEATURE_STATUS_UNKNOWN` and blocks nothing |
| 0020 composite without ImageBridge | no-op | every hunk sits behind `!ImageBridgeChild::GetSingleton()`; with OMTC the bridge exists and upstream's TextureClient path carries the plugin's bitmap to the compositor |
| 0021 windowless invalidate offscreen | changes | its widget invalidate makes the daemon re-render the document; an ImageLayer update now reaches the compositor through the ImageBridge, so the invalidate means "a composite is due", not a repaint |
| 0022 RecvShow copy + frameID | stays | the copy guards the child's double buffer and the frameID is what the ImageContainer coalesces on, both still true through the bridge |
| 0035 BasicPlanarYCbCrImage::CopyData | no-op | `BasicImages` belong to the BasicLayerManager; the compositor takes PlanarYCbCr frames over the ImageBridge and converts them in its YCbCr shader |
| 0024 plugin input, widget part | stays | the dirty rect feeds the readback rectangle (the Flash column's 2–5 ms reads) |
| 0026, 0028, 0029, 0031 frame pacing and pull | stay | Flash's frames are the child's; the daemon's pull drives a composite instead of a paint |
| 0027 CPU boost | stays | the clock lever |
| the other 28 | untouched | toolchain, embedding, branding, NPAPI host/input/loader/logging/semaphore, NSS, audio, video controls, add-ons: nothing to do with how pixels reach the card |

Daemon side: `JIHAD_DISABLE_OMTC=1` leaves the three `packaging/event.d` launchers. Option 2 retires
nothing: WebGL needs the EGL provider (present) and the feature not blocked (`webgl.force-enabled`
until `GfxInfo` reports the Adreno).

### T-502, the spike: what the device measured (2026-10-17)

The OpenGL compositor runs in the headless daemon on the Adreno 220 through the vendor EGL (an EGL
pbuffer context, `CompositorOGL`, backend 2), composites a four-layer page and animates a CSS
transform with the main thread painting 0.1–2.8 ms a frame. Five rounds and a readback bench
(`docs/rounds/t502-2026-10-17/`); the frame on the device against the model:

| Stage | Model | Device (Enyo, 768×942 frame) |
|---|---|---|
| Composite four layers | 12.9 ms | 1–5 ms |
| Read the frame back (`glReadPixels`) | 12.3 ms | 22–60 ms inside the daemon (clock-bound: 12–21 ms from the pbuffer and 12 ms from a renderbuffer FBO in the bench at a high clock); 370–640 ms from a texture-backed FBO, which the stock offscreen screen buffer is |
| RGBA→BGRA (the driver refuses BGRA reads) | — | 7–16 ms NEON, 12–35 plain C |
| The snapshot road (shmem, sync IPC, a cairo draw) | — | 12–18 ms, all removable |
| Paint an animation frame on the main thread | 0 | 0.1–2.8 ms |
| New content: CPU raster + upload | 53 + 10.3 ms | 60–64 + 180–240 ms (the upload takes the tiled `glTexSubImage2D` path the bench measured at 160 ms per 3 MB) |
| Frame as the daemon sees it | 42 ms, 24 fps | 54–96 ms; the pipeline 7–9 fps on the daemon's 100 ms idle tick |
| **Round 6, the same day:** compose + read on the compositor thread straight into the shared buffer, a renderbuffer FBO as the window, pull mode (the compositor counts requests, the daemon pulls), the clock held | 42 ms, 24 fps | compose 1.4–4.6 ms, read 7–17 ms, compose+read 8–21 ms from the main thread; **the pipeline 17–24 fps** on the animation page (the daemon's own copy, scale and publish are the rest of the 41–62 ms tick) |

Verdict (after round 6): **the 24 fps row holds on the device** — 17–24 fps measured on the
four-layer animation page once the frame is read on the compositor thread into the shared buffer
from a renderbuffer FBO with the clock held; compose + read is 8–21 ms and the daemon's own copy,
scale and publish make up the rest of the frame. The content upload must become a whole-texture `glTexImage2D`
(10 ms per 3 MB) or the new-content row falls from 9 fps to 4. `EGL_KHR_lock_surface` is a copy,
not a mapping (26 ms), and `GL_EXT_read_format_bgra` is absent, so the swizzle stays (NEON, ~3 ms
at a high clock). The compositor also composes on its own vsync beside the daemon's pull (wasted
work to remove), and the daemon's tick must follow the compositor's "composite due" instead of
the widget's dirty flag.

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
