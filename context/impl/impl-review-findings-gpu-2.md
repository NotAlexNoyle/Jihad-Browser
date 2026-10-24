---
created: "2026-10-19"
last_edited: "2026-10-24"
---

# Review — Tier 87, the GPU path (T-513, 2026-10-19)

Build site: context/plans/build-site.md

Codex by direct invocation (Cavekit 3.1's own review prompt, `codex exec -m gpt-5.6-sol -s read-only`)
was tried first and refused both halves with its usage limit (until 17:54); per the kit (R11) and the
owner's standing rule the `ck:inspector` reviewed instead, read-only, in two halves run side by side:
the daemon and the adapter (`git diff ff7e1d20^ HEAD -- render/`, Tier 87's whole span) and the engine
(patches 0040-0057 of `build/desktop/patches`, read against the patched tree). The interim review of the
same day (`impl-review-findings-gpu.md`, seven findings) was given to both; neither repeats it, and
neither found one of its fixes wrong. Twelve findings, every one fixed.

## The engine (patches 0040-0057)

| # | Severity | Where | Finding | Disposition |
|---|---|---|---|---|
| E1 | P1 heap overflow | `CompositorOGL.cpp` `JihadBandScratch` | The scratch buffer is each compositor's, its size a function-static shared by all: with two cards, one card was handed a buffer another had outgrown and `glReadPixels` wrote past it | Fixed: the size is a member (`mJihadBandScratchSize`) |
| E2 | P1 leak | `GLContextProviderEGL.cpp` `~GLContextEGL`, `CompositorOGL::CleanupResources` | The frame-sized and band pbuffers were never destroyed, nor the window targets', band target's and swizzle pass's renderbuffers, framebuffers, texture and program; in the process-long share group (0051) that is ~13 MB of GPU memory per card closed | Fixed: the context destroys its two pbuffers; the compositor deletes its objects in `CleanupResources` |
| E3 | P2 wrong surface | `CompositorBridgeParent.cpp` band path | A band frame of 257-320 rows (over the band surface, under the band maximum) is stacked in the frame-sized target while the band pbuffer may still be current: on the Adreno 220 every target must match the current surface | Fixed: the frame's surface made current (and the targets sized) when no band pass is taken |
| E4 | P2 double delete | `TextureHostOGL.cpp` `GLTextureHost::Lock` | A WebGL canvas's texture name was deleted by the compositor's texture source and by the producer's `SharedSurface_GLTexture`; in one share group the second delete could hit a name handed out again (a tile, a WebGL texture) | Fixed: the source is made externally owned (the client's, as upstream's comment always said) |
| E5 | P2 late write | `CompositorOGL.cpp` reads, `CompositorBridgeParent.cpp` band path | `cancelled` was checked only before the shift and the read; a GPU stall shows up inside `glReadPixels`, so a frame given up past the 3 s bound could still be written into a slot the software path paints | Fixed: the reads carry the request's cancel flag (`JihadSetCancel`); after `glReadPixels` a cancelled read stops before the swizzle and before any copy into the slot; a whole read (straight into the slot) counts a late write, `jihad_offscreen_late_writes()`, and the daemon then paints every slot whole again |
| E6 | P3 stale columns | `CompositorBridgeParent.cpp` band path | The regions of one composite are capped at 12 bands included while up to 12 chunks were packed: chunks past the cap were read without being composed | Fixed: such a frame does not fit and takes the whole path |
| E7 | P3 ghost strip | `AsyncCompositionManager.cpp` sticky probe | A sticky layer counted as held only within a fixed 64 px of sticking; a fast flick's step crosses it by more, its stuck position was not read, its old pixels slid with the content | Fixed: probed as far as twice the composite's scroll step (64 px at least, 1024 at most), set only when the scroll moved |

## The daemon and the adapter

| # | Severity | Where | Finding | Disposition |
|---|---|---|---|---|
| D1 | P1 tearing on screen | `BrowserPageGoanna.cpp` `xStart`, `gpuJobStartNext` | An in-flight slot counted as free once its mark was 2 s old, and the adapter can return its own current buffer: after a still page, a zoom's express frames were composed into the buffer on screen, and every later step of the pinch too | Fixed: `mShownKey` (the last `msgPainted`'s key) is never composed into by the express lane or the main lane's second job |
| D2 | P2 write after fallback | `BrowserPageGoanna.cpp` `gpuFallBack` | The fallback forgot the main lane's jobs only; the express lane's stayed in the engine with their slot pointers (a recovering compositor writing over software frames, or into a detached segment) | Fixed: `xForget` of both express jobs first, while the GPU path is on; `ComposeForget` no longer gated on it |
| D3 | P2 wrong rows | `BrowserPageGoanna.cpp` main-lane poll | A job composed before the overscan turned on (a document's first scroll) was published at the new frame height: rows below its own from an older frame | Fixed: a job whose size differs from the tick's is given up with the jobs built on it; the next tick composes at the new size |
| D4 | P3 bounds | `render/adapter/BrowserOffscreen.cpp` `windowBytes` | The trailer's window check ran in 32-bit `size_t` and could wrap | Fixed: in 64 bits |
| D5 | P3 debug channel | `JihadBrowserServer.cpp` inject pan and zoom | Detached threads with raw pointers: a closed card's proxy address reused by a new card, `this` used after the server's destruction | Fixed: the card matched by proxy and page; the threads kept, stopped and joined in the destructor |

Found correct by the reviews: YapProxy's send pairs and locks, the lock order (frame lock, want lock,
wake lock), the reader thread's teardown, the two lanes' slot exclusions, the returned-buffer accounting,
the segment bounds of both lanes and the trailer; PuppetWidget's job list and monitors, every fixed-size
array of the band and whole paths, the row moves and reads, the swizzle, the regions (0055), the tile
cull (0056), the diff-first timing, the texture pool, the verify guard. Not reviewed deeply: the opt-in
read thread and swizzle pass.

Proposed and taken into the kit: R10, twenty card open/close cycles leave the GPU's and the daemon's
memory flat within a margin (the leak); R2, a fast flick (a shift over 256 rows a frame) and two cards
panned in turn under verify (the scratch and the surface findings).

## Codex on the fixes, T-499 and T-487 (2026-10-24)

Codex (`codex exec -m gpt-5.6-sol -s read-only`, Cavekit 3.1's review prompt) read the whole uncommitted
diff: the twelve fixes above, the verify mode's reference composed behind its job, the inject channel's
`target`, T-499's layout viewport per document and T-487's updater fixes. Nine findings, every one real,
every one fixed. (The inspector started the same review on 2026-10-19 and stopped at the weekly limit.)

| # | Severity | Where | Finding | Disposition |
|---|---|---|---|---|
| C1 | P1 | `CompositorOGL.cpp` reads | A given-up read could still reach the slot on the opt-in paths: the scratch read copied before the cancel check, the swizzle pass and the read thread had none | Fixed: the scratch read stops before its copy; the swizzle pass and the read thread check after their read and count a late write, the read thread also before it, and a cancelled read-thread frame is not good. (The default path's whole read lands in the slot by design and is counted: E5.) |
| C2 | P1 | `BrowserPageGoanna.cpp` publish | The buffer shown until a publish kept its old in-flight mark: on a still page older than the 2 s valve, the next job could take it while the adapter still blitted it | Fixed: `noteShown` restarts the previous buffer's mark when the card is told to show another |
| C3 | P2 | `BrowserPageGoanna.cpp` verify pool | The reference buffers were reused round robin with no record of a pending reference, and a resize could free one a compose still wrote into | Fixed: six buffers made once at the largest frame and never resized, each taken again only once polled or 5 s after it was given up; none free, the late reference |
| C4 | P2 | `GoannaRenderPage.cpp` `DocumentKey` | A document's address can be reused by the next | Fixed: the inner window's id (`currentInnerWindowID`) |
| C5 | P2 | `gpu-spike-start.sh` | It warned and went on when a daemon outlived the wait | Fixed: SIGKILL after 30 s, then the run does not start if one is left |
| C6 | P2 | `gpu-spike-collect-stop.sh` | The same on the way out | Fixed: as C5; the profile and the upstart job are left alone if one is left |
| C7 | P2 | `gpu-spike-collect-stop.sh` | The upstart daemon was started before the run's prefs and `goanna.js` were put back (it could save them back) | Fixed: every restore first, then the start |
| C8 | P3 | `BrowserOffscreen.cpp` `windowBytes` | 64 bits do not stop the product of three header values from wrapping | Fixed: the rows held against the room the raster has, by division |
| C9 | P3 | `JihadBrowserServer.cpp` inject threads | Finished threads were kept until shutdown | Fixed: the finished ones joined when the next starts |
