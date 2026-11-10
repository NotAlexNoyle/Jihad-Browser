# Finger suite report — 2026-11-09T22:36:39

Shells: enyo. Scripts: 1. 128 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | media-scrub-drag | **pass** | 8 ok, 0 failed | WAIT ok 23-text seek = [60]; frames: kept/enyo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |

## The device at the end

```
DEVICE ok job-enyo: (1516600/499779946) jihad (start) running, process 19295
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (1516614/275015647) jihad-mochi (start) running, process 4858
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (1516627/386573275) jihad-mojo (start) running, process 5782
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

### enyo / media-scrub-drag — pass

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +3 s, load done at +11 s, 21 content frames at +11 s; the GPU path, frames off the main thread (the 
    WAIT ok 8-rect v
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    WAIT ok 10-text status = [playing]
    WAIT ok 12-text status = [paused t=15.1]
    WAIT ok 13-rect anon:#v|controlBar
    WAIT ok 14-rect anon:#v|volumeControl
    WAIT ok 18-text vol = [volume=0.16 muted=false]
    WAIT ok 22-settled: the card's scroll 0 0
    WAIT ok 23-text seek = [60]

