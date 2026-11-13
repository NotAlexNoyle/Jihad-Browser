# Finger suite report — 2026-11-13T14:17:04

Shells: enyo, mochi, mojo. Scripts: 1. 234 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/enyo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mochi | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/mochi-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mojo | flash-game-sound | **pass** | 2 ok, 0 failed | CHECK ok 17-audio: the movie's audio: open running audible; frames: kept/mojo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |

## The device at the end

```
DEVICE ok job-enyo: (121676/505498946) jihad (start) running, process 23681
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (121683/648063685) jihad-mochi (start) running, process 27211
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (121688/696563832) jihad-mojo (start) running, process 30671
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
DEVICE FAIL tmp-dumps: /tmp/sb-probe.png /tmp/sb-probe2.png /tmp/wb.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 5% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### enyo / flash-game-sound — pass

    enyo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +3 s, load done at +9 s, 11 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### mochi / flash-game-sound — pass

    mochi card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +2 s, load done at +9 s, 14 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

### mojo / flash-game-sound — pass

    mojo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +1 s, load done at +8 s, 11 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 10-rect sel:embed
    CHECK ok 17-audio: the movie's audio: open running audible

