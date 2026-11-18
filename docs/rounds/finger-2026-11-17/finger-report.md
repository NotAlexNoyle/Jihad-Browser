# Finger suite report — 2026-11-17T21:39:40

First run 2026-11-17T11:47:07; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 1. 21147 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2948,2956,2935,2950,2913, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| enyo | autoplay-sound | **pass** | 3 ok, 0 failed | WAIT ok 13-text status = [playing]; frames: kept/enyo-autoplay-sound-refused_.jpg | cavekit-gre-widgets.md R1 |
| enyo | flash-app-mp3 | **pass** | 5 ok, 0 failed | CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 | cavekit-flash.md R7 |
| enyo | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/enyo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| enyo | flash-lane (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,284: 3.0 s, flips 41 (13.7/s), changes 0 (0.0/s), reads 42, distinct 1, spacing median 66 p90 146 max 172 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| enyo | flash-sound (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2952 rms 2057 frames 45056 hz 440 env 2944,2952,2943,2950,2938,2948, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| enyo | volume-keys (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 33-sound: sound at the output sink: peak 2954 rms 2059 frames 45056 hz 440 env 2950,2954,2940,2949,2954,2909, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| mochi | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2956,2939,2950,2955,2919, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| mochi | flash-app-mp3 | **pass** | 5 ok, 0 failed | CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 | cavekit-flash.md R7 |
| mochi | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/mochi-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mochi | flash-lane (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,278: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 66 p90 86 max 180 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mochi | flash-sound (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2955 rms 2057 frames 45056 hz 440 env 2935,2950,2955,2943,2951,2929, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| mochi | volume-keys (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 33-sound: sound at the output sink: peak 2955 rms 2059 frames 45056 hz 440 env 2940,2937,2950,2935,2947,2955, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| mojo | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2956,2939,2950,2955,2919, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| mojo | flash-app-mp3 | **pass** | 5 ok, 0 failed | CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 | cavekit-flash.md R7 |
| mojo | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/mojo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mojo | flash-lane (rerun) | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,275: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 67 p90 98 max 161 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mojo | flash-sound (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2957 rms 2057 frames 45056 hz 440 env 2934,2951,2957,2944,2952,2943, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| mojo | volume-keys (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 33-sound: sound at the output sink: peak 2963 rms 2060 frames 45056 hz 440 env 2957,2950,2963,2950,2942,2934, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| enyo | flash-video-controls (rerun) | **pass** | 18 ok, 0 failed | CHECK ok 35-plugin: plugin-container 28387: alive (state S, cpu ticks 2252 to 2263) | cavekit-flash.md R8 |
| enyo | flash-video-flowplayer (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-plugin: plugin-container 25795: alive (state S, cpu ticks 815 to 828) | cavekit-flash.md R8 |
| enyo | flash-video-h264-240 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 30955: alive (state R, cpu ticks 1069 to 1081) | cavekit-flash.md R8 |
| enyo | flash-video-h264-360 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 4654: alive (state S, cpu ticks 1009 to 1021) | cavekit-flash.md R8 |
| enyo | flash-video-h264-480 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 10267: alive (state S, cpu ticks 1139 to 1150) | cavekit-flash.md R8 |
| enyo | flash-video-h264-mp4 (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 15582: alive (state S, cpu ticks 536 to 544) | cavekit-flash.md R8 |
| enyo | flash-video-mediaelement (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-plugin: plugin-container 20755: alive (state S, cpu ticks 629 to 637) | cavekit-flash.md R8 |
| enyo | flash-video-slow (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 22-plugin: plugin-container 25869: alive (state S, cpu ticks 672 to 684) | cavekit-flash.md R8 |
| enyo | flash-video-spark-240 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 27040: alive (state S, cpu ticks 528 to 539) | cavekit-flash.md R8 |
| enyo | flash-video-spark-360 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 30759: alive (state S, cpu ticks 681 to 691) | cavekit-flash.md R8 |
| enyo | flash-video-spark-480 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 1806: alive (state S, cpu ticks 789 to 800) | cavekit-flash.md R8 |
| enyo | flash-video-spark-sheep (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 2708: alive (state S, cpu ticks 317 to 328) | cavekit-flash.md R8 |
| enyo | flash-video-spark-small (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 9041: alive (state S, cpu ticks 438 to 449) | cavekit-flash.md R8 |
| enyo | flash-video-vp6-176 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 2411: alive (state S, cpu ticks 2572 to 2583) | cavekit-flash.md R8 |
| enyo | flash-video-vp6-272 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 7589: alive (state S, cpu ticks 2304 to 2315) | cavekit-flash.md R8 |
| enyo | flash-video-vp6a (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 24-plugin: plugin-container 12154: alive (state S, cpu ticks 553 to 563) | cavekit-flash.md R8 |
| mochi | flash-video-controls (rerun) | **pass** | 18 ok, 0 failed | CHECK ok 35-plugin: plugin-container 17716: alive (state S, cpu ticks 2239 to 2249) | cavekit-flash.md R8 |
| mochi | flash-video-flowplayer (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-plugin: plugin-container 3431: alive (state S, cpu ticks 783 to 796) | cavekit-flash.md R8 |
| mochi | flash-video-h264-240 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 20764: alive (state S, cpu ticks 1086 to 1097) | cavekit-flash.md R8 |
| mochi | flash-video-h264-360 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 10872: alive (state S, cpu ticks 971 to 982) | cavekit-flash.md R8 |
| mochi | flash-video-h264-480 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 14314: alive (state S, cpu ticks 1125 to 1135) | cavekit-flash.md R8 |
| mochi | flash-video-h264-mp4 (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 17651: alive (state S, cpu ticks 541 to 549) | cavekit-flash.md R8 |
| mochi | flash-video-mediaelement (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-plugin: plugin-container 20750: alive (state S, cpu ticks 621 to 628) | cavekit-flash.md R8 |
| mochi | flash-video-slow (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 22-plugin: plugin-container 24039: alive (state R, cpu ticks 706 to 717) | cavekit-flash.md R8 |
| mochi | flash-video-spark-240 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 28398: alive (state S, cpu ticks 536 to 548) | cavekit-flash.md R8 |
| mochi | flash-video-spark-360 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 32016: alive (state S, cpu ticks 669 to 680) | cavekit-flash.md R8 |
| mochi | flash-video-spark-480 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 3215: alive (state S, cpu ticks 803 to 814) | cavekit-flash.md R8 |
| mochi | flash-video-spark-sheep (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 21164: alive (state S, cpu ticks 307 to 319) | cavekit-flash.md R8 |
| mochi | flash-video-spark-small (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 10288: alive (state S, cpu ticks 429 to 440) | cavekit-flash.md R8 |
| mochi | flash-video-vp6-176 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 24364: alive (state R, cpu ticks 2561 to 2572) | cavekit-flash.md R8 |
| mochi | flash-video-vp6-272 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 29381: alive (state R, cpu ticks 2281 to 2292) | cavekit-flash.md R8 |
| mochi | flash-video-vp6a (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 24-plugin: plugin-container 1363: alive (state S, cpu ticks 564 to 575) | cavekit-flash.md R8 |
| mojo | flash-video-controls (rerun) | **pass** | 18 ok, 0 failed | CHECK ok 35-plugin: plugin-container 5864: alive (state S, cpu ticks 2061 to 2072) | cavekit-flash.md R8 |
| mojo | flash-video-flowplayer (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-plugin: plugin-container 4135: alive (state S, cpu ticks 816 to 829) | cavekit-flash.md R8 |
| mojo | flash-video-h264-240 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 10008: alive (state R, cpu ticks 896 to 907) | cavekit-flash.md R8 |
| mojo | flash-video-h264-360 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 11557: alive (state S, cpu ticks 767 to 778) | cavekit-flash.md R8 |
| mojo | flash-video-h264-480 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 26-plugin: plugin-container 14901: alive (state S, cpu ticks 981 to 992) | cavekit-flash.md R8 |
| mojo | flash-video-h264-mp4 (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 18355: alive (state S, cpu ticks 415 to 424) | cavekit-flash.md R8 |
| mojo | flash-video-mediaelement (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 17-plugin: plugin-container 21592: alive (state S, cpu ticks 457 to 465) | cavekit-flash.md R8 |
| mojo | flash-video-slow (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 22-plugin: plugin-container 24812: alive (state S, cpu ticks 626 to 638) | cavekit-flash.md R8 |
| mojo | flash-video-spark-240 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 29009: alive (state S, cpu ticks 503 to 513) | cavekit-flash.md R8 |
| mojo | flash-video-spark-360 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 32418: alive (state S, cpu ticks 619 to 630) | cavekit-flash.md R8 |
| mojo | flash-video-spark-480 (rerun) | **pass** | 13 ok, 0 failed | CHECK ok 25-plugin: plugin-container 3616: alive (state S, cpu ticks 744 to 756) | cavekit-flash.md R8 |
| mojo | flash-video-spark-sheep (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 7003: alive (state S, cpu ticks 297 to 309) | cavekit-flash.md R8 |
| mojo | flash-video-spark-small (rerun) | **pass** | 11 ok, 0 failed | CHECK ok 23-plugin: plugin-container 10592: alive (state S, cpu ticks 411 to 421) | cavekit-flash.md R8 |
| mojo | flash-video-vp6-176 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 13734: alive (state S, cpu ticks 2365 to 2376) | cavekit-flash.md R8 |
| mojo | flash-video-vp6-272 (rerun) | **pass** | 14 ok, 0 failed | CHECK ok 25-plugin: plugin-container 18487: alive (state S, cpu ticks 2120 to 2132) | cavekit-flash.md R8 |
| mojo | flash-video-vp6a (rerun) | **fail** | 11 ok, 1 failed | CHECK FAIL 17-flash-rate: content 40,114-440,412: 10.0 s, flips 115 (11.5/s), changes 94 (9.4/s), reads 122, distinct 93, spacing median 67 p90 178 max 277 ms (wanted 9.8..40/s); frames: kept/mojo-flash-video-vp6a-17-flash-rate_.jpg | cavekit-flash.md R8 |

## The device at the end

```
(not checked)
```

## Every test's lines

### enyo / audio-heard — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +8 s, 18 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31144 rms 21986 frames 45056 hz 440 env 31130,31134,31137,31144,31137,31131, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2948,2956,2935,2950,2913, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### enyo / autoplay-sound — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread 
    WAIT ok 8-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 10-text status = [play refused NotAllowedError]
    WAIT ok 13-text status = [playing]

### enyo / flash-app-mp3 — pass

    enyo card on http://192.168.1.245:8097/title/mp3-player-multi: connected at +2 s, load done at +7 s, 14 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect m
    WAIT ok 10-still m
    CHECK ok 12-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    CHECK ok 15-sound-watched: sound at the output sink over the gesture: peak 1820 rms 191 frames 353280 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,289,502,509,265,1131,1757,1746,1761,1203,1257,919,1023,1537,1011,1155,750,820,959,1820,1196,1694,224, the speaker amp on, the media volume 23
    CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

### enyo / flash-game-sound — pass

    enyo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +2 s, load done at +10 s, 13 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### enyo / flash-lane — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +2 s, load done at +8 s, 15 content frames at +8 s; the GPU path, frames off t
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,123-440,421: 3.0 s, flips 43 (14.3/s), changes 41 (13.7/s), reads 44, distinct 42, spacing median 67 p90 86 max 181 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,123-440,421: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 66 p90 87 max 167 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,123-440,421: 3.0 s, flips 42 (14.0/s), changes 40 (13.3/s), reads 44, distinct 41, spacing median 66 p90 88 max 220 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 138
    CHECK ok 16-flash-rate: content 40,0-440,284: 3.0 s, flips 42 (14.0/s), changes 31 (10.3/s), reads 43, distinct 32, spacing median 68 p90 86 max 196 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,284: 3.0 s, flips 41 (13.7/s), changes 0 (0.0/s), reads 42, distinct 1, spacing median 66 p90 146 max 172 ms (wanted 0..4/s)

### enyo / flash-sound — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31139 rms 21985 frames 45056 hz 440 env 31139,31136,31138,31133,31134,31126, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2952 rms 2057 frames 45056 hz 440 env 2944,2952,2943,2950,2938,2948, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### enyo / volume-keys — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +7 s, 16 content frames at +7 s; the GPU path, frames off the main thread
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2955 rms 2058 frames 45056 hz 440 env 2950,2938,2948,2955,2949,2930, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume-hud: the window server raised its alert for the key (syslog: void LockWindow::slotTransientAlertActivated(): has displayable content); it draws no HUD in the frame buffer on this build
    CHECK ok 17-volume: the media volume 66, higher (marked 44)
    CHECK ok 18-volume: the codec's output level 41 of 63 for the media volume 66
    CHECK ok 19-sound: sound at the output sink: peak 2958 rms 2057 frames 45056 hz 440 env 2938,2942,2958,2950,2956,2923, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 22-volume: the media volume 44, lower (marked 66)
    CHECK ok 23-volume: the codec's output level 27 of 63 for the media volume 44
    CHECK ok 24-sound: sound at the output sink: peak 2963 rms 2057 frames 45056 hz 440 env 2963,2943,2953,2938,2943,2929, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 28-volume: the media volume 0, zero (marked 66)
    CHECK ok 29-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2958 rms 2060 frames 45056 hz 440 env 2952,2941,2946,2958,2946,2950): no sound out
    CHECK ok 32-volume: the media volume 22, higher (marked 0)
    CHECK ok 33-sound: sound at the output sink: peak 2954 rms 2059 frames 45056 hz 440 env 2950,2954,2940,2949,2954,2909, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### mochi / audio-heard — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +4 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31139 rms 21985 frames 45056 hz 440 env 31138,31133,31134,31139,31130,31127, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2956,2939,2950,2955,2919, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mochi / flash-app-mp3 — pass

    mochi card on http://192.168.1.245:8097/title/mp3-player-multi: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect m
    WAIT ok 10-still m
    CHECK ok 12-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    CHECK ok 15-sound-watched: sound at the output sink over the gesture: peak 1761 rms 178 frames 353280 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,502,509,265,1131,1757,1746,1761,1531,1033,1257,930,1537,1106,1155,1052,820,959,694,46, the speaker amp on, the media volume 23
    CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

### mochi / flash-game-sound — pass

    mochi card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +1 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### mochi / flash-lane — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off t
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,119-440,417: 3.0 s, flips 44 (14.7/s), changes 42 (14.0/s), reads 46, distinct 43, spacing median 67 p90 92 max 223 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,119-440,417: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 67 p90 94 max 178 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,119-440,417: 3.0 s, flips 45 (15.0/s), changes 44 (14.7/s), reads 46, distinct 45, spacing median 68 p90 81 max 160 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 140
    CHECK ok 16-flash-rate: content 40,0-440,278: 3.0 s, flips 44 (14.7/s), changes 24 (8.0/s), reads 45, distinct 25, spacing median 67 p90 82 max 149 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,278: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 66 p90 86 max 180 ms (wanted 0..4/s)

### mochi / flash-sound — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +9 s, 13 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31144 rms 21985 frames 45056 hz 440 env 31134,31128,31144,31139,31134,31128, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2955 rms 2057 frames 45056 hz 440 env 2935,2950,2955,2943,2951,2929, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mochi / volume-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2955 rms 2058 frames 45056 hz 440 env 2950,2948,2955,2938,2949,2932, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume-hud: the window server raised its alert for the key (syslog: void LockWindow::slotTransientAlertActivated(): has displayable content); it draws no HUD in the frame buffer on this build
    CHECK ok 17-volume: the media volume 66, higher (marked 44)
    CHECK ok 18-volume: the codec's output level 41 of 63 for the media volume 66
    CHECK ok 19-sound: sound at the output sink: peak 2964 rms 2060 frames 45056 hz 440 env 2956,2950,2955,2964,2960,2953, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 22-volume: the media volume 44, lower (marked 66)
    CHECK ok 23-volume: the codec's output level 27 of 63 for the media volume 44
    CHECK ok 24-sound: sound at the output sink: peak 2953 rms 2059 frames 45056 hz 440 env 2944,2938,2946,2953,2947,2932, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 28-volume: the media volume 0, zero (marked 66)
    CHECK ok 29-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2954 rms 2058 frames 45056 hz 440 env 2948,2944,2943,2949,2954,2945): no sound out
    CHECK ok 32-volume: the media volume 22, higher (marked 0)
    CHECK ok 33-sound: sound at the output sink: peak 2955 rms 2059 frames 45056 hz 440 env 2940,2937,2950,2935,2947,2955, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### mojo / audio-heard — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +1 s, load done at +6 s, 14 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31141 rms 21985 frames 45056 hz 440 env 31133,31133,31137,31141,31139,31127, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440 env 2951,2956,2939,2950,2955,2919, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mojo / flash-app-mp3 — pass

    mojo card on http://192.168.1.245:8097/title/mp3-player-multi: connected at +1 s, load done at +6 s, 17 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect m
    WAIT ok 10-still m
    CHECK ok 12-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    CHECK ok 15-sound-watched: sound at the output sink over the gesture: peak 1820 rms 208 frames 353280 hz 379 env 0,0,0,0,0,0,0,0,0,0,0,0,0,289,502,509,265,1131,1757,1746,1761,1203,1257,919,1023,1537,1106,1155,1052,820,959,1820,1196,1694,416,151,1127,1126,1068,688,121, the speaker amp on, the media volume 23
    CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 env 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

### mojo / flash-game-sound — pass

    mojo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +2 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### mojo / flash-lane — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off th
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,114-440,412: 3.0 s, flips 42 (14.0/s), changes 40 (13.3/s), reads 43, distinct 41, spacing median 69 p90 108 max 154 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,114-440,412: 3.0 s, flips 39 (13.0/s), changes 0 (0.0/s), reads 40, distinct 1, spacing median 68 p90 104 max 186 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,114-440,412: 3.0 s, flips 44 (14.7/s), changes 43 (14.3/s), reads 45, distinct 44, spacing median 67 p90 83 max 145 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 93
    CHECK ok 16-flash-rate: content 40,0-440,275: 3.0 s, flips 43 (14.3/s), changes 34 (11.3/s), reads 44, distinct 35, spacing median 66 p90 89 max 149 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,275: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 67 p90 98 max 161 ms (wanted 0..4/s)

### mojo / flash-sound — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +1 s, load done at +7 s, 15 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31144 rms 21985 frames 45056 hz 440 env 31128,31144,31139,31133,31135,31133, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2957 rms 2057 frames 45056 hz 440 env 2934,2951,2957,2944,2952,2943, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mojo / volume-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +1 s, load done at +6 s, 13 content frames at +6 s; the GPU path, frames off the main thread
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2975 rms 2058 frames 45056 hz 440 env 2940,2952,2943,2951,2975,2931, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume-hud: the window server raised its alert for the key (syslog: void LockWindow::slotTransientAlertActivated(): has displayable content); it draws no HUD in the frame buffer on this build
    CHECK ok 17-volume: the media volume 66, higher (marked 44)
    CHECK ok 18-volume: the codec's output level 41 of 63 for the media volume 66
    CHECK ok 19-sound: sound at the output sink: peak 2963 rms 2059 frames 45056 hz 440 env 2956,2945,2941,2963,2937,2926, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 22-volume: the media volume 44, lower (marked 66)
    CHECK ok 23-volume: the codec's output level 27 of 63 for the media volume 44
    CHECK ok 24-sound: sound at the output sink: peak 2959 rms 2058 frames 45056 hz 440 env 2942,2955,2954,2938,2959,2929, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 28-volume: the media volume 0, zero (marked 66)
    CHECK ok 29-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2956 rms 2059 frames 45056 hz 440 env 2955,2951,2948,2946,2956,2940): no sound out
    CHECK ok 32-volume: the media volume 22, higher (marked 0)
    CHECK ok 33-sound: sound at the output sink: peak 2963 rms 2060 frames 45056 hz 440 env 2957,2950,2963,2950,2942,2934, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### enyo / flash-video-controls — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +2 s, load done at +9 s, 7 content frame
    WAIT ok 13-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 14-text video = [video t=3.892 dropped=32 fps=22.008253094910593]
    WAIT ok 15-content m
    CHECK ok 16-sound: sound at the output sink: peak 2065 rms 387 frames 45056 hz 166 env 870,1292,1085,1400,2065,1466, the speaker amp on, the media volume 23
    WAIT ok 18-text vctl = [vctl pause t=10.213]
    WAIT ok 19-still m
    CHECK ok 21-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 22-region-equal: region 40,123-440,421: colours 2055 hash 399fe976 rows 298 uniform 0 light none blue none = the sample's hash
    WAIT ok 24-text vctl = [vctl resume t=10.213]
    WAIT ok 25-differs m
    CHECK ok 26-sound: sound at the output sink: peak 4231 rms 1148 frames 45056 hz 146 env 3734,4231,2948,3581,2830,2570, the speaker amp on, the media volume 23
    WAIT ok 29-text vctl = [vctl seek 40 t=21.054]
    WAIT ok 30-differs m
    WAIT ok 31-text video = [video t=44.982 dropped=124 fps=17.81472684085511]
    CHECK ok 32-number: vtime = 44.982 (wanted 38..58)
    WAIT ok 33-text vend = [vend NetStream.Play.Stop t=86.833 after video t=86.23 dropped=339 fps=25.236593059936908]
    CHECK ok 34-number: vendtime = 86.23 (wanted 84.3..87.3)
    CHECK ok 35-plugin: plugin-container 28387: alive (state S, cpu ticks 2252 to 2263)

### enyo / flash-video-flowplayer — pass

    enyo card on http://192.168.1.245:8097/title/i004-vp6a: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 10-rect m
    WAIT ok 11-content m
    CHECK ok 12-flash-rate: content 0,83-478,511: 5.0 s, flips 76 (15.2/s), changes 76 (15.2/s), reads 77, distinct 77, spacing median 65 p90 89 max 156 ms (wanted 5..40/s)
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-drawn: drawn: region 0,83-478,511: colours 3387 hash 8076abf2 rows 428 uniform 95 light 83-510 blue none
    WAIT ok 15-still m
    CHECK ok 16-flash-drawn: drawn: region 0,83-478,511: colours 4096 hash 6b8e994c rows 428 uniform 103 light 83-483 blue none
    CHECK ok 17-plugin: plugin-container 25795: alive (state S, cpu ticks 815 to 828)

### enyo / flash-video-h264-240 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-240.flv: connected at +2 s, load done at +7 s, 5 content frames at +7 s; th
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=7 w=426 h=240 d=20.025]
    WAIT ok 15-text video = [video t=1.28 dropped=0 fps=27.272727272727273]
    CHECK ok 16-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 62c4f5e5 rows 298 uniform 0 light none blue 305-324
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3896,3896,3896,3895,3898,3895, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,123-440,421: 10.0 s, flips 214 (21.4/s), changes 169 (16.9/s), reads 216, distinct 170, spacing median 37 p90 84 max 215 ms (wanted 11.2..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 47766a5c rows 298 uniform 0 light none blue none
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=19.017 dropped=98 fps=26.97841726618705]
    CHECK ok 23-number: vendtime = 19.017 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 98 (wanted 0..480)
    CHECK ok 25-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash c0507fb0 rows 298 uniform 0 light none blue none
    CHECK ok 26-plugin: plugin-container 30955: alive (state R, cpu ticks 1069 to 1081)

### enyo / flash-video-h264-360 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-360.flv: connected at +2 s, load done at +13 s, 4 content frames at +13 s; 
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=7 w=640 h=360 d=20.025]
    WAIT ok 15-text video = [video t=1.202 dropped=0 fps=38.07106598984772]
    CHECK ok 16-flash-drawn: drawn: region 40,123-440,421: colours 105 hash 868607ae rows 298 uniform 284 light 123-420 blue none
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3896,3897,3896,3897,3898,3892, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,123-440,421: 10.0 s, flips 252 (25.2/s), changes 178 (17.8/s), reads 253, distinct 167, spacing median 25 p90 70 max 159 ms (wanted 13.4..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,123-440,421: colours 3687 hash 54cab748 rows 298 uniform 0 light none blue 123-420
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=18.442 dropped=58 fps=29.761904761904763]
    CHECK ok 23-number: vendtime = 18.442 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 58 (wanted 0..480)
    CHECK ok 25-flash-drawn: drawn: region 40,123-440,421: colours 3797 hash 3a7d37ae rows 298 uniform 0 light none blue 123-420
    CHECK ok 26-plugin: plugin-container 4654: alive (state S, cpu ticks 1009 to 1021)

### enyo / flash-video-h264-480 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-480.flv: connected at +2 s, load done at +8 s, 6 content frames at +8 s; th
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=7 w=854 h=480 d=20.025]
    WAIT ok 15-text video = [video t=1.672 dropped=3 fps=13.774104683195592]
    CHECK ok 16-flash-drawn: drawn: region 40,123-440,421: colours 3422 hash 3468203d rows 298 uniform 0 light none blue 123-420
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3898,3896,3897,3896,3897,3894, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,123-440,421: 10.0 s, flips 208 (20.8/s), changes 164 (16.4/s), reads 209, distinct 165, spacing median 37 p90 93 max 166 ms (wanted 9.3..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,123-440,421: colours 3357 hash a0884fbd rows 298 uniform 0 light none blue 123-420
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.775 dropped=121 fps=19.28020565552699]
    CHECK ok 23-number: vendtime = 19.775 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 121 (wanted 0..480)
    CHECK ok 25-flash-drawn: drawn: region 40,123-440,421: colours 3467 hash 88ef1992 rows 298 uniform 0 light none blue 123-420
    CHECK ok 26-plugin: plugin-container 10267: alive (state S, cpu ticks 1139 to 1150)

### enyo / flash-video-h264-mp4 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/bbb-h264-360.mp4: connected at +2 s, load done at +7 s, 4 content frames at +7 s; th
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=avc1 w=640 h=360 d=10]
    WAIT ok 15-text video = [video t=1.367 dropped=19 fps=0]
    CHECK ok 16-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 9ca4ec7d rows 298 uniform 0 light none blue none
    CHECK ok 17-flash-rate: content 40,123-440,421: 5.0 s, flips 155 (31.0/s), changes 110 (22.0/s), reads 156, distinct 111, spacing median 22 p90 58 max 125 ms (wanted 7.5..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 7332d9d2 rows 298 uniform 0 light none blue none
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=10 after video t=8.967 dropped=29 fps=48.85993485342019]
    CHECK ok 20-number: vendtime = 8.967 (wanted 7.5..10.5)
    CHECK ok 21-number: venddropped = 29 (wanted 0..300)
    CHECK ok 22-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 7332d9d2 rows 298 uniform 0 light none blue none
    CHECK ok 23-plugin: plugin-container 15582: alive (state S, cpu ticks 536 to 544)

### enyo / flash-video-mediaelement — pass

    enyo card on http://192.168.1.245:8097/title/bbb-h264-360: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    CHECK ok 10-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 11-rect m
    WAIT ok 12-content m
    CHECK ok 13-flash-rate: content 0,83-640,441: 5.0 s, flips 191 (38.2/s), changes 146 (29.2/s), reads 192, distinct 147, spacing median 24 p90 33 max 138 ms (wanted 5..40/s)
    CHECK ok 14-flash-drawn: drawn: region 0,83-640,441: colours 4096 hash 561b5711 rows 358 uniform 0 light none blue none
    WAIT ok 15-still m
    CHECK ok 16-flash-drawn: drawn: region 0,83-640,441: colours 4096 hash 561b5711 rows 358 uniform 0 light none blue none
    CHECK ok 17-plugin: plugin-container 20755: alive (state S, cpu ticks 629 to 637)

### enyo / flash-video-slow — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/slow/40000/500000/10/video/gen-spark-240.flv: connected at +2 s, load done at +7 s, 6 cont
    WAIT ok 11-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 12-text video = [video t=0.025 dropped=0 fps=0]
    CHECK ok 13-flash-drawn: drawn: region 40,123-440,421: colours 3906 hash 28d6e5f7 rows 298 uniform 0 light none blue 198-217
    WAIT ok 14-text vstatus = [vstatus NetStream.Buffer.Empty t=3.709]
    CHECK ok 15-plugin: plugin-container 25869: alive (state S, cpu ticks 203 to 219)
    WAIT ok 16-text vstatus = [vstatus NetStream.Buffer.Full t=3.84]
    WAIT ok 18-differs m
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=20.01 after video t=20.01 dropped=66 fps=20.352781546811396]
    CHECK ok 20-number: vendtime = 20.01 (wanted 17.5..20.5)
    CHECK ok 21-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash c2e9e7b1 rows 298 uniform 0 light none blue none
    CHECK ok 22-plugin: plugin-container 25869: alive (state S, cpu ticks 672 to 684)

### enyo / flash-video-spark-240 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-240.flv: connected at +4 s, load done at +9 s, 6 content frames at +9 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 14-text video = [video t=3.239 dropped=26 fps=22.831050228310502]
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 0ddb46d9 rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3897,3895,3896,3895,3895,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,123-440,421: 10.0 s, flips 175 (17.5/s), changes 162 (16.2/s), reads 181, distinct 162, spacing median 44 p90 84 max 457 ms (wanted 10.3..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 53017963 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=20.01 after video t=20.01 dropped=114 fps=20.63273727647868]
    CHECK ok 22-number: vendtime = 20.01 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 114 (wanted 0..480)
    CHECK ok 24-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 53017963 rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 27040: alive (state S, cpu ticks 528 to 539)

### enyo / flash-video-spark-360 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-360.flv: connected at +2 s, load done at +8 s, 8 content frames at +8 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=640 h=360 d=20.025]
    WAIT ok 14-text video = [video t=3.344 dropped=25 fps=24.429967426710096]
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 6d62406a rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3897,3895,3896,3895,3895,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,123-440,421: 10.0 s, flips 188 (18.8/s), changes 176 (17.6/s), reads 189, distinct 177, spacing median 43 p90 101 max 202 ms (wanted 12.8..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 70587089 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.487 dropped=101 fps=24.193548387096776]
    CHECK ok 22-number: vendtime = 19.487 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 101 (wanted 0..480)
    CHECK ok 24-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 59fc9a0b rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 30759: alive (state S, cpu ticks 681 to 691)

### enyo / flash-video-spark-480 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-480.flv: connected at +3 s, load done at +8 s, 5 content frames at +8 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=854 h=480 d=20.025]
    WAIT ok 14-text video = [video t=3.709 dropped=31 fps=23.4375]
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 4076 hash 34e9b6cd rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3897,3896,3895,3896,3895,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,123-440,421: 10.0 s, flips 177 (17.7/s), changes 169 (16.9/s), reads 180, distinct 170, spacing median 44 p90 112 max 217 ms (wanted 11.5..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 3d63abdf rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.487 dropped=119 fps=24.271844660194176]
    CHECK ok 22-number: vendtime = 19.487 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 119 (wanted 0..480)
    CHECK ok 24-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash b3689f88 rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 1806: alive (state S, cpu ticks 789 to 800)

### enyo / flash-video-spark-sheep — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/sheep-spark.flv: connected at +3 s, load done at +9 s, 8 content frames at +9 s; the
    WAIT ok 13-text video = [video t=2.299 dropped=24 fps=0]
    CHECK ok 14-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 3a1fc867 rows 298 uniform 0 light 123-230 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,123-440,421: 5.0 s, flips 61 (12.2/s), changes 43 (8.6/s), reads 62, distinct 42, spacing median 62 p90 177 max 201 ms (wanted 6.9..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 70252f20 rows 298 uniform 0 light 123-148 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=8.803 after video t=8.542 dropped=116 fps=17.54385964912281]
    CHECK ok 20-number: vendtime = 8.542 (wanted 6.3..9.3)
    CHECK ok 21-number: venddropped = 116 (wanted 0..220)
    CHECK ok 22-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 70252f20 rows 298 uniform 0 light 123-148 blue none
    CHECK ok 23-plugin: plugin-container 2708: alive (state S, cpu ticks 317 to 328)

### enyo / flash-video-spark-small — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/zelda-spark.flv: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the
    WAIT ok 13-text video = [video t=1.3 dropped=0 fps=0]
    CHECK ok 14-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash a126cfa8 rows 298 uniform 0 light 160-162 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,123-440,421: 10.0 s, flips 125 (12.5/s), changes 121 (12.1/s), reads 130, distinct 122, spacing median 75 p90 101 max 337 ms (wanted 7.7..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 6706e27e rows 298 uniform 0 light 185-367 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=29.75 after video t=29.675 dropped=33 fps=12.048192771084338]
    CHECK ok 20-number: vendtime = 29.675 (wanted 27.2..30.2)
    CHECK ok 21-number: venddropped = 33 (wanted 0..354)
    CHECK ok 22-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 5f4a777a rows 298 uniform 0 light 245-420 blue none
    CHECK ok 23-plugin: plugin-container 9041: alive (state S, cpu ticks 438 to 449)

### enyo / flash-video-vp6-176 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/mrandmrs-vp6.flv%26seek=60: connected at +2 s, load done at +7 s, 7 content frames a
    WAIT ok 10-text vmeta = [vmeta codec=4 w=320 h=176 d=151.125]
    WAIT ok 11-text video = [video t=2.873 dropped=18 fps=25.252525252525253]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,123-440,421: 10.0 s, flips 216 (21.6/s), changes 214 (21.4/s), reads 218, distinct 215, spacing median 41 p90 65 max 209 ms (wanted 11.0..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 15f3e9d5 rows 298 uniform 0 light none blue none
    WAIT ok 18-text vctl = [vctl seek 60 t=22.752]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=66.403 dropped=68 fps=19.132653061224488]
    CHECK ok 21-number: vtime = 66.403 (wanted 58..78)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=151.125 after video t=150.648 dropped=368 fps=25.11773940345369]
    CHECK ok 24-number: vendtime = 150.648 (wanted 148.6..151.6)
    CHECK ok 25-plugin: plugin-container 2411: alive (state S, cpu ticks 2572 to 2583)

### enyo / flash-video-vp6-272 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +2 s, load done at +8 s, 9 content frame
    WAIT ok 10-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 11-text video = [video t=3.056 dropped=18 fps=24.691358024691358]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,123-440,421: 10.0 s, flips 222 (22.2/s), changes 219 (21.9/s), reads 223, distinct 220, spacing median 41 p90 61 max 193 ms (wanted 7.1..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 4096 hash 65af1c58 rows 298 uniform 0 light none blue 209-269
    WAIT ok 18-text vctl = [vctl seek 40 t=22.935]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=43.676 dropped=87 fps=26.446280991735538]
    CHECK ok 21-number: vtime = 43.676 (wanted 38..58)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=86.833 after video t=86.413 dropped=353 fps=25.19685039370079]
    CHECK ok 24-number: vendtime = 86.413 (wanted 84.3..87.3)
    CHECK ok 25-plugin: plugin-container 7589: alive (state S, cpu ticks 2304 to 2315)

### enyo / flash-video-vp6a — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/i004-vp6a.flv: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the G
    WAIT ok 13-text vmeta = [vmeta codec=5 w=478 h=430 d=14]
    WAIT ok 14-text video = [video t=3.526 dropped=14 fps=16.216216216216218]
    CHECK ok 15-flash-drawn: drawn: region 40,123-440,421: colours 2592 hash 0fb2e28e rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-flash-rate: content 40,123-440,421: 10.0 s, flips 121 (12.1/s), changes 103 (10.3/s), reads 126, distinct 102, spacing median 63 p90 178 max 301 ms (wanted 9.8..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,123-440,421: colours 1605 hash df749eb5 rows 298 uniform 0 light none blue none
    CHECK ok 19-audio: the movie's audio: open running audible
    WAIT ok 20-text vend = [vend NetStream.Play.Stop t=13.975 after video t=13.975 dropped=45 fps=14.272121788772598]
    CHECK ok 21-number: vendtime = 13.975 (wanted 11.5..14.5)
    CHECK ok 22-number: venddropped = 45 (wanted 0..209)
    CHECK ok 23-flash-drawn: drawn: region 40,123-440,421: colours 1644 hash dfd24d6c rows 298 uniform 0 light none blue none
    CHECK ok 24-plugin: plugin-container 12154: alive (state S, cpu ticks 553 to 563)

### mochi / flash-video-controls — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +1 s, load done at +7 s, 7 content fram
    WAIT ok 13-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 14-text video = [video t=3.134 dropped=9 fps=25.19685039370079]
    WAIT ok 15-content m
    CHECK ok 16-sound: sound at the output sink: peak 3872 rms 872 frames 45056 hz 272 env 2769,2122,2875,3350,3872,2223, the speaker amp on, the media volume 23
    WAIT ok 18-text vctl = [vctl pause t=11.493]
    WAIT ok 19-still m
    CHECK ok 21-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 22-region-equal: region 40,119-440,417: colours 4096 hash 712f794a rows 298 uniform 0 light none blue 191-256 = the sample's hash
    WAIT ok 24-text vctl = [vctl resume t=11.493]
    WAIT ok 25-differs m
    CHECK ok 26-sound: sound at the output sink: peak 6476 rms 1172 frames 45056 hz 226 env 6476,4945,3651,3340,3592,3165, the speaker amp on, the media volume 23
    WAIT ok 29-text vctl = [vctl seek 40 t=23.196]
    WAIT ok 30-differs m
    WAIT ok 31-text video = [video t=44.042 dropped=134 fps=21.650879566982407]
    CHECK ok 32-number: vtime = 44.916 (wanted 38..58)
    WAIT ok 33-text vend = [vend NetStream.Play.Stop t=86.833 after video t=85.759 dropped=392 fps=23.84737678855326]
    CHECK ok 34-number: vendtime = 85.759 (wanted 84.3..87.3)
    CHECK ok 35-plugin: plugin-container 17716: alive (state S, cpu ticks 2239 to 2249)

### mochi / flash-video-flowplayer — pass

    mochi card on http://192.168.1.245:8097/title/i004-vp6a: connected at +1 s, load done at +6 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 10-rect m
    WAIT ok 11-content m
    CHECK ok 12-flash-rate: content 0,79-478,507: 5.0 s, flips 76 (15.2/s), changes 76 (15.2/s), reads 77, distinct 77, spacing median 68 p90 78 max 134 ms (wanted 5..40/s)
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-drawn: drawn: region 0,79-478,507: colours 3153 hash ec0f9ae1 rows 428 uniform 95 light 79-506 blue none
    WAIT ok 15-still m
    CHECK ok 16-flash-drawn: drawn: region 0,79-478,507: colours 4096 hash 6b8e994c rows 428 uniform 103 light 79-479 blue none
    CHECK ok 17-plugin: plugin-container 3431: alive (state S, cpu ticks 783 to 796)

### mochi / flash-video-h264-240 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-240.flv: connected at +1 s, load done at +7 s, 8 content frames at +7 s; t
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=7 w=426 h=240 d=20.025]
    WAIT ok 15-text video = [video t=1.384 dropped=0 fps=35.97122302158273]
    CHECK ok 16-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 638d91ab rows 298 uniform 0 light none blue 248-267
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3895,3898,3894,3897,3896,3896, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,119-440,417: 10.0 s, flips 211 (21.1/s), changes 167 (16.7/s), reads 212, distinct 168, spacing median 38 p90 90 max 189 ms (wanted 11.2..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 98bc4d5e rows 298 uniform 0 light none blue none
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=20.01 dropped=118 fps=20.026702269692922]
    CHECK ok 23-number: vendtime = 20.01 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 118 (wanted 0..480)
    CHECK ok 25-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 5745be60 rows 298 uniform 0 light none blue none
    CHECK ok 26-plugin: plugin-container 20764: alive (state S, cpu ticks 1086 to 1097)

### mochi / flash-video-h264-360 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-360.flv: connected at +1 s, load done at +7 s, 13 content frames at +7 s; 
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=7 w=640 h=360 d=20.025]
    WAIT ok 15-text video = [video t=1.097 dropped=0 fps=0]
    CHECK ok 16-flash-drawn: drawn: region 40,119-440,417: colours 3766 hash 95739b3f rows 298 uniform 0 light none blue 119-416
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3898,3894,3897,3896,3897,3897, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,119-440,417: 10.0 s, flips 227 (22.7/s), changes 166 (16.6/s), reads 229, distinct 167, spacing median 33 p90 81 max 219 ms (wanted 13.4..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,119-440,417: colours 3669 hash 6921ee93 rows 298 uniform 0 light none blue 119-416
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.304 dropped=72 fps=38.265306122448976]
    CHECK ok 23-number: vendtime = 19.304 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 72 (wanted 0..480)
    CHECK ok 25-flash-drawn: drawn: region 40,119-440,417: colours 3820 hash 09d16b5c rows 298 uniform 0 light none blue 119-416
    CHECK ok 26-plugin: plugin-container 10872: alive (state S, cpu ticks 971 to 982)

### mochi / flash-video-h264-480 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-480.flv: connected at +1 s, load done at +7 s, 7 content frames at +7 s; t
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=7 w=854 h=480 d=20.025]
    WAIT ok 15-text video = [video t=1.202 dropped=0 fps=0]
    CHECK ok 16-flash-drawn: drawn: region 40,119-440,417: colours 3401 hash e932ec80 rows 298 uniform 0 light none blue 119-416
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3896,3896,3896,3895,3898,3895, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,119-440,417: 10.0 s, flips 191 (19.1/s), changes 155 (15.5/s), reads 192, distinct 156, spacing median 42 p90 103 max 200 ms (wanted 9.3..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,119-440,417: colours 3331 hash 7aed870c rows 298 uniform 0 light none blue 119-416
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=19.67 dropped=98 fps=34.80278422273782]
    CHECK ok 23-number: vendtime = 19.67 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 98 (wanted 0..480)
    CHECK ok 25-flash-drawn: drawn: region 40,119-440,417: colours 3529 hash ab04f160 rows 298 uniform 0 light none blue 119-416
    CHECK ok 26-plugin: plugin-container 14314: alive (state S, cpu ticks 1125 to 1135)

### mochi / flash-video-h264-mp4 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/bbb-h264-360.mp4: connected at +1 s, load done at +7 s, 10 content frames at +7 s; 
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=avc1 w=640 h=360 d=10]
    WAIT ok 15-text video = [video t=1.833 dropped=24 fps=0]
    CHECK ok 16-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 30dea7e4 rows 298 uniform 0 light none blue none
    CHECK ok 17-flash-rate: content 40,119-440,417: 5.0 s, flips 157 (31.4/s), changes 112 (22.4/s), reads 158, distinct 113, spacing median 22 p90 52 max 155 ms (wanted 7.5..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash b643450e rows 298 uniform 0 light none blue none
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=10 after video t=9.233 dropped=26 fps=48.07692307692308]
    CHECK ok 20-number: vendtime = 9.233 (wanted 7.5..10.5)
    CHECK ok 21-number: venddropped = 26 (wanted 0..300)
    CHECK ok 22-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash e03af1b8 rows 298 uniform 0 light none blue none
    CHECK ok 23-plugin: plugin-container 17651: alive (state S, cpu ticks 541 to 549)

### mochi / flash-video-mediaelement — pass

    mochi card on http://192.168.1.245:8097/title/bbb-h264-360: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    CHECK ok 10-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 11-rect m
    WAIT ok 12-content m
    CHECK ok 13-flash-rate: content 0,79-640,437: 5.0 s, flips 187 (37.4/s), changes 162 (32.4/s), reads 188, distinct 163, spacing median 25 p90 35 max 89 ms (wanted 5..40/s)
    CHECK ok 14-flash-drawn: drawn: region 0,79-640,437: colours 4096 hash d68bfc3f rows 358 uniform 0 light none blue none
    WAIT ok 15-still m
    CHECK ok 16-flash-drawn: drawn: region 0,79-640,437: colours 4096 hash a30a8c59 rows 358 uniform 0 light none blue none
    CHECK ok 17-plugin: plugin-container 20750: alive (state S, cpu ticks 621 to 628)

### mochi / flash-video-slow — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/slow/40000/500000/10/video/gen-spark-240.flv: connected at +1 s, load done at +7 s, 11 co
    WAIT ok 11-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 12-text video = [video t=0.025 dropped=0 fps=0]
    CHECK ok 13-flash-drawn: drawn: region 40,119-440,417: colours 3906 hash 28d6e5f7 rows 298 uniform 0 light none blue 194-213
    WAIT ok 14-text vstatus = [vstatus NetStream.Buffer.Empty t=3.709]
    CHECK ok 15-plugin: plugin-container 24039: alive (state S, cpu ticks 214 to 230)
    WAIT ok 16-text vstatus = [vstatus NetStream.Buffer.Full t=3.84]
    WAIT ok 18-differs m
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=19.984 after video t=19.252 dropped=29 fps=24]
    CHECK ok 20-number: vendtime = 19.252 (wanted 17.5..20.5)
    CHECK ok 21-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash c2e9e7b1 rows 298 uniform 0 light none blue none
    CHECK ok 22-plugin: plugin-container 24039: alive (state R, cpu ticks 706 to 717)

### mochi / flash-video-spark-240 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-240.flv: connected at +2 s, load done at +8 s, 14 content frames at +8 s;
    WAIT ok 13-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 14-text video = [video t=2.769 dropped=17 fps=25.848142164781905]
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 6c2e90e7 rows 298 uniform 0 light none blue 156-173
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3898,3897,3896,3897,3897,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,119-440,417: 10.0 s, flips 188 (18.8/s), changes 185 (18.5/s), reads 190, distinct 186, spacing median 42 p90 98 max 333 ms (wanted 10.3..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 36015a03 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=20.01 after video t=19.853 dropped=94 fps=23.923444976076556]
    CHECK ok 22-number: vendtime = 19.853 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 94 (wanted 0..480)
    CHECK ok 24-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 53017963 rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 28398: alive (state S, cpu ticks 536 to 548)

### mochi / flash-video-spark-360 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-360.flv: connected at +2 s, load done at +8 s, 13 content frames at +8 s;
    WAIT ok 13-text vmeta = [vmeta codec=2 w=640 h=360 d=20.025]
    WAIT ok 14-text video = [video t=0.836 dropped=6 fps=0]
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 3979 hash 524a7128 rows 298 uniform 0 light none blue 314-363
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3896,3896,3896,3895,3898,3895, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,119-440,417: 10.0 s, flips 208 (20.8/s), changes 201 (20.1/s), reads 209, distinct 202, spacing median 41 p90 69 max 188 ms (wanted 12.8..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash a2f70cd8 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=20.01 after video t=20.01 dropped=92 fps=25.682182985553773]
    CHECK ok 22-number: vendtime = 20.01 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 92 (wanted 0..480)
    CHECK ok 24-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 70587089 rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 32016: alive (state S, cpu ticks 669 to 680)

### mochi / flash-video-spark-480 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-480.flv: connected at +1 s, load done at +7 s, 8 content frames at +7 s; 
    WAIT ok 13-text vmeta = [vmeta codec=2 w=854 h=480 d=20.025]
    WAIT ok 14-text video = [video t=1.019 dropped=9 fps=0]
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 3434 hash cb76e63d rows 298 uniform 0 light none blue 182-191
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3895,3898,3897,3896,3897,3892, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,119-440,417: 10.0 s, flips 186 (18.6/s), changes 176 (17.6/s), reads 189, distinct 177, spacing median 43 p90 86 max 322 ms (wanted 11.5..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash b7158359 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=20.01 after video t=20.01 dropped=125 fps=11.92368839427663]
    CHECK ok 22-number: vendtime = 20.01 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 125 (wanted 0..480)
    CHECK ok 24-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 3d63abdf rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 3215: alive (state S, cpu ticks 803 to 814)

### mochi / flash-video-spark-sheep — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/sheep-spark.flv: connected at +1 s, load done at +7 s, 6 content frames at +7 s; th
    WAIT ok 13-text video = [video t=1.202 dropped=10 fps=0]
    CHECK ok 14-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 22289aad rows 298 uniform 0 light 119-164 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,119-440,417: 5.0 s, flips 67 (13.4/s), changes 54 (10.8/s), reads 69, distinct 54, spacing median 60 p90 152 max 217 ms (wanted 6.9..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 7f2a41df rows 298 uniform 0 light 119-144 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=8.803 after video t=8.803 dropped=125 fps=12.386457473162675]
    CHECK ok 20-number: vendtime = 8.803 (wanted 6.3..9.3)
    CHECK ok 21-number: venddropped = 125 (wanted 0..220)
    CHECK ok 22-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 70252f20 rows 298 uniform 0 light 119-144 blue none
    CHECK ok 23-plugin: plugin-container 21164: alive (state S, cpu ticks 307 to 319)

### mochi / flash-video-spark-small — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/zelda-spark.flv: connected at +2 s, load done at +7 s, 6 content frames at +7 s; th
    WAIT ok 13-text video = [video t=2.414 dropped=0 fps=15.340364333652925]
    CHECK ok 14-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 4d3ea43e rows 298 uniform 0 light 119-293 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,119-440,417: 10.0 s, flips 128 (12.8/s), changes 124 (12.4/s), reads 133, distinct 125, spacing median 75 p90 98 max 313 ms (wanted 7.7..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash f9bdbda3 rows 298 uniform 0 light 119-188 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=29.75 after video t=29.628 dropped=35 fps=10.623229461756374]
    CHECK ok 20-number: vendtime = 29.628 (wanted 27.2..30.2)
    CHECK ok 21-number: venddropped = 35 (wanted 0..354)
    CHECK ok 22-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 5f4a777a rows 298 uniform 0 light 241-416 blue none
    CHECK ok 23-plugin: plugin-container 10288: alive (state S, cpu ticks 429 to 440)

### mochi / flash-video-vp6-176 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/mrandmrs-vp6.flv%26seek=60: connected at +1 s, load done at +8 s, 10 content frames
    WAIT ok 10-text vmeta = [vmeta codec=4 w=320 h=176 d=151.125]
    WAIT ok 11-text video = [video t=2.69 dropped=11 fps=26.362038664323375]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,119-440,417: 10.0 s, flips 217 (21.7/s), changes 213 (21.3/s), reads 219, distinct 214, spacing median 41 p90 61 max 208 ms (wanted 11.0..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash b684e6dc rows 298 uniform 0 light none blue none
    WAIT ok 18-text vctl = [vctl seek 60 t=23.458]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=66.586 dropped=58 fps=25.276461295418642]
    CHECK ok 21-number: vtime = 67.813 (wanted 58..78)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=151.125 after video t=150.648 dropped=337 fps=24.096385542168676]
    CHECK ok 24-number: vendtime = 150.648 (wanted 148.6..151.6)
    CHECK ok 25-plugin: plugin-container 24364: alive (state R, cpu ticks 2561 to 2572)

### mochi / flash-video-vp6-272 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +1 s, load done at +7 s, 12 content fra
    WAIT ok 10-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 11-text video = [video t=2.166 dropped=7 fps=25.31645569620253]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,119-440,417: 10.0 s, flips 210 (21.0/s), changes 209 (20.9/s), reads 211, distinct 210, spacing median 43 p90 68 max 184 ms (wanted 7.1..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 4096 hash 7f1910ca rows 298 uniform 0 light none blue 205-265
    WAIT ok 18-text vctl = [vctl seek 40 t=23.484]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=43.781 dropped=72 fps=19.157088122605366]
    CHECK ok 21-number: vtime = 43.781 (wanted 38..58)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=86.833 after video t=86.595 dropped=339 fps=24.50980392156863]
    CHECK ok 24-number: vendtime = 86.595 (wanted 84.3..87.3)
    CHECK ok 25-plugin: plugin-container 29381: alive (state R, cpu ticks 2281 to 2292)

### mochi / flash-video-vp6a — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/i004-vp6a.flv: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the 
    WAIT ok 13-text vmeta = [vmeta codec=5 w=478 h=430 d=14]
    WAIT ok 14-text video = [video t=1.4 dropped=3 fps=16.46090534979424]
    CHECK ok 15-flash-drawn: drawn: region 40,119-440,417: colours 2603 hash e064dcca rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-flash-rate: content 40,119-440,417: 10.0 s, flips 121 (12.1/s), changes 105 (10.5/s), reads 127, distinct 106, spacing median 71 p90 142 max 262 ms (wanted 9.8..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,119-440,417: colours 1605 hash df749eb5 rows 298 uniform 0 light none blue none
    CHECK ok 19-audio: the movie's audio: open running audible
    WAIT ok 20-text vend = [vend NetStream.Play.Stop t=13.975 after video t=13.975 dropped=47 fps=10.344827586206897]
    CHECK ok 21-number: vendtime = 13.975 (wanted 11.5..14.5)
    CHECK ok 22-number: venddropped = 47 (wanted 0..209)
    CHECK ok 23-flash-drawn: drawn: region 40,119-440,417: colours 1644 hash dfd24d6c rows 298 uniform 0 light none blue none
    CHECK ok 24-plugin: plugin-container 1363: alive (state S, cpu ticks 564 to 575)

### mojo / flash-video-controls — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +1 s, load done at +6 s, 5 content frame
    WAIT ok 13-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 14-text video = [video t=2.5 dropped=18 fps=19.5822454308094]
    WAIT ok 15-content m
    CHECK ok 16-sound: sound at the output sink: peak 1292 rms 253 frames 45056 hz 147 env 278,870,858,1292,1085,1132, the speaker amp on, the media volume 23
    WAIT ok 18-text vctl = [vctl pause t=9.926]
    WAIT ok 19-still m
    CHECK ok 21-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0 env 0,0,0,0,0,0
    CHECK ok 22-region-equal: region 40,114-440,412: colours 1925 hash 8619563e rows 298 uniform 0 light none blue none = the sample's hash
    WAIT ok 24-text vctl = [vctl resume t=9.926]
    WAIT ok 25-differs m
    CHECK ok 26-sound: sound at the output sink: peak 6027 rms 1499 frames 45056 hz 80 env 2948,3581,2830,4896,6027,3740, the speaker amp on, the media volume 23
    WAIT ok 29-text vctl = [vctl seek 40 t=21.75]
    WAIT ok 30-differs m
    WAIT ok 31-text video = [video t=44.875 dropped=192 fps=18.633540372670808]
    CHECK ok 32-number: vtime = 46.875 (wanted 38..58)
    WAIT ok 33-text vend = [vend NetStream.Play.Stop t=86.833 after video t=86.791 dropped=534 fps=14.414414414414415]
    CHECK ok 34-number: vendtime = 86.791 (wanted 84.3..87.3)
    CHECK ok 35-plugin: plugin-container 5864: alive (state S, cpu ticks 2061 to 2072)

### mojo / flash-video-flowplayer — pass

    mojo card on http://192.168.1.245:8097/title/i004-vp6a: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 10-rect m
    WAIT ok 11-content m
    CHECK ok 12-flash-rate: content 0,74-478,502: 5.0 s, flips 74 (14.8/s), changes 74 (14.8/s), reads 75, distinct 75, spacing median 68 p90 82 max 251 ms (wanted 5..40/s)
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-drawn: drawn: region 0,74-478,502: colours 2814 hash ea7a192a rows 428 uniform 95 light 74-501 blue none
    WAIT ok 15-still m
    CHECK ok 16-flash-drawn: drawn: region 0,74-478,502: colours 4039 hash a7504443 rows 428 uniform 103 light 74-474 blue none
    CHECK ok 17-plugin: plugin-container 4135: alive (state S, cpu ticks 816 to 829)

### mojo / flash-video-h264-240 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-240.flv: connected at +1 s, load done at +6 s, 6 content frames at +7 s; th
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=7 w=426 h=240 d=20.025]
    WAIT ok 15-text video = [video t=3.605 dropped=25 fps=18.65671641791045]
    CHECK ok 16-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 6fc0aa1c rows 298 uniform 0 light none blue none
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3896 rms 2750 frames 45056 hz 440 env 3896,3895,3895,3896,3896,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,114-440,412: 10.0 s, flips 183 (18.3/s), changes 165 (16.5/s), reads 184, distinct 166, spacing median 45 p90 88 max 187 ms (wanted 11.2..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 39609185 rows 298 uniform 0 light none blue none
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=18.939 dropped=131 fps=28.957528957528957]
    CHECK ok 23-number: vendtime = 18.939 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 131 (wanted 0..480)
    CHECK ok 25-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 0d584e18 rows 298 uniform 0 light none blue none
    CHECK ok 26-plugin: plugin-container 10008: alive (state R, cpu ticks 896 to 907)

### mojo / flash-video-h264-360 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-360.flv: connected at +1 s, load done at +6 s, 4 content frames at +6 s; th
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=7 w=640 h=360 d=20.025]
    WAIT ok 15-text video = [video t=2.525 dropped=17 fps=24.875621890547265]
    CHECK ok 16-flash-drawn: drawn: region 40,114-440,412: colours 3439 hash e5bac07c rows 298 uniform 0 light none blue 114-411
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3897,3896,3895,3895,3896,3891, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,114-440,412: 10.0 s, flips 190 (19.0/s), changes 149 (14.9/s), reads 191, distinct 150, spacing median 38 p90 107 max 162 ms (wanted 13.4..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,114-440,412: colours 3321 hash c336ab10 rows 298 uniform 0 light none blue 114-411
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.957 dropped=104 fps=21.09704641350211]
    CHECK ok 23-number: vendtime = 19.957 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 104 (wanted 0..480)
    CHECK ok 25-flash-drawn: drawn: region 40,114-440,412: colours 3678 hash 37e5a9c6 rows 298 uniform 0 light none blue 114-411
    CHECK ok 26-plugin: plugin-container 11557: alive (state S, cpu ticks 767 to 778)

### mojo / flash-video-h264-480 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-h264-480.flv: connected at +1 s, load done at +6 s, 6 content frames at +6 s; th
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=7 w=854 h=480 d=20.025]
    WAIT ok 15-text video = [video t=1.75 dropped=13 fps=10.00667111407605]
    CHECK ok 16-flash-drawn: drawn: region 40,114-440,412: colours 3048 hash 3012e08e rows 298 uniform 0 light none blue 114-411
    CHECK ok 17-audio: the movie's audio: open running audible
    CHECK ok 18-sound: sound at the output sink: peak 3898 rms 2750 frames 45056 hz 440 env 3897,3897,3898,3897,3896,3892, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 19-flash-rate: content 40,114-440,412: 10.0 s, flips 169 (16.9/s), changes 155 (15.5/s), reads 170, distinct 156, spacing median 47 p90 101 max 179 ms (wanted 9.3..40/s)
    CHECK ok 20-flash-drawn: drawn: region 40,114-440,412: colours 2957 hash fb35047f rows 298 uniform 0 light none blue 114-411
    CHECK ok 21-audio: the movie's audio: open running audible
    WAIT ok 22-text vend = [vend NetStream.Play.Stop t=20.01 after video t=20.01 dropped=161 fps=13.698630136986301]
    CHECK ok 23-number: vendtime = 20.01 (wanted 17.5..20.5)
    CHECK ok 24-number: venddropped = 161 (wanted 0..480)
    CHECK ok 25-flash-drawn: drawn: region 40,114-440,412: colours 3256 hash f5d747a9 rows 298 uniform 0 light none blue 114-411
    CHECK ok 26-plugin: plugin-container 14901: alive (state S, cpu ticks 981 to 992)

### mojo / flash-video-h264-mp4 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/bbb-h264-360.mp4: connected at +1 s, load done at +6 s, 8 content frames at +6 s; th
    CHECK ok 13-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 14-text vmeta = [vmeta codec=avc1 w=640 h=360 d=10]
    WAIT ok 15-text video = [video t=1.067 dropped=9 fps=0]
    CHECK ok 16-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 2925917c rows 298 uniform 0 light none blue none
    CHECK ok 17-flash-rate: content 40,114-440,412: 5.0 s, flips 107 (21.4/s), changes 87 (17.4/s), reads 108, distinct 88, spacing median 38 p90 71 max 135 ms (wanted 7.5..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 1b5156d7 rows 298 uniform 0 light none blue none
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=10 after video t=9.567 dropped=38 fps=21.1864406779661]
    CHECK ok 20-number: vendtime = 9.567 (wanted 7.5..10.5)
    CHECK ok 21-number: venddropped = 38 (wanted 0..300)
    CHECK ok 22-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 1b5156d7 rows 298 uniform 0 light none blue none
    CHECK ok 23-plugin: plugin-container 18355: alive (state S, cpu ticks 415 to 424)

### mojo / flash-video-mediaelement — pass

    mojo card on http://192.168.1.245:8097/title/bbb-h264-360: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    CHECK ok 10-decoder: the kernel's video decoder: no arena fault on this boot
    WAIT ok 11-rect m
    WAIT ok 12-content m
    CHECK ok 13-flash-rate: content 0,74-640,432: 5.0 s, flips 142 (28.4/s), changes 125 (25.0/s), reads 143, distinct 126, spacing median 32 p90 43 max 113 ms (wanted 5..40/s)
    CHECK ok 14-flash-drawn: drawn: region 0,74-640,432: colours 4096 hash 2453734f rows 358 uniform 0 light none blue none
    WAIT ok 15-still m
    CHECK ok 16-flash-drawn: drawn: region 0,74-640,432: colours 4096 hash 2453734f rows 358 uniform 0 light none blue none
    CHECK ok 17-plugin: plugin-container 21592: alive (state S, cpu ticks 457 to 465)

### mojo / flash-video-slow — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/slow/40000/500000/10/video/gen-spark-240.flv: connected at +1 s, load done at +6 s, 7 cont
    WAIT ok 11-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 12-text video = [video t=0.025 dropped=0 fps=0]
    CHECK ok 13-flash-drawn: drawn: region 40,114-440,412: colours 3861 hash 05442396 rows 298 uniform 0 light none blue 189-208
    WAIT ok 14-text vstatus = [vstatus NetStream.Buffer.Empty t=4.754]
    CHECK ok 15-plugin: plugin-container 24812: alive (state S, cpu ticks 215 to 231)
    WAIT ok 16-text vstatus = [vstatus NetStream.Buffer.Full t=4.754]
    WAIT ok 18-differs m
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=19.983 after video t=18.965 dropped=74 fps=23.183925811437405]
    CHECK ok 20-number: vendtime = 18.965 (wanted 17.5..20.5)
    CHECK ok 21-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash ea789dc8 rows 298 uniform 0 light none blue none
    CHECK ok 22-plugin: plugin-container 24812: alive (state S, cpu ticks 626 to 638)

### mojo / flash-video-spark-240 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-240.flv: connected at +1 s, load done at +6 s, 6 content frames at +6 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=426 h=240 d=20.025]
    WAIT ok 14-text video = [video t=1.202 dropped=2 fps=0]
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 8576bd13 rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3897,3896,3896,3895,3896,3895, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,114-440,412: 10.0 s, flips 171 (17.1/s), changes 161 (16.1/s), reads 172, distinct 162, spacing median 45 p90 114 max 165 ms (wanted 10.3..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash d53ed227 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.304 dropped=104 fps=22.692889561270803]
    CHECK ok 22-number: vendtime = 19.304 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 104 (wanted 0..480)
    CHECK ok 24-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash ea789dc8 rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 29009: alive (state S, cpu ticks 503 to 513)

### mojo / flash-video-spark-360 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-360.flv: connected at +1 s, load done at +6 s, 8 content frames at +6 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=640 h=360 d=20.025]
    WAIT ok 14-text video = [video t=2.65 dropped=20 fps=21.037868162692845]
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash dc919769 rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3895,3896,3896,3897,3896,3892, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,114-440,412: 10.0 s, flips 169 (16.9/s), changes 159 (15.9/s), reads 172, distinct 160, spacing median 46 p90 110 max 214 ms (wanted 12.8..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 1577af17 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.122 dropped=116 fps=23.58490566037736]
    CHECK ok 22-number: vendtime = 19.122 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 116 (wanted 0..480)
    CHECK ok 24-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash eb1248d8 rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 32418: alive (state S, cpu ticks 619 to 630)

### mojo / flash-video-spark-480 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/gen-spark-480.flv: connected at +1 s, load done at +6 s, 5 content frames at +6 s; t
    WAIT ok 13-text vmeta = [vmeta codec=2 w=854 h=480 d=20.025]
    WAIT ok 14-text video = [video t=1.858 dropped=25 fps=16.556291390728475]
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 3547 hash a86c93fd rows 298 uniform 0 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK ok 17-sound: sound at the output sink: peak 3897 rms 2750 frames 45056 hz 440 env 3896,3897,3895,3896,3895,3893, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    CHECK ok 18-flash-rate: content 40,114-440,412: 10.0 s, flips 162 (16.2/s), changes 154 (15.4/s), reads 165, distinct 155, spacing median 45 p90 122 max 225 ms (wanted 11.5..40/s)
    CHECK ok 19-flash-drawn: drawn: region 40,114-440,412: colours 3759 hash 01cf1db3 rows 298 uniform 0 light none blue none
    CHECK ok 20-audio: the movie's audio: open running audible
    WAIT ok 21-text vend = [vend NetStream.Play.Stop t=19.983 after video t=19.592 dropped=138 fps=24.577572964669738]
    CHECK ok 22-number: vendtime = 19.592 (wanted 17.5..20.5)
    CHECK ok 23-number: venddropped = 138 (wanted 0..480)
    CHECK ok 24-flash-drawn: drawn: region 40,114-440,412: colours 4077 hash b377425f rows 298 uniform 0 light none blue none
    CHECK ok 25-plugin: plugin-container 3616: alive (state S, cpu ticks 744 to 756)

### mojo / flash-video-spark-sheep — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/sheep-spark.flv: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the
    WAIT ok 13-text video = [video t=1.097 dropped=12 fps=0]
    CHECK ok 14-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash a395f184 rows 298 uniform 33 light 114-236 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,114-440,412: 5.0 s, flips 54 (10.8/s), changes 44 (8.8/s), reads 57, distinct 44, spacing median 55 p90 188 max 379 ms (wanted 6.9..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash da0e1ad8 rows 298 uniform 5 light 114-139 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=8.803 after video t=8.803 dropped=123 fps=16.243654822335024]
    CHECK ok 20-number: vendtime = 8.803 (wanted 6.3..9.3)
    CHECK ok 21-number: venddropped = 123 (wanted 0..220)
    CHECK ok 22-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 7043f561 rows 298 uniform 5 light 114-139 blue none
    CHECK ok 23-plugin: plugin-container 7003: alive (state S, cpu ticks 297 to 309)

### mojo / flash-video-spark-small — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/zelda-spark.flv: connected at +1 s, load done at +7 s, 11 content frames at +7 s; th
    WAIT ok 13-text video = [video t=2.693 dropped=4 fps=11.644832605531295]
    CHECK ok 14-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash adf5c6a0 rows 298 uniform 0 light 114-273 blue none
    CHECK ok 15-audio: the movie's audio: open running audible
    CHECK ok 16-flash-rate: content 40,114-440,412: 10.0 s, flips 123 (12.3/s), changes 122 (12.2/s), reads 125, distinct 123, spacing median 76 p90 102 max 288 ms (wanted 7.7..40/s)
    CHECK ok 17-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash a3dfa453 rows 298 uniform 0 light 194-353 blue none
    CHECK ok 18-audio: the movie's audio: open running audible
    WAIT ok 19-text vend = [vend NetStream.Play.Stop t=29.75 after video t=29.442 dropped=38 fps=12.232415902140673]
    CHECK ok 20-number: vendtime = 29.442 (wanted 27.2..30.2)
    CHECK ok 21-number: venddropped = 38 (wanted 0..354)
    CHECK ok 22-flash-drawn: drawn: region 40,114-440,412: colours 3717 hash 5348d36b rows 298 uniform 0 light 236-411 blue none
    CHECK ok 23-plugin: plugin-container 10592: alive (state S, cpu ticks 411 to 421)

### mojo / flash-video-vp6-176 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/mrandmrs-vp6.flv%26seek=60: connected at +1 s, load done at +7 s, 5 content frames a
    WAIT ok 10-text vmeta = [vmeta codec=4 w=320 h=176 d=151.125]
    WAIT ok 11-text video = [video t=5.093 dropped=13 fps=22.865853658536587]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,114-440,412: 10.0 s, flips 185 (18.5/s), changes 185 (18.5/s), reads 186, distinct 186, spacing median 45 p90 88 max 183 ms (wanted 11.0..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash f3e0aa0a rows 298 uniform 0 light none blue none
    WAIT ok 18-text vctl = [vctl seek 60 t=26.096]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=66.69 dropped=96 fps=23.771790808240887]
    CHECK ok 21-number: vtime = 67.813 (wanted 58..78)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=151.125 after video t=150.282 dropped=486 fps=25.423728813559322]
    CHECK ok 24-number: vendtime = 150.282 (wanted 148.6..151.6)
    CHECK ok 25-plugin: plugin-container 13734: alive (state S, cpu ticks 2365 to 2376)

### mojo / flash-video-vp6-272 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/harrypotter-vp6.flv%26seek=40: connected at +2 s, load done at +7 s, 6 content frame
    WAIT ok 10-text vmeta = [vmeta codec=4 w=480 h=272 d=86.833]
    WAIT ok 11-text video = [video t=3.892 dropped=34 fps=22.22222222222222]
    WAIT ok 12-content m
    CHECK ok 13-audio: the movie's audio: open running audible
    CHECK ok 14-flash-rate: content 40,114-440,412: 10.0 s, flips 194 (19.4/s), changes 193 (19.3/s), reads 195, distinct 194, spacing median 42 p90 79 max 176 ms (wanted 7.1..40/s)
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 4096 hash 6c7571f3 rows 298 uniform 0 light none blue 200-260
    WAIT ok 18-text vctl = [vctl seek 40 t=24.166]
    WAIT ok 19-differs m
    WAIT ok 20-text video = [video t=44.125 dropped=173 fps=22.970903522205205]
    CHECK ok 21-number: vtime = 44.125 (wanted 38..58)
    CHECK ok 22-audio: the movie's audio: open running audible
    WAIT ok 23-text vend = [vend NetStream.Play.Stop t=86.833 after video t=85.942 dropped=509 fps=25]
    CHECK ok 24-number: vendtime = 85.942 (wanted 84.3..87.3)
    CHECK ok 25-plugin: plugin-container 18487: alive (state S, cpu ticks 2120 to 2132)

### mojo / flash-video-vp6a — fail

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&vars=video=http://192.168.1.245:8097/video/i004-vp6a.flv: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the G
    WAIT ok 13-text vmeta = [vmeta codec=5 w=478 h=430 d=14]
    WAIT ok 14-text video = [video t=2.69 dropped=6 fps=14.071294559099437]
    CHECK ok 15-flash-drawn: drawn: region 40,114-440,412: colours 2457 hash 2a40f2f0 rows 298 uniform 67 light none blue none
    CHECK ok 16-audio: the movie's audio: open running audible
    CHECK FAIL 17-flash-rate: content 40,114-440,412: 10.0 s, flips 115 (11.5/s), changes 94 (9.4/s), reads 122, distinct 93, spacing median 67 p90 178 max 277 ms (wanted 9.8..40/s)
    CHECK ok 18-flash-drawn: drawn: region 40,114-440,412: colours 1495 hash 9ca6657e rows 298 uniform 67 light none blue none
    CHECK ok 19-audio: the movie's audio: open running audible
    WAIT ok 20-text vend = [vend NetStream.Play.Stop t=13.975 after video t=13.374 dropped=48 fps=14.638609332113449]
    CHECK ok 21-number: vendtime = 13.374 (wanted 11.5..14.5)
    CHECK ok 22-number: venddropped = 48 (wanted 0..209)
    CHECK ok 23-flash-drawn: drawn: region 40,114-440,412: colours 1495 hash 9ca6657e rows 298 uniform 67 light none blue none
    CHECK ok 24-plugin: plugin-container 22635: alive (state S, cpu ticks 544 to 555)

