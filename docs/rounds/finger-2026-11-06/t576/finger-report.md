# Finger suite report — 2026-11-06T10:54:30

Shells: enyo, mochi, mojo. Scripts: 14. 2727 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,287: 3.0 s, flips 41 (13.7/s), changes 0 (0.0/s), reads 42, distinct 1, spacing median 68 p90 137 max 158 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| enyo | flash-sound | **a person's** | 0 ok, 0 failed | hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route) | cavekit-engine-update.md R8 |
| enyo | pan-long-drag | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| enyo | wikipedia-keyboard-zoom | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-mochi-ui.md R6 |
| enyo | zoom-new-page | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-input-bridging.md R3 |
| enyo | flash-audio | **pass** | 4 ok, 0 failed | CHECK ok 10-audio: the movie's audio: closed | cavekit-flash.md R2 |
| enyo | flash-drawn | **pass** | 12 ok, 0 failed | CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,123-440,421: colours 2 hash 4db04545 rows 298 uniform 0 light none blue 123-420 | cavekit-flash.md R2 |
| enyo | flash-plugin | **pass** | 5 ok, 0 failed | CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?) | cavekit-flash.md R2 |
| enyo | flash-rate | **pass** | 5 ok, 0 failed | CHECK ok 12-flash-rate-report: content 40,123-440,421: 8.0 s, flips 115 (14.4/s), changes 115 (14.4/s), reads 116, distinct 60, spacing median 66 p90 85 max 160 ms; the movie's own rate 14.5096/s (frames 30 t=2632 .. frames 180 t=12970) | cavekit-flash.md R2 |
| enyo | flash-reacted | **pass** | 10 ok, 0 failed | CHECK ok 22-flash-reacted: reacted: yes (content 40,123-440,421: 3.0 s, flips 16 (5.3/s), changes 0 (0.0/s), reads 17, distinct 1, spacing median 185 p90 191 max 192 ms new 17 of 17 first 0 ms) | cavekit-flash.md R2 |
| enyo | flash-signature | **pass** | 5 ok, 0 failed | CHECK ok 13-screen: the screen against its reference: difference 8.4 (same is 5 or less; wanted other) | cavekit-flash.md R2 |
| enyo | flash-stage-z1 | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| enyo | flash-stage-z2 | **pass** | 23 ok, 0 failed | CHECK ok 45-flash-at: the movie reported down 150,119 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| enyo | flash-stage-zfit | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mochi | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,0-440,278: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 68 p90 86 max 162 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mochi | flash-sound | **a person's** | 0 ok, 0 failed | hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route) | cavekit-engine-update.md R8 |
| mochi | pan-long-drag | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mochi | wikipedia-keyboard-zoom | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-mochi-ui.md R6 |
| mochi | zoom-new-page | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-input-bridging.md R3 |
| mochi | flash-audio | **pass** | 4 ok, 0 failed | CHECK ok 10-audio: the movie's audio: closed | cavekit-flash.md R2 |
| mochi | flash-drawn | **pass** | 12 ok, 0 failed | CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,119-440,417: colours 2 hash 4db04545 rows 298 uniform 0 light none blue 119-416 | cavekit-flash.md R2 |
| mochi | flash-plugin | **pass** | 5 ok, 0 failed | CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?) | cavekit-flash.md R2 |
| mochi | flash-rate | **pass** | 5 ok, 0 failed | CHECK ok 12-flash-rate-report: content 40,119-440,417: 8.0 s, flips 114 (14.2/s), changes 114 (14.2/s), reads 115, distinct 60, spacing median 67 p90 84 max 163 ms; the movie's own rate 14.4286/s (frames 20 t=1778 .. frames 170 t=12174) | cavekit-flash.md R2 |
| mochi | flash-reacted | **pass** | 10 ok, 0 failed | CHECK ok 22-flash-reacted: reacted: yes (content 40,119-440,417: 3.0 s, flips 16 (5.3/s), changes 0 (0.0/s), reads 17, distinct 1, spacing median 188 p90 191 max 194 ms new 17 of 17 first 0 ms) | cavekit-flash.md R2 |
| mochi | flash-signature | **pass** | 5 ok, 0 failed | CHECK ok 13-screen: the screen against its reference: difference 8.3 (same is 5 or less; wanted other) | cavekit-flash.md R2 |
| mochi | flash-stage-z1 | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mochi | flash-stage-z2 | **pass** | 23 ok, 0 failed | CHECK ok 45-flash-at: the movie reported down 150,120 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mochi | flash-stage-zfit | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mojo | flash-lane | **pass** | 7 ok, 0 failed | CHECK ok 18-flash-rate: content 40,26-440,324: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 68 p90 85 max 170 ms (wanted 0..4/s) | cavekit-flash.md R3 |
| mojo | flash-sound | **a person's** | 0 ok, 0 failed | hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route) | cavekit-engine-update.md R8 |
| mojo | pan-long-drag | **pass** | 9 ok, 0 failed | CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56 | cavekit-gpu-rendering.md R3 |
| mojo | wikipedia-keyboard-zoom | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-mochi-ui.md R6 |
| mojo | zoom-new-page | **not run** | 0 ok, 0 failed | needs internet (run with --with internet) | cavekit-input-bridging.md R3 |
| mojo | flash-audio | **pass** | 4 ok, 0 failed | CHECK ok 10-audio: the movie's audio: closed | cavekit-flash.md R2 |
| mojo | flash-drawn | **pass** | 12 ok, 0 failed | CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,114-440,412: colours 1 hash b955b145 rows 298 uniform 298 light none blue 114-411 | cavekit-flash.md R2 |
| mojo | flash-plugin | **pass** | 5 ok, 0 failed | CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?) | cavekit-flash.md R2 |
| mojo | flash-rate | **pass** | 5 ok, 0 failed | CHECK ok 12-flash-rate-report: content 40,114-440,412: 8.0 s, flips 110 (13.8/s), changes 110 (13.8/s), reads 111, distinct 60, spacing median 69 p90 92 max 166 ms; the movie's own rate 14.3664/s (frames 50 t=3742 .. frames 200 t=14183) | cavekit-flash.md R2 |
| mojo | flash-reacted | **pass** | 10 ok, 0 failed | CHECK ok 22-flash-reacted: reacted: yes (content 40,114-440,412: 3.0 s, flips 15 (5.0/s), changes 0 (0.0/s), reads 21, distinct 1, spacing median 209 p90 217 max 219 ms new 21 of 21 first 0 ms) | cavekit-flash.md R2 |
| mojo | flash-signature | **pass** | 5 ok, 0 failed | CHECK ok 13-screen: the screen against its reference: difference 8.3 (same is 5 or less; wanted other) | cavekit-flash.md R2 |
| mojo | flash-stage-z1 | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mojo | flash-stage-z2 | **pass** | 23 ok, 0 failed | CHECK ok 45-flash-at: the movie reported down 150,119 for stage 150,120 (within 3) | cavekit-flash.md R2 |
| mojo | flash-stage-zfit | **pass** | 15 ok, 0 failed | CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3) | cavekit-flash.md R2 |

## The device at the end

```
DEVICE ok job-enyo: (1217871/207442109) jihad (start) running, process 14676
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (1217882/161728626) jihad-mochi (start) running, process 23390
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (1217898/89080178) jihad-mojo (start) running, process 32029
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

### enyo / flash-lane — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +3 s, load done at +10 s, 12 content frames at +11 s; the GPU path, frames off
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,123-440,421: 3.0 s, flips 42 (14.0/s), changes 42 (14.0/s), reads 43, distinct 43, spacing median 68 p90 79 max 197 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,123-440,421: 3.0 s, flips 47 (15.7/s), changes 0 (0.0/s), reads 48, distinct 1, spacing median 62 p90 92 max 163 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,123-440,421: 3.0 s, flips 43 (14.3/s), changes 43 (14.3/s), reads 44, distinct 44, spacing median 68 p90 90 max 162 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 135
    CHECK ok 16-flash-rate: content 40,0-440,287: 3.0 s, flips 42 (14.0/s), changes 25 (8.3/s), reads 43, distinct 26, spacing median 68 p90 88 max 189 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,287: 3.0 s, flips 41 (13.7/s), changes 0 (0.0/s), reads 42, distinct 1, spacing median 68 p90 137 max 158 ms (wanted 0..4/s)

### enyo / flash-sound — a person's

    hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route)

### enyo / pan-long-drag — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 321 hash 98b5debf rows 250 uniform 0 light 150-396 blue none; sel:#content p on the screen at 16,154-748,210, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 639 (want 650 within 100); the engine's 639
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,83-756,137: colours 208 hash 34266017 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 967 hash faecb0c6 rows 600 uniform 212 light 300-899 blue none against the sample's hash 6a2b3a47
    CHECK ok 18-top: the page's top measured 82 (the tap at y 223 landed on document y 780), the model says 82 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### enyo / wikipedia-keyboard-zoom — not run

    needs internet (run with --with internet)

### enyo / zoom-new-page — not run

    needs internet (run with --with internet)

### enyo / flash-audio — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-audio-long.swf: connected at +1 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main thread (the ex
    WAIT ok 7-rect m
    CHECK ok 8-audio: the movie's audio: open running
    WAIT ok 9-launch: load done
    CHECK ok 10-audio: the movie's audio: closed

### enyo / flash-drawn — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +2 s, load done at +9 s, 11 content frames at +9 s; the GPU path, frames off the main thread (the
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 9-flash-drawn: drawn: region 40,123-440,421: colours 32 hash 3dd6b7d0 rows 298 uniform 0 light none blue none
    WAIT ok 10-launch: load done
    WAIT ok 11-still m
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 40,123-440,421: colours 2 hash 5bd08ca5 rows 298 uniform 0 light 123-420 blue none
    WAIT ok 13-launch: load done
    WAIT ok 14-still m
    CHECK ok 15-flash-drawn: reported not drawn, as it should be: region 40,123-440,421: colours 2 hash 1cecd705 rows 298 uniform 0 light none blue none
    WAIT ok 16-launch: load done
    WAIT ok 17-still m
    CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,123-440,421: colours 2 hash 4db04545 rows 298 uniform 0 light none blue 123-420

### enyo / flash-plugin — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +3 s, load done at +11 s, 13 content frames at +11 s; the GPU path, frames off the main thread (the express lane)
    WAIT ok 7-rect m
    CHECK ok 8-plugin: plugin-container 21453: alive (state S, cpu ticks 96 to 104)
    CHECK ok 10-plugin: plugin-container 21453: wedged (state T, cpu ticks 104 to 104)
    CHECK ok 12-plugin: plugin-container 21453: alive (state S, cpu ticks 105 to 112)
    CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?)

### enyo / flash-rate — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate10.swf: connected at +2 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    CHECK ok 9-flash-rate-report: content 40,123-440,421: 8.0 s, flips 78 (9.8/s), changes 78 (9.8/s), reads 79, distinct 61, spacing median 98 p90 173 max 180 ms; the movie's own rate 9.76302/s (frames 50 t=5658 .. frames 160 t=16925)
    WAIT ok 10-launch: load done
    WAIT ok 11-rect m
    CHECK ok 12-flash-rate-report: content 40,123-440,421: 8.0 s, flips 115 (14.4/s), changes 115 (14.4/s), reads 116, distinct 60, spacing median 66 p90 85 max 160 ms; the movie's own rate 14.5096/s (frames 30 t=2632 .. frames 180 t=12970)

### enyo / flash-reacted — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +3 s, load done at +9 s, 5 content frames at +9 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 11-flash-reacted: reacted: no (content 40,123-440,421: 4.0 s, flips 22 (5.5/s), changes 1 (0.2/s), reads 23, distinct 2, spacing median 186 p90 193 max 199 ms new 0 of 23)
    WAIT ok 12-launch: load done
    WAIT ok 13-rect m
    CHECK ok 16-flash-reacted: reacted: no (content 40,123-440,421: 5.0 s, flips 143 (28.6/s), changes 143 (28.6/s), reads 144, distinct 40, spacing median 35 p90 38 max 50 ms new 0 of 144)
    WAIT ok 17-launch: load done
    WAIT ok 18-rect m
    WAIT ok 19-still m
    CHECK ok 22-flash-reacted: reacted: yes (content 40,123-440,421: 3.0 s, flips 16 (5.3/s), changes 0 (0.0/s), reads 17, distinct 1, spacing median 185 p90 191 max 192 ms new 17 of 17 first 0 ms)

### enyo / flash-signature — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-reacts.swf: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 10-screen: the screen against its reference: difference 0.0 (same is 5 or less; wanted same)
    WAIT ok 12-still m
    CHECK ok 13-screen: the screen against its reference: difference 8.4 (same is 5 or less; wanted other)

### enyo / flash-stage-z1 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main th
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,149.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 151.95,119.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,150.65 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 152,121.3 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,149.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 152,119.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3)

### enyo / flash-stage-z2 — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +2 s, load done at +9 s, 6 content frames at +9 s; the GPU path, frames off the main th
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-zoom
    CHECK ok 10-zoom: zoom 1.9268 (want 1.92 within 0.3)
    CHECK ok 13-flash-at: the movie reported down 199.95,149.95 for stage 200,150 (within 3)
    CHECK ok 15-flash-at: the movie reported down 149.95,119.95 for stage 150,120 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 19-zoom
    CHECK ok 20-zoom: zoom 1.9265 (want 1.92 within 0.3)
    CHECK ok 23-flash-at: the movie reported down 199.95,150.65 for stage 200,150 (within 3)
    CHECK ok 25-flash-at: the movie reported down 150.65,119.95 for stage 150,120 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 29-zoom
    CHECK ok 30-zoom: zoom 1.9361 (want 1.92 within 0.3)
    CHECK ok 33-flash-at: the movie reported down 199.95,149.95 for stage 200,150 (within 3)
    CHECK ok 35-flash-at: the movie reported down 149.3,117.95 for stage 150,120 (within 3)
    WAIT ok 36-launch: load done
    WAIT ok 37-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    WAIT ok 39-zoom
    CHECK ok 40-zoom: zoom 1.9416 (want 1.92 within 0.3)
    CHECK ok 43-flash-at: the movie reported down 200,150 for stage 200,150 (within 3)
    CHECK ok 45-flash-at: the movie reported down 150,119 for stage 150,120 (within 3)

### enyo / flash-stage-zfit — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150&fit=1: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the m
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,147.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 149.95,117.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,149.3 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 150.65,118.65 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,147.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 150.65,117.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,149 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3)

### mochi / flash-lane — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +5 s, load done at +12 s, 12 content frames at +12 s; the GPU path, frames of
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,119-440,417: 3.0 s, flips 42 (14.0/s), changes 41 (13.7/s), reads 43, distinct 42, spacing median 67 p90 100 max 170 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,119-440,417: 3.0 s, flips 44 (14.7/s), changes 0 (0.0/s), reads 45, distinct 1, spacing median 63 p90 111 max 163 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,119-440,417: 3.0 s, flips 44 (14.7/s), changes 44 (14.7/s), reads 45, distinct 45, spacing median 68 p90 82 max 164 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 140
    CHECK ok 16-flash-rate: content 40,0-440,278: 3.0 s, flips 44 (14.7/s), changes 27 (9.0/s), reads 45, distinct 28, spacing median 66 p90 84 max 154 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,0-440,278: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 68 p90 86 max 162 ms (wanted 0..4/s)

### mochi / flash-sound — a person's

    hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route)

### mochi / pan-long-drag — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +9 s, 4 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 276 hash 0c6c964c rows 250 uniform 0 light 150-399 blue none; sel:#content p on the screen at 16,150-748,206, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 639 (want 650 within 100); the engine's 639
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,79-756,133: colours 204 hash 5486acf3 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 840 hash 3f74d198 rows 600 uniform 214 light 307-899 blue none against the sample's hash 653a2a3a
    CHECK ok 18-top: the page's top measured 78 (the tap at y 219 landed on document y 780), the model says 78 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mochi / wikipedia-keyboard-zoom — not run

    needs internet (run with --with internet)

### mochi / zoom-new-page — not run

    needs internet (run with --with internet)

### mochi / flash-audio — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-audio-long.swf: connected at +2 s, load done at +9 s, 4 content frames at +9 s; the GPU path, frames off the main thread (the e
    WAIT ok 7-rect m
    CHECK ok 8-audio: the movie's audio: open running
    WAIT ok 9-launch: load done
    CHECK ok 10-audio: the movie's audio: closed

### mochi / flash-drawn — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main thread (the
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 9-flash-drawn: drawn: region 40,119-440,417: colours 32 hash 3dd6b7d0 rows 298 uniform 0 light none blue none
    WAIT ok 10-launch: load done
    WAIT ok 11-still m
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 40,119-440,417: colours 2 hash 5bd08ca5 rows 298 uniform 0 light 119-416 blue none
    WAIT ok 13-launch: load done
    WAIT ok 14-still m
    CHECK ok 15-flash-drawn: reported not drawn, as it should be: region 40,119-440,417: colours 2 hash 1cecd705 rows 298 uniform 0 light none blue none
    WAIT ok 16-launch: load done
    WAIT ok 17-still m
    CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,119-440,417: colours 2 hash 4db04545 rows 298 uniform 0 light none blue 119-416

### mochi / flash-plugin — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-rect m
    CHECK ok 8-plugin: plugin-container 30384: alive (state S, cpu ticks 98 to 105)
    CHECK ok 10-plugin: plugin-container 30384: wedged (state T, cpu ticks 106 to 106)
    CHECK ok 12-plugin: plugin-container 30384: alive (state S, cpu ticks 106 to 113)
    CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?)

### mochi / flash-rate — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate10.swf: connected at +4 s, load done at +11 s, 9 content frames at +11 s; the GPU path, frames off the main thread (t
    WAIT ok 7-rect m
    CHECK ok 9-flash-rate-report: content 40,119-440,417: 8.0 s, flips 78 (9.8/s), changes 78 (9.8/s), reads 80, distinct 60, spacing median 97 p90 173 max 209 ms; the movie's own rate 9.75134/s (frames 50 t=5748 .. frames 150 t=16003)
    WAIT ok 10-launch: load done
    WAIT ok 11-rect m
    CHECK ok 12-flash-rate-report: content 40,119-440,417: 8.0 s, flips 114 (14.2/s), changes 114 (14.2/s), reads 115, distinct 60, spacing median 67 p90 84 max 163 ms; the movie's own rate 14.4286/s (frames 20 t=1778 .. frames 170 t=12174)

### mochi / flash-reacted — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +2 s, load done at +9 s, 10 content frames at +9 s; the GPU path, frames off the main thread (th
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 11-flash-reacted: reacted: no (content 40,119-440,417: 4.0 s, flips 20 (5.0/s), changes 1 (0.2/s), reads 24, distinct 2, spacing median 189 p90 234 max 235 ms new 0 of 24)
    WAIT ok 12-launch: load done
    WAIT ok 13-rect m
    CHECK ok 16-flash-reacted: reacted: no (content 40,119-440,417: 5.0 s, flips 143 (28.6/s), changes 143 (28.6/s), reads 144, distinct 46, spacing median 35 p90 39 max 94 ms new 0 of 144)
    WAIT ok 17-launch: load done
    WAIT ok 18-rect m
    WAIT ok 19-still m
    CHECK ok 22-flash-reacted: reacted: yes (content 40,119-440,417: 3.0 s, flips 16 (5.3/s), changes 0 (0.0/s), reads 17, distinct 1, spacing median 188 p90 191 max 194 ms new 17 of 17 first 0 ms)

### mochi / flash-signature — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-reacts.swf: connected at +2 s, load done at +9 s, 9 content frames at +9 s; the GPU path, frames off the main thread (the
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 10-screen: the screen against its reference: difference 0.0 (same is 5 or less; wanted same)
    WAIT ok 12-still m
    CHECK ok 13-screen: the screen against its reference: difference 8.3 (same is 5 or less; wanted other)

### mochi / flash-stage-z1 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +2 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the main t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,149.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 151.95,119.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,150.65 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 152,121.3 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,149.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 152,119.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3)

### mochi / flash-stage-z2 — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +1 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the main t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-zoom
    CHECK ok 10-zoom: zoom 1.9326 (want 1.92 within 0.3)
    CHECK ok 13-flash-at: the movie reported down 199.95,147.95 for stage 200,150 (within 3)
    CHECK ok 15-flash-at: the movie reported down 149.95,117.95 for stage 150,120 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 19-zoom
    CHECK ok 20-zoom: zoom 1.9326 (want 1.92 within 0.3)
    CHECK ok 23-flash-at: the movie reported down 199.95,149.3 for stage 200,150 (within 3)
    CHECK ok 25-flash-at: the movie reported down 150.65,119.95 for stage 150,120 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 29-zoom
    CHECK ok 30-zoom: zoom 1.9361 (want 1.92 within 0.3)
    CHECK ok 33-flash-at: the movie reported down 199.95,149.95 for stage 200,150 (within 3)
    CHECK ok 35-flash-at: the movie reported down 149.3,117.95 for stage 150,120 (within 3)
    WAIT ok 36-launch: load done
    WAIT ok 37-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    WAIT ok 39-zoom
    CHECK ok 40-zoom: zoom 1.9265 (want 1.92 within 0.3)
    CHECK ok 43-flash-at: the movie reported down 200,150 for stage 200,150 (within 3)
    CHECK ok 45-flash-at: the movie reported down 150,120 for stage 150,120 (within 3)

### mochi / flash-stage-zfit — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150&fit=1: connected at +2 s, load done at +9 s, 7 content frames at +9 s; the GPU path, frames off the 
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,147.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 149.95,117.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,149.3 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 150.65,118.65 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,147.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 150.65,117.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,149 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3)

### mojo / flash-lane — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate15.swf&w=400&h=300&over=1: connected at +1 s, load done at +8 s, 5 content frames at +8 s; the GPU path, frames off th
    WAIT ok 7-rect toggle
    CHECK ok 9-flash-rate: content 40,114-440,412: 3.0 s, flips 42 (14.0/s), changes 41 (13.7/s), reads 43, distinct 42, spacing median 66 p90 111 max 159 ms (wanted 8..100/s)
    CHECK ok 11-flash-rate: content 40,114-440,412: 3.0 s, flips 43 (14.3/s), changes 0 (0.0/s), reads 44, distinct 1, spacing median 66 p90 86 max 156 ms (wanted 0..4/s)
    CHECK ok 13-flash-rate: content 40,114-440,412: 3.0 s, flips 43 (14.3/s), changes 43 (14.3/s), reads 44, distinct 44, spacing median 67 p90 86 max 154 ms (wanted 8..100/s)
    WAIT ok 15-settled: the card's scroll 0 44
    CHECK ok 16-flash-rate: content 40,26-440,324: 3.0 s, flips 43 (14.3/s), changes 43 (14.3/s), reads 44, distinct 44, spacing median 67 p90 88 max 152 ms (wanted 8..100/s)
    CHECK ok 18-flash-rate: content 40,26-440,324: 3.0 s, flips 42 (14.0/s), changes 0 (0.0/s), reads 43, distinct 1, spacing median 68 p90 85 max 170 ms (wanted 0..4/s)

### mojo / flash-sound — a person's

    hearing the speaker is a person's; the decode path is measured by script (cavekit-addons-extensions R7, the audio route)

### mojo / pan-long-drag — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    CHECK ok 8-zoom: zoom 1.0000 (want 1.0 within 0.01)
    CHECK ok 9-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 10-content: region 0,150-768,400: colours 293 hash 8b1a67cb rows 250 uniform 0 light 150-399 blue none; sel:#content p on the screen at 16,145-748,201, text holds [quick brown]
    CHECK ok 14-card-scroll: the card's scroll y 605 (want 650 within 100); the engine's 605
    WAIT ok 15-still header
    CHECK ok 16-region-equal: region 0,29-756,83: colours 185 hash eeaac7d6 rows 54 uniform 33 light none blue none = the sample's hash
    CHECK ok 17-region-changed: region 0,300-756,900: colours 767 hash 8fa49cd9 rows 600 uniform 218 light 300-895 blue none against the sample's hash 5ec8059e
    CHECK ok 18-top: the page's top measured 73 (the tap at y 203 landed on document y 780), the model says 73 (header hidden: 1)
    CHECK ok 20-hit: clickAt 383,848 inside sel:#content p:nth-child(12) at 16,820 732x56

### mojo / wikipedia-keyboard-zoom — not run

    needs internet (run with --with internet)

### mojo / zoom-new-page — not run

    needs internet (run with --with internet)

### mojo / flash-audio — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-audio-long.swf: connected at +1 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the ex
    WAIT ok 7-rect m
    CHECK ok 8-audio: the movie's audio: open running
    WAIT ok 9-launch: load done
    CHECK ok 10-audio: the movie's audio: closed

### mojo / flash-drawn — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +1 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 9-flash-drawn: drawn: region 40,114-440,412: colours 11 hash 899456dd rows 298 uniform 219 light none blue none
    WAIT ok 10-launch: load done
    WAIT ok 11-still m
    CHECK ok 12-flash-drawn: reported not drawn, as it should be: region 40,114-440,412: colours 1 hash aec36065 rows 298 uniform 298 light 114-411 blue none
    WAIT ok 13-launch: load done
    WAIT ok 14-still m
    CHECK ok 15-flash-drawn: reported not drawn, as it should be: region 40,114-440,412: colours 1 hash ed7df245 rows 298 uniform 298 light none blue none
    WAIT ok 16-launch: load done
    WAIT ok 17-still m
    CHECK ok 18-flash-drawn: reported not drawn, as it should be: region 40,114-440,412: colours 1 hash b955b145 rows 298 uniform 298 light none blue 114-411

### mojo / flash-plugin — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-rect m
    CHECK ok 8-plugin: plugin-container 6954: alive (state S, cpu ticks 93 to 100)
    CHECK ok 10-plugin: plugin-container 6954: wedged (state T, cpu ticks 100 to 100)
    CHECK ok 12-plugin: plugin-container 6954: alive (state S, cpu ticks 100 to 108)
    CHECK ok 14-plugin: plugin-container none: gone (state ?, cpu ticks ? to ?)

### mojo / flash-rate — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-rate10.swf: connected at +1 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    CHECK ok 9-flash-rate-report: content 40,114-440,412: 8.0 s, flips 78 (9.8/s), changes 78 (9.8/s), reads 79, distinct 61, spacing median 98 p90 159 max 195 ms; the movie's own rate 9.7156/s (frames 60 t=6933 .. frames 170 t=18255)
    WAIT ok 10-launch: load done
    WAIT ok 11-rect m
    CHECK ok 12-flash-rate-report: content 40,114-440,412: 8.0 s, flips 110 (13.8/s), changes 110 (13.8/s), reads 111, distinct 60, spacing median 69 p90 92 max 166 ms; the movie's own rate 14.3664/s (frames 50 t=3742 .. frames 200 t=14183)

### mojo / flash-reacted — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-frozen.swf: connected at +1 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 11-flash-reacted: reacted: no (content 40,114-440,412: 4.0 s, flips 19 (4.8/s), changes 1 (0.2/s), reads 25, distinct 2, spacing median 210 p90 222 max 225 ms new 0 of 25)
    WAIT ok 12-launch: load done
    WAIT ok 13-rect m
    CHECK ok 16-flash-reacted: reacted: no (content 40,114-440,412: 5.0 s, flips 137 (27.4/s), changes 137 (27.4/s), reads 138, distinct 40, spacing median 35 p90 41 max 115 ms new 0 of 138)
    WAIT ok 17-launch: load done
    WAIT ok 18-rect m
    WAIT ok 19-still m
    CHECK ok 22-flash-reacted: reacted: yes (content 40,114-440,412: 3.0 s, flips 15 (5.0/s), changes 0 (0.0/s), reads 21, distinct 1, spacing median 209 p90 217 max 219 ms new 21 of 21 first 0 ms)

### mojo / flash-signature — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-plant-reacts.swf: connected at +1 s, load done at +8 s, 4 content frames at +8 s; the GPU path, frames off the main thread (the 
    WAIT ok 7-rect m
    WAIT ok 8-still m
    CHECK ok 10-screen: the screen against its reference: difference 0.0 (same is 5 or less; wanted same)
    WAIT ok 12-still m
    CHECK ok 13-screen: the screen against its reference: difference 8.3 (same is 5 or less; wanted other)

### mojo / flash-stage-z1 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +1 s, load done at +8 s, 6 content frames at +8 s; the GPU path, frames off the main th
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,149.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 151.95,119.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,150.65 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 152,121.3 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,149.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 152,119.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 151,120 for stage 150,120 (within 3)

### mojo / flash-stage-z2 — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150: connected at +1 s, load done at +8 s, 9 content frames at +8 s; the GPU path, frames off the main th
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 9-zoom
    CHECK ok 10-zoom: zoom 1.9265 (want 1.92 within 0.3)
    CHECK ok 13-flash-at: the movie reported down 199.95,149.95 for stage 200,150 (within 3)
    CHECK ok 15-flash-at: the movie reported down 149.95,119.95 for stage 150,120 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 19-zoom
    CHECK ok 20-zoom: zoom 1.9377 (want 1.92 within 0.3)
    CHECK ok 23-flash-at: the movie reported down 199.95,150.65 for stage 200,150 (within 3)
    CHECK ok 25-flash-at: the movie reported down 150.65,119.95 for stage 150,120 (within 3)
    WAIT ok 26-launch: load done
    WAIT ok 27-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    WAIT ok 29-zoom
    CHECK ok 30-zoom: zoom 1.9400 (want 1.92 within 0.3)
    CHECK ok 33-flash-at: the movie reported down 199.95,149.95 for stage 200,150 (within 3)
    CHECK ok 35-flash-at: the movie reported down 150.65,119.95 for stage 150,120 (within 3)
    WAIT ok 36-launch: load done
    WAIT ok 37-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    WAIT ok 39-zoom
    CHECK ok 40-zoom: zoom 1.9326 (want 1.92 within 0.3)
    CHECK ok 43-flash-at: the movie reported down 200,149 for stage 200,150 (within 3)
    CHECK ok 45-flash-at: the movie reported down 150,119 for stage 150,120 (within 3)

### mojo / flash-stage-zfit — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2&scale=showAll&w=300&h=150&fit=1: connected at +1 s, load done at +8 s, 8 content frames at +8 s; the GPU path, frames off the m
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201.95,147.95 for stage 200,150 (within 3)
    CHECK ok 11-flash-at: the movie reported down 149.95,117.95 for stage 150,120 (within 3)
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201.3,149.3 for stage 200,150 (within 3)
    CHECK ok 17-flash-at: the movie reported down 150.65,118.65 for stage 150,120 (within 3)
    WAIT ok 18-launch: load done
    WAIT ok 19-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 21-flash-at: the movie reported down 201.3,147.95 for stage 200,150 (within 3)
    CHECK ok 23-flash-at: the movie reported down 150.65,117.95 for stage 150,120 (within 3)
    WAIT ok 24-launch: load done
    WAIT ok 25-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=300x150 ei=true]
    CHECK ok 27-flash-at: the movie reported down 201,149 for stage 200,150 (within 3)
    CHECK ok 29-flash-at: the movie reported down 150,119 for stage 150,120 (within 3)

