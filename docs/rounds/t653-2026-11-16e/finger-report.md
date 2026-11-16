# Finger suite report — 2026-11-16T09:08:13

Shells: enyo, mochi, mojo. Scripts: 12. 2548 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | frame-field-hold | **pass** | 14 ok, 0 failed | WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false; frames: kept/enyo-frame-field-hold-field-menu_.jpg | cavekit-ui-shell.md R7 |
| enyo | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| enyo | keyboard-fields | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,284-522,319 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| enyo | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| enyo | long-press-text-field | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/enyo-long-press-text-field-hold-text_.jpg, kept/enyo-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| enyo | pan-long-flick | **known** | 3 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 16-stream-stale: stale 3 (want at most 0) / CHECK FAIL 17-stream-uniform: uniform 35 (want at most 0) | cavekit-gpu-rendering.md R3 |
| enyo | pan-speeds | **known** | 6 ok, 1 failed | the open defect T-567: CHECK FAIL 20-stream-uniform: uniform 31 (want at most 0) | cavekit-finger-testing.md R4 |
| enyo | personas-page-steady | **pass** | 5 ok, 0 failed | CHECK ok 18-stream-gap-p90: gap-p90 35 (at most 50) | cavekit-personas-ui.md R9 |
| enyo | volume-keys | **pass** | 10 ok, 0 failed | CHECK ok 30-sound: sound at the output sink: peak 2958 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| enyo | wikipedia-zoom | **fail** | 11 ok, 1 failed | CHECK FAIL 24-zoom: zoom 0.9079 (want 0.69 within 0.12); frames: kept/enyo-wikipedia-zoom-24-zoom_.jpg | cavekit-gpu-rendering.md R6 |
| mochi | frame-field-hold | **pass** | 14 ok, 0 failed | WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false; frames: kept/mochi-frame-field-hold-field-menu_.jpg | cavekit-ui-shell.md R7 |
| mochi | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | keyboard-fields | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,280-522,315 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| mochi | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| mochi | long-press-text-field | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/mochi-long-press-text-field-hold-text_.jpg, kept/mochi-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| mochi | mochi-paste | **pass** | 4 ok, 0 failed | CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused); frames: kept/mochi-mochi-paste-mochi-field-sheet_.jpg | cavekit-mochi-ui.md R2 |
| mochi | pan-long-flick | **known** | 4 ok, 1 failed | the open defect T-567: CHECK FAIL 17-stream-uniform: uniform 26 (want at most 0) | cavekit-gpu-rendering.md R3 |
| mochi | pan-speeds | **known** | 6 ok, 1 failed | the open defect T-567: CHECK FAIL 20-stream-uniform: uniform 15 (want at most 0) | cavekit-finger-testing.md R4 |
| mochi | personas-page-steady | **pass** | 5 ok, 0 failed | CHECK ok 18-stream-gap-p90: gap-p90 38 (at most 50) | cavekit-personas-ui.md R9 |
| mochi | volume-keys | **pass** | 10 ok, 0 failed | CHECK ok 30-sound: sound at the output sink: peak 2956 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| mochi | wikipedia-zoom | **known** | 11 ok, 1 failed | the open defect T-567: CHECK FAIL 27-stream-uniform: uniform 6 (want at most 0) | cavekit-gpu-rendering.md R6 |
| mojo | frame-field-hold | **pass** | 14 ok, 0 failed | WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false; frames: kept/mojo-frame-field-hold-field-menu_.jpg | cavekit-ui-shell.md R7 |
| mojo | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mojo | keyboard-fields | **pass** | 6 ok, 0 failed | CHECK ok 17-visible: field on the screen at 16,275-522,310 (the reach ends at 600, keyboard 1) | cavekit-input-bridging.md R2 |
| mojo | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| mojo | long-press-text-field | **pass** | 6 ok, 0 failed | CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html; frames: kept/mojo-long-press-text-field-hold-text_.jpg, kept/mojo-long-press-text-field-hold-field_.jpg | cavekit-ui-shell.md R7 |
| mojo | mojo-address-typing | **fail** | 3 ok, 1 failed | CHECK FAIL 14-nav-paint: no first paint of a new page within the bound; frames: kept/mojo-mojo-address-typing-14-nav-paint_.jpg | cavekit-mojo-ui.md R6 |
| mojo | pan-long-flick | **fail** | 2 ok, 3 failed | CHECK FAIL 17-stream-uniform: uniform 17 (want at most 0) / CHECK FAIL 19-stream-gap-median: gap-median 35 (want at most 34) / CHECK FAIL 20-stream-gap-p90: gap-p90 53 (want at most 50) | cavekit-gpu-rendering.md R3 |
| mojo | pan-speeds | **known** | 5 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 19-stream-stale: stale 8 (want at most 0) / CHECK FAIL 20-stream-uniform: uniform 22 (want at most 0) | cavekit-finger-testing.md R4 |
| mojo | personas-page-steady | **pass** | 5 ok, 0 failed | CHECK ok 18-stream-gap-p90: gap-p90 41 (at most 50) | cavekit-personas-ui.md R9 |
| mojo | volume-keys | **pass** | 10 ok, 0 failed | CHECK ok 30-sound: sound at the output sink: peak 2959 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz | cavekit-finger-testing.md R7 |
| mojo | wikipedia-zoom | **known** | 11 ok, 1 failed | the open defect T-567: CHECK FAIL 27-stream-uniform: uniform 3 (want at most 0) | cavekit-gpu-rendering.md R6 |

## The device at the end

```
DEVICE ok job-enyo: (364653/728451141) jihad (start) running, process 5907
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (364659/159736621) jihad-mochi (start) running, process 14923
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (364665/911057657) jihad-mojo (start) running, process 29939
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

### enyo / frame-field-hold — pass

    enyo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +3 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect f
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 151 landed on document y 88)
    WAIT ok 12-line: [jihad-bs] hold at 77,336 editable=0 wasFocused=0
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=false readOnly=false selected=true
    WAIT ok 15-still f
    WAIT ok 18-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=0
    WAIT ok 19-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false
    WAIT ok 22-line: [jihad-bs] notify: page-hold: {"editable":true,"readOnly":false,"selected":false} (pushed)
    WAIT ok 23-differs f
    CHECK ok 24-region-changed: region 13,323-756,528: colours 299 hash 49e48b9b rows 205 uniform 0 light 324-526 blue none against the sample's hash 261e5084
    CHECK ok 25-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 30-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 32-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 34-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=1
    WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false

### enyo / frame-field — pass

    enyo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect f
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 151 landed on document y 88)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 11-visible: f on the screen at 13,322-759,529 (the reach ends at 600, keyboard 1)
    WAIT ok 14-differs f
    CHECK ok 15-region-changed: region 13,323-756,528: colours 203 hash 9f55f686 rows 205 uniform 0 light 324-526 blue none against the sample's hash 2a543c72
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-still f
    WAIT ok 20-still f
    CHECK ok 21-region-equal: region 13,323-756,528: colours 202 hash af7f74a8 rows 205 uniform 0 light 324-526 blue none = the sample's hash
    CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / keyboard-fields — pass

    enyo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 301 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,160-484,200 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,284-522,319 (the reach ends at 600, keyboard 1)

### enyo / long-press-menu — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### enyo / long-press-text-field — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +3 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 269,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### enyo / pan-long-flick — known

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 10 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 21-card-scroll-min: the card's scroll y 3386 (at least 300)
    STREAM fs-enyo-pan-long-flick.fbs: 177 samples (14.5/s asked 15, read median 2.23 ms max 50.4 ms, mmap stride 8, re-read 6, torn 0), 140 flips, cpu 7.6% of a core, polls 690/s | shown frames while moving: 136 gaps, median 27.0 ms, p90 42 ms, max 238 ms | shown scroll: 1550 rows over the stream (0 samples with the shift unknown); stale bands 3, uniform bands 35, still rows moved 0; the columns outside 0-756 changed in 49 samples | tracking (finger down, 23 samples): error median 29 px, max 262 px; lag median 289 ms, max 991 ms |   gesture at 1794838459022: the finger travelled 600 rows, the page showe
    CHECK FAIL 16-stream-stale: stale 3 (want at most 0)
    CHECK FAIL 17-stream-uniform: uniform 35 (want at most 0)
    CHECK ok 19-stream-gap-median: gap-median 27 (at most 34)
    CHECK ok 20-stream-gap-p90: gap-p90 42 (at most 50)

### enyo / pan-speeds — known

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 650 (at least 500)
    CHECK ok 25-card-scroll-min: the card's scroll y 4607 (at least 2500)
    STREAM fs-enyo-pan-speeds.fbs: 411 samples (14.8/s asked 15, read median 2.22 ms max 56.69 ms, mmap stride 8, re-read 25, torn 0), 370 flips, cpu 6.8% of a core, polls 699/s | shown frames while moving: 363 gaps, median 25 ms, p90 40 ms, max 214 ms | shown scroll: 4199 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 31, still rows moved 0; the columns outside 0-756 changed in 129 samples | tracking (finger down, 123 samples): error median -13 px, max 187 px; lag median 68 ms, max 807 ms |   gesture at 1794838523579: the finger travelled 650 rows, the page showed 
    CHECK ok 19-stream-stale: stale 0 (at most 0)
    CHECK FAIL 20-stream-uniform: uniform 31 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 187 (at most 500)
    CHECK ok 23-stream-gap-median: gap-median 25 (at most 34)
    CHECK ok 24-stream-gap-p90: gap-p90 40 (at most 50)

### enyo / personas-page-steady — pass

    enyo card on about:personas: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 280 landed on document y 198)
    CHECK ok 19-content: region 0,100-768,900: colours 4096 hash 9425246f rows 800 uniform 0 light 100-899 blue 278-752; sel:body on the screen at 0,-415-764,2154
    STREAM fs-enyo-personas-page-steady.fbs: 108 samples (15.0/s asked 15, read median 2.4 ms max 40.21 ms, mmap stride 8, re-read 6, torn 0), 147 flips, cpu 6.4% of a core, polls 648/s | shown frames while moving: 146 gaps, median 24.0 ms, p90 35 ms, max 49 ms | shown scroll: 499 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 49 samples | tracking (finger down, 51 samples): error median -10 px, max 30 px; lag median 67 ms, max 221 ms |   gesture at 1794838604961: the finger travelled 500 rows, the page sho
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)
    CHECK ok 17-stream-gap-median: gap-median 24 (at most 34)
    CHECK ok 18-stream-gap-p90: gap-p90 35 (at most 50)

### enyo / volume-keys — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +7 s, 17 content frames at +7 s; the GPU path, frames off the main thread
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2958 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume: the media volume 66, higher (marked 44)
    CHECK ok 17-sound: sound at the output sink: peak 2958 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 20-volume: the media volume 44, lower (marked 66)
    CHECK ok 21-sound: sound at the output sink: peak 2956 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 25-volume: the media volume 0, zero (marked 66)
    CHECK ok 26-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2959 rms 2058 frames 45056 hz 440): no sound out
    CHECK ok 29-volume: the media volume 22, higher (marked 0)
    CHECK ok 30-sound: sound at the output sink: peak 2958 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### enyo / wikipedia-zoom — fail

    enyo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +2 s, load done at +15 s, 9 content frames at +15 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 11-rect sel:.infobox
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 158 landed on document y 110)
    WAIT ok 15-zoom
    CHECK ok 16-zoom-min: zoom 2.8444 (at least 1.3)
    WAIT ok 18-zoom
    WAIT ok 20-zoom
    WAIT ok 22-still 0,100,756,700
    CHECK FAIL 24-zoom: zoom 0.9079 (want 0.69 within 0.12)
    CHECK ok 25-content: region 0,100-756,700: colours 4096 hash 8c986608 rows 600 uniform 32 light 100-699 blue 176-180; sel:body on the screen at 0,23-768,968
    CHECK ok 26-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    STREAM fs-enyo-wikipedia-zoom.fbs: 220 samples (13.7/s asked 15, read median 1.69 ms max 19.81 ms, mmap stride 8, re-read 6, torn 0), 155 flips, cpu 5.9% of a core, polls 671/s | shown frames while moving: 146 gaps, median 36.0 ms, p90 53 ms, max 243 ms | shown scroll: -1 rows over the stream (47 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 170-600 changed in 14 samples | tracking (finger down, 40 samples): error median 0.0 px, max 1 px; lag median 403.5 ms, max 1097 ms |   gesture at 1794838750861: the finger travelled 0 rows, the page show
    CHECK ok 27-stream-uniform: uniform 0 (at most 0)
    CHECK ok 29-stream-gap-median: gap-median 36 (at most 45)
    CHECK ok 30-stream-gap-p90: gap-p90 53 (at most 80)

### mochi / frame-field-hold — pass

    mochi card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect f
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 147 landed on document y 88)
    WAIT ok 12-line: [jihad-bs] hold at 77,336 editable=0 wasFocused=0
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=false readOnly=false selected=true
    WAIT ok 15-still f
    WAIT ok 18-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=0
    WAIT ok 19-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false
    WAIT ok 22-line: [jihad-bs] notify: page-hold: {"editable":true,"readOnly":false,"selected":false} (pushed)
    WAIT ok 23-differs f
    CHECK ok 24-region-changed: region 13,319-756,524: colours 218 hash c90db968 rows 205 uniform 0 light none blue none against the sample's hash 22936c02
    CHECK ok 25-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 30-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 32-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 34-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=1
    WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false

### mochi / frame-field — pass

    mochi card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 7-rect f
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 147 landed on document y 88)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 11-visible: f on the screen at 13,318-759,525 (the reach ends at 600, keyboard 1)
    WAIT ok 14-differs f
    CHECK ok 15-region-changed: region 13,319-756,524: colours 229 hash 4e051f6c rows 205 uniform 0 light 320-522 blue none against the sample's hash 47ef556c
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-still f
    WAIT ok 20-still f
    CHECK ok 21-region-equal: region 13,319-756,524: colours 228 hash ea9cee95 rows 205 uniform 0 light 320-522 blue none = the sample's hash
    CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / keyboard-fields — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,156-484,196 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,280-522,315 (the reach ends at 600, keyboard 1)

### mochi / long-press-menu — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mochi / long-press-text-field — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 269,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mochi / mochi-paste — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 297 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-differs 100,100,668,420
    WAIT ok 15-still 100,100,668,420
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / pan-long-flick — known

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 21-card-scroll-min: the card's scroll y 3396 (at least 300)
    STREAM fs-mochi-pan-long-flick.fbs: 236 samples (15.0/s asked 15, read median 2.19 ms max 26.9 ms, mmap stride 8, re-read 6, torn 0), 155 flips, cpu 6.0% of a core, polls 744/s | shown frames while moving: 152 gaps, median 27.0 ms, p90 42 ms, max 91 ms | shown scroll: 2648 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 26, still rows moved 41; the columns outside 0-756 changed in 53 samples | tracking (finger down, 21 samples): error median 29 px, max 263 px; lag median 276 ms, max 930 ms |   gesture at 1794839257496: the finger travelled 600 rows, the page show
    CHECK ok 16-stream-stale: stale 0 (at most 0)
    CHECK FAIL 17-stream-uniform: uniform 26 (want at most 0)
    CHECK ok 19-stream-gap-median: gap-median 27 (at most 34)
    CHECK ok 20-stream-gap-p90: gap-p90 42 (at most 50)

### mochi / pan-speeds — known

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 646 (at least 500)
    CHECK ok 25-card-scroll-min: the card's scroll y 4545 (at least 2500)
    STREAM fs-mochi-pan-speeds.fbs: 473 samples (15.0/s asked 15, read median 2.21 ms max 51.21 ms, mmap stride 8, re-read 18, torn 0), 361 flips, cpu 6.1% of a core, polls 732/s | shown frames while moving: 354 gaps, median 26.0 ms, p90 41 ms, max 100 ms | shown scroll: 3865 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 15, still rows moved 111; the columns outside 0-756 changed in 133 samples | tracking (finger down, 122 samples): error median -12.0 px, max 149 px; lag median 67.5 ms, max 827 ms |   gesture at 1794839330283: the finger travelled 650 rows, the pag
    CHECK ok 19-stream-stale: stale 0 (at most 0)
    CHECK FAIL 20-stream-uniform: uniform 15 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 149 (at most 500)
    CHECK ok 23-stream-gap-median: gap-median 26 (at most 34)
    CHECK ok 24-stream-gap-p90: gap-p90 41 (at most 50)

### mochi / personas-page-steady — pass

    mochi card on about:personas: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 276 landed on document y 198)
    CHECK ok 19-content: region 0,100-768,900: colours 4096 hash db2bd3e2 rows 800 uniform 0 light 103-648 blue 108-656; sel:body on the screen at 0,-417-764,2442
    STREAM fs-mochi-personas-page-steady.fbs: 130 samples (15.0/s asked 15, read median 2.21 ms max 20.17 ms, mmap stride 8, re-read 6, torn 0), 142 flips, cpu 6.3% of a core, polls 700/s | shown frames while moving: 141 gaps, median 25 ms, p90 38 ms, max 97 ms | shown scroll: 497 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 50 samples | tracking (finger down, 49 samples): error median -10 px, max 30 px; lag median 66 ms, max 162 ms |   gesture at 1794839413161: the finger travelled 500 rows, the page sho
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)
    CHECK ok 17-stream-gap-median: gap-median 25 (at most 34)
    CHECK ok 18-stream-gap-p90: gap-p90 38 (at most 50)

### mochi / volume-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +1 s, load done at +7 s, 13 content frames at +7 s; the GPU path, frames off the main threa
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2952 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume: the media volume 66, higher (marked 44)
    CHECK ok 17-sound: sound at the output sink: peak 2958 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 20-volume: the media volume 44, lower (marked 66)
    CHECK ok 21-sound: sound at the output sink: peak 2963 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 25-volume: the media volume 0, zero (marked 66)
    CHECK ok 26-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2955 rms 2059 frames 45056 hz 440): no sound out
    CHECK ok 29-volume: the media volume 22, higher (marked 0)
    CHECK ok 30-sound: sound at the output sink: peak 2956 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### mochi / wikipedia-zoom — known

    mochi card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +15 s, 9 content frames at +15 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 11-rect sel:.infobox
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 154 landed on document y 110)
    WAIT ok 15-zoom
    CHECK ok 16-zoom-min: zoom 2.8444 (at least 1.3)
    WAIT ok 18-zoom
    WAIT ok 20-zoom
    WAIT ok 22-still 0,100,756,700
    CHECK ok 24-zoom: zoom 0.6882 (want 0.69 within 0.12)
    CHECK ok 25-content: region 0,100-756,700: colours 4096 hash 07733800 rows 600 uniform 62 light 100-699 blue 232-236; sel:body on the screen at 0,78-768,1028
    CHECK ok 26-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    STREAM fs-mochi-wikipedia-zoom.fbs: 242 samples (14.8/s asked 15, read median 1.71 ms max 31.87 ms, mmap stride 8, re-read 3, torn 0), 144 flips, cpu 6.4% of a core, polls 754/s | shown frames while moving: 135 gaps, median 37 ms, p90 50 ms, max 271 ms | shown scroll: 87 rows over the stream (37 samples with the shift unknown); stale bands 0, uniform bands 6, still rows moved 1; the columns outside 170-600 changed in 18 samples | tracking (finger down, 32 samples): error median 0.0 px, max 1 px; lag median 330.0 ms, max 662 ms |   gesture at 1794839567182: the finger travelled 0 rows, the page showed
    CHECK FAIL 27-stream-uniform: uniform 6 (want at most 0)
    CHECK ok 29-stream-gap-median: gap-median 37 (at most 45)
    CHECK ok 30-stream-gap-p90: gap-p90 50 (at most 80)

### mojo / frame-field-hold — pass

    mojo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect f
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 142 landed on document y 88)
    WAIT ok 12-line: [jihad-bs] hold at 77,336 editable=0 wasFocused=0
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=false readOnly=false selected=true
    WAIT ok 15-still f
    WAIT ok 18-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=0
    WAIT ok 19-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false
    WAIT ok 22-line: [jihad-bs] notify: page-hold: {"editable":true,"readOnly":false,"selected":false} (pushed)
    WAIT ok 23-differs f
    CHECK ok 24-region-changed: region 13,314-756,519: colours 609 hash 4f7ab6f1 rows 205 uniform 0 light 326-444 blue none against the sample's hash ce1bdacc
    CHECK ok 25-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 30-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 32-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 34-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=1
    WAIT ok 35-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false

### mojo / frame-field — pass

    mojo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect f
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 142 landed on document y 88)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 11-visible: f on the screen at 13,313-759,520 (the reach ends at 600, keyboard 1)
    WAIT ok 14-differs f
    CHECK ok 15-region-changed: region 13,314-756,519: colours 203 hash 3245e2d2 rows 205 uniform 0 light 315-517 blue none against the sample's hash 14b1cd9a
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-still f
    WAIT ok 20-still f
    CHECK ok 21-region-equal: region 13,314-756,519: colours 202 hash ba939f24 rows 205 uniform 0 light 315-517 blue none = the sample's hash
    CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / keyboard-fields — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 292 landed on document y 279)
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 10-visible: area on the screen at 9,151-484,191 (the reach ends at 600, keyboard 1)
    CHECK ok 12-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-launch: load done
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 17-visible: field on the screen at 16,275-522,310 (the reach ends at 600, keyboard 1)

### mojo / long-press-menu — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mojo / long-press-text-field — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,100,756,600
    WAIT ok 13-still 0,100,756,600
    WAIT ok 15-line: [jihad-bs] hold at 269,219 editable=1 wasFocused=0
    WAIT ok 18-still 0,100,400,240
    WAIT ok 20-load
    CHECK ok 21-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mojo / mojo-address-typing — fail

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK FAIL 14-nav-paint: no first paint of a new page within the bound
    WAIT ok 15-load
    CHECK ok 16-url: load done uri=https://lite.duckduckgo.com/lite/?q=webos

### mojo / pan-long-flick — fail

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 21-card-scroll-min: the card's scroll y 2927 (at least 300)
    STREAM fs-mojo-pan-long-flick.fbs: 228 samples (14.2/s asked 15, read median 2.15 ms max 80.53 ms, mmap stride 8, re-read 5, torn 0), 87 flips, cpu 6.6% of a core, polls 723/s | shown frames while moving: 80 gaps, median 35.0 ms, p90 53 ms, max 233 ms | shown scroll: 101 rows over the stream (4 samples with the shift unknown); stale bands 0, uniform bands 17, still rows moved 0; the columns outside 0-756 changed in 32 samples | tracking (finger down, 23 samples): error median 30 px, max 525 px; lag median 209 ms, max 927 ms |   gesture at 1794840253311: the finger travelled 600 rows, the page showed 
    CHECK ok 16-stream-stale: stale 0 (at most 0)
    CHECK FAIL 17-stream-uniform: uniform 17 (want at most 0)
    CHECK FAIL 19-stream-gap-median: gap-median 35 (want at most 34)
    CHECK FAIL 20-stream-gap-p90: gap-p90 53 (want at most 50)

### mojo / pan-speeds — known

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 605 (at least 500)
    CHECK ok 25-card-scroll-min: the card's scroll y 4482 (at least 2500)
    STREAM fs-mojo-pan-speeds.fbs: 543 samples (14.8/s asked 15, read median 2.18 ms max 67.83 ms, mmap stride 8, re-read 21, torn 0), 292 flips, cpu 6.2% of a core, polls 742/s | shown frames while moving: 285 gaps, median 32 ms, p90 47 ms, max 257 ms | shown scroll: 3235 rows over the stream (0 samples with the shift unknown); stale bands 8, uniform bands 22, still rows moved 0; the columns outside 0-756 changed in 115 samples | tracking (finger down, 123 samples): error median -12 px, max 188 px; lag median 74 ms, max 863 ms |   gesture at 1794840329808: the finger travelled 650 rows, the page showed 
    CHECK FAIL 19-stream-stale: stale 8 (want at most 0)
    CHECK FAIL 20-stream-uniform: uniform 22 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 188 (at most 500)
    CHECK ok 23-stream-gap-median: gap-median 32 (at most 34)
    CHECK ok 24-stream-gap-p90: gap-p90 47 (at most 50)

### mojo / personas-page-steady — pass

    mojo card on about:personas: connected at +1 s, load done at +8 s, 15 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 19-content: region 0,100-768,900: colours 4096 hash 4f6212d6 rows 800 uniform 0 light 181-736 blue 155-744; sel:body on the screen at 0,-426-764,2441
    STREAM fs-mojo-personas-page-steady.fbs: 140 samples (15.0/s asked 15, read median 2.18 ms max 16.33 ms, mmap stride 8, re-read 2, torn 0), 109 flips, cpu 6.5% of a core, polls 725/s | shown frames while moving: 108 gaps, median 31.0 ms, p90 41 ms, max 250 ms | shown scroll: 501 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 46 samples | tracking (finger down, 54 samples): error median -10.0 px, max 62 px; lag median 74.0 ms, max 405 ms |   gesture at 1794840431451: the finger travelled 500 rows, the pa
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)
    CHECK ok 17-stream-gap-median: gap-median 31 (at most 34)
    CHECK ok 18-stream-gap-p90: gap-p90 41 (at most 50)

### mojo / volume-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread 
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2975 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume: the media volume 66, higher (marked 44)
    CHECK ok 17-sound: sound at the output sink: peak 2964 rms 2060 frames 45056 hz 440, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 20-volume: the media volume 44, lower (marked 66)
    CHECK ok 21-sound: sound at the output sink: peak 2946 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 25-volume: the media volume 0, zero (marked 66)
    CHECK ok 26-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2954 rms 2059 frames 45056 hz 440): no sound out
    CHECK ok 29-volume: the media volume 22, higher (marked 0)
    CHECK ok 30-sound: sound at the output sink: peak 2959 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### mojo / wikipedia-zoom — known

    mojo card on https://en.wikipedia.org/wiki/HP_TouchPad: connected at +1 s, load done at +12 s, 8 content frames at +12 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 11-rect sel:.infobox
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 149 landed on document y 110)
    WAIT ok 15-zoom
    CHECK ok 16-zoom-min: zoom 2.7529 (at least 1.3)
    WAIT ok 18-zoom
    WAIT ok 20-zoom
    WAIT ok 22-still 0,100,756,700
    CHECK ok 24-zoom: zoom 0.6882 (want 0.69 within 0.12)
    CHECK ok 25-content: region 0,100-756,700: colours 4096 hash 073c673d rows 600 uniform 62 light 100-699 blue 227-231; sel:body on the screen at 0,73-768,1073
    CHECK ok 26-text-has: sel:#firstHeading = [HP TouchPad] holds [TouchPad]
    STREAM fs-mojo-wikipedia-zoom.fbs: 249 samples (14.8/s asked 15, read median 1.68 ms max 106.44 ms, mmap stride 8, re-read 8, torn 0), 135 flips, cpu 5.9% of a core, polls 741/s | shown frames while moving: 128 gaps, median 39.0 ms, p90 79 ms, max 290 ms | shown scroll: 90 rows over the stream (35 samples with the shift unknown); stale bands 0, uniform bands 3, still rows moved 0; the columns outside 170-600 changed in 20 samples | tracking (finger down, 35 samples): error median 0 px, max 14 px; lag median 348 ms, max 721 ms |   gesture at 1794840587704: the finger travelled 0 rows, the page showed 
    CHECK FAIL 27-stream-uniform: uniform 3 (want at most 0)
    CHECK ok 29-stream-gap-median: gap-median 39 (at most 45)
    CHECK ok 30-stream-gap-p90: gap-p90 79 (at most 80)

