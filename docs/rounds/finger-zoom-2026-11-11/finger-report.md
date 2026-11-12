# Finger suite report — 2026-11-12T11:44:13

Shells: enyo, mochi, mojo. Scripts: 4. 904 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | zoom-dtap-column | **fail** | 5 ok, 1 failed | CHECK FAIL 13-element-at: sel:#col on the screen at 507,31-1239,2226 does not hold 384,420; frames: kept/enyo-zoom-dtap-column-13-element-at_.jpg | cavekit-input-bridging.md R5 |
| enyo | zoom-field-kept | **fail** | 5 ok, 1 failed | CHECK FAIL 11-keyboard: the keyboard is not up within 8 s (its last reading: focused 1 wanted, 0 read); frames: kept/enyo-zoom-field-kept-11-keyboard_.jpg | cavekit-input-bridging.md R5 |
| enyo | zoom-locked-pinch-off | **fail** | 1 ok, 1 failed | CALIBRATION FAIL (enyo): the tap on sel:h1 aimed with the page top 82 reached no clickAt — the round stops; frames: kept/enyo-zoom-locked-pinch-off-calibration_.jpg | cavekit-input-bridging.md R5 |
| enyo | zoom-locked-pinch | **fail** | 1 ok, 1 failed | CALIBRATION FAIL (enyo): the tap on sel:h1 aimed with the page top 82 reached no clickAt — the round stops; frames: kept/enyo-zoom-locked-pinch-calibration_.jpg | cavekit-input-bridging.md R5 |
| mochi | zoom-dtap-column | **fail** | 5 ok, 1 failed | CHECK FAIL 13-element-at: sel:#col on the screen at 507,28-1239,2223 does not hold 384,420; frames: kept/mochi-zoom-dtap-column-13-element-at_.jpg | cavekit-input-bridging.md R5 |
| mochi | zoom-field-kept | **fail** | 5 ok, 1 failed | CHECK FAIL 11-keyboard: the keyboard is not up within 8 s (its last reading: focused 1 wanted, 0 read); frames: kept/mochi-zoom-field-kept-11-keyboard_.jpg | cavekit-input-bridging.md R5 |
| mochi | zoom-locked-pinch-off | **fail** | 1 ok, 1 failed | CALIBRATION FAIL (mochi): the tap on sel:h1 aimed with the page top 78 reached no clickAt — the round stops; frames: kept/mochi-zoom-locked-pinch-off-calibration_.jpg | cavekit-input-bridging.md R5 |
| mochi | zoom-locked-pinch | **fail** | 1 ok, 1 failed | CALIBRATION FAIL (mochi): the tap on sel:h1 aimed with the page top 78 reached no clickAt — the round stops; frames: kept/mochi-zoom-locked-pinch-calibration_.jpg | cavekit-input-bridging.md R5 |
| mojo | zoom-dtap-column | **fail** | 5 ok, 1 failed | CHECK FAIL 13-element-at: sel:#col on the screen at 507,17-1239,2212 does not hold 384,420; frames: kept/mojo-zoom-dtap-column-13-element-at_.jpg | cavekit-input-bridging.md R5 |
| mojo | zoom-field-kept | **fail** | 5 ok, 1 failed | CHECK FAIL 11-keyboard: the keyboard is not up within 8 s (its last reading: focused 1 wanted, 0 read); frames: kept/mojo-zoom-field-kept-11-keyboard_.jpg | cavekit-input-bridging.md R5 |
| mojo | zoom-locked-pinch-off | **fail** | 1 ok, 1 failed | CALIBRATION FAIL (mojo): the tap on sel:h1 aimed with the page top 73 reached no clickAt — the round stops; frames: kept/mojo-zoom-locked-pinch-off-calibration_.jpg | cavekit-input-bridging.md R5 |
| mojo | zoom-locked-pinch | **fail** | 1 ok, 1 failed | CALIBRATION FAIL (mojo): the tap on sel:h1 aimed with the page top 73 reached no clickAt — the round stops; frames: kept/mojo-zoom-locked-pinch-calibration_.jpg | cavekit-input-bridging.md R5 |

## The device at the end

```
DEVICE ok job-enyo: (26769/753157478) jihad (start) running, process 32489
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (26777/957450958) jihad-mochi (start) running, process 14648
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (26787/289884883) jihad-mojo (start) running, process 29988
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

### enyo / zoom-dtap-column — fail

    enyo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#col
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 114 landed on document y 50)
    WAIT ok 10-zoom
    WAIT ok 11-settled: the card's scroll 590 272
    CHECK ok 12-zoom-min: zoom 1.8286 (at least 1.5)
    CHECK FAIL 13-element-at: sel:#col on the screen at 507,31-1239,2226 does not hold 384,420
    CHECK ok 14-content: region 0,100-756,700: colours 438 hash 2976668a rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 507,31-1239,2226

### enyo / zoom-field-kept — fail

    enyo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +2 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#f
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 114 landed on document y 50)
    CHECK ok 9-zoom-max: zoom 0.6400 (at most 0.8)
    CHECK FAIL 11-keyboard: the keyboard is not up within 8 s (its last reading: focused 1 wanted, 0 read)
    WAIT ok 12-settled: the card's scroll 0 430
    CHECK ok 13-visible: sel:#f on the screen at 64,484-324,513 (the reach ends at 1024, keyboard 0)
    CHECK ok 14-zoom-max: zoom 0.6400 (at most 0.8)

### enyo / zoom-locked-pinch-off — fail

    enyo card on http://192.168.1.245:8077/jihad-locked.html: connected at +2 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:h1
    CALIBRATION FAIL (enyo): the tap on sel:h1 aimed with the page top 82 reached no clickAt — the round stops

### enyo / zoom-locked-pinch — fail

    enyo card on http://192.168.1.245:8077/jihad-locked.html: connected at +2 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 7-rect sel:h1
    CALIBRATION FAIL (enyo): the tap on sel:h1 aimed with the page top 82 reached no clickAt — the round stops

### mochi / zoom-dtap-column — fail

    mochi card on http://192.168.1.245:8077/jihad-dtap.html: connected at +2 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 7-rect sel:#col
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 110 landed on document y 50)
    WAIT ok 10-zoom
    WAIT ok 11-settled: the card's scroll 590 271
    CHECK ok 12-zoom-min: zoom 1.8286 (at least 1.5)
    CHECK FAIL 13-element-at: sel:#col on the screen at 507,28-1239,2223 does not hold 384,420
    CHECK ok 14-content: region 0,100-756,700: colours 402 hash 6c6a1b91 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 507,28-1239,2223

### mochi / zoom-field-kept — fail

    mochi card on http://192.168.1.245:8077/jihad-dtap.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 7-rect sel:#f
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 110 landed on document y 50)
    CHECK ok 9-zoom-max: zoom 0.6400 (at most 0.8)
    CHECK FAIL 11-keyboard: the keyboard is not up within 8 s (its last reading: focused 1 wanted, 0 read)
    WAIT ok 12-settled: the card's scroll 0 425
    CHECK ok 13-visible: sel:#f on the screen at 64,485-324,514 (the reach ends at 1024, keyboard 0)
    CHECK ok 14-zoom-max: zoom 0.6400 (at most 0.8)

### mochi / zoom-locked-pinch-off — fail

    mochi card on http://192.168.1.245:8077/jihad-locked.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 8-rect sel:h1
    CALIBRATION FAIL (mochi): the tap on sel:h1 aimed with the page top 78 reached no clickAt — the round stops

### mochi / zoom-locked-pinch — fail

    mochi card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 7-rect sel:h1
    CALIBRATION FAIL (mochi): the tap on sel:h1 aimed with the page top 78 reached no clickAt — the round stops

### mojo / zoom-dtap-column — fail

    mojo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +3 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#col
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 105 landed on document y 50)
    WAIT ok 10-zoom
    WAIT ok 11-settled: the card's scroll 590 277
    CHECK ok 12-zoom-min: zoom 1.8286 (at least 1.5)
    CHECK FAIL 13-element-at: sel:#col on the screen at 507,17-1239,2212 does not hold 384,420
    CHECK ok 14-content: region 0,100-756,700: colours 325 hash 735e4dc6 rows 600 uniform 0 light 100-699 blue none; sel:#col on the screen at 507,17-1239,2212

### mojo / zoom-field-kept — fail

    mojo card on http://192.168.1.245:8077/jihad-dtap.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 7-rect sel:#f
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 105 landed on document y 50)
    CHECK ok 9-zoom-max: zoom 0.6400 (at most 0.8)
    CHECK FAIL 11-keyboard: the keyboard is not up within 8 s (its last reading: focused 1 wanted, 0 read)
    WAIT ok 12-settled: the card's scroll 0 420
    CHECK ok 13-visible: sel:#f on the screen at 64,440-324,469 (the reach ends at 1024, keyboard 0)
    CHECK ok 14-zoom-max: zoom 0.6400 (at most 0.8)

### mojo / zoom-locked-pinch-off — fail

    mojo card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 8-rect sel:h1
    CALIBRATION FAIL (mojo): the tap on sel:h1 aimed with the page top 73 reached no clickAt — the round stops

### mojo / zoom-locked-pinch — fail

    mojo card on http://192.168.1.245:8077/jihad-locked.html: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 7-rect sel:h1
    CALIBRATION FAIL (mojo): the tap on sel:h1 aimed with the page top 73 reached no clickAt — the round stops

