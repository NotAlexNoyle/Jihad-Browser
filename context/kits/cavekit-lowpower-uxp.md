---
created: "2026-10-26"
last_edited: "2026-11-10"
complexity: thorough
---

# Cavekit: The Engine Tuned for Low-Power Hardware

## Scope
The owner, 2026-10-26: **"scan the source code for the entire uxp stack to look for optimizations that jihad browser
could benefit from given the low powered hardware legacy webos runs on"**, starting from Pale Moon, Epyrus, Basilisk,
K-Meleon, SeaLion, BrassMonkey, MailNews, Borealis Navigator, Ambassador, Arctic Fox, IceApe, IceDove, OpenKomodoIDE
and the rtfreesoft builds.

The scan is done: `docs/research/lowpower-uxp.md` (2026-10-26) ranks twelve candidate levers with their sources. Its
finding: upstream and the forks hold little Jihad lacks (the forks are application layers, Windows-XP work or
PowerPC/AltiVec); the leverage is in what Jihad has not tuned for two cores, 1 GB and slow storage — the JS helper
threads, the optimization level, the allocator's retained memory, a handful of prefs, lazy images. None is measured.

This kit turns the candidates into measured decisions: each lever is tried on the TouchPad against the same baseline,
kept only where it gains, and recorded either way.

Reference: `docs/research/lowpower-uxp.md`; `build/webos-oe/mozconfig.goanna-arm`; `build/webos-oe/make-device-bundle.sh`
(the device prefs); `packaging/prefs/jihad-platform-prefs.js`; cavekit-engine-update.md (engine patches go through
the queue).

## Requirements

### R1: One baseline, measured the same way every time
**Description:** A lever is judged against numbers taken the same way before and after.
**Acceptance Criteria:**
- [x] A device benchmark script runs, on one variant, with sensors as the bench rules say: desktop Wikipedia
  (HP_TouchPad) load to idle (time, main-thread pump lengths), `jihad-jsbench.html`, a long article's resident memory
  after load and 30 s later (`/proc/<pid>/smaps` RSS and PSS of the daemon), daemon start to first frame (cold and
  warm), and the scripted pan's frame rate on Wikipedia and the long page
- [x] It prints one row per run and repeats each measurement at least five times, reporting median and spread
- [x] The baseline of the shipped build is recorded in `docs/research/lowpower-uxp.md` before any lever is tried
  *(2026-11-09, T-555: `tools/device/lowpower-bench.sh`, five runs, §4 of the research document; the main thread 82 %
  busy at rest on Wikipedia is the baseline's own finding, T-604)*
**Dependencies:** none

### R2: The CPU levers measured
**Description:** The levers that trade CPU (research rows 1, 2, 12).
**Acceptance Criteria:**
- [x] JS helper threads capped for two cores (Ion and parse threads, and a lower priority for helpers) tried as an
  engine patch: the Wikipedia pumps and jsbench against the baseline; kept only if both improve or neither regresses
  beyond the spread *(2026-11-10: kept — two interleaved rounds on the clean profile gain on the load, the main thread's
  sum and jsbench, nothing lost beyond the spread; `JIHAD_JS_HELPER_THREADS=2 JIHAD_JS_ION_THREADS=1
  JIHAD_JS_PARSE_THREADS=1` in the three jobs (`packaging/gen-variant-scripts.sh`); the helpers' nice value dropped,
  no effect)*
- [~] -O2 (whole tree, or js/src + layout + gfx) tried against -Os: script and paint times, libxul size, start time,
  resident memory; the choice recorded with the numbers *(2026-11-09/10: measured, §5; the owner, 2026-11-10: decided
  after PGO's numbers, -Os with PGO against -O2)*
- [~] PGO is either tried (with the on-device profiling run's procedure written down) or declined by the owner
  *(2026-11-10: the owner's "try it"; the procedure is §7 of the research document, its first profiling run found two
  traps (written there) and runs again)*
- [ ] GCC's tuning for the Scorpion cores measured: the default (Cortex-A8, as `-march=armv7-a` gives) against
  `-mtune=cortex-a9` (`mozconfig.goanna-arm-a9`), interleaved, from copied directories
**Dependencies:** R1, cavekit-engine-update.md R1

### R3: The memory levers measured
**Description:** The levers that trade resident memory (research rows 3, 4).
**Acceptance Criteria:**
- [~] The allocator's arena count and dirty-page cap set for the daemon (environment, no rebuild) and a purge after a
  load goes idle: resident memory after Wikipedia and a long article, and the script and pan numbers, against the
  baseline *(2026-11-10: nnff saves 9-12 % PSS twice but costs Wikipedia's pipeline rate twice; the arena count and
  the dirty cap are measured apart next; the purge dropped)*
- [~] Each memory pref of research row 4 tried alone (predictor off, empty GC chunks, image cache size, surface-cache
  expiry): its memory and time effect recorded; the kept ones in the device prefs with a comment naming the
  measurement *(2026-11-10: all six measured on the clean profile; predictor, GC chunks, surface expiry dropped; the
  image cache, decode chunk and parser flush timer go to a second round)*
- [ ] Three daemons running at once (one per variant) with the kept levers stay inside the memory budget of
  cavekit-device-build.md R5
**Dependencies:** R1

### R4: The page-content levers, owner's choices
**Description:** Levers that change what a page does (research rows 5-8).
**Acceptance Criteria:**
- [x] Reduced motion (`prefers-reduced-motion`) is put to the owner as a choice (default on, a setting, or off) with
  its measured effect on two animated sites *(2026-11-10: on the clean profile Wikipedia at rest -65 %, apple.com's paint
  -95 %, animate.style unchanged; the owner: **on by default, with a setting** — `ui.prefersReducedMotion=1` in the
  platform prefs, the Settings page's "Page animations" row)*
- [x] Lazy image loading is put to the owner (carry upstream's reverted implementation with a scroll-driven check, or
  not) with its measured effect on a long article (requests, resident memory) *(2026-11-10: on the clean profile the long
  page loads 17 % slower with 7 % more PSS, Wikipedia unchanged; the owner: **not carried** — `jihad.images.lazy` stays
  false)*
- [x] `image/jxl` dropped from the Accept headers, or kept with the reason *(2026-11-09, T-558: kept — on the device JPEG XL
  decodes the same 1600x1200 picture in 204 ms against WebP's 304 and JPEG's 226, `docs/research/lowpower-uxp.md` §5)*
- [~] The VP9/WebM decoder A/B (ffvpx vs libvpx, MSE WebM) measured at 360p and 480p (dropped frames), and the faster
  path chosen without losing FLAC/MP3 playback *(2026-11-10, T-558: measured clean — at 480p ffvpx drops 855-891 of 900
  frames, libvpx 45-87; at 360p 37-78 against 0-23. Chosen: libvpx for VP8/VP9 and ffvpx for FLAC, by engine patch 0093
  (`media.ffvpx.vpx.enabled`, false in the platform prefs); on the device with the next deploy, where the A/B runs again
  and FLAC and MP3 play on the shipped prefs. MSE WebM stays on: YouTube, the MSE site this device meets, already plays
  H.264 here (formats 160/133/134, `docs/rounds/headroom-2026-11-07/README.md`) and `jihadWebCompat.js` refuses it VP8/VP9/AV1)*
**Dependencies:** R1, cavekit-gre-widgets.md R1

### R5: The build and startup levers measured
**Description:** Research rows 9-11.
**Acceptance Criteria:**
- [x] The GRE packed into `omni.ja` tried: start to first frame, cold and warm; kept if faster *(2026-11-10: not faster —
  cold +60 % against the loose GRE in place, of which +51 % is any copied directory's; the card's first frame -8 % cold;
  dropped)*
- [ ] The content backend A/B (Skia vs the current) measured for paint time on Wikipedia and judged by eye for text
  *(2026-11-10: not yet measured — set as a profile pref it never took effect (the eye frames are pixel-identical); runs
  again as a default pref)*
- [x] Removing accessibility and the devtools server tried only if no shipped feature or supported add-on needs them
  (checked against cavekit-addons-extensions.md), with libxul size and start time *(2026-11-10: the add-ons kit names
  neither; the lean libxul is 419 KB smaller stripped and starts cold 8 % sooner, within the spread, warm the same: not
  taken)*
**Dependencies:** R1

### R6: Every result recorded, nothing kept unmeasured
**Acceptance Criteria:**
- [ ] `docs/research/lowpower-uxp.md` has, per lever, the numbers and the decision (kept / dropped / owner's call)
- [ ] Every kept lever is in the build or the device prefs with a comment pointing at its measurement; every engine
  change is a patch in the queue (cavekit-engine-update.md R1)
- [x] Stock webOS 3.0.5 is not on the bench: any lever whose effect may differ there is named for the owner
  *(2026-11-09: `docs/research/lowpower-uxp.md` §6 — the memory levers, the start levers, the rest-time lever)*
**Dependencies:** R2, R3, R4, R5

## Out of Scope
- The GPU compositor's own levers (cavekit-gpu-rendering.md R3, T-505): measured there
- Engine features beyond lazy images; an engine update (cavekit-engine-release-2026-10.md)

## Cross-References
- cavekit-gpu-rendering.md — the pan rate this kit's baseline also measures
- cavekit-device-build.md R5 — the memory budget
- cavekit-engine-update.md — the patch queue
- cavekit-modern-web.md — sites' behaviour under the levers

## Changelog
- 2026-11-10: the owner's calls: reduced motion on with a setting, lazy images not carried, -O2 after PGO. Chain 4 on the clean profile: the helper pool kept (R2), omni.ja and the lean engine not taken (R5), the VP9 path chosen (R4, patch 0093), the memory prefs' first round (R3); Skia's A/B found unmeasured.
- 2026-11-09 (evening): the lever chain measured every R2-R5 lever against a control of its hour (§5); the PREF levers' controls carried the lever (the spike stop stripped no pref): those rows void, re-measured; R6's stock note met.
- 2026-11-09: R1 met (T-555, the baseline script and five runs); the rest-time main thread finding added as T-604 under R2.
- 2026-10-26: created from the owner's request and the day's research (`docs/research/lowpower-uxp.md`).
