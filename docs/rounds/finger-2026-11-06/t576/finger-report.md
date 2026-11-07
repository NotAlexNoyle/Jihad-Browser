# Finger suite report — 2026-11-07T00:51:11

First run 2026-11-06T22:40:54; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 1. 6804 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| enyo | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes); frames: kept/enyo-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| enyo | flash-input-cover (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| enyo | flash-input-hold (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 19-press-rate: 24.2 moves a second sent on (at least 20): ] a press of 1504 ms: 30 moves from the card in 1238 ms, 30 sent on, 600 ms in sending them | cavekit-flash.md R5 |
| enyo | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| enyo | flash-input-keys | **pass** | 12 ok, 0 failed | CHECK ok 28-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R5 |
| enyo | flash-input-lock | **pass** | 12 ok, 0 failed | CHECK ok 24-card-scroll-min: the card's scroll y 100 (at least 30) | cavekit-flash.md R5 |
| enyo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| enyo | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,284: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 65 p90 109 max 146 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| enyo | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| enyo | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 52 (17.3/s), changes 43 (14.3/s), reads 53, distinct 44, spacing median 62 p90 74 max 95 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| enyo | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| enyo | flash-audio | **pass** | 4 ok, 0 failed | CHECK ok 10-audio: the movie's audio: closed | cavekit-flash.md R2 |
| enyo | flash-drawn | **pass** | 12 ok, 0 failed | CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,123-440,421: colours 2 hash 4db04545 rows 298 uniform 0 light none blue 123-420 | cavekit-flash.md R2 |
| enyo | flash-plugin | **pass** | 5 ok, 0 failed | CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?) | cavekit-flash.md R2 |
| enyo | flash-rate (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 12-flash-rate-report: content 40,123-440,421: 8.0 s, flips 117 (14.6/s), changes 113 (14.1/s), reads 118, distinct 59, spacing median 67 p90 85 max 181 ms; the movie's own rate 14.3954/s (frames 40 t=3072 .. frames 190 t=13492) | cavekit-flash.md R2 |
| enyo | flash-reacted | **pass** | 10 ok, 0 failed | CHECK ok 22-flash-reacted: reacted: yes (content 40,123-440,421: 3.0 s, flips 16 (5.3/s), changes 2 (0.7/s), reads 18, distinct 2, spacing median 183 p90 195 max 243 ms new 18 of 18 first 0 ms) | cavekit-flash.md R2 |
| enyo | flash-signature | **pass** | 5 ok, 0 failed | CHECK ok 13-screen: the screen against its reference: difference 8.4 (same is 5 or less; wanted other) | cavekit-flash.md R2 |
| enyo | flash-stage-z1 | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| enyo | flash-stage-z2 | **pass** | 23 ok, 0 failed | CHECK ok 45-flash-at: the movie reported down 150,120 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| enyo | flash-stage-zfit | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mochi | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mochi | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes); frames: kept/mochi-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| mochi | flash-input-cover (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mochi | flash-input-hold (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 19-press-rate: 21.8 moves a second sent on (at least 20): ] a press of 1411 ms: 31 moves from the card in 1330 ms, 29 sent on, 490 ms in sending them | cavekit-flash.md R5 |
| mochi | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mochi | flash-input-keys | **pass** | 12 ok, 0 failed | CHECK ok 28-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mochi | flash-input-lock | **pass** | 12 ok, 0 failed | CHECK ok 24-card-scroll-min: the card's scroll y 97 (at least 30) | cavekit-flash.md R5 |
| mochi | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mochi | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,280: 3.0 s, flips 44 (14.7/s), changes 0 (0.0/s), reads 45, distinct 1, spacing median 66 p90 141 max 190 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mochi | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mochi | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 44 (14.7/s), changes 44 (14.7/s), reads 45, distinct 45, spacing median 69 p90 76 max 87 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mochi | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mochi | flash-audio | **pass** | 4 ok, 0 failed | CHECK ok 10-audio: the movie's audio: closed | cavekit-flash.md R2 |
| mochi | flash-drawn | **pass** | 12 ok, 0 failed | CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,119-440,417: colours 2 hash 4db04545 rows 298 uniform 0 light none blue 119-416 | cavekit-flash.md R2 |
| mochi | flash-plugin | **pass** | 5 ok, 0 failed | CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?) | cavekit-flash.md R2 |
| mochi | flash-rate (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 12-flash-rate-report: content 40,119-440,417: 8.0 s, flips 118 (14.8/s), changes 113 (14.1/s), reads 119, distinct 73, spacing median 67 p90 86 max 188 ms; the movie's own rate 14.4157/s (frames 20 t=1798 .. frames 180 t=12897) | cavekit-flash.md R2 |
| mochi | flash-reacted | **pass** | 10 ok, 0 failed | CHECK ok 22-flash-reacted: reacted: yes (content 40,119-440,417: 3.0 s, flips 16 (5.3/s), changes 2 (0.7/s), reads 18, distinct 2, spacing median 181 p90 189 max 246 ms new 18 of 18 first 0 ms) | cavekit-flash.md R2 |
| mochi | flash-signature | **pass** | 5 ok, 0 failed | CHECK ok 13-screen: the screen against its reference: difference 8.3 (same is 5 or less; wanted other) | cavekit-flash.md R2 |
| mochi | flash-stage-z1 | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mochi | flash-stage-z2 | **pass** | 23 ok, 0 failed | CHECK ok 45-flash-at: the movie reported down 150,120 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mochi | flash-stage-zfit | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mojo | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mojo | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 192 of 192 cells (wanted yes); frames: kept/mojo-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| mojo | flash-input-cover (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mojo | flash-input-hold (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 19-press-rate: 24.6 moves a second sent on (at least 20): ] a press of 1428 ms: 29 moves from the card in 1180 ms, 29 sent on, 457 ms in sending them | cavekit-flash.md R5 |
| mojo | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mojo | flash-input-keys | **pass** | 12 ok, 0 failed | CHECK ok 28-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mojo | flash-input-lock | **pass** | 12 ok, 0 failed | CHECK ok 24-card-scroll-min: the card's scroll y 100 (at least 30) | cavekit-flash.md R5 |
| mojo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mojo | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,273: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 67 p90 96 max 179 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mojo | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mojo | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 43 (14.3/s), changes 43 (14.3/s), reads 44, distinct 44, spacing median 69 p90 84 max 121 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mojo | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mojo | flash-audio | **pass** | 4 ok, 0 failed | CHECK ok 10-audio: the movie's audio: closed | cavekit-flash.md R2 |
| mojo | flash-drawn | **pass** | 12 ok, 0 failed | CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,114-440,412: colours 1 hash b955b145 rows 298 uniform 298 light none blue 114-411 | cavekit-flash.md R2 |
| mojo | flash-plugin | **pass** | 5 ok, 0 failed | CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?) | cavekit-flash.md R2 |
| mojo | flash-rate (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 12-flash-rate-report: content 40,114-440,412: 8.0 s, flips 118 (14.8/s), changes 115 (14.4/s), reads 119, distinct 61, spacing median 68 p90 87 max 158 ms; the movie's own rate 14.3805/s (frames 30 t=2468 .. frames 160 t=11508) | cavekit-flash.md R2 |
| mojo | flash-reacted | **pass** | 10 ok, 0 failed | CHECK ok 22-flash-reacted: reacted: yes (content 40,114-440,412: 3.0 s, flips 15 (5.0/s), changes 0 (0.0/s), reads 22, distinct 1, spacing median 204 p90 210 max 210 ms new 22 of 22 first 0 ms) | cavekit-flash.md R2 |
| mojo | flash-signature | **pass** | 5 ok, 0 failed | CHECK ok 13-screen: the screen against its reference: difference 8.3 (same is 5 or less; wanted other) | cavekit-flash.md R2 |
| mojo | flash-stage-z1 | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mojo | flash-stage-z2 | **pass** | 23 ok, 0 failed | CHECK ok 45-flash-at: the movie reported down 150,119 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mojo | flash-stage-zfit | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3) | cavekit-flash.md R2 |

## The device at the end

```
DEVICE ok job-enyo: (1265454/993743854) jihad (start) running, process 15537
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (1265459/472732742) jihad-mochi (start) running, process 25410
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (1265464/490060741) jihad-mojo (start) running, process 18967
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
DEVICE FAIL tmp-dumps: /tmp/fl-lane.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 4% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### enyo / flash-api — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +3 s, load done at +11 s, 9 content frames at +11 s; the GPU path,
    WAIT ok 7-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 8-text vars = [vars a=hello]
    WAIT ok 9-text so = [so n=1]
    WAIT ok 10-text echoback = [echoback back hi]
    CHECK ok 11-text: echo = [echo hi]
    WAIT ok 12-text geturl = [geturl ok]
    WAIT ok 13-text fscommand = [fscommand jcmd jarg]
    WAIT ok 14-text data = [data true x=42]
    WAIT ok 15-text kid = [kid w=376]
    WAIT ok 17-text so = [so n=2]
    WAIT ok 18-launch: load done
    WAIT ok 19-rect m
    CHECK ok 20-text: count = [0]
    CHECK ok 22-text: count = [0]

### enyo / flash-fullscreen — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main 
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes)

### enyo / flash-input-cover — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +5 s, load done at +14 s, 7 content frames at +14 s; the GPU path, frames off the main thread (the exp
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 10-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 229 px up for the keyboard, the page at 229
    WAIT ok 11-settled: the card's scroll 0 229
    CHECK ok 13-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### enyo / flash-input-hold — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-text up = [up 201 150]
    CHECK ok 13-number: heldms = 2292 (wanted 1500..3200)
    CHECK ok 14-number: n-down = 1 (wanted 1..1)
    CHECK ok 15-number: n-up = 1 (wanted 1..1)
    WAIT ok 17-text up = [up 361 278]
    CHECK ok 18-flash-el: the movie's move at 358,276 for 360,277 (within 30)
    CHECK ok 19-press-rate: 24.2 moves a second sent on (at least 20): ] a press of 1504 ms: 30 moves from the card in 1238 ms, 30 sent on, 600 ms in sending them

### enyo / flash-input-kernel — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express l
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 13-text keydown = [keydown 38 0]
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 40 0]
    CHECK ok 18-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 23-settled: the card's scroll 0 51
    CHECK ok 24-card-scroll-min: the card's scroll y 51 (at least 20)

### enyo / flash-input-keys — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-text keydown = [keydown 65 97]
    WAIT ok 14-text keydown = [keydown 49 49]
    WAIT ok 16-text keydown = [keydown 32 32]
    CHECK ok 17-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    WAIT ok 19-text keydown = [keydown 38 0]
    WAIT ok 21-text keydown = [keydown 39 0]
    WAIT ok 23-text keydown = [keydown 40 0]
    WAIT ok 25-text keydown = [keydown 37 0]
    CHECK ok 26-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 28-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / flash-input-lock — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main thread (the express l
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 97
    CHECK ok 10-card-scroll-min: the card's scroll y 97 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-settled: the card's scroll 0 0
    CHECK ok 19-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 20-number: n-move = 14 (wanted 5..400)
    WAIT ok 22-settled: the card's scroll 0 100
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 24-card-scroll-min: the card's scroll y 100 (at least 30)

### enyo / flash-input-tap — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-number: n-down = 1 (wanted 1..1)
    CHECK ok 11-number: n-up = 1 (wanted 1..1)
    CHECK ok 14-flash-at: the movie reported down 101,60 for stage 100,60 (within 3)
    CHECK ok 15-number: n-down = 2 (wanted 2..2)
    CHECK ok 16-number: n-up = 2 (wanted 2..2)
    WAIT ok 17-launch: load done
    WAIT ok 18-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 21-number: n-down = 1 (wanted 1..1)
    CHECK ok 22-number: n-up = 1 (wanted 1..1)

### enyo / flash-lane — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +2 s, load done at +10 s, 15 content frames at +10 s; the GPU path, frames off
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,123-440,421: 3.0 s, flips 42 (14.0/s), changes 40 (13.3/s), reads 43, distinct 41, spacing median 68 p90 94 max 192 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,123-440,421: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 65 p90 93 max 165 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,123-440,421: 3.0 s, flips 45 (15.0/s), changes 43 (14.3/s), reads 46, distinct 44, spacing median 68 p90 86 max 153 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 138
    CHECK ok 16-flash-rate: content 40,0-440,284: 3.0 s, flips 43 (14.3/s), changes 25 (8.3/s), reads 44, distinct 26, spacing median 67 p90 89 max 196 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,284: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 65 p90 109 max 146 ms (wanted 0..4/s)

### enyo / flash-layout — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the e
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
    WAIT ok 23-settled: the card's scroll 0 1389
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-rect show
    WAIT ok 29-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### enyo / flash-tags — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +9 s, 10 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (
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
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 52 (17.3/s), changes 43 (14.3/s), reads 53, distinct 44, spacing median 62 p90 74 max 95 ms (wanted 5..100/s)

### enyo / flash-wmode — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +2 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main threa
    WAIT ok 7-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 8-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 10-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 11-launch: load done
    WAIT ok 12-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 13-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 18-flash-behind: the colour 00ff00 in 188 of 192 cells (wanted yes)
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 21-launch: load done
    WAIT ok 22-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 23-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 28-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### enyo / flash-audio — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-audio-long.swf: connected at +2 s, load done at +10 s, 31 content frames at +10 s; the GPU path, frames off the main thread (the
    WAIT ok 7-rect m
    CHECK ok 8-audio: the movie's audio: open running
    WAIT ok 9-launch: load done
    CHECK ok 10-audio: the movie's audio: closed

### enyo / flash-drawn — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +2 s, load done at +9 s, 8 content frames at +9 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 9-flash-drawn: drawn: region 40,123-440,421: colours 32 hash 3dd6b7d0 rows 298 uniform 0 light none blue none
    WAIT ok 10-launch: load done
    WAIT ok 11-still m
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 40,123-440,421: colours 2 hash 5bd08ca5 rows 298 uniform 0 light 123-420 blue none
    WAIT ok 13-launch: load done
    WAIT ok 14-still m
    CHECK ok 15-flash-drawn: reported not drawn, as it should be: region 40,123-440,421: colours 2 hash 1cecd705 rows 298 uniform 0 light none blue none
    WAIT ok 16-launch: load done
    WAIT ok 17-still m
    CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,123-440,421: colours 2 hash 4db04545 rows 298 uniform 0 light none blue 123-420

### enyo / flash-plugin — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +3 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-rect m
    CHECK ok 8-plugin: plugin-container 26736: alive (state S, cpu ticks 93 to 101)
    CHECK ok 10-plugin: plugin-container 26736: wedged (state T, cpu ticks 101 to 101)
    CHECK ok 12-plugin: plugin-container 26736: alive (state S, cpu ticks 102 to 109)
    CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?)

### enyo / flash-rate — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate10.swf: connected at +2 s, load done at +10 s, 14 content frames at +10 s; the GPU path, frames off the main thread (t
    WAIT ok 7-rect m
    CHECK ok 9-flash-rate-report: content 40,123-440,421: 8.0 s, flips 85 (10.6/s), changes 78 (9.8/s), reads 89, distinct 61, spacing median 94 p90 117 max 229 ms; the movie's own rate 9.69556/s (frames 70 t=7747 .. frames 170 t=18061)
    WAIT ok 10-launch: load done
    WAIT ok 11-rect m
    CHECK ok 12-flash-rate-report: content 40,123-440,421: 8.0 s, flips 117 (14.6/s), changes 113 (14.1/s), reads 118, distinct 59, spacing median 67 p90 85 max 181 ms; the movie's own rate 14.3954/s (frames 40 t=3072 .. frames 190 t=13492)

### enyo / flash-reacted — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +2 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 11-flash-reacted: reacted: no (content 40,123-440,421: 4.0 s, flips 22 (5.5/s), changes 0 (0.0/s), reads 23, distinct 1, spacing median 181 p90 186 max 190 ms new 0 of 23)
    WAIT ok 12-launch: load done
    WAIT ok 13-rect m
    CHECK ok 16-flash-reacted: reacted: no (content 40,123-440,421: 5.0 s, flips 144 (28.8/s), changes 144 (28.8/s), reads 145, distinct 41, spacing median 35 p90 39 max 110 ms new 0 of 145)
    WAIT ok 17-launch: load done
    WAIT ok 18-rect m
    WAIT ok 19-still m
    CHECK ok 22-flash-reacted: reacted: yes (content 40,123-440,421: 3.0 s, flips 16 (5.3/s), changes 2 (0.7/s), reads 18, distinct 2, spacing median 183 p90 195 max 243 ms new 18 of 18 first 0 ms)

### enyo / flash-signature — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-reacts.swf: connected at +1 s, load done at +9 s, 11 content frames at +9 s; the GPU path, frames off the main thread (the
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 10-screen: the screen against its reference: difference 0.0 (same is 5 or less; wanted same)
    WAIT ok 12-still m
    CHECK ok 13-screen: the screen against its reference: difference 8.4 (same is 5 or less; wanted other)

### enyo / flash-stage-z1 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +2 s, load done at +9 s, 4 content frames at +9 s; the GPU path, frames off the main th
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,149.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 151.95,119.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,150.65 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 152,121.3 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,149.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 152,119.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3)

### enyo / flash-stage-z2 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +2 s, load done at +9 s, 10 content frames at +9 s; the GPU path, frames off the main t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-zoom
    CHECK ok 10-zoom: zoom 1.9326 (want 1.92 within 0.3)
    CHECK ok 13-flash-at: the movie reported down 199.95,147.95 for stage 200,150 (within 3)
    CHECK ok 15-flash-at: the movie reported down 149.95,117.95 for stage 150,120 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 19-zoom
    CHECK ok 20-zoom: zoom 1.9265 (want 1.92 within 0.3)
    CHECK ok 23-flash-at: the movie reported down 199.95,150.65 for stage 200,150 (within 3)
    CHECK ok 25-flash-at: the movie reported down 150.65,119.95 for stage 150,120 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 29-zoom
    CHECK ok 30-zoom: zoom 1.9268 (want 1.92 within 0.3)
    CHECK ok 33-flash-at: the movie reported down 199.95,149.95 for stage 200,150 (within 3)
    CHECK ok 35-flash-at: the movie reported down 150.65,119.95 for stage 150,120 (within 3)
    WAIT ok 36-launch: load done
    WAIT ok 37-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    WAIT ok 39-zoom
    CHECK ok 40-zoom: zoom 1.9265 (want 1.92 within 0.3)
    CHECK ok 43-flash-at: the movie reported down 200,150 for stage 200,150 (within 3)
    CHECK ok 45-flash-at: the movie reported down 150,120 for stage 150,120 (within 3)

### enyo / flash-stage-zfit — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150&fit=1: connected at +2 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the m
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,147.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 149.95,117.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,149.3 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 150.65,118.65 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,147.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 150.65,117.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,149 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3)

### mochi / flash-api — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +4 s, load done at +12 s, 4 content frames at +12 s; the GPU path
    WAIT ok 7-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 8-text vars = [vars a=hello]
    WAIT ok 9-text so = [so n=1]
    WAIT ok 10-text echoback = [echoback back hi]
    CHECK ok 11-text: echo = [echo hi]
    WAIT ok 12-text geturl = [geturl ok]
    WAIT ok 13-text fscommand = [fscommand jcmd jarg]
    WAIT ok 14-text data = [data true x=42]
    WAIT ok 15-text kid = [kid w=376]
    WAIT ok 17-text so = [so n=2]
    WAIT ok 18-launch: load done
    WAIT ok 19-rect m
    CHECK ok 20-text: count = [0]
    CHECK ok 22-text: count = [0]

### mochi / flash-fullscreen — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +2 s, load done at +10 s, 9 content frames at +10 s; the GPU path, frames off the ma
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes)

### mochi / flash-input-cover — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +2 s, load done at +10 s, 7 content frames at +10 s; the GPU path, frames off the main thread (the ex
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 10-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 225 px up for the keyboard, the page at 225
    WAIT ok 11-settled: the card's scroll 0 225
    CHECK ok 13-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mochi / flash-input-hold — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +10 s, 13 content frames at +10 s; the GPU path, frames off the main thread (the express lane
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-text up = [up 201 154]
    CHECK ok 13-number: heldms = 2296 (wanted 1500..3200)
    CHECK ok 14-number: n-down = 1 (wanted 1..1)
    CHECK ok 15-number: n-up = 1 (wanted 1..1)
    WAIT ok 17-text up = [up 361 282]
    CHECK ok 18-flash-el: the movie's move at 343,270 for 360,277 (within 30)
    CHECK ok 19-press-rate: 21.8 moves a second sent on (at least 20): ] a press of 1411 ms: 31 moves from the card in 1330 ms, 29 sent on, 490 ms in sending them

### mochi / flash-input-kernel — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +3 s, load done at +11 s, 9 content frames at +11 s; the GPU path, frames off the main thread (the expres
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-text keydown = [keydown 38 0]
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 40 0]
    CHECK ok 18-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 23-settled: the card's scroll 0 51
    CHECK ok 24-card-scroll-min: the card's scroll y 51 (at least 20)

### mochi / flash-input-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +3 s, load done at +11 s, 12 content frames at +11 s; the GPU path, frames off the main thread (the express lane
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-text keydown = [keydown 65 97]
    WAIT ok 14-text keydown = [keydown 49 49]
    WAIT ok 16-text keydown = [keydown 32 32]
    CHECK ok 17-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    WAIT ok 19-text keydown = [keydown 38 0]
    WAIT ok 21-text keydown = [keydown 39 0]
    WAIT ok 23-text keydown = [keydown 40 0]
    WAIT ok 25-text keydown = [keydown 37 0]
    CHECK ok 26-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 28-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / flash-input-lock — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +11 s, 12 content frames at +11 s; the GPU path, frames off the main thread (the expre
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 97
    CHECK ok 10-card-scroll-min: the card's scroll y 97 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-settled: the card's scroll 0 0
    CHECK ok 19-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 20-number: n-move = 15 (wanted 5..400)
    WAIT ok 22-settled: the card's scroll 0 97
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 24-card-scroll-min: the card's scroll y 97 (at least 30)

### mochi / flash-input-tap — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-number: n-down = 1 (wanted 1..1)
    CHECK ok 11-number: n-up = 1 (wanted 1..1)
    CHECK ok 14-flash-at: the movie reported down 101,60 for stage 100,60 (within 3)
    CHECK ok 15-number: n-down = 2 (wanted 2..2)
    CHECK ok 16-number: n-up = 2 (wanted 2..2)
    WAIT ok 17-launch: load done
    WAIT ok 18-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 21-number: n-down = 1 (wanted 1..1)
    CHECK ok 22-number: n-up = 1 (wanted 1..1)

### mochi / flash-lane — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +2 s, load done at +11 s, 17 content frames at +11 s; the GPU path, frames of
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,119-440,417: 3.0 s, flips 44 (14.7/s), changes 41 (13.7/s), reads 45, distinct 42, spacing median 66 p90 92 max 190 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,119-440,417: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 66 p90 84 max 187 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,119-440,417: 3.0 s, flips 43 (14.3/s), changes 40 (13.3/s), reads 44, distinct 41, spacing median 68 p90 81 max 149 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 138
    CHECK ok 16-flash-rate: content 40,0-440,280: 3.0 s, flips 43 (14.3/s), changes 26 (8.7/s), reads 44, distinct 27, spacing median 68 p90 75 max 168 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,280: 3.0 s, flips 44 (14.7/s), changes 0 (0.0/s), reads 45, distinct 1, spacing median 66 p90 141 max 190 ms (wanted 0..4/s)

### mochi / flash-layout — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the 
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
    WAIT ok 23-settled: the card's scroll 0 1385
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-rect show
    WAIT ok 29-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mochi / flash-tags — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +10 s, 10 content frames at +10 s; the GPU path, frames off the main thread (the express lane
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
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 44 (14.7/s), changes 44 (14.7/s), reads 45, distinct 45, spacing median 69 p90 76 max 87 ms (wanted 5..100/s)

### mochi / flash-wmode — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +2 s, load done at +9 s, 10 content frames at +9 s; the GPU path, frames off the main thr
    WAIT ok 7-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 8-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 10-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 11-launch: load done
    WAIT ok 12-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 13-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 18-flash-behind: the colour 00ff00 in 188 of 192 cells (wanted yes)
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 21-launch: load done
    WAIT ok 22-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 23-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 28-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mochi / flash-audio — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-audio-long.swf: connected at +2 s, load done at +9 s, 13 content frames at +9 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    CHECK ok 8-audio: the movie's audio: open running
    WAIT ok 9-launch: load done
    CHECK ok 10-audio: the movie's audio: closed

### mochi / flash-drawn — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +1 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 9-flash-drawn: drawn: region 40,119-440,417: colours 32 hash 3dd6b7d0 rows 298 uniform 0 light none blue none
    WAIT ok 10-launch: load done
    WAIT ok 11-still m
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 40,119-440,417: colours 2 hash 5bd08ca5 rows 298 uniform 0 light 119-416 blue none
    WAIT ok 13-launch: load done
    WAIT ok 14-still m
    CHECK ok 15-flash-drawn: reported not drawn, as it should be: region 40,119-440,417: colours 2 hash 1cecd705 rows 298 uniform 0 light none blue none
    WAIT ok 16-launch: load done
    WAIT ok 17-still m
    CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,119-440,417: colours 2 hash 4db04545 rows 298 uniform 0 light none blue 119-416

### mochi / flash-plugin — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +3 s, load done at +10 s, 7 content frames at +10 s; the GPU path, frames off the main thread (the express lane)
    WAIT ok 7-rect m
    CHECK ok 8-plugin: plugin-container 10801: alive (state S, cpu ticks 98 to 106)
    CHECK ok 10-plugin: plugin-container 10801: wedged (state T, cpu ticks 106 to 106)
    CHECK ok 12-plugin: plugin-container 10801: alive (state S, cpu ticks 106 to 114)
    CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?)

### mochi / flash-rate — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate10.swf: connected at +2 s, load done at +9 s, 12 content frames at +9 s; the GPU path, frames off the main thread (th
    WAIT ok 7-rect m
    CHECK ok 9-flash-rate-report: content 40,119-440,417: 8.0 s, flips 86 (10.8/s), changes 78 (9.8/s), reads 90, distinct 60, spacing median 93 p90 114 max 221 ms; the movie's own rate 9.78378/s (frames 50 t=5741 .. frames 150 t=15962)
    WAIT ok 10-launch: load done
    WAIT ok 11-rect m
    CHECK ok 12-flash-rate-report: content 40,119-440,417: 8.0 s, flips 118 (14.8/s), changes 113 (14.1/s), reads 119, distinct 73, spacing median 67 p90 86 max 188 ms; the movie's own rate 14.4157/s (frames 20 t=1798 .. frames 180 t=12897)

### mochi / flash-reacted — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +2 s, load done at +9 s, 3 content frames at +9 s; the GPU path, frames off the main thread (the
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 11-flash-reacted: reacted: no (content 40,119-440,417: 4.0 s, flips 22 (5.5/s), changes 1 (0.2/s), reads 24, distinct 2, spacing median 181 p90 188 max 210 ms new 0 of 24)
    WAIT ok 12-launch: load done
    WAIT ok 13-rect m
    CHECK ok 16-flash-reacted: reacted: no (content 40,119-440,417: 5.0 s, flips 141 (28.2/s), changes 141 (28.2/s), reads 142, distinct 47, spacing median 35 p90 39 max 74 ms new 0 of 142)
    WAIT ok 17-launch: load done
    WAIT ok 18-rect m
    WAIT ok 19-still m
    CHECK ok 22-flash-reacted: reacted: yes (content 40,119-440,417: 3.0 s, flips 16 (5.3/s), changes 2 (0.7/s), reads 18, distinct 2, spacing median 181 p90 189 max 246 ms new 18 of 18 first 0 ms)

### mochi / flash-signature — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-reacts.swf: connected at +1 s, load done at +9 s, 11 content frames at +9 s; the GPU path, frames off the main thread (th
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 10-screen: the screen against its reference: difference 0.0 (same is 5 or less; wanted same)
    WAIT ok 12-still m
    CHECK ok 13-screen: the screen against its reference: difference 8.3 (same is 5 or less; wanted other)

### mochi / flash-stage-z1 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +2 s, load done at +9 s, 8 content frames at +9 s; the GPU path, frames off the main t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,149.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 151.95,119.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,150.65 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 152,121.3 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,149.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 152,119.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3)

### mochi / flash-stage-z2 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +2 s, load done at +10 s, 7 content frames at +10 s; the GPU path, frames off the main
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-zoom
    CHECK ok 10-zoom: zoom 1.9245 (want 1.92 within 0.3)
    CHECK ok 13-flash-at: the movie reported down 199.95,149.95 for stage 200,150 (within 3)
    CHECK ok 15-flash-at: the movie reported down 149.95,119.95 for stage 150,120 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 19-zoom
    CHECK ok 20-zoom: zoom 1.9326 (want 1.92 within 0.3)
    CHECK ok 23-flash-at: the movie reported down 199.95,149.3 for stage 200,150 (within 3)
    CHECK ok 25-flash-at: the movie reported down 150.65,119.95 for stage 150,120 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 29-zoom
    CHECK ok 30-zoom: zoom 1.9265 (want 1.92 within 0.3)
    CHECK ok 33-flash-at: the movie reported down 199.95,149.95 for stage 200,150 (within 3)
    CHECK ok 35-flash-at: the movie reported down 150.65,119.95 for stage 150,120 (within 3)
    WAIT ok 36-launch: load done
    WAIT ok 37-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    WAIT ok 39-zoom
    CHECK ok 40-zoom: zoom 1.9403 (want 1.92 within 0.3)
    CHECK ok 43-flash-at: the movie reported down 200,150 for stage 200,150 (within 3)
    CHECK ok 45-flash-at: the movie reported down 150,120 for stage 150,120 (within 3)

### mochi / flash-stage-zfit — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150&fit=1: connected at +2 s, load done at +10 s, 10 content frames at +10 s; the GPU path, frames off t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,147.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 149.95,117.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,150.65 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 152,121.3 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,147.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 150.65,117.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,149 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3)

### mojo / flash-api — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +2 s, load done at +10 s, 8 content frames at +10 s; the GPU path,
    WAIT ok 7-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 8-text vars = [vars a=hello]
    WAIT ok 9-text so = [so n=1]
    WAIT ok 10-text echoback = [echoback back hi]
    CHECK ok 11-text: echo = [echo hi]
    WAIT ok 12-text geturl = [geturl ok]
    WAIT ok 13-text fscommand = [fscommand jcmd jarg]
    WAIT ok 14-text data = [data true x=42]
    WAIT ok 15-text kid = [kid w=376]
    WAIT ok 17-text so = [so n=2]
    WAIT ok 18-launch: load done
    WAIT ok 19-rect m
    CHECK ok 20-text: count = [0]
    CHECK ok 22-text: count = [0]

### mojo / flash-fullscreen — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main 
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 192 of 192 cells (wanted yes)

### mojo / flash-input-cover — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +1 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the expre
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 10-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 175 px up for the keyboard, the page at 175
    WAIT ok 11-settled: the card's scroll 0 175
    CHECK ok 13-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / flash-input-hold — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +8 s, 12 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-text up = [up 201 159]
    CHECK ok 13-number: heldms = 2322 (wanted 1500..3200)
    CHECK ok 14-number: n-down = 1 (wanted 1..1)
    CHECK ok 15-number: n-up = 1 (wanted 1..1)
    WAIT ok 17-text up = [up 361 287]
    CHECK ok 18-flash-el: the movie's move at 355,283 for 360,277 (within 30)
    CHECK ok 19-press-rate: 24.6 moves a second sent on (at least 20): ] a press of 1428 ms: 29 moves from the card in 1180 ms, 29 sent on, 457 ms in sending them

### mojo / flash-input-kernel — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +10 s, 9 content frames at +10 s; the GPU path, frames off the main thread (the express
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-text keydown = [keydown 38 0]
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 40 0]
    CHECK ok 18-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 23-settled: the card's scroll 0 51
    CHECK ok 24-card-scroll-min: the card's scroll y 51 (at least 20)

### mojo / flash-input-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-text keydown = [keydown 65 97]
    WAIT ok 14-text keydown = [keydown 49 49]
    WAIT ok 16-text keydown = [keydown 32 32]
    CHECK ok 17-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    WAIT ok 19-text keydown = [keydown 38 0]
    WAIT ok 21-text keydown = [keydown 39 0]
    WAIT ok 23-text keydown = [keydown 40 0]
    WAIT ok 25-text keydown = [keydown 37 0]
    CHECK ok 26-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 28-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / flash-input-lock — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +1 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express l
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 52
    CHECK ok 10-card-scroll-min: the card's scroll y 52 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-settled: the card's scroll 0 0
    CHECK ok 19-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 20-number: n-move = 14 (wanted 5..400)
    WAIT ok 22-settled: the card's scroll 0 100
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 24-card-scroll-min: the card's scroll y 100 (at least 30)

### mojo / flash-input-tap — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-number: n-down = 1 (wanted 1..1)
    CHECK ok 11-number: n-up = 1 (wanted 1..1)
    CHECK ok 14-flash-at: the movie reported down 101,60 for stage 100,60 (within 3)
    CHECK ok 15-number: n-down = 2 (wanted 2..2)
    CHECK ok 16-number: n-up = 2 (wanted 2..2)
    WAIT ok 17-launch: load done
    WAIT ok 18-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 21-number: n-down = 1 (wanted 1..1)
    CHECK ok 22-number: n-up = 1 (wanted 1..1)

### mojo / flash-lane — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +1 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off th
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,114-440,412: 3.0 s, flips 44 (14.7/s), changes 42 (14.0/s), reads 45, distinct 43, spacing median 67 p90 88 max 149 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,114-440,412: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 66 p90 85 max 150 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,114-440,412: 3.0 s, flips 45 (15.0/s), changes 42 (14.0/s), reads 46, distinct 43, spacing median 65 p90 84 max 175 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 95
    CHECK ok 16-flash-rate: content 40,0-440,273: 3.0 s, flips 44 (14.7/s), changes 37 (12.3/s), reads 46, distinct 38, spacing median 65 p90 82 max 220 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,273: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 67 p90 96 max 179 ms (wanted 0..4/s)

### mojo / flash-layout — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +1 s, load done at +8 s, 13 content frames at +8 s; the GPU path, frames off the main thread (the 
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
    WAIT ok 23-settled: the card's scroll 0 1335
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-rect show
    WAIT ok 29-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / flash-tags — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
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
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 43 (14.3/s), changes 43 (14.3/s), reads 44, distinct 44, spacing median 69 p90 84 max 121 ms (wanted 5..100/s)

### mojo / flash-wmode — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +1 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main threa
    WAIT ok 7-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 8-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 10-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 11-launch: load done
    WAIT ok 12-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 13-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 18-flash-behind: the colour 00ff00 in 188 of 192 cells (wanted yes)
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 21-launch: load done
    WAIT ok 22-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 23-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 28-flash-behind: the colour 00ff00 in 0 of 192 cells (wanted no)
    CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / flash-audio — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-audio-long.swf: connected at +1 s, load done at +8 s, 25 content frames at +8 s; the GPU path, frames off the main thread (the e
    WAIT ok 7-rect m
    CHECK ok 8-audio: the movie's audio: open running
    WAIT ok 9-launch: load done
    CHECK ok 10-audio: the movie's audio: closed

### mojo / flash-drawn — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 9-flash-drawn: drawn: region 40,114-440,412: colours 11 hash 899456dd rows 298 uniform 219 light none blue none
    WAIT ok 10-launch: load done
    WAIT ok 11-still m
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 40,114-440,412: colours 1 hash aec36065 rows 298 uniform 298 light 114-411 blue none
    WAIT ok 13-launch: load done
    WAIT ok 14-still m
    CHECK ok 15-flash-drawn: reported not drawn, as it should be: region 40,114-440,412: colours 1 hash ed7df245 rows 298 uniform 298 light none blue none
    WAIT ok 16-launch: load done
    WAIT ok 17-still m
    CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,114-440,412: colours 1 hash b955b145 rows 298 uniform 298 light none blue 114-411

### mojo / flash-plugin — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-rect m
    CHECK ok 8-plugin: plugin-container 26440: alive (state S, cpu ticks 94 to 101)
    CHECK ok 10-plugin: plugin-container 26440: wedged (state T, cpu ticks 101 to 101)
    CHECK ok 12-plugin: plugin-container 26440: alive (state S, cpu ticks 101 to 109)
    CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?)

### mojo / flash-rate — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate10.swf: connected at +1 s, load done at +8 s, 15 content frames at +8 s; the GPU path, frames off the main thread (the
    WAIT ok 7-rect m
    CHECK ok 9-flash-rate-report: content 40,114-440,412: 8.0 s, flips 84 (10.5/s), changes 80 (10.0/s), reads 86, distinct 62, spacing median 92 p90 117 max 216 ms; the movie's own rate 9.74237/s (frames 40 t=4617 .. frames 130 t=13855)
    WAIT ok 10-launch: load done
    WAIT ok 11-rect m
    CHECK ok 12-flash-rate-report: content 40,114-440,412: 8.0 s, flips 118 (14.8/s), changes 115 (14.4/s), reads 119, distinct 61, spacing median 68 p90 87 max 158 ms; the movie's own rate 14.3805/s (frames 30 t=2468 .. frames 160 t=11508)

### mojo / flash-reacted — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +1 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 11-flash-reacted: reacted: no (content 40,114-440,412: 4.0 s, flips 20 (5.0/s), changes 0 (0.0/s), reads 27, distinct 1, spacing median 207 p90 215 max 221 ms new 0 of 27)
    WAIT ok 12-launch: load done
    WAIT ok 13-rect m
    CHECK ok 16-flash-reacted: reacted: no (content 40,114-440,412: 5.0 s, flips 142 (28.4/s), changes 142 (28.4/s), reads 143, distinct 41, spacing median 35 p90 40 max 95 ms new 0 of 143)
    WAIT ok 17-launch: load done
    WAIT ok 18-rect m
    WAIT ok 19-still m
    CHECK ok 22-flash-reacted: reacted: yes (content 40,114-440,412: 3.0 s, flips 15 (5.0/s), changes 0 (0.0/s), reads 22, distinct 1, spacing median 204 p90 210 max 210 ms new 22 of 22 first 0 ms)

### mojo / flash-signature — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-reacts.swf: connected at +1 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 10-screen: the screen against its reference: difference 0.0 (same is 5 or less; wanted same)
    WAIT ok 12-still m
    CHECK ok 13-screen: the screen against its reference: difference 8.3 (same is 5 or less; wanted other)

### mojo / flash-stage-z1 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +1 s, load done at +7 s, 10 content frames at +8 s; the GPU path, frames off the main t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,149.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 151.95,119.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,150.65 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 152,121.3 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,149.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 152,119.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3)

### mojo / flash-stage-z2 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +1 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-zoom
    CHECK ok 10-zoom: zoom 1.9416 (want 1.92 within 0.3)
    CHECK ok 13-flash-at: the movie reported down 199.95,149.95 for stage 200,150 (within 3)
    CHECK ok 15-flash-at: the movie reported down 149.95,117.95 for stage 150,120 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 19-zoom
    CHECK ok 20-zoom: zoom 1.9326 (want 1.92 within 0.3)
    CHECK ok 23-flash-at: the movie reported down 199.95,149.3 for stage 200,150 (within 3)
    CHECK ok 25-flash-at: the movie reported down 150.65,119.95 for stage 150,120 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 29-zoom
    CHECK ok 30-zoom: zoom 1.9326 (want 1.92 within 0.3)
    CHECK ok 33-flash-at: the movie reported down 199.95,147.95 for stage 200,150 (within 3)
    CHECK ok 35-flash-at: the movie reported down 150.65,117.95 for stage 150,120 (within 3)
    WAIT ok 36-launch: load done
    WAIT ok 37-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    WAIT ok 39-zoom
    CHECK ok 40-zoom: zoom 1.9326 (want 1.92 within 0.3)
    CHECK ok 43-flash-at: the movie reported down 200,149 for stage 200,150 (within 3)
    CHECK ok 45-flash-at: the movie reported down 150,119 for stage 150,120 (within 3)

### mojo / flash-stage-zfit — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150&fit=1: connected at +1 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the m
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,147.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 149.95,117.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,149.3 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 150.65,118.65 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,147.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 150.65,117.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,149 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3)

