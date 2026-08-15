---
created: "2026-08-15"
last_edited: "2026-08-15"
status: device-session
---

# Device session 2026-08-15 — the TouchPad reappeared mid-session

The device (topaz-linux) was absent for the whole host-side wave, then plugged in near the end. A
Monitor watch on USB VID `0830` caught it; `novacomd` was already running (needed no root — it was
up from an earlier boot). This file records what was verified ON HARDWARE. novacom notes: every
`run` that must reply is `sleep N |`-piped (stdin-EOF trap); `-c` strings are staged as scripts.

Board: `uname -r` = **`2.6.35-palm-tenderloin`** — confirms T-122's method (tenderloin control case)
and, by the shared derivation, the `2.6.35-palm-shortloin` pin for opal. Device up ~1h49m,
`/var` 54% full, net up (192.168.1.155).

## Deploy performed

- **Two stray upstart jobs, `jihad.bak.12881` and `jihad.bak.8805`, were ENYO-job copies** — three
  daemons contending for the one enyo socket `/tmp/yapserver.jihad-browser`. Stopped and removed.
  If daemon behaviour ever looks schizophrenic, check `initctl list | grep jihad` for `.bak.*`.
- Installed **enyo `.ipk` 1.0.4** (the libasound-bundled build). Verified `deviceroot/hl/libasound.so.2`
  present (836 KB, the Jessie armel lib). Older ipks ≤1.0.3 lack it.
- Swapped the **adapter** to the ipk's PDK build (`BrowserAdapterImpl.so` md5 `a26ff0b1…`, the
  touch-events build) after backing the old one up to `/var/palm/jihad/adapter-backup-20260815/`.
- Pushed the current **daemon** (`out-arm/jihad-browserserver-arm` md5 `24251922…`, cert store +
  contract-green) over the ipk's stale copy. Clean restart; log grew, init clean.

## VERIFIED ON DEVICE

### Cert store (T-135 / browser-services R5) — the whole write path, END TO END
Drove the card to a host self-signed TLS server (`https://192.168.1.245:8443/`). The daemon log,
in order:
```
cert: certificate files -> /var/ssl/jihad/enyo (platform store root)   <- WRITE ACCESS: the daemon
                                                                          creates the store subdir.
                                                                          The kit's open question.
ssl: cert error on 192.168.1.245:8443 secInfo=1 prov=1 status=0 cert=1  <- real nsIX509Cert captured
cert: wrote 793-byte DER as PEM -> /var/ssl/jihad/enyo/jihad-cert-5685-2.pem   <- non-empty certFile
dialog ssl-confirm -> card (pipe …/dialog-4.fifo) host=192.168.1.245 status=0x805a2fe3
    flags=1(untrusted=1 selfSigned=1 mismatch=1 badTime=0 expired=0) -> ordinal 18   <- classified
                                                        from nsISSLStatus FLAGS, not the transport
                                                        nsresult (which is 0x805a2fe3 = malformed
                                                        hello — the "lie" the kit names). ordinal 18
                                                        = the untrusted band of Browser.js's table.
```
Then answering Trust-Once through the reply FIFO (the reply is a WIRE FRAME:
`printf '\000\000\000\002\062\000'` = 4-byte BE length 2 then `"2\0"`, NOT a bare `2`):
```
cert: platform store available (CertInitCertMgr rc=0, read-direction syms dbinfo=1 dbstr=1 path=1)
Adding Entry with serial number 6 to DB for jihad-cert-5685-2   <- libPmCertificateMgr (openssl) accepted it
cert: platform store now trusts serial 6 (session)              <- CertAddTrustedCert (session arm) rc=0
ssl: session override remembered for 192.168.1.245 — reloading  <- NSS override + reload
state top … STOP status=0x0                                      <- PAGE LOADED after trust
```
`cert-session-serials` file then held `6` (the startup-sweep tracking). So on THIS hardware:
`libPmCertificateMgr.so` is present (`/usr/lib/`, Dec 2011), its nine symbols resolve via dlopen,
`CertInitCertMgr`/`CertAddTrustedCert`/the three read-direction symbols all return rc=0, the daemon
is in group `luna` (owns `/var/ssl`), and the accept→platform-write→override→reload chain works. The
read-direction symbols resolving (`dbinfo/dbstr/path=1`) also unblocks the store→NSS enumeration the
kit costed but left unbuilt — the enum-value obstacle for that half is now testable here.

### Non-blocking toast (T-146 / gre-widgets R5) — card half, DEVICE-VERIFIED
`luna-send -P` (public bus) `clearCookies` → daemon logged `notify: privacy: Cookies cleared.
(pushed)` and the **toast is visible in the device screenshot** (`/tmp/toast.png` pulled: a black
transient bar reading "Cookies cleared." over the start page). So: the `notifications` Luna
subscription is reached on the public bus, the daemon pushes, and the Enyo card renders the toast.
The three device unknowns the kit named are all answered YES. Pushed via `push-card-js.sh enyo`
(stamp-proven reload — Browser.js MUST be in the file set or the boot stamp never binds).

### Focus transitions (input-bridging R2 / preferences-ui R6) — CORRECT on device
`clickid in1` → `engine editorFocused=1` (active=in1); `clickid gap` (non-editable) →
`editorFocused=0` (active=BODY); re-tap → `1`. The clean 1→0→1 the desktop focus_test could not
show. **The desktop focus_test failure (exit 4) is HARNESS-SIDE, not the daemon** — recorded so it
is not chased as a product bug. (Trap noted: `click X Y` is NOT an inject command and is silently
ignored; use `clickid`/`clickoff`/`touch`. That is what produced a false `active=BODY` first read.)

## FOUND ON DEVICE — the audio backend is BUILT and REACHES pulse, but webOS GATES output

This is the important finding and it saves the next session a rebuild hunt. The ALSA backend from
the host wave is genuinely live in the ARM libxul:
- With the daemon loading the **device's own** `/usr/lib/libasound.so.2` (0.9.8-era), `<audio>` for
  WAV and Ogg/Vorbis reports **`err=0`, `readyState=4`** — no decode error, stream accepted. (With
  the BUNDLED Jessie `libasound.so.2` it was `err=3` MEDIA_ERR_DECODE — see the deploy note below.)
- `pactl list` during playback shows **5+ sink-inputs**, each `Sample Specification: s16le 2ch
  44100Hz`, `media.name = "ALSA Playback"`, owned by module-native-protocol (pulse). So cubeb's
  ALSA backend opened the PCM, connected to pulseaudio, and is feeding it. The backend WORKS.

**But no sound, and the reason is webOS policy, not our code.** Every pulse sink
(`pcm_output`, `pmedia`, …) stays **`State: SUSPENDED`** and each of our sink-inputs shows
**`Volume: 0% / -inf dB`**. The hardware PCM (`/proc/asound/card0/pcm0p`) is owned by Android's
**`mediaserver`** (the audio HAL), and webOS routes/unsuspends a sink only when its **audio policy
manager** grants an app a playback "scenario" over LunaService. Our daemon connects to pulse as an
anonymous client, gets a stream at zero volume against a suspended sink, and nothing routes it.
`paplay --device=pmedia` from a shell HANGS the same way; `pactl suspend-sink pcm_output 0` returns
`Failure: Invalid argument`. This is the exact shape of the "webOS Flash isn't a generic NPAPI
plugin" lesson, one layer over: **webOS audio isn't generic ALSA/pulse either — it is policy-gated,
and Flash makes sound because plugin-container runs under a recognised media policy.** The engine
must request an audio scenario through the webOS audio service (audiod / `com.palm.audio`) before a
stream will be un-muted. That is the remaining work for engine audio, and it is a LunaService
integration, not a cubeb or decoder fix. MP3/AAC/H.264 decoders remain separately off.

**UPDATE (later same session) — the policy gate is fully reverse-engineered and the port's path is
specified.** The stock mechanism is `element.palm.audioClass = "media"` (Palm WebKit extension,
`/usr/palm/frameworks/media/media.js:288`) which `libWebKitLuna` reads to route to the `pmedia` sink
and drive the audiod media scenario; Goanna lacks it. The audio API is `palm://com.palm.audio/media/`
(`enableScenario`/`setCurrentScenario`, scenarios `media_front_speaker`/`media_back_speaker`), audiod
owns `msm_playback_route`, policy is pulse `module-palm-policy` over `/tmp/palmaudio`. **`enableScenario`
alone measured insufficient** — returns true, sinks stay SUSPENDED, and `paplay` HANGS to every sink
incl. raw hardware `pcm_output` (100%/unmuted), so there is no shell reference playback and the codec
route is powered only while a policy-recognised media stream plays. Full three-part port spec (pmedia
ALSA routing + ref-counted audiod scenario hold with `$activity` + validation) in
`impl-audio-backend.md` §2026-08-15. It's one focused build+deploy+listen task, deliberately left
unstarted here because it needs a rebuild cycle AND has no A/B reference on-device.

**RESOLVED LATER THE SAME SESSION — the pulse backend was built, deployed and VERIFIED to unmute
engine audio and feed the DAC.** The ALSA route was proven a dead end (policy-muted, even stock
`aplay -D media`); switched to cubeb's pulse backend. Now the engine stream is pulse-native
(`application.name="Jihad Browser"`), `Volume: 100%`/uncorked (ALSA was forced 0%), and
`/proc/asound/card0/pcm0p/sub0/status` shows the WM8994 DAC consuming its samples (`hw_ptr` advancing
~44100/s). Built from: mozconfig `--enable-pulseaudio` + pulse-0.9.22 sysroot headers + patch 0030
(`JIHAD_PULSE_SINK`). All 62 cubeb pulse symbols verified present in the device `libpulse.so.0`; cubeb
dlopens it. Residual is a human ear (both automated audibility oracles are broken here) + a possible
daemon media-scenario hold for the speaker amp (the unmute does not need it). Full record:
`impl-audio-backend.md` §RESULT 2026-08-15. NEW TREE INPUTS for reproducibility: pulse-0.9.22 headers
staged in `build/webos-oe/arm-sysroot/root/usr/include/pulse/` + `usr/lib/pkgconfig/libpulse.pc` +
`usr/lib/arm-linux-gnueabi/libpulse.so{,.0}` (link stub = the device's own libpulse) — from the
freedesktop `pulseaudio-0.9.22.tar.gz` (`src/pulse/*.h`); these are in the git-ignored sysroot, re-stage
per the impl note. Patch `build/desktop/patches/0030-cubeb-pulse-webos-virtual-sink.patch` and the
mozconfig edit ARE in-tree.

Deploy detail that matters for the next audio attempt: **cryptofs forbids symlinks**
(`ln -s … Operation not permitted`), so the bundled-vs-device libasound swap could not be done by
symlink; the daemon fell through to `/usr/lib/libasound.so.2` on its own (loader search path), and
that DEVICE lib is what cleared `err=3`→`err=0`. So the kit's "bundle Jessie's libasound" decision
is WRONG for this device: the Jessie 1.0.28 lib mis-decodes against the 0.9.8 pulse plugin ABI here.
Prefer the device's own libasound (do NOT bundle), which is the opposite of what impl-audio-backend.md
concluded from the host — recorded there as a correction.

## NOT reproduced on device — the file:// XPI install trigger
Navigating the card to `file:///tmp/t103-mismatch.xpi` did NOT fire the install flow — the daemon
loaded it as a document (`STOP`), no `InstallTrigger`, no `addon-install-failed`, so the T-103
observer was never exercised on hardware. The desktop harness proved the observer fires; the device
install TRIGGER from a local `file://` XPI is a separate path (the 2026-08-03 device XPI proof used
the card's own install affordance / an http-served trigger, not a bare file navigation). T-103's
device screenshot therefore remains open, and the closer is an http-served or card-initiated XPI,
not a `file://` navigation. Recorded so the next session does not read this as a regression.

## Device left as
- enyo daemon = current out-arm (cert store), adapter = touch-events PDK build, ipk 1.0.4 installed.
  mochi/mojo still on their 2026-08-10 binaries (only enyo was redeployed this session).
- Job env is BACK to shipped flags (`JIHAD_INJECT=1` present; the temporary `ALSA_CONFIG_PATH` /
  `MOZ_LOG` edits were reverted, backup at `/var/palm/jihad/jihad.job.backup-20260815`).
- Test certs swept from `/var/ssl/jihad/enyo`; `cert-session-serials` removed. `/var/ssl/jihad/enyo`
  dir itself left (harmless, daemon owns it).
- `mediaserver` was SIGKILL'd once during the audio hunt; it respawned (new pid) on its own.
- Adapter backup at `/var/palm/jihad/adapter-backup-20260815/`; old enyo daemon binary was removed
  (rm before push, cryptofs mmap rule).

## DOWNLOAD flow — DEVICE-VERIFIED (2026-08-15, added late in session)

Drove the card to an `application/octet-stream` file served from the host
(`http://192.168.1.245:8088/jihad-testfile.bin`, 64 KB random). The daemon log showed the whole
lifecycle: `download handoff mime=application/octet-stream url=…` → `[jihad-dl] start …` →
`[jihad-dl] finished … -> /media/internal/downloads/jihad-testfile.bin` → `download finished
path=/media/internal/downloads/jihad-testfile.bin`. The landed file's md5 `df838cbc5b59c5ae3b9801256c7baa2b`
is BYTE-IDENTICAL to the source. So downloads intercept correctly (DownloadService), complete, and land
in the webOS convention dir with correct content. This closes the DOWNLOAD third of device-build R4
(with cert + dialog, both also verified this session) down to a human-eyeball-the-Downloads-app residual.
Trap re-confirmed: the card disconnects between a `push`/relaunch and a separate test run — combine the
`killall LunaSysMgr` + double-launch + test into ONE script or the inject hits `inject: no page`.

## T-131 chrome-page typing — DIAGNOSED then FIXED + VERIFIED (2026-08-15, late)

Ran the `JIHAD_LOG_INSERT` instrument on device: focusing an about:preferences field then injecting
text logged `insert: branch=NO-TARGET chrome=1 editable=0` — `mChrome->mFocusedEditable` is NULL in a
chrome document (the capture-phase focus listener is on the top CONTENT document and never sees the
chrome field's focus event). Content pages set it fine. So the FIX: `GoannaRenderPage::InsertText`
now recovers a null `mFocusedEditable` from the focused document's `document.activeElement`
(per-document, so not subject to the focus-manager's window-scoping), accepting it only if
`edIsTextInput`. Rebuilt the daemon (`build-daemon-arm.sh`, YAP contract still OK), deployed (new
`jihad-browserserver` md5 `35fdba44…`, replacing `24251922…`). VERIFIED on device: typing into the
home-url field logged `recovered editable from document.activeElement` then `branch=value/sel
before=29 want=32 readback=32` — the value grew by exactly the inserted 3 chars (daemon's own
`edGetValue` readback). Chrome-page keyboard entry now works through the real editable path. Residual:
a real finger tap must set the field as `activeElement` (human, = T-150); the test focused it via a
scripted `.focus()`. Also found: bare `about:preferences` renders only header/footer — the card's
`#chrome=` fragment is required to render the panes. The fix rides `push-engine-update.sh` (it is in
`jihad-browserserver`, not the adapter).
