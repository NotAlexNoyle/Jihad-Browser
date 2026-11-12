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

**Correction (2026-11-09, 22:30): the PREF levers below were not measured.** `gpu-spike-stop.sh` stripped none of a
run's `PREF=` words, so after a lever's first run its pref stayed in the Enyo profile and every control after it carried
the lever; the pref then stayed for every later round. In order: the decoder A/B's libvpx side left
`media.ffvpx.enabled=false` (its "ffvpx" runs 2 and 3 were libvpx), reduced motion left `ui.prefersReducedMotion=1`,
the Skia round left `gfx.content.azure.backends=skia`, and T-604's no-script profile left `javascript.enabled=false`.
So the rows for the decoder (but its FLAC finding), reduced motion, lazy images and Skia are void, and so is the
"page of the hour" reading of the rest time: the drop from 24 s to 5 s falls exactly where reduced motion and lazy
images stuck. The env levers (the helper pool, nice, nnff, the purge, -O2 through `XULDIR`, omni.ja through `HLDIR`)
set no pref and stand, the omni.ja pair measured on the Skia backend on both sides. The stop script now strips the
run's lines and puts back what they replaced (and the start undoes a run never stopped); the profile was cleaned.
Every void row is measured again.

**Second correction (2026-11-10): the first fix raced.** The stop stripped the run's lines 2 s after SIGTERM, but an exiting daemon saves its prefs, and it saved them back: from T-604's no-script profile at 22:40 to 08:15 the Enyo profile had `javascript.enabled=false` (and Skia from 00:38), so the night's re-measurement ("chain 3": the PREF levers on Wikipedia, the decoder A/B, reduced motion, lazy images, and all of chain 2: the alternate directory, the lean start, omni.ja back to back, the pool's third round, nnff's second, the six memory prefs, the timer clamp) ran without page script and is void, but for one reading it does give: a copy of the shipped libxul in its own directory starts cold 1.7 s slower than the shipped one in place (6 249 against 4 566 ms, four runs each; script off on both sides), so every `XULDIR`/`HLDIR` cold start is read against such a copy. The stop now waits for the daemon to be gone (30 s, then SIGKILL) before it edits, says so if a stripped pref is still there, and the chain checks the profile after every lever. Valid from that night: T-605's two clean rounds (22:31-22:36, before the leak) and the clean rest profile (22:38: the main thread 80 % busy at rest, 1 % with script off).

| lever (row) | words | result | decision |
|---|---|---|---|
| JS helper threads for two cores (row 1, T-556) | `JIHAD_JS_HELPER_THREADS=2 JIHAD_JS_ION_THREADS=1 JIHAD_JS_PARSE_THREADS=1` | Wikipedia's load 16 547 ms [13 878..17 834] against 13 973 [13 540..15 102]; the main thread's sum over the load 24 131 against 21 681; at rest the same (24 599 against 24 613); jsbench within the spread (loop 578 / objects 1 418 / strings 988 / regex 140 / dom 5 866 against 586 / 1 308 / 1 002 / 152 / 6 086) | **dropped**: the capped pool serialises Ion compiles and off-thread parses behind each other and the load pays 2.5 s; upstream's cores + 4 stays. The helpers' nice value (`JIHAD_JS_HELPER_NICE`) is a separate lever, below |
| the helpers' nice value (row 1, T-556) | `JIHAD_JS_HELPER_NICE=5` | load 16 924 [14 222..17 458], the main thread's sum 24 167, jsbench loop 542 / objects 1 548 / dom 6 234 — the same hour's numbers as the capped pool's, within its spread | **inconclusive** without a control of the hour (above); re-run interleaved with `LEVER_B=baseline`, and the capped pool with it |
| `image/jxl` out of the Accept headers (row 7, T-558) | `tools/device/imgdecode-round.sh`: the same 1600x1200 photograph-like picture (`make-image-assets.sh`, quality 85) fetched and decoded eight times a format on the device, `img.decode()` timed by the page (`jihad-imgdecode.html`) | JPEG XL 204 ms a picture (114 KB), JPEG 226 ms (186 KB), WebP 304 ms (98 KB), medians of eight; every one decoded | **kept**: the premise was wrong on this device — libjxl decodes faster than both the WebP and the JPEG path here, at a smaller file than JPEG; the fetch is in the figure (one Wi-Fi hop) and does not reverse it |
| the capped pool again, interleaved (row 1, T-556; the second round) | the same words, `LEVER_B=helpers-control`, four runs each, alternating | load 15 562 [13 845..18 123] against its hour's control 17 248 [16 239..18 326]; the main thread's sum over the load 22 189 [21 414..25 678] against 24 727 [24 365..26 774]; at rest the same (24 488 / 24 491); jsbench loop 563 / objects 1 450 / strings 972 / dom 6 034 against 658 / 1 385 / 931 / 6 074; the scripted pan 10.8 fps against 12.1 (the pipeline 11.1 / 11.2); RSS +4.8 % | **reversed, not yet kept**: measured against its own hour the pool gains 10 % on the load's main thread, the opposite of the first round (whose loss was the hour's); the pan's published rate lower within the spread. A third interleaved round decides it (chain 2) |
| the helpers' nice value, interleaved (row 1, T-556) | `JIHAD_JS_HELPER_NICE=5`, `LEVER_B=nice-control` | load 16 512 against 16 432; the main thread's sum 23 990 against 24 201; jsbench 598 / 1 327 / 978 / 148 / 6 122 against 630 / 1 379 / 1 037 / 146 / 5 943; the pan 12.4 against 12.3 fps | **dropped**: no effect beyond the spread on any number |
| -O2 instead of -Os, the whole tree (row 2, T-556) | `XULDIR=<app>/hl-o2`: an -O2 libxul (the same tree and patches, `--enable-optimize=-O2`) beside the shipped one, `LEVER_B=os-control`, four runs each | jsbench loop 517 against 594 (-13 %), **strings 436 against 968 (-55 %)**, regex 132 / 144, **dom 4 921 against 6 207 (-21 %)**, objects the same; Wikipedia's longest main-thread call 3 341 against 3 992 (-16 %), paint sum -17 %, the load's sum the same (+1 %); the card's first frame -9 % cold, -10 % warm; RSS +2 %; **libxul's text 39.9 MB against 28.1 MB (+42 %)**; the daemon's cold start to engine up 6 532 against 4 091 (+60 %) | **owner's call**, with a confound named: the omni.ja lever below, whose libxul is the shipped size, pays the same +2.3 s cold from its own copied directory, so part or all of the cold penalty is the alternate directory, not the size (chain 2 runs the shipped libxul from a copied directory to settle it). A middle way untried: -O2 for js/src alone (where the gain is), -Os elsewhere |
| jemalloc: fewer arenas, a smaller dirty cap (row 3, T-557) | `MALLOC_OPTIONS=nnff` (arenas /4, dirty cap /4), `LEVER_B=malloc-control` | Wikipedia 30 s after the load **PSS 237 472 against 259 062 kB (-21 MB, -8.3 %)**, after the load -15 MB; load, the main thread's sum, the longest call the same; the scripted pan 12.2 against 12.0 fps; the pipeline 11.6 against 12.9 (spread 8..15 against 11..14) | **likely kept**: the memory gain is outside the spread and nothing else moved beyond it; chain 2 confirms with the long page and the pipeline rate, then it goes into the daemon's job environment |
| the purge 5 s after a load (row 3, T-557) | `JIHAD_PURGE_AFTER_LOAD_MS=5000`, `LEVER_B=purge-control` | PSS 30 s after the load 247 405 against 246 525 kB; paint sum +38 %; the rest within the spread | **dropped**: jemalloc's own dirty-page handling (and `memory.free_dirty_pages`) already returns what a purge would; it costs paint |
| the VP9 decoder A/B (row 8, T-558) — **VOID, its control carried the lever (see the correction)** | `tools/device/decoder-ab.sh 3`: a 30 s VP9 WebM clip at 360p and 480p, ffvpx (shipped) against `media.ffvpx.enabled=false` (libvpx), three runs | 360p dropped 23 / 15 / 0 (ffvpx) against 11 / 9 / 5 (libvpx), painted 620-674 against 653-661; **480p dropped 53 / 31 / 23 against 76 / 43 / 65**, painted 560-590 against 564-577; with ffvpx off MP3 plays, **FLAC fails (error 4)** | **ffvpx kept**: the same at 360p within the spread, fewer drops at 480p, and libvpx alone loses FLAC. **MSE WebM stays on**: YouTube, the MSE site this device meets, already plays H.264 here (formats 160/133/134, the headroom rounds of 2026-11-07) and `jihadWebCompat.js` refuses it VP8/VP9/AV1 (the h264ify technique, T-593); turning WebM off for every site has nothing left to gain |
| reduced motion (row 5, T-558) — **VOID, its control carried the lever (see the correction)** | `PREF=ui.prefersReducedMotion=1` on animate.style and apple.com, `LEVER_B=motion-control`, three runs each | animate.style: load, the main thread and the pan the same (sum 14 771 against 13 751, the pan 27.8 against 29.0 fps); apple.com: the same (19 256 / 19 221 ms, the pipeline 10.6 against 12.6 fps within the spread); memory the same | **owner's choice, measured as no gain**: neither site does less work with it on this engine; recommended as a setting (off by default) for the user who wants calmer pages |
| lazy images (row 6, T-558) — **VOID, its control carried the lever (see the correction)** | `PREF=jihad.images.lazy=true` (the carried patch, T-558's engine) on `jihad-lazy-images.html?n=60`, `LEVER_B=lazy-control`, three runs | **PSS 175 080 against 189 808 kB (-15 MB, -7.8 %)**, paint sum 125 against 216 ms (-42 %), load 4 084 against 4 522 (-10 %); the pan the same (25.6 / 25.7 fps); every picture loaded by the end of the pan both ways (60 / 60) | **owner's choice, measured as a gain**: less memory and paint with no picture missing after the pan; recommended on |
| the Skia content backend (row 9, T-559) — **VOID, its control carried the lever (see the correction)** | `PREF=gfx.content.azure.backends=skia`, `LEVER_B=cairo-control`, four runs | paint sum 2 583 against 3 414 ms (-24 %), **the pipeline 11.9 against 9.9 fps (+20 %)**, the pan 12.1 against 11.7; RSS +3 %; the load the same | **promising, not yet kept**: the text is to be judged by eye (R5) — chain 2 takes the same Wikipedia frame with each backend, and a second round confirms the rate |
| the GRE packed into omni.ja (row 10, T-559) | `HLDIR=<app>/hl-omni`: a copy of the GRE with chrome, modules, res, defaults and the stock components packed into `omni.ja` (179 files against 2 126), `LEVER_B=loose` | load 16 106 against 18 641 (-14 %, the spread overlapping: 14 600..17 859 / 14 605..18 802); the card's first frame -6 % cold, -3 % warm; **the daemon's start to engine up 6 251 against 3 922 cold (+59 %), 3 194 against 2 458 warm (+30 %)**; the pipeline 9.2 against 11.3 fps | **not yet decided**: the run is confounded twice — the copied directory (the -O2 row's +2.3 s cold) and the startup cache, whose bundle id flips between the two layouts so every omni run starts with it dropped. Chain 2 runs omni back to back after one warm-up |

### Chain 4: the void rows measured again on the clean profile (2026-11-10, 08:18-13:33)

Every lever interleaved with a control of its hour, the profile checked clean after each (`docs/rounds/lowpower-2026-11-10/`,
`lowpower-bench.tsv`; medians, the spread in brackets). Rows 1-4 and 9 run on desktop Wikipedia.

| lever | runs | result | decision |
|---|---|---|---|
| reduced motion, Wikipedia (`PREF=ui.prefersReducedMotion=1`, T-604) | 4 | **at rest 8 696 [5 235..12 384] against 24 823 ms [24 203..25 105] of 30 s (-65 %)**, paint -67 %, load -9.5 %, the pan the same | **owner's call, a strong case**: the main thread's rest work on Wikipedia is animation |
| reduced motion, apple.com / animate.style | 3 | apple.com: **paint 159 against 3 032 ms (-95 %)**, at rest -40 %, load -10 %, PSS 30 s after the load -15 %; animate.style: nothing beyond the spread (its rest is 235 ms either way) | as above |
| the page's timers clamped to 100 ms (`dom.min_timeout_value`, T-604) | 3 | at rest 24 212 against 24 439 ms (-0.9 %) | **dropped**: the rest work is not timers |
| lazy images, Wikipedia and the long page (`PREF=jihad.images.lazy=true`) | 4 / 3 | Wikipedia: nothing beyond the spread; the long page: **load +17 %, PSS +6.7 %**, paint -15 % | **not recommended**: the void night's gain was the no-script profile's; owner's call (it was put as "recommended on") |
| Skia (`PREF=gfx.content.azure.backends=skia`) | 4 | nothing beyond the spread; **the eye frames are pixel-identical to Cairo's** but for the status bar | **not measured**: a gfx pref set in prefs.js comes too late (the gfx prefs are read before the profile's); measured again through the default pref (`GPREF=`, chain 5) |
| the VP9 decoder A/B (`decoder-ab.sh 3`, T-558) | 3 | 360p: ffvpx dropped 57 / 37 / 78 of 900, libvpx 0 / 18 / 23; **480p: ffvpx dropped 883 / 891 / 855 (painted 2-27), libvpx 87 / 56 / 45 (painted 536-558)**; with ffvpx off MP3 plays, FLAC fails (error 4) | **libvpx for VP8/VP9, ffvpx for FLAC**: engine patch 0093 (`media.ffvpx.vpx.enabled`, false in Jihad's platform prefs); confirmed on the device by chain 5 |
| the alternate directory (a copy of the shipped libxul, `XULDIR`) | 4 | the engine up **8 190 against 5 411 ms cold (+51 %)**, warm the same; the card's first frame -8 % cold | every `XULDIR`/`HLDIR` cold start is read against such a copy |
| the lean engine (no accessibility, no devtools server; both from copied directories) | 4 | the engine up 7 723 against 8 400 ms cold (-8 %), warm +17 % (the spreads overlap), the card the same; 419 KB smaller stripped | **not taken**: within the run-to-run spread, for an engine without accessibility |
| omni.ja (`HLDIR`, back to back against the loose GRE in place) | 5 | the engine up 8 381 against 5 238 ms cold (+60 %: the copy's +51 % and about +6 % of its own), the card -8 % cold, warm the same | **dropped**: no start gain once the copy is counted |
| the capped helper pool, third round | 4 | jsbench loop -14 %, objects -15 %, regex -10 %, strings -4 %; load -4 %, PSS -4 %; paint +28 % [1 805..6 355 against 1 769..3 680] | **kept**: the second and third rounds both gain (load -10 % / -4 %, the main thread's sum -10 % / -5 %, loop -14 % / -14 %) and the losses of either round are within the other's spread; into the jobs' environment |
| jemalloc nnff, second round | 4 | **PSS -11.6 % after the load, -9 % 30 s after**, load -16 %; the long page's pan the same (28.6 / 28.2 fps); **Wikipedia's pipeline 9.2 against 12.7 fps** (the first round 11.6 against 12.9) | **not yet**: the pipeline lost twice; chain 5 measures ff (the dirty cap) and nn (the arenas) apart |
| the network predictor off | 3 | nothing beyond the spread | dropped |
| `gc_max_empty_chunk_count=4` | 3 | nothing beyond the spread (at rest +11 %) | dropped |
| `image.cache.size` 8 MB | 3 | PSS 30 s after the load 251 616 against 263 144 kB (-4.4 %), paint -36 % | a second round (chain 5) |
| surface cache expiry 30 s | 3 | PSS +2.4 % | dropped |
| `decode_bytes_at_a_time` 64 KB | 3 | **load 16 467 [16 109..16 586] against 17 514 [17 374..17 800] (-6 %)**, PSS 30 s after -3 % | a second round (chain 5) |
| the parser's flush timer 250 ms | 3 | **load -16.5 %**, the main thread's sum -13 %, PSS after the load -5.6 % | a second round (chain 5) |

### 2026-11-11: the research picks, Pale Moon's items, PGO and the CPU tuning (chains 7-12)

Each switch built into the engine or the daemon OFF by default, then interleaved against a control of its hour, kept
only if faster with the frames at rest matching the control by eye (the owner, 2026-11-11). The rounds are under
`docs/rounds/{switches,levers,pgo,a9,npixmap,cache-cap,gpu-bench}-2026-11-11/`; build-site carries every number.
Wikipedia's load reads ~14 s or ~17 s by the hour on both sides alike, so a 2-run gain is confirmed with 5-7 runs.

| lever (row) | runs | result | decision |
|---|---|---|---|
| **PGO, -O2** (T-556; `mozconfig.goanna-arm-pgo`, §7) | 3 | Wikipedia **13.7 s against 17.0**, the main thread 19.0 s against 24.3; jsbench **dom 4.0 s against 6.2**, strings 0.72 against 0.96, loop 0.65 against 0.79; the card's frame **5.9 s against 7.0 cold**, 4.4 against 5.7 warm; RSS at 30 s 270 MB against 284; the pan even; the daemon's cold start 5.9 s against 4.7 (38 MB of libraries against 33), paint 1.9 s against 1.5; frames at rest identical | **KEPT**: `make-device-bundle.sh` bundles the PGO build (a newer plain build is bundled instead, with a warning) |
| `-mtune=cortex-a9` (T-608) | 3 | Wikipedia 14.1 s against 17.4; jsbench even; the long page's pan 31.9 fps against 32.7; the daemon's cold start 7.3 s against 5.0 | dropped: PGO gives more on every row |
| Ion warm-up 500 (T-622) | 2 + 7 | the 2-run gain (13.9 s against 18.1) gone over 7 runs: 17.5 against 17.5; jsbench mixed | dropped; baseline warm-up 2 (pan 7.8 fps against 13.1) and Ion 2000 (neutral) dropped too |
| text-frame property flags (T-624) | 2 + 5 | the two rounds point opposite ways; jsbench within 2 % | dropped (`JIHAD_TEXT_PROPFLAGS` unset) |
| short DOM strings inline (T-625) | 2 + 5 | Wikipedia 17.1 s against 14.1, paint +0.8 s, RSS +8 MB | dropped |
| JS source compression off / no wait (T-620) | 2 | 17.0 s against 14.2; nowait 16.4 against 16.0 | dropped |
| bidi skipped for left-to-right blocks (T-621) | 2 | 16.2 s against 14.4, paint 3.9 s against 2.1 | dropped |
| TenFourFox's GC and CC timers (T-623) | 2 | the pipeline 9.8 fps against 11.8, the cold start +0.9 s | dropped |
| the CPU floor held by the tickle device (T-630) | 2 | jsbench dom 6.7 s against 6.0, the cold card 7.7 s against 7.0 | dropped; the scaling_min_freq hold stays |
| 565 express frames during drags (T-637) | 2 | the pan 9.3 fps against 13.9: the 565 band reads stall the twin FBO | dropped as built |
| NPixmap-backed tiles (T-636) | 3 | the page never finished (120 s), the pan stopped after 4 s; each pixmap 2.5-49 ms to make, none reused | dropped; a pooled design would be another row |
| initial paint delay 300 ms (T-639) | 6 | load 16.1 s against 16.5 (within the spread), paint 1.7 s against 4.9, RSS -7 MB, **the pan 12.2 fps against 13.5**, the first frame 200 ms later by design; frames at rest match | dropped: 100 ms stays |
| a deeper video queue (T-626) | 3 + 2 | 15 drops against 26 of 900 at 480p; the daemon's RSS during play up to 17 MB more | dropped: the RAM goes to pages |
| media preload prefs (T-640) | 3 | YouTube's pan 2.2 fps against 4.2, RSS +10 MB | dropped |
| Flash SetWindow dedup (T-638) | 5 | the movies 114.5 fps against 115.6, during the pan 82.6 against 80.3, the page pan 12.0 against 12.3 | dropped: even |
| the guard on the kernel's MemAvailable formula (T-634) | 3 | 50 MB under the sum at every poll: a threshold shift; the hog stopped 7-27 MB sooner, swap lower in 2 runs of 3, higher in 1 | dropped: the thresholds tuned on the sum stay |
| a 2 MB disk-cache cap (T-641) | 2 | nothing to cut: cache2 held about 1 MB either way | dropped |
| lock_surface readback (T-610) | bench | 28.4 ms a frame against 16.6 + 18.0, and ABGR | dropped |
| Mochi minified by Enyo 2's deploy (T-618) | 5 | the cold launch 3.2 s against 4.7, the card's script 2.8 s against 3.6; frames the same | **KEPT**: `build-mochi-ipk.sh` minifies by default |
| GCC 12.3 against GCC 9.4, both -O2 (T-633) | 4 | jsbench objects 1463 ms against 1209, dom 4.67 s against 4.79; long pan 33.8 against 32.6 fps; Wikipedia 13.0 s against 12.5; daemon cold start 6.8 s against 5.3; RSS +8 MB; frames identical | dropped: GCC 9.4 + PGO stays |
| GC slices of 40 ms (T-628) | 3 + 5 | jsbench strings 356 ms against 704, but Wikipedia 12.7 s against 10.1 and its pan 11.7 fps against 13.9 | dropped |
| the blocker's cache as hash tables and keyed blobs (T-646) | 2 | the first card's connect: meta.json 22 ms against 370, host sets 82 against 1 000, maps 68, install 146, the sheet 140: ~0.46 s against ~1.7 | **KEPT** (`.idx`, `.kv`/`.kidx`; old caches migrate once, 2.1 s) |

## 6. Stock webOS 3.0.5 (R6: named for the owner, not measured)

The bench is a CE 3.1.0 TouchPad; the same hardware runs stock 3.0.5 (the owner's second TouchPad is on its way). The
engine, the daemon and the allocator are the same binaries on both, so the script, JS and paint levers (the helper
pool, -O2, reduced motion, lazy images, Skia, the VP9 decoder) should read the same there. Three may not:

- **The memory levers** (`MALLOC_OPTIONS=nnff`, the six prefs of row 4): what they save matters by the headroom around
  the daemon, and stock's resident set beside it (its own LunaSysMgr and WebAppMgr, without CE's services) is not CE's —
  the same megabytes can be the difference between swapping and not on one and nothing on the other.
- **The start levers** (omni.ja, -O2's larger libxul): cold start is the flash and the page cache; stock's other
  processes take a different share of the cache, so the cold numbers may move either way.
- **The rest-time lever** (T-604): stock's own background work on the same two cores changes what the main thread at
  rest costs the rest of the system.

Each is re-read on the stock device by the same script (`lowpower-bench.sh`, interleaved) when it arrives.

## 7. PGO on a cross build (R2: the owner's "try it", 2026-11-10; "-O2 + PGO, redone", 2026-11-11)

GCC 9.4 cross, no profiling server on the host: the profile is made on the TouchPad by the engine itself. The plan and
its evidence: `docs/research/pgo-review-2026-11-11.md` (section 3).

**Why -O2:** GCC 9 uses a profile only to optimise for speed, and under -Os every function is optimised for size: -Os +
PGO changes block order and nothing else (verified with the cross compiler: a trained hot loop stays byte-identical).
Under -O2 + PGO, trained code gets -O2 with feedback and never-run code compiles -Os-like into `.text.unlikely`, so
most of plain -O2's speed comes with little of its +42 % text.

1. **One mozconfig, two phases:** `build/webos-oe/mozconfig.goanna-arm-pgo` = the shipped options, its own objdir
   (`/out/obj-jihad-goanna-arm-pgo`), `--enable-optimize=-O2`, and the phase-invariant flags
   `-fprofile-update=single -fno-profile-values -fdevirtualize-speculatively -Wno-error=coverage-mismatch`. The phase
   comes from UXP's own make-time switches, `build-goanna-arm.sh`'s 4th argument:
   `in-container.sh --arm build/webos-oe/build-goanna-arm.sh all "" mozconfig.goanna-arm-pgo gen` sets
   `MOZ_PROFILE_GENERATE=1` (config/config.mk adds `-fprofile-generate`; objects come out as `.i_o`; libraries are
   relinked), and `... build "" mozconfig.goanna-arm-pgo use` sets `MOZ_PROFILE_USE=1` (`-fprofile-use
   -fprofile-correction`, objects as `.o`). Configure sees the same flags in both phases, so nothing has to be cleared
   between them.
2. **Profile** (`tools/device/pgo-profile.sh` does all of it and starts the use build): the daemon with the `pgo-dump` inject
   command (`JihadBrowserServer.cpp`: `__gcov_flush` from the main loop); every instrumented library of the objdir's
   `dist/bin`, stripped, to `<app>/deviceroot/hl-pgo`; the daemon's environment
   `XULDIR=<app>/deviceroot/hl-pgo GCOV_PREFIX=<app>/pgo-prof GCOV_PREFIX_STRIP=0 JIHAD_TEARDOWN_SEC=300`;
   `/tmp/spike-pgo-dump` makes `gpu-spike-stop.sh` send `pgo-dump` and wait for "profile written" before it stops the
   daemon, and `gpu-spike-start.sh` launches the card once the engine is up; `lowpower-bench.sh` with `WAIT_SCALE=4`.
   Workload: the bench's pages twice (Wikipedia load, rest and pan; jsbench; the long page's pan; start cold and warm),
   then lazy images, image decode, CSS animation, transitions, a form, modern-web, apple.com, animate.style and a
   YouTube watch page once each. libgcov merges every dump into the files.
3. **Pull:** tar `<app>/pgo-prof/out` and extract it over `build/webos-oe/out-arm` with `--strip-components=1`, so
   each `.gcda` lands beside its `.i_o` object, where `-fprofile-use` looks.
4. **Use**, on the same objdir, then count in its log: "does not match its profile data", "not found in profile
   data", "profile count data file not found".
5. **Measure:** the used libxul, stripped, in its own copied directory, against a copy of the shipped one (the
   alternate-directory penalty, §5's second correction), on the bench's interleaved rows: jsbench, Wikipedia's load and
   pan, the start, libxul's size.

**The -Os try of 2026-11-10/11, and what it taught** (all fixed by the above): `-fprofile-generate=<device path>` in
CFLAGS put the profile in one flat FAT directory (2 172 files) and baked the path in. The daemon's 8 s teardown, then
the stop's SIGKILL, cut dumps (unequal `runs` across directories). The instrumented engine ran 10x slow: an
indirect-call probe through `__tls_get_addr` at every function entry, atomic 64-bit counters (`-pthread` picks
`-fprofile-update=prefer-atomic`), lost inlining. Every page row timed out, because the card launched 6 s after a start
that took 29-51 s, so the profile was start-up only (6.1 % of functions). Two traps hit the use build:
- the gen phase's objects survived it, because make does not track flags;
- `js/src/config.status` kept `-fprofile-generate`. old-configure is autoconf 2.13, which writes no `ac_cv_env_*`
  entries, so `subconfigure.py` never notices an environment-only CFLAGS change. The whole JS engine was rebuilt
  instrumented, and libxul did not link.

The first try also found that the instrumented libxul takes libgcov's profilers from the other instrumented libraries
of `dist/bin` (they export them), so all of them go to the device.

## Sources

UXP: https://repo.palemoon.org/MoonchildProductions/UXP (issues 3030, 3085, 3092, 3172, 3179); Pale Moon prefs
`palemoon/app/profile/palemoon.js:618`; Basilisk `basilisk/app/profile/basilisk.js:457-460, 627`; Fennec 52
`mobile/android/app/mobile.js:84, 397-405, 583` (hg.mozilla.org FIREFOX_52_0_RELEASE); https://github.com/wicknix/SeaLion,
/brass-monkey, /AF-OSX-PPC, /SpiderWeb; https://github.com/rmottola/Arctic-Fox; https://rtfreesoft.blogspot.com/ and
https://github.com/roytam1/UXP (commits 04e063dde, 454565b2a, b69cdf1cd, c7116a213, b44216a88);
https://git.hyperbola.info:50100/software/icedove-uxp.git/; https://repo.hyperbola.info:50000/other/iceape-uxp/;
https://github.com/Ascrod/ambassador; https://github.com/ActiveState/OpenKomodoIDE;
https://repo.palemoon.org/athenian200/epyrus; https://repo.palemoon.org/Basilisk-Dev/Basilisk.
