# Finger suite report — 2026-11-11T01:15:20

First run 2026-11-11T00:44:06; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 1. 492 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | settings-every-control | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| mochi | settings-every-control | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| mojo | settings-every-control | **by a tool** | 0 ok, 0 failed | tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt) | cavekit-preferences-ui.md R6 |
| mojo | download-list (rerun) | **pass** | 9 ok, 0 failed | WAIT ok 26-content 10,90,400,300; frames: kept/mojo-download-list-panel_.jpg, kept/mojo-download-list-downloads_.jpg | cavekit-device-build.md R4 |
| mojo | media-scrub-drag (rerun) | **known** | 7 ok, 1 failed | the open defect T-598: CHECK FAIL 25-wait-text: seek never read 60 within the bound; frames: kept/mojo-media-scrub-drag-25-wait-text_.jpg, kept/mojo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |

## The device at the end

```
DEVICE ok job-enyo: (1612557/135130988) jihad (start) running, process 7569
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE FAIL cards-enyo: 1 card(s) of net.riverstonerelay.jihad-browser still open
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (1612567/108876468) jihad-mochi (start) running, process 9934
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (1612575/864895579) jihad-mojo (start) running, process 18770
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
DEVICE FAIL tmp-dumps: /tmp/eye-cairo.png /tmp/eye-skia.png /tmp/og-plain.png /tmp/og-sticky.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 8% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### enyo / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### mochi / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### mojo / settings-every-control — by a tool

    tools/device/finger/prefs-finger-walk.sh <shell> (62 of 62 on each shell, 2026-11-04: docs/rounds/finger-2026-11-04/prefs-finger-walk-<shell>.txt)

### mojo / download-list — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +19 s, 5 content frames at +19 s; the GPU path, frames off the main thread (the express lane) (the job's 
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 9-line: [jihad-dl] finished data:application/octet-stream;base64,SGVsbG8gZnJvbSBKaWhhZAo= -> /media/internal/downloads/jihad-download-29
    CHECK ok 10-file: /media/internal/downloads/jihad-download-29 (17 bytes) holds [Hello from Jihad]
    WAIT ok 12-differs 0,100,756,960
    WAIT ok 14-still 0,100,756,960
    WAIT ok 17-differs 0,100,756,960
    WAIT ok 18-still 0,100,756,960
    WAIT ok 22-differs 450,80,768,400
    WAIT ok 23-still 450,80,768,400
    WAIT ok 26-content 10,90,400,300

### mojo / media-scrub-drag — known

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +11 s, 25 content frames at +11 s; the GPU path, frames off the main thread (the 
    WAIT ok 10-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 12-text status = [playing]
    WAIT ok 14-text status = [paused t=42.3]
    WAIT ok 15-rect anon:#v|controlBar
    WAIT ok 16-rect anon:#v|volumeControl
    WAIT ok 20-text vol = [volume=0.16 muted=false]
    WAIT ok 24-settled: the card's scroll 0 0
    CHECK FAIL 25-wait-text: seek never read 60 within the bound

