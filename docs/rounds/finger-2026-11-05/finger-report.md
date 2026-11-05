# Finger suite report — 2026-11-05T08:51:52

Shells: enyo, mochi, mojo. Scripts: 5. 673 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | keyboard-address | **pass** | 2 ok, 0 failed | CHECK ok 10-keyboard: IME: Window got focus change in sysmgr net.riverstonerelay.jihad-browser focused: 0, fieldtype: 0, fieldactions: 0x00 | cavekit-input-bridging.md R2 |
| enyo | pan-long-drag | **pass** | 8 ok, 0 failed | CHECK ok 18-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| enyo | settings-tabs | **pass** | 4 ok, 0 failed | CHECK ok 11-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| enyo | accelerometer | **a person's** | 0 ok, 0 failed | the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side | cavekit-device-build.md R4 |
| enyo | deliberate-fail | **fail** | 0 ok, 1 failed | CHECK FAIL 7-text: sel:#header = [jihad-long-static: a fixed header], want [not the header]; frames: kept/enyo-deliberate-fail-7-text_.jpg | cavekit-finger-testing.md R5 |
| mochi | keyboard-address | **pass** | 2 ok, 0 failed | CHECK ok 10-keyboard: IME: Window got focus change in sysmgr net.riverstonerelay.jihad-browser-mochi focused: 0, fieldtype: 0, fieldactions: 0 | cavekit-input-bridging.md R2 |
| mochi | pan-long-drag | **pass** | 8 ok, 0 failed | CHECK ok 18-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mochi | settings-tabs | **pass** | 4 ok, 0 failed | CHECK ok 11-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| mochi | accelerometer | **a person's** | 0 ok, 0 failed | the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side | cavekit-device-build.md R4 |
| mojo | keyboard-address | **pass** | 2 ok, 0 failed | CHECK ok 10-keyboard: IME: Window got focus change in sysmgr net.riverstonerelay.jihad-browser-mojo focused: 0, fieldtype: 0, fieldactions: 0x | cavekit-input-bridging.md R2 |
| mojo | pan-long-drag | **pass** | 8 ok, 0 failed | CHECK ok 18-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mojo | settings-tabs | **pass** | 4 ok, 0 failed | CHECK ok 11-text-has: sel:h2.group-title = [Content] holds [Content] | cavekit-preferences-ui.md R6 |
| mojo | accelerometer | **a person's** | 0 ok, 0 failed | the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side | cavekit-device-build.md R4 |

## The device at the end

```
DEVICE ok job-enyo: (1122091/992449702) jihad (start) running, process 30277
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (1122096/708450589) jihad-mochi (start) running, process 7717
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (1122102/75888958) jihad-mojo (start) running, process 18648
DEVICE ok daemons-mojo: one daemon
DEVICE ok hand-daemon-mojo: none
DEVICE ok hooks-mojo: none
DEVICE ok adapter-gates-mojo: none
DEVICE ok proxy-mojo: network.proxy.type unset
DEVICE ok look-mojo: {"returnValue":true,"theme":null}
DEVICE ok gate-files: none in /tmp
DEVICE ok device-scripts: none left in /tmp
DEVICE ok tmp-dumps: none
DEVICE ok media-scratch: none
DEVICE ok overall: 39% of /tmp used
```

## Every test's lines

### enyo / keyboard-address — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 8-keyboard: IME: Window got focus change in sysmgr net.riverstonerelay.jihad-browser focused: 1, fieldtype: 7, fieldactions: 0x00
    CHECK ok 10-keyboard: IME: Window got focus change in sysmgr net.riverstonerelay.jihad-browser focused: 0, fieldtype: 0, fieldactions: 0x00

### enyo / pan-long-drag — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +3 s, load done at +10 s, 5 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's 
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 7-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 8-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 9-content: region 0,150-768,400: colours 321 hash 98b5debf rows 250 uniform 0 light 150-396; sel:#content p on the screen at 16,154-748,210, text holds [quick brown]
    CHECK ok 13-card-scroll: the card's scroll y 650 (want 650 within 80); the engine's 650
    CHECK ok 14-region-equal: region 0,83-756,137: colours 208 hash 34266017 rows 54 uniform 33 light none = the sample's hash
    CHECK ok 15-region-changed: region 0,300-756,900: colours 680 hash 52c6519f rows 600 uniform 219 light 300-899 against the sample's hash 6a2b3a47
    CHECK ok 16-top: the page's top measured 82 (the tap at y 212 landed on document y 780), the model says 82 (header hidden: 1)
    CHECK ok 18-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### enyo / settings-tabs — pass

    enyo card on about:settings: connected at +1 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 6-rect page-title
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 117 landed on document y 35)
    CHECK ok 8-text: page-title = [Settings]
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane=content]
    CHECK ok 11-text-has: sel:h2.group-title = [Content] holds [Content]

### enyo / accelerometer — a person's

    the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side

### enyo / deliberate-fail — fail

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK FAIL 7-text: sel:#header = [jihad-long-static: a fixed header], want [not the header]

### mochi / keyboard-address — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +3 s, load done at +10 s, 5 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 8-keyboard: IME: Window got focus change in sysmgr net.riverstonerelay.jihad-browser-mochi focused: 1, fieldtype: 7, fieldactions: 0
    CHECK ok 10-keyboard: IME: Window got focus change in sysmgr net.riverstonerelay.jihad-browser-mochi focused: 0, fieldtype: 0, fieldactions: 0

### mochi / pan-long-drag — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +3 s, load done at +10 s, 7 content frames at +10 s; the GPU path, frames off the main thread (the express lane) (the job's
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 7-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 8-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 9-content: region 0,150-768,400: colours 276 hash 0c6c964c rows 250 uniform 0 light 150-399; sel:#content p on the screen at 16,150-748,206, text holds [quick brown]
    CHECK ok 13-card-scroll: the card's scroll y 639 (want 650 within 80); the engine's 639
    CHECK ok 14-region-equal: region 0,79-756,133: colours 204 hash 5486acf3 rows 54 uniform 33 light none = the sample's hash
    CHECK ok 15-region-changed: region 0,300-756,900: colours 840 hash 3f74d198 rows 600 uniform 214 light 307-899 against the sample's hash 653a2a3a
    CHECK ok 16-top: the page's top measured 78 (the tap at y 219 landed on document y 780), the model says 78 (header hidden: 1)
    CHECK ok 18-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mochi / settings-tabs — pass

    mochi card on about:settings: connected at +2 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 6-rect page-title
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 113 landed on document y 35)
    CHECK ok 8-text: page-title = [Settings]
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane=content]
    CHECK ok 11-text-has: sel:h2.group-title = [Content] holds [Content]

### mochi / accelerometer — a person's

    the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side

### mojo / keyboard-address — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +8 s, 2 content frames at +38 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 8-keyboard: IME: Window got focus change in sysmgr net.riverstonerelay.jihad-browser-mojo focused: 1, fieldtype: 0, fieldactions: 0x
    CHECK ok 10-keyboard: IME: Window got focus change in sysmgr net.riverstonerelay.jihad-browser-mojo focused: 0, fieldtype: 0, fieldactions: 0x

### mojo / pan-long-drag — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +7 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 7-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 8-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 9-content: region 0,150-768,400: colours 293 hash 8b1a67cb rows 250 uniform 0 light 150-399; sel:#content p on the screen at 16,145-748,201, text holds [quick brown]
    CHECK ok 13-card-scroll: the card's scroll y 594 (want 650 within 80); the engine's 594
    CHECK ok 14-region-equal: region 0,29-756,83: colours 185 hash eeaac7d6 rows 54 uniform 33 light none = the sample's hash
    CHECK ok 15-region-changed: region 0,300-756,900: colours 488 hash 8a2d02d6 rows 600 uniform 219 light 302-899 against the sample's hash 5ec8059e
    CHECK ok 16-top: the page's top measured 73 (the tap at y 214 landed on document y 780), the model says 73 (header hidden: 1)
    CHECK ok 18-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mojo / settings-tabs — pass

    mojo card on about:settings: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 6-rect page-title
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 108 landed on document y 35)
    CHECK ok 8-text: page-title = [Settings]
    WAIT ok 10-rect sel:button.pane-tab.on[data-pane=content]
    CHECK ok 11-text-has: sel:h2.group-title = [Content] holds [Content]

### mojo / accelerometer — a person's

    the device's rotation by the accelerometer; the self-test hook's rotate scenario covers the window's side

