# Finger suite report — 2026-11-13T14:10:58

Shells: enyo, mochi, mojo. Scripts: 1. 341 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | flash-game-sound | **fail** | 2 ok, 3 failed | CHECK FAIL 11-tap: the tap on sel:embed reached no clickAt / CHECK FAIL 13-tap: the tap on sel:embed reached no clickAt / CHECK FAIL 15-tap: the tap on sel:embed reached no clickAt; frames: kept/enyo-flash-game-sound-11-tap_.jpg, kept/enyo-flash-game-sound-13-tap_.jpg, kept/enyo-flash-game-sound-15-tap_.jpg, kept/enyo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mochi | flash-game-sound | **fail** | 2 ok, 3 failed | CHECK FAIL 11-tap: the tap on sel:embed reached no clickAt / CHECK FAIL 13-tap: the tap on sel:embed reached no clickAt / CHECK FAIL 15-tap: the tap on sel:embed reached no clickAt; frames: kept/mochi-flash-game-sound-11-tap_.jpg, kept/mochi-flash-game-sound-13-tap_.jpg, kept/mochi-flash-game-sound-15-tap_.jpg, kept/mochi-flash-game-sound-game_.jpg | cavekit-flash.md R9 |
| mojo | flash-game-sound | **fail** | 2 ok, 3 failed | CHECK FAIL 11-tap: the tap on sel:embed reached no clickAt / CHECK FAIL 13-tap: the tap on sel:embed reached no clickAt / CHECK FAIL 15-tap: the tap on sel:embed reached no clickAt; frames: kept/mojo-flash-game-sound-11-tap_.jpg, kept/mojo-flash-game-sound-13-tap_.jpg, kept/mojo-flash-game-sound-15-tap_.jpg, kept/mojo-flash-game-sound-game_.jpg | cavekit-flash.md R9 |

## The device at the end

```
DEVICE ok job-enyo: (121395/378986556) jihad (start) running, process 8841
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (121406/606277813) jihad-mochi (start) running, process 13309
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (121419/942198403) jihad-mojo (start) running, process 17902
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

### enyo / flash-game-sound — fail

    enyo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +2 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 10-rect sel:embed
    CHECK FAIL 11-tap: the tap on sel:embed reached no clickAt
    CHECK FAIL 13-tap: the tap on sel:embed reached no clickAt
    CHECK FAIL 15-tap: the tap on sel:embed reached no clickAt
    CHECK ok 16-audio: the movie's audio: open running audible

### mochi / flash-game-sound — fail

    mochi card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +1 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 10-rect sel:embed
    CHECK FAIL 11-tap: the tap on sel:embed reached no clickAt
    CHECK FAIL 13-tap: the tap on sel:embed reached no clickAt
    CHECK FAIL 15-tap: the tap on sel:embed reached no clickAt
    CHECK ok 16-audio: the movie's audio: open running audible

### mojo / flash-game-sound — fail

    mojo card on http://www.ferryhalim.com/orisinal/g3/bells.htm: connected at +1 s, load done at +8 s, 10 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 10-rect sel:embed
    CHECK FAIL 11-tap: the tap on sel:embed reached no clickAt
    CHECK FAIL 13-tap: the tap on sel:embed reached no clickAt
    CHECK FAIL 15-tap: the tap on sel:embed reached no clickAt
    CHECK ok 16-audio: the movie's audio: open running audible

