# Finger suite report — 2026-11-12T12:00:49

Shells: mojo. Scripts: 1. 90 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| mojo | download-list | **pass** | 9 ok, 0 failed | WAIT ok 26-content 10,90,400,300; frames: kept/mojo-download-list-panel_.jpg, kept/mojo-download-list-downloads_.jpg | cavekit-device-build.md R4 |

## The device at the end

```
DEVICE ok job-enyo: (26963/638288551) jihad (start) running, process 32489
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (26967/519876996) jihad-mochi (start) running, process 14648
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (26971/222553439) jihad-mojo (start) running, process 9008
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

### mojo / download-list — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-31
    CHECK ok 10-file: /media/internal/downloads/jihad-download-31 (17 bytes) holds [Hello from Jihad]
    WAIT ok 12-differs 0,100,756,960
    WAIT ok 14-still 0,100,756,960
    WAIT ok 17-differs 0,100,756,960
    WAIT ok 18-still 0,100,756,960
    WAIT ok 22-differs 450,80,768,400
    WAIT ok 23-still 450,80,768,400
    WAIT ok 26-content 10,90,400,300

