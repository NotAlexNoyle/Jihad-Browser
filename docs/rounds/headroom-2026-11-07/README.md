# The device kept reachable, and YouTube measured (2026-11-07, cavekit-device-headroom.md, Tier 94)

## What happened

A finger round (T-576) was playing Learn to Fly 3 (a 17 MB movie built for a much newer player) in Enyo. A tap reached
its sponsor link; the card went to learntoflymobile.com and on to `m.youtube.com/watch?v=ZgZM0ewM4lU`. The Enyo daemon
grew from 68 to 268 MB, the load average reached 15.7, 219 MB of the 512 MB swap partition was in use, and novacom
answered on and off, then not at all, until the owner replugged the cable.

Device facts read that day: novacomd runs at nice -4, the web app manager (`LunaSysMgr -s`) at 5, the browser daemons at
0; LunaSysMgr's oom_adj is -17 and the kernel already ranks a big daemon first (oom_score 5844). What cut USB was swap,
not CPU and not the OOM killer.

## The YouTube watch page, alone (Enyo, `tools/device/finger` probe, 60 s, nothing touched)

| | Jihad (Enyo, mobile identity) | Atlas (WPE 2.52) |
|---|---|---|
| site served | m.youtube.com (mobile) | www.youtube.com/watch?app=desktop |
| CPU | the daemon's main thread a full core, all minute | 1.6 cores (both) |
| memory | 88 → 245 MB | 43 → 483 MB |
| on screen at the end | the page, thumbnail and related videos | grey placeholders |

Loads it asked for in 45 s (`jihad.blocking.trace`): 59, no request loop; the ES5 bundle, the player's base.js
(~2.7 MB), fonts, thumbnails, two telemetry calls; the ad calls refused by the blocker.

Where the main thread goes (`jihad.compat.activity`, `javascript.options.mem.log`): the page's timers, frames,
listeners and promise callbacks take 0.5-3 s of each 10 s (one long promise chain 7 s once); garbage collection about
3 s in the minute. The cosmetic filter on or off: no difference. The refresh driver at 2 Hz instead of 30: no
difference. A static page: the main thread idles after its load. The stall sampler's resolved profile: below.

Playback (yt-play.sh): with `jihad.youtube.lite` on or off, YouTube's own stats report formats 160, 133 and 134 (144p,
240p, 360p H.264) with Opus audio: it already avoids VP9 here. The daemon's main thread 35-95 % of a core while
playing, total 1-1.25 cores, 200-245 MB.

## Atlas, add-ons and user scripts (R5)

Atlas has no YouTube-specific code (its scripts and its server's strings read). Its general levers beside Jihad's:

| Atlas | Jihad |
|---|---|
| `JSC_forceRAMSize` 192 MB (smaller heaps, earlier GC) | GC at a 64 MB allocation threshold, 32 MB high-water mark; the guard below |
| 16-bit tiles (`WEBKIT_EGL_PIXEL_LAYOUT=RGB565`) | the GPU path's own formats (cavekit-gpu-rendering.md) |
| `MALLOC_ARENA_MAX=2` | jemalloc in the daemon, dirty pages returned on every purge |
| H.264 in hardware through gst-omx (`OMX.qcom.video.decoder.avc`) | H.264 in software (FFmpeg 4.4.5, T-501); hardware: T-594 |
| a compiled content blocker (`blocklist.json`) | EasyList and EasyPrivacy in the engine (cavekit-content-blocking.md) |
| an autoplay switch | the engine's autoplay pref |
| a 90 s deadlock watchdog, turned off | none |

User scripts and add-ons for YouTube on old engines do one thing that applies here: offer the player only H.264 and
small frame sizes (nikisby's "YouTube h264ify" on OpenUserJS, the "enhanced-h264ify" extension; a Greasy Fork bundle,
"YT - Advanced Web Tweaks", adds more). That technique is now the browser's own (`jihad.youtube.lite`,
components/jihadWebCompat.js, our code). Third-party front-ends (Invidious, Piped) are another party's service: the
owner's choice, not taken.

## The guard (R1)

`EngineHost::CheckMemoryPressure`, every 5 s. Flush under 48 MB available (as before). Then, if this daemon and its
players hold the larger of 128 MB and 35 % of the RAM, it sheds one step every 10 s while memory stays short (under
24 MB, swapping in over 256 pages per 5 s twice running, or past half the RAM with under 96 MB left): its Flash
players first, then its most recently navigated page, replaced by `about:stopped?url=…` without a history entry and
collected (cycle collection, GC, again) by the blocker component at 2 and 6 s. oom_adj 5.

Test: `render/goanna/test/jihad-memhog.html?mb=700&rate=10` in Enyo, `tools/device/finger/headroom-test.sh`, detached:

| run | shed at | the daemon after the stop | worst load | worst app manager answer |
|---|---|---|---|---|
| 1 (swap rule 640, no collection) | 36 s | 610 MB, held until the card closed | 11.8 | 7.7 s |
| 2 (collection after the stop) | 40 s | 87 MB in 4 s | 11.2 | 22.3 s (during the flushes) |
| 3 (the half-RAM budget) | 28 s | 106 MB in 2 s | 5.7 | 26.4 s once, while the system swapped back in |

The device answered over USB through every run.

## The main thread's profile (the daemon's stall sampler, every event over 20 ms, 60 s on the watch page)

4,902 samples (about 49 s of main-thread time), resolved by `tools/host/stall-resolve.py`:

| where | share |
|---|---|
| the page's JavaScript: parsing and bytecode 9.5 %, JIT compiling 8.3 %, interpreter and runtime 10.3 %, JIT code 6.7 % | 35 % |
| garbage collection 6.5 %, cycle collection 1.2 % | 8 % |
| waiting on locks (libpthread) | 7 % |
| paint and display lists 3.1 %, style 0.8 % | 4 % |
| scattered engine code, no single function above 1.5 % | 35 % |

The optimizing JIT off (`javascript.options.ion` false), 60 s each, main-thread jiffies summed: on 5716 and 6003, off
6193. No gain; kept on.

## The stress (R1): Learn to Fly 3 in Mochi, the watch page playing in Enyo, 10 minutes

`tools/device/finger/headroom-stress.sh`, detached, with a novacom command timed every 10 s from the host. Memory held at
about 220 MB available the whole run and the guard did nothing, rightly. At about 4 minutes, near the trailer's end
where YouTube starts the next video by itself, the load went from 5 to 14.5 in half a minute (a burst of 1,862 pages
swapped in), and the owner saw the screen hang on the video; one novacom probe failed. Closing the two cards ended it.
The daemons run at nice 0 and LunaSysMgr and WebAppMgr at 5: under load the browser outranks the system's interface.

Flash sound: the owner heard YouTube from the front card and nothing from Learn to Fly 3 behind it. The player holds no
ALSA device, only sockets; pulseaudio owns the hardware stream, RUNNING with or without YouTube (cavekit-flash.md R9).
