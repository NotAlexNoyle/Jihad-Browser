# T-653 and T-654, the tools working (2026-11-16, engine 0113, the three shells)

Rounds: `../t653-2026-11-16/` (the first run, the sink reading 0 frames), `../t653-2026-11-16d/` (the instruments in
the three shells), this one (after the review's fixes), `rerun/` (the last three scripts again after their fixes).

## What was wrong, and what reads now

- **Sound at the sink.** The device's `parec` refuses every stream name (`Invalid stream name`, with any locale): it
  never recorded. `hwinput record SOURCE MS` records through `libpulse-simple.so.0` (dlopen): one second of
  `pcm_output.monitor`. The Flash tone reads rms 21985 at 447 Hz, the page's tones rms 2057 at 439-440 Hz (the assets
  are 440 Hz). `expect sound heard|silent|muted [BOUND] [MIN_RMS] [HZ]`: no state passes unless the recorder handed
  back its frames, the mixer its amp switch and the audio service its volume.
- **The media volume is the codec's, after the sink**: the sink's level is the same at volume 23, 45 and 0. So
  "heard" wants the signal AND the amp on AND the volume above 0; "muted" wants the signal with the way out shut.
  `volume-keys` sets 44, keys it up, down, then down to 0 (`muted`), then up again (`heard`).
- **The volume HUD.** LunaSysMgr logs an alert for each key (`alert: got the following message`,
  `LockWindow::slotTransientAlertActivated`); nothing is drawn in the frame buffer (screenshots 0.1 s and 0.7 s after
  a key, with and without sound). `expect volume-hud` reads the log line and says so.
- **The launch from another app** (`com.palm.app.notes` in front, then the launch with a target): the page, no
  keyboard. The first run's failure was the check reading the frame before the page was drawn (a wait added).
- **The turn**: `rotate` in the three shells. Back upright the daemon logs only `win=768x686` (the width first); the
  tall window is proven by the content check down to row 900.
- **Mojo, Enter to the first paint**: the daemon logs `nav: first paint N ms after the load started` at the first
  content frame of a new address; `mojo-address-typing` read 2810 ms for a DuckDuckGo search (bar 6000).
- **Smoothness bars** (gpu-rendering R12; the gap between shown frames while the page moves, median / p90 ms): pans
  at most 34 / 50, the zoom gestures of desktop Wikipedia at most 45 / 80. Read here: Enyo 24-27 / 35-42 (zoom 36 /
  53), Mochi 25-27 / 38-42 (zoom 37 / 50), Mojo 31-35 / 41-53 (zoom 39 / 79). **Mojo's long flick misses its bar
  (35 / 53).**
- **T-654, a hold on a field in a frame.** `EditableAt` follows frames (the frame loader's docshell; the padding
  and border taken off). Two faults found on the way: `GetContentDocument` called with no script running aborts the
  engine (`nsContentUtils::SubjectPrincipal`; the daemon died at the first hold), and a framed field whose keyboard
  was hidden and brought back by a tap leaves the top document's focus on `<body>`, so the page's commands refused
  the field (the field the frame's own document still calls active is given the window's focus again; a field a page
  kept from being focused is left alone, as before). `frame-field-hold`: plain text in the frame (the page's menu), the
  field (the editing menu: Paste, Select All, kept frame), the field again when already focused.
- **The fatal report** carries the stack's words that point into libxul (the unwinder gives nothing from the signal
  frame), and a daemon that dies under a page's start leaves its report in the round (`crash-<shell>-<id>-*.txt`).

## The adversarial review (ck:inspector, Fable)

One crash path and five wrong-result paths, all fixed before this round: `jihadIsInsideFrameOf` on the same aborting
call; `silent` and `muted` passing on a recorder or a mixer that did not answer; `flash-sound` without a pitch;
`volume-keys` depending on the volume it started at; the menu not judged (the card's `page-hold` line and the
frame's picture now are); the C++ and the script disagreeing on a padded frame; the recorder without a timeout; the
pitch thrown off by a click or a late start.

## Results

`finger-report.md`: 32 runs. Pass in the three shells: `frame-field-hold`, `frame-field`, `long-press-text-field`,
`long-press-menu`, `keyboard-fields`, `volume-keys`, `personas-page-steady`; `mochi-paste`. Known (T-567, the bands
under a flick): `pan-long-flick`, `pan-speeds`, `wikipedia-zoom`. Enyo's `wikipedia-zoom` read zoom 0.91 for 0.69
once and passed in `rerun/`. `rerun/`: `mojo-address-typing`, `wikipedia-zoom`, `frame-field-hold`, `volume-keys`
with the HUD reading, all pass.

## After the second review (ck:inspector)

No crash path left; fixed: the focus mend narrowed to the frame's own active field (it had overridden a page that
cancels mousedown), the address followed on every tick so a pushState between loads cannot time the old page's frame,
the gaps read as decimals with at least 40 of them, a volume check with no mark fails, the recorder's timeout armed
before it connects, and `expect nav-paint` wants the frame BEFORE the load's end. The volume alert's control: three 4 s
windows without a key logged no alert, three with a key up and three with a key down logged it every time; the alert
is deactivated some 2.5 s later and draws nothing. The final runs of every script named here are merged into
`../finger-2026-11-16/` (`run-merge-t653.log`, `run-merge-t653b.log`).
