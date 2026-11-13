# Finger suite report — 2026-11-13T01:10:16

Shells: mojo. Scripts: 19. 1362 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| mojo | addons-list-pan | **pass** | 7 ok, 0 failed | pads: 12 put in /media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mojo/profile/extensions | cavekit-chrome-look.md R2 |
| mojo | drag-and-pinch-page | **pass** | 7 ok, 0 failed | CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15) | cavekit-input-bridging.md R3 |
| mojo | everyday-browsing | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-engine-update.md R8 |
| mojo | express-burst-wiki | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-gpu-rendering.md R3 |
| mojo | flash-input-hold | **pass** | 8 ok, 0 failed | CHECK ok 16-press-rate: 22.3 moves a second sent on (at least 20): ] a press of 1480 ms: 32 moves from the card in 1390 ms, 31 sent on, 566 ms in sending them | cavekit-flash.md R5 |
| mojo | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R6 |
| mojo | flash-input-keys | **pass** | 13 ok, 0 failed | CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mojo | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 97 (at least 30) | cavekit-flash.md R5 |
| mojo | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,278: 3.0 s, flips 41 (13.7/s), changes 0 (0.0/s), reads 42, distinct 1, spacing median 68 p90 147 max 152 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mojo | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mojo | media-scrub-drag | **pass** | 8 ok, 0 failed | WAIT ok 25-text seek = [60]; frames: kept/mojo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| mojo | pan-long-drag | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mojo | pan-long-flick | **known** | 2 ok, 1 failed | the open defect T-567: CHECK FAIL 17-stream-uniform: uniform 26 (want at most 0) | cavekit-gpu-rendering.md R3 |
| mojo | pan-speeds | **known** | 3 ok, 2 failed | the open defect T-567, T-567: CHECK FAIL 19-stream-stale: stale 8 (want at most 0) / CHECK FAIL 20-stream-uniform: uniform 23 (want at most 0) | cavekit-finger-testing.md R4 |
| mojo | personas-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mojo | personas-page-steady | **pass** | 3 ok, 0 failed | CHECK ok 15-stream-uniform: uniform 0 (at most 0) | cavekit-personas-ui.md R9 |
| mojo | script-hold-zoom | **pass** | 6 ok, 0 failed | CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2) | cavekit-gpu-rendering.md R3 |
| mojo | settings-flick | **pass** | 3 ok, 0 failed | CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0 | cavekit-chrome-look.md R3 |
| mojo | webgl-cube | **pass** | 7 ok, 0 failed | WAIT ok 16-changed c | cavekit-gpu-rendering.md R8 |

## The device at the end

```
DEVICE ok job-enyo: (75595/951252466) jihad (start) running, process 13413
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (75602/814997204) jihad-mochi (start) running, process 17430
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (75608/776306091) jihad-mojo (start) running, process 25176
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
DEVICE FAIL tmp-dumps: /tmp/sb-probe.png /tmp/sb-probe2.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 4% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### mojo / addons-list-pan — pass

    mojo card on about:addons: connected at +1 s, load done at +9 s, 7 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 8-rect sel:#category-extension
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 249 landed on document y 224)
    WAIT ok 10-rect sel:#category-extension[selected='true']
    WAIT ok 11-rect sel:richlistitem[value='palefill@addons.martoks-place.de']
    WAIT ok 12-rect sel:richlistitem[value='pad12@jihad.test']
    WAIT ok 15-still 0,100,756,1000
    WAIT ok 17-rect sel:richlistitem[value='pad12@jihad.test'][selected='true']
    CHECK ok 18-hit: clickAt 843,1439 inside sel:richlistitem[value="pad12@jihad.test"] at 294,1412 656x99

### mojo / drag-and-pinch-page — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 9-card-scroll-min: the card's scroll y 594 (at least 400)
    WAIT ok 11-zoom
    CHECK ok 12-zoom-min: zoom 2.6394 (at least 1.5)
    CHECK ok 13-content: region 0,150-756,600: colours 312 hash 9fed57f6 rows 450 uniform 225 light 176-599 blue none; sel:body on the screen at -630,-2310-1386,51766
    WAIT ok 15-zoom
    WAIT ok 17-zoom
    CHECK ok 18-zoom: zoom 1.0000 (want 1.0 within 0.15)

### mojo / everyday-browsing — not run

    needs internet (run with --with internet)

### mojo / express-burst-wiki — not run

    needs internet (run with --with internet)

### mojo / flash-input-hold — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 159]
    CHECK ok 10-number: heldms = 2286 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 14-text up = [up 361 287]
    CHECK ok 15-flash-el: the movie's move at 352,281 for 360,277 (within 30)
    CHECK ok 16-press-rate: 22.3 moves a second sent on (at least 20): ] a press of 1480 ms: 32 moves from the card in 1390 ms, 31 sent on, 566 ms in sending them

### mojo / flash-input-keyhold — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 134 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 88 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / flash-input-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 49 49]
    WAIT ok 19-text keydown = [keydown 32 32]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    WAIT ok 22-text keydown = [keydown 38 0]
    WAIT ok 24-text keydown = [keydown 39 0]
    WAIT ok 26-text keydown = [keydown 40 0]
    WAIT ok 28-text keydown = [keydown 37 0]
    CHECK ok 29-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / flash-input-lock — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread (the express l
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 30
    CHECK ok 10-card-scroll-min: the card's scroll y 30 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-number: n-down = 1 (wanted 1..1)
    CHECK ok 16-number: n-up = 1 (wanted 1..1)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 22-number: n-move = 15 (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 97
    CHECK ok 31-card-scroll-min: the card's scroll y 97 (at least 30)

### mojo / flash-lane — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +1 s, load done at +6 s, 10 content frames at +6 s; the GPU path, frames off t
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,114-440,412: 3.0 s, flips 39 (13.0/s), changes 37 (12.3/s), reads 40, distinct 38, spacing median 68 p90 118 max 168 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,114-440,412: 3.0 s, flips 38 (12.7/s), changes 0 (0.0/s), reads 39, distinct 1, spacing median 67 p90 147 max 199 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,114-440,412: 3.0 s, flips 43 (14.3/s), changes 41 (13.7/s), reads 44, distinct 42, spacing median 67 p90 87 max 153 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 90
    CHECK ok 16-flash-rate: content 40,0-440,278: 3.0 s, flips 44 (14.7/s), changes 35 (11.7/s), reads 45, distinct 36, spacing median 67 p90 82 max 147 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,278: 3.0 s, flips 41 (13.7/s), changes 0 (0.0/s), reads 42, distinct 1, spacing median 68 p90 147 max 152 ms (wanted 0..4/s)

### mojo / flash-layout — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the e
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 4)
    WAIT ok 10-launch: load done
    WAIT ok 11-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 13-flash-at: the movie reported down 101,100 for stage 100,100 (within 3)
    CHECK ok 15-flash-at: the movie reported down 301,200 for stage 300,200 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 23-settled: the card's scroll 0 1354
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-rect show
    WAIT ok 29-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / media-scrub-drag — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +7 s, 18 content frames at +7 s; the GPU path, frames off the main thread (the ex
    WAIT ok 10-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 12-text status = [playing]
    WAIT ok 14-text status = [paused t=22.6]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    WAIT ok 25-text seek = [60]

### mojo / pan-long-drag — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 2 content frames at +36 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 374 hash 85b98daf rows 250 uniform 0 light 150-399 blue none; sel:#content p on the screen at 16,145-748,201, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 594 (want 650 within 100); the engine's 594
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,29-756,83: colours 185 hash eeaac7d6 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 594 hash 9bf60c66 rows 600 uniform 221 light 302-899 blue none against the sample's hash 39f383a9
    CHECK ok 18-top: the page's top measured 73 (the tap at y 214 landed on document y 780), the model says 73 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mojo / pan-long-flick — known

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +3 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 18-card-scroll-min: the card's scroll y 2900 (at least 300)
    STREAM fs-mojo-pan-long-flick.fbs: 235 samples (14.7/s asked 15, read median 2.24 ms max 67.2 ms, mmap stride 8, re-read 5, torn 0), 63 flips, cpu 7.2% of a core, polls 747/s | shown frames while moving: 59 gaps, median 32 ms, p90 46 ms, max 208 ms | shown scroll: 1286 rows over the stream (1 samples with the shift unknown); stale bands 0, uniform bands 26, still rows moved 0; the columns outside 0-756 changed in 24 samples | tracking (finger down, 9 samples): error median -75 px, max 487 px; lag median 71 ms, max 202 ms |   gesture at 1794551064028: the finger travelled 600 rows, the page showed 0 b
    CHECK ok 16-stream-stale: stale 0 (at most 0)
    CHECK FAIL 17-stream-uniform: uniform 26 (want at most 0)

### mojo / pan-speeds — known

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 13-card-scroll-min: the card's scroll y 605 (at least 500)
    CHECK ok 22-card-scroll-min: the card's scroll y 4440 (at least 2500)
    STREAM fs-mojo-pan-speeds.fbs: 374 samples (14.5/s asked 15, read median 2.3 ms max 52.23 ms, mmap stride 8, re-read 15, torn 0), 297 flips, cpu 6.8% of a core, polls 651/s | shown frames while moving: 290 gaps, median 32.0 ms, p90 47 ms, max 268 ms | shown scroll: 3873 rows over the stream (0 samples with the shift unknown); stale bands 8, uniform bands 23, still rows moved 0; the columns outside 0-756 changed in 120 samples | tracking (finger down, 123 samples): error median -12 px, max 300 px; lag median 80 ms, max 795 ms |   gesture at 1794551136922: the finger travelled 650 rows, the page showed
    CHECK FAIL 19-stream-stale: stale 8 (want at most 0)
    CHECK FAIL 20-stream-uniform: uniform 23 (want at most 0)
    CHECK ok 21-stream-error-max: error-max 300 (at most 500)

### mojo / personas-flick — pass

    mojo card on about:personas: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect sel:#mine .card
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 10-card-scroll-min: the card's scroll y 1602 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mojo / personas-page-steady — pass

    mojo card on about:personas: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-rect sel:#mine .card
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 271 landed on document y 198)
    CHECK ok 16-content: region 0,100-768,900: colours 4096 hash 8b3a5134 rows 800 uniform 0 light 184-739 blue 158-747; sel:body on the screen at 0,-423-764,2444
    STREAM fs-mojo-personas-page-steady.fbs: 102 samples (15.0/s asked 15, read median 2.32 ms max 68.69 ms, mmap stride 8, re-read 6, torn 0), 101 flips, cpu 6.3% of a core, polls 673/s | shown frames while moving: 99 gaps, median 28 ms, p90 40 ms, max 235 ms | shown scroll: 498 rows over the stream (0 samples with the shift unknown); stale bands 0, uniform bands 0, still rows moved 0; the columns outside 0-756 changed in 41 samples | tracking (finger down, 48 samples): error median -10.0 px, max 62 px; lag median 76.5 ms, max 351 ms |   gesture at 1794551293413: the finger travelled 500 rows, the page 
    CHECK ok 15-stream-uniform: uniform 0 (at most 0)

### mojo / script-hold-zoom — pass

    mojo card on http://192.168.1.245:8077/jihad-script-hold.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 101 landed on document y 28)
    WAIT ok 9-zoom
    CHECK ok 10-zoom-min: zoom 1.9326 (at least 1.3)
    CHECK ok 12-card-scroll-min: the card's scroll y 441 (at least 200)
    WAIT ok 14-zoom
    WAIT ok 16-zoom
    CHECK ok 17-zoom: zoom 1.0000 (want 1.0 within 0.2)

### mojo / settings-flick — pass

    mojo card on about:settings: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 7-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 10-card-scroll-min: the card's scroll y 675 (at least 500)
    CHECK ok 14-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0

### mojo / webgl-cube — pass

    mojo card on http://192.168.1.245:8077/jihad-webgl-cube.html: connected at +1 s, load done at +6 s, 14 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect c
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 115 landed on document y 57)
    CHECK ok 9-text-has: out = [webgl fps=18.2 shaders=ok texture=ok framebuffer=ok readPixels=ok maxtex=4096 ext=14 missing=ANGLE_instanced_arrays,WEBGL_compressed_texture_s3tc,EXT_blend_minmax,EXT_shader_texture_lod depthrb=ok renderer=Mozilla vendor=Mozilla] holds [webgl]
    CHECK ok 10-content: region 105,166-457,515: colours 624 hash a45c30e5 rows 349 uniform 138 light none blue 269-405; c on the screen at 105,165-457,516
    WAIT ok 11-changed c
    WAIT ok 13-zoom
    CHECK ok 15-content: region 0,100-768,900: colours 997 hash f461b52d rows 800 uniform 323 light none blue 276-385; c on the screen at -51,26-499,576
    WAIT ok 16-changed c

