# Finger suite report — 2026-11-16T07:22:30

Shells: enyo. Scripts: 4. 319 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | audio-heard | **fail** | 4 ok, 2 failed | CHECK FAIL 11-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard / CHECK FAIL 14-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard; frames: kept/enyo-audio-heard-11-sound_.jpg, kept/enyo-audio-heard-14-sound_.jpg | cavekit-device-build.md R4 |
| enyo | flash-sound | **fail** | 6 ok, 2 failed | CHECK FAIL 11-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard / CHECK FAIL 16-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard; frames: kept/enyo-flash-sound-11-sound_.jpg, kept/enyo-flash-sound-16-sound_.jpg | cavekit-engine-update.md R8 |
| enyo | rotate | **pass** | 10 ok, 0 failed | CHECK ok 26-card-scroll-min: the card's scroll y 629 (at least 400); frames: kept/enyo-rotate-landscape-left_.jpg, kept/enyo-rotate-landscape-right_.jpg | cavekit-device-build.md R4 |
| enyo | volume-keys | **fail** | 3 ok, 2 failed | CHECK FAIL 10-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard / CHECK FAIL 17-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard; frames: kept/enyo-volume-keys-10-sound_.jpg, kept/enyo-volume-keys-17-sound_.jpg | cavekit-finger-testing.md R7 |

## The device at the end

```
DEVICE ok job-enyo: (356082/37323023) jihad (start) running, process 5223
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (356088/881959023) jihad-mochi (start) running, process 2558
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (356095/132252947) jihad-mojo (start) running, process 13496
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
DEVICE FAIL tmp-dumps: /tmp/volshot.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 17% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### enyo / audio-heard — fail

    enyo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +7 s, 13 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK FAIL 11-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK FAIL 14-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard

### enyo / flash-sound — fail

    enyo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +7 s, 13 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK FAIL 11-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK FAIL 16-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard

### enyo / rotate — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 11-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x686 mZoom=1.3333
    WAIT ok 12-content 0,150,756,900
    WAIT ok 15-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x686 mZoom=0.7500
    WAIT ok 16-content 0,150,756,900
    WAIT ok 18-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x686 mZoom=1.3333
    WAIT ok 19-content 0,150,756,900
    WAIT ok 22-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x686 mZoom=0.7500
    WAIT ok 23-settled: the card's scroll 0 0
    CHECK ok 24-content: region 0,150-756,600: colours 421 hash 727e4039 rows 450 uniform 173 light 150-599 blue none; sel:body on the screen at 0,82-764,20570
    CHECK ok 26-card-scroll-min: the card's scroll y 629 (at least 400)

### enyo / volume-keys — fail

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +7 s, 20 content frames at +7 s; the GPU path, frames off the main thread
    WAIT ok 9-text status = [playing]
    CHECK FAIL 10-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard
    CHECK ok 13-volume: the media volume 45, higher than/as the marked 23
    CHECK ok 16-volume: the media volume 23, lower than/as the marked 45
    CHECK FAIL 17-sound: sound at the output sink: peak 0 rms 0 frames 0, the speaker amp on; wanted heard

