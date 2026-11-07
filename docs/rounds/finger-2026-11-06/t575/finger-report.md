# Finger suite report — 2026-11-06T22:33:13

First run 2026-11-06T21:23:49; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 1. 3479 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | flash-input-cover (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 14-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| enyo | flash-input-hold (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 19-press-rate: 21.1 moves a second sent on (at least 20): ] a press of 1553 ms: 32 moves from the card in 1466 ms, 31 sent on, 598 ms in sending them | cavekit-flash.md R5 |
| enyo | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| enyo | flash-input-keys | **pass** | 12 ok, 0 failed | CHECK ok 28-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-flash.md R5 |
| enyo | flash-input-lock (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 24-card-scroll-min: the card's scroll y 100 (at least 30) | cavekit-flash.md R5 |
| enyo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mochi | flash-input-cover (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 14-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mochi | flash-input-hold (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 19-press-rate: 27.0 moves a second sent on (at least 20): ] a press of 1501 ms: 33 moves from the card in 1224 ms, 33 sent on, 459 ms in sending them | cavekit-flash.md R5 |
| mochi | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mochi | flash-input-keys | **pass** | 12 ok, 0 failed | CHECK ok 28-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mochi | flash-input-lock (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 24-card-scroll-min: the card's scroll y 100 (at least 30) | cavekit-flash.md R5 |
| mochi | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mojo | flash-input-cover (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3) | cavekit-flash.md R5 |
| mojo | flash-input-hold (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 19-press-rate: 27.2 moves a second sent on (at least 20): ] a press of 1435 ms: 33 moves from the card in 1215 ms, 33 sent on, 709 ms in sending them | cavekit-flash.md R5 |
| mojo | flash-input-kernel | **pass** | 9 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R5 |
| mojo | flash-input-keys | **pass** | 12 ok, 0 failed | CHECK ok 28-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-flash.md R5 |
| mojo | flash-input-lock (rerun) | **pass** | 12 ok, 0 failed | CHECK ok 24-card-scroll-min: the card's scroll y 98 (at least 30) | cavekit-flash.md R5 |
| mojo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |

## The device at the end

```
DEVICE ok job-enyo: (1257174/975800718) jihad (start) running, process 5752
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (1257184/745484271) jihad-mochi (start) running, process 12107
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (1257194/560900270) jihad-mojo (start) running, process 28284
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

### enyo / flash-input-cover — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +2 s, load done at +10 s, 11 content frames at +10 s; the GPU path, frames off the main thread (the ex
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 10-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 229 px up for the keyboard, the page at 229
    WAIT ok 11-settled: the card's scroll 0 229
    CHECK ok 12-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 14-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### enyo / flash-input-hold — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +10 s, 6 content frames at +10 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 12-text up = [up 201 150]
    CHECK ok 13-number: heldms = 2234 (wanted 1500..3200)
    CHECK ok 14-number: n-down = 1 (wanted 1..1)
    CHECK ok 15-number: n-up = 1 (wanted 1..1)
    WAIT ok 17-text up = [up 361 278]
    CHECK ok 18-flash-el: the movie's move at 349,270 for 360,277 (within 30)
    CHECK ok 19-press-rate: 21.1 moves a second sent on (at least 20): ] a press of 1553 ms: 32 moves from the card in 1466 ms, 31 sent on, 598 ms in sending them

### enyo / flash-input-kernel — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +10 s, 11 content frames at +10 s; the GPU path, frames off the main thread (the expres
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

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +12 s, 11 content frames at +12 s; the GPU path, frames off the main thread (the express lane)
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

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +3 s, load done at +11 s, 6 content frames at +11 s; the GPU path, frames off the main thread (the express
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 100
    CHECK ok 10-card-scroll-min: the card's scroll y 100 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-settled: the card's scroll 0 0
    CHECK ok 19-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 20-number: n-move = 11 (wanted 5..400)
    WAIT ok 22-settled: the card's scroll 0 100
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 24-card-scroll-min: the card's scroll y 100 (at least 30)

### enyo / flash-input-tap — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +9 s, 10 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (
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

### mochi / flash-input-cover — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +3 s, load done at +11 s, 11 content frames at +11 s; the GPU path, frames off the main thread (the e
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 10-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 225 px up for the keyboard, the page at 225
    WAIT ok 11-settled: the card's scroll 0 225
    CHECK ok 12-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 14-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mochi / flash-input-hold — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +10 s, 8 content frames at +10 s; the GPU path, frames off the main thread (the express lane)
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-text up = [up 201 154]
    CHECK ok 13-number: heldms = 2172 (wanted 1500..3200)
    CHECK ok 14-number: n-down = 1 (wanted 1..1)
    CHECK ok 15-number: n-up = 1 (wanted 1..1)
    WAIT ok 17-text up = [up 361 282]
    CHECK ok 18-flash-el: the movie's move at 358,280 for 360,277 (within 30)
    CHECK ok 19-press-rate: 27.0 moves a second sent on (at least 20): ] a press of 1501 ms: 33 moves from the card in 1224 ms, 33 sent on, 459 ms in sending them

### mochi / flash-input-kernel — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +3 s, load done at +12 s, 6 content frames at +12 s; the GPU path, frames off the main thread (the expres
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

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +3 s, load done at +10 s, 4 content frames at +10 s; the GPU path, frames off the main thread (the express lane)
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

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main thread (the express 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 97
    CHECK ok 10-card-scroll-min: the card's scroll y 97 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-settled: the card's scroll 0 0
    CHECK ok 19-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 20-number: n-move = 13 (wanted 5..400)
    WAIT ok 22-settled: the card's scroll 0 100
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 24-card-scroll-min: the card's scroll y 100 (at least 30)

### mochi / flash-input-tap — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +10 s, 3 content frames at +10 s; the GPU path, frames off the main thread (the express lane)
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

### mojo / flash-input-cover — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&layout=low: connected at +1 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the expr
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 10-line: [jihad-bs] js-console (no source):0:0 page: the movie moved 175 px up for the keyboard, the page at 175
    WAIT ok 11-settled: the card's scroll 0 175
    CHECK ok 13-above-keyboard: m on the screen at 40,373-440,673, above the keyboard
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)

### mojo / flash-input-hold — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-text up = [up 201 159]
    CHECK ok 13-number: heldms = 2310 (wanted 1500..3200)
    CHECK ok 14-number: n-down = 1 (wanted 1..1)
    CHECK ok 15-number: n-up = 1 (wanted 1..1)
    WAIT ok 17-text up = [up 361 287]
    CHECK ok 18-flash-el: the movie's move at 346,277 for 360,277 (within 30)
    CHECK ok 19-press-rate: 27.2 moves a second sent on (at least 20): ] a press of 1435 ms: 33 moves from the card in 1215 ms, 33 sent on, 709 ms in sending them

### mojo / flash-input-kernel — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +2 s, load done at +10 s, 11 content frames at +10 s; the GPU path, frames off the main thread (the expres
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

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +10 s, 8 content frames at +10 s; the GPU path, frames off the main thread (the express lane) 
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

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&over=1: connected at +1 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main thread (the express l
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-settled: the card's scroll 0 52
    CHECK ok 10-card-scroll-min: the card's scroll y 52 (at least 30)
    CHECK ok 11-number: n-move = 0 (wanted 0..0)
    WAIT ok 13-settled: the card's scroll 0 0
    CHECK ok 15-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-settled: the card's scroll 0 0
    CHECK ok 19-card-scroll: the card's scroll y 0 (want 0 within 10); the engine's 0
    CHECK ok 20-number: n-move = 13 (wanted 5..400)
    WAIT ok 22-settled: the card's scroll 0 98
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 24-card-scroll-min: the card's scroll y 98 (at least 30)

### mojo / flash-input-tap — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
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

