# Finger suite report — 2026-11-13T09:07:12

Shells: enyo, mochi, mojo. Scripts: 1. 218 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | codecs-play | **pass** | 9 ok, 0 failed | WAIT ok 19-text r9 = [mp3 canPlay=maybe result=ended t=10.06/10.06 err=0] | cavekit-lowpower-uxp.md R4 |

## The device at the end

```
DEVICE ok job-enyo: (103051/807687739) jihad (start) running, process 13228
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (103064/66177662) jihad-mochi (start) running, process 17229
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (103076/37172771) jihad-mojo (start) running, process 18492
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
DEVICE FAIL tmp-dumps: /tmp/sb-probe.png /tmp/sb-probe2.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 4% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### enyo / codecs-play — pass

    enyo card on http://192.168.1.245:8077/jihad-codecs.html: connected at +2 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 11-text state = [DONE 11 rows]
    WAIT ok 12-text r0 = [vp8 canPlay=probably result=ended t=10.00/10.00 err=0 frames=149/0 vw=320x240]
    WAIT ok 13-text r0 = [vp8 canPlay=probably result=ended t=10.00/10.00 err=0 frames=149/0 vw=320x240]
    WAIT ok 14-text r1 = [vp9 canPlay=probably result=ended t=10.00/10.00 err=0 frames=149/0 vw=320x240]
    WAIT ok 15-text r1 = [vp9 canPlay=probably result=ended t=10.00/10.00 err=0 frames=149/0 vw=320x240]
    WAIT ok 16-text r8 = [flac canPlay=maybe result=ended t=10.00/10.00 err=0]
    WAIT ok 17-text r8 = [flac canPlay=maybe result=ended t=10.00/10.00 err=0]
    WAIT ok 18-text r9 = [mp3 canPlay=maybe result=ended t=10.06/10.06 err=0]
    WAIT ok 19-text r9 = [mp3 canPlay=maybe result=ended t=10.06/10.06 err=0]

