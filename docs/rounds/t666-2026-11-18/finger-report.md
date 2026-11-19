# Finger suite report — 2026-11-19T01:21:03

First run 2026-11-18T20:28:34; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 1. 2702 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | eagler-webgl-attrs | **pass** | 3 ok, 0 failed | CHECK ok 15-probe-has: the probe's answer holds [b1 fresh: webgl -> context]; frames: kept/enyo-eagler-webgl-attrs-rows_.jpg | cavekit-eaglercraft.md R2 |
| mochi | eagler-webgl-attrs | **pass** | 3 ok, 0 failed | CHECK ok 15-probe-has: the probe's answer holds [b1 fresh: webgl -> context]; frames: kept/mochi-eagler-webgl-attrs-rows_.jpg | cavekit-eaglercraft.md R2 |
| mojo | eagler-webgl-attrs | **pass** | 3 ok, 0 failed | CHECK ok 15-probe-has: the probe's answer holds [b1 fresh: webgl -> context]; frames: kept/mojo-eagler-webgl-attrs-rows_.jpg | cavekit-eaglercraft.md R2 |
| enyo | eagler-load-188 (rerun) | **pass** | 6 ok, 0 failed | CHECK ok 28-quiet: no line jihad-never-logged for 30 s; frames: kept/enyo-eagler-load-188-slow-script_.jpg, kept/enyo-eagler-load-188-launch_.jpg, kept/enyo-eagler-load-188-after-launch_.jpg, kept/enyo-eagler-load-188-after-launch-60_.jpg | cavekit-eaglercraft.md R2 |
| mochi | eagler-load-188 (rerun) | **fail** | 4 ok, 1 failed | CHECK FAIL 18-probe: no answer to the probe state within 30 s; frames: kept/mochi-eagler-load-188-t30_.jpg, kept/mochi-eagler-load-188-t60_.jpg, kept/mochi-eagler-load-188-18-probe_.jpg, kept/mochi-eagler-load-188-t120_.jpg, kept/mochi-eagler-load-188-t180_.jpg | cavekit-eaglercraft.md R2 |
| mojo | eagler-load-188 (rerun) | **fail** | 4 ok, 1 failed | CHECK FAIL 18-probe: no answer to the probe state within 30 s; frames: kept/mojo-eagler-load-188-t30_.jpg, kept/mojo-eagler-load-188-t60_.jpg, kept/mojo-eagler-load-188-18-probe_.jpg, kept/mojo-eagler-load-188-t120_.jpg, kept/mojo-eagler-load-188-t180_.jpg | cavekit-eaglercraft.md R2 |
| enyo | weakmap-dom-keys (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 17-probe-has: the probe's answer holds [AudioParam: weakmap ok, weakset ok]; frames: kept/enyo-weakmap-dom-keys-rows_.jpg | cavekit-eaglercraft.md R2 |
| mochi | weakmap-dom-keys (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 17-probe-has: the probe's answer holds [AudioParam: weakmap ok, weakset ok]; frames: kept/mochi-weakmap-dom-keys-rows_.jpg | cavekit-eaglercraft.md R2 |
| mojo | weakmap-dom-keys (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 17-probe-has: the probe's answer holds [AudioParam: weakmap ok, weakset ok]; frames: kept/mojo-weakmap-dom-keys-rows_.jpg | cavekit-eaglercraft.md R2 |

## The device at the end

```
DEVICE ok job-enyo: (97694/542521671) jihad (start) running, process 27338
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (97703/665016334) jihad-mochi (start) running, process 12434
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (97712/841805964) jihad-mojo (start) running, process 15530
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

### enyo / eagler-load-188 — pass

    enyo card on http://192.168.1.245:8099/eaglerx-1.8.8/unpacked/eaglercraft1-8.html: connected at +2 s, load done at +53 s, 38 content frames at +53 s; the GPU path, frames off the main thread (the expr
    CHECK ok 17-dialog: the dialog's reply pipe stands: /var/palm/jihad/enyo/dialog-1.fifo
    CHECK ok 20-dialog: [jihad-bs] dialog confirm -> ACCEPT (answer=1)
    WAIT ok 21-rect sel:._eaglercraftX_mobile_launch_client
    WAIT ok 25-gone sel:._eaglercraftX_mobile_launch_client
    CHECK ok 26-quiet: no line jihad-never-logged for 30 s
    CHECK ok 28-quiet: no line jihad-never-logged for 30 s

### mochi / eagler-load-188 — fail

    mochi card on http://192.168.1.245:8099/eaglerx-1.8.8/unpacked/eaglercraft1-8.html: connected at +2 s, load done at +33 s, 72 content frames at +33 s; the GPU path, frames off the main thread (the exp
    CHECK ok 14-quiet: no line jihad-never-logged for 30 s
    CHECK ok 16-quiet: no line jihad-never-logged for 30 s
    CHECK FAIL 18-probe: no answer to the probe state within 30 s
    CHECK ok 19-quiet: no line jihad-never-logged for 60 s
    CHECK ok 21-quiet: no line jihad-never-logged for 60 s

### mojo / eagler-load-188 — fail

    mojo card on http://192.168.1.245:8099/eaglerx-1.8.8/unpacked/eaglercraft1-8.html: connected at +2 s, load done at +32 s, 49 content frames at +32 s; the GPU path, frames off the main thread (the expr
    CHECK ok 14-quiet: no line jihad-never-logged for 30 s
    CHECK ok 16-quiet: no line jihad-never-logged for 30 s
    CHECK FAIL 18-probe: no answer to the probe state within 30 s
    CHECK ok 19-quiet: no line jihad-never-logged for 60 s
    CHECK ok 21-quiet: no line jihad-never-logged for 60 s

### enyo / weakmap-dom-keys — pass

    enyo card on http://192.168.1.245:8077/jihad-weakmap-dom.html: connected at +2 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 12-text done = [rows 8]
    CHECK ok 14-probe-lacks: the probe's answer lacks [threw]
    CHECK ok 15-probe-lacks: the probe's answer lacks [lost]
    CHECK ok 16-probe-has: the probe's answer holds [WebGLTexture: weakmap ok, weakset ok]
    CHECK ok 17-probe-has: the probe's answer holds [AudioParam: weakmap ok, weakset ok]

### mochi / weakmap-dom-keys — pass

    mochi card on http://192.168.1.245:8077/jihad-weakmap-dom.html: connected at +3 s, load done at +8 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 12-text done = [rows 8]
    CHECK ok 14-probe-lacks: the probe's answer lacks [threw]
    CHECK ok 15-probe-lacks: the probe's answer lacks [lost]
    CHECK ok 16-probe-has: the probe's answer holds [WebGLTexture: weakmap ok, weakset ok]
    CHECK ok 17-probe-has: the probe's answer holds [AudioParam: weakmap ok, weakset ok]

### mojo / weakmap-dom-keys — pass

    mojo card on http://192.168.1.245:8077/jihad-weakmap-dom.html: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 12-text done = [rows 8]
    CHECK ok 14-probe-lacks: the probe's answer lacks [threw]
    CHECK ok 15-probe-lacks: the probe's answer lacks [lost]
    CHECK ok 16-probe-has: the probe's answer holds [WebGLTexture: weakmap ok, weakset ok]
    CHECK ok 17-probe-has: the probe's answer holds [AudioParam: weakmap ok, weakset ok]

