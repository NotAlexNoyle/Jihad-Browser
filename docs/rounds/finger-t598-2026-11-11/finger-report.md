# Finger suite report — 2026-11-12T12:02:20

Shells: enyo, mochi, mojo. Scripts: 1. 310 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | media-scrub-drag | **pass** | 8 ok, 0 failed | WAIT ok 25-text seek = [60]; frames: kept/enyo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| mochi | media-scrub-drag | **pass** | 8 ok, 0 failed | WAIT ok 25-text seek = [60]; frames: kept/mochi-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| mojo | media-scrub-drag | **known** | 7 ok, 1 failed | the open defect T-598: CHECK FAIL 25-wait-text: seek never read 60 within the bound; frames: kept/mojo-media-scrub-drag-25-wait-text_.jpg, kept/mojo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |

## The device at the end

```
DEVICE ok job-enyo: (27269/562279457) jihad (start) running, process 15188
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (27274/935005381) jihad-mochi (start) running, process 19409
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (27279/711820639) jihad-mojo (start) running, process 23681
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
DEVICE FAIL tmp-dumps: /tmp/closed.png /tmp/hang-1-stalled.png /tmp/hang-2-colours.png /tmp/hang-4-newcard.png /tmp/hang-A-colours.png /tmp/hang-A-newcard.png /tmp/hang-B-colours.png /tmp/open1.png /tmp/page-a.png /tmp/page-b.png /tmp/page-c.png /tmp/reopen.png /tmp/sb-atlas-HP_TouchPad_-r3.png /tmp/st-rest.png /tmp/t613-enyo-12.png /tmp/t613-enyo-24.png /tmp/t613-enyo-4.png /tmp/t613-mochi-12.png /tmp/t613-mochi-24.png /tmp/t613-mochi-4.png /tmp/t613-mojo-12.png /tmp/t613-mojo-24.png /tmp/t613-mojo-4.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 10% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### enyo / media-scrub-drag — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +9 s, 17 content frames at +9 s; the GPU path, frames off the main thread (the ex
    WAIT ok 10-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 12-text status = [playing]
    WAIT ok 14-text status = [paused t=24.2]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    WAIT ok 25-text seek = [60]

### mochi / media-scrub-drag — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the ex
    WAIT ok 10-rect v
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 12-text status = [playing]
    WAIT ok 14-text status = [paused t=24.6]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    WAIT ok 25-text seek = [60]

### mojo / media-scrub-drag — known

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the exp
    WAIT ok 10-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 12-text status = [playing]
    WAIT ok 14-text status = [paused t=13.2]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    CHECK FAIL 25-wait-text: seek never read 60 within the bound

