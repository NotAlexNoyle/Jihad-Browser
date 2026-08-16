# PICKUP — Jihad Browser, current state (2026-08-10 body, deltas 2026-08-15 + 2026-08-16 below)

**Read this file first. It is the only current handoff.** Everything else in `docs/` is
reference material for a specific subsystem, not a status document.

---

## 2026-08-16 session delta — bitbake `.ipk`s ship + the media scrubber is fixed (read first)

The two biggest open items are CLOSED, and this session's work IS committed (branch
`jihad/about-preferences-and-settings-merge`; engine mods on `third_party/uxp` branch
`jihad-engine-mods`).

- **OE bitbake produces all three `.ipk`s (device-build R3 DONE).** `bitbake
  net.riverstonerelay.jihad-browser{,-mochi,-mojo}` → `639 tasks ... all succeeded`, zero errors;
  each `.ipk` extracted + verified COMPLETE by content (daemon, libxul, plugin-container, ld-2.23,
  goanna.js, 1104-file chrome). Five recipe gaps were fixed to get here — see `../context/plans/build-site.md`
  T-154 and `../context/impl/dead-ends.md`. **Trap that ate two "successful" builds:** a goanna
  SRCREV bump does NOT re-fetch in this bitbake-1.18 setup (do_unpack keeps the old checkout); a
  chrome-only change needs `bitbake -c cleansstate goanna jihad-deviceroot
  net.riverstonerelay.jihad-browser{,-mochi,-mojo}` (after pre-warming the DL_DIR git mirror) + a
  CONTENT check of the packaged file, never mtime/size.
- **Media scrubber jitter/flash/no-settle FIXED (gre-widgets R1 back to `[x]`), device-confirmed.**
  Instrumentation proved the media/DOM state is perfect — the defect was the daemon's offscreen
  paint: damage-only repaint into three rotating buffers left GHOST thumbs (each buffer got only the
  latest frame's damage though it was several frames stale). Fixed with per-buffer damage
  accumulation (`BrowserPageGoanna` `BufFrame.dmg*`). Plus videocontrols: `ended` snaps the thumb to
  `duration`, and a 40ms interval interpolates the thumb from the live currentTime clock. 40ms is
  the ceiling — faster (25ms) overruns the adapter round-trip and FREEZES (checked against Atlas
  BrowserServer's identical double-buffer + wait-for-return model). Residual un-smoothness is the
  ~25–30fps software-paint cap, i.e. hardware.
- **Audio plays** through the cubeb PULSE backend (`JIHAD_PULSE_SINK=pmedia`), device-confirmed
  audible — the ALSA-bridged path was policy-muted; the pulse-native path gets 100%.
- **Engine provenance changed:** the jihad engine delta is now a durable commit `0c10df5c` on
  `third_party/uxp` branch `jihad-engine-mods` (pristine UXP `b2594a4` + the delta), pinned by the
  goanna recipe SRCREV — not only the desktop patch queue. Reproducibility caveat: that commit is
  local-only; push `jihad-engine-mods` to the UXP fork remote for a fresh clone to build.
- **Dotted-mochi `.ipk` was a 07-29 orphan (removed).** Current recipes emit only the HYPHENATED
  ids (`…-mochi` / `…-mojo`).

Device was used this session (TouchPad on novacom); it runs the pulse-audio libxul + per-buffer-fix
daemon + scrubber-fixed videocontrols. The 2026-08-15 and 2026-08-10 sections below are retained as
history — where they say "nothing is committed" or "the device is absent", read this delta instead.

---

## 2026-08-15 DEVICE session — the TouchPad reappeared near the end (historical delta)

Full transcript: `context/impl/impl-device-2026-08-15.md`. The device was plugged in mid-session and
the enyo variant was redeployed (ipk 1.0.4 + current daemon/adapter). Verified ON HARDWARE:

- **Cert store (T-135) — CLOSED end to end.** `/var/ssl/jihad/enyo` created (daemon is in group
  `luna`), real DER→PEM written, classified to ordinal 18 from `nsISSLStatus` flags, `CertInitCertMgr
  rc=0`, `Adding Entry … serial 6`, `CertAddTrustedCert` rc=0, NSS override + reload, page loads.
  `libPmCertificateMgr.so` present on 3.0.5. browser-services R5 device box → `[x]`.
- **Toast channel (T-146) — CLOSED.** `clearCookies` on the public bus pushed a notify and the Enyo
  card rendered the "Cookies cleared." toast, visible in a device screenshot. gre-widgets R5 last → `[x]`.
- **Focus transitions correct on device** (editorFocused 1→0→1); the desktop focus_test failure is
  harness-side, not the daemon.
- **AUDIO: the backend works and reaches pulse, but output is webOS-POLICY-gated — and the gate is now
  FULLY reverse-engineered.** `<audio>` gives `err=0` and pulse shows our sink-inputs at
  `s16le 2ch 44100Hz`, but sinks stay SUSPENDED — webOS powers the WM8994 codec route only while a
  policy-recognised media stream plays. Stock mechanism: `element.palm.audioClass="media"` (Palm
  WebKit extension, `media.js:288`) → `pmedia` sink + audiod media scenario. Port's path (specified,
  three parts, none human-gated, in `impl-audio-backend.md` §2026-08-15): (1) route engine ALSA at the
  `pmedia` sink via `ALSA_CONFIG_PATH`, (2) hold a `com.palm.audio/media` scenario ref-counted over the
  playback lifetime (daemon already has an outbound Luna client), (3) validate. the mechanism is now
  FULLY DECODED and the fix is DETERMINED (a bounded build task, not research). Read
  `module-palm-policy.so`: virtual sinks (pmedia/…) are NULL sinks; audiod bridges one to hardware +
  powers the codec ONLY for a stream CREATED on that virtual sink; default-device streams are muted
  ("PLEASE CATEGORIZE USING A VIRTUAL STREAM"). Engine decode+clock WORKS (t 0→3.00, ended, err=0) but
  is policy-muted on pcm_output at 0%. **The ALSA backend was a DEAD END (policy-muted); the cubeb PULSE
  backend was BUILT + DEPLOYED + DEVICE-VERIFIED 2026-08-15 and it UNMUTES engine audio.** The engine
  stream is now pulse-native (`application.name="Jihad Browser"`), `Volume: 100%`/uncorked (not the
  ALSA path's forced 0%), and `/proc/asound/…/status` shows the WM8994 DAC consuming its samples
  (`hw_ptr` advancing ~44100/s). Implemented: mozconfig `--enable-pulseaudio`, pulse-0.9.22 headers
  staged in the sysroot (device-ABI-matching; re-fetch via `curl
  freedesktop.org/software/pulseaudio/releases/pulseaudio-0.9.22.tar.gz`, headers `src/pulse/*.h` +
  a `libpulse.pc`), patch `0030-cubeb-pulse-webos-virtual-sink.patch` (`JIHAD_PULSE_SINK`; cubeb
  dlopens libpulse, no link dep — all 62 symbols present in the device lib). The pulse-native app
  IDENTITY is what unmutes it. **Residual (the only thing left, needs a human EAR — both automated
  oracles broken here: `parec` monitor = 0 bytes, `hw_ptr` runs during pause): whether the speaker
  physically emits, plus a possible daemon-side hold of the `com.palm.audio/media` scenario to power
  the speaker amp (the unmute does not need it, measured).** Full spec: `impl-audio-backend.md`
  §"RESULT 2026-08-15". **Bundle-Jessie-libasound decision is WRONG:** Jessie 1.0.28 → `err=3`,
  device's own `/usr/lib/libasound.so.2` → `err=0`. Stop shipping the bundled one. MP3/AAC/H.264
  decoders remain a separate, orthogonal gap.

Device traps found: two stray `jihad.bak.*` upstart jobs were contending for the enyo socket
(removed); cryptofs forbids symlinks; a `file://` XPI navigation does NOT trigger the install flow
(so T-103's device screenshot is still open — needs an http-served or card-initiated XPI). Device
left: enyo on current binaries, job env back to shipped flags, test certs swept, adapter backup at
`/var/palm/jihad/adapter-backup-20260815/`.

## 2026-08-15 session delta — read before the 2026-08-10 body

A cavekit loop (`context/plans/build-site.md`, now reconciled row-by-row; per-wave record in
`context/impl/loop-log.md` Iteration 3) closed or advanced every host-doable open criterion.
No device all session, so nothing below is device-verified unless it says so.

**Closed outright:** ipc-contract R1 (generator vendored+patched, regeneration reproduces the
shipped wire, zero hand edits — `render/browserserver/CodeGen/`); gre-widgets R5 first criterion
(notificationbox attach/dismiss PASS on the offscreen PuppetWidget path — `prefsui_test`);
gre-widgets R7 prefs-single-source (eleven rows observed from the shared file on desktop);
preferences-ui flush box (was already device-proven 2026-08-10, box reconciled); opal machine
config (kernel string was WRONG — board is `shortloin`; pinned from HP's own source drop);
T-115 prebuilt-input manifests (187-deb sysroot + adapter-deps, generators in `build/webos-oe/`).

**Built, awaiting a device:**
- **Engine audio output** — cubeb ALSA backend is IN the ARM libxul (`--enable-alsa`, Jessie
  alsa-lib staged; `DT_NEEDED libasound.so.2`). First listen: `jihad-media.html` with an
  Ogg/Vorbis or WAV asset (MP3/AAC/H.264 decoders are STILL OFF — ffvpx config, separate gap).
  Fallbacks if silent, in order, in `context/impl/impl-audio-backend.md` (PCM `default` may route
  to a nonexistent pulse plugin; `ALSA_CONFIG_PATH` override; drop-bundled-lib).
- **Cert store (T-135)** — all three 2026-08-10 defects fixed and desktop-proven live (real PEM
  certFile, Palm-ordinal error mapping off `nsISSLStatus` — the raw nsresult was a transport-class
  lie —, three-way trust with session-serial sweep at teardown AND startup). Platform writes via
  dlopen'd `libPmCertificateMgr`; first device line: `ls /usr/lib/libPmCertificateMgr.so*`.
  Read-direction (store→NSS) is COSTED not built (`context/impl/impl-cert-store.md`).
- **Touch events (T-120)** — fence out behind **`JIHAD_TOUCH_EVENTS=1` (default OFF)**; 28-byte
  header re-staged (sizeof guard proven both ways); double-activation suppressor daemon-side;
  device procedure in `context/impl/impl-touch-events.md`. Two real bugs fixed en route (TouchEnd
  OOB index; `npPalmEnableTouchEvents` was never set — un-fencing alone was a silent no-op).
- **Non-blocking toast channel (T-146/T-148)** — `jihad::PostNotification` → `notifications` Luna
  subscription → plain-DOM toasts in all three shells. The only informational dialog (T-103
  incompatibility alert) moved onto it; cookies/cache-cleared and add-on-installed notices ADDED
  (they emitted nothing before). Three device unknowns; one Preferences→Clear-cookies run splits
  them (`context/impl/impl-toast-channel.md`).
- **XPI mismatch alert (T-103)** — observer fires on desktop with a discriminating control; device
  screenshot is the closer.

**NEW TRAPS, same class as the body's:**
- **`out-ipk/` ipks ≤1.0.3 have NO libasound.so.2 and their daemon will NOT START** against the
  new libxul. Install 1.0.4+ only; the enyo 1.0.4 / mochi / mojo ipks built 2026-08-15 carry it
  (verified in all three).
- **`build-goanna-arm.sh` exited 1 on every SUCCESSFUL build until 2026-08-15** (grep|while under
  pipefail; 27/29 patches have no JS). Fixed; historical automation that trusted its exit code
  never saw its own staleness guards run.
- **`mFlashGestureLock` is set ONLY by a DOUBLE-tap** (`handlePenDoubleClick`, which also
  smart-zooms) — the "tap inside a plugin rect" wording in older notes is wrong, and NO latch log
  compiles (`Debug.h` DEBUG off). T-124/T-144 rescoped; keyboard-arbitration procedure rebuilt
  around the address-bar screenshot (addons kit R7, 2026-08-15 annotation).
- **`javascript:` URLs are dead in ALL chrome documents** (HTML too, not just XUL) — so
  `GoannaRenderPage::ScrollTo` is a no-op on chrome pages. And a **stale desktop profile** fakes
  pref regressions (see `context/impl/dead-ends.md`, both dated 2026-08-15).
- The dotted Mochi app id in the workspace CLAUDE.md is fixed (hyphens are correct).

**What remains, exhaustively:** every open criterion is device-, hardware-, or human-gated. With
a device + novacom (needs root for novacomd): the verification batch above, T-108 (hidd routing),
T-116, T-119/T-133, T-131, T-141, T-142→T-147. Human at device: T-150/T-151/T-152 and the frame
pacing EYEBALL decision (device deliberately left in Flash-alone — body below). Human decisions:
T-155 sign-off, T-156 Mojo settings exemption, whether to commit this tree (137+ files were
already uncommitted before 2026-08-15; the session added more and committed nothing).

---

## What this is

Jihad Browser: a fork of isis-browser (webOS 3 / HP TouchPad) with **UXP/Goanna** replacing
QtWebKit, keeping the isis UI and the BrowserAdapter↔BrowserServer YAP IPC contract. Three
variants ship independently: Enyo (`app/`), Mochi (`app-mochi/`), Mojo (`app-mojo/`).

Requirements live in `context/kits/` — start at `cavekit-overview.md`. The Flash/NPAPI work is
`cavekit-addons-extensions.md` **R7**.

---

## Flash: where it actually stands

Adobe Flash renders animated content on the TouchPad through UXP/Goanna, takes mouse and
keyboard input, **plays sound**, survives crashes and freeze/thaw, and holds ~30 fps composite
with patch `0027`. Two things are UNFINISHED: chrome-side keyboard arbitration (untestable on
this hardware) and frame PACING — the composite RATE is met, and the pacing ROOT CAUSE is now
found and measured: **two producers.** Flash invalidates itself and we also drive it, and our
frames land between its own. Isolated three ways; Flash alone is slower but has **zero** gaps
under 16 ms. The decision left is a design one, not a debugging one — see open work item 2.
**Audio is CLOSED**, at parity with stock; an
earlier version of this sentence listed "MP3 static (fix landed, unheard)" and that contradicted
the dated audio section below, which cost a session start. Precisely:

| | state |
|---|---|
| renders animated content | ✅ device-verified |
| **frame rate** | ✅ **composite 30.6-32.5 fps** via patch `0027` (CPU boost); gap avg 28-29 / max 45-49 ms |
| **mouse input** | ✅ `palm event 0x1/0x2/0x10000` at correct coordinates |
| **keyboard input → plugin** | ✅ Flash's own AVM1 runs on a keypress (green 11768 → red 11750) |
| **keyboard arbitration (chrome)** | ⚠️ **UNTESTABLE on this hardware** — no keyboard exists; synthetic one is read by hidd but never dispatched |
| crash containment | ✅ daemon survives a plugin SIGSEGV; plugin respawns on reload |
| spotlight event | ✅ suppressed by default (delivering it kills Flash — matches stock) |
| **audio** | ✅ **CLOSED 2026-08-10 at parity with stock.** All three defects were in the TEST ASSETS. The residual per-loop click is Flash restarting its MP3 decoder, and the STOCK browser does it identically |
| **frame PACING** | ⚠️ **ROOT CAUSE FOUND 2026-08-10: TWO PRODUCERS.** Flash invalidates itself ~25/s AND we drive it ~26/s, and our frames land between its own — that interleaving is the bunching. Isolated and measured all three ways: Flash alone is **slower but perfectly unbunched (zero gaps under 16 ms)**, the pull is faster and bunched. The device is LEFT in Flash-alone for a human to eyeball |

### Audio: solved, and the bug was in the TEST ASSET

**Flash audio needed no port-side change at all.** Every "silent" measurement in this tree was
taken against SWFs that could not have made a sound in any player.

`make-audio-swf.py` emitted `DefineShape` **and** `DefineSound` with the **same character ID 1**.
A SWF has ONE character dictionary shared by shapes and sounds, so the sound redefined an ID that
was already taken; the player keeps the first definition and drops the sound, `StartSound 1` then
resolves to a shape, and nothing plays. Silently: no error, no audio device ever opened, no mixer
thread. All three assets (PCM, MP3 mono, MP3 stereo) shared the bug, which is exactly why they
"behaved identically" and made this look like a player-side gate.

Fixed by giving the sound its own ID (`SOUND_ID = 2`). With the STOCK, UNPATCHED
`libflashplayer.so` the corrected SWF immediately produces the whole ALSA bring-up that was
missing for the entire investigation:

```
open("/usr/share/alsa/alsa.conf")                       = 27
access("/etc/asound.conf", R_OK)                        = 0
open("/usr/lib/alsa-lib/libasound_module_pcm_pulse.so") = 27
open("/usr/lib/libsndfile.so.1")                        = 27
```
plugin-container goes **3 → 5 threads**, and the tone is audible from the device speaker.

All three assets have been regenerated and re-pushed. Regenerate before trusting any audio
measurement anyway — `/tmp` is volatile, and any SWF still sitting on a device from before
2026-08-10 is a broken one. The `--mp3` variants need a BARE frame stream with no ID3; make one
with `ffmpeg -f lavfi -i "aevalsrc=..." -codec:a libmp3lame -write_xing 0 -id3v2_version 0`.
PCM is confirmed clean by ear. **MP3 was heard too and IS clean** — the per-iteration click is
Flash restarting its decoder at the loop point, and the stock browser does it identically, so it
is not a port defect. Judge MP3 quality ONLY with `jihad-audio-long.swf` (30 s, `--loops 1`),
which has no loop point at all; every other MP3 asset here clicks once per iteration no matter how
healthy the port is. There is no open audio item.

### The "audio gate" in the previous handoff was the VIDEO DECODER — do not chase it

The function at **0x70ee0** and the loader at **0x73020** have nothing to do with audio. Measured,
not guessed: 0x73020's string table is `libmm-omxcore.so` / `libOmxCore.so` resolving `OMX_Init`,
`OMX_GetHandle`, `OMX_GetComponentsOfRole`; 0x70ee0 passes `"video_decoder.avc"` (rodata 0x760cec)
and `strncmp`s candidates against `"OMX.Nvidia"` (0x760d00) with 128-byte entries
(`OMX_MAX_STRINGNAME_SIZE`). It is hardware H.264 decoder setup. `+24` is a codec enum, and it
**does** equal 7 — the factory at 0x5bff0 returns NULL unless the caller asks for type 7 and the
constructor at 0x72a30 stores that same 7 (`72a94: str r4,[r8,#24]`). The old note had the gate
inverted as well as in the wrong subsystem.

**Where the real ALSA code lives**, if it is ever needed again (all GOT-relative, base 0x85897c):
- **0x2f7bd0** — the ALSA loader. `dlopen("libasound.so")`, falls back to `"libasound.so.2"`,
  resolves ~50 `snd_*`, then NULL-checks every one; any NULL and it returns 0.
- **0x2f71e0** — `OpenPCM`, vtable slot +8 at vptr 0x84e440, ctor 0x2f75f0. Opens the PCM, sets
  access/format/rate/channels, 500 ms buffer / 20 ms period, then `pthread_create`s the mixer.
  It asks for **`plughw:0,0`** (0x7b54c8) as its FALLBACK name — the old note's `hw:0,0` was that
  string read four bytes in.
- `0x2fbf08` is a separate availability **probe** that loads libasound unconditionally at startup.
  That is the `dlopen` every previous trace saw; it proves nothing about playback.

**Already eliminated — do NOT redo these:**
- Our runtime can play sound. `render/goanna/test/alsa_probe.c` writes a square wave and it is
  AUDIBLE, under both the device's own loader and `./ld-2.23.so --library-path $HL`.
- Not PIpc (zero `PIpcClient` ctors even with an audio SWF), not LS2 permissions (role grants
  `outbound:["*"]`), not Flash's OEM profile (`/media/cryptofs/apps/etc/adobe/{mms,oem}.cfg`,
  `cctrl = 3`), not click-to-play deferred start, and not the host — `libWebKitLuna` has no
  PluginView audio methods and no ALSA/pulse references at all.
- Not the two gates that looked like candidates. Both were nopped on-device and changed NOTHING:
  the sample-count check `2f7234: cmp r1,#31 / ble` in OpenPCM, and the `audioEnabled` check
  `19ba54: beq` on `sound+0x270`. That pair of null results is what finally pointed at the asset.
- One real bug WAS found and fixed here: Flash's `kind not registered:
  'com.palm.app.flashplayer.prefs:1' (-3970)` on every instantiation. The db8 kind is now
  registered (owner `com.palm.flashgraphics`) and the error is gone. It was unrelated.

---

### Frame rate: the cause is CPU FREQUENCY, not the pipeline and not Flash

**Measured 2026-08-10.** Sampling `/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq` once a
second while `jihad-anim.swf` played:

```
192000 192000 192000 192000 192000 192000 192000 540000 384000 384000 384000 384000 384000
idle, right after navigating AWAY: 1188000
```

Flash renders at **192-384 MHz out of a 1188 MHz maximum**. Frame rate tracks it exactly — daemon
paints 20.8-23.9 fps, composite 16.3-29.1 fps — and the SPREAD is what reads as "skippy". The
system was only 69% busy across 2 cores at the time, so this was never a capacity wall; the
pipeline is clean (`deferred=0`, `wanted == done`), and on one run at a higher clock the same
pipeline reached **39.2 fps**, well above the 30 the content is authored at.

The governor is Palm's **`ondemandtcl`** — touch-biased. It boosts while the user is touching the
screen and decays otherwise, so passive playback (watching an animation, listening to audio) runs
at the floor. This also explains the audio xruns: MP3 decode at 192 MHz underruns.

**DO NOT "just set the governor to performance" — it WEDGES THE DEVICE.** Writing
`scaling_governor` while `ondemandtcl` is live deadlocks the cpufreq policy lock in the kernel.
Every later reader of any cpufreq sysfs node blocks in **uninterruptible D state** — including the
Jihad daemon, `powerlog`, and the restore script itself, so the governor never gets put back and
the daemon stops painting. D state cannot be killed; only a reboot clears it. Nothing persists
(it is a runtime sysfs value), so a reboot returns to `ondemandtcl`, but the whole session's
measurements are lost. This cost a device reboot on 2026-08-10.

**THE SAFE LEVER, AND IT WORKS — measured 2026-08-10.** `ondemandtcl` exposes ordinary governor
tunables at `/sys/devices/system/cpu/cpufreq/ondemandtcl/`. Writing those does NOT switch
governors and does NOT take the path that deadlocks; the write returns instantly and a readback
succeeds. Stock values and the two that matter:

```
up_threshold  = 95      <-- only ramps when a core exceeds 95% load
sampling_rate = 200000  <-- and only looks 5 times a second
```

95% is why Flash never boosts: the daemon runs ~26% and plugin-container ~11% of two cores, so no
single core ever approaches the threshold. Setting `up_threshold=40` and `sampling_rate=50000`
pins the clock at 1188000 and produces, on `jihad-anim.swf` (30 fps authored), single instance:

| | stock | tuned |
|---|---|---|
| daemon paints | 24.0-28.7 fps | **31.4-33.0 fps** |
| card composite | 20.4-27.5 fps | **30.7-35.2 fps** |
| frame gap avg / max | 34-42 / 63-81 ms | **30-31 / 49-56 ms** |

Composite now meets the authored rate AND is matched to the draw rate, which is what the
criterion asks for. `deferred` stayed 0 throughout, both before and after — the pipeline was
never the constraint.

**What is NOT yet done: making this OURS rather than a manual poke.** These are SYSTEM-WIDE
tunables, so the daemon must apply them only while a plugin instance is alive and restore the
stock values on teardown (and on crash — an upstart `post-stop` is the honest place for the
restore, since a daemon that dies with `up_threshold=40` leaves the whole device tuned for
battery-burn). Read the stock values rather than hard-coding 95/200000. Do NOT touch
`/sys/devices/system/cpu/cpufreq/override/{turbo_mode,vdd_*}` — that is overclocking, and
`vdd_freqs` goes to 1836000.

---

## Frame pacing — what was wrong, what is fixed, what is left (2026-08-10)

The user's report was "the pink square is smoother in the STOCK browser". It is a real defect and
the average frame rate never showed it: the daemon was delivering 35-42 fps for a 30 fps SWF.

**The measurement that found it is a gap HISTOGRAM, not min/avg/max.** Summary statistics cannot
distinguish "evenly late" from "unevenly on time", and every earlier session's numbers were
summary statistics. Added to the 2 s counter block; the failure looked like this:

```
gap hist ms <16=19 16-23=20 24-31=10 32-39=24 40-55=6 56+=0     <-- FLAT = no pacing at all
```

Frames were landing at effectively random instants between 8 and 55 ms, so the compositor held
each one for anywhere between half a refresh and three. Three separate causes, all measured:

1. **`PumpFor` burned the tick.** It `g_usleep(1000)`s every iteration, and a 1 ms sleep really
   costs ~3 ms on this device, so `PumpFor(4)` measured **12 ms** of a 16 ms tick with the page
   IDLE. The tick was therefore WORK-bound at 26-28 ms rather than timer-bound at 16, and since
   a frame can only be published on a tick, every gap quantised to that. Fixed by `PumpReady()`,
   which drains what is ready and returns without sleeping — **work avg 12 ms → 1 ms idle**.
   `PumpFor` KEEPS its old behaviour because the tests use it as "let the engine run for N ms".
2. **Nothing paced the publish.** The damage gate was an 8 ms floor deliberately set BELOW the
   tick so it never gated. A fixed 33 ms deadline grid (not a floor — a floor drifts and beats,
   which is what the 2026-08 note in the source records) moved 30% → 69% of frames into the
   32-39 ms bucket.
3. **The daemon could not tell a NEW plugin frame from a repeat**, because its only signal is
   PuppetWidget's sticky dirty boolean, which the refresh driver and ordinary page damage also
   set. That ambiguity makes BOTH available policies wrong: publishing on every dirty tick sent
   ~78 card frames for ~60 plugin frames (duplicates, and `deferred` 22-44 because the adapter
   could not drain them), while the 33 ms grid sent ~57 for ~68 and DROPPED frames, so the
   animation stepped two positions at once. Fixed by exporting `jihad_plugin_frame_seq()` from
   `PluginInstanceParent::RecvShow` (weak-linked exactly like `jihad_offscreen_take_dirty`) and
   publishing one card frame per plugin frame: **done=91 against palm draw=92, `deferred=0`.**

**Why stock is structurally smoother, and it is not a tuning difference.** Stock has exactly ONE
clock: `PluginViewQt`/`PluginViewPalm` pulls the plugin synchronously inside the page paint, and
the producer blocks until the consumer drains, so publish times ARE drain times. We have two
free-running clocks — the plugin child's own repaint timer (a delayed task RE-POSTED after its
work, so its period drifts) and the daemon's tick — and we SAMPLE the plugin rather than pull it.
That is the remaining gap, and it is the next thing to attack.

**Two things were tried and are NOT settled — the runs that judged them were confounded:**
- **An 8 ms tick** (`JIHAD_TICK_MS=8`). Works mechanically: tick period 9-11 ms, 1:1 delivery,
  `deferred=0`. But the plugin then redrew ~45 times/s for 30 fps content, so content frames are
  shown 1 or 2 times in a beat — even DELIVERY of uneven CONTENT, the same artifact one layer up.
  Worth revisiting only together with a draw rate that matches the content.
- **`JIHAD_PLUGIN_REPAINT_DIV=3`** on top of it, to bring draws back to ~30. Draw rate did land
  at 28-34 fps, but the daemon then logged `flashRect remove id=N (gone)` and stopped painting
  while the child kept drawing.
  **That was NOT caused by the divisor, and the first write-up of it here was wrong.** The same
  stall persisted after reverting to `JIHAD_TICK_MS=16` and the default divisor, and a CLEAN
  restart (`stop jihad; pkill -f jihad-browserserver; pkill -f plugin-container; start jihad;
  killall LunaSysMgr`) cleared it completely on the identical build: `done=61 palm draw=63
  deferred=0`. What actually happened is a plugin CRASH LOOP — the log carries a
  plugin-container maps dump — reached by cycling `inject url` between pages repeatedly. Once the
  child is in that state the plugin rect goes away, widget invalidation stops with it, and every
  later number reads as a pacing regression that is not one.
  **NEW TRAP, same family as the two below: repeated inject-driven navigation can wedge the
  plugin, and the symptom is silence, not an error.** `flashRect remove id=N (gone)` with the
  child still logging `palm draw` is the signature. Do a clean restart before believing any
  frame measurement that follows one.

Everything here is env-tunable for A/B without a rebuild: `JIHAD_TICK_MS` (4-64, default 8 in
code / 16 on the device job), `JIHAD_PLUGIN_FRAME_MS` (0 disables the pacer and the seq gate with
it), `JIHAD_PLUGIN_REPAINT_DIV` (1-8, child side), and — added 2026-08-10 because the frame
question could not be A/B'd without them — the two DRIVER switches:

| var | side | effect |
|---|---|---|
| `JIHAD_PLUGIN_PULL=0` | daemon | disables the host request ONLY, leaving the publish policy alone. Use this, NOT `JIHAD_PLUGIN_FRAME_MS=0`, which also drops the publish gate to the 8 ms floor and so changes two things at once |
| `JIHAD_PLUGIN_SELF_DRIVE=0` | child | disables the child's own repaint timer outright |

**Setting only `JIHAD_PLUGIN_PULL=0` does NOT isolate the plugin** — the child's host-drive lease
expires and its timer takes over, so you measure patch `0026`, not Flash. Both switches together
leave the plugin's own `NPN_InvalidateRect` as the sole frame source. Each logs a line when it
engages (`plugin PULL disabled` / `self-drive timer DISABLED`); grep for it before believing a run.

**And when reading `show req:`, `hostinval` counts everything that reached
`JihadPalmRepaintTick` — the pull AND the child timer. It is not "the pull" on its own.**

### The PULL is built and measured (patch `0029`, 2026-08-10) — and it did NOT fix the pacing

`0029` does exactly what the item above asked for: an async parent→child `JihadRequestFrame`, the
child answering it with its existing `JihadPalmRepaintTick()`, its own timer standing down on a
150 ms LEASE (not a latch — the daemon can stop asking at any moment, and a latched plugin would
then never paint again), and the daemon asking once per whole-tick grid slot from its own tick.
It is live on all three variants. **The mechanism works and the outcome is not a win:**

|  | `0028` (sample) | `0029` (pull) |
|---|---|---|
| daemon done | 29.4-31.9 fps | 32.8-34.0 fps |
| frame gap avg | 31-34 ms | 29-30 ms |
| frame gap max | 47-62 ms | 45-55 ms |
| gap hist `<16` ms | 1-4 of ~60 | **5-8 of ~68** |
| `deferred` | 0 | 0 |

Average and max improve; the HISTOGRAM gets flatter, which is the metric that matters. Both runs
were clean restarts of `jihad-anim.swf` at `JIHAD_TICK_MS=16`, log-growth and card-connection
asserted. Why it does not work is open work item 2: **the pull is not exclusive** — 3988 draws
against 3072 requests — so a third driver is producing most of the frames.

Two things worth keeping from this even if `0029` is later reverted:

- **THE TICK IS ITSELF A JITTERY CLOCK, and that bounds every pacing scheme.** Against a 16 ms
  timer the measured period is `min=15 avg=18 max=41`, with `work avg=10-11` (pump 7, paint 3).
  Locking the plugin to that clock inherits its jitter, so "one clock" is necessary and not
  sufficient. **`JIHAD_TICK_MS=8` is WORSE, measured, and not for the reason recorded earlier:**
  the tick cannot actually run at 8 ms (work is 11-13 ms, so the observed period stays 14-16 ms),
  the draw rate rises to 36-40 fps for 30 fps content, and the histogram spreads across the 16-23
  and 24-31 buckets. Leave the device job at 16.
- **There is no clean runtime A/B for the pull alone.** `JIHAD_PLUGIN_FRAME_MS=0` does stop the
  requests (the child's lease expires and it resumes self-driving, i.e. `0026`/`0028` behaviour),
  but the same variable also drops the publish policy to the 8 ms floor, so the two effects cannot
  be separated without a rebuild. Do not read a `=0` run as "the pull, off".

## Open work, in priority order

1. **Keyboard arbitration, card side — blocked on the hardware, not on the code.** Keys reach and
   drive Flash; what is NOT verified is that the CARD does not also act on them. The gate is
   deployed (`handleKeyDown/Up` on `mPageFocused && (bEditorFocused || mFlashGestureLock)`) and
   its return value IS the arbitration, so the expression is not in doubt — what needs proving is
   that `mFlashGestureLock` is true during play.

   **A human at the device cannot test this either.** There is no keyboard: `/proc/bus/input/
   devices` has only `gpio-keys`, `pmic8058_pwrkey`, `headset`, and webOS's on-screen keyboard is
   drawn by LunaSysMgr in-process, not through `/dev/input`. The daemon's `key` inject command
   goes straight to the page and bypasses the adapter entirely, which is why it proved "Flash gets
   keys" and can never prove anything about the chrome.

   `render/goanna/test/uinput_kbd.c` synthesises a real one (PDK build → device glibc, no bundled
   loader; `build` it with the PDK gcc plus `--sysroot`). `/dev/input/uinput` exists and works,
   the device registers as `event3`, and **both `hidd` and LunaSysMgr open it** — yet no key ever
   reaches the daemon and `window.pageYOffset` never moves. So hidd reads it and does not dispatch
   it. Next: `/etc/hidd/HidPlugins.xml` routes generic devices through `HidInputDev`
   (`/usr/lib/libhidinputdev.so`, `/var/run/hidd/InputDevEventSocket`) — find out what it requires
   of a device before it forwards. Also fix the observable: arrows were used on the assumption the
   chrome scrolls, which a touch OS may not do at all; the adapter's own comment says the leak
   appeared as keystrokes landing in the ADDRESS BAR, so screenshot the URL field instead.
   `jihad-keyarb.html` (tall page + key SWF + `gettext #sy` scroll readout) is staged for this.
2. **Frame pacing: the pull is BUILT (patch `0029`) and it is NOT the fix. Find the third draw
   driver.** The pull described in the previous handoff now exists end to end and is device-live —
   see the section above for the numbers. It did not tighten the gap histogram, and the reason is
   a measurement nobody had taken before: **the plugin's draw rate is not set by whoever you think
   drives it.** Over 123 s the child logged **3988 draws against 3072 host requests**, and at an
   8 ms tick the gap widened to 3988-vs-~1900 territory (38 fps of draws for ~15 requests/s). So
   the majority of draws — and ALL of the excess — come from neither the child's own timer (it is
   provably stood down: exactly one `frame clock -> HOST` transition and no fall back for the
   whole run) nor from Flash self-invalidating (`gJihadNPNInvalidateRectCalls` stays 0, which is
   precisely why the host-driven path keeps running).
   **ANSWERED 2026-08-10, and the suspect this paragraph used to name was WRONG.** The counters
   were added (every `AsyncShowPluginFrame` call site, tagged, reported next to `palm draw` as
   `show req:`) and `RecvUpdateBackground` measured **exactly zero** — as did every other
   background path. The hypothesis that it was parent-driven and correlated with daemon paints is
   refuted; do not re-run it.
   **It was OUR OWN LEASE FLAPPING.** Every draw traces to `InvalidateRect`, and the accounting
   closes to the unit: **1856 host requests + 1846 child-timer invalidates = 3702 total**, with
   the child logging `HOST -> SELF -> HOST`. The daemon does not re-ask while a request is
   outstanding — it waits out `kJihadPluginReqTimeoutMs` (250 ms) — but the child's lease was
   150 ms, i.e. SHORTER, so one late frame dropped the lease and restarted the second clock. The
   invariant is now written into the source: **lease > request timeout + one grid period +
   margin** (lease 500 ms). Verified: the clock log now shows one `-> HOST` and no fallback.
   **AND THEN THE REAL ONE — PATCH `0026`'s FOUNDING PREMISE IS FALSE.** With the lease fixed,
   1344 host requests still did not account for 2662 invalidates. Splitting the counter at the
   funnel by whether `JihadPalmRepaintTick` is on the stack settles it:
   **`hostinval` ≈ 27/s against `plugininval` ≈ 22/s.** Flash invalidates itself constantly.
   The reason three sessions believed otherwise is that `gJihadNPNInvalidateRectCalls` — the
   counter `0026` reads to decide "the plugin never asks to be repainted" — is **structurally
   blind**: it lives in `JihadNPNInterpose.cpp` and only sees calls that bind the
   `NPN_InvalidateRect` SYMBOL, while an ordinary NPAPI plugin calls through the NPNetscapeFuncs
   table, which `PluginModuleChild` fills with its own `_invalidaterect` (`:986`) that forwards
   straight to `InstCast(aNPP)->InvalidateRect` (`:1302-1310`) and never touches the interpose.
   It can read 0 forever while the plugin invalidates 22 times a second.
   **So the pacing defect is TWO PRODUCERS BEATING**, ours and Flash's, funnelled into one
   coalescing path (~49 invalidates/s collapsing to ~32 draws/s). That is exactly the shape that
   produces an unconcentrated gap histogram at a healthy average.
   **NEXT EXPERIMENT, and it is cheap:** measure Flash with BOTH drivers off. `JIHAD_PLUGIN_FRAME_MS=0`
   alone is not enough — it stops the pull, the lease expires and the child's own timer takes
   over, which is just `0026` again. It needs one more child-side kill switch to disable the
   timer outright. If Flash alone holds the authored rate, the host clock should be deleted for
   plugins that self-invalidate rather than tuned.
   Measure with the gap histogram, NOT with average fps: the average was 35-42 fps throughout the
   whole period the animation looked worst, and `0029` IMPROVED the average and the max while
   making the histogram flatter.
3. **The partial-paint path publishes with no blank check** (cavekit-offscreen-rendering.md R8,
   last criterion). Anything that corrupts a partial frame is invisible to the daemon.
4. **`plugin-container` aborts on teardown.** Every navigation away from a Flash page ends in
   `mozalloc_abort` (pc 0xc35c in plugin-container's own text) — a deliberate Gecko abort, not a
   segfault. Harmless today (the child is going away) but it is noise in every log.
5. **The in-daemon plugin scan is a landmine.** `nsPluginFile::GetPluginInfo` dlopens EVERY
   plugin in the daemon ("Sadly we have to load the library for this to work"), pulling
   libWebKitLuna/libPiranha/libv8/libEGL into it, and `dlclose` does not unload them. A hostile
   plugin can wedge the daemon before any plugin logging runs.

---

## Traps — every one cost real time and none is discoverable from the source

**Device / measurement**
- **AVERAGE FRAME RATE CANNOT SEE THE DEFECT THE USER IS REPORTING.** Throughout the entire
  period the animation looked worst by eye, the daemon reported 35-42 fps for a 30 fps source —
  ABOVE the content rate. min/avg/max cannot separate "evenly late" from "unevenly on time"
  either. Read the `gap hist ms` bucket line: flat across the buckets means no pacing, weight in
  one bucket means smooth. Every earlier session's frame-rate conclusions were drawn from summary
  statistics and are worth re-reading with that in mind.
- **THE PLUGIN'S DRAW RATE IS NOT SET BY WHOEVER YOU THINK DRIVES IT — count, do not reason.**
  Three sessions have now attributed the draw rate to a specific clock (the child's timer, then
  the daemon's tick, then the host pull) and every attribution was made from reading the code, not
  from a count. When `0029` finally put a counter on the request side the numbers did not agree
  with ANY of them: 3988 draws against 3072 requests, with the child's own timer provably stood
  down and Flash provably not self-invalidating. Whenever you change what drives a frame, log both
  ends and divide. The ratio is the finding; the fps number cannot express it.
  **And the follow-up is the sharper lesson: "provably stood down" was itself an inference, and it
  was wrong.** The child's timer was gated on a lease, one `frame clock -> HOST` line was taken as
  proof the gate held, and nobody checked for a LATER transition. There were three. Tagging every
  call site closed the accounting to the unit — 1856 host + 1846 timer = 3702 — and the answer was
  a bug in the gate, not a third party. A transition log only proves what it logged; grep for
  ALL of them, and make counters that must SUM to something you already measure.
- **A WEDGED PLUGIN LOOKS EXACTLY LIKE A PACING REGRESSION.** Cycling `inject url` between pages
  repeatedly can leave the child in a crash loop; the daemon then logs `flashRect remove id=N
  (gone)`, painting stops entirely, and the child keeps logging `palm draw` the whole time so the
  plugin looks alive. This produced a confident, WRONG attribution to a tick-rate change on
  2026-08-10 — the identical build measured healthy after a clean restart. Before believing any
  frame number that follows repeated navigation: `stop jihad; pkill -f jihad-browserserver;
  pkill -f plugin-container; start jihad; killall LunaSysMgr`.
- **AN EMPTY LOG PASSES EVERY CHECK.** `grep -c 'inject: no page' $LOG` returns 0 on a log that
  is empty because the daemon wrote NOTHING, which reads as "card fine" and is the opposite of
  the truth. This produced two clean-looking but entirely void runs on 2026-08-10. Assert the log
  GREW (`wc -c`) before believing any grep over it — a zero-byte log means the run never
  happened, not that it went well. Same class as the "card connected" trap below, one level down.
- **NEVER write `/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor`.** Switching away from
  Palm's `ondemandtcl` deadlocks the cpufreq policy lock in the kernel; every later reader of any
  cpufreq node blocks in unkillable **D state** — the daemon, `powerlog`, and your own restore
  script, so the governor is never put back and the daemon stops painting. Only a reboot clears
  it, and **a clean `reboot` will NOT run**, because init waits on those D-state tasks: it takes
  `sync; sync; reboot -f`. The governor's own TUNABLES (`cpufreq/ondemandtcl/*`) are safe to
  write and return instantly — that is what patch `0027` uses. Cost a device reboot 2026-08-10.
- **Regenerating `/etc/event.d/<job>` from `packaging/gen-variant-scripts.sh` and pushing it
  verbatim KILLS THE INJECT CHANNEL.** The shipping template does not set `JIHAD_INJECT=1`; the
  device's copy carries it as a local addition. Lose it and every inject silently does nothing,
  the daemon logs nothing, and — see the first trap — every subsequent check "passes". Re-add
  `JIHAD_INJECT=1` to the `exec env` line whenever you reinstall the job, and remember the running
  daemon keeps its OLD environment until you `stop`/`start` it.
- **ASSERT THE CARD IS CONNECTED before believing any measurement.** If it disconnected, the
  daemon logs `inject: no page` and every downstream number — container present? draw count?
  thread count? — reads as a dramatic failure that is really "no page". This invalidated three
  of my own measurements and twice produced a wrong "audio kills the plugin" conclusion. Inject
  a known page and grep for `inject: no page` first.
- `novacom run` drops output arriving after host stdin hits EOF — pipe `sleep N |` into every
  run whose reply matters.
- `novacom run file://bin/sh -- -c '…'` silently drops the `-c` string. Stage a script with
  `novacom put` and run that.
- There is **no `timeout` binary** on the device. Background the process and `kill` it.
- `strace` DOES exist (`/usr/bin/strace`) and settled two problems in one run each.
- A screenshot captures the FOREGROUND card. The double-launch dance creates a second card, so a
  screenshot can silently be of the wrong one.
- The first card launch after `killall LunaSysMgr` does not navigate — launch twice, or use the
  inject channel (`JIHAD_INJECT=1`, `<state>/inject.cmd`), which is better.
- `/tmp` is cleared by a reboot; `/etc` comes back read-only (`mount -o remount,rw /`).

- **NEVER regenerate the YAP interface from `../ref-BrowserServer/CodeGen` — it would break every
  keystroke.** Established 2026-08-10 by building `YapCodeGen.cpp` against Qt 5.15 and RUNNING it,
  not by reading. The upstream `.defs` is stale against its **own** upstream output: it declares
  `KeyDown`/`KeyUp` (0x1008/0x1009) as two `uint16_t`, while both upstream's generated
  `BrowserServerBase.cpp` and ours read a third `int32 chr` — upstream hand-edited its generated
  file and never updated the `.defs` (the tell is the six-space indent on those lines inside a
  tab-indented generated file). Regenerating from it silently rewrites two LIVE commands from 12
  wire bytes to 4. Note this is a bigger break than the missing 0x1600, and the obvious fix —
  "just add our command to the upstream `.defs`" — would have shipped it.
  **Use the corrected in-tree copy: `render/browserserver/CodeGen/BrowserYapCommandMessages.defs`.**
  `build/webos-oe/check-yap-contract.sh` now runs inside `build-daemon-arm.sh` and fails the build
  on any drift (verified three ways: passes here, fails with 10 named defects on a real upstream
  regeneration, fails with exactly one on a regeneration from the corrected `.defs` — that one
  being the hand edit no `.defs` can express, since the generator emits every async command as
  `= 0;` and 0x1600 needs an empty default body to stay optional).
  **TRAP INSIDE THE TRAP:** a `.defs` line beginning with a SPACE hangs `YapCodeGen` forever —
  `ignoreLine()` scans for a non-space without advancing its pointer.

**Cross-ABI (bundled glibc 2.23 vs device glibc 2.8)** — see cavekit-device-build.md **R9**
- glibc 2.21 changed `sem_t`'s value word to `(tokens << 1) | has-waiters`. A named semaphore
  created by a 2.8 process reads to us as "0 tokens, waiters pending" and blocks FOREVER. This
  took Flash out entirely after one reboot and looked exactly like a code regression on
  untouched binaries — **it is a per-boot race, not a bug in your change.** Contained for
  `/PmLogLib` by `render/goanna/JihadPmLogSem.c` (patch 0025).
- `sem.browserserver.*` are created by the ADAPTER and nothing ever waits on them. The trap is
  ARMED, not sprung: adding a daemon-side `sem_wait` springs it, and `nwaiters` sits at a
  different offset too (8 in the 2.8 family, 12 in 2.23 — measured). Use SysV `semop` (a kernel
  object, no userspace layout) if a blocking handshake is ever needed. It does NOT fire on an
  EMPTY semaphore (raw 0 and `0 << 1 | 0` are the same word), so a first `sem_wait` would look
  fine right up until the adapter posts.
- **THIS WHOLE CLASS IS HOST-TESTABLE — stop spending device sessions on it.** Both toolchains are
  in the tree and `qemu-arm` runs their output on the build host: static-link one probe with the
  PDK gcc (sysroot glibc 2.5, the pre-2.21 side) and one with the crosstool gcc (2.23), run both,
  and the DIFFERENCE is the ABI. The `sem_t` hang above reproduces this way in about a minute.
  The PDK sysroot is 2.5 and the device is 2.8, so it is a pre-2.21 PROXY, not the device.
- **`pthread_mutex_t` PROCESS_SHARED is a SECOND armed trap, and the layouts being IDENTICAL is
  what makes it dangerous.** 24 bytes and the same fields in both, so every size and marker check
  passes — but glibc 2.23 sets `__kind = 128` (PTHREAD_MUTEX_PSHARED_BIT) and takes the futex
  private/shared flag from that bit, while the pre-2.21 side writes `__kind = 0` and returns
  EINVAL on a mutex whose `__kind` is 128. So one direction hangs under CONTENTION ONLY, and the
  other turns every `lock()` into a silent no-op. `ProcessMutex` is adapter-only and instantiated
  by nothing today; see cavekit-device-build.md R9 and the comment in `ProcessMutex.cpp`.
- `pthread_cond_t` still matches (48 bytes, same fields) — but **2.23 is the LAST glibc where that
  is true**; the condvar was rewritten in 2.25, and 2.24/2.25 tarballs are already in the
  toolchain source dir. `pthread_rwlock_t` ALREADY differs at 2.23 (same size, `__flags` narrowed
  and a new `__shared` byte added), which is the standing proof that `sizeof` does not clear a
  struct.
- The framebuffer header IS clean: `BrowserOffscreenInfo` is 32 bytes with identical offsets under
  both toolchains (plain int/double, so AAPCS fixes it and libc has no say). Do not re-check it.
- Prove each device library with a standalone probe run under BOTH loaders — the difference IS
  the diagnosis. See `plugin_mime_probe.c` and `alsa_probe.c`.

**Reverse-engineering libflashplayer.so**
- The code is **ARM, not Thumb**.
- String addresses are PIC: `add rX, r5, <negative literal>` with a base near 0x8ad010. Searching
  for an absolute address AND for `movw`/`movt` immediates both find ZERO references. Read the
  literal pool and add the base.
- Take PLT stub addresses from `objdump -d --section=.plt`, which labels them. A hand-rolled
  decode gave a wrong stub and made a BL scan report no callers.
- A function's entry can be one instruction BEFORE the `push` (the PIC `ldr rX,[pc,…]` comes
  first). Being off by 4 made a caller scan report zero callers and nearly sent the audio hunt
  down a "it must be a vtable" dead end.

**Build / deploy**
- `push-engine-update.sh` pushes libxul + daemon + goanna.js. It does **NOT** push
  `plugin-container` or the adapter — **nor the loose JS components.**
  `components/jihadInstallPrompt.js` and `components/jihadAboutPreferences.js` live in each
  variant's `deviceroot/hl/components/` and are refreshed only by a bundle/`.ipk` push, or by
  pushing the one file per variant (which is what was done 2026-08-10 for the R3 install-failure
  observer). Editing one and running `push-engine-update.sh` ships nothing.
- **The variant app ids on device use a HYPHEN, not a dot: `net.riverstonerelay.jihad-browser-mochi`
  and `…-mojo`.** `CLAUDE.md` writes the Mochi id with a dot and is WRONG against the device.
  Pushing to the dotted path fails with `bad or error response from other side: 'file open failed'`,
  and the `ls` that follows reads as "this variant does not ship that file" — I briefly concluded
  mochi and mojo had no `components/` directory at all before checking the parent directory.
  Confirm a path against `ls /media/cryptofs/apps/usr/palm/applications/` before believing an absence.
- The `.ipk` ships the **PDK** adapter (`build-adapter-pdk.sh` → `adapter-deps/build-pdk/`), NOT
  the crosstool one (`build-adapter-arm.sh` → `adapter-deps/build/`). Rebuilding only the latter
  ships an old adapter in the package while the device runs the new one.
- The adapter runs INSIDE LunaSysMgr. Deploy it by swapping
  `/usr/lib/jihad/<variant>/BrowserAdapterImpl.so` + `/usr/lib/BrowserPlugins/BrowserAdapterJihad*.so`
  (needs `mount -o remount,rw /`), then restart LunaSysMgr. Back up first —
  `/var/palm/jihad/adapter-backup/` holds the pre-2026-08-10 pair.
- A hand-launched daemon survives `stop jihad` and serves a stale libxul. "Nothing changed" is a
  lie until you `pkill -f jihad-browserserver`.
- `cp` onto a mapped binary fails with "Bad file descriptor" on cryptofs — stop the daemon and
  `rm -f` first, or push to `.new` and `mv`.

---

## Build / deploy loop

```bash
# libxul (incremental ~45 s) and the daemon
podman run --rm --userns=keep-id \
  -v "$PWD/third_party/uxp:/src/uxp" -v "$PWD:/jihad" \
  -v "$PWD/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi:/tc" \
  -v "$PWD/build/webos-oe/arm-sysroot/root:/sysroot" \
  -v "$PWD/build/desktop:/cfg" -v "$PWD/build/webos-oe:/armcfg" \
  -v "$PWD/build/webos-oe/out-arm:/out" \
  localhost/jihad-goanna-build bash /armcfg/build-goanna-arm.sh build   # or build-daemon-arm.sh

build/webos-oe/push-engine-update.sh enyo mochi mojo   # md5-verified

bash build/webos-oe/build-adapter-arm.sh    # adapter for a live swap
bash build/webos-oe/build-adapter-pdk.sh    # adapter the .ipk ships  <-- do both
bash build/webos-oe/build-variant-ipk.sh    # all three .ipks

bash build/webos-oe/build-test-plugin-arm.sh   # NPAPI control plugin
bash build/webos-oe/build-uinput-kbd.sh        # synthetic keyboard (PDK; needs --sysroot)
```

**Only `enyo` has libxul with patch `0027`.** `push-engine-update.sh enyo mochi mojo` pushes the
SAME libxul to all three, so just run it with all three variants next time you push and they are
back in step. Until then, a frame-rate measurement taken on mochi or mojo will show the OLD,
governor-limited numbers and look like a regression that is not one.

Clean device run:

```sh
stop jihad; sleep 2; pkill -f jihad-browserserver; pkill -f plugin-container; sleep 2
: > /var/palm/jihad/enyo/daemon.log
start jihad; sleep 3
killall LunaSysMgr; sleep 32
luna-send -n 1 palm://com.palm.applicationManager/launch \
  '{"id":"net.riverstonerelay.jihad-browser","params":{"target":"file:///tmp/jihad-plain.html"}}'
# then drive it with the inject channel:
echo 'url file:///tmp/jihad-anim.html' > /var/palm/jihad/enyo/inject.cmd
```

Read the `paint pipeline:` / `frame gap ms min/avg/max` counter pair for frame-rate work — the
gap SPREAD is what reads as stutter, not the average.

### Inject commands, and the two added 2026-08-10

`url`, `text`, `find`, `rect`, `gettext`, `clickid`, `dblclickid`, `clickoff`, `touch`, `cookie`,
`addon`, `spotlight`, `freeze`, `thaw`, `popups`, `jsurl`, and now:

| command | what it is for |
|---|---|
| `setpref <b\|i\|s> <name> <value>` | write a pref, with an immediate read-back on the same log line |
| `getpref <b\|i\|s> <name>` | read one back — after a restart, which is the half that proves persistence |
| `anon <selector>` | the element's NATIVE ANONYMOUS CONTENT: did an XBL binding actually attach. `?` = no such element, `0` = element exists and bound NOTHING (the real negative), otherwise a count plus the first few node names and `anonid`s |

**Both were added because their absence had already cost device runs, and both replace a probe
route that DOES NOT WORK — do not go back to it.** `jsurl` executes with the system PRINCIPAL but
in a CONTENT scope, so `Components` is not exposed: a `Components.classes[…preferences-service…]`
probe returns success and silently does nothing, which reads as a negative result about the thing
under test rather than a broken probe. Likewise `document.getAnonymousNodes` throws
`not a function` there regardless of privilege. Both questions have to be answered in the daemon,
which is in-process with libxul and already holds `nsIPrefBranch` and `nsIDOMDocumentXBL`.

---

## Test assets (`render/goanna/test/`)

| asset | what it answers |
|---|---|
| `make-anim-swf.py` → `jihad-anim.swf` | does content MOVE (x-centroid = frame number) |
| `make-audio-swf.py` → `jihad-audio*.swf` | audio; PCM by default, `--mp3`, `--stereo` |
| `make-key-swf.py` → `jihad-key.swf` | real AVM1: GREEN until a key is handled, then RED |
| `alsa_probe.c` | can THIS runtime play sound (run under both loaders) |
| `plugin_mime_probe.c` | reproduce the daemon's plugin scan standalone |
| `audio_shim.c` | LD_PRELOAD: trace dlopen/dlsym, substitute `snd_pcm_open` |
| `npapi_test_plugin.c` | control plugin — proves defects are OURS, not Flash's |
| `uinput_kbd.c` | synthesises a HARDWARE keyboard (this device has none) for chrome-side key tests |
| `abi_probe.c`, `sem_dump.c`, `mtx_dump.c` | **cross-ABI, and they need NO DEVICE.** Build each with both in-tree toolchains (PDK gcc 4.3.3 / glibc 2.5 and the crosstool 9.4 / glibc 2.23), run both under `qemu-arm`, and the diff IS the ABI. Reproduced the PmLogLib `sem_t` hang on the build host in about a minute. See the cross-ABI section at the end of this file for the 2.5-is-a-proxy caveat |

Pages/assets staged in `/tmp` on the device (volatile — re-push after any reboot). The HTML
wrappers are trivial and are NOT in the repo; regenerate them as a `<embed
type="application/x-shockwave-flash" src="<swf>" wmode="opaque" width="320" height="240">`:

| on-device page | asset | what it is for |
|---|---|---|
| `jihad-plain.html` | none | the SWF-FREE page. Navigate here first when you need a plugin-container to spawn FRESH on the next navigate, and to release the `0027` CPU boost |
| `jihad-anim.html` | `jihad-anim.swf` | frame-rate work |
| `jihad-audio.html` | `jihad-audio.swf` | PCM square — the only asset confirmed CLEAN by ear |
| `jihad-audio-mp3.html` / `-44k.html` | mono 22 k / stereo 44.1 k MP3 | MP3 decode path |
| `jihad-audio-5s.html` | `jihad-audio-5s.swf` | 5 s sine MP3. LOOPED, so it clicks once per iteration by design — that is Flash restarting its decoder, not a defect |
| `jihad-audio-long.html` | `jihad-audio-long.swf` | **30 s single-shot MP3 (`--loops 1`). The ONLY asset that can answer a question about audio QUALITY**, because it has no loop point at all. Every other MP3 asset here clicks per iteration no matter how healthy the port is |
| `jihad-keyarb.html` | `jihad-key.swf` + 3000 px scroll region + `#sy` readout | keyboard arbitration: read scroll back with `gettext #sy` |
| `jihad-media.html` | `jt.webm` (VP8) + `jt.mp3` | **ENGINE media, nothing to do with Flash.** Reports `canPlayType`/`readyState`/`currentTime`/`error` into `#r`; read it back with `gettext #r`. Measured 2026-08-10: **VP8 plays**, H.264 unsupported, **MP3 refused outright** (`error.code=4`) because the ARM build compiles cubeb with NO backend. Regenerate the two media files with `ffmpeg -f lavfi -i sine=…` and `-f lavfi -i testsrc=… -c:v libvpx` |

SWF authoring traps — **every one of these has already produced a false negative in this tree,
and a bad asset is indistinguishable from a broken port:**
- **Shapes and sounds share ONE character dictionary.** `DefineSound` may not reuse a
  `DefineShape` ID. The player keeps the first definition and drops the second, `StartSound`
  resolves to the wrong character, and the sound is silently never played. This cost a whole
  session chasing a nonexistent player-side audio gate. Fixed 2026-08-10 (`SOUND_ID = 2`).
- `0x00020000` is ClipEvent**KeyPress** (and carries a trailing KeyCode byte) — KeyDown is
  `0x00000040`; getting it wrong makes the SWF draw NOTHING rather than ignore keys.
- A sprite with no `Stop` on frame 1 loops, which looks exactly like a key response and is not.
- For MP3 event sounds the SoundData begins with a **SeekSamples SI16**, not the frame stream.

Parse an asset back out before believing a negative result from it. A 30-line tag walk that
prints each tag code and character ID is enough, and it is what caught the ID collision.

---

## Device state as left

- **2026-08-10, later session:** the enyo daemon and libxul now also carry the frame-pacing work
  (`PumpReady`, the 1:1 seq gate, patch `0028`, the tick/gap counters). `/etc/event.d/jihad`
  carries `JIHAD_TICK_MS=16` and `JIHAD_PLUGIN_FRAME_MS=33` as device-local additions ALONGSIDE
  `JIHAD_INJECT=1` — all three are lost if the job is regenerated from the template. The device
  was last left on a CLEAN restart running `/tmp/jihad-anim.html`, measured healthy: `done=61
  palm draw=63 deferred=0`, gap mode 32-39 ms.
- **2026-08-10, pull session: ALL THREE VARIANTS ARE IN STEP** and were re-pushed together at the
  end of the session, so every one carries the same libxul and daemon with patches `0027`, `0028`
  AND `0029`. The previous enyo-ahead-of-mochi/mojo skew is gone, so a frame measurement on any
  variant is now comparable. (Read the current md5s off the device rather than trusting a number
  written here — this file has carried a stale one before. `push-engine-update.sh` prints them.)
- **The libxul on the device carries the pacing INSTRUMENTATION as well as the fix**, and it is
  what makes the open question answerable: the child logs `show req: setwin=… hostinval=…
  plugininval=… retry=… bgupd=… bgtail=… bgdestroy=…` next to `palm draw`, on the same 2 s
  boundary. `hostinval` is our pull, `plugininval` is the plugin invalidating itself. Those two
  counters are the whole of open work item 2 — read them before changing anything about pacing.
  Adapter is unchanged — still the PDK build matching the `.ipk` (md5
  `0410a348042af58a7e711b495db69833`).
- **THE DEVICE IS DELIBERATELY LEFT IN THE "FLASH ALONE" CONFIGURATION — this is not an accident
  and it is not the shipping default.** `/etc/event.d/jihad` carries `JIHAD_PLUGIN_PULL=0` and
  `JIHAD_PLUGIN_SELF_DRIVE=0`, so BOTH host frame drivers are off and the plugin's own
  `NPN_InvalidateRect` is the only frame source (verified: `hostinval` is exactly 0).
  **It is left this way so a human can LOOK at it**, because the eye is the only instrument that
  has ever detected this defect: Flash alone measures SLOWER (24-27 vs 30-33 fps) but perfectly
  unbunched (**zero** gaps under 16 ms, against 1-5 with the pull on). If evenly-spaced 25 fps
  reads better than bunched 32 fps, the host frame clock should be deleted — see the R7 frame-rate
  criterion for the full three-way table and the recommendation.
  **Revert to the shipped default in one line** (then `stop jihad; start jihad`):
  ```sh
  mount -o remount,rw / && sed -i 's/ JIHAD_PLUGIN_PULL=0 JIHAD_PLUGIN_SELF_DRIVE=0//' /etc/event.d/jihad
  ```
- `/etc/event.d/jihad` is back at **`JIHAD_TICK_MS=16`**. It was moved to 8 for one A/B during this
  session and moved back; 8 measured WORSE (see the pull section). The edit was a `sed` on that one
  token, never a regeneration from the template, so `JIHAD_INJECT=1` survived — verified after each
  edit. If you A/B it again, do it the same way and re-check `JIHAD_INJECT=1` afterwards.
- `/etc/event.d/jihad` is the normal job (`JIHAD_INJECT=1`, no `LD_PRELOAD`, no kill switches) and
  now also carries the `post-stop` governor restore. **TRAP: the SHIPPING template in
  `packaging/gen-variant-scripts.sh` does NOT set `JIHAD_INJECT=1` — that is a device-local
  addition.** Regenerating the job and pushing it verbatim silently kills the inject channel, and
  every subsequent test then "passes" against an empty log. Re-add it when you reinstall the job.
- db8 kind `com.palm.app.flashplayer.prefs:1` registered (owner `com.palm.flashgraphics`) — this
  is a deliberate addition that fixes Flash's only reported error. Remove with `delKind` if
  unwanted.
- `/var/palm/jihad/adapter-backup/` holds the pre-2026-08-10 adapter pair.
- Flash's `oem.cfg` was temporarily modified and has been RESTORED to original.
- **The device was REBOOTED on 2026-08-10** (forced: `sync; sync; reboot -f`, because a clean
  `reboot` hangs on D-state tasks — see the cpufreq trap). It came back healthy on `ondemandtcl`
  and all three daemons auto-started. Governor tunables are at STOCK (`up_threshold=95`,
  `sampling_rate=200000`); the daemon takes them to 40/50000 only while a plugin is instantiated.
- Test pages/SWFs re-pushed to `/tmp` after that reboot and ALL CURRENT — `jihad-anim`,
  `jihad-audio`, `jihad-audio-mp3`, `jihad-audio-44k`, `jihad-audio-5s`, `jihad-audio-sine`,
  `jihad-key`, plus `jihad-plain.html` and `jihad-keyarb.html`. `/tmp` is volatile: re-push after
  any reboot (see the Test assets table for what each one is for).
- The synthetic keyboard is NOT running — it was destroyed (`echo quit > /tmp/uinput.cmd`) and the
  process is gone. `/tmp/uinput_kbd` may or may not still be there; rebuild with
  `build/webos-oe/build-uinput-kbd.sh`.
- `libflashplayer.so` in the enyo variant's `profile/plugins/` is **stock**, md5
  `14c20ba9fc2183a93648c70488e0ec8e`, byte-identical to
  `/media/cryptofs/apps/usr/lib/BrowserServerPlugins/libflashplayer.so`, which is the restore
  source. Two patched builds were run there during the audio hunt and both have been removed.

---

## Uncommitted work in the tree (2026-08-10)

Nothing is committed. What this session added, and why, so it is not mistaken for cruft:

| path | what it is |
|---|---|
| `build/desktop/patches/0027-npapi-plugin-cpu-boost.patch` | **the frame-rate fix.** Holds a CPU-governor boost for the life of a plugin instance. Applied to `third_party/uxp` already, and the enyo device libxul is built from it |
| `packaging/gen-variant-scripts.sh` | adds the `post-stop` governor restore to the upstart template, so a crashed daemon cannot strand the tuning |
| `render/goanna/test/uinput_kbd.c` + `build/webos-oe/build-uinput-kbd.sh` | the synthetic hardware keyboard, for the open keyboard-arbitration criterion |
| `render/goanna/test/make-audio-swf.py` | two real asset bugs fixed: duplicate character ID, and `SeekSamples` 0 → 1105 |
| `render/goanna/test/audio_shim.c` | header rewritten — its old "measured" conclusion was wrong |
| `docs/PICKUP.md`, `context/kits/cavekit-addons-extensions.md` | this handoff and R7 |
| `build/desktop/patches/0028-npapi-plugin-frame-seq.patch` | **the 1:1 frame fix.** Exports `jihad_plugin_frame_seq()` from `PluginInstanceParent::RecvShow` so the daemon can tell a NEW plugin frame from a repeat. Applied to `third_party/uxp` already; the enyo device libxul is built from it. Dry-run verified against a reconstructed pristine copy, since several other patches also touch that file |
| `render/goanna/GoannaRenderPage.{h,cpp}` | `PumpReady()` (drain-and-return, the tick's pump) beside `PumpFor()` (spend-the-budget, what the tests need); weak `jihad_plugin_frame_seq` binding + `PluginFrameSeq()` |
| `render/goanna/BrowserPageGoanna.{h,cpp}` | the publish policy: 1:1 on the frame seq with a stale-damage fallback, the deadline pacer as the no-seq fallback, and the frame-gap HISTOGRAM that made the defect visible at all |
| `render/browserserver/JihadBrowserServer.cpp` | tick cadence counters (period vs work, split pump/paint) on a monotonic clock; the tick pumps with `drainOnly` |
| `render/browserserver/Main.cpp` | tick period is `JIHAD_TICK_MS` (4-64), so cadence can be A/B'd without a rebuild |
| `render/goanna/test/dump-swf.py` | walks a SWF back out and FAILS it on the three asset bugs that have each impersonated a port defect here (duplicate character ID, SeekSamples, SyncNoMultiple retrigger) |
| `render/goanna/test/make-audio-assets.sh` | regenerates every audio asset from scratch and verifies each one. The MP3 streams behind the checked-in SWFs previously existed only in `/tmp`, so the repo could not reproduce its own test assets |
| `build/webos-oe/stage-test-pages.sh` | pushes the SWFs + generates the HTML wrappers into the device `/tmp`, md5-verified. Replaces hand-making them after every reboot |
| `README.md`, `docs/*.md` | **de-duplicated against the kits** (user instruction 2026-08-10). The kits own requirements and per-criterion STATUS; docs own procedure. 233 lines of restated status removed, and with them three claims that had already gone stale and wrong: "Flash audio does not work", DEVICE-BUILD's "no device sysroot, no OE tree … and no TouchPad", and TOOLCHAIN's three-way toolchain choice presented as still open. README is now user-facing and positions against isis and Atlas |
| `context/plans/build-site.md` + `plan-overview.md` | regenerated from the kits: **56 tasks, 6 tiers, covering the 41 still-open criteria at 100%**, with the 8 hardware/human-gated ones quarantined in a final tier. The 2026-06-30 site is kept as `build-site-2026-06-30.archived.md` |
| `build/desktop/patches/0029-npapi-plugin-pull-frame.patch` | **the PULL.** New async parent→child `JihadRequestFrame`; the child answers with `JihadPalmRepaintTick()` and stands its own timer down on a 150 ms lease, logging every transition; `PluginInstanceParent` exports `jihad_plugin_request_frame()` over a file-static live-instance list. Applied to `third_party/uxp` already and live on all three variants. **Its own header records that it does NOT fix the pacing and why** — read it before building on it. Verified by reconstructing the pre-0029 baseline (reverse-applying only this session's edits), dry-running the patch onto it, and confirming it reproduces the working tree byte-for-byte |
| `render/goanna/BrowserPageGoanna.{h,cpp}` (pull) | `requestNextPluginFrame()`: the whole-tick request grid, the one-outstanding backpressure gate with a 250 ms timeout, and `jihadTickPeriodMs()` — the single definition of the tick period, which `Main.cpp` now calls instead of reading the env itself. Deliberately NOT gated on `mEmittedFlashRects`: that map is what EMPTIES when the plugin wedges, so gating there turns a recoverable wedge into a permanent one |
| `render/browserserver/JihadBrowserServer.cpp` (pull) | calls `requestNextPluginFrame()` once per tick — NOT from `maybePaint()`, which `returnBuffer()` also reaches off-tick, which would put requests off the grid they exist to stay on |
| `build/desktop/patches/make-0029.sh` + `.baseline-pre-0029/` | **the fix for the patch-authoring trap this file has warned about three times.** Reconstructs the correct baseline — `git show HEAD:` plus every EARLIER patch that touches the same four files (`0008 0017 0020 0022 0024 0026 0028`) — and diffs the live tree against it, so a new patch cannot silently fold in its predecessors or come out empty. `--rebuild-baseline` regenerates the dir. Generalise it before authoring `0030`; the file list and the `EARLIER` list are the only per-patch parts |

`third_party/uxp/dom/plugins/ipc/PluginModuleParent.cpp` is modified in place (that is how patches
work here — the build applies them to the shared tree, idempotently, with a dry-run gate). If you
regenerate `0027`, diff against a pristine copy of that file, not against the working tree.

---

## If you are picking this up cold, do this first

1. Read the Flash status table above, then the two open items at the top of **Open work**.
2. `novacom -l` — confirm `topaz-linux`. Re-push `/tmp` assets if the device rebooted.
3. The cheapest real progress available is **counting the third draw driver** (open work item 2):
   two 2 s counters, one on `PluginInstanceParent::SendUpdateBackground` and one on each
   `AsyncShowPluginFrame` call site, then one clean-restart run. Every pacing conclusion in this
   document rests on not knowing that number.
   **Audio is CLOSED — do not start there.** An earlier revision of this list sent you to an
   "MP3 static" question that the audio section above had already resolved at parity with stock;
   the contradiction cost a session start. If two parts of this file disagree, the dated
   subsection wins over this list.
4. Keyboard arbitration is NOT blocked on the browser — it is blocked on getting a key out of
   `hidd` into the card. Start at `/usr/lib/libhidinputdev.so`, not at the adapter.
5. Whatever you measure: assert the daemon log GREW, and assert the card is connected, BEFORE
   believing any number. Both failure modes look exactly like success.

---

## The cross-ABI class is now HOST-TESTABLE — do not wait for the device (2026-08-10)

`render/goanna/test/{abi_probe,sem_dump,mtx_dump}.c`, promoted out of a scratch dir because this
is the single most reusable thing found this session. Both toolchains are already in the tree and
`qemu-arm` runs their output on the build host:

- `build/webos-oe/pdk/opt/PalmPDK/arm-gcc` — gcc 4.3.3, sysroot glibc **2.5**
- `build/webos-oe/toolchain/.../arm-webos-linux-gnueabi` — gcc 9.4, glibc **2.23**

Static-link the same probe with each, run both under `qemu-arm`, and **the diff IS the ABI.**
This reproduced the PmLogLib `sem_t` hang off-device in about a minute — a bug that previously
cost a device session to find.

**CAVEAT, stated because it matters: the PDK sysroot is glibc 2.5 and the device is 2.8.** Both
are pre-2.21, so it is the right side of every cliff that matters here, but 2.5 is a PROXY and a
result that hinges on 2.5-vs-2.8 rather than pre-vs-post-2.21 is not proven by this method.
