# T-653, first run of the tools (2026-11-16, Enyo, engine 0113)

`finger-run.py --shells enyo --only rotate,volume-keys,audio-heard,flash-sound` (`run-a.log`).

| script | result |
|---|---|
| `rotate` | **pass**: turned left, up, right, up by `hwinput orientation`; the window 1024x686 in both landscapes, content drawn, a drag scrolls after the last turn. The first geometry line after a turn back reads `win=768x686 mZoom=0.7500` (the width before the height): the script's `wait line … win=768x` takes that line; it should wait for `win=768x9`. |
| `volume-keys` | the keys: **ok** (media volume 23 → 45 after two presses up, → 23 after two down, a clip playing). The two sound checks fail (below). |
| `audio-heard`, `flash-sound` | `expect audio audible` ok (the stream above 0 %, the amp on); **`expect sound heard` fails: `peak 0 rms 0 frames 0`**: `parec -d pcm_output.monitor` wrote nothing in its 1.2 s. `expect sound silent` passes for the same reason, so it proves nothing yet. |

Not run: `mojo-launch-from-app`; Mochi and Mojo.

Earlier the same day, by hand: `hwinput orientation` 0..7 with a screenshot each (3 upside down, 4 upright, 5 and 6
the landscapes, the others nothing); a key datagram to `/var/run/hidd/KeypadEventSocket` moved nothing, a key written
to gpio-keys' event device is published by `com.palm.keys/audio` as `volume_up` down/up and raises the system's alert.
