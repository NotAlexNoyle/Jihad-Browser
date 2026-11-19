# Finger suite report — 2026-11-18T20:28:34

Shells: enyo, mochi, mojo. Scripts: 1. 180 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | eagler-webgl-attrs | **pass** | 3 ok, 0 failed | CHECK ok 15-probe-has: the probe's answer holds [b1 fresh: webgl -> context]; frames: kept/enyo-eagler-webgl-attrs-rows_.jpg | cavekit-eaglercraft.md R2 |
| mochi | eagler-webgl-attrs | **pass** | 3 ok, 0 failed | CHECK ok 15-probe-has: the probe's answer holds [b1 fresh: webgl -> context]; frames: kept/mochi-eagler-webgl-attrs-rows_.jpg | cavekit-eaglercraft.md R2 |
| mojo | eagler-webgl-attrs | **pass** | 3 ok, 0 failed | CHECK ok 15-probe-has: the probe's answer holds [b1 fresh: webgl -> context]; frames: kept/mojo-eagler-webgl-attrs-rows_.jpg | cavekit-eaglercraft.md R2 |

## The device at the end

```
DEVICE ok job-enyo: (79978/492169251) jihad (start) running, process 11400
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (79984/612646879) jihad-mochi (start) running, process 14401
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (79990/998783469) jihad-mojo (start) running, process 17447
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
DEVICE ok volume-subscribers: none without its daemon
DEVICE ok media-scratch: none
DEVICE ok overall: 47% of /tmp used
```

## Every test's lines

### enyo / eagler-webgl-attrs — pass

    enyo card on http://192.168.1.245:8077/jihad-webgl-attrs.html: connected at +2 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 12-text done = [rows 15]
    CHECK ok 14-probe-has: the probe's answer holds [a2 same canvas: webgl -> context]
    CHECK ok 15-probe-has: the probe's answer holds [b1 fresh: webgl -> context]

### mochi / eagler-webgl-attrs — pass

    mochi card on http://192.168.1.245:8077/jihad-webgl-attrs.html: connected at +3 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 12-text done = [rows 15]
    CHECK ok 14-probe-has: the probe's answer holds [a2 same canvas: webgl -> context]
    CHECK ok 15-probe-has: the probe's answer holds [b1 fresh: webgl -> context]

### mojo / eagler-webgl-attrs — pass

    mojo card on http://192.168.1.245:8077/jihad-webgl-attrs.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 12-text done = [rows 15]
    CHECK ok 14-probe-has: the probe's answer holds [a2 same canvas: webgl -> context]
    CHECK ok 15-probe-has: the probe's answer holds [b1 fresh: webgl -> context]

