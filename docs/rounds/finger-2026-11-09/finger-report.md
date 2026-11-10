# Finger suite report — 2026-11-09T22:16:25

Shells: enyo, mochi, mojo. Scripts: 1. 413 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | media-scrub-drag | **fail** | 1 ok, 8 failed | CHECK FAIL 8-wait-rect: no element v with a size within the bound / CHECK FAIL 10-wait-text: status never read playing within the bound / CHECK FAIL 11-tap: the tap on v reached no clickAt / CHECK FAIL 12-wait-text: status never read paused within the bound / CHECK FAIL 13-wait-rect: no element anon; frames: kept/enyo-media-scrub-drag-8-wait-rect_.jpg, kept/enyo-media-scrub-drag-10-wait-text_.jpg, kept/enyo-media-scrub-drag-11-tap_.jpg, kept/enyo-media-scrub-drag-12-wait-text_.jpg, kept/enyo-media-scrub-drag-13-wait-rect_.jpg, kept/enyo-media-scrub-drag-14-wait-rect_.jpg, kept/enyo-media-scrub-drag-18-wait-text_.jpg, kept/enyo-media-scrub-drag-23-wait-text_.jpg, kept/enyo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| mochi | media-scrub-drag | **pass** | 8 ok, 0 failed | WAIT ok 23-text seek = [60]; frames: kept/mochi-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |
| mojo | media-scrub-drag | **known** | 7 ok, 1 failed | the open defect T-598: CHECK FAIL 23-wait-text: seek never read 60 within the bound; frames: kept/mojo-media-scrub-drag-23-wait-text_.jpg, kept/mojo-media-scrub-drag-seek_.jpg | cavekit-gre-widgets.md R1 |

## The device at the end

```
DEVICE ok job-enyo: (1515678/135580422) jihad (start) running, process 16078
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (1515688/627862642) jihad-mochi (start) running, process 20111
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (1515700/793907233) jihad-mojo (start) running, process 24640
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
DEVICE ok media-scratch: none
DEVICE ok overall: 4% of /tmp used
```

## Every test's lines

### enyo / media-scrub-drag — fail

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +2 s, load done at +8 s, 3 content frames at +9 s; the GPU path, frames off the main thread (the exp
    CHECK FAIL 8-wait-rect: no element v with a size within the bound
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 102 landed on document y 25)
    CHECK FAIL 10-wait-text: status never read playing within the bound
    CHECK FAIL 11-tap: the tap on v reached no clickAt
    CHECK FAIL 12-wait-text: status never read paused within the bound
    CHECK FAIL 13-wait-rect: no element anon:#v|controlBar with a size within the bound
    CHECK FAIL 14-wait-rect: no element anon:#v|volumeControl with a size within the bound
    CHECK FAIL 18-wait-text: vol never read volume=0. within the bound
    WAIT ok 22-settled: the card's scroll 0 0
    CHECK FAIL 23-wait-text: seek never read 60 within the bound

### mochi / media-scrub-drag — pass

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +4 s, load done at +13 s, 13 content frames at +13 s; the GPU path, frames off the main thread (the
    WAIT ok 8-rect v
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 98 landed on document y 25)
    WAIT ok 10-text status = [playing]
    WAIT ok 12-text status = [paused t=11.5]
    WAIT ok 13-rect anon:#v|controlBar
    WAIT ok 14-rect anon:#v|volumeControl
    WAIT ok 18-text vol = [volume=0.16 muted=false]
    WAIT ok 22-settled: the card's scroll 0 0
    WAIT ok 23-text seek = [60]

### mojo / media-scrub-drag — known

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1: connected at +1 s, load done at +10 s, 16 content frames at +10 s; the GPU path, frames off the main thread (the 
    WAIT ok 8-rect v
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 93 landed on document y 25)
    WAIT ok 10-text status = [playing]
    WAIT ok 12-text status = [paused t=19.3]
    WAIT ok 13-rect anon:#v|controlBar
    WAIT ok 14-rect anon:#v|volumeControl
    WAIT ok 18-text vol = [volume=0.16 muted=false]
    WAIT ok 22-settled: the card's scroll 0 0
    CHECK FAIL 23-wait-text: seek never read 60 within the bound

