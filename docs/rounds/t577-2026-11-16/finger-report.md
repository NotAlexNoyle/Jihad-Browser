# Finger suite report — 2026-11-16T14:47:37

First run 2026-11-16T13:19:23; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 1. 5035 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| enyo | flash-app-calculator | **pass** | 9 ok, 0 failed | CHECK ok 33-region-equal: region 165,181-397,237: colours 131 hash 175995c6 rows 56 uniform 0 light none blue 183-236 = the sample's hash; frames: kept/enyo-flash-app-calculator-keyed-15_.jpg, kept/enyo-flash-app-calculator-sum_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-chart (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 17-text last = [click 4 May series 0]; frames: kept/enyo-flash-app-chart-chart_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-clock | **pass** | 5 ok, 0 failed | CHECK ok 15-region-changed: region 0,83-300,381: colours 1348 hash 29cc2d0c rows 298 uniform 25 light 215-216 blue 101-364 against the sample's hash c99c6107 | cavekit-flash.md R7 |
| enyo | flash-app-drums | **pass** | 6 ok, 0 failed | CHECK ok 22-flash-drawn: drawn: region 51,338-222,449: colours 74 hash 7bd0a9f1 rows 111 uniform 37 light 375-448 blue none; frames: kept/enyo-flash-app-drums-players_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-mp3 | **pass** | 5 ok, 0 failed | CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 | cavekit-flash.md R7 |
| enyo | flash-app-paint (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 22-region-equal: region 95,287-200,320: colours 5 hash ab012dda rows 33 uniform 21 light 287-319 blue none = the sample's hash; frames: kept/enyo-flash-app-paint-poster_.jpg, kept/enyo-flash-app-paint-painted_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-phonemic | **pass** | 8 ok, 0 failed | CHECK ok 25-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 | cavekit-flash.md R7 |
| enyo | flash-app-piano | **pass** | 7 ok, 0 failed | CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1704 frames 45056 hz 521, the speaker amp on, the media volume 23; frames: kept/enyo-flash-app-piano-key_.jpg | cavekit-flash.md R7 |
| enyo | flash-app-postcards | **pass** | 11 ok, 0 failed | CHECK ok 26-flash-drawn: drawn: region 269,370-499,543: colours 272 hash e51c4611 rows 173 uniform 71 light 412-519 blue none; frames: kept/enyo-flash-app-postcards-forward_.jpg | cavekit-flash.md R7 |
| enyo | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/enyo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| enyo | flash-sound | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2956 rms 2049 frames 45056 hz 436, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| enyo | flash-text | **pass** | 8 ok, 0 failed | CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R7 |
| enyo | volume-keys | **pass** | 11 ok, 0 failed | CHECK ok 31-sound: sound at the output sink: peak 2954 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| mochi | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| mochi | flash-app-calculator | **pass** | 9 ok, 0 failed | CHECK ok 33-region-equal: region 165,177-397,233: colours 128 hash 6a298bcf rows 56 uniform 0 light none blue 179-232 = the sample's hash; frames: kept/mochi-flash-app-calculator-keyed-15_.jpg, kept/mochi-flash-app-calculator-sum_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-chart (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 17-text last = [click 4 May series 0]; frames: kept/mochi-flash-app-chart-chart_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-clock | **pass** | 5 ok, 0 failed | CHECK ok 15-region-changed: region 0,79-300,377: colours 1345 hash fe55eb5c rows 298 uniform 28 light 211-212 blue 97-360 against the sample's hash 85181fdf | cavekit-flash.md R7 |
| mochi | flash-app-drums | **pass** | 6 ok, 0 failed | CHECK ok 22-flash-drawn: drawn: region 51,334-222,445: colours 115 hash 25236ca7 rows 111 uniform 37 light 371-444 blue none; frames: kept/mochi-flash-app-drums-players_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-mp3 | **pass** | 5 ok, 0 failed | CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 | cavekit-flash.md R7 |
| mochi | flash-app-paint (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 22-region-equal: region 95,283-200,316: colours 11 hash c0bb1138 rows 33 uniform 21 light 283-315 blue none = the sample's hash; frames: kept/mochi-flash-app-paint-poster_.jpg, kept/mochi-flash-app-paint-painted_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-phonemic | **pass** | 8 ok, 0 failed | CHECK ok 25-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 | cavekit-flash.md R7 |
| mochi | flash-app-piano | **pass** | 7 ok, 0 failed | CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1681 frames 45056 hz 533, the speaker amp on, the media volume 23; frames: kept/mochi-flash-app-piano-key_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-postcards | **pass** | 11 ok, 0 failed | CHECK ok 26-flash-drawn: drawn: region 269,366-499,539: colours 276 hash ef64f90b rows 173 uniform 68 light 408-515 blue 469-514; frames: kept/mochi-flash-app-postcards-forward_.jpg | cavekit-flash.md R7 |
| mochi | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/mochi-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mochi | flash-sound | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| mochi | flash-text | **pass** | 8 ok, 0 failed | CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R7 |
| mochi | volume-keys | **pass** | 11 ok, 0 failed | CHECK ok 31-sound: sound at the output sink: peak 2955 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| mojo | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| mojo | flash-app-calculator | **pass** | 9 ok, 0 failed | CHECK ok 33-region-equal: region 165,172-397,228: colours 115 hash 708ae6c9 rows 56 uniform 0 light none blue 174-227 = the sample's hash; frames: kept/mojo-flash-app-calculator-keyed-15_.jpg, kept/mojo-flash-app-calculator-sum_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-chart (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 17-text last = [click 4 May series 0]; frames: kept/mojo-flash-app-chart-chart_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-clock | **pass** | 5 ok, 0 failed | CHECK ok 15-region-changed: region 0,74-300,372: colours 1116 hash 26847597 rows 298 uniform 38 light 206-207 blue 92-355 against the sample's hash 4ddd93ac | cavekit-flash.md R7 |
| mojo | flash-app-drums | **pass** | 6 ok, 0 failed | CHECK ok 22-flash-drawn: drawn: region 51,329-222,440: colours 49 hash 65c12f83 rows 111 uniform 37 light 366-439 blue none; frames: kept/mojo-flash-app-drums-players_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-mp3 | **pass** | 5 ok, 0 failed | CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 | cavekit-flash.md R7 |
| mojo | flash-app-paint (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 22-region-equal: region 95,278-200,311: colours 5 hash ab012dda rows 33 uniform 21 light 278-310 blue none = the sample's hash; frames: kept/mojo-flash-app-paint-poster_.jpg, kept/mojo-flash-app-paint-painted_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-phonemic | **pass** | 8 ok, 0 failed | CHECK ok 25-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0 | cavekit-flash.md R7 |
| mojo | flash-app-piano | **pass** | 7 ok, 0 failed | CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1735 frames 45056 hz 501, the speaker amp on, the media volume 23; frames: kept/mojo-flash-app-piano-key_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-postcards | **pass** | 11 ok, 0 failed | CHECK ok 26-flash-drawn: drawn: region 269,361-499,534: colours 178 hash d85d5257 rows 173 uniform 68 light 403-510 blue 404-509; frames: kept/mojo-flash-app-postcards-forward_.jpg | cavekit-flash.md R7 |
| mojo | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/mojo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mojo | flash-sound | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2955 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| mojo | flash-text | **pass** | 8 ok, 0 failed | CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R7 |
| mojo | volume-keys | **pass** | 11 ok, 0 failed | CHECK ok 31-sound: sound at the output sink: peak 2961 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| enyo | ddg-results (rerun) | **pass** | 3 ok, 0 failed | CHECK ok 10-content: region 0,100-756,900: colours 1689 hash 89c1489e rows 800 uniform 160 light 100-899 blue 148-152; sel:body on the screen at 0,82-768,2007 | cavekit-modern-web.md R2 |
| enyo | everyday-browsing (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 17-url: https://duckduckgo.com/ | cavekit-engine-update.md R8 |
| enyo | flash-tags (rerun) | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 55 (18.3/s), changes 41 (13.7/s), reads 56, distinct 42, spacing median 62 p90 74 max 103 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mochi | ddg-results (rerun) | **pass** | 3 ok, 0 failed | CHECK ok 10-content: region 0,100-756,900: colours 1568 hash 32e4b9ad rows 800 uniform 162 light 100-899 blue 144-148; sel:body on the screen at 0,78-768,1987 | cavekit-modern-web.md R2 |
| mochi | everyday-browsing (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 17-url: https://duckduckgo.com/ | cavekit-engine-update.md R8 |
| mochi | flash-tags (rerun) | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 44 (14.7/s), changes 37 (12.3/s), reads 46, distinct 38, spacing median 65 p90 94 max 230 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mojo | ddg-results (rerun) | **pass** | 3 ok, 0 failed | CHECK ok 10-content: region 0,100-756,900: colours 1186 hash 56f5cde4 rows 800 uniform 155 light 100-899 blue 139-142; sel:body on the screen at 0,73-768,1966 | cavekit-modern-web.md R2 |
| mojo | everyday-browsing (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 17-url: https://duckduckgo.com/ | cavekit-engine-update.md R8 |
| mojo | flash-tags (rerun) | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 50 (16.7/s), changes 39 (13.0/s), reads 51, distinct 40, spacing median 67 p90 78 max 93 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| enyo | flash-app-form (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer; frames: kept/enyo-flash-app-form-asked_.jpg | cavekit-flash.md R7 |
| mochi | flash-app-form (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer; frames: kept/mochi-flash-app-form-asked_.jpg | cavekit-flash.md R7 |
| mojo | flash-app-form (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer; frames: kept/mojo-flash-app-form-asked_.jpg | cavekit-flash.md R7 |
| enyo | flash-clipboard-out (rerun) | **fail** | 8 ok, 1 failed | CHECK FAIL 21-wait-text: pfv never read pf jihad-clip-7391 within the bound; frames: kept/enyo-flash-clipboard-out-menu_.jpg, kept/enyo-flash-clipboard-out-21-wait-text_.jpg | cavekit-flash.md R7 |
| mochi | flash-clipboard-out (rerun) | **fail** | 8 ok, 1 failed | CHECK FAIL 21-wait-text: pfv never read pf jihad-clip-7391 within the bound; frames: kept/mochi-flash-clipboard-out-menu_.jpg, kept/mochi-flash-clipboard-out-21-wait-text_.jpg | cavekit-flash.md R7 |
| mojo | flash-clipboard-out (rerun) | **fail** | 8 ok, 1 failed | CHECK FAIL 21-wait-text: pfv never read pf jihad-clip-7391 within the bound; frames: kept/mojo-flash-clipboard-out-menu_.jpg, kept/mojo-flash-clipboard-out-21-wait-text_.jpg | cavekit-flash.md R7 |

## The device at the end

```
DEVICE ok job-enyo: (382736/860470601) jihad (start) running, process 5027
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (382742/296260082) jihad-mochi (start) running, process 9278
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (382747/184106748) jihad-mojo (start) running, process 13367
DEVICE ok daemons-mojo: one daemon
DEVICE ok hand-daemon-mojo: none
DEVICE ok hooks-mojo: none
DEVICE ok adapter-gates-mojo: none
DEVICE ok proxy-mojo: network.proxy.type unset
DEVICE ok history-mojo: browser.sessionhistory.max_entries the default
DEVICE ok cards-mojo: none
DEVICE ok audio-lib-mojo: the device's own
DEVICE ok look-mojo: {"returnValue":true,"theme":null}
DEVICE ok gate-files: none in /tmp
DEVICE ok device-scripts: none left in /tmp
DEVICE ok test-pads-enyo: none
DEVICE ok dialog-pipes-enyo: none
DEVICE ok test-pads-mochi: none
DEVICE ok dialog-pipes-mochi: none
DEVICE ok test-pads-mojo: none
DEVICE ok dialog-pipes-mojo: none
DEVICE ok tmp-dumps: none
DEVICE ok media-scratch: none
DEVICE ok overall: 16% of /tmp used
```

## Every test's lines

### enyo / audio-heard — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31139 rms 21985 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### enyo / flash-app-calculator — pass

    enyo card on http://192.168.1.245:8097/title/cyberchase-calculator: connected at +2 s, load done at +7 s, 10 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the jo
    WAIT ok 10-rect m
    WAIT ok 11-still m@550,550:165,99,397,155
    WAIT ok 13-still m@550,550:165,99,397,155
    WAIT ok 18-still m@550,550:165,99,397,155
    CHECK ok 20-region-changed: region 165,181-397,237: colours 131 hash 175995c6 rows 56 uniform 0 light none blue 183-236 against the sample's hash 44702382
    WAIT ok 23-still m@550,550:165,99,397,155
    CHECK ok 24-region-equal: region 165,181-397,237: colours 131 hash 44702382 rows 56 uniform 0 light none blue 183-236 = the sample's hash
    WAIT ok 32-still m@550,550:165,99,397,155
    CHECK ok 33-region-equal: region 165,181-397,237: colours 131 hash 175995c6 rows 56 uniform 0 light none blue 183-236 = the sample's hash

### enyo / flash-app-chart — pass

    enyo card on http://192.168.1.245:8097/title/yui-charts: connected at +2 s, load done at +8 s, 4 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 11-rect m
    WAIT ok 12-text last = [ready]
    CHECK ok 13-flash-drawn: drawn: region 0,83-500,456: colours 107 hash 88958c2d rows 373 uniform 27 light 83-455 blue 140-427
    WAIT ok 15-text last = [click 2 Mar series 0]
    WAIT ok 17-text last = [click 4 May series 0]

### enyo / flash-app-clock — pass

    enyo card on http://192.168.1.245:8097/title/flash-clock: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 9-rect m
    CHECK ok 10-flash-drawn: drawn: region 0,83-300,381: colours 1340 hash 27a120e6 rows 298 uniform 25 light 215-216 blue 101-364
    WAIT ok 12-differs m
    WAIT ok 14-differs m
    CHECK ok 15-region-changed: region 0,83-300,381: colours 1348 hash 29cc2d0c rows 298 uniform 25 light 215-216 blue 101-364 against the sample's hash c99c6107

### enyo / flash-app-drums — pass

    enyo card on http://192.168.1.245:8097/title/drum-machine: connected at +2 s, load done at +7 s, 26 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 10-rect m
    WAIT ok 11-still m@900,450:60,300,260,430
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 51,338-222,449: colours 1 hash a2e425f4 rows 111 uniform 111 light 338-448 blue none
    WAIT ok 20-differs m@900,450:60,300,260,430
    CHECK ok 21-sound-watched: sound at the output sink over the gesture: peak 32768 rms 5337 frames 397312 hz 285, the speaker amp on, the media volume 23
    CHECK ok 22-flash-drawn: drawn: region 51,338-222,449: colours 74 hash 7bd0a9f1 rows 111 uniform 37 light 375-448 blue none

### enyo / flash-app-mp3 — pass

    enyo card on http://192.168.1.245:8097/title/mp3-player-multi: connected at +2 s, load done at +8 s, 16 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect m
    WAIT ok 10-still m
    CHECK ok 12-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0
    CHECK ok 15-sound-watched: sound at the output sink over the gesture: peak 1820 rms 208 frames 353280 hz 482, the speaker amp on, the media volume 23
    CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0

### enyo / flash-app-paint — pass

    enyo card on http://192.168.1.245:8097/title/poster-paint-party: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 10-rect m
    WAIT ok 11-still m
    WAIT ok 13-still m
    WAIT ok 20-differs m@480,320:150,40,270,90
    CHECK ok 21-region-changed: region 150,122-270,172: colours 82 hash 7871ec3e rows 50 uniform 8 light 122-171 blue 127-158 against the sample's hash 42ffb962
    CHECK ok 22-region-equal: region 95,287-200,320: colours 5 hash ab012dda rows 33 uniform 21 light 287-319 blue none = the sample's hash

### enyo / flash-app-phonemic — pass

    enyo card on http://192.168.1.245:8097/title/phonemic-chart: connected at +4 s, load done at +9 s, 10 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 10-rect m
    WAIT ok 11-still m
    CHECK ok 12-flash-drawn: drawn: region 0,83-654,652: colours 3877 hash 6f903e29 rows 569 uniform 0 light 116-634 blue none
    CHECK ok 14-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0
    CHECK ok 17-sound-watched: sound at the output sink over the gesture: peak 7524 rms 282 frames 177152 hz 793, the speaker amp on, the media volume 23
    CHECK ok 20-sound-watched: sound at the output sink over the gesture: peak 6688 rms 157 frames 177152 hz 1901, the speaker amp on, the media volume 23
    CHECK ok 23-sound-watched: sound at the output sink over the gesture: peak 1666 rms 174 frames 177152 hz 259, the speaker amp on, the media volume 23
    CHECK ok 25-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0

### enyo / flash-app-piano — pass

    enyo card on http://192.168.1.245:8097/title/virtual-piano: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 9-rect m
    WAIT ok 10-still m
    WAIT ok 12-still m
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    CHECK ok 15-sound: sound at the output sink: peak 19426 rms 1900 frames 45056 hz 647, the speaker amp on, the media volume 23
    CHECK ok 17-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1704 frames 45056 hz 521, the speaker amp on, the media volume 23

### enyo / flash-app-postcards — pass

    enyo card on http://192.168.1.245:8097/title/postcard-viewer: connected at +2 s, load done at +8 s, 19 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 269,370-499,543: colours 237 hash 607e011b rows 173 uniform 71 light 412-519 blue none
    CHECK ok 14-flash-drawn: reported not drawn, as it should be: region 96,140-672,293: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    WAIT ok 17-differs m
    WAIT ok 18-still m
    CHECK ok 20-flash-drawn: drawn: region 96,140-672,293: colours 23 hash 4e10b9b0 rows 153 uniform 58 light 217-225 blue 140-215
    WAIT ok 23-differs m
    WAIT ok 24-still m
    CHECK ok 25-flash-drawn: reported not drawn, as it should be: region 96,140-672,293: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    CHECK ok 26-flash-drawn: drawn: region 269,370-499,543: colours 272 hash e51c4611 rows 173 uniform 71 light 412-519 blue none

### enyo / flash-game-sound — pass

    enyo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +3 s, load done at +9 s, 5 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### enyo / flash-sound — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31138 rms 21984 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2956 rms 2049 frames 45056 hz 436, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### enyo / flash-text — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-text text = [text Hello]
    WAIT ok 17-text text = [text Hello World 123]
    WAIT ok 19-text text = [text Hello World x9]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / volume-keys — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +7 s, 18 content frames at +7 s; the GPU path, frames off the main thread
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2955 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume-hud: the window server raised its alert for the key (syslog: void LockWindow::slotTransientAlertActivated(): has displayable content); it draws no HUD in the frame buffer on this build
    CHECK ok 17-volume: the media volume 66, higher (marked 44)
    CHECK ok 18-sound: sound at the output sink: peak 2958 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 21-volume: the media volume 44, lower (marked 66)
    CHECK ok 22-sound: sound at the output sink: peak 2963 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 26-volume: the media volume 0, zero (marked 66)
    CHECK ok 27-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2958 rms 2060 frames 45056 hz 440): no sound out
    CHECK ok 30-volume: the media volume 22, higher (marked 0)
    CHECK ok 31-sound: sound at the output sink: peak 2954 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### mochi / audio-heard — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31141 rms 21985 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mochi / flash-app-calculator — pass

    mochi card on http://192.168.1.245:8097/title/cyberchase-calculator: connected at +2 s, load done at +8 s, 15 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the j
    WAIT ok 10-rect m
    WAIT ok 11-still m@550,550:165,99,397,155
    WAIT ok 13-still m@550,550:165,99,397,155
    WAIT ok 18-still m@550,550:165,99,397,155
    CHECK ok 20-region-changed: region 165,177-397,233: colours 128 hash 6a298bcf rows 56 uniform 0 light none blue 179-232 against the sample's hash fb4ff083
    WAIT ok 23-still m@550,550:165,99,397,155
    CHECK ok 24-region-equal: region 165,177-397,233: colours 128 hash fb4ff083 rows 56 uniform 0 light none blue 179-232 = the sample's hash
    WAIT ok 32-still m@550,550:165,99,397,155
    CHECK ok 33-region-equal: region 165,177-397,233: colours 128 hash 6a298bcf rows 56 uniform 0 light none blue 179-232 = the sample's hash

### mochi / flash-app-chart — pass

    mochi card on http://192.168.1.245:8097/title/yui-charts: connected at +1 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 11-rect m
    WAIT ok 12-text last = [ready]
    CHECK ok 13-flash-drawn: drawn: region 0,79-500,452: colours 107 hash 88958c2d rows 373 uniform 27 light 79-451 blue 136-423
    WAIT ok 15-text last = [click 2 Mar series 0]
    WAIT ok 17-text last = [click 4 May series 0]

### mochi / flash-app-clock — pass

    mochi card on http://192.168.1.245:8097/title/flash-clock: connected at +2 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 9-rect m
    CHECK ok 10-flash-drawn: drawn: region 0,79-300,377: colours 1347 hash 24299cea rows 298 uniform 28 light 211-212 blue 97-360
    WAIT ok 12-differs m
    WAIT ok 14-differs m
    CHECK ok 15-region-changed: region 0,79-300,377: colours 1345 hash fe55eb5c rows 298 uniform 28 light 211-212 blue 97-360 against the sample's hash 85181fdf

### mochi / flash-app-drums — pass

    mochi card on http://192.168.1.245:8097/title/drum-machine: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 10-rect m
    WAIT ok 11-still m@900,450:60,300,260,430
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 51,334-222,445: colours 1 hash a2e425f4 rows 111 uniform 111 light 334-444 blue none
    WAIT ok 20-differs m@900,450:60,300,260,430
    CHECK ok 21-sound-watched: sound at the output sink over the gesture: peak 9529 rms 1897 frames 397312 hz 738, the speaker amp on, the media volume 23
    CHECK ok 22-flash-drawn: drawn: region 51,334-222,445: colours 115 hash 25236ca7 rows 111 uniform 37 light 371-444 blue none

### mochi / flash-app-mp3 — pass

    mochi card on http://192.168.1.245:8097/title/mp3-player-multi: connected at +1 s, load done at +7 s, 17 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 9-rect m
    WAIT ok 10-still m
    CHECK ok 12-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0
    CHECK ok 15-sound-watched: sound at the output sink over the gesture: peak 1820 rms 203 frames 353280 hz 395, the speaker amp on, the media volume 23
    CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0

### mochi / flash-app-paint — pass

    mochi card on http://192.168.1.245:8097/title/poster-paint-party: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's
    WAIT ok 10-rect m
    WAIT ok 11-still m
    WAIT ok 13-still m
    WAIT ok 20-differs m@480,320:150,40,270,90
    CHECK ok 21-region-changed: region 150,118-270,168: colours 84 hash 54dc37d8 rows 50 uniform 6 light 118-167 blue 123-154 against the sample's hash d0397bfa
    CHECK ok 22-region-equal: region 95,283-200,316: colours 11 hash c0bb1138 rows 33 uniform 21 light 283-315 blue none = the sample's hash

### mochi / flash-app-phonemic — pass

    mochi card on http://192.168.1.245:8097/title/phonemic-chart: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 10-rect m
    WAIT ok 11-still m
    CHECK ok 12-flash-drawn: drawn: region 0,79-654,648: colours 3877 hash 6f903e29 rows 569 uniform 0 light 112-630 blue none
    CHECK ok 14-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0
    CHECK ok 17-sound-watched: sound at the output sink over the gesture: peak 7524 rms 282 frames 177152 hz 793, the speaker amp on, the media volume 23
    CHECK ok 20-sound-watched: sound at the output sink over the gesture: peak 6688 rms 157 frames 177152 hz 1901, the speaker amp on, the media volume 23
    CHECK ok 23-sound-watched: sound at the output sink over the gesture: peak 1666 rms 174 frames 177152 hz 259, the speaker amp on, the media volume 23
    CHECK ok 25-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0

### mochi / flash-app-piano — pass

    mochi card on http://192.168.1.245:8097/title/virtual-piano: connected at +2 s, load done at +8 s, 14 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 9-rect m
    WAIT ok 10-still m
    WAIT ok 12-still m
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    CHECK ok 15-sound: sound at the output sink: peak 19426 rms 1890 frames 45056 hz 663, the speaker amp on, the media volume 23
    CHECK ok 17-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1681 frames 45056 hz 533, the speaker amp on, the media volume 23

### mochi / flash-app-postcards — pass

    mochi card on http://192.168.1.245:8097/title/postcard-viewer: connected at +2 s, load done at +8 s, 15 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 269,366-499,539: colours 244 hash 65ec63a8 rows 173 uniform 68 light 408-515 blue 469-514
    CHECK ok 14-flash-drawn: reported not drawn, as it should be: region 96,136-672,289: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    WAIT ok 17-differs m
    WAIT ok 18-still m
    CHECK ok 20-flash-drawn: drawn: region 96,136-672,289: colours 29 hash e79983c9 rows 153 uniform 57 light 212-221 blue none
    WAIT ok 23-differs m
    WAIT ok 24-still m
    CHECK ok 25-flash-drawn: reported not drawn, as it should be: region 96,136-672,289: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    CHECK ok 26-flash-drawn: drawn: region 269,366-499,539: colours 276 hash ef64f90b rows 173 uniform 68 light 408-515 blue 469-514

### mochi / flash-game-sound — pass

    mochi card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +1 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### mochi / flash-sound — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31141 rms 21984 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mochi / flash-text — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +1 s, load done at +7 s, 13 content frames at +7 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 7-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 15-text text = [text Hello]
    WAIT ok 17-text text = [text Hello World 123]
    WAIT ok 19-text text = [text Hello World x9]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / volume-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +7 s, 13 content frames at +7 s; the GPU path, frames off the main threa
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2958 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume-hud: the window server raised its alert for the key (syslog: void LockWindow::slotTransientAlertActivated(): has displayable content); it draws no HUD in the frame buffer on this build
    CHECK ok 17-volume: the media volume 66, higher (marked 44)
    CHECK ok 18-sound: sound at the output sink: peak 2964 rms 2061 frames 45056 hz 440, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 21-volume: the media volume 44, lower (marked 66)
    CHECK ok 22-sound: sound at the output sink: peak 2953 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 26-volume: the media volume 0, zero (marked 66)
    CHECK ok 27-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2958 rms 2058 frames 45056 hz 440): no sound out
    CHECK ok 30-volume: the media volume 22, higher (marked 0)
    CHECK ok 31-sound: sound at the output sink: peak 2955 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### mojo / audio-heard — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31139 rms 21985 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mojo / flash-app-calculator — pass

    mojo card on http://192.168.1.245:8097/title/cyberchase-calculator: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job
    WAIT ok 10-rect m
    WAIT ok 11-still m@550,550:165,99,397,155
    WAIT ok 13-still m@550,550:165,99,397,155
    WAIT ok 18-still m@550,550:165,99,397,155
    CHECK ok 20-region-changed: region 165,172-397,228: colours 115 hash 708ae6c9 rows 56 uniform 0 light none blue 174-227 against the sample's hash 17d55502
    WAIT ok 23-still m@550,550:165,99,397,155
    CHECK ok 24-region-equal: region 165,172-397,228: colours 115 hash 17d55502 rows 56 uniform 0 light none blue 174-227 = the sample's hash
    WAIT ok 32-still m@550,550:165,99,397,155
    CHECK ok 33-region-equal: region 165,172-397,228: colours 115 hash 708ae6c9 rows 56 uniform 0 light none blue 174-227 = the sample's hash

### mojo / flash-app-chart — pass

    mojo card on http://192.168.1.245:8097/title/yui-charts: connected at +1 s, load done at +6 s, 14 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 11-rect m
    WAIT ok 12-text last = [ready]
    CHECK ok 13-flash-drawn: drawn: region 0,74-500,447: colours 96 hash 29da18c5 rows 373 uniform 27 light 74-446 blue 131-418
    WAIT ok 15-text last = [click 2 Mar series 0]
    WAIT ok 17-text last = [click 4 May series 0]

### mojo / flash-app-clock — pass

    mojo card on http://192.168.1.245:8097/title/flash-clock: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 9-rect m
    CHECK ok 10-flash-drawn: drawn: region 0,74-300,372: colours 1119 hash 0d895ed7 rows 298 uniform 38 light 206-207 blue 92-355
    WAIT ok 12-differs m
    WAIT ok 14-differs m
    CHECK ok 15-region-changed: region 0,74-300,372: colours 1116 hash 26847597 rows 298 uniform 38 light 206-207 blue 92-355 against the sample's hash 4ddd93ac

### mojo / flash-app-drums — pass

    mojo card on http://192.168.1.245:8097/title/drum-machine: connected at +1 s, load done at +8 s, 24 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 10-rect m
    WAIT ok 11-still m@900,450:60,300,260,430
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 51,329-222,440: colours 1 hash a2e425f4 rows 111 uniform 111 light 329-439 blue none
    WAIT ok 20-differs m@900,450:60,300,260,430
    CHECK ok 21-sound-watched: sound at the output sink over the gesture: peak 8526 rms 1075 frames 397312 hz 680, the speaker amp on, the media volume 23
    CHECK ok 22-flash-drawn: drawn: region 51,329-222,440: colours 49 hash 65c12f83 rows 111 uniform 37 light 366-439 blue none

### mojo / flash-app-mp3 — pass

    mojo card on http://192.168.1.245:8097/title/mp3-player-multi: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect m
    WAIT ok 10-still m
    CHECK ok 12-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0
    CHECK ok 15-sound-watched: sound at the output sink over the gesture: peak 1820 rms 191 frames 353280 hz 667, the speaker amp on, the media volume 23
    CHECK ok 19-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0

### mojo / flash-app-paint — pass

    mojo card on http://192.168.1.245:8097/title/poster-paint-party: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 10-rect m
    WAIT ok 11-still m
    WAIT ok 13-still m
    WAIT ok 20-differs m@480,320:150,40,270,90
    CHECK ok 21-region-changed: region 150,113-270,163: colours 56 hash 272829a5 rows 50 uniform 8 light 113-162 blue 118-149 against the sample's hash 1b151848
    CHECK ok 22-region-equal: region 95,278-200,311: colours 5 hash ab012dda rows 33 uniform 21 light 278-310 blue none = the sample's hash

### mojo / flash-app-phonemic — pass

    mojo card on http://192.168.1.245:8097/title/phonemic-chart: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 10-rect m
    WAIT ok 11-still m
    CHECK ok 12-flash-drawn: drawn: region 0,74-654,643: colours 3117 hash ed71f967 rows 569 uniform 0 light 107-625 blue none
    CHECK ok 14-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0
    CHECK ok 17-sound-watched: sound at the output sink over the gesture: peak 7524 rms 282 frames 177152 hz 793, the speaker amp on, the media volume 23
    CHECK ok 20-sound-watched: sound at the output sink over the gesture: peak 6688 rms 157 frames 177152 hz 1901, the speaker amp on, the media volume 23
    CHECK ok 23-sound-watched: sound at the output sink over the gesture: peak 1666 rms 167 frames 177152 hz 309, the speaker amp on, the media volume 23
    CHECK ok 25-sound-watched: no sound at the output sink over the gesture: peak 0 rms 0 frames 133120 hz 0

### mojo / flash-app-piano — pass

    mojo card on http://192.168.1.245:8097/title/virtual-piano: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 9-rect m
    WAIT ok 10-still m
    WAIT ok 12-still m
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    CHECK ok 15-sound: sound at the output sink: peak 19426 rms 1951 frames 45056 hz 594, the speaker amp on, the media volume 23
    CHECK ok 17-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    CHECK ok 19-sound: sound at the output sink: peak 12626 rms 1735 frames 45056 hz 501, the speaker amp on, the media volume 23

### mojo / flash-app-postcards — pass

    mojo card on http://192.168.1.245:8097/title/postcard-viewer: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 269,361-499,534: colours 138 hash 248eeb78 rows 173 uniform 68 light 403-510 blue 404-509
    CHECK ok 14-flash-drawn: reported not drawn, as it should be: region 96,131-672,284: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    WAIT ok 17-differs m
    WAIT ok 18-still m
    CHECK ok 20-flash-drawn: drawn: region 96,131-672,284: colours 22 hash 5e5dbe6c rows 153 uniform 47 light 208-217 blue none
    WAIT ok 23-differs m
    WAIT ok 24-still m
    CHECK ok 25-flash-drawn: reported not drawn, as it should be: region 96,131-672,284: colours 1 hash 65667185 rows 153 uniform 153 light none blue none
    CHECK ok 26-flash-drawn: drawn: region 269,361-499,534: colours 178 hash d85d5257 rows 173 uniform 68 light 403-510 blue 404-509

### mojo / flash-game-sound — pass

    mojo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +1 s, load done at +8 s, 16 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### mojo / flash-sound — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31139 rms 21985 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2955 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mojo / flash-text — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 15-text text = [text Hello]
    WAIT ok 17-text text = [text Hello World 123]
    WAIT ok 19-text text = [text Hello World x9]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 22-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / volume-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +7 s, 21 content frames at +7 s; the GPU path, frames off the main thread
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2975 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume-hud: the window server raised its alert for the key (syslog: void LockWindow::slotTransientAlertActivated(): has displayable content); it draws no HUD in the frame buffer on this build
    CHECK ok 17-volume: the media volume 66, higher (marked 44)
    CHECK ok 18-sound: sound at the output sink: peak 2956 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 21-volume: the media volume 44, lower (marked 66)
    CHECK ok 22-sound: sound at the output sink: peak 2958 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 26-volume: the media volume 0, zero (marked 66)
    CHECK ok 27-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2959 rms 2057 frames 45056 hz 440): no sound out
    CHECK ok 30-volume: the media volume 22, higher (marked 0)
    CHECK ok 31-sound: sound at the output sink: peak 2961 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### enyo / ddg-results — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 8-launch: load done
    WAIT ok 9-line: [jihad-bs] titleAndUrl title=[webOS at DuckDuckGo] uri=https://duckduckgo.com/?q=webOS&ia=web back=1 fwd=0
    CHECK ok 10-content: region 0,100-756,900: colours 1689 hash 89c1489e rows 800 uniform 160 light 100-899 blue 148-152; sel:body on the screen at 0,82-768,2007

### enyo / everyday-browsing — pass

    enyo card on https://duckduckgo.com/: connected at +3 s, load done at +34 s, 25 content frames at +34 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:textarea[name='q']
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 440 landed on document y 358)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-launch: load done
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    CHECK ok 15-card-scroll-min: the card's scroll y 844 (at least 300)
    WAIT ok 17-url: https://duckduckgo.com/

### enyo / flash-tags — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 12 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [10.3]
    CHECK ok 11-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [mime yes]
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 23-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 24-launch: load done
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 55 (18.3/s), changes 41 (13.7/s), reads 56, distinct 42, spacing median 62 p90 74 max 103 ms (wanted 5..100/s)

### mochi / ddg-results — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 8-launch: load done
    WAIT ok 9-line: [jihad-bs] titleAndUrl title=[webOS at DuckDuckGo] uri=https://duckduckgo.com/?q=webOS&ia=web back=1 fwd=0
    CHECK ok 10-content: region 0,100-756,900: colours 1568 hash 32e4b9ad rows 800 uniform 162 light 100-899 blue 144-148; sel:body on the screen at 0,78-768,1987

### mochi / everyday-browsing — pass

    mochi card on https://duckduckgo.com/: connected at +3 s, load done at +33 s, 25 content frames at +33 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:textarea[name='q']
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 437 landed on document y 359)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-launch: load done
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    CHECK ok 15-card-scroll-min: the card's scroll y 873 (at least 300)
    WAIT ok 17-url: https://duckduckgo.com/

### mochi / flash-tags — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 12 content frames at +7 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [10.3]
    CHECK ok 11-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [mime yes]
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 23-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 24-launch: load done
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 44 (14.7/s), changes 37 (12.3/s), reads 46, distinct 38, spacing median 65 p90 94 max 230 ms (wanted 5..100/s)

### mojo / ddg-results — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +6 s, 2 content frames at +36 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 8-launch: load done
    WAIT ok 9-line: [jihad-bs] titleAndUrl title=[webOS at DuckDuckGo] uri=https://duckduckgo.com/?q=webOS&ia=web back=1 fwd=0
    CHECK ok 10-content: region 0,100-756,900: colours 1186 hash 56f5cde4 rows 800 uniform 155 light 100-899 blue 139-142; sel:body on the screen at 0,73-768,1966

### mojo / everyday-browsing — pass

    mojo card on https://duckduckgo.com/: connected at +1 s, load done at +30 s, 21 content frames at +30 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:textarea[name='q']
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 436 landed on document y 363)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-launch: load done
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    CHECK ok 15-card-scroll-min: the card's scroll y 808 (at least 300)
    WAIT ok 17-url: https://duckduckgo.com/

### mojo / flash-tags — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [10.3]
    CHECK ok 11-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [mime yes]
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 23-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 24-launch: load done
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 50 (16.7/s), changes 39 (13.0/s), reads 51, distinct 40, spacing median 67 p90 78 max 93 ms (wanted 5..100/s)

### enyo / flash-app-form — pass

    enyo card on http://192.168.1.245:8097/title/website-viewer: connected at +3 s, load done at +9 s, 8 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 0,83-640,561: colours 4096 hash e07ca3aa rows 478 uniform 0 light none blue none
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 18-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 19-still m
    CHECK ok 20-flash-drawn: drawn: region 0,83-640,561: colours 4096 hash edc73612 rows 478 uniform 0 light none blue none
    CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer

### mochi / flash-app-form — pass

    mochi card on http://192.168.1.245:8097/title/website-viewer: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 0,79-640,557: colours 4096 hash c4460238 rows 478 uniform 0 light none blue none
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/mochi/dialog-1.fifo
    CHECK ok 18-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 19-still m
    CHECK ok 20-flash-drawn: drawn: region 0,79-640,557: colours 4096 hash c4460238 rows 478 uniform 0 light none blue none
    CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer

### mojo / flash-app-form — pass

    mojo card on http://192.168.1.245:8097/title/website-viewer: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHA
    WAIT ok 11-rect m
    WAIT ok 12-still m
    CHECK ok 13-flash-drawn: drawn: region 0,74-640,552: colours 4096 hash 82825cf1 rows 478 uniform 0 light none blue none
    CHECK ok 15-dialog: the dialog's reply pipe stands: /var/palm/jihad/mojo/dialog-1.fifo
    CHECK ok 18-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 19-still m
    CHECK ok 20-flash-drawn: drawn: region 0,74-640,552: colours 4096 hash 80ac146e rows 478 uniform 0 light none blue none
    CHECK ok 21-url: load done uri=http://192.168.1.245:8097/title/website-viewer

### enyo / flash-clipboard-out — fail

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text&clip=1: connected at +1 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express l
    WAIT ok 12-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 14-text clip = [clip set]
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] hold at 224,385 editable=1 wasFocused=1
    CHECK FAIL 21-wait-text: pfv never read pf jihad-clip-7391 within the bound
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 25-text file = [file browse false]
    WAIT ok 27-text file = [file download false]
    CHECK ok 28-quiet: no line jihad-dl\|dialog  for 4 s

### mochi / flash-clipboard-out — fail

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text&clip=1: connected at +1 s, load done at +7 s, 13 content frames at +7 s; the GPU path, frames off the main thread (the express
    WAIT ok 12-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 14-text clip = [clip set]
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] hold at 224,385 editable=1 wasFocused=1
    CHECK FAIL 21-wait-text: pfv never read pf jihad-clip-7391 within the bound
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 25-text file = [file browse false]
    WAIT ok 27-text file = [file download false]
    CHECK ok 28-quiet: no line jihad-dl\|dialog  for 4 s

### mojo / flash-clipboard-out — fail

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text&clip=1: connected at +1 s, load done at +7 s, 12 content frames at +7 s; the GPU path, frames off the main thread (the express 
    WAIT ok 12-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 14-text clip = [clip set]
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] hold at 224,385 editable=1 wasFocused=1
    CHECK FAIL 21-wait-text: pfv never read pf jihad-clip-7391 within the bound
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 25-text file = [file browse false]
    WAIT ok 27-text file = [file download false]
    CHECK ok 28-quiet: no line jihad-dl\|dialog  for 4 s

