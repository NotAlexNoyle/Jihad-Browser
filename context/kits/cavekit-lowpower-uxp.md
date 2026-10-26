---
created: "2026-10-26"
last_edited: "2026-10-26"
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
- [ ] A device benchmark script runs, on one variant, with sensors as the bench rules say: desktop Wikipedia
  (HP_TouchPad) load to idle (time, main-thread pump lengths), `jihad-jsbench.html`, a long article's resident memory
  after load and 30 s later (`/proc/<pid>/smaps` RSS and PSS of the daemon), daemon start to first frame (cold and
  warm), and the scripted pan's frame rate on Wikipedia and the long page
- [ ] It prints one row per run and repeats each measurement at least five times, reporting median and spread
- [ ] The baseline of the shipped build is recorded in `docs/research/lowpower-uxp.md` before any lever is tried
**Dependencies:** none

### R2: The CPU levers measured
**Description:** The levers that trade CPU (research rows 1, 2, 12).
**Acceptance Criteria:**
- [ ] JS helper threads capped for two cores (Ion and parse threads, and a lower priority for helpers) tried as an
  engine patch: the Wikipedia pumps and jsbench against the baseline; kept only if both improve or neither regresses
  beyond the spread
- [ ] -O2 (whole tree, or js/src + layout + gfx) tried against -Os: script and paint times, libxul size, start time,
  resident memory; the choice recorded with the numbers
- [ ] PGO is either tried (with the on-device profiling run's procedure written down) or declined by the owner
**Dependencies:** R1, cavekit-engine-update.md R1

### R3: The memory levers measured
**Description:** The levers that trade resident memory (research rows 3, 4).
**Acceptance Criteria:**
- [ ] The allocator's arena count and dirty-page cap set for the daemon (environment, no rebuild) and a purge after a
  load goes idle: resident memory after Wikipedia and a long article, and the script and pan numbers, against the
  baseline
- [ ] Each memory pref of research row 4 tried alone (predictor off, empty GC chunks, image cache size, surface-cache
  expiry): its memory and time effect recorded; the kept ones in the device prefs with a comment naming the
  measurement
- [ ] Three daemons running at once (one per variant) with the kept levers stay inside the memory budget of
  cavekit-device-build.md R5
**Dependencies:** R1

### R4: The page-content levers, owner's choices
**Description:** Levers that change what a page does (research rows 5-8).
**Acceptance Criteria:**
- [ ] Reduced motion (`prefers-reduced-motion`) is put to the owner as a choice (default on, a setting, or off) with
  its measured effect on two animated sites
- [ ] Lazy image loading is put to the owner (carry upstream's reverted implementation with a scroll-driven check, or
  not) with its measured effect on a long article (requests, resident memory)
- [ ] `image/jxl` dropped from the Accept headers, or kept with the reason
- [ ] The VP9/WebM decoder A/B (ffvpx vs libvpx, MSE WebM) measured at 360p and 480p (dropped frames), and the faster
  path chosen without losing FLAC/MP3 playback
**Dependencies:** R1, cavekit-gre-widgets.md R1

### R5: The build and startup levers measured
**Description:** Research rows 9-11.
**Acceptance Criteria:**
- [ ] The GRE packed into `omni.ja` tried: start to first frame, cold and warm; kept if faster
- [ ] The content backend A/B (Skia vs the current) measured for paint time on Wikipedia and judged by eye for text
- [ ] Removing accessibility and the devtools server tried only if no shipped feature or supported add-on needs them
  (checked against cavekit-addons-extensions.md), with libxul size and start time
**Dependencies:** R1

### R6: Every result recorded, nothing kept unmeasured
**Acceptance Criteria:**
- [ ] `docs/research/lowpower-uxp.md` has, per lever, the numbers and the decision (kept / dropped / owner's call)
- [ ] Every kept lever is in the build or the device prefs with a comment pointing at its measurement; every engine
  change is a patch in the queue (cavekit-engine-update.md R1)
- [ ] Stock webOS 3.0.5 is not on the bench: any lever whose effect may differ there is named for the owner
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
- 2026-10-26: created from the owner's request and the day's research (`docs/research/lowpower-uxp.md`).
