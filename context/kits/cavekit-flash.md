---
created: "2026-11-05"
last_edited: "2026-11-05"
---

# Flash, finished — the player proven on real games, apps and videos, by the finger

**Why this kit exists.** The owner, 2026-11-05: "add a kit for comprehensive testing and completion of flash support,
with autonomous finger tests, on a variety of flash games, apps, and videos". Flash runs: `cavekit-addons-extensions.md`
R7 got the device's own player drawing, sounding and taking input inside Goanna, and `cavekit-gpu-rendering.md` R7 kept
it on the GPU path. But nearly all of that evidence comes from a dozen movies this project generated. One real title has
ever been verified (Bloxorz, 2026-10-07, "running"), "Flash game feel" still waits on an eye (T-364), and the open
criteria sit scattered across four kits. This kit is the authority for finishing Flash: what "works" means on real
content, measured by a machine, in all three shells.

**What is known (measured unless it says otherwise).**
- The player is the device's stock `libflashplayer.so`, "Shockwave Flash 10.3 d185", ARM, unpatched, and not a generic
  NPAPI plugin (addons R7). By Adobe's version table that means SWF 12 at most and no Stage3D; R12 measures the edge.
- A movie's inside is opaque to the page: no DOM, no `rect`, no text a check can read, unless the movie itself reports.
- The touchscreen is Flash's mouse. A double tap on a movie latches the gesture lock, which sends the finger to the movie
  instead of to the page's pan and zoom.
- Rates so far: on the GPU path `anim` publishes 21-25 frames a second against 25-27 on the software path. The composite
  rate alone once measured 30.6-32.5 with the CPU boost, on the software path before the GPU default. Neither is a
  real game's content rate.
- Where real movies come from (research of 2026-10-07, not re-run): Armor Games embeds the real SWF, Y8 needs a changed
  browser identity, Kongregate, CrazyGames, Newgrounds and archive.org's item pages force Ruffle, and archive.org's
  direct downloads run. The five seed games' direct downloads answered 200 on 2026-11-05.
- The bench is webOS CE 3.1.0. Stock 3.0.5 cannot be tested here.

**What moves here, to be closed by this kit's evidence:** addons R7's "smooth, sustained frame rate" and "keyboard reaches
a focused plugin and does not drive the chrome" (both `[~]`); input-bridging's pinch and tap over a plugin, keys swallowed
for content, keys verified with an interactive plugin, and the keyboard raised for a plugin that wants text (all `[~]`);
offscreen-rendering's real pan and pinch over a plugin (`[ ]`); T-364's game feel; and PICKUP's open items 4 (the
plugin-container abort on every teardown) and 5 (the in-daemon plugin scan that can wedge the daemon).

**Every title ends with one of three verdicts.** *Pass*: every criterion that applies to its class holds. *Limit*: it
fails, and the same failure is reproduced in the stock browser or traced to the player itself; the reason is recorded. A
limit claimed without that evidence is a fail. *Fail*: a defect of ours, filed and fixed. Flash is finished when no title
fails in any shell.

## R1: The corpus — a declared variety, reproducible, not redistributed
**Description:** the titles are chosen for variety in how they are played and how they were made, listed in a manifest,
fetched and verified by a tool, and kept out of the repository (it is public and these files are other people's work).
**Acceptance Criteria:**
- [x] At least 24 games, at least 4 of each kind: hold or one button; point and click; drag or aim; arrow keys; other keys; heavy (physics or many sprites). The five seeds (Helicopter, Bloxorz, Motherload, QWOP, Alien Hominid) are among them, and a title counts in one kind only *(2026-11-06: 37 games, each of the six kinds 4 to 9, the five seeds among them; `tools/host/flash-corpus.json`. Whether a kind's input is as declared is the game's own script's to prove, T-576)*
- [x] At least 8 apps: a drawing tool, a calculator or converter, a chart, a music player, a form with text fields, a scrolling list or document viewer, and two more of different kinds *(10: calc, chart, clock, draw, form, instrument, music, reference, sequencer, viewer. The form is `website-viewer`, a real title whose file holds 2 input fields by the reader; a sign-up or login form was looked for on archive.org and those found hold none)*
- [x] At least 8 videos: Sorenson Spark, On2 VP6 and H.264, each at two or more sizes up to 480 rows, played by at least 2 different players; plus at least 2 authored vector cartoons *(10: Spark 160x120 and 640x480, VP6 320x176, 480x272 and an alpha 478x430, H.264 426x240 and 640x360, by Flowplayer and MediaElement; three vector cartoons)*
- [x] The spread is read from the files by the tool, not typed: SWF versions from 6 or older through 12 are all present, AVM1 and AVM2 each make up at least a third of the titles, stage widths run from 320 px or less to 800 px or more, and at least 3 titles are newer than the player (the controls of R12) *(`flash-corpus.py check`: SWF 5 to 12 each, 14, 15, 17, 28; AVM1 33, AVM2 17 of 50; widths 200 to 950; five newer than the player)*
- [x] The manifest records for each title: id, kind, source URL, SHA-256, size, SWF version, frame rate, stage size, AVM generation, the input it needs, and a provenance note *(all read by the tool, but id, kind, source, input and note; `input_fields` and `embedded_video` too)*
- [x] One command fetches what is missing into a cache outside the repository and verifies every hash; a changed hash or a dead source is reported by title, and a replacement leaves a manifest change with the reason. Rounds run from the cache over the local network, so an internet failure is never a Flash verdict *(`flash-corpus.py fetch`; tried on an empty cache, a file changed by one byte (CHANGED, by title, exit 1) and a dead source (DEAD, by title, exit 1). The cache is ~/.cache/jihad-flash, 198 MB. Serving it is T-574's)*
- [x] A check confirms that no third-party movie is tracked in the repository; a movie this project generates is parsed back by a tool before any result from it is believed (a malformed test asset faked a player defect for a whole session, 2026-08-10) *(`flash-corpus.py tracked`: none but `render/goanna/test/jihad-*`, generated and read back by `dump-swf.py`)*
**Dependencies:** None

## R2: The instruments — verdicts about a movie the page cannot see into
**Description:** a test learns what a movie did from instrumented movies that report their state to the page, from the shown frames inside the movie's rectangle, from the daemon's and the window server's logs, from the audio device's state and from the process table. A movie is addressed in its own stage coordinates, never in screen coordinates.
**Acceptance Criteria:**
- [x] Instrumented movies, one for AVM1 and one for AVM2, report to text the check library can read: each mouse event with its stage coordinates, each key code, frames elapsed, a timer, the stage size, the player's version string, and for R8 the video's time, codec and dropped frames *(2026-11-06, T-571: `render/goanna/test/make-instrument-swf.py` assembles `jihad-instr-avm1.swf` and `jihad-instr-avm2.swf`, read back by it and by `flash-corpus.py read`; `jihad-flash-instr.html` sorts their reports into elements. On Enyo: a tap at stage 100,100 reads `down 101 100` from both, the version `WEB 10,3,185,65`, the frames and timer; the video reports on AVM2 by FlashVars: VP6 `codec=4 480x272`, H.264 `codec=avc1 640x360`, Spark, each with its time, dropped frames and rate. A key reaches the report; its real codes through the keyboard are T-575's. A malformed hand-written shape froze the first AVM1 movie and looked like a player limit: the file's header says so)*
- [x] A gesture named in stage coordinates lands within 3 stage pixels of that point, by the instrumented movie's report, in each scale mode at page zooms of the fit zoom, 1 and 2, in each shell *(2026-11-06, T-572: `flash-stage-z1`, `-zfit`, `-z2`, on the three shells: showAll, noBorder, exactFit and noScale at the fit zoom, 1 and about 2, two points each, every one within 3 px by the movie's own report; `fl_flash_point`, `fl_flash_tap`, `expect flash-at`)*
- [x] The content rate (distinct frames a second inside the movie's rectangle) is reported apart from the composite rate; on a generated movie of known rate it reads within 5 % of that rate *(`fbstream -C`: flips are the composite rate, changes the content rate; `flash-rate` on the three shells reads within 5 % of what the movie counts for itself: 9.2 against 9.3, 12.8 against 12.5. A 10 fps movie runs at 9.0-9.3 and a 15 fps one at 12.5-13 by its own count: R3 says whether that is the player's)*
- [x] A "reacted" check compares the movie's rectangle after a gesture with a no-input control of the same length, and it is tested against planted cases: a frozen movie and a looping animation that ignores input must both be reported as not reacting *(`flash-control`, `expect flash-reacted`, `flash-reacted` on the three shells: frozen and a 40-frame loop say no, a movie whose sprite moves on a mouse down says yes. It compares 16 x 12 cell pictures: an exact hash moved by a shade with any tap)*
- [x] A movie counts as drawn only with 8 or more distinct colours inside its rectangle, and a planted blank, black or one-colour movie is reported as not drawn *(`expect flash-drawn`, `flash-drawn` on the three shells: blank, black and one colour no, ten coloured squares yes)*
- [~] A title's screen is recognised by a stored reference signature, and each signature is tested: the same screen matches in 3 separate runs, and another screen of that title does not. A reference is looked at by eye once when made (a setting-up step, not a verdict) and kept with the cache; from then on the machine decides *(the tool: `sig-save`, `--make-refs`, `expect screen`, references in the corpus's cache; `flash-signature`, a plant: the same screen 0.0 in three separate runs across the shells, another 8.4 against a same-screen bound of 5. Each title's own signatures are T-576's)*
- [x] A plugin wedged by an earlier run gets a verdict of its own and is never read as a slow frame rate (it cost a false diagnosis, 2026-08-10); plugin-container's process id across a test and its unexpected exits are reported *(`expect plugin`, `flash-plugin` on the three shells: alive, wedged (a planted SIGSTOP: state T, the CPU's ticks still) and gone (SIGKILL) told apart; `fl_plugin_pid` follows the process through a test)*
- [x] Sound is machine-read as far as a machine can: the audio stream open and running for the movie's process, and closed after it. Hearing stays a person's *(`expect audio`, `flash-audio` on the three shells: the player's stream on PulseAudio by its process id and the codec's PCM RUNNING, then closed on leaving the page. Written first, it read closed with a movie sounding: the bundled Jessie libasound could not load its pulse plugin and the player had no stream at all, since 2026-08-15; that fix is in the bundle now)*
**Dependencies:** cavekit-finger-testing.md R1-R3, R2 (the frame stream)

## R3: The bar — the stock browser on the same file
**Description:** webOS's own browser runs the same player on the same device, so it says what the player itself can do. Every title is measured there with the same tools, where it runs on CE 3.1.0.
**Acceptance Criteria:**
- [x] For each title the stock browser's content rate, shown-frame spacing and reaction time are measured by the same tools and stored with the corpus; a title the stock browser cannot run is marked so, drawn or not drawn by R2's check *(2026-11-06, T-573: `tools/device/finger/flash/title-round.py stock`, the numbers in `flash-corpus.json` under `stock`. CE's launcher sends the system's browser to Atlas, which has no Flash, so `flash/stock-bench.sh` installs a copy under a test id for the round and takes it away. 49 of 57 draw there; those that show one colour after 40 s on the page are marked so. The stock server's player dies after some 40 movies in a row, as this browser's does (T-580): a blank tail was run again)*
- [ ] In each shell a title's content rate is at least 90 % of the lower of its authored rate and the stock browser's rate
- [ ] Where the stock browser cannot run a title or cannot be measured at all, the bar is 90 % of the lower of its authored rate and the rate it reaches on this browser's own software path; the report says which bar each title was held to
**Dependencies:** R1, R2

## R4: Embedding — every way a page carries a movie
**Description:** real pages carry movies in many ways. Each one draws at its bounds and takes input, and none takes the daemon down (a windowed plugin once did, T-330).
**Acceptance Criteria:**
- [ ] Each wmode (window, opaque, transparent, direct, gpu) draws at the element's bounds and takes a tap; a transparent movie shows the page behind it
- [ ] `<embed>`, `<object>` with a movie param, nested objects and a direct navigation to a `.swf` each draw and take a tap; the common detection script finds Flash and reads 10.3 from `navigator.plugins` and `navigator.mimeTypes`
- [ ] FlashVars arrive; ExternalInterface works both ways; `getURL` and `fscommand` reach the page; `allowScriptAccess` is honoured; a local shared object survives a daemon stop and start; a movie loads another movie and data from its own origin
- [ ] Each scale mode, a percentage size, three movies on one page, a movie in a frame, a movie scrolled in from below the fold, and a movie shown after being hidden each draw at their bounds with input landing right
- [ ] Full screen is entered and left by the finger, or the limit is recorded under this kit's verdict rule
- [ ] At least 5 real sites that carry movies are visited in each shell, and each outcome is recorded by host; a site that is down is recorded as down and is no verdict on Flash, and one that forces Ruffle is recorded with whether a changed browser identity alters it
**Dependencies:** R2

## R5: Input — the finger as a mouse, and the keys of a device that has no keyboard
**Description:** a movie is played with a finger: tap, hold, drag and the gesture lock. A movie that wants keys is playable on a TouchPad with no keyboard attached, and keys meant for a movie do not act on the card.
**Acceptance Criteria:**
- [ ] On the instrumented movie a tap gives one press and one release at the stage point, and never a doubled click (a tap was once two clicks, T-535)
- [ ] A hold keeps the button down for its whole length (Helicopter flies), and a drag sends moves at 20 or more a second along its path
- [ ] The gesture lock is engaged and released by the finger: while engaged a drag moves the movie and not the page; released, or outside the movie, the page pans as usual. Each state is read from an oracle
- [ ] Tap to first changed frame inside the instrumented movie: median at most 50 ms above the stock browser's on the same movie; if the stock browser cannot be measured, at most 200 ms
- [ ] The system keyboard can be raised over a movie by the finger, with no editor focused; arrows, space, letters and digits typed on it arrive as the right key codes on the instrumented movie, and it can be dropped again
- [ ] With the keyboard up the movie's play area stays on the screen, or moves so that it does, and stage coordinates still land right
- [ ] With an emulated kernel keyboard (input-bridging R2a) and the gesture lock engaged, keys reach the movie and the card acts on none of them: no address-bar focus, no scroll, no back. With the lock released, the keys go to the card as usual
**Dependencies:** R2, cavekit-input-bridging.md R2a, R5

## R6: Games — every game played, in every shell
**Description:** each game is started and played by a script in stage coordinates, long enough to show that it is playable. Winning is not required.
**Acceptance Criteria:**
- [ ] Each game in each shell reaches its first interactive screen by its reference signature, is started by the finger, and takes a scripted input sequence of at least 60 s with the reacted check true throughout
- [ ] During that minute the content rate meets R3, the shown-frame spacing's p90 is at most twice the authored frame period or within 10 % of the stock browser's, and no check reads the movie as blank
- [ ] During that minute plugin-container does not exit, the card does not close and the daemon logs no fault
- [ ] Each keyboard game is played on the system keyboard of R5 in each shell, or it is a limit with the reason (the keyboard covering its play area counts as our fail unless the stock browser does the same)
- [ ] Two heavy games and two others each run 15 minutes, and plugin-container's resident memory and the content rate at the end stay within 10 % of their values at minute 1
**Dependencies:** R1, R2, R3, R5

## R7: Apps — a task done in each, and text that lands
**Description:** an app is proven by its task's result, not by the fact that it opened.
**Acceptance Criteria:**
- [ ] Each app in each shell completes a scripted task checked by a signature of its result: a drag leaves a stroke along its path and nowhere else; a calculation shows its answer; a chart draws and reacts to a tap; a list scrolls by drag
- [ ] Text typed on the system keyboard lands exact in a movie's text field, on the instrumented movie by its report: mixed case, digits and Backspace, in a TextField, and in a Flex form when one is in the corpus
- [ ] Clipboard between a movie and the page is exercised both ways and recorded by this kit's verdict rule, as are FileReference upload and download
**Dependencies:** R1, R2, R5

## R8: Videos — played to the end, controlled by the finger
**Description:** each clip plays to its end in each shell at its codec's rate, its picture always present, and its player's controls answer the finger.
**Acceptance Criteria:**
- [ ] Each clip in each shell plays to its end by its player's report or its known length, with the video's rectangle never blank and the content rate meeting R3
- [ ] On the instrumented player the dropped frames are reported per codec and size, and the table says which sizes each codec holds on this device
- [ ] Pause holds the picture still and resume moves it; a seek changes the reported time to the target within 2 s and the picture follows
- [ ] The audio stream is open and running for each clip's whole length; audio-to-picture sync is a person's, on R14's clips
- [ ] A progressive download whose connection is throttled to below the clip's rate, and then stalled for 10 s, recovers and plays on with no plugin exit
- [ ] At least 2 real pages with a Flash video player are visited in each shell, and each outcome is recorded; a page that is down is no verdict on Flash
**Dependencies:** R1, R2, R3

## R9: Sound with the movie's life
**Description:** a movie's sound starts and ends with the movie, and follows the card.
**Acceptance Criteria:**
- [ ] The audio stream closes within 2 s of navigating away, of a reload and of closing the card, and no stream is left open by plugin-container
- [ ] With the card sent to the background the sound behaves as in the stock browser on the same file, and it comes back when the card does
- [ ] A music player app runs 10 minutes with its stream open throughout and no plugin exit; clicks and dropouts are a person's, on R14's clips
**Dependencies:** R2

## R10: The movie's life — teardown, crashes, memory, more than one
**Description:** movies are opened and left many times over, and a bad one costs only itself.
**Acceptance Criteria:**
- [ ] Twenty cycles of navigating away, back, reload and closing the card on each of three titles leave no wedged plugin, no second plugin-container and no orphan, with the daemon's resident memory within 10 MB of its value after cycle 1
- [ ] Leaving a page with a movie ends plugin-container without an abort: no `mozalloc_abort` and no crash report (PICKUP's item 4)
- [ ] A corrupt or hostile library in the plugin folder does not wedge the daemon: the daemon reaches `serving YAP` and the page loads, with that plugin absent (PICKUP's item 5)
- [ ] plugin-container killed three times in a row: the daemon survives, the card shows a state a person can act on, a reload brings the movie back, and a crash loop stops itself within its bound
- [ ] A background card's movie publishes no frames, and its CPU boost is released within 5 s; back in front it draws and takes a tap within 2 s
- [ ] Two cards with movies at once, in one shell and across two shells, both draw and take input; the largest title in the corpus runs with the device's other cards closed and does not exhaust memory, or it is a limit
- [ ] The Flash switch off means no plugin instantiates on a corpus page and the page says so; on brings it back; the CPU boost is released after the last movie goes
**Dependencies:** R1, R2

## R11: The movie on its page — pan, zoom, rotation, the frame stream
**Description:** a movie stays part of its page while the page moves, on the default path.
**Acceptance Criteria:**
- [ ] Under a drag, a flick, a pinch and a double tap on a page holding a playing movie, the frame stream reports the movie's rectangle moving with the page and no stale or uniform bands beyond the page's own control round without a movie
- [ ] At the settled zooms of R2 the movie is redrawn at its new bounds and input lands right
- [ ] A rotation by the card's own request lays the movie out again at its bounds, and input lands right
- [ ] The daemon's verify mode reports no structurally wrong rows outside the movie's rectangle while a movie plays, on the default GPU path
- [ ] Page content over and under a movie stands in the right order for an opaque and a transparent movie, with a fixed header over a scrolling movie; a tap on page content standing over a movie reaches that content and not the movie
**Dependencies:** R2, cavekit-gpu-rendering.md R2, R3

## R12: Finished, and honest about the limits
**Description:** the end state is a table a person can trust, and every open Flash criterion in another kit answered.
**Acceptance Criteria:**
- [ ] Each title newer than the player (R1's controls) is recorded as drawn or not drawn by R2's check, with its frame kept, and none of them crashes the plugin or the daemon
- [ ] One table, generated from the last report and kept in the docs, gives each title's verdict in each shell with its measured rates and, for a limit, the evidence
- [ ] No title's verdict is a fail in any shell
- [ ] Every criterion this kit's preamble moves here is closed in its own kit, citing a script id, or marked a person's with the reason
**Dependencies:** R3-R11

## R13: One command, the same discipline as the suite
**Description:** the corpus runs as part of the finger suite, under the suite's rules.
**Acceptance Criteria:**
- [ ] Each title is a script of the suite with an id and its verdict rule, and the corpus round runs unattended on the three shells from one command and ends with the device as it began, the corpus files removed from the device
- [ ] A smoke subset (at least one title of each kind and the instrumented movies, one shell, under 15 minutes) exists for any change to a Flash path
- [ ] The report compares each number with the stored baseline, and a content rate fallen by more than 10 % is a failed check
- [ ] A script holds stage coordinates and chrome names only; pictures are kept for failures and for reference signatures, and game art stays in the cache
**Dependencies:** cavekit-finger-testing.md R4, R5

## R14: Accuracy against a person (BLOCKED-EXTERNAL: the owner at the device)
**Description:** the suite's verdicts are held against the owner's on real games, as cavekit-finger-testing.md R7 does for the list.
**Acceptance Criteria:**
- [ ] The owner plays at least 5 games, 2 apps and 2 videos of the corpus on the same build; their verdicts sit beside the suite's, and each disagreement is dispositioned (the suite fixed, or the item moved to a person's with the reason). A title the owner finds broken goes back to fail until that is done, so R12's table holds only after this round
- [ ] The owner's word on game feel, audio-to-picture sync and clicks or dropouts is recorded beside the stream's numbers, and the two agree or the gap is named. This answers addons R7's smoothness question and T-364
**Dependencies:** R6, R7, R8 (BLOCKED-EXTERNAL)

## Out of Scope
- Silverlight, Moonlight and other plugins: addons R7 records why not.
- Ruffle or any other player, and patching `libflashplayer.so`: the stock player is the one being finished.
- Content the player cannot run (SWF 13 and newer, Stage3D): recorded by R12, not made to run.
- RTMP streaming, DRM video, a webcam or a microphone: no server or device on the bench. A movie that needs them is a limit.
- Bluetooth keyboards and hearing: a person's. Stock webOS 3.0.5 and the TouchPad Go: not testable here.
- Winning a game: a game is proven playable, not beaten.

## Cross-References
- Depends on: `cavekit-finger-testing.md` R1-R5 (the check library, frame stream, geometry, script form, runner); `cavekit-finger-emulation.md` R1-R2 (the emulated finger); `cavekit-input-bridging.md` R2a, R5 (the kernel keyboard, plugin input); `cavekit-gpu-rendering.md` R2, R3, R7.
- Takes over open criteria from: `cavekit-addons-extensions.md` R7; `cavekit-input-bridging.md`; `cavekit-offscreen-rendering.md`.
- Related: `cavekit-preferences-ui.md` (the Flash switch); `cavekit-device-build.md` (the role and the plugin's library closure).

## Changelog
- 2026-11-06: R3's stock numbers (T-573). The first comparison: on Enyo as shipped many games run at half the stock rate or less, which is T-576's first work.
- 2026-11-06: R2's plugin checks (T-572), each proven against planted faults on the three shells; one criterion stays open for each title's own signatures (T-576).
- 2026-11-06: R2's first criterion met (T-571): the instrumented movies, AVM1 and AVM2, assembled in the tree.
- 2026-11-06: R1 met (T-570): the manifest and the tool `tools/host/flash-corpus.py`, 61 titles, the spread read from the files.
- 2026-11-05: created from the owner's request; R1-R14; one independent review (`ck:cavekit-reviewer`), its findings applied: a fallback bar the player can reach, one latency bar, tested signatures, machine-read controls, a tap on content over a movie, a site down no verdict. The bars of R3, R5 and R6 are the author's first numbers, to be revised once the first stock round has measured the device.
