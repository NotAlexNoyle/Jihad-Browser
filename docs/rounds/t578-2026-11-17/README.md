# T-578, Flash video: the round of 2026-11-17

cavekit-flash.md R8. Two readings:

- `video-round.py` (the screen's reading, the one the stock browser can give too): each clip on the instrumented player,
  the content rate over ten seconds, a kept picture with the player's report lines. `stock-video.json` and
  `stock-run2/`, `stock-run3/` are the stock browser's three runs; a clip's bar in the suite is 90 % of the lower of its
  authored rate and the median of the three.
- the suite's `flash-video-*` scripts (the player's own reports, the three shells): docs/rounds/finger-2026-11-17.

## The device's hardware decoder

`before-reboot/` holds the first readings: every H.264 clip blank, in the stock browser and the system's video player
too. The kernel said `pmem: pmem_kalloc: Kernel 0x2 memory arena is not initialized` at every start of the decoder
(`/dev/msm_vidc_dec`); the device was five days up after a boot whose command line said `lastboot=panic`. A restart
cured it. The H.264 scripts begin with `expect decoder ok`, which reads the kernel log for that line.

## Dropped frames, the content rate, the stream's time at its end (the suite, 2026-11-17)

| clip | codec, rows, authored | bar | Enyo | Mochi | Mojo |
|---|---|---|---|---|---|
| gen-spark-240 | Spark 240, 24/s, 20.0 s | 10.3/s | dropped 114, 16.2/s, end 20.0 | dropped 94, 18.5/s, end 19.9 | dropped 104, 16.1/s, end 19.3 |
| gen-spark-360 | Spark 360, 24/s, 20.0 s | 12.8/s | dropped 101, 17.6/s, end 19.5 | dropped 92, 20.1/s, end 20.0 | dropped 116, 15.9/s, end 19.1 |
| gen-spark-480 | Spark 480, 24/s, 20.0 s | 11.5/s | dropped 119, 16.9/s, end 19.5 | dropped 125, 17.6/s, end 20.0 | dropped 138, 15.4/s, end 19.6 |
| zelda-spark | Spark 120, 12/s, 29.7 s | 7.7/s | dropped 33, 12.1/s, end 29.7 | dropped 35, 12.4/s, end 29.6 | dropped 38, 12.2/s, end 29.4 |
| sheep-spark | Spark 480, 25/s, 8.8 s | 6.9/s | dropped 116, 8.6/s, end 8.5 | dropped 125, 10.8/s, end 8.8 | dropped 123, 8.8/s, end 8.8 |
| gen-h264-240 | H.264 240, 24/s, 20.0 s | 11.2/s | dropped 98, 16.9/s, end 19.0 | dropped 118, 16.7/s, end 20.0 | dropped 131, 16.5/s, end 18.9 |
| gen-h264-360 | H.264 360, 24/s, 20.0 s | 13.4/s | dropped 58, 17.8/s, end 18.4 | dropped 72, 16.6/s, end 19.3 | dropped 104, 14.9/s, end 20.0 |
| gen-h264-480 | H.264 480, 24/s, 20.0 s | 9.3/s | dropped 121, 16.4/s, end 19.8 | dropped 98, 15.5/s, end 19.7 | dropped 161, 15.5/s, end 20.0 |
| bbb-h264-360 | H.264 (MP4) 360, 30/s, 10.0 s | 7.5/s | dropped 29, 22.0/s, end 9.0 | dropped 26, 22.4/s, end 9.2 | dropped 38, 17.4/s, end 9.6 |
| i004-vp6a | VP6 alpha 430, 15/s, 14.0 s | 9.8/s | dropped 45, 10.3/s, end 14.0 | dropped 47, 10.5/s, end 14.0 | dropped 48, **9.4/s**, end 13.4 |

The "end" is the stream's time at the last report before `NetStream.Play.Stop` (a report every thirty frames of the
movie, so up to a second or two short of the clip's length). The two long VP6 clips (176 and 272 rows) are played from
their start, sought ahead and played out (`flash-video-vp6-176`, `-272`): no count to the end.

What the sizes cost: no codec holds any size without drops. At 24 frames a second a fifth to a third of the frames are
dropped from 240 rows to 480, Spark and H.264 alike; the 12 and 15 frame clips drop a tenth to a fifth. The stock
browser's kept pictures show the same order (15 to 34 dropped in the first six to eight seconds).

## What was wrong on the way, and was not the browser

- A seek "running to the clip's end": the scripts sought to within 16 s of the end and then spent seconds on checks. By
  hand (a press at 1, 16, 32 and 56 s) the seek to 135 s of the 151 s clip lands in Enyo as in the stock browser. A first
  diagnosis (the plugin host's 100 ms wait between a stream's pieces) was wrong and its change was taken out again: the
  log added with it (engine patch 0115, `streams:`) showed the 5 MB clip all delivered in three seconds, no wait asked.
- The device's sessions dropping in the middle of a round: the daemon's volume subscriber of commit f0f4043c outlived a
  daemon that was killed, one left on the bus by every restart. Fixed (the script ends its own process group when its
  daemon goes); `device-check.sh` counts orphans.

## Open

- Mojo's VP6-alpha rate, 9.4/s under 9.8.
- Mochi, `flash-video-controls`: the paused picture read differently some seconds into the pause in two runs of four.
- The audio stream over a clip's whole length (it is read at the start and after the rate's ten seconds).
- Two real pages with a Flash video player.
