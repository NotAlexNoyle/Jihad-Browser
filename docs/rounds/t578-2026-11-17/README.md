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

## Dropped frames, the content rate, the stream's time at its end (the suite, 2026-11-18)

Every run of this table is of 2026-11-18, on the pinned engine (queue 0001-0115, head `abfe97ca13`) and the daemon of
commit 7af318c3. "Stock at 8 s" is the stock browser's dropped frames a second read off its three kept pictures (the
report line in each, six to nine seconds into the clip; three to four for the two short clips); the shells' column
beside it is the same reading from `video-round.py`'s pictures of the shells, so the two are like for like. The other
columns are the suite's: dropped frames to the clip's end, the content rate on the screen, the stream's last reported
time.

| clip | codec, rows, authored | bar | stock at 8 s, dropped/s | shells at 8 s, dropped/s | Enyo | Mochi | Mojo |
|---|---|---|---|---|---|---|---|
| gen-spark-240 | Spark 240, 24/s, 20.0 s | 10.3/s | 5.4, 4.3, 0.6 | 1.0, 1.5, 1.0 | dropped 87, 18.4/s, end 20.0 | dropped 79, 18.0/s, end 19.7 | dropped 93, 18.4/s, end 19.7 |
| gen-spark-360 | Spark 360, 24/s, 20.0 s | 12.8/s | 1.9, 0.8, 4.1 | 2.6, 2.6, 2.2 | dropped 86, 19.0/s, end 19.7 | dropped 93, 17.4/s, end 19.7 | dropped 104, 16.5/s, end 19.7 |
| gen-spark-480 | Spark 480, 24/s, 20.0 s | 11.5/s | 1.9, 5.7, 6.0 | 3.6, 2.7, 3.6 | dropped 120, 16.0/s, end 19.5 | dropped 107, 17.6/s, end 19.7 | dropped 146, 16.1/s, end 20.0 |
| zelda-spark | Spark 120, 12/s, 29.7 s | 7.7/s | 0.9, 0, 0 | 0.5, 0.1, 0 | dropped 35, 12.0/s, end 29.6 | dropped 31, 12.0/s, end 29.2 | dropped 36, 12.1/s, end 29.6 |
| sheep-spark | Spark 480, 25/s, 8.8 s | 6.9/s | 7.0, 7.0, 7.8 | (4.6), 9.1, 8.3 | dropped 108, 9.6/s, end 8.6 | dropped 107, 10.2/s, end 8.7 | dropped 103, 12.6/s, end 8.7 |
| gen-h264-240 | H.264 240, 24/s, 20.0 s | 11.2/s | 4.8, 1.8, 0.1 | 2.6, -, - | dropped 91, 17.5/s, end 19.5 | dropped 110, 16.4/s, end 19.5 | dropped 116, 13.8/s, end 19.7 |
| gen-h264-360 | H.264 360, 24/s, 20.0 s | 13.4/s | 0, 0, 0 | 0.2, 3.5, 6.6 | dropped 70, 16.8/s, end 19.7 | dropped 91, 17.7/s, end 20.0 | dropped 96, 14.1/s, end 19.0 |
| gen-h264-480 | H.264 480, 24/s, 20.0 s | 9.3/s | 0, 3.1, 4.3 | 1.0, 0, 2.9 | dropped 119, 16.6/s, end 19.6 | dropped 105, 17.9/s, end 19.7 | dropped 150, 14.6/s, end 19.3 |
| bbb-h264-360 | H.264 (MP4) 360, 30/s, 10.0 s | 7.5/s | 0, 0, 0 | -, 7.4, 6.3 | dropped 33, 23.0/s, end 9.1 | dropped 26, 20.6/s, end 9.3 | dropped 25, 19.6/s, end 9.2 |
| i004-vp6a | VP6 alpha 430, 15/s, 14.0 s | 9.8/s | 1.1, 0.2, 0.2 | 0.8, 0.6, 0.3 | dropped 39, 10.5/s, end 13.6 | dropped 35, **9.6/s**, end 14.0 | dropped 41, 10.4/s, end 13.0 |
| mrandmrs-vp6 | VP6 176, 24/s, 151.1 s | 11.0/s | 1.9, 1.7, 4.5 | 2.8, 1.5, 2.3 | dropped 285 with a seek, end 150.8 | dropped 316, end 151.0 | dropped 377, end 150.6 |
| harrypotter-vp6 | VP6 272, 24/s, 86.8 s | 7.1/s | 5.8, 6.5, 5.3 | 5.1, 3.4, 4.4 | dropped 352 with a seek, end 86.1 | dropped 340, end 86.5 | dropped 387, end 86.2 |

(A dash: that picture holds no report line. The shells' H.264 pictures are the first video round's after the restart,
in which the 240-row clip, the first H.264 clip each shell played after it, drew late: 3.5, 0.3 and 0.1 changes a
second on the screen against the stock browser's 9.5. The suite's runs of the same clip since, on two days, read 14 to
18. Not explained; the first H.264 clip after a restart of the device is the thing to measure again.)

What the sizes cost on this device:
- **Spark and VP6 (the player's own decoders): as the stock browser.** Every size drops frames, more with the rows:
  about one in five at 240 and 360 rows, one in four to one in three at 480, half of the 480-row clip at 25 frames a
  second, one in ten at 120 rows and 12 frames. The stock browser's first eight seconds read the same order.
- **H.264 (the kernel's hardware decoder): more dropped here than in the stock browser at 360 rows.** The stock
  browser dropped none in its first eight seconds of either 360-row clip, three runs each; Mochi and Mojo dropped 3.5 to
  7.4 a second there, Enyo 0.2. To the end of the clips the shells drop 15 to 25 % at every size. This is a defect to
  find, not a limit: the two readings are few, and the suite cannot read the stock browser's count to a clip's end.
- The content rate on the screen is above each clip's bar in every shell but one reading (Mochi's VP6-alpha, 9.6
  against 9.8; Mojo read 9.4 the day before): inside the stock browser's own spread of 9.0 to 11.6 over three runs. The
  bars are loose (the stock browser's lowest run is under its own bar for 11 of the 12 clips) and want five runs over
  the suite's own window before they are trusted to a tenth.

## The slow and stalled download

`flash-video-slow`, three shells: the longest wait between a Buffer.Empty and the next Buffer.Full was 16.5, 16.6 and
16.3 s on the page's clock, with the stream standing at 4.6, 4.0 and 4.4 s; the clip then played to 20 s, the plugin
alive. (The first form of the script saw only the throttle's short underrun; the review said so.)

## Found: a paused video's picture flips between two frames

`flash-video-controls` fails in one shell or another each round at "the paused picture equals its sample". A probe
(fourteen samples of the movie's rectangle two seconds apart, the stream's time standing at 13.818 s) read two hashes
in turn: A A B A A B B B B B A B B B. The screen shows one of two neighbouring frames of the paused video, changing as
the page beside it repaints (the page's report lines change every second). Ours: the frame path keeps the movie's
last frame in one buffer and the one before it in another. T-660.

## What was wrong on the way, and was not the browser

- A seek "running to the clip's end": the scripts sought to within 16 s of the end and then spent seconds on checks. By
  hand (a press at 1, 16, 32 and 56 s) the seek to 135 s of the 151 s clip lands in Enyo as in the stock browser. A first
  diagnosis (the plugin host's 100 ms wait between a stream's pieces) was wrong and its change was taken out again: the
  log added with it (engine patch 0115, `streams:`) showed the 5 MB clip all delivered in three seconds, no wait asked.
- The device's sessions dropping in the middle of a round: the daemon's volume subscriber of commit f0f4043c outlived a
  daemon that was killed, one left on the bus by every restart. Fixed (the script ends its own process group when its
  daemon goes); `device-check.sh` counts orphans.

## Open

- T-660: the paused picture between two frames (above).
- H.264's dropped frames against the stock browser's at 360 rows (above).
- The first H.264 clip after a restart of the device, in the shells.
- The audio stream over a clip's whole length (it is read at the start and after the rate's ten seconds).
- Two real pages with a Flash video player.
- The bars: five stock runs over the suite's window.
