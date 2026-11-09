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

## 4. The baseline (2026-11-09; cavekit-lowpower-uxp.md R1, T-555)

`tools/device/lowpower-bench.sh`, five runs on the Enyo variant's shipped build (the GPU path with the express lane,
the viewport pref as shipped; the engine of the day carries the scaler and the helper-thread switches, both off), the
daemon started by hand with the inject channel and the timeline (`docs/rounds/lowpower-2026-11-09/`: the rows in
`lowpower-bench.tsv`, a daemon log per measurement). Medians, [min..max] over the five runs. Every lever of R2-R5 is
measured against these rows by the same script (`LEVER=<name> EXTRA_ENV="<words>"`).

| measurement | median | spread | what it is |
|---|---|---|---|
| wiki load_ms | 13 973 | 13 540..15 102 | desktop Wikipedia (HP_TouchPad): the first content frame to "load done" |
| wiki main calls / ≥ 100 ms / max / sum | 81 / 24 / 3 834 / 21 681 ms | 72..88 / 22..26 / 3 786..3 910 / 21 094..22 715 | the main thread's calls of 15 ms or more over the load and its first 10 s |
| wiki rest calls / sum / max (30 s at rest) | 374 / **24 613 ms** / 1 469 | 363..385 / 24 237..25 228 / 1 166..1 796 | the main thread while the page sits still: **82 % of the 30 s busy** |
| wiki RSS / PSS after the load | 264 904 / 257 714 kB | 257 696..273 712 / 250 143..266 100 | the daemon's `/proc/<pid>/smaps` |
| wiki RSS / PSS 30 s later | 264 460 / 257 270 kB | 247 656..271 272 / 240 516..264 092 | — |
| wiki scripted pan | 12.3 fps (124 frames / 10.08 s); the paint pipeline 11.0 fps | 11.2..14.2; 9.3..14.5 | inject "pan 0 3000 10000", both lanes' published frames |
| jsbench loop 50M / objects 2M / strings 200k / regex 100k / dom 20k | 586 / 1 308 / 1 002 / 152 / 6 086 ms | 530..1 036 / 1 006..1 482 / 940..1 112 / 132..172 / 5 786..6 240 | the page's own timings (its title) |
| long page scripted pan | 33.2 fps; the pipeline 32.5 | 32.6..34.3; 31.3..34.5 | `jihad-long-static.html` |
| daemon start to engine up, cold / warm | 4 207 / 2 632 ms | 3 619..6 534 / 2 513..2 793 | "daemon main" to "engine up" on the timeline; cold = the page cache dropped first |
| the card's connection to its first content frame | 6 843 / 5 723 ms (cold / warm) | 6 783..7 049 / 5 380..5 851 | "card connected" to "first painted frame", the colours page |

**What the baseline says before any lever:** (1) desktop Wikipedia **at rest** keeps the engine's main thread busy
82 % of the time (a pump of 58-95 ms in every 100 ms tick, `tick period … pump avg=77 paint avg=0`): not paints — the
page's own script and the engine's periodic work (the refresh driver at 30 Hz, timers, style flushes). That is the
biggest number on this table and no research row names it: T-604. (2) The first content frame of a trivial page comes
5.4-7.0 s after the card's connection — the page's own creation inside the daemon, a startup lever beside R5's
omni.ja (the daemon's own start is 2.5-2.8 s warm, 3.6-6.5 s cold). (3) jsbench's loop timing swings 530-1 036 ms
run to run (the ondemand governor's ramp): every CPU lever is read on the median of five, never one run.

## 5. The levers, measured (2026-11-09 →; R2-R5)

Each lever alone, by `lowpower-bench.sh LEVER=<name> EXTRA_ENV="<words>"`, five runs, read against §4's medians; kept
only where it gains without a loss elsewhere beyond the spread. A run a USB drop interrupted (run 5 of the first
lever) comes out NA and is left out. **The hour matters:** the first two levers, measured in the hour after the
baseline, both read Wikipedia's load 2.5 s slower whatever their words (the page or the network of the hour), so
from the second lever on every lever runs interleaved with a control of its own hour (`LEVER_B=baseline`), and a
lever is read against that control, the baseline table being the reference for the build as shipped.

| lever (row) | words | result | decision |
|---|---|---|---|
| JS helper threads for two cores (row 1, T-556) | `JIHAD_JS_HELPER_THREADS=2 JIHAD_JS_ION_THREADS=1 JIHAD_JS_PARSE_THREADS=1` | Wikipedia's load 16 547 ms [13 878..17 834] against 13 973 [13 540..15 102]; the main thread's sum over the load 24 131 against 21 681; at rest the same (24 599 against 24 613); jsbench within the spread (loop 578 / objects 1 418 / strings 988 / regex 140 / dom 5 866 against 586 / 1 308 / 1 002 / 152 / 6 086) | **dropped**: the capped pool serialises Ion compiles and off-thread parses behind each other and the load pays 2.5 s; upstream's cores + 4 stays. The helpers' nice value (`JIHAD_JS_HELPER_NICE`) is a separate lever, below |
| the helpers' nice value (row 1, T-556) | `JIHAD_JS_HELPER_NICE=5` | load 16 924 [14 222..17 458], the main thread's sum 24 167, jsbench loop 542 / objects 1 548 / dom 6 234 — the same hour's numbers as the capped pool's, within its spread | **inconclusive** without a control of the hour (above); re-run interleaved with `LEVER_B=baseline`, and the capped pool with it |
| `image/jxl` out of the Accept headers (row 7, T-558) | `tools/device/imgdecode-round.sh`: the same 1600x1200 photograph-like picture (`make-image-assets.sh`, quality 85) fetched and decoded eight times a format on the device, `img.decode()` timed by the page (`jihad-imgdecode.html`) | JPEG XL 204 ms a picture (114 KB), JPEG 226 ms (186 KB), WebP 304 ms (98 KB), medians of eight; every one decoded | **kept**: the premise was wrong on this device — libjxl decodes faster than both the WebP and the JPEG path here, at a smaller file than JPEG; the fetch is in the figure (one Wi-Fi hop) and does not reverse it |

## Sources

UXP: https://repo.palemoon.org/MoonchildProductions/UXP (issues 3030, 3085, 3092, 3172, 3179); Pale Moon prefs
`palemoon/app/profile/palemoon.js:618`; Basilisk `basilisk/app/profile/basilisk.js:457-460, 627`; Fennec 52
`mobile/android/app/mobile.js:84, 397-405, 583` (hg.mozilla.org FIREFOX_52_0_RELEASE); https://github.com/wicknix/SeaLion,
/brass-monkey, /AF-OSX-PPC, /SpiderWeb; https://github.com/rmottola/Arctic-Fox; https://rtfreesoft.blogspot.com/ and
https://github.com/roytam1/UXP (commits 04e063dde, 454565b2a, b69cdf1cd, c7116a213, b44216a88);
https://git.hyperbola.info:50100/software/icedove-uxp.git/; https://repo.hyperbola.info:50000/other/iceape-uxp/;
https://github.com/Ascrod/ambassador; https://github.com/ActiveState/OpenKomodoIDE;
https://repo.palemoon.org/athenian200/epyrus; https://repo.palemoon.org/Basilisk-Dev/Basilisk.
