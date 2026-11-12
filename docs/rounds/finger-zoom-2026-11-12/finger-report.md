# Finger suite report — 2026-11-12T12:32:09

Shells: mochi, mojo. Scripts: 4. 534 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| mochi | zoom-dtap-column | **pass** | 6 ok, 0 failed | CHECK ok 14-content: region 0,100-756,700: colours 402 hash 6c6a1b91 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-195-750,1998 | cavekit-input-bridging.md R5 |
| mochi | zoom-field-kept | **pass** | 6 ok, 0 failed | CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8) | cavekit-input-bridging.md R5 |
| mochi | zoom-locked-pinch-off | **pass** | 3 ok, 0 failed | CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05) | cavekit-input-bridging.md R5 |
| mochi | zoom-locked-pinch | **pass** | 4 ok, 0 failed | CHECK ok 13-zoom-min: zoom 1.9123 (at least 1.3) | cavekit-input-bridging.md R5 |
| mojo | zoom-dtap-column | **pass** | 6 ok, 0 failed | CHECK ok 14-content: region 0,100-756,700: colours 325 hash 735e4dc6 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-211-750,1982 | cavekit-input-bridging.md R5 |
| mojo | zoom-field-kept | **pass** | 6 ok, 0 failed | CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8) | cavekit-input-bridging.md R5 |
| mojo | zoom-locked-pinch-off | **pass** | 3 ok, 0 failed | CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05) | cavekit-input-bridging.md R5 |
| mojo | zoom-locked-pinch | **pass** | 4 ok, 0 failed | CHECK ok 13-zoom-min: zoom 1.9123 (at least 1.3) | cavekit-input-bridging.md R5 |

## The device at the end

```
DEVICE ok job-enyo: (29282/984018622) jihad (start) running, process 9021
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (29288/906495509) jihad-mochi (start) running, process 24814
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (29294/566074027) jihad-mojo (start) running, process 6317
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
DEVICE FAIL tmp-dumps: /tmp/sb-atlas-HP_TouchPad_-r3.png /tmp/st-rest.png /tmp/t613-enyo-12.png /tmp/t613-enyo-24.png /tmp/t613-enyo-4.png /tmp/t613-mochi-12.png /tmp/t613-mochi-24.png /tmp/t613-mochi-4.png /tmp/t613-mojo-12.png /tmp/t613-mojo-24.png /tmp/t613-mojo-4.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 5% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### mochi / zoom-dtap-column — pass

    mochi card on http://192.168.1.245:8077/jihad-dtap.html: connected at +3 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 7-rect sel:#col
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 110 landed on document y 50)
    WAIT ok 10-zoom
    WAIT ok 11-settled: the card's scroll 590 271
    CHECK ok 12-zoom-min: zoom 1.8286 (at least 1.5)
    CHECK ok 13-element-at: sel:#col on the screen at 18,-195-750,1998 holds 384,420
    CHECK ok 14-content: region 0,100-756,700: colours 402 hash 6c6a1b91 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-195-750,1998

### mochi / zoom-field-kept — pass

    mochi card on http://192.168.1.245:8077/jihad-dtap.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 7-rect sel:#f
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 110 landed on document y 50)
    CHECK ok 9-zoom-max: zoom 0.6400 (at most 0.8)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 12-settled: the card's scroll 0 425
    CHECK ok 14-above-keyboard: sel:#f on the screen at 64,638-324,667, above the keyboard
    CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8)

### mochi / zoom-locked-pinch-off — pass

    mochi card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:h1
    WAIT ok 12-settled: the card's scroll 0 0
    CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05)

### mochi / zoom-locked-pinch — pass

    mochi card on http://192.168.1.245:8077/jihad-locked.html: connected at +2 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect sel:h1
    WAIT ok 11-zoom
    WAIT ok 12-settled: the card's scroll 184 201
    CHECK ok 13-zoom-min: zoom 1.9123 (at least 1.3)

### mojo / zoom-dtap-column — pass

    mojo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#col
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 105 landed on document y 50)
    WAIT ok 10-zoom
    WAIT ok 11-settled: the card's scroll 590 277
    CHECK ok 12-zoom-min: zoom 1.8286 (at least 1.5)
    CHECK ok 13-element-at: sel:#col on the screen at 18,-211-750,1982 holds 384,420
    CHECK ok 14-content: region 0,100-756,700: colours 325 hash 735e4dc6 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 18,-211-750,1982

### mojo / zoom-field-kept — pass

    mojo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#f
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 105 landed on document y 50)
    CHECK ok 9-zoom-max: zoom 0.6400 (at most 0.8)
    CHECK ok 11-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-settled: the card's scroll 0 420
    CHECK ok 14-above-keyboard: sel:#f on the screen at 64,591-324,621, above the keyboard
    CHECK ok 15-zoom-max: zoom 0.6400 (at most 0.8)

### mojo / zoom-locked-pinch-off — pass

    mojo card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:h1
    WAIT ok 12-settled: the card's scroll 0 0
    CHECK ok 13-zoom: zoom 1.0000 (want 1.0 within 0.05)

### mojo / zoom-locked-pinch — pass

    mojo card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 7-rect sel:h1
    WAIT ok 11-zoom
    WAIT ok 12-settled: the card's scroll 184 180
    CHECK ok 13-zoom-min: zoom 1.9123 (at least 1.3)

