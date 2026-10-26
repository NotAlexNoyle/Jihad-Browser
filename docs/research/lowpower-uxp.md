# Low-power levers in the UXP stack, for the TouchPad

Research of 2026-10-26 (two research passes in one session; read-only; nothing measured on the device yet).
Subject: the local engine `third_party/uxp` (UXP `RB_20261006` + Jihad's 69 patches), Moonchild Productions'
projects (Pale Moon, Basilisk, Epyrus, the UXP repository's history and issues after our base), and the third-party
UXP and Gecko-legacy forks the owner listed. Compared against what Jihad already sets:
`build/webos-oe/mozconfig.goanna-arm`, the device prefs appended by `build/webos-oe/make-device-bundle.sh`,
`packaging/prefs/jihad-platform-prefs.js`, `build/desktop/patches/`.

The hardware: two Scorpion cores (ARMv7-A, NEON, softfp) up to 1.2 GHz under an ondemand governor, 1 GB RAM shared
with the OS and up to three daemons, an Adreno 220 shared with the system compositor, slow eMMC, a 40 MB `/tmp`.
Pain points: pan frame rate, heavy-page script (desktop Wikipedia's 1-3 s pumps), ~200 MB per daemon, startup.

Tags: **[read]** read in the source; **[upstream]** an upstream report or commit; **[est]** an estimate, not measured.

## 1. What upstream and the forks have

- **UXP after our base:** nothing performance-related. `release` (RC_20261026) and `master` carry security fixes,
  libjxl 0.12, SQLite 3.54.1, certdata. Nothing to cherry-pick for speed. [upstream]
- **SeaLion, BrassMonkey (wicknix):** application layers over stock UXP (`platform` is a submodule of upstream UXP);
  no AltiVec, PPC JIT or GC work. `.mozconfig`: `-O2`, `--disable-webrtc/-devtools/-safe-browsing/-necko-wifi`.
- **Arctic Fox / AF-OSX-PPC:** TenFourFox-lineage Firefox 38; its SIMD is AltiVec (`yuv_convert_ppc.cpp`,
  qcms, Blur, nsTextFragment, UTF-8). The ARM equivalents mostly exist in UXP already (`gfx/2d/BlurNEON.cpp`,
  `gfx/ycbcr/yuv_convert_arm.cpp`, libpng NEON). One pref: `image.mem.decode_bytes_at_a_time=200000` (ours 16384).
  The comment in `make-device-bundle.sh` that Arctic Fox ships `gc_incremental_slice_ms=10` does not match the tree
  read (100, "issue 253").
- **rtfreesoft / roytam1's UXP branches (and the boc-uxp MailNews / Borealis builds):** Windows-XP and SSE-less work
  (SRWLOCK, pre-NT6 ICU, AVX guards): none applies to ARM Linux. Their perf commits are already in our base except
  two below (parse threads, Xoroshiro register order).
- **IceApe/IceDove-UXP (Hyperbola), Ambassador, OpenKomodoIDE, Epyrus:** nothing for a browser engine's speed.
- **K-Meleon forum thread:** unreadable (the host served a mismatched certificate); unknown.
- **Pale Moon / Basilisk prefs:** `network.predictor.enabled=false` (both); Fennec 52's mobile prefs are the closest
  low-memory reference (`mobile.js`).

## 2. The candidates, ranked

| # | lever | where | why it should help here | gain [est] | risk | in Jihad? |
|---|---|---|---|---|---|---|
| 1 | **JS helper threads capped and niced on 2 cores**: Ion compile threads 1 (not `cpuCount+4` = 6), parse threads 1 (roytam1 `04e063dde`), the helpers at a lower priority | `js/src/vm/HelperThreads.cpp:72-81, ~919-974` [read] | six normal-priority helpers compete with the main thread and the compositor on two cores; upstream issue #3172 reports large slowdowns on dual cores [upstream] | 10-30 % off the Wikipedia pumps | slower Ion warm-up | no |
| 2 | **-O2 instead of -Os** (all, or js/src + layout + gfx), later GCC PGO by hand | `mozconfig.goanna-arm:69`; UXP's own default `old-configure.in:821-825` [read] | the interpreter, Baseline, regexp and layout favour -O2 on an in-order-ish core | 5-15 % CPU; PGO 10-20 % more | libxul +10-15 % (page cache), a full rebuild; PGO needs on-device profiling runs | no |
| 3 | **jemalloc arenas and dirty pages** (`MALLOC_OPTIONS`: fewer arenas, smaller dirty cap), and a purge a few seconds after a load goes idle | `memory/mozjemalloc/jemalloc.c:413, ~5362, 5586` [read]; purge today only on pressure or the last card's close (`EngineHost.cpp:177-182`) | 8 arenas x up to 16 MB of freed-but-resident pages each | 20-60 MB less resident per daemon after a heavy page | more madvise calls; lock contention | no |
| 4 | **prefs**: `network.predictor.enabled=false`; `javascript.options.mem.gc_max_empty_chunk_count` 30 -> 4; `image.cache.size` 25 MB -> 4-8 MB; `image.mem.surfacecache.min_expiration_ms` 180000 -> 30000; `image.mem.decode_bytes_at_a_time` 16384 -> 65536; `html5.flushtimer.*delay` 120 -> 250 ms | `all.js:1911, 4210, 4257, 4261, 4515` [read]; Pale Moon/Basilisk/Fennec 52 [upstream] | less SQLite I/O, fewer cached empty GC chunks and images, fewer decode dispatches and load reflows | a few to ~30 MB; some load time | small; each measured alone | no |
| 5 | **`ui.prefersReducedMotion=1`** as a default or a user toggle | `nsMediaFeatures.cpp:613` [read]; Jihad sets 0 | many sites drop CSS animation and parallax: less paint and compose | site-dependent, can be large | sites look plainer: the owner's call | no (0) |
| 6 | **lazy image loading** (`loading="lazy"`) carried as a patch: upstream wrote it and reverted it twice (7574161abe, 17b7e131b4, 5185da6cf0; reverts 30a1c98c9d, 5212aa7bf1), its 250 ms poll per image replaced by a scroll check | `dom/html/HTMLImageElement.cpp` [read] | long pages fetch and decode every image | large on long pages (network, decode, memory) | a reverted feature: our own maintenance | no |
| 7 | **no `image/jxl` in the Accept headers** | `goanna.js:1393, 3326` [read] | software JXL decode is expensive on Scorpion; servers fall back to WebP/JPEG/PNG (NEON paths) | only on JXL sites | none | no |
| 8 | **media**: `media.ffvpx.enabled=false` A/B (ffvpx's ARM tree has no VP8/VP9 NEON, libvpx has); MSE WebM off so sites offer H.264 | `PDMFactory.cpp:355-358` [read] | VP9 decode is CPU-bound here | dropped frames at 360p/480p | moves FLAC/MP3 paths; needs a working H.264 path | no |
| 9 | **Skia content backend** A/B | `gfx.content.azure.backends`; `gfxPlatformHeadless.cpp:97-102` [read] | Fennec 52 used Skia+NEON for content | paint time unknown | text rendering differences | no |
| 10 | **startup**: the GRE packed into `omni.ja` (2126 loose files today); elfhack-style relocation packing (361781 relative relocations) | `make-device-bundle.sh:143-147` [read] | fewer opens and stats on eMMC | start to first frame, tens to hundreds of ms | packaging change | no |
| 11 | `--disable-accessibility`, `--disable-devtools-server` | `old-configure.in:2456, 4093` [read] | a smaller libxul | page-in and startup | add-ons needing them break (cavekit-addons-extensions.md) | no |
| 12 | Xoroshiro128++ register order on 32-bit (roytam1 `454565b2a`, `b69cdf1cd`); zlib-ng (UXP #3085, open) | [upstream] | fewer spills in `Math.random`; faster inflate | tiny | — | no |

Already in Jihad (verified by the research): Thumb-2, `-Os`, `--enable-release`, gold with ICF, jemalloc in the
program, `--disable-webrtc/-gamepad/-necko-wifi/-dbus/-updater/-crashreporter/-tests/-printing`; surface cache 32 MB,
`size_factor` 8, no bfcache viewers; the JS GC set (high water mark 32, slice 5 ms, heap growth 120, allocation
threshold 64 MB tuned on jsbench); media cache 32 MB; `layout.frame_rate=30`; RAM cache 16 MB, disk cache 50 MB;
`memory.free_dirty_pages`; the parse-thread cap of 2 (upstream 0f0cbed455).

## 3. Attractive but not applicable

- `content.notify.interval`, `content.sink.*`: only `nsContentSink` reads them, not the HTML5 parser.
- `image.mem.max_decoded_image_kb`: no such pref in UXP.
- mimalloc (`--enable-mimalloc`): throughput-tuned; Jihad keeps jemalloc for its reporter and purge.
- Parallel/idle GC (#3092) and incremental CC (#3179): reverted or slower upstream.
- Fennec's `layers.low-precision-buffer`, progressive paint, `gfx.displayport.strategy`: APZ/Android only.
- `--disable-precompiled-startupcache` (SeaLion): the opposite of what startup needs.
- PPC/GCC 4.8 workarounds (`-fpermissive`, `-flax-vector-conversions`), AltiVec paths, AV1 (not built).

## Sources

UXP: https://repo.palemoon.org/MoonchildProductions/UXP (issues 3030, 3085, 3092, 3172, 3179); Pale Moon prefs
`palemoon/app/profile/palemoon.js:618`; Basilisk `basilisk/app/profile/basilisk.js:457-460, 627`; Fennec 52
`mobile/android/app/mobile.js:84, 397-405, 583` (hg.mozilla.org FIREFOX_52_0_RELEASE); https://github.com/wicknix/SeaLion,
/brass-monkey, /AF-OSX-PPC, /SpiderWeb; https://github.com/rmottola/Arctic-Fox; https://rtfreesoft.blogspot.com/ and
https://github.com/roytam1/UXP (commits 04e063dde, 454565b2a, b69cdf1cd, c7116a213, b44216a88);
https://git.hyperbola.info:50100/software/icedove-uxp.git/; https://repo.hyperbola.info:50000/other/iceape-uxp/;
https://github.com/Ascrod/ambassador; https://github.com/ActiveState/OpenKomodoIDE;
https://repo.palemoon.org/athenian200/epyrus; https://repo.palemoon.org/Basilisk-Dev/Basilisk.
