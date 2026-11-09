# Finger suite report — 2026-11-09T12:36:29

Shells: enyo, mochi, mojo. Scripts: 3. 421 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | autoplay-sound | **pass** | 3 ok, 0 failed | WAIT ok 13-text status = [playing]; frames: kept/enyo-autoplay-sound-refused_.jpg | cavekit-gre-widgets.md R1 |
| enyo | autoplay-unmute | **pass** | 3 ok, 0 failed | WAIT ok 11-text status = [paused t=29.8]; frames: kept/enyo-autoplay-unmute-paused-on-unmute_.jpg | cavekit-gre-widgets.md R1 |
| enyo | screen-lock | **pass** | 5 ok, 0 failed | WAIT ok 32-text status = [paused t=19.6]; frames: kept/enyo-screen-lock-lock-button_.jpg, kept/enyo-screen-lock-locked_.jpg, kept/enyo-screen-lock-ask_.jpg, kept/enyo-screen-lock-unlocked_.jpg | cavekit-gre-widgets.md R1 |
| mochi | screen-lock | **pass** | 5 ok, 0 failed | WAIT ok 32-text status = [paused t=20.0]; frames: kept/mochi-screen-lock-lock-button_.jpg, kept/mochi-screen-lock-locked_.jpg, kept/mochi-screen-lock-ask_.jpg, kept/mochi-screen-lock-unlocked_.jpg | cavekit-gre-widgets.md R1 |
| mojo | screen-lock | **pass** | 5 ok, 0 failed | WAIT ok 32-text status = [paused t=28.8]; frames: kept/mojo-screen-lock-lock-button_.jpg, kept/mojo-screen-lock-locked_.jpg, kept/mojo-screen-lock-ask_.jpg, kept/mojo-screen-lock-unlocked_.jpg | cavekit-gre-widgets.md R1 |

## The device at the end

```
DEVICE ok job-enyo: (1480881/921574124) jihad (start) running, process 18722
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (1480894/364841975) jihad-mochi (start) running, process 22313
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (1480911/470901823) jihad-mojo (start) running, process 26171
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
DEVICE ok overall: 4% of /tmp used
```

## Every test's lines

### enyo / autoplay-sound — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +10 s, 8 content frames at +10 s; the GPU path, frames off the main threa
    WAIT ok 8-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 10-text status = [play refused NotAllowedError]
    WAIT ok 13-text status = [playing]

### enyo / autoplay-unmute — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&unmute=30000: connected at +2 s, load done at +9 s, 11 content frames at +9 s; the GPU path, frames off the main t
    WAIT ok 8-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 10-text status = [playing]
    WAIT ok 11-text status = [paused t=29.8]

### enyo / screen-lock — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +9 s, 11 content frames at +9 s; the GPU path, frames off the main thread (the ex
    WAIT ok 7-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 9-text status = [playing]
    WAIT ok 18-text status = [playing]
    CHECK ok 20-text: status = [playing]
    WAIT ok 32-text status = [paused t=19.6]

### mochi / screen-lock — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +12 s, 16 content frames at +12 s; the GPU path, frames off the main thread (the
    WAIT ok 7-rect v
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 9-text status = [playing]
    WAIT ok 18-text status = [playing]
    CHECK ok 20-text: status = [playing]
    WAIT ok 32-text status = [paused t=20.0]

### mojo / screen-lock — pass

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +10 s, 18 content frames at +10 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 9-text status = [playing]
    WAIT ok 18-text status = [playing]
    CHECK ok 20-text: status = [playing]
    WAIT ok 32-text status = [paused t=28.8]

