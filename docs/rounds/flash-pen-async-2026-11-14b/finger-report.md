# Finger suite report — 2026-11-14T05:34:14

Shells: enyo, mochi, mojo. Scripts: 7. 1915 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| enyo | flash-input-cover | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| enyo | flash-input-hold | **fail** | 6 ok, 2 failed | CHECK FAIL 15-flash-el: the movie's move at [325,254] for 360,277 (more than 30 off) / CHECK FAIL 16-press-rate: 11.6 moves a second sent on (at least 20): ] a press of 1464 ms: 34 moves from the card in 1382 ms, 16 sent on, 390 ms in sending them; frames: kept/enyo-flash-input-hold-15-flash-el_.jpg, kept/enyo-flash-input-hold-16-press-rate_.jpg | cavekit-flash.md R5 |
| enyo | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| enyo | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R6 |
| enyo | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 98 (at least 30) | cavekit-flash.md R5 |
| enyo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mochi | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mochi | flash-input-cover | **fail** | 5 ok, 2 failed | CHECK FAIL 16-above-keyboard: m on the screen at 40,598-440,898, under the keyboard / CHECK FAIL 18-flash-at: the movie reported down 201,84 for stage 200,150 (more than 3 off); frames: kept/mochi-flash-input-cover-16-above-keyboard_.jpg, kept/mochi-flash-input-cover-18-flash-at_.jpg | cavekit-flash.md R5 |
| mochi | flash-input-hold | **fail** | 7 ok, 1 failed | CHECK FAIL 16-press-rate: 11.9 moves a second sent on (at least 20): ] a press of 1516 ms: 33 moves from the card in 1341 ms, 16 sent on, 444 ms in sending them; frames: kept/mochi-flash-input-hold-16-press-rate_.jpg | cavekit-flash.md R5 |
| mochi | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mochi | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R6 |
| mochi | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 98 (at least 30) | cavekit-flash.md R5 |
| mochi | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mojo | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mojo | flash-input-cover | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mojo | flash-input-hold | **fail** | 7 ok, 1 failed | CHECK FAIL 16-press-rate: 14.2 moves a second sent on (at least 20): ] a press of 1422 ms: 31 moves from the card in 1195 ms, 17 sent on, 344 ms in sending them; frames: kept/mojo-flash-input-hold-16-press-rate_.jpg | cavekit-flash.md R5 |
| mojo | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mojo | flash-input-keyhold | **pass** | 7 ok, 0 failed | CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R6 |
| mojo | flash-input-lock | **pass** | 15 ok, 0 failed | CHECK ok 31-card-scroll-min: the card's scroll y 100 (at least 30) | cavekit-flash.md R5 |
| mojo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |

## The device at the end

```
DEVICE ok job-enyo: (178385/37915518) jihad (start) running, process 19634
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (178394/135776997) jihad-mochi (start) running, process 16073
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (178399/328158034) jihad-mojo (start) running, process 11736
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

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, f
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

### enyo / flash-input-cover — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the expre
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 229 px up for the keyboard, the page at 229
    WAIT ok 14-settled: the card's scroll 0 229
    CHECK ok 16-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### enyo / flash-input-hold — fail

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 150]
    CHECK ok 10-number: heldms = 2358 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 14-text up = [up 361 278]
    CHECK FAIL 15-flash-el: the movie's move at [325,254] for 360,277 (more than 30 off)
    CHECK FAIL 16-press-rate: 11.6 moves a second sent on (at least 20): ] a press of 1464 ms: 34 moves from the card in 1382 ms, 16 sent on, 390 ms in sending them

### enyo / flash-input-kernel — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the express 
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

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 98 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 120 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / flash-input-lock — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +8 s, 12 content frames at +8 s; the GPU path, frames off the main thread (the express 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 98
    CHECK ok 10-card-scroll-min: the card's scroll y 98 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-number: n-down = 1 (wanted 1..1)
    CHECK ok 16-number: n-up = 1 (wanted 1..1)
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 20-settled: the card's scroll 0 0
    CHECK ok 21-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 22-number: n-move = 12 (wanted 5..400)
    CHECK ok 24-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 27-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 30-settled: the card's scroll 0 98
    CHECK ok 31-card-scroll-min: the card's scroll y 98 (at least 30)

### enyo / flash-input-tap — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
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

### mochi / flash-api — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +4 s, load done at +10 s, 6 content frames at +10 s; the GPU path
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

### mochi / flash-input-cover — fail

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +1 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the expr
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 225 px up for the keyboard, the page at 225
    WAIT ok 14-settled: the card's scroll 0 0
    CHECK FAIL 16-above-keyboard: m on the screen at 40,598-440,898, under the keyboard
    CHECK FAIL 18-flash-at: the movie reported down 201,84 for stage 200,150 (more than 3 off)

### mochi / flash-input-hold — fail

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 154]
    CHECK ok 10-number: heldms = 2230 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 14-text up = [up 361 282]
    CHECK ok 15-flash-el: the movie's move at 355,278 for 360,277 (within 30)
    CHECK FAIL 16-press-rate: 11.9 moves a second sent on (at least 20): ] a press of 1516 ms: 33 moves from the card in 1341 ms, 16 sent on, 444 ms in sending them

### mochi / flash-input-kernel — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express 
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

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +3 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 110 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 94 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / flash-input-lock — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express 
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

### mochi / flash-input-tap — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the express lane) 
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

### mojo / flash-api — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +2 s, load done at +8 s, 9 content frames at +8 s; the GPU path, f
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

### mojo / flash-input-cover — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +1 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the expr
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 12-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 175 px up for the keyboard, the page at 175
    WAIT ok 14-settled: the card's scroll 0 175
    CHECK ok 16-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 18-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / flash-input-hold — fail

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-text up = [up 201 159]
    CHECK ok 10-number: heldms = 2428 (wanted 1500..3200)
    CHECK ok 11-number: n-down = 1 (wanted 1..1)
    CHECK ok 12-number: n-up = 1 (wanted 1..1)
    WAIT ok 14-text up = [up 361 287]
    CHECK ok 15-flash-el: the movie's move at 352,281 for 360,277 (within 30)
    CHECK FAIL 16-press-rate: 14.2 moves a second sent on (at least 20): ] a press of 1422 ms: 31 moves from the card in 1195 ms, 17 sent on, 344 ms in sending them

### mojo / flash-input-kernel — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express l
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

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 14-text keyup = [keyup 65 97]
    CHECK ok 15-number: keyheldms = 84 (wanted 60..2000)
    WAIT ok 17-text keyup = [keyup 39 0]
    CHECK ok 18-number: keyheldms = 132 (wanted 60..2000)
    CHECK ok 20-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / flash-input-lock — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express l
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 45
    CHECK ok 10-card-scroll-min: the card's scroll y 45 (at least 30)
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
    WAIT ok 30-settled: the card's scroll 0 100
    CHECK ok 31-card-scroll-min: the card's scroll y 100 (at least 30)

### mojo / flash-input-tap — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
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

