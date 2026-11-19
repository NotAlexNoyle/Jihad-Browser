# Finger suite report — 2026-11-19T09:20:28

Shells: enyo, mochi, mojo. Scripts: 1. 153 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | memory-guard-task | **pass** | 6 ok, 0 failed | WAIT ok 19-launch: load done | cavekit-device-headroom.md R1 |

## The device at the end

```
DEVICE ok job-enyo: (01631/978367094) jihad (start) running, process 12720
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (01642/510686501) jihad-mochi (start) running, process 6303
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (01651/61149313) jihad-mojo (start) running, process 7192
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
DEVICE ok overall: 1% of /tmp used
```

## Every test's lines

### enyo / memory-guard-task — pass

    enyo card on http://192.168.1.245:8077/jihad-memhog.html?mb=750&rate=12&task=1: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express la
    WAIT ok 14-line: [jihad-bs] memory guard (thread): this daemon 470716 kB, its players 0 kB (budget 470184), 28212 kB free (floor 65536): stopping the running content script, the page to be stopped
    WAIT ok 15-line: [jihad-bs] memory guard: the running content script stopped
    WAIT ok 16-line: [jihad-bs] memory shed: a page stopped
    WAIT ok 17-url: about:stopped?url=http%3A%2F%2F192.168.1.245%3A8077%2Fjihad-memhog.html%3Fmb%3D750%26rate%3D12%26task%3D1
    CHECK ok 18-gpu-calm: no new GPU fault in the kernel's log (0 lines, 0 at the start)
    WAIT ok 19-launch: load done

