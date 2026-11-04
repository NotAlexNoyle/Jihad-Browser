# Review findings — T-505 item 4, the band frames composed on the CPU (2026-11-04)

One adversarial review of the engine's `JihadCpuBands` (CompositorBridgeParent.cpp, TiledContentHost.h; the fork's
commit after `928649199e`) and the daemon's `JIHAD_GPU_RES_ZOOM` rule (GoannaRenderPage.cpp), by Codex
(codex-cli 0.166.0, the diffs and the engine's sources; every finding verified in the code before the fix). Ten
findings; seven fixed in the build the final rounds ran, one closed as a guard, two left open in the daemon's switch
(documented in tools/device/t502/README.md; the switch measured a loss). The round's record:
docs/rounds/finger-2026-11-04/notes.txt.

## Findings

| # | sev | where | finding | fix | status |
|---|---|---|---|---|---|
| F1 | bug | engine TextureHost.cpp MaybeUpload; TiledContentClient GetBackBuffer | tiles carry IMMEDIATE_UPLOAD: the host uploads at the transaction and ReadUnlocks, and the content side then paints the tile's next content into the same front buffer in place while the compositor thread copies it (newer or torn rows against the tree's state) | under the switch the lock stays after the upload until the tile is dropped (the content side double-buffers); the CPU path reads only tiles whose lock is held | fixed |
| F2 | bug | engine JihadCpuCollect | a container with an intermediate surface (opacity under 1 over a group, a mask) composes apart: its children's effective transforms, scissors and opacities are the surface's, and the band test used them | such a container (as one with a blend mode, a 3D context, an isolated group) sends the frame to the GPU when its own bounds meet a band, else its subtree is skipped | fixed |
| F3 | bug | engine JihadCpuDraw | the bands cleared transparent black; the window target clears opaque white (CompositorOGL BeginFrame): an entering band's unpainted rows black, a translucent tile over the clear dark | cleared white | fixed |
| F4 | bug | engine JihadComposeAndRead (the prep) | the animations sampled at Now() while the next JihadDiffNow samples at the scheduler's last compose time (older): a rewind | the scheduler notes the CPU frame as a compose (JihadNoteCompose); the prep samples at that time as ForceComposeToTarget does | fixed |
| F5 | likely | daemon gpuZoomRelease / SetGpuViewport | a pinch below 1.1 repaints at each step a frame gets out of (the hold releases on the first frame at the new zoom, not on a settled zoom) | a settle wait before the resolution follows a zoom under 1.1 | open (the switch stays off; the owner's) |
| F6 | bug | daemon zoomBackKeep / overview / zoomBackDrop gates | T-527's gates take "a resolution above 1" as the zoom in's sign: with the resolution following the zoom, a zoom back to a fit zoom under 0.5 keeps no frame and xUnderHalf waits for the repaint | compare the destination's resolution with the tree's, not with 1 | open (as F5) |
| F7 | bug | engine JihadCpuCollect | a scale within 1e-3 of 1 accepted: 1.0009 over 1024 px is a pixel off at the edge against the GPU's resampling; the tiled buffer's resolution within 1e-3 of 1 likewise | scale and shear within 2e-5 (float noise), the translation within 0.01 px of whole pixels, the resolution exactly 1 | fixed |
| F8 | likely | engine JihadCpuDraw | PaintWillResample read from the host's cached flag, which the GPU refreshes from Layer::MayResample at every composite | MayResample | fixed |
| F9 | risk | engine JihadCpuDraw | every visible tile's host dereferenced before the band clip, the collect having checked the tiles under the bands only | the region cut to the band first; a non-buffer host skipped | fixed |
| F10 | risk | engine JihadCpuCollect; the prep | image and canvas layers off the bands: a composite's traversal keeps their timed frames and notifications going, and the composite-until time renews the request | a tree with an image or canvas layer stays on the GPU; the composite-until renewal as CompositeToTarget's tail | fixed |

Not found (the review's own words): the ordinary frame-space scissor chain, the tile offsets at resolution 1, the
valid-region intersection, premultiplied over, colour layers' opacity, the alpha force; backface, empty visible
regions, opacity 0, component alpha, the low-precision buffer and fade-in in the flat-tree case; row bounds of a
tile validated and drawn; the diff baseline (taken by the prep since the first build's verify, notes.txt), the
layers' invalidations, the sticky probe.
