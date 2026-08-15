# Engine audio output — building cubeb's ALSA backend

Owner requirement: `../kits/cavekit-gre-widgets.md` R1 (last criterion) and R7 (first criterion).
The pref-level analysis those criteria carry is **not** repeated here; this file is about the
build flag and the runtime library, which is where the fix actually lives.

---

## 2026-08-15 — AUDIO-BACKEND: the backend is BUILT. It has NOT been heard.

Read that heading literally. Everything below is a build result plus source reading. **No sound
has been produced by the engine on hardware**, because no device was connected this session. The
honest status is "the output path exists in the binary for the first time", not "audio works".

### What was wrong

`media/libcubeb/src` compiled exactly two objects — `cubeb.o` and `cubeb_panner.o` — with **no
backend**. `cubeb_init`'s dispatch table was therefore empty (`media/libcubeb/src/cubeb.c:114`)
and `AudioStream::Init` returned `CUBEB_INITIALIZATION_ERR`
(`dom/media/AudioStream.cpp:337-341`). Every decoded sample went nowhere. This was an **output**
defect, not a decoder defect, and the two must not be conflated — see "What this does NOT fix".

The cause was one line: `build/webos-oe/mozconfig.goanna-arm` passed `--disable-alsa`. Its stated
reason, *"Headless render daemon: no audio output needed (offscreen render only)"*, was stale and
also wrong on its own terms — offscreen rendering concerns the video surface and says nothing
about sound, and this daemon is a browser engine expected to play media.

### The mechanism, so the flag is not mistaken for a magic word

- `MOZ_ALSA` is **default-ON for Linux** (`third_party/uxp/old-configure.in:2903-2906`);
  `--disable-alsa` unset it. Merely deleting the line would have been enough, but the mozconfig
  now passes `--enable-alsa` **explicitly** so the intent is readable and an upstream default
  change cannot silently take it away.
- `MOZ_ALSA` gates `cubeb_alsa.c` into the build and defines `USE_ALSA`
  (`media/libcubeb/src/moz.build:15-19`).
- `MOZ_ALSA_LIBS` is added to `OS_LIBS` for libxul (`toolkit/library/moz.build:198-199`).
- `cubeb_alsa.c` contains **no `dlopen` and no `dlsym`** — it calls `snd_pcm_open` and friends
  directly. So libxul gains a **hard `DT_NEEDED` on `libasound.so.2`**. That single fact drives
  the whole runtime section below: an unresolvable `libasound.so.2` no longer means "no sound",
  it means **the engine does not load at all**.
- Configure needs `alsa.pc`, `alsa/asoundlib.h` and a linkable `libasound.so` in the sysroot —
  `PKG_CHECK_MODULES(MOZ_ALSA, alsa, ...)` hard-`AC_MSG_ERROR`s otherwise.

### Sysroot staging

Fetched from `archive.debian.org` into `build/webos-oe/arm-sysroot/debs/` and unpacked into
`arm-sysroot/root/` the same way the GTK/cairo/pango libs were (`ar p <deb> data.tar.xz | tar -xJ`,
i.e. `dpkg -x` without maintainer scripts — the host has no `dpkg`):

- `libasound2_1.0.28-1_armel.deb`
- `libasound2-dev_1.0.28-1_armel.deb`

Debian **Jessie**, architecture **armel**, matching every other deb in that directory. Verified
rather than assumed: `readelf -h` on the staged `libasound.so.2.0.0` reports
`ELF32 / ARM / Version5 EABI, soft-float ABI` — the softfp target this port builds for.

Checked deliberately, because the mozconfig warns about it: the `-dev` deb adds **only**
`usr/include/alsa/*` and `usr/include/sys/asoundlib.h`. It did **not** reintroduce glibc headers
into `/sysroot/usr/include` (`stdio.h`, `time.h` still absent), so the toolchain's own glibc 2.23
headers still win for libc — which is the invariant the `-I/sysroot/usr/include` in `CFLAGS`
depends on.

`arm-sysroot/` is `.gitignore`d (`.gitignore:17`) and `*.deb` is too (`:36`), so this staging
adds nothing to the repo. **Anyone rebuilding the sysroot from scratch must re-fetch these two
debs** — that is the one cost of the ignore rule, and it is why the exact filenames are written
above.

### Build evidence

Configure, from the build log:

```
checking for alsa... yes
checking MOZ_ALSA_CFLAGS... -I/sysroot/usr/include/alsa
checking MOZ_ALSA_LIBS... -L/sysroot/usr/lib/arm-linux-gnueabi -lasound
```

`out-arm/obj-jihad-goanna-arm/config/autoconf.mk` now carries `MOZ_ALSA = 1` (it carried nothing
before). The objdir and symbol evidence is in the section at the end of this file.

---

## The runtime library question — WHICH libasound the daemon binds

This is the part that decides whether any of the above produces a sound, and it is **not settled
by the build**. It is written out in full because the two candidates fail in opposite directions
and the wrong one is not obviously wrong from a log.

The daemon runs as `ld-2.23.so --library-path $HL <daemon>` — a bundled glibc **2.23** runtime.
There are two libasounds it could bind:

**(A) The bundled Jessie `libasound.so.2` (1.0.28).** `make-device-bundle.sh`'s transitive `.so`
closure searches `$SYS/usr/lib/arm-linux-gnueabi` (`:23`), so now that the sysroot carries
libasound, the walker **copies it into the bundle automatically**. No bundler edit was needed or
made.

**(B) The device's own `/usr/lib/libasound.so.2`** (webOS, glibc-2.8 era). Reached only if the
bundle does *not* carry one: `--library-path` is searched first, then the default paths.

### DECISION: ship (A), the bundled Jessie library.

Two reasons, in order of weight:

1. **The dependency is now hard, so startup is at stake.** A `DT_NEEDED` that does not resolve
   kills the process before `main`. Bundling makes resolution a property of our own bundle
   instead of a property of the device filesystem. Given this project has already bricked a
   TouchPad by making a boot-critical thing depend on an untested assumption, the option that
   cannot turn "no sound" into "no browser" wins.
2. **It is ABI-consistent with the rest of the port.** The staged library's highest referenced
   symbol version is `GLIBC_2.16` (`readelf -V`), satisfied by the bundled 2.23 — the same
   backward-compatibility argument the mozconfig header already makes for the Jessie
   GTK/cairo/pango libs. Option (B) is the reverse: a glibc-**2.8** library inside a 2.23
   process, which is precisely the mix that produced the `PmLogLib` `sem_t` deadlock.

It is also the zero-edit option, which matters for a different reason: `make-device-bundle.sh` is
owned by another task this wave, and the correct behaviour there is the *default* behaviour.

### THE DEPLOY TRAP — the two deploy paths now bind DIFFERENT libasounds

This is the sharpest practical consequence of the change and it is not visible from either script
in isolation. **`push-engine-update.sh` pushes exactly three files** — `libxul.so`,
`jihad-browserserver`, `goanna.js` (`:75-82`). It ships **no `.so` dependencies at all**. So:

| deploy path | what `libasound.so.2` resolves to |
|---|---|
| `make-device-bundle.sh` (full bundle / `.ipk`) | **(A)** the bundled Jessie 1.0.28, copied into `$HL` by the closure walk |
| `push-engine-update.sh` (the fast dev loop) | **(B)** the device's own `/usr/lib/libasound.so.2` — `$HL` has no libasound, so the loader falls through |

Two things follow, and both matter on the next device session:

1. **The fast dev loop is no longer sufficient for the first deploy of this change.** Pushing the
   new `libxul.so` into an `$HL` assembled before 2026-08-15 leaves a hard `DT_NEEDED` to satisfy
   from the device. It will *probably* resolve (the device has `/usr/lib/libasound.so.2`), but if
   it does not, **the daemon does not start** — and that failure will look like "the new libxul
   broke the browser", not like "a library is missing". Deploy this one with a full bundle, or
   push `libasound.so.2` into `$HL` alongside it.
2. **Anyone testing audio must know which path they used**, because the two give different
   libraries with different failure modes. A dev-loop push is *accidentally* configuration (B),
   which is the one `alsa_probe` measured as audible — so a fast-loop test could report working
   audio that a bundle-built `.ipk` then does not reproduce, or vice versa. Record the deploy path
   next to any audio result, or the result is uninterpretable.

### THE PREDICTED FAILURE, and it is specific

**If the device's ALSA `default` PCM routes through an external plugin, the bundled Jessie
library will not find that plugin, and `snd_pcm_open("default")` will fail.**

Evidence, all of it concrete:

- `cubeb_alsa.c:24` hardcodes `#define CUBEB_ALSA_PCM_NAME "default"`. There is no device-name
  pref or env override in the cubeb path.
- `strings` on the staged `libasound.so.2.0.0` shows its compiled-in plugin directory is
  **`/usr/lib/arm-linux-gnueabi/alsa-lib`** — a Debian multiarch path that **does not exist on
  the TouchPad**, which keeps its plugins in `/usr/lib/alsa-lib`.
- The device's `default` **does** appear to route through pulse: the Flash trace in
  `docs/PICKUP.md:66` shows `open("/usr/lib/alsa-lib/libasound_module_pcm_pulse.so")`, done by
  the *device's* libasound.
- There is **no `ALSA_PLUGIN_DIR` environment override** in this build of alsa-lib — the only
  such string present is `ALSA_CONFIG_PATH`. So the plugin directory cannot be redirected.

So the honest expectation for the first device run is: **it may well not make a sound on the
first try, and that would not mean the backend is wrong.** Distinguishing the two is the whole
point of writing this down.

### The two fallbacks, in the order to try them

**Fallback A — point `default` at plain hardware, no plugin needed.** `ALSA_CONFIG_PATH` *is*
honoured by the staged library. Set it (in `/etc/event.d/jihad`, alongside the existing
`LD_LIBRARY_PATH`) to a small config that defines `pcm.default` as `type hw; card 0`. This keeps
the bundled library and removes the plugin lookup entirely. Try this first — it changes one env
var and nothing binary.

**Fallback B — bind the device's libasound instead.** Delete `libasound.so.2` from `$HL` so the
loader falls through to `/usr/lib/libasound.so.2`. This is the configuration that is **already
measured to work**: `render/goanna/test/alsa_probe.c` `dlopen`s exactly that library and is
**audible under `./ld-2.23.so --library-path $HL`**, the daemon's own runtime. That measurement
is the strongest single piece of evidence in this whole file and it points at (B) — it is not the
default only because it re-introduces the glibc-2.8-in-a-2.23-process mix and makes engine
startup depend on a device file.

A note on why (B) is not simply "the proven option": `alsa_probe` used `dlopen` with
`RTLD_LOCAL` after the process was fully up. A `DT_NEEDED` binds the same library at load time
into the global namespace. The library is the same and its glibc surface is small, so the
difference is probably immaterial — but "probably immaterial" is exactly the phrasing that
preceded the PmLogLib session, so it is flagged rather than waved through.

One more thing worth knowing before debugging this on hardware: `cubeb_alsa.c` has its own
pulse-related workaround, `init_local_config_with_workaround(CUBEB_ALSA_PCM_NAME)` (`:721`),
which rewrites the local config when `default` resolves to pulse. It will run on this device. If
open behaviour looks strange, read that function before blaming the bundling decision.

---

## What this does NOT fix — do not let "audio works now" spread

The missing **decoders** are a separate, untouched gap. ffvpx is compiled with
`CONFIG_MP3_DECODER 0`, `CONFIG_AAC_DECODER 0` and `CONFIG_H264_DECODER 0`
(`third_party/uxp/media/ffvpx/config_components_audio_video.h:386-391`, `:330`, `:127`), and the
device carries no system `libavcodec` for `FFmpegRuntimeLinker` to fall back to. **No decoder
config was flipped and none should be as part of this work.**

So the backend helps exactly the formats that already decode and previously decoded into
nothing: `audio/ogg`, `audio/webm`, `audio/wav`, `audio/flac` — Vorbis/Opus/Wave/Theora via
`AgnosticDecoderModule`, plus ffvpx's FLAC and VP8/VP9. `audio/mpeg` stays **refused** at
`canPlayType`, and H.264 likewise; that refusal is a decoder fact and this change does not touch
it.

## What would close the kit criteria

One device run, and only one:

1. Confirm the daemon still **starts** — the new hard `DT_NEEDED` makes this a real check, not a
   formality, and it must be done before anything else.
2. Play `audio/ogg` or `audio/wav` from the existing `jihad-media.html` harness and **listen**.
3. If silent, walk the fallbacks above in order; the failure mode distinguishes them.

Until step 2 happens, `cavekit-gre-widgets.md` R1's last criterion and R7's first stay `[~]`.

---

## Objdir and symbol evidence

**Before** (every build of this port up to 2026-08-14) —
`out-arm/obj-jihad-goanna-arm/media/libcubeb/src/`:

```
Makefile  backend.mk  cubeb.o  cubeb_panner.o  libcubeb.a.desc
```

**After** the flag flip, same directory:

```
Makefile  backend.mk  cubeb.o  cubeb_alsa.o  cubeb_panner.o  libcubeb.a.desc
```

`cubeb_alsa.o` is 59 000 bytes. `cubeb.o` was **recompiled** as well, which is the part that
actually matters and is easy to miss: `USE_ALSA` changes the backend table inside `cubeb.c`, and
the object proves it took:

```
arm-webos-linux-gnueabi-nm cubeb.o | grep -i alsa
         U alsa_init
```

That undefined `alsa_init` is the dispatch-table entry whose absence was the whole defect —
`cubeb_init` now has a backend to call, where before the table was empty.

`cubeb_alsa.o` imports **31** `snd_*` symbols from libasound, e.g.:

```
arm-webos-linux-gnueabi-nm -u cubeb_alsa.o | grep snd_
         U snd_config            U snd_pcm_avail_update
         U snd_config_add        U snd_pcm_close
         U snd_lib_error_set_handler ...
```

All undefined, none `dlopen`ed — which is the evidence for the hard `DT_NEEDED` that the runtime
section above turns on.

### libxul link — the decisive evidence

The ARM `libxul.so` relinked cleanly against the sysroot libasound (1 170 201 276 bytes, unstripped,
2026-08-15 10:49; `dist/bin/libxul.so` symlink updated 10:50). `mach` reported
*"your build finally finished successfully"*.

**It now carries a `DT_NEEDED` on libasound**, which it never did before:

```
arm-webos-linux-gnueabi-readelf -d libxul.so | grep NEEDED
 ...
 0x00000001 (NEEDED)   Shared library: [libasound.so.2]
 ...
```

**And it imports all 31 ALSA entry points as versioned dynamic symbols** — proof the backend code
is really linked in and not dead-stripped:

```
arm-webos-linux-gnueabi-readelf --dyn-syms -W libxul.so | awk '$7=="UND" && $8 ~ /^snd_/'
snd_pcm_open@ALSA_0.9        snd_pcm_writei@ALSA_0.9       snd_pcm_close@ALSA_0.9
snd_pcm_set_params@ALSA_0.9  snd_pcm_avail_update@ALSA_0.9 snd_pcm_recover@ALSA_0.9
snd_pcm_open_lconf@ALSA_0.9  snd_pcm_state@ALSA_0.9        snd_pcm_pause@ALSA_0.9
snd_lib_error_set_handler@ALSA_0.9 ... (31 total)
```

Note `nm -D` is useless on this artifact — it reports *"corrupt string table index … no symbols"*
on the 1.1 GB unstripped libxul. Use `readelf --dyn-syms -W`. That is a tooling trap, not a
defect in the binary.

---

## A PRE-EXISTING BUILD-SCRIPT BUG FOUND ALONG THE WAY — not caused by this change

`build/webos-oe/build-goanna-arm.sh` **exits 1 on a fully successful build.** This was hit on
every run of this task and must not be mistaken for the ALSA change failing. It is the mirror
image of the fail-open class this project keeps documenting: a **fail-CLOSED** step that reports
failure it did not suffer, and silently skips the rest of its own verification.

The cause is the JS-patch assertion loop at the end of the script, under `set -euo pipefail`:

```bash
grep -E '^\+\+\+ b/.*\.(js|jsm)$' "$p" | sed 's|^+++ b/||' | while read -r f; do ... done
```

When a patch adds no `.js`/`.jsm` file, `grep` exits 1; `pipefail` promotes that to the pipeline;
`set -e` kills the script. **27 of the 29 patches add no JS**, and the first of them is
`0001-js-format-overflow.patch` — so this fires on the very first iteration, every time.

Consequences, both bad:

1. The script exits 1 with **no error message at all** and never prints its
   `== ARM build stage 'build' done ==` line, so any caller reads a successful engine build as a
   failure.
2. Everything after that point never runs — including the JS-dist staleness assertions the block
   exists to perform. Those guards have **never** executed.

Verified rather than inferred: the pipeline was reproduced standalone (`simulated exit=1`), and an
earlier hypothesis blaming the `newest`-patch loop was tested and **disproved** (that loop exits 0;
commands inside a `&&`/`||` list are exempt from `errexit`). The wrong guess is recorded because
the plausible-but-wrong explanation is the one a later reader would otherwise repeat.

The fix is one character class of change — `{ grep ... || true; } | sed ...`, or restructure to
avoid the pipeline — but `build-goanna-arm.sh` is **not owned by this task**, so it was left
alone and is reported instead. **Until it is fixed, judge an ARM build by its artifacts
(`libxul.so` mtime + `readelf`), not by the script's exit code.**

---

## 2026-08-15 device test — the backend WORKS, output is webOS-POLICY-gated, and the bundling decision was WRONG

Tested on the TouchPad (full transcript `impl-device-2026-08-15.md`). Two corrections to this doc:

**1. The output pipe is live and reaches pulseaudio.** `<audio>` for WAV and Ogg/Vorbis reports
`err=0 readyState=4`, and `pactl list` during playback shows 5+ pulse sink-inputs at
`s16le 2ch 44100Hz` named "ALSA Playback". cubeb opened the PCM and connected to pulse — the backend
is genuinely functional on hardware, not merely linked.

**2. There is no sound, and it is NOT our bug: webOS audio is POLICY-gated.** Every pulse sink stays
`State: SUSPENDED` and each of our sink-inputs reads `Volume: 0% / -inf dB`. The hardware PCM is held
by Android's `mediaserver` HAL; webOS un-suspends/routes a sink only when its audio POLICY manager
grants an app a playback "scenario" over LunaService (`com.palm.audio`/audiod). The remaining work is
a LunaService audio-scenario request from the engine, NOT a cubeb/decoder change. (`paplay` hangs the
same way; `pactl suspend-sink … 0` returns Invalid argument; killing mediaserver does not free it.)

**3. BUNDLING JESSIE'S libasound IS WRONG FOR THIS DEVICE — reverse of what this doc concluded from
the host.** With the bundled Jessie 1.0.28 `libasound.so.2`, `<audio>` gave `err=3`
(MEDIA_ERR_DECODE) — the 1.0.28 lib mis-negotiates against the device's 0.9.8-era pulse plugin ABI.
With the DEVICE's own `/usr/lib/libasound.so.2`, `err=0`. So: do NOT bundle; let the daemon load the
device libasound (its loader search path already falls through to `/usr/lib`, and cryptofs forbids
the bundling symlink anyway). The `.ipk` should stop shipping `deviceroot/hl/libasound.so.2`, or the
runtime library path should prefer `/usr/lib` for that one SONAME. Follow-up for the next build.

---

## 2026-08-15 — THE POLICY GATE FULLY REVERSE-ENGINEERED, and the port's path is now specified

Kept digging until the whole mechanism was mapped. This section is the spec for the remaining work;
it turns "engine audio produces no sound, cause unknown" into a bounded, three-part integration.

**The stock mechanism, found in the device's own framework.** `/usr/palm/frameworks/media/media.js:288`
does `this.audio.palm.audioClass = "media"` — a **Palm WebKit extension on the HTML `<audio>`
element**. `libWebKitLuna`'s media backend reads `element.palm.audioClass` and, for the "media"
class, (a) routes its pulse stream to the `pmedia` virtual sink and (b) drives the audiod **media
scenario** so `module-palm-policy` powers the WM8994 codec route. Goanna/UXP has NO `palm.audioClass`
— it is a Palm extension absent from upstream — so our cubeb streams arrive at pulse as anonymous
clients that policy never routes. This is the exact analog of "webOS Flash isn't a generic NPAPI
plugin": Flash is audible because it has a dedicated `pflash` sink and plugin-container runs the flash
policy path.

**The audio-policy API (device-probed).** `palm://com.palm.audio/media/` with `enableScenario`,
`disableScenario`, `setCurrentScenario`, `getCurrentScenario`, `listScenarios`, `setVolume`. Media
scenarios: `media_front_speaker`, `media_back_speaker` (the two `listScenarios` returns). audiod
(`/usr/sbin/audiod`) owns `msm_playback_route` and talks to pulse's `module-palm-policy` over the
socket `/tmp/palmaudio`.

**Why `enableScenario` alone did NOTHING (measured).** Calling `enableScenario` +
`setCurrentScenario` from a shell returned `{"returnValue":true}` but left every pulse sink
`SUSPENDED` and `paplay` HANGING — to the default sink, to `pmedia`, AND straight to the hardware
`pcm_output`. So the codec route is NOT powered by a bare scenario enable; audiod powers it only
while it believes a policy-recognised media stream is actively playing. There is **no shell reference
playback on this device** — a raw tone cannot be made audible without replicating the media-app
handshake — which also means this integration cannot be validated by "does paplay work first".

**THE PORT'S PATH — three parts, all in the daemon/build, none human-gated:**
1. **Route the engine's ALSA output at the `pmedia` sink**, not the pulse default. The device
   `asound.conf` default is `type pulse` (→ `pcm_output` directly); media must land on `pmedia`
   (`pcm.!default { type pulse; device pmedia }`) so `module-palm-policy` can route it. Ship this as
   the engine's `ALSA_CONFIG_PATH` (the env hook already exists and was exercised this session).
2. **Hold an audiod media scenario for the lifetime of engine playback.** Wire a LunaService client
   call into the AudioStream/cubeb start/stop path: `com.palm.audio/media/enableScenario`
   (+ `setCurrentScenario media_front_speaker`) on the FIRST active stream, ref-counted, and
   `disableScenario` on the LAST stop. The daemon already has an outbound Luna client
   (`JihadLunaService.cpp`; the toast channel proved outbound push works), so this is new calls on an
   existing handle, not new plumbing. The scenario must be held via an `$activity` so audiod does not
   drop it — the API schema shows every method accepts an optional `$activity` object.
3. **Validate** by `pactl list` showing our sink-input on `pmedia` with the sink RESUMED (not
   SUSPENDED) while a scenario is active, then by ear.

This is one focused build+deploy+listen task. It was deliberately NOT implemented in this sitting
because: (a) it needs a daemon rebuild + redeploy cycle, (b) the scenario-hold lifecycle (ref-count,
`$activity`, front-vs-back speaker selection) deserves careful wiring rather than a drive-by, and
(c) there is no working reference playback on the device to A/B against, so shipping it blind would
be guessing — it wants a session that can iterate on the pmedia-routing + scenario-timing together.
The MP3/AAC/H.264 decoder gap remains separate and orthogonal to all of this.

### The simple 3-part path was TESTED end to end on device and it is NOT SUFFICIENT — the real blocker is deeper

Ran the whole orchestration from a shell against the engine's real stream, so the next session does
not spend a build cycle discovering this: `ALSA_CONFIG_PATH` pointing the engine default at `pmedia`
+ `enableScenario`/`setCurrentScenario media_front_speaker` HELD active + `setVolume 90` + an active
engine `<audio>` stream → **`pcm_output` stayed `SUSPENDED`. No sound.** So parts (1) and (2) above,
even done together WITH a live stream, do not power the WM8994 route. The reason, narrowed:

**audiod powers the codec route based on the STREAM being registered with its policy engine as a
"media"-class stream, not on the scenario call alone.** Our stream reaches pulse as
`application.name = "ALSA plug-in [ld-2.23.so]"` with no `media.role` and no Palm audio class, so
`module-palm-policy`/audiod does not count it as an active media playback and never powers the route
— exactly what `palm.audioClass="media"` establishes inside `libWebKitLuna` on the stock browser.
There is **no confirmed shell or pulse-property equivalent**: this build's `paplay` rejects
`--property=media.role=…` outright, and the `/tmp/palmaudio` socket the policy module advertises is
not at that path (abstract or relocated), so the registration protocol itself needs reverse
engineering (trace how audiod + module-palm-policy classify a stream, likely via the audiod↔pulse
control socket and/or a per-stream property the pcm_pulse ALSA plugin cannot set).

**So the honest scope is bigger than "wire up a LunaService call":** engine audio needs the stream
REGISTERED with audiod's policy as media-class, which on stock is a WebKit-internal extension. The
plausible port routes, in order of likelihood, are (a) build the cubeb **PULSE** backend
(`cubeb_pulse.c`, libpulse-direct — NOT the ALSA backend we built) so the stream can carry a
`pa_proplist` role/property that module-palm-policy keys on, then discover WHICH property that is by
tracing a stock-browser media stream's proplist; or (b) reverse-engineer the audiod policy
registration protocol and have the daemon register the stream directly. Either is a research task.
The `enableScenario`/pmedia scaffolding above is still necessary, just not sufficient. This is the
correct, tested boundary — the simple version is disproven, the deeper blocker is named.

### FULLY RESOLVED 2026-08-15 — the mechanism is decoded and the fix is DETERMINED (build task, not research)

Pulled `module-palm-policy.so` (39 KB, `/usr/lib/pulse-0.9.22/modules/`) and `audiod` to the host and
read them, then ran targeted device experiments. The whole routing mechanism is now known, and the
ALSA backend we built is proven a DEAD END for device audio. Do not deploy it expecting sound.

**The policy mechanism (from `module-palm-policy.so` strings + device tests):**
- The webOS virtual sinks (`pmedia`, `pflash`, `pringtones`, …) are **`module-null-sink`s**. A stream
  on one goes nowhere by itself; audiod's per-category scenario module bridges the virtual sink to the
  hardware `pcm_output` and powers the WM8994 codec ONLY for a stream it recognises.
- `module-palm-policy` hooks every new stream (`route_sink_input_new_hook_callback`). Its decisive
  log string: **"THE DEFAULT DEVICE WAS USED TO CREATE THIS STREAM - PLEASE CATEGORIZE USING A VIRTUAL
  STREAM"** — a stream created on the default device is muted (0% policy volume) and left on
  `pcm_output`. Recognition requires the stream to be CREATED on a virtual sink, at which point policy
  calls `virtual_sink_input_set_physical_sink` and notifies audiod
  (`handle_io_event_socket: sent opened input stream count to audiod`), and audiod powers the route.
- Stock browsers do this via the WebKit extension `element.palm.audioClass="media"`, which makes
  `libWebKitLuna` open its libpulse stream on the media virtual sink.

**What was TESTED on device (all with the media scenario enabled + volume 90):**
- Engine `<audio>` (WAV/Ogg) DECODES and the **playback clock runs to completion** — `readyState=4`,
  `currentTime` 0→3.00, `ended=true`, `err=0` — when routed through an `ALSA_CONFIG_PATH` aliasing
  `default`→the media PCM. So the decode + playback pipeline WORKS end to end; the only thing missing
  is audibility. (Without any alias it stalled at `t=0.00`.)
- BUT the engine stream still lands on **`Sink: 0` (pcm_output) at `Volume: 0%`** — policy-muted,
  NOT on the pmedia virtual sink.
- **The killer control: `aplay -D media` (the stock ALSA virtual-sink PCM) ALSO lands on `Sink: 0` at
  `Volume: 0%`** — identical to ours. So the ALSA pulse plugin's `device pmedia` does NOT actually
  target the pmedia pulse sink; ANY plain ALSA client is policy-muted on the hardware sink. Moving an
  existing stream to pmedia with `pactl move-sink-input` also does nothing (the new-stream hook
  already fired on the default sink), and forcing its volume to 100% on pmedia (a null sink) produces
  no hardware samples.

**THE FIX, DETERMINED BY ELIMINATION — build cubeb's PULSE backend targeting the pmedia sink.**
The ALSA backend cannot reach the virtual sink on this device (proven: even stock `aplay -D media`
can't). The stock path is libpulse-direct: `pa_stream_connect_playback` with the sink name `"pmedia"`
and a `pa_proplist`. So:
1. **mozconfig**: turn OFF `--disable-pulseaudio` and turn ON pulse (we currently ship ALSA-only); it
   builds `cubeb_pulse.c`. Needs `libpulse` headers in the sysroot (`libpulse.so.0` is already on the
   device, `/usr/lib/libpulse.so.0.12.3`).
2. **Target the pmedia sink at connect.** cubeb_pulse's `pa_stream_connect_playback(stm->stream,
   sink_name, …)` must pass `"pmedia"` (not NULL). Simplest: patch cubeb_pulse to read a sink name
   from an env (`JIHAD_PULSE_SINK=pmedia`), or hardcode for the device build. Set the stream's
   `media.role` in the proplist too (belt and suspenders; the sink target is the load-bearing part).
3. **Hold the audiod media scenario** for the stream lifetime (`com.palm.audio/media/enableScenario`
   + `setCurrentScenario media_front_speaker`, ref-counted, from the daemon's existing Luna client) —
   the scenario is what audiod uses to pick front/back speaker and set the media volume once it sees
   the recognised stream.
4. Rebuild libxul, deploy, **confirm by ear** (no on-device capture works — all pulse monitors yield
   0 bytes here; audibility is a human check, but the sink-input landing on `Sink 5`=pmedia with a
   non-zero policy volume is the strong observable to gate on first).

This is a bounded build+deploy+listen task with an exact target, NOT open-ended research. The RE is
done. **Critical negative to carry: do NOT ship the current ALSA-backend libxul expecting sound — it
is proven silent on device (policy-muted on pcm_output). The pulse backend is the path.** MP3/AAC/H.264
decoders remain a separate, orthogonal gap.

### IMPLEMENTED 2026-08-15 — pulse backend built and staged (patch 0030)

The fix above was implemented the same session. What landed:
- **mozconfig** (`build/webos-oe/mozconfig.goanna-arm`): `--disable-pulseaudio` → `--enable-pulseaudio`
  (alsa kept as fallback). The old "ALSA already routes into pulse" comment is replaced with the
  disproof.
- **Sysroot**: pulse **0.9.22** headers (matching the device ABI, NOT Jessie's 5.0) staged into
  `arm-sysroot/root/usr/include/pulse/` from the freedesktop 0.9.22 release, plus a `libpulse.pc`
  (Version 0.9.22) in `usr/lib/pkgconfig/` and the device's own `libpulse.so.0.12.3` as
  `libpulse.so` for link-safety. `configure` picks it up: `checking for libpulse... yes`. These are
  new git-ignored sysroot inputs — record them in `arm-sysroot-debs.manifest`/adapter-deps style, or
  re-fetch: `curl freedesktop.org/software/pulseaudio/releases/pulseaudio-0.9.22.tar.gz`,
  headers under `src/pulse/*.h`.
- **cubeb_pulse.c** (patch `0030-cubeb-pulse-webos-virtual-sink.patch`): when no explicit output
  device is requested, `pa_stream_connect_playback` now honours `JIHAD_PULSE_SINK` so the device
  build targets `"pmedia"`. Desktop is unaffected (env unset → NULL → default sink, upstream
  behaviour). Verified viable first: all 62 pulse symbols cubeb needs are present in the device's
  0.9.22 `libpulse.so.0` (host-`nm` of the pulled lib), and cubeb dlopens libpulse (no link dep).
- **Device job**: `JIHAD_PULSE_SINK=pmedia` to be added to `/etc/event.d/jihad`'s exec line.
- **Scenario hold**: for the first validation the audiod media scenario is enabled from the shell
  around the test; PRODUCTIONISING it is the one remaining piece — hold
  `com.palm.audio/media/enableScenario` ref-counted over the stream lifetime from the daemon's Luna
  client (small, in `JihadLunaService.cpp`'s existing outbound path).

**Validation gate (no ears needed):** the engine sink-input must land on `pmedia` (not pcm_output)
and carry a NON-ZERO policy volume while the media scenario is active — that proves policy recognised
it. Final audibility is a human ear check (no on-device pulse-monitor capture works here). Result of
the on-device test is recorded below once the build completes and deploys.

### RESULT 2026-08-15 — BUILT, DEPLOYED, VERIFIED: the pulse backend UNMUTES engine audio and the DAC is fed

The pulse-backend libxul built clean (`cubeb_pulse.o` present, `JIHAD_PULSE_SINK` + `libpulse.so.0`
strings in libxul, `pulse_init` in the object), was pushed to the device with `push-engine-update.sh
enyo`, and tested. Outcome, and it is a WIN:

- **The engine now creates a PULSE-NATIVE stream.** The sink-input's `application.name = "Jihad
  Browser"` (cubeb_pulse sets it from the context name). Before, the ALSA-bridged stream showed
  `application.name = "ALSA plug-in [ld-2.23.so]"`. That identity is the whole difference:
  module-palm-policy recognises a native pulse client and does NOT mute it.
- **The stream is `Volume: 100%`, uncorked, `Mute: no`** — NOT the forced `0% / -inf dB` the ALSA
  path always got. This is the decisive change.
- **The DAC is physically consuming samples.** `/proc/asound/card0/pcm0p/sub0/status` shows
  `state: RUNNING` with `hw_ptr` advancing ~44,400 frames/s (≈44100 Hz) — the WM8994 codec's DAC is
  clocking through the buffer our 100%-volume stream is mixed into. `<audio>` reports
  `readyState=4`, `currentTime` advancing, plays to `ended`, `err=0`.
- **The scenario is NOT required for the unmute** — measured: with `disableScenario` the stream is
  still 100%/uncorked and the DAC still runs. So the pulse-native app identity alone unmutes it; the
  ALSA path's mute was specifically because it presented as an anonymous "ALSA plug-in".

**What is NOT automatically confirmable, and it is the only thing left:** whether the speaker
physically EMITS the sound. The two automated oracles are both broken on this device — `parec` on any
pulse monitor yields 0 bytes, and `hw_ptr` advances during pause too (pulse keeps the sink fed), so
neither can distinguish our-audio-to-the-speaker from silence-to-a-running-DAC. A human ear is the
final check. Two things may still be needed for the SPEAKER AMP (not the unmute, which is done):
1. **Hold the media scenario during playback** — `media_front_speaker` powers the WM8994 speaker
   amp path. The daemon should enable it on first active stream / disable on last, from
   `JihadLunaService.cpp`'s existing outbound Luna client. (The unmute doesn't need it; the amp
   routing might.)
2. **pmedia routing was NOT achieved and turned out unnecessary for the unmute** — `JIHAD_PULSE_SINK=
   pmedia` fell back to `pcm_output` (the null-sink connect didn't take), and the stream is 100% on
   pcm_output regardless. Landing on pmedia WOULD let audiod's `MediaScenarioModule::onSinkChanged`
   auto-power the scenario (no daemon call needed) — worth solving later, but the fix works without it.

**Shipped state:** the pulse-backend libxul is deployed on the enyo variant. `JIHAD_PULSE_SINK` was
removed from the device job (no-op — pmedia didn't take, default works). The mozconfig, patch 0030,
and the sysroot pulse-0.9.22 inputs are in the tree. **Engine audio output is now functional at the
DAC; the residual is the speaker-amp scenario-hold + a human listen.** MP3/AAC/H.264 decoders remain
a separate, orthogonal gap (only WAV/Ogg/WebM/FLAC decode).

### AUDIO IS AUDIBLE — HUMAN-CONFIRMED 2026-08-15

**The user, at the device, reported "I hear the tone" while `jihad-audio3.html` (a looping WAV sine
tone) played through the pulse-backend engine.** That is the acoustic confirmation no software check
on this device could produce (`parec` monitor capture is broken, `hw_ptr` advances during pause). So
engine HTML5 `<audio>` now works end to end on hardware: decode (ffvpx) → cubeb PULSE backend →
pulseaudio → DAC → codec output → **audible**. The pulse-backend fix (mozconfig `--enable-pulseaudio`,
pulse-0.9.22 sysroot headers, patch 0030) is the whole of it; the earlier ALSA backend was silent
because it presented as an anonymous ALSA-plugin stream that module-palm-policy muted, whereas the
pulse-native stream identifies as `application.name="Jihad Browser"` and is left at 100%.

**Which output carried it:** at the moment it was audible the mixer showed `SPKL/SPKR DAC1 Switch`
OFF (speaker) but `Left/Right Output Mixer DAC Switch` ON (headphone/HPOUT), stream at 100% on
`pcm_output`. So it came out the HEADPHONE path — either headphones were attached, or this device's
default routing carries that mixer to the built-in speaker. Either way it is AUDIBLE, which retires
the "human ear" residual. The DAC→speaker (`SPKL DAC1`) auto-routing for the built-in speaker when no
headset is present is the one remaining refinement (audiod's job — see the codec-chain section
below), but it is no longer a blocker on "does engine audio produce sound": it does.

**Also observed by the user: "the audio seeker is going crazy."** Expected for the test asset, not a
defect: `jihad-audio3.html` played a ~3 s tone with `loop=true`, so the `<audio controls>` scrubber
reset to 0 every ~3 s — it looks frantic because the clip is short and loops. Paused it. Worth a
one-line re-check with a longer, non-looping clip to confirm the seek bar tracks smoothly (that is a
videocontrols/offscreen-compositing question, separate from whether audio plays).

### THE FULL CODEC SIGNAL CHAIN TO THE SPEAKER — verified ON device via the mixer (2026-08-15)

`parec` monitor capture is broken here and `hw_ptr` advances during pause, so neither can prove
acoustic output — but the WM8994 codec MIXER (`amixer -c 0`, 269 controls) can be read directly, and
it settles how far the audio physically gets. Findings:
- **DAC1 is fed and routed to the HEADPHONE output, NOT the speaker, at rest.** `DAC1 Switch`=on,
  `Left/Right Output Mixer DAC Switch`=on (→ HPOUT), but `SPKL DAC1 Switch`/`SPKR DAC1 Switch`=**off**.
  So our engine audio reaches the DAC and the headphone amp — and with no headphones plugged in, it
  is inaudible. That, not the pulse policy, is why the device is silent by default for our stream.
- **The luna media scenario call does NOT flip the speaker routing.** `enableScenario`/
  `setCurrentScenario media_front_speaker` left `SPKL/SPKR DAC1 Switch` off — audiod only sets the
  DAC→speaker route for a stream it has registered as media on the pmedia virtual sink, which our
  stream (physical `pcm_output`, `JIHAD_PULSE_SINK=pmedia` having fallen back) does not trigger.
- **Forcing the route completes the chain, and every link is confirmed ON while our stream plays:**
  `amixer cset` `SPKL DAC1 Switch`=on + `SPKR DAC1 Switch`=on, with `DAC1 Switch`=on,
  `Speaker Switch`=on, `Speaker Volume`=63/63 (max), our sink-input at 100%, and `hw_ptr` advancing
  ~44100/s. So the COMPLETE physical path — engine decode → cubeb pulse → DAC1 → speaker mixer →
  speaker amp → speaker — is verified functional end to end on hardware, carrying our audio, every
  stage enabled. This is as close to "audio works" as any software check can reach; only the literal
  acoustic output needs a human ear or a microphone. (Mixer restored to its original headphone-routed
  state after the test; audiod owns it.)

**So the production fix, now precisely bounded to the LAST link:** the daemon must cause the
DAC→speaker route (`SPKL/SPKR DAC1 Switch`) to be ON for media playback. Two clean options: (a) get
the stream registered on the pmedia virtual sink so audiod's `MediaScenarioModule::onSinkChanged`
sets it (debug why `JIHAD_PULSE_SINK=pmedia` fell back to pcm_output — likely the pmedia null-sink
connect needs the policy hook, or the sink name/target); or (b) the daemon sets the two ALSA mixer
switches itself around media playback (simplest, but fights audiod's ownership — do it only if (a)
proves intractable). Everything UPSTREAM of that last link is proven working.
