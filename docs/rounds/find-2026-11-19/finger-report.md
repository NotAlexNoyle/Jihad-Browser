# Finger suite report — 2026-11-19T18:59:47

First run 2026-11-19T18:03:21; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 8. 3808 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | find-ctrl-f | **pass** | 9 ok, 0 failed | kernel keyboard: up; frames: kept/enyo-find-ctrl-f-ctrl-f_.jpg, kept/enyo-find-ctrl-f-ctrl-f-page_.jpg | cavekit-find-on-page.md R1 |
| enyo | find-hold-menu | **pass** | 8 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-find-on-page.md R1 |
| enyo | find-navigate | **pass** | 9 ok, 0 failed | CHECK ok 24-url: load done uri=http://192.168.1.245:8077/jihad-card-events.html; frames: kept/enyo-find-navigate-new-page_.jpg | cavekit-find-on-page.md R5 |
| enyo | find-none | **pass** | 6 ok, 0 failed | WAIT ok 23-line: [jihad-bs] js-console (no source):0:0 page: find: cleared; frames: kept/enyo-find-none-no-matches_.jpg | cavekit-find-on-page.md R4 |
| enyo | find-own-pages | **pass** | 15 ok, 0 failed | WAIT ok 38-line: [jihad-bs] js-console (no source):0:0 page: find: cleared; frames: kept/enyo-find-own-pages-own-page_.jpg | cavekit-find-on-page.md R1 |
| enyo | find-reopen | **pass** | 22 ok, 0 failed | WAIT ok 61-line: [jihad-bs] js-console (no source):0:0 page: find: cleared; frames: kept/enyo-find-reopen-dark-band_.jpg, kept/enyo-find-reopen-reopened_.jpg | cavekit-find-on-page.md R2, R5 |
| enyo | find-steps | **pass** | 19 ok, 0 failed | CHECK ok 57-probe-lacks: the probe's answer lacks [/y=0/]; frames: kept/enyo-find-steps-went-round-back_.jpg, kept/enyo-find-steps-after-done_.jpg | cavekit-find-on-page.md R1, R2, R3, R4, R5 |
| enyo | find-text | **pass** | 11 ok, 0 failed | WAIT ok 32-line: [jihad-bs] notify: find-result: {"index":2,"total":7,"more":false,"wrapped":false} (pushed) | cavekit-find-on-page.md R2, R4 |
| mochi | find-ctrl-f | **pass** | 9 ok, 0 failed | kernel keyboard: up; frames: kept/mochi-find-ctrl-f-ctrl-f_.jpg, kept/mochi-find-ctrl-f-ctrl-f-page_.jpg | cavekit-find-on-page.md R1 |
| mochi | find-hold-menu | **pass** | 8 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-find-on-page.md R1 |
| mochi | find-navigate | **pass** | 9 ok, 0 failed | CHECK ok 24-url: load done uri=http://192.168.1.245:8077/jihad-card-events.html; frames: kept/mochi-find-navigate-new-page_.jpg | cavekit-find-on-page.md R5 |
| mochi | find-none | **pass** | 6 ok, 0 failed | WAIT ok 23-line: [jihad-bs] js-console (no source):0:0 page: find: cleared; frames: kept/mochi-find-none-no-matches_.jpg | cavekit-find-on-page.md R4 |
| mochi | find-own-pages | **pass** | 15 ok, 0 failed | WAIT ok 38-line: [jihad-bs] js-console (no source):0:0 page: find: cleared; frames: kept/mochi-find-own-pages-own-page_.jpg | cavekit-find-on-page.md R1 |
| mochi | find-reopen | **pass** | 22 ok, 0 failed | WAIT ok 61-line: [jihad-bs] js-console (no source):0:0 page: find: cleared; frames: kept/mochi-find-reopen-dark-band_.jpg, kept/mochi-find-reopen-reopened_.jpg | cavekit-find-on-page.md R2, R5 |
| mochi | find-steps | **pass** | 19 ok, 0 failed | CHECK ok 57-probe-lacks: the probe's answer lacks [/y=0/]; frames: kept/mochi-find-steps-went-round-back_.jpg, kept/mochi-find-steps-after-done_.jpg | cavekit-find-on-page.md R1, R2, R3, R4, R5 |
| mochi | find-text | **pass** | 11 ok, 0 failed | WAIT ok 32-line: [jihad-bs] notify: find-result: {"index":2,"total":7,"more":false,"wrapped":false} (pushed) | cavekit-find-on-page.md R2, R4 |
| mojo | find-ctrl-f (rerun) | **pass** | 9 ok, 0 failed | kernel keyboard: up; frames: kept/mojo-find-ctrl-f-ctrl-f_.jpg, kept/mojo-find-ctrl-f-ctrl-f-page_.jpg | cavekit-find-on-page.md R1 |
| mojo | find-hold-menu (rerun) | **pass** | 8 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-find-on-page.md R1 |
| mojo | find-navigate (rerun) | **pass** | 9 ok, 0 failed | CHECK ok 24-url: load done uri=http://192.168.1.245:8077/jihad-card-events.html; frames: kept/mojo-find-navigate-new-page_.jpg | cavekit-find-on-page.md R5 |
| mojo | find-none (rerun) | **pass** | 6 ok, 0 failed | WAIT ok 23-line: [jihad-bs] js-console (no source):0:0 page: find: cleared; frames: kept/mojo-find-none-no-matches_.jpg | cavekit-find-on-page.md R4 |
| mojo | find-own-pages (rerun) | **pass** | 15 ok, 0 failed | WAIT ok 38-line: [jihad-bs] js-console (no source):0:0 page: find: cleared; frames: kept/mojo-find-own-pages-own-page_.jpg | cavekit-find-on-page.md R1 |
| mojo | find-reopen (rerun) | **pass** | 22 ok, 0 failed | WAIT ok 61-line: [jihad-bs] js-console (no source):0:0 page: find: cleared; frames: kept/mojo-find-reopen-dark-band_.jpg, kept/mojo-find-reopen-reopened_.jpg | cavekit-find-on-page.md R2, R5 |
| mojo | find-steps (rerun) | **pass** | 19 ok, 0 failed | CHECK ok 57-probe-lacks: the probe's answer lacks [/y=0/]; frames: kept/mojo-find-steps-went-round-back_.jpg, kept/mojo-find-steps-after-done_.jpg | cavekit-find-on-page.md R1, R2, R3, R4, R5 |
| mojo | find-text (rerun) | **pass** | 11 ok, 0 failed | WAIT ok 32-line: [jihad-bs] notify: find-result: {"index":2,"total":7,"more":false,"wrapped":false} (pushed) | cavekit-find-on-page.md R2, R4 |
| enyo | flash-key-modifiers (rerun) | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R7 |
| enyo | keyboard-modifiers (rerun) | **pass** | 6 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| mochi | flash-key-modifiers (rerun) | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R7 |
| mochi | keyboard-modifiers (rerun) | **pass** | 6 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |
| mojo | flash-key-modifiers (rerun) | **pass** | 10 ok, 0 failed | kernel keyboard: up | cavekit-flash.md R7 |
| mojo | keyboard-modifiers (rerun) | **pass** | 6 ok, 0 failed | kernel keyboard: up | cavekit-input-bridging.md R2 |

## The device at the end

```
DEVICE ok job-enyo: (36935/688271514) jihad (start) running, process 20245
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (36941/244772551) jihad-mochi (start) running, process 31469
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (36946/485828105) jihad-mojo (start) running, process 21892
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
DEVICE ok overall: 2% of /tmp used
```

## Every test's lines

### enyo / find-ctrl-f — pass

    enyo card on http://192.168.1.245:8077/jihad-find.html: connected at +2 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 23-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 24-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 28-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    WAIT ok 30-launch: load done
    WAIT ok 31-rect plain
    calibrated (enyo): the page's top on the screen 83 (the table said 82, the tap at y 143 landed on document y 77)
    WAIT ok 37-line: [jihad-bs] notify: find-open: 1 (pushed)
    WAIT ok 45-line: [jihad-bs] find (6 bytes) -> 1
    WAIT ok 46-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 50-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### enyo / find-hold-menu — pass

    enyo card on http://192.168.1.245:8077/jihad-find.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 133 landed on document y 51)
    WAIT ok 12-line: [jihad-bs] notify: page-hold: {"editable":false,"readOnly":false,"selected":false} (pushed)
    WAIT ok 13-differs 0,100,500,500
    WAIT ok 14-still 0,100,500,500
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / find-navigate — pass

    enyo card on http://192.168.1.245:8077/jihad-find.html: connected at +2 s, load done at +7 s, 9 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 133 landed on document y 51)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 21-load
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 24-url: load done uri=http://192.168.1.245:8077/jihad-card-events.html

### enyo / find-none — pass

    enyo card on http://192.168.1.245:8077/jihad-find.html: connected at +2 s, load done at +6 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 133 landed on document y 51)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (4 bytes) -> 0
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":0,"total":0,"more":false,"wrapped":false} (pushed)
    WAIT ok 23-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### enyo / find-own-pages — pass

    enyo card on about:settings: connected at +2 s, load done at +7 s, 3 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 9-rect page-title
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (8 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":3,"more":false,"wrapped":false} (pushed)
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    WAIT ok 23-launch: load done
    WAIT ok 24-rect page-title
    WAIT ok 27-differs 0,30,300,180
    WAIT ok 28-still 0,30,300,180
    CHECK ok 31-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 33-line: [jihad-bs] find (9 bytes) -> 1
    WAIT ok 34-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 38-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### enyo / find-reopen — pass

    enyo card on http://192.168.1.245:8077/jihad-find.html: connected at +2 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 133 landed on document y 51)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    WAIT ok 25-line: [jihad-bs] js-console (no source):0:0 page: find: 3 of 6
    WAIT ok 28-line: [jihad-bs] js-console (no source):0:0 page: find: 4 of 6
    WAIT ok 32-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 33-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 36-differs 0,30,300,180
    WAIT ok 37-still 0,30,300,180
    CHECK ok 39-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 42-line: [jihad-bs] find (4 bytes) -> 1
    WAIT ok 43-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 46-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    WAIT ok 51-differs 0,30,300,180
    WAIT ok 52-still 0,30,300,180
    CHECK ok 54-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 57-line: [jihad-bs] find (4 bytes) -> 1
    WAIT ok 58-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 61-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### enyo / find-steps — pass

    enyo card on http://192.168.1.245:8077/jihad-find.html: connected at +1 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 133 landed on document y 51)
    WAIT ok 17-differs 0,30,300,180
    WAIT ok 18-still 0,30,300,180
    CHECK ok 21-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 23-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 24-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 27-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    CHECK ok 28-above-keyboard: p2 on the screen at 16,363-748,387, above the keyboard
    CHECK ok 29-visible: p2 on the screen at 16,363-748,387 (the reach ends at 600, keyboard 1)
    WAIT ok 32-line: [jihad-bs] js-console (no source):0:0 page: find: 3 of 6
    CHECK ok 34-probe-has: the probe's answer holds [ZEBRA]
    WAIT ok 37-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    WAIT ok 40-line: [jihad-bs] js-console (no source):0:0 page: find: 1 of 6
    WAIT ok 43-line: [jihad-bs] js-console (no source):0:0 page: find: 6 of 6, went round
    WAIT ok 47-line: [jihad-bs] js-console (no source):0:0 page: find: 1 of 6, went round
    WAIT ok 50-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    WAIT ok 53-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 54-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 56-probe-lacks: the probe's answer lacks [Zebra]
    CHECK ok 57-probe-lacks: the probe's answer lacks [|y=0|]

### enyo / find-text — pass

    enyo card on http://192.168.1.245:8077/jihad-find.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 133 landed on document y 51)
    WAIT ok 13-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 15-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 17-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 21-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 23-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 25-line: [jihad-bs] inject find "okapi" -> 1
    WAIT ok 26-line: [jihad-bs] notify: find-result: {"index":0,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 28-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    CHECK ok 30-probe-has: the probe's answer holds [added]
    WAIT ok 32-line: [jihad-bs] notify: find-result: {"index":2,"total":7,"more":false,"wrapped":false} (pushed)

### mochi / find-ctrl-f — pass

    mochi card on http://192.168.1.245:8077/jihad-find.html: connected at +3 s, load done at +7 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    WAIT ok 23-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 24-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 28-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    WAIT ok 30-launch: load done
    WAIT ok 31-rect plain
    calibrated (mochi): the page's top on the screen 79 (the table said 78, the tap at y 139 landed on document y 77)
    WAIT ok 37-line: [jihad-bs] notify: find-open: 1 (pushed)
    WAIT ok 45-line: [jihad-bs] find (6 bytes) -> 1
    WAIT ok 46-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 50-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### mochi / find-hold-menu — pass

    mochi card on http://192.168.1.245:8077/jihad-find.html: connected at +3 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 129 landed on document y 51)
    WAIT ok 12-line: [jihad-bs] notify: page-hold: {"editable":false,"readOnly":false,"selected":false} (pushed)
    WAIT ok 13-differs 0,100,500,500
    WAIT ok 14-still 0,100,500,500
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / find-navigate — pass

    mochi card on http://192.168.1.245:8077/jihad-find.html: connected at +2 s, load done at +6 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 129 landed on document y 51)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 21-load
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 24-url: load done uri=http://192.168.1.245:8077/jihad-card-events.html

### mochi / find-none — pass

    mochi card on http://192.168.1.245:8077/jihad-find.html: connected at +1 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 129 landed on document y 51)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (4 bytes) -> 0
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":0,"total":0,"more":false,"wrapped":false} (pushed)
    WAIT ok 23-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### mochi / find-own-pages — pass

    mochi card on about:settings: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 9-rect page-title
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (8 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":3,"more":false,"wrapped":false} (pushed)
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    WAIT ok 23-launch: load done
    WAIT ok 24-rect page-title
    WAIT ok 27-differs 0,30,300,180
    WAIT ok 28-still 0,30,300,180
    CHECK ok 31-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 33-line: [jihad-bs] find (9 bytes) -> 1
    WAIT ok 34-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 38-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### mochi / find-reopen — pass

    mochi card on http://192.168.1.245:8077/jihad-find.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 129 landed on document y 51)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    WAIT ok 25-line: [jihad-bs] js-console (no source):0:0 page: find: 3 of 6
    WAIT ok 28-line: [jihad-bs] js-console (no source):0:0 page: find: 4 of 6
    WAIT ok 32-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 33-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 36-differs 0,30,300,180
    WAIT ok 37-still 0,30,300,180
    CHECK ok 39-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 42-line: [jihad-bs] find (4 bytes) -> 1
    WAIT ok 43-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 46-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    WAIT ok 51-differs 0,30,300,180
    WAIT ok 52-still 0,30,300,180
    CHECK ok 54-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 57-line: [jihad-bs] find (4 bytes) -> 1
    WAIT ok 58-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 61-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### mochi / find-steps — pass

    mochi card on http://192.168.1.245:8077/jihad-find.html: connected at +2 s, load done at +7 s, 3 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 129 landed on document y 51)
    WAIT ok 17-differs 0,30,300,180
    WAIT ok 18-still 0,30,300,180
    CHECK ok 21-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 23-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 24-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 27-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    CHECK ok 28-above-keyboard: p2 on the screen at 16,364-748,388, above the keyboard
    CHECK ok 29-visible: p2 on the screen at 16,364-748,388 (the reach ends at 600, keyboard 1)
    WAIT ok 32-line: [jihad-bs] js-console (no source):0:0 page: find: 3 of 6
    CHECK ok 34-probe-has: the probe's answer holds [ZEBRA]
    WAIT ok 37-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    WAIT ok 40-line: [jihad-bs] js-console (no source):0:0 page: find: 1 of 6
    WAIT ok 43-line: [jihad-bs] js-console (no source):0:0 page: find: 6 of 6, went round
    WAIT ok 47-line: [jihad-bs] js-console (no source):0:0 page: find: 1 of 6, went round
    WAIT ok 50-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    WAIT ok 53-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 54-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 56-probe-lacks: the probe's answer lacks [Zebra]
    CHECK ok 57-probe-lacks: the probe's answer lacks [|y=0|]

### mochi / find-text — pass

    mochi card on http://192.168.1.245:8077/jihad-find.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 129 landed on document y 51)
    WAIT ok 13-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 15-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 17-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 21-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 23-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 25-line: [jihad-bs] inject find "okapi" -> 1
    WAIT ok 26-line: [jihad-bs] notify: find-result: {"index":0,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 28-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    CHECK ok 30-probe-has: the probe's answer holds [added]
    WAIT ok 32-line: [jihad-bs] notify: find-result: {"index":2,"total":7,"more":false,"wrapped":false} (pushed)

### mojo / find-ctrl-f — pass

    mojo card on http://192.168.1.245:8077/jihad-find.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    WAIT ok 23-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 24-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 28-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    WAIT ok 30-launch: load done
    WAIT ok 31-rect plain
    calibrated (mojo): the page's top on the screen 74 (the table said 73, the tap at y 134 landed on document y 77)
    WAIT ok 37-line: [jihad-bs] notify: find-open: 1 (pushed)
    WAIT ok 45-line: [jihad-bs] find (6 bytes) -> 1
    WAIT ok 46-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 50-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### mojo / find-hold-menu — pass

    mojo card on http://192.168.1.245:8077/jihad-find.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 124 landed on document y 51)
    WAIT ok 12-line: [jihad-bs] notify: page-hold: {"editable":false,"readOnly":false,"selected":false} (pushed)
    WAIT ok 13-differs 0,100,500,500
    WAIT ok 14-still 0,100,500,500
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / find-navigate — pass

    mojo card on http://192.168.1.245:8077/jihad-find.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 124 landed on document y 51)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 21-load
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 23-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 24-url: load done uri=http://192.168.1.245:8077/jihad-card-events.html

### mojo / find-none — pass

    mojo card on http://192.168.1.245:8077/jihad-find.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 124 landed on document y 51)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (4 bytes) -> 0
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":0,"total":0,"more":false,"wrapped":false} (pushed)
    WAIT ok 23-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### mojo / find-own-pages — pass

    mojo card on about:settings: connected at +1 s, load done at +5 s, 4 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 9-rect page-title
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (8 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":3,"more":false,"wrapped":false} (pushed)
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    WAIT ok 23-launch: load done
    WAIT ok 24-rect page-title
    WAIT ok 27-differs 0,30,300,180
    WAIT ok 28-still 0,30,300,180
    CHECK ok 31-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 33-line: [jihad-bs] find (9 bytes) -> 1
    WAIT ok 34-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 38-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### mojo / find-reopen — pass

    mojo card on http://192.168.1.245:8077/jihad-find.html: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 124 landed on document y 51)
    WAIT ok 12-differs 0,30,300,180
    WAIT ok 13-still 0,30,300,180
    CHECK ok 16-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 22-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    WAIT ok 25-line: [jihad-bs] js-console (no source):0:0 page: find: 3 of 6
    WAIT ok 28-line: [jihad-bs] js-console (no source):0:0 page: find: 4 of 6
    WAIT ok 32-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 33-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 36-differs 0,30,300,180
    WAIT ok 37-still 0,30,300,180
    CHECK ok 39-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 42-line: [jihad-bs] find (4 bytes) -> 1
    WAIT ok 43-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 46-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    WAIT ok 51-differs 0,30,300,180
    WAIT ok 52-still 0,30,300,180
    CHECK ok 54-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 57-line: [jihad-bs] find (4 bytes) -> 1
    WAIT ok 58-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 61-line: [jihad-bs] js-console (no source):0:0 page: find: cleared

### mojo / find-steps — pass

    mojo card on http://192.168.1.245:8077/jihad-find.html: connected at +1 s, load done at +5 s, 3 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 124 landed on document y 51)
    WAIT ok 17-differs 0,30,300,180
    WAIT ok 18-still 0,30,300,180
    CHECK ok 21-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 23-line: [jihad-bs] find (5 bytes) -> 1
    WAIT ok 24-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 27-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    CHECK ok 28-above-keyboard: p2 on the screen at 16,414-748,438, above the keyboard
    CHECK ok 29-visible: p2 on the screen at 16,414-748,438 (the reach ends at 600, keyboard 1)
    WAIT ok 32-line: [jihad-bs] js-console (no source):0:0 page: find: 3 of 6
    CHECK ok 34-probe-has: the probe's answer holds [ZEBRA]
    WAIT ok 37-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    WAIT ok 40-line: [jihad-bs] js-console (no source):0:0 page: find: 1 of 6
    WAIT ok 43-line: [jihad-bs] js-console (no source):0:0 page: find: 6 of 6, went round
    WAIT ok 47-line: [jihad-bs] js-console (no source):0:0 page: find: 1 of 6, went round
    WAIT ok 50-line: [jihad-bs] js-console (no source):0:0 page: find: 2 of 6
    WAIT ok 53-line: [jihad-bs] js-console (no source):0:0 page: find: cleared
    CHECK ok 54-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 56-probe-lacks: the probe's answer lacks [Zebra]
    CHECK ok 57-probe-lacks: the probe's answer lacks [|y=0|]

### mojo / find-text — pass

    mojo card on http://192.168.1.245:8077/jihad-find.html: connected at +1 s, load done at +6 s, 2 content frames at +36 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GP
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 124 landed on document y 51)
    WAIT ok 13-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    WAIT ok 15-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 17-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 19-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 21-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 23-line: [jihad-bs] notify: find-result: {"index":1,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 25-line: [jihad-bs] inject find "okapi" -> 1
    WAIT ok 26-line: [jihad-bs] notify: find-result: {"index":0,"total":1,"more":false,"wrapped":false} (pushed)
    WAIT ok 28-line: [jihad-bs] notify: find-result: {"index":1,"total":6,"more":false,"wrapped":false} (pushed)
    CHECK ok 30-probe-has: the probe's answer holds [added]
    WAIT ok 32-line: [jihad-bs] notify: find-result: {"index":2,"total":7,"more":false,"wrapped":false} (pushed)

### enyo / flash-key-modifiers — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +2 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 15-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 20-text text = [text a]
    WAIT ok 23-text text = [text aB]
    WAIT ok 24-line: [jihad-bs] key: the modifier mask 0x80 read as 0x4
    WAIT ok 26-text keydown = [keydown 17 17]
    WAIT ok 28-text keydown = [keydown 86 118]
    WAIT ok 29-line: [jihad-bs] key: the modifier mask 0x20 read as 0x1
    CHECK ok 30-text-has: text = [text aBv] holds [v]
    CHECK ok 31-text-lacks: text = [text aBv] lacks [prime]
    CHECK ok 32-clipboard: the system clipboard holds [jihad-prime-42]

### enyo / keyboard-modifiers — pass

    enyo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 11-rect plain
    calibrated (enyo): the page's top on the screen 83 (the table said 82, the tap at y 143 landed on document y 77)
    CHECK ok 14-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 19-line: [jihad-bs] key: the modifier mask 0x40 read as 0x1
    WAIT ok 22-text echo = [plain n=1 c=0 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 24-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=0]
    WAIT ok 26-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]

### mochi / flash-key-modifiers — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +3 s, load done at +9 s, 12 content frames at +9 s; the GPU path, frames off the main thread (the express lane) 
    WAIT ok 15-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 20-text text = [text a]
    WAIT ok 23-text text = [text aB]
    WAIT ok 24-line: [jihad-bs] key: the modifier mask 0x80 read as 0x4
    WAIT ok 26-text keydown = [keydown 17 17]
    WAIT ok 28-text keydown = [keydown 86 118]
    WAIT ok 29-line: [jihad-bs] key: the modifier mask 0x20 read as 0x1
    CHECK ok 30-text-has: text = [text aBv] holds [v]
    CHECK ok 31-text-lacks: text = [text aBv] lacks [prime]
    CHECK ok 32-clipboard: the system clipboard holds [jihad-prime-42]

### mochi / keyboard-modifiers — pass

    mochi card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +3 s, load done at +7 s, 6 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 11-rect plain
    calibrated (mochi): the page's top on the screen 79 (the table said 78, the tap at y 139 landed on document y 77)
    CHECK ok 14-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 19-line: [jihad-bs] key: the modifier mask 0x40 read as 0x1
    WAIT ok 22-text echo = [plain n=1 c=0 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 24-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=0]
    WAIT ok 26-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]

### mojo / flash-key-modifiers — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=text: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 15-text ready = [ready text ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 20-text text = [text a]
    WAIT ok 23-text text = [text aB]
    WAIT ok 24-line: [jihad-bs] key: the modifier mask 0x80 read as 0x4
    WAIT ok 26-text keydown = [keydown 17 17]
    WAIT ok 28-text keydown = [keydown 86 118]
    WAIT ok 29-line: [jihad-bs] key: the modifier mask 0x20 read as 0x1
    CHECK ok 30-text-has: text = [text aBv] holds [v]
    CHECK ok 31-text-lacks: text = [text aBv] lacks [prime]
    CHECK ok 32-clipboard: the system clipboard holds [jihad-prime-42]

### mojo / keyboard-modifiers — pass

    mojo card on http://192.168.1.245:8077/jihad-input-echo.html: connected at +2 s, load done at +6 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 11-rect plain
    calibrated (mojo): the page's top on the screen 74 (the table said 73, the tap at y 134 landed on document y 77)
    CHECK ok 14-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 19-line: [jihad-bs] key: the modifier mask 0x40 read as 0x1
    WAIT ok 22-text echo = [plain n=1 c=0 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]
    WAIT ok 24-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=area keydowns=0]
    WAIT ok 26-text echo = [plain n=1 c=1 last=[B] value=[B] area n=0 c=0 last=[] value=[] ctl n=0 c=0 last=[] state=[] value=[] num n=0 c=0 last=[] value=[] focus=plain keydowns=0]

