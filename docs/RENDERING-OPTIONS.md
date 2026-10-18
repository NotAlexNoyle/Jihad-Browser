# Rendering options for Jihad — what the TouchPad measured (2026-10-17)

The report the owner reads is the Claude doc "Jihad rendering options: speed comparison"; this page
keeps the measurements in the repository with the tools that took them. Every number is from the
TouchPad (webOS CE 3.1.0 — the bench since the CE loop of 2026-10-01, 3.0.5 before — APQ8060,
Adreno 220).

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

### T-502, the spike: what the device measured (2026-10-17, 86 rounds)

The OpenGL compositor runs in the headless daemon on the Adreno 220 (an EGL pbuffer context,
`CompositorOGL`, patch 0040 behind `JIHAD_GPU_COMPOSITOR`), draws into a renderbuffer window
target, and the daemon reads the frame back into the adapter's shared buffer. **The first 30
rounds measured black or wrong frames** (an un-set render bound, a window target deleted per
frame, a y-flip from the intermediate-surface path, a view the widget's size instead of the
frame's); every number below is from a frame checked by eye (`docs/rounds/t502-2026-10-17/`,
the `spike-run*-seq.jpg` contact sheets and the card screenshots).

| Page (`render/goanna/test/`) | Compositor path | Software path (today) | Chart row |
|---|---|---|---|
| `jihad-anim-css.html`: four layers, a CSS transform animation, a fixed header, a half-transparent overlay | **21–27 fps** (runs 59–86) | 23–30 fps (the damage-box partial paints) | 24 fps |
| `jihad-scroll-js.html`: 300 paragraphs scrolled 6 px every 16 ms by script | **12–17 fps** | 5–9.5 fps | 24 fps |
| `jihad-colors.html`: four blocks, static | a frame per change | a frame per change | — |

Where a frame's time goes on the device (768×942, the clock floor held, `io_fraction` 100):

| Stage | Measured | Note |
|---|---|---|
| Paint the layers on the main thread | 0.1–3 ms (up to 100 when content changes) | `presShell->Paint(PAINT_LAYERS)` in `JihadComposeStart` |
| Compose, CPU side | 3–6 ms | the compositor thread issuing the draws |
| Compose, GPU side | 18–32 ms for four full-screen layers | the bench: one opaque layer 12 ms, four 32; a scissor to 160 rows saves nothing (29 ms), a 160-row viewport is worse (38): the GPU is memory-bound at ~500 MB/s and resolves the whole target |
| Read the frame back | 20–45 ms whole, 4–15 ms for 160 rows once the GPU is done | `glReadPixels` from the renderbuffer; RGBA only, BGRA reads refused |
| RGBA→BGRA and the row flip | 5–7 ms whole, ~1 ms for 160 rows | NEON, in place, on the compositor thread |
| A content upload | 8 ms for 768×942 BGRA with the GPU idle; **18–78 ms whatever the size while GPU work is pending** | any `glTexImage2D` waits for the queued composite; a fresh name waits too |
| The daemon's publish | 0–2 ms | the painted message, the slot bookkeeping |
| The adapter / LunaSysMgr | ~33 ms cadence, a 2.9 MB card upload a frame | one buffer returned per painted message; three slots |

What is in the engine and the daemon now (patches 0040 and 0041, `render/goanna`):

- The frame in two halves: the daemon starts a job (`jihad_offscreen_compose_start`: the layer
  paint on the main thread, the compose and the read on the compositor thread), polls it on its
  8 ms tick, publishes when it is in, and starts the next on the publish tick when the compositor
  asked again meanwhile (the request used to be consumed by the publish, which cost a frame).
  Up to two jobs in flight into any slot the adapter does not hold.
- Band reads: the compositor reports the rows its layer-tree diff changed; the daemon keeps,
  per shared-buffer slot, the rows that changed since that slot was read and asks for only
  those (the animation page reads 160 rows a frame); a slot composed for other geometry is read
  whole. The composite itself is scissored to the rows read (kept: harmless, no GPU saving).
- The window's flipped view kept on the renderbuffer target (the intermediate-surface draw
  cancels a flip that must exist) and the rows reversed in the readback; render bounds of the
  frame's size; the compositor's request counter as the daemon's "frame due" signal.
- Uploads as BGRA on EGL (`GetPreferredARGB32Format`, GLES-legal types in the upload helper and
  `BasicTextureImage::Resize`): four times faster than RGBA when the GPU is idle.
- Tiled content layers by default (`layers.enable-tiles` in the engine's all.js; the bundle's
  goanna.js is read after the once-only gfx prefs are snapshotted).
- The docshell active from creation and inactive while the card is frozen (it was active already
  through the content activation on touch; kept for the frozen card).
- Opt-in, measured as no gain: a read thread with a shared context, EGL fences and two window
  targets (`JIHAD_GPU_READ_THREAD=1`: equal on the animation page, a third slower on the scroll
  page: every GL call on the compositor context waits behind the read anyway); a GPU swizzle pass
  (`JIHAD_GPU_PASS=1`); fresh texture names per upload (`JIHAD_GPU_FRESH_TEX=1`); the round-5
  snapshot road (`JIHAD_GPU_SNAPSHOT=1`).

Verdict: **the animation row holds** (21–27 fps against the chart's 24, the hardware floor of a
four-layer composite plus a readback on this GPU), **the scroll row does not yet** (12–17). The
scroll page repaints and re-uploads content on every step because the daemon scrolls the
engine's main thread per step; the displayport only makes the paint bigger, not rarer. The
remedy is the one option 4 implies: apply the scroll offset in the compositor while the scroll
stays inside the painted displayport (`AsyncCompositionManager`'s async transform for the root
scroll layer, with `AlignFixedAndStickyLayers` keeping the header still), and move the engine's
scroll only when it leaves it; a scroll frame is then a compose and a 942-row read (~45 ms, 22
fps) with no upload, and a band read where the compositor can tell what moved. After it: a
fourth shared buffer (the two-deep queue mostly finds the slot after the published one still at
the adapter), a damage rectangle in the frame header so the adapter invalidates and LunaSysMgr
uploads only the band, then option 2 (WebGL through EGL). The diagnostics in the engine
(`[jihad-gpu]` lines, the first N of each) are still in and cheap; prune them with the fold.

**2026-10-18, the compositor-side scroll (patch 0042, rounds 90-98) — the corrected account:**
the daemon sends the card's scroll to the compositor (`jihad_offscreen_set_scroll`), which
applies the difference to the engine's own scroll as an async transform on the layer carrying
the root content metrics, with `AlignFixedAndStickyLayers` keeping fixed layers still (the APZ
path without APZ); the engine is scrolled only when the card's scroll nears the edge of the
painted displayport (every ~840 CSS px at the fit zoom). The daemon keeps each slot's frame,
shifts its rows by the card's scroll since (on the compositor thread, overlapping the
composite), and asks only for the rows that entered and the rows fixed layers cover; a
scroll-only frame (no composite request since the last one: no transaction, no animation
step) composes those bands into a 256-row renderbuffer target and reads it back once.

Rounds 90-93 reported 17-25 fps pans "with the content right" from screenshots. That was wrong:
a scripted pan moves only the daemon's idea of the scroll (the adapter's view stays at 0), so a
screenshot shows one frame at `renderedY=0`, and any single frame looks right. Frame dumps
mid-pan (`JIHAD_GPU_DUMP_EVERY`) and the daemon's verify mode (`JIHAD_GPU_VERIFY=1`: the same
frame composed whole at the job's scroll, row runs that differ logged) showed the content
jumping only at the engine's anchor moves, with the entering band repeated between them. Three
causes, found in that order:

1. `SetDisplayPortMarginsForElement` had failed on every call since the first round (`rv
   0x80004005`, logged and never read): no displayport, nothing painted past the viewport.
   Replaced by `jihad_layout_set_displayport` (nsLayoutUtils.cpp, from the docshell: base rect
   plus margins on the root scroll frame's content), retried by the daemon until the document
   takes it.
2. Without APZ, `nsDisplayList::PaintRoot` attaches root scroll metadata only when
   `AsyncPanZoomEnabled`, so the compositor had no layer to move (the six early hits were
   about:blank, which has no root scroll frame). A root scroll frame with a displayport now
   counts like APZ there (patch 0042).
3. `ContainerLayer::DefaultComputeEffectiveTransforms` gives a container an intermediate
   surface when its transform "has a non-integer translation" (true for any scale) and a child
   has a clip: under the world zoom the root rendered its children through a surface the size
   of its visible region, one viewport — an extra full-frame pass every composite, nothing of
   the displayport beyond the viewport (white below), and fixed-layer compensation in the
   surface's space (the header off the frame). On the compositor's tree an axis-aligned
   scale+translate now clips by scissor instead (`JIHAD_GPU_SURFACES=1` restores the surface).

Measured after 1 and 2, before 3: 13-18 fps with the content moving and the header missing,
composes of 17-29 ms (the surface pass). The numbers after 3 are in `docs/PICKUP.md`.

**Rounds 99-112 (2026-10-18): the driver's surface rule, then 25-31 fps.** The white rows below
frame row 484 in one composite of five were the Adreno 220 binning an FBO render by the current
EGL surface's size (1400-row pbuffer, 942-row frame: the bottom 458 GL rows never rasterized;
proved with a magenta clear, raw GL state checked, a second read, one target vs two, a
frame-sized FBO). Fix: the context's pbuffer at the frame's size. Band frames on their own
768x256 pbuffer and FBO (switching into it costs 11-13 ms, so it stays current), the row shift
behind the GPU, a done-pipe that wakes the daemon's loop, the displayport only from a page's first
pan, the compositor-side scroll engaged by card pans only and settled after them. The pan:
25-31 fps (frame gaps mostly 24-31 ms), the animation page 21-25, frames verified by dumps and by
eye. Diagnostics that found it (env, off by default): `JIHAD_GPU_CLEAR_MAGENTA`,
`JIHAD_GPU_DRAWSTATS`, `JIHAD_GPU_WHITE`, `JIHAD_GPU_READ_VERIFY`, `JIHAD_GPU_DUMP_TARGET`.

**Rounds 113-117:** each frame built from the newest finished slot (one whole read after an anchor
move repairs the pipeline); the diff treats the root-content layer's scroll as a shift (a page's
own scroll reads the entering band) and is scaled to frame pixels (a latent wrong-rows bug on
fit-zoomed animated pages); the two-deep queue made correct and kept for still pages. The pan
27-32 fps, the animation page 23.5-25, the self-scrolling page 11-14.

**Rounds 118-119:** the diff first, without a render: a frame whose tree changed is composed on the
band surface when its rows fit (the animation page 26-31 fps, the self-scrolling page 13.5-17.4);
the displayport also from a page's own first scroll.

Traps, for whoever continues: a GPU wait left unbounded wedged the driver for every process
(the system UI crash-looped on `QFatal: EGL error`; only a reboot recovered — the fence wait is
bounded to 1 s now); a screenshot at a fixed delay shows the load phase (the counters on the test
pages read 1–2 a second there while timers fire 26–60 times a second later: wait for content
frames before shooting); a launch into an open card relays the URL into it (close the cards
first); the device's `/tmp` is 40 MB (the frame dumps fill it).

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
