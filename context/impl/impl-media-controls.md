# Media controls (HTML5 videocontrols) — touch, volume widget, scrubber, volume HUD

Build site: n/a (investigation). Started 2026-08-20.

> **2026-10-01 — THE 2026-08-20 DIAGNOSIS AT THE BOTTOM OF THIS FILE IS SUPERSEDED. Do not build on it.**
> "`currentTime` never advances, cubeb stream stuck PREPARED" was an artifact of the hand-pushed engine
> that session left on the device (its own last paragraph says the device no longer matched the `.ipk`).
> On a clean install of all three variants on webOS CE 3.1.0 (2026-09-27) an Ogg/Vorbis `<audio>` read
> `paused=false t=9.926 dur=15`, ALSA `state: RUNNING`, pulse sink-input RUNNING at 100% — the media
> clock advances and the stream runs. See `docs/PICKUP.md` §2026-10-01.
> **What that does and does not settle:** it removes the *explanation* for the stuck scrubber, not the
> user's three reports. The volume widget, the scrubber thumb and the volume HUD have NOT been re-tested
> since; they are now acceptance criteria in `../kits/cavekit-gre-widgets.md` R1 (added 2026-10-01) and
> must be re-run on CE 3.1.0 with all three variants on one engine build. A separate, real `<video>`
> defect was found in the same stock-take (MEDIA_ERR_DECODE after 10–60 frames — a
> `BasicPlanarYCbCrImage::CopyData` return-value bug, not a media-clock one); it is tracked in the same
> requirement and must be fixed before any `<video>` controls result is believed.

## What the user reports (on the bitbake .ipk, engine SRCREV bd49435b)

1. **Player volume widget doesn't work at all** (the in-player mute button + volume slider).
2. **Hardware volume buttons DO move the tone** (the daemon speaker/volume live-sync works end-to-end)
   **but no webOS music-volume HUD** appears when they're pressed on a page.
3. **A glitchy `0:00` indicator is stuck at the far left of the scrubber** (the scrubber thumb bubble
   at position 0 — i.e. the thumb is not advancing).

## Atlas reference (WPE/WebKit webOS browser, ~/eclipse-workspace/Jihad/Atlas)

Surveyed for how a shipping offscreen webOS browser solved these:
- **Media controls get input purely by synthesizing MOUSE (pointer) events from touch** —
  `BrowserPageWPE.cpp::dispatchPointer()` emits `wpe_input_pointer_event_type_motion` then
  `..._type_button`. A tap = down+up = one click (`clickAt`/`touchEvent`/`gestureEvent`). **Sliders
  (seek + volume) need button-1-HELD motion**: `dragStart/dragProcess/dragEnd` hold button 1 down
  across motion so WebKit treats a finger drag as a slider drag. No shadow-DOM hit-test, no media-
  control special-casing — stock WebKit controls just respond to synthesized mouse events.
- **The webOS volume HUD for page audio is NOT implemented in Atlas either.** Its PulseAudio sink is a
  bare `pa_simple` PLAYBACK stream (no proplist/scenario). Volume keys are not handled. The only audio-
  scenario integration is a FULLSCREEN-video handoff to `com.palm.mediad` (`audioClass:kAudioStreamMedia`
  via `LSCallFromApplication`) — never for in-browser audio. So the HUD is **greenfield in both browsers**.
- **No custom media-controls CSS** in Atlas — it rides stock WPE WebKit 2.52.4 controls.

## Jihad code analysis (third_party/uxp videocontrols.xml + engine)

- The installed .ipk chrome HAS the scrubber fixes (`startScrubTimer` ×3) and the audio-only guard
  (`isAudioOnly` ×23) — device-verified. So these are RUNTIME issues, not missing code.
- `mozHasAudio` is a **HTMLVideoElement-only** WebIDL attribute; on `<audio>` it is `undefined`. Line 392
  disables the mute/volume widget on `!isAudioOnly && !mozHasAudio`, BUT `<audio>` sets
  `isAudioOnly = (video instanceof HTMLAudioElement)` (lines 565/1640), so the widget is NOT disabled
  for audio. (For a `<video>` WITH audio, `MozHasAudio()` returns `HasAudio()` = true, so also not
  disabled.) → the "volume widget dead" is therefore NOT the disable path; it is input-reaching-the-
  slider or `video.volume` → cubeb wiring.
- Scrubber thumb advances only if `video.currentTime` advances (showPosition sets `scrubber.value =
  currentTime`; `startScrubTimer` nudges it every 40ms but only when `video.duration` is finite &gt; 0).
  Audio plays (hardware volume moves the tone) yet the thumb is stuck at 0 → suspect **currentTime not
  advancing for this content**, or **duration unknown** (NaN/Infinity → the timer guard returns and, if
  timeupdate is also sparse, the thumb barely moves). Needs the runtime `currentTime`/`duration`/
  `mozHasAudio` values, which requires the browser FOREGROUND (the device was locked during this pass;
  content `console.log` is bridged to daemon.log only for chrome/resource/about URLs, and a locked card
  does not run the page timer — `daemon.log` showed `work avg=0 paint avg=0`).

## Interaction with the shipped audio fix

The daemon's `audio-vol.sh` forces the WM8994 output level (numid 74/86) from the SYSTEM media volume
every ~400ms. The player's volume slider sets `video.volume` → (should) scale the cubeb/pulse stream,
which is UPSTREAM of the hardware mixer, so both should apply multiplicatively. If the pulse sink-input
stays 100% when the slider moves, `video.volume`→cubeb is not wired (engine gap) — that would make the
player volume widget inert regardless of input.

## Next steps (need the device UNLOCKED + browser foreground)

1. Observe whether the on-screen time counter advances (currentTime) for an AUDIO clip vs a VIDEO clip
   — isolates an audio-only media-clock gap from a general one.
2. Tap the volume slider and watch `pactl list sink-inputs` Volume — does `video.volume` reach the
   pulse stream? If it stays 100%, the fix is engine-side (wire cubeb set_volume); if it drops but is
   inaudible, the daemon mixer is masking it and the two need reconciling.
3. For sliders, confirm Jihad synthesizes a button-HELD drag (Atlas `dragProcess` pattern), not only a
   tap — otherwise dragging the scrubber/volume won't work even when a single tap does.
4. Volume HUD: greenfield feature — build on the `com.palm.mediad` / audiod media-session hook (the
   only existing lever), not a quick fix.

## 2026-08-20 device diagnosis — scrubber freeze = media currentTime never advances (SUPERSEDED 2026-10-01 — does not reproduce on a clean install; kept as the record of a contaminated-device reading)

Drove the device directly (screenshots via takeScreenShot + a state-readout test page). CONFIRMED, not
inferred:
- An HTML5 `<audio>` element reports `paused=false rs=4 dur=15` (loaded + "playing") yet
  **`currentTime` stays 0.0** — even after a real user tap (gesture) and after a clean reboot. The
  scrubber thumb is therefore frozen at the far-left "0:00"; the scrubber CODE is fine.
- ALSA `/proc/asound/card0/pcm0p/sub0/status` = **`state: PREPARED, hw_ptr: 0`** — the cubeb output
  stream is created but **never reaches RUNNING**, so no samples flow and the audio clock (which drives
  an audio-only element's currentTime — `pulse_stream_get_position` → `pa_stream_get_time`) stays 0.
- `document.hidden=false`, `visibilityState=visible` — so it is NOT the page-visibility/decoder-dormant
  gate (`HTMLMediaElement` IsHidden/NotifyOwnerDocumentActivityChanged). `nsIDocument::IsActive()` is
  `mDocumentContainer && !mRemovedFromDocShell` (container presence, always true), so the docshell
  active flag is a red herring.

Dead ends this session (reverted / noted):
- `GoannaRenderPage::Create` `docShell->SetIsActive(true)` — **broke offscreen rendering entirely**
  (blank page, work avg=0). Reverted. It also doesn't touch the suspend gate (see IsActive above).
- `media.block-autoplay-until-in-foreground=false` (device-bundle/goanna.js, UNTRACKED build artifact,
  deployed only) — did not start playback; a real gesture didn't either, so autoplay-gating is NOT the
  cause. Reasonable to keep for a single-card browser but unverified; the source is make-device-bundle,
  not git.

Leading hypothesis (unconfirmed): the stream stays PREPARED because module-palm-policy isn't
powering/resuming pcm_output for our stream, OR the media-decoder→AudioSink never calls
cubeb_stream_start for this content. Note the tension with the user's earlier report that on the .ipk
"hardware volume moves the tone" (audio DID output) while the scrubber was stuck — i.e. on the .ipk the
audio may have PLAYED but currentTime still froze (clock decoupled), whereas after my push-experiments
the stream is PREPARED (no output). The device engine (libxul/daemon/goanna.js) was overwritten by
push-engine-update during this session and no longer matches the pristine .ipk — reset by reinstalling
the .ipk before the next diagnosis pass, then re-check whether audio OUTPUTS while currentTime freezes
(clock-feedback bug) vs never outputs (stream-start bug). Next trace target: the MediaDecoderStateMachine
→ AudioSink → cubeb_stream_start path, and whether pa_stream_get_time advances when audio is audible.
