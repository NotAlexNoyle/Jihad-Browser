# Media controls (HTML5 videocontrols) — touch, volume widget, scrubber, volume HUD

Build site: n/a (investigation). Started 2026-08-20.

## What the user reports (on the bitbake .ipk, engine SRCREV bd49435b)

1. **Player volume widget doesn't work at all** (the in-player mute button + volume slider).
2. **Hardware volume buttons DO move the tone** (the daemon speaker/volume live-sync works end-to-end)
   **but no webOS music-volume HUD** appears when they're pressed on a page.
3. **A glitchy `0:00` indicator is stuck at the far left of the scrubber** (the scrubber thumb bubble
   at position 0 — i.e. the thumb is not advancing).

## Atlas reference (WPE/WebKit webOS browser, /home/notalexnoyle/eclipse-workspace/Jihad/Atlas)

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
