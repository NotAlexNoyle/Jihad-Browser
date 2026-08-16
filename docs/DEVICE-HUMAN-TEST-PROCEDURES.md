# Human-at-device test procedures — the criteria only a person can close

These are the acceptance criteria that need a real finger, a real keyboard, or a human ear at the
TouchPad — everything an automated agent verified is recorded in `context/impl/impl-device-2026-08-15.md`.
Each item below names the criterion, the exact steps, and the pass condition. Do them on the **enyo**
variant unless noted; it carries the current binaries (pulse-audio libxul `c93f93fe`, T-131-fixed
daemon `35fdba44`).

Setup once: launch the enyo card (`Jihad Enyo` in the launcher), and keep the device on a page you
can navigate from the address bar.

---

## 1. Chrome-page typing, end to end (preferences-ui R6 — the FINAL step)
The daemon fix is verified: text inserts into about:preferences fields once one is focused. The only
thing left is that a REAL TAP focuses the field (the automated test focused it via script).

1. In the address bar, go to `about:preferences`.
2. Tap the **Home button target** field (the URL box under "Home Page").
3. Type a few characters on the VKB.
- **PASS:** the characters appear in the field. (If the VKB does not come up on the tap, that is
  input-bridging R2 / the hit-test item below, not this one.)

## 2. Touch hit-testability of the prefs controls (preferences-ui R6)
1. On `about:preferences`, tap the pane tabs across the top (General / Content / Privacy / Security /
   Advanced) and the **Restore this pane's defaults** button.
- **PASS:** each tap switches the pane / fires the button. (An injected click already did this
  2026-08-06; this is the real-finger confirmation.)

## 3. Two-finger pinch changes zoom (input-bridging R3)
1. Load an ordinary web page (e.g. a news site) with the address bar.
2. Pinch out, then pinch in, with two fingers.
- **PASS:** the page magnifies and shrinks and re-composites cleanly. (The zoom command path is
  device-verified 2026-07-27; this confirms the gesture drives it.)

## 4. VKB typing regression (gre-widgets R3 / input-bridging R2a — highest-risk)
On a plain page with a text field (or the address bar):
1. Type a mix of letters, digits, symbols, non-ASCII.
2. Hold **Backspace** — it should accelerate after a moment (delete faster).
3. Press **Enter** in the address bar — it should submit/navigate.
4. Type a word, then use the on-screen arrow keys (if present) or tap mid-word to move the caret, and
   type — the new text should land at the caret, not the end.
- **PASS:** all four behave. Core insertion + single Backspace + Enter are already device-verified
  via inject; this is the real-VKB confirmation, and settles the arrow/caret question the inject path
  could not (the `key` inject channel is not the adapter path).
- Note: the on-screen VKB has no dedicated Home/End keys; a single-line field reaches start/end via
  Up/Down degrade. The binary reads `0xE0A0=Up 0xE0A1=Down`, the reverse of an older note — one real
  arrow press settles which is which.

## 5. Keyboard arbitration with a plugin (input-bridging R7 / addons R7) — needs a Bluetooth keyboard
This device has no built-in keyboard and the synthetic-uinput route does not reach the card (hidd
does not forward it — see the addons R7 note). So this needs a **paired Bluetooth keyboard**.
1. Pair a BT keyboard.
2. Load a Flash page that takes keys (`/tmp/jihad-keyarb.html`, staged by
   `build/webos-oe/stage-test-pages.sh`), and **double-tap inside the Flash rect** to engage it
   (the `mFlashGestureLock` latch is set by a double-tap — see T-124).
3. Press keys. Then take a screenshot (`luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot
   '{"file":"/tmp/k.png"}'`) and check the ADDRESS BAR.
- **PASS:** keys drive the plugin and do NOT appear in the address bar while the plugin is engaged;
  after a tap OUTSIDE the plugin, keys reach the chrome again.

## 6. Audio: built-in speaker vs. headphones (gre-widgets R1/R7)
Audio is confirmed audible (you heard the tone). This distinguishes the output path.
1. With NO headphones plugged in, play an engine `<audio>` (e.g. `/tmp/jihad-audiolong.html`).
- **If you hear it from the built-in speaker:** the default routing already carries it — done.
- **If silent from the speaker but audible on headphones:** the built-in-speaker auto-route is the
  open audiod refinement (the `SPKL/SPKR DAC1` mixer switch, which audiod does not flip for our
  stream). Recorded in `../context/impl/impl-audio-backend.md`; the clean fix is getting the stream
  registered on the `pmedia` virtual sink so audiod's `onSinkChanged` sets the speaker route.

## 7. Media controls scrubber smoothness (gre-widgets R1) — a REPORTED defect, needs a fix + your eyes
You reported the seek bar is jittery and does not come to rest at clip end. The media element logic
is correct (at end it reads `t=duration, paused=true, ended=true`); the defect is the videocontrols
SCRUBBER rendering on the offscreen surface (uneven repaint cadence, no final settle). This is
offscreen-composite work (same domain as Flash frame pacing). When a fix is attempted, watch a
20-30 s clip (`/tmp/jihad-audiolong.html`) and confirm the thumb moves smoothly and settles at the
end. This is NOT yet fixed — it is the one open item here that is engineering, not just a human check.

## 8. Cert/dialog/download human session (device-build R4) — mostly done
Cert accept + platform store, the SSL dialog, and a real download (byte-identical, in
`/media/internal/downloads`) are all device-verified this session. The only human bit left is
**opening the Downloads app / mounting over USB** to see the downloaded file listed. Do that once to
close the last of R4.
