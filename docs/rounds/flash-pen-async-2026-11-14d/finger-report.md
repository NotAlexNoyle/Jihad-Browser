# Finger suite report — 2026-11-14T07:49:27

Shells: enyo, mochi, mojo. Scripts: 14. 3967 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| enyo | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes); frames: kept/enyo-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| enyo | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/enyo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| enyo | flash-input-cover | **fail** | 6 ok, 1 failed | CHECK FAIL 18-flash-at: the movie reported down 201,80 for stage 200,150 (more than 3 off); frames: kept/enyo-flash-input-cover-18-flash-at_.jpg | cavekit-flash.md R5 |
| enyo | flash-input-hold | **fail** | 7 ok, 1 failed | CHECK FAIL 15-flash-el: the movie's move at [325,254] for 360,277 (more than 30 off); frames: kept/enyo-flash-input-hold-15-flash-el_.jpg | cavekit-flash.md R5 |
| enyo | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| enyo | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R6 |
| enyo | flash-input-keys | **pass** | 13 ok, 0 failed | CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R5 |
| enyo | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 98 (at least 30) | cavekit-flash.md R5 |
| enyo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| enyo | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,282: 3.0 s, flips 44 (14.7/s), changes 0 (0.0/s), reads 45, distinct 1, spacing median 66 p90 107 max 191 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| enyo | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| enyo | flash-tags | **fail** | 14 ok, 1 failed | CHECK FAIL 9-flash-at: the movie reported no down: [-]; frames: kept/enyo-flash-tags-9-flash-at_.jpg | cavekit-flash.md R4 |
| enyo | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mochi | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mochi | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes); frames: kept/mochi-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| mochi | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/mochi-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mochi | flash-input-cover | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mochi | flash-input-hold | **fail** | 7 ok, 1 failed | CHECK FAIL 15-flash-el: the movie's move at [316,252] for 360,277 (more than 30 off); frames: kept/mochi-flash-input-hold-15-flash-el_.jpg | cavekit-flash.md R5 |
| mochi | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mochi | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R6 |
| mochi | flash-input-keys | **pass** | 13 ok, 0 failed | CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mochi | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 98 (at least 30) | cavekit-flash.md R5 |
| mochi | flash-input-tap | **fail** | 10 ok, 2 failed | CHECK FAIL 14-flash-at: the movie reported down 201,150 for stage 100,60 (more than 3 off) / CHECK FAIL 15-number: n-down = [1] (wanted 2..2); frames: kept/mochi-flash-input-tap-14-flash-at_.jpg, kept/mochi-flash-input-tap-15-number_.jpg | cavekit-flash.md R5 |
| mochi | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,62-440,360: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 67 p90 116 max 175 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mochi | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mochi | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 44 (14.7/s), changes 42 (14.0/s), reads 45, distinct 43, spacing median 67 p90 75 max 84 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mochi | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mojo | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mojo | flash-fullscreen | **pass** | 8 ok, 0 failed | CHECK ok 20-flash-behind: the colour 00ff00 in 192 of 192 cells (wanted yes); frames: kept/mojo-flash-fullscreen-fullscreen_.jpg | cavekit-flash.md R4 |
| mojo | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/mojo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mojo | flash-input-cover | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mojo | flash-input-hold | **pass** | 8 ok, 0 failed | CHECK ok 16-press-rate: 25.6 moves a second sent on (at least 20): ] a press of 1441 ms: 33 moves from the card in 1289 ms, 33 sent on, 130 ms in sending them | cavekit-flash.md R5 |
| mojo | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mojo | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R6 |
| mojo | flash-input-keys | **pass** | 13 ok, 0 failed | CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mojo | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 97 (at least 30) | cavekit-flash.md R5 |
| mojo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mojo | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,275: 3.0 s, flips 44 (14.7/s), changes 0 (0.0/s), reads 45, distinct 1, spacing median 65 p90 87 max 170 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mojo | flash-layout | **pass** | 17 ok, 0 failed | CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |
| mojo | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 45 (15.0/s), changes 44 (14.7/s), reads 46, distinct 45, spacing median 69 p90 80 max 102 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mojo | flash-wmode | **pass** | 19 ok, 0 failed | CHECK ok 30-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R4 |

## The device at the end

```
DEVICE ok job-enyo: (188550/139603918) jihad (start) running, process 10606
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (188558/184172214) jihad-mochi (start) running, process 2513
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (188562/919689101) jihad-mojo (start) running, process 25791
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
DEVICE FAIL tmp-dumps: /tmp/rw.png /tmp/sb-probe.png /tmp/sb-probe2.png /tmp/wb.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 16% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### enyo / flash-api — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, f
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

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main 
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes)

### enyo / flash-game-sound — pass

    enyo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +2 s, load done at +10 s, 16 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### enyo / flash-input-cover — fail

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +2 s, load done at +8 s, 12 content frames at +8 s; the GPU path, frames off the main thread (the expr
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 229 px up for the keyboard, the page at 229
    WAIT ok 14-settled: the card's scroll 0 229
    CHECK ok 16-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK FAIL 18-flash-at: the movie reported down 201,80 for stage 200,150 (more than 3 off)

### enyo / flash-input-hold — fail

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 150]
    CHECK ok 10-number: heldms = 2334 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 14-text up = [up 361 278]
    CHECK FAIL 15-flash-el: the movie's move at [325,254] for 360,277 (more than 30 off)
    CHECK ok 16-press-rate: 27.2 moves a second sent on (at least 20): ] a press of 1370 ms: 32 moves from the card in 1178 ms, 32 sent on, 106 ms in sending them

### enyo / flash-input-kernel — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +8 s, 14 content frames at +8 s; the GPU path, frames off the main thread (the express 
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 10-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 13-text keydown = [keydown 38 0]
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 40 0]
    CHECK ok 18-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 23-settled: the card's scroll 0 51
    CHECK ok 24-card-scroll-min: the card's scroll y 51 (at least 20)

### enyo / flash-input-keyhold — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 152 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 82 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / flash-input-keys — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +8 s, 13 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 49 49]
    WAIT ok 19-text keydown = [keydown 32 32]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    WAIT ok 22-text keydown = [keydown 38 0]
    WAIT ok 24-text keydown = [keydown 39 0]
    WAIT ok 26-text keydown = [keydown 40 0]
    WAIT ok 28-text keydown = [keydown 37 0]
    CHECK ok 29-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / flash-input-lock — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express l
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 97
    CHECK ok 10-card-scroll-min: the card's scroll y 97 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-number: n-down = 1 (wanted 1..1)
    CHECK ok 16-number: n-up = 1 (wanted 1..1)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 22-number: n-move = 11 (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 98
    CHECK ok 31-card-scroll-min: the card's scroll y 98 (at least 30)

### enyo / flash-input-tap — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +3 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
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

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off th
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,123-440,421: 3.0 s, flips 42 (14.0/s), changes 40 (13.3/s), reads 43, distinct 41, spacing median 67 p90 80 max 153 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,123-440,421: 3.0 s, flips 44 (14.7/s), changes 0 (0.0/s), reads 45, distinct 1, spacing median 65 p90 83 max 195 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,123-440,421: 3.0 s, flips 45 (15.0/s), changes 43 (14.3/s), reads 46, distinct 44, spacing median 67 p90 87 max 153 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 140
    CHECK ok 16-flash-rate: content 40,0-440,282: 3.0 s, flips 44 (14.7/s), changes 43 (14.3/s), reads 45, distinct 44, spacing median 66 p90 82 max 148 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,282: 3.0 s, flips 44 (14.7/s), changes 0 (0.0/s), reads 45, distinct 1, spacing median 66 p90 107 max 191 ms (wanted 0..4/s)

### enyo / flash-layout — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +2 s, load done at +7 s, 10 content frames at +7 s; the GPU path, frames off the main thread (the 
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
    WAIT ok 23-settled: the card's scroll 0 1400
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-rect show
    WAIT ok 29-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### enyo / flash-tags — fail

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK FAIL 9-flash-at: the movie reported no down: [-]
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
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 63 (21.0/s), changes 44 (14.7/s), reads 64, distinct 45, spacing median 40 p90 72 max 108 ms (wanted 5..100/s)

### enyo / flash-wmode — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +2 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thre
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

### mochi / flash-api — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +3 s, load done at +9 s, 5 content frames at +9 s; the GPU path, 
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

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 180 of 192 cells (wanted yes)

### mochi / flash-game-sound — pass

    mochi card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +3 s, load done at +10 s, 11 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### mochi / flash-input-cover — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the exp
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 225 px up for the keyboard, the page at 225
    WAIT ok 14-settled: the card's scroll 0 225
    CHECK ok 16-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mochi / flash-input-hold — fail

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 154]
    CHECK ok 10-number: heldms = 2296 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 14-text up = [up 361 282]
    CHECK FAIL 15-flash-el: the movie's move at [316,252] for 360,277 (more than 30 off)
    CHECK ok 16-press-rate: 26.6 moves a second sent on (at least 20): ] a press of 1434 ms: 34 moves from the card in 1278 ms, 34 sent on, 194 ms in sending them

### mochi / flash-input-kernel — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +3 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express 
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 10-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-text keydown = [keydown 38 0]
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 40 0]
    CHECK ok 18-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 23-settled: the card's scroll 0 51
    CHECK ok 24-card-scroll-min: the card's scroll y 51 (at least 20)

### mochi / flash-input-keyhold — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 84 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 60 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / flash-input-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 49 49]
    WAIT ok 19-text keydown = [keydown 32 32]
    CHECK ok 20-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    WAIT ok 22-text keydown = [keydown 38 0]
    WAIT ok 24-text keydown = [keydown 39 0]
    WAIT ok 26-text keydown = [keydown 40 0]
    WAIT ok 28-text keydown = [keydown 37 0]
    CHECK ok 29-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 31-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / flash-input-lock — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +1 s, load done at +7 s, 12 content frames at +7 s; the GPU path, frames off the main thread (the express
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 98
    CHECK ok 10-card-scroll-min: the card's scroll y 98 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-number: n-down = 1 (wanted 1..1)
    CHECK ok 16-number: n-up = 1 (wanted 1..1)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 22-number: n-move = 10 (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 98
    CHECK ok 31-card-scroll-min: the card's scroll y 98 (at least 30)

### mochi / flash-input-tap — fail

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 12 content frames at +7 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-number: n-down = 1 (wanted 1..1)
    CHECK ok 11-number: n-up = 1 (wanted 1..1)
    CHECK FAIL 14-flash-at: the movie reported down 201,150 for stage 100,60 (more than 3 off)
    CHECK FAIL 15-number: n-down = [1] (wanted 2..2)
    CHECK ok 16-number: n-up = 2 (wanted 2..2)
    WAIT ok 17-launch: load done
    WAIT ok 18-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 21-number: n-down = 1 (wanted 1..1)
    CHECK ok 22-number: n-up = 1 (wanted 1..1)

### mochi / flash-lane — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +1 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off t
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,119-440,417: 3.0 s, flips 44 (14.7/s), changes 41 (13.7/s), reads 45, distinct 42, spacing median 67 p90 90 max 164 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,119-440,417: 3.0 s, flips 40 (13.3/s), changes 0 (0.0/s), reads 41, distinct 1, spacing median 66 p90 136 max 182 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,119-440,417: 3.0 s, flips 44 (14.7/s), changes 42 (14.0/s), reads 45, distinct 43, spacing median 67 p90 86 max 161 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 58
    CHECK ok 16-flash-rate: content 40,62-440,360: 3.0 s, flips 43 (14.3/s), changes 42 (14.0/s), reads 46, distinct 43, spacing median 67 p90 82 max 234 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,62-440,360: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 67 p90 116 max 175 ms (wanted 0..4/s)

### mochi / flash-layout — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=percent: connected at +2 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the 
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
    WAIT ok 23-settled: the card's scroll 0 1400
    CHECK ok 25-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-rect show
    WAIT ok 29-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 31-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mochi / flash-tags — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (
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
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 44 (14.7/s), changes 42 (14.0/s), reads 45, distinct 43, spacing median 67 p90 75 max 84 ms (wanted 5..100/s)

### mochi / flash-wmode — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +2 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the main thr
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

### mojo / flash-api — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, f
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

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&vars=fs=1: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main 
    WAIT ok 8-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 9-text kid = [kid w=376]
    WAIT ok 11-text fs = [fs fullScreen]
    WAIT ok 14-text fs = [fs normal]
    WAIT ok 17-text fs = [fs fullScreen]
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 20-flash-behind: the colour 00ff00 in 192 of 192 cells (wanted yes)

### mojo / flash-game-sound — pass

    mojo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +2 s, load done at +9 s, 11 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### mojo / flash-input-cover — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the expre
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 175 px up for the keyboard, the page at 175
    WAIT ok 14-settled: the card's scroll 0 175
    CHECK ok 16-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / flash-input-hold — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 159]
    CHECK ok 10-number: heldms = 2482 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 14-text up = [up 361 287]
    CHECK ok 15-flash-el: the movie's move at 337,271 for 360,277 (within 30)
    CHECK ok 16-press-rate: 25.6 moves a second sent on (at least 20): ] a press of 1441 ms: 33 moves from the card in 1289 ms, 33 sent on, 130 ms in sending them

### mojo / flash-input-kernel — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the main thread (the express 
    WAIT ok 8-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 10-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-text keydown = [keydown 38 0]
    WAIT ok 15-text keydown = [keydown 65 97]
    WAIT ok 17-text keydown = [keydown 40 0]
    CHECK ok 18-card-scroll: the card's scroll y 0 (want 0 within 5); the engine's 0
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 23-settled: the card's scroll 0 51
    CHECK ok 24-card-scroll-min: the card's scroll y 51 (at least 20)

### mojo / flash-input-keyhold — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 110 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 200 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / flash-input-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
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

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express l
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 48
    CHECK ok 10-card-scroll-min: the card's scroll y 48 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-number: n-down = 1 (wanted 1..1)
    CHECK ok 16-number: n-up = 1 (wanted 1..1)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 22-number: n-move = 9 (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 97
    CHECK ok 31-card-scroll-min: the card's scroll y 97 (at least 30)

### mojo / flash-input-tap — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
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

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +1 s, load done at +6 s, 8 content frames at +6 s; the GPU path, frames off th
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,114-440,412: 3.0 s, flips 43 (14.3/s), changes 42 (14.0/s), reads 44, distinct 43, spacing median 66 p90 105 max 155 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,114-440,412: 3.0 s, flips 40 (13.3/s), changes 0 (0.0/s), reads 41, distinct 1, spacing median 67 p90 128 max 146 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,114-440,412: 3.0 s, flips 45 (15.0/s), changes 43 (14.3/s), reads 46, distinct 44, spacing median 66 p90 86 max 156 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 93
    CHECK ok 16-flash-rate: content 40,0-440,275: 3.0 s, flips 45 (15.0/s), changes 35 (11.7/s), reads 46, distinct 36, spacing median 67 p90 78 max 144 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,275: 3.0 s, flips 44 (14.7/s), changes 0 (0.0/s), reads 45, distinct 1, spacing median 65 p90 87 max 170 ms (wanted 0..4/s)

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

### mojo / flash-tags — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
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
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 45 (15.0/s), changes 44 (14.7/s), reads 46, distinct 45, spacing median 69 p90 80 max 102 ms (wanted 5..100/s)

### mojo / flash-wmode — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm1&bg=00ff00&wmode=window: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main threa
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

