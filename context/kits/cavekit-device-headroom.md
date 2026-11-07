---
created: "2026-11-07"
last_edited: "2026-11-07"
complexity: thorough
---

# Cavekit: The Device Kept Reachable, and YouTube Made Light

## Scope
The owner, 2026-11-07, after a finger round's game ad sent a card to YouTube and the TouchPad stopped answering until
the cable was replugged: **"make the browser more efficient, especially youtube, so this doesnt happen in the
future"**, then **"see if atlas does any youtube tricks that might be applicable. you can also look for extensions,
plugins, or user scripts"**.

What happened (2026-11-07, 06:00-06:57 device time): Learn to Fly 3 (a 17 MB movie) was playing; a tap reached its
sponsor link, the card went to learntoflymobile.com and from there to `m.youtube.com/watch?v=ZgZM0ewM4lU`. The Enyo
daemon grew from 68 to 268 MB, the load average reached 15.7, swap was in use, novacom answered on and off and then
not at all; a player process from an earlier card was still alive. Measured alone afterwards, that watch page keeps the
daemon's main thread at a full core for at least 60 s and grows it from 88 to 245 MB while nothing on the screen
moves (59 loads, no request loop: the cost is script). Atlas on the same address gets the desktop site, uses 1.6
cores for 45 s, reaches 483 MB and still shows grey placeholders; it has no YouTube-specific code (its levers are
general: a JS heap budget, 16-bit tiles, two malloc arenas, hardware H.264 through gst-omx, a content blocker, an
autoplay switch). This kit covers the device's headroom against any page, and YouTube's cost in this browser.

## Requirements

### R1: The device stays reachable whatever a page does
**Description:** no page, movie or pair of them can take the system's own services down with the browser.
**Acceptance Criteria:**
- [ ] The browser's processes (each shell's daemon and every Flash player) are the kernel's first choice when memory
  runs out, before the system's own; read back from `/proc` on the device. Their CPU priority is changed only if a
  measurement shows the system's services starved of CPU by them *(2026-11-07, T-591: oom_adj 5 read back on the Enyo
  and Mochi daemons (LunaSysMgr and WebAppMgr -17), but set only at a daemon's first page: Mojo's read 0. CPU: the
  daemons run at nice 0, LunaSysMgr and WebAppMgr at 5, and in the stress below the screen froze at load 14 with memory
  to spare: the measurement that says the browser must not outrank the system's UI)*
- [ ] When available memory stays under a floor after the engine's caches are flushed, or the system swaps in
  without pause for 10 s, the browser sheds load in a fixed order (a background page's player, the foreground page's
  player, then the heaviest page, stopped with a notice that names the reason and offers a reload); each step is
  logged, and the floor and order are in the device prefs *(2026-11-07, T-591: EngineHost::CheckMemoryPressure, the
  test page render/goanna/test/jihad-memhog.html and tools/device/finger/headroom-test.sh, three runs in
  docs/rounds/headroom-2026-11-07/: the page stopped at 28 s and its 700 MB back in 2 s, the device answering
  throughout. Open: the players' step not yet exercised, and the thresholds are environment words, not prefs)*
- [ ] A Flash player with no movie left exits within the engine's unload delay (30 s) and uses no CPU meanwhile; under
  the memory floor it exits at once (on 2026-11-07 a round read the reused player of the next card as an orphan;
  game-round.py now waits 35 s before it names one)
- [ ] The stress: the YouTube watch page in one card and the heaviest Flash title playing in another for 10 minutes;
  a novacom command every 10 s answers within 3 s, a call to the application manager answers within 2 s, the USB link
  never drops, and the browser recovers or sheds as above *(2026-11-07, tools/device/finger/headroom-stress.sh: memory
  stayed at 220 MB available and the guard rightly did nothing, but 4 minutes in, at about the trailer's end, the load
  went from 5 to 14.5 and the owner saw the screen hang on the video; one novacom probe failed. Closing the cards ended
  it. Open until the next video stops starting by itself (R4) and the CPU priority is set (above))*
**Dependencies:** cavekit-flash.md R10

### R2: YouTube measured the same way every time
**Description:** one probe, so every lever is a number and not an impression.
**Acceptance Criteria:**
- [x] A probe opens an address in a shell for a set time and records, every 5 s: the daemon's main-thread and total
  CPU, its resident memory and thread count, free memory, swap and the load average; it can list every load the page
  asks for; it closes the card itself at the end *(2026-11-07, T-590: tools/device/finger/page-probe.sh; the loads by
  `jihad.blocking.trace`; where script time goes by `jihad.compat.activity`; the main thread's profile by the daemon's
  stall sampler and tools/host/stall-resolve.py)*
- [ ] Baselines recorded in Enyo for the home feed, the watch page left alone, and the watch page playing for 60 s;
  Atlas on the same addresses as reference *(2026-11-07: the watch page alone, a full core for 60 s and 88 to 245 MB;
  playing, 35-95 % of a core and 200-245 MB; Atlas on the same address 1.6 cores and 483 MB. The home feed still to
  do. docs/rounds/headroom-2026-11-07/)*
**Dependencies:** None

### R3: YouTube's pages lighter
**Description:** the page's own work cut where the browser can cut it, without losing what a person uses there.
**Acceptance Criteria:**
- [ ] Each lever is tried alone against the R2 baseline and kept only if it lowers CPU or memory beyond the spread of
  three runs: at least the web fonts, the animation library, the telemetry calls, the JS heap budget and off-thread
  script parsing; the result of each recorded
- [ ] With the kept levers the home feed, a search, the watch page, playback and the comments still work (a finger
  test each)
- [ ] The watch page left alone settles: from 30 to 60 s its main-thread CPU is under a quarter of a core, or what is
  still running is named and recorded as the site's *(2026-11-07, the watch page's profile: its own JavaScript 35 % of
  the main thread, compiling 18 % of that, collection 8 %, paint and style 4 %. The levers not yet tried)*
**Dependencies:** R2

### R4: YouTube video cheap to play
**Description:** the formats this device decodes cheapest, and nothing playing that was not asked for.
**Acceptance Criteria:**
- [x] On YouTube the player is offered H.264 only, at no more than 30 fps and 480 rows by default (the h264ify
  technique, built into the browser's site shim); a setting turns it off *(2026-11-07, T-593: `jihad.youtube.lite`,
  components/jihadWebCompat.js, the settings row under content blocking; asked from the page on the device: VP9 false,
  H.264 360p true, H.264 720p false. YouTube already chose 144p-360p H.264 here without it)*
- [ ] Previews and the next video do not play by themselves *(2026-11-07: the stress run's hang came at about the
  trailer's end, where the next video starts by itself)*
- [ ] A 60 s play at the default quality drops under 5 % of its frames, with its CPU and memory recorded
- [ ] Hardware H.264 decoding through the device's OMX decoder (what Atlas does with gst-omx) is evaluated with a
  probe that decodes a clip from the daemon's process; built, or declined by the owner with the measured cost
**Dependencies:** R2, cavekit-modern-web.md R6

### R5: What Atlas and add-ons do, read
**Description:** the owner's question answered with sources.
**Acceptance Criteria:**
- [x] Atlas's engine settings each set beside Jihad's equivalent: has it, tried (with the number), or not applicable
  *(2026-11-07, T-595: docs/rounds/headroom-2026-11-07/README.md; Atlas has no YouTube-specific code)*
- [x] YouTube add-ons and user scripts that run on UXP surveyed; the techniques adopted are written into the browser's
  own shim, and no third-party code ships without its licence *(2026-11-07, T-595: h264ify's technique, our own code)*
**Dependencies:** None

## Out of Scope
- Third-party YouTube front-ends (Invidious, Piped): another party's service, the owner's choice
- A changed identity to get past a site's bot check (the auto-mode classifier refused it 2026-11-06; the owner's)
- Sites other than YouTube, except as R1's stress

## Cross-References
- cavekit-modern-web.md R3 (the identity table that sends youtube.com to its mobile site), R6 (performance)
- cavekit-lowpower-uxp.md R2, R3 (the CPU and memory levers; R3 here uses the same measurement discipline)
- cavekit-flash.md R10 (a player's life), R6 (the round that met this)
- cavekit-content-blocking.md (the blocker whose trace R2 uses)

## Changelog
- 2026-11-07: created from the owner's two requests of the day; R1's priority criterion reworded from the measurement
