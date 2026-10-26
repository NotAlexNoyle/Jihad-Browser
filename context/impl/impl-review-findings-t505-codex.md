# Review findings — T-505 adversarial review by Codex (2026-10-26)

Asked by the owner after the read-only measurement ("Do an adversarial review with codex to find a way to get the fps up"). Codex CLI 0.164, gpt-5.6-sol, read-only, run directly (`codex exec`), prompt: the system, R3, the measurement (`docs/rounds/t505-2026-10-26/`), the dead ends, the constraints. Its report follows verbatim; the checks made against the source after it are at the end.

## Ranked proposals

### 1. Make frame windows work with fixed/sticky content

**Evidence:** Frame windows were designed to avoid the 3.7 MB cross-slot copy ([JihadFrameWindow.h:12](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/browserserver/Src/JihadFrameWindow.h:12)), but same-slot reuse is explicitly rejected when fixed/sticky content exists ([BrowserPageGoanna.cpp:4657](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/BrowserPageGoanna.cpp:4657), [BrowserPageGoanna.cpp:5205](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/BrowserPageGoanna.cpp:5205)). Consequently, `JihadShiftRows` still copies nearly the entire base ([CompositorOGL.cpp:2183](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/third_party/uxp/gfx/layers/opengl/CompositorOGL.cpp:2183)).

**Change:** Store fixed/sticky generation, rectangles, and keep metadata per slot. When reusing an older same slot, move its frame window and force the union of its old and current fixed/sticky rectangles into the correction reads. Permit opaque keeps only when that slot’s recorded generation matches. This is materially different from the incorrect “own-slot lane” dead end: the compositor knows exactly how old the reused base is.

**Expected gain:** Removing median copy time gives optimistic bounds:

- Wikipedia: `9.5 × 64/(64−16) = 12.7 fps`
- Long page: `16.6 × 30/(30−13) = 29.3 fps`

**Measure:** Require zero verify-mode errors on Wikipedia plus `fixed-both` and `sticky-mix`; confirm cross-slot `shift` falls near zero and compare displayed adapter sequence numbers, not merely daemon publications.

**Risk:** Moderate correctness risk around accumulated stale sticky regions; moderate implementation effort.

### 2. Send delta frames to the adapter instead of reconstructed full frames

**Evidence:** The daemon reconstructs every complete frame, then the adapter wraps and blits the complete shared-memory surface ([BrowserAdapter.cpp:1267](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/adapter/BrowserAdapter.cpp:1267), [BrowserAdapter.cpp:1325](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/adapter/BrowserAdapter.cpp:1325)). Partial invalidation is rejected whenever scroll geometry changes ([BrowserAdapter.cpp:4346](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/adapter/BrowserAdapter.cpp:4346)), and every touch-scroll calls full `invalidate()` ([BrowserAdapter.cpp:6307](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/adapter/BrowserAdapter.cpp:6307)). The documented card upload is about 2.9 MB/frame ([RENDERING-OPTIONS.md:135](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/docs/RENDERING-OPTIONS.md:135)).

**Change:** Extend the existing trailer with patch rectangles and scroll displacement. Send only newly exposed bands/sidebar corrections; have the adapter scroll its retained surface and blit patches. Preserve a full-frame fallback without changing YAP message framing.

**Expected gain:** Copy removal alone gives 12.7/29.3 fps as above. If full card uploads cause even 24 ms of Wikipedia’s current 36 ms read stall, the upper bound becomes `9.5 × 64/(64−16−24) = 25.3 fps`. Thus the defensible range is approximately **12.7–25.3 Wikipedia, 29.3 long-page**.

**Measure:** Log bytes blitted, adapter paint/present sequence IDs, and card paint duration. Run identical drags with full-frame versus delta mode and verify every presented frame.

**Risk:** High: adapter bugs can crash LunaSysMgr; PGSurface may internally re-upload more than the requested rectangle; translucent sticky patches require exact ordering.

### 3. Pipeline band readback with the next compose

**Evidence:** The active band path deliberately disables the read thread ([CompositorBridgeParent.cpp:2031](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/third_party/uxp/gfx/layers/ipc/CompositorBridgeParent.cpp:2031)) and performs shift plus `glReadPixels` serially on the compositor thread ([CompositorBridgeParent.cpp:2129](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/third_party/uxp/gfx/layers/ipc/CompositorBridgeParent.cpp:2129)). Express refuses another job while one is composed ([BrowserPageGoanna.cpp:5006](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/render/goanna/BrowserPageGoanna.cpp:5006)).

**Change:** Double-buffer the short/tall band renderbuffers. Fence band N, let the read context wait/read it, and immediately compose band N+1 into the other target. Keep three output slots initially; “no slot” is rare, so a fourth segment is not the first lever.

**Expected gain:** Ideal throughput becomes the slower of read and remaining work:

- Wikipedia: `max(36, 14+16)=36 ms` → `9.5×64/36 = 16.9 fps`
- Long: `max(11, 3+13)=16 ms` → `16.6×30/16 = 31.1 fps`

Previous asynchronous-read experiments found no gain ([RENDERING-OPTIONS.md:166](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/docs/RENDERING-OPTIONS.md:166)); therefore this deserves a small A/B prototype before architectural work.

**Measure:** Prove overlap from timestamps: compose N+1 must begin before read N completes. Compare GPU wait, job cadence, and displayed cadence.

**Risk:** Driver-wide GL serialization may make the gain zero; additional surfaces consume roughly 1–3 MB.

### 4. Cache sidebar foreground, not final sidebar pixels

**Evidence:** Wikipedia repeatedly reads extra column chunks: one sampled frame reads 68 band rows plus three column reads ([fp-wiki1-daemon-gpu.log:1238](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/docs/rounds/t505-2026-10-26/fp-wiki1-daemon-gpu.log:1238)); another has 442 column rows ([fp-wiki1-daemon-gpu.log:1571](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/docs/rounds/t505-2026-10-26/fp-wiki1-daemon-gpu.log:1571)). Keeping final translucent pixels failed because underlying content changes.

**Change:** Cache the sidebar layer subtree as premultiplied foreground plus alpha, shift/read the underlying page normally, then CPU-blend the cached foreground over it. Refresh only when the sidebar’s own layer generation changes. Restrict eligibility to axis-aligned layers without masks or filters.

**Expected gain:** Using long-page compose cost as the sidebar-free approximation: `3 compose + 16 copy + ~18 read/blend = 37 ms`; Wikipedia upper bound `9.5×64/37 = 16.4 fps`. Long page: unchanged.

**Measure:** Count cache refreshes and column bytes; compare against a diagnostic “no sidebar correction” run while verify mode checks final pixels.

**Risk:** High correctness and engineering risk; selection/highlight changes may invalidate the cache frequently.

### 5. Remove band-surface switching from the critical path

**Evidence:** Surface acquisition reaches 24 ms in [fp-wiki1-daemon-gpu.log:1223](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/docs/rounds/t505-2026-10-26/fp-wiki1-daemon-gpu.log:1223) and 22.5 ms in [fp-wiki1-daemon-gpu.log:2163](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/docs/rounds/t505-2026-10-26/fp-wiki1-daemon-gpu.log:2163). Switching calls forced `eglMakeCurrent` when the target differs ([GLContextProviderEGL.cpp:378](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/third_party/uxp/gfx/gl/GLContextProviderEGL.cpp:378)).

**Change:** Give band composition a permanently-current shared context/surface rather than alternating frame and band pbuffers.

**Expected gain:** Median gain is probably negligible, but removing the complete p90 surface cost bounds Wikipedia’s 123 ms tail near 100.5 ms and long-page 49 ms near 38.2 ms. Expect under 5% average FPS unless these stalls cluster.

**Risk:** Shared-context synchronization and Adreno driver stability.

### 6. CE-only zero-copy presentation

Option 6 removes readback, base reconstruction, and card upload together ([RENDERING-OPTIONS.md:326](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/docs/RENDERING-OPTIONS.md:326)). At the document’s 17 ms projected path, observed-rate scaling gives **35.8 Wikipedia and 29.3 long-page fps**. This is the most credible route comfortably past R3, but ranks last per effort: approximately five weeks, CE-specific, unknown buffer-sharing API, and no stock solution.

RGB565 is not acceptable under R2 because it changes pixels. PBOs are not a solution merely by name: source initialization is conditional on a pixel-buffer extension ([GLContext.cpp:896](/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser/third_party/uxp/gfx/gl/GLContext.cpp:896)), and mapping would simply move the stall unless used as a fenced ring. Probe extensions and native read format before investing.

## Claims that are wrong or unproven

- **“LunaSysMgr redraw causes the GPU wait” is not proven.** `glReadPixels` synchronizes the daemon’s own compose, surface switch, and queued uploads too. Tiny reads sometimes take tens of milliseconds, but there are no Luna paint timestamps or GPU queue counters establishing ownership. Run the same invalidation stream with card GPU painting suppressed or replaced by a CPU-only diagnostic surface.

- **“Read time” is not pure transfer time.** It includes waiting for all prior GPU work. Calling it readback bandwidth overstates what smaller reads can fix.

- **Aggregate CPU idle does not prove a GPU bottleneck.** A serialized 13–16 ms compositor-thread memcpy can coexist with 25% total idle. The failed worker-copy experiment shows that relocation did not help; it does not show that eliminating the copy cannot help.

- **Tile transactions are understated.** Wikipedia spends 1.24 seconds of compositor time processing them, about `1240/150 = 8.3 ms` per published frame. Median queue delay is small, but the 54 ms p90 means transactions plausibly explain important tails.

- **“While frames flow” FPS is not the requirement’s displayed FPS.** The parser counts daemon output over selected windows; it does not prove the card presented every sequence. Instrument `msgPainted` through adapter paint completion and report continuous time-under-finger, dropped publications, and inter-present percentiles.

- **The sampled medians are fragile.** Band phase logging samples only a subset of frames, while Wikipedia is highly variable. Report full-distribution per-frame phases or increase sampling before treating 64/30 ms as stable causal decompositions.

## Checked against the source (Claude, 2026-10-26)

- Proposal 1 confirmed: the express lane builds on the slot's own frame (frame window moved, no copy) only with no fixed or stuck layers then or in the slot's frame (`render/goanna/BrowserPageGoanna.cpp`, the `own` rule in `xStart`; the main lane's `ownWin` likewise). Both test pages have them (desktop Wikipedia's sidebars, the long page's fixed header: its band frames say "1 kept"), so every pan frame copies from the base: the 13-16 ms "shift". In the logs the own-slot lane ran 9 times on Wikipedia, 0 on the long page.
- Proposal 3 confirmed: the band pass reads on the compositor thread unless `JIHAD_GPU_READ_THREAD` is set (`CompositorBridgeParent.cpp`, the band-pass condition), so a band frame's read is serial with the next compose.
- Dispositions: the owner's to choose (asked as multiple choice).
