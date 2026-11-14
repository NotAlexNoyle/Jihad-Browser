# Finger suite report — 2026-11-14T12:21:58

Shells: enyo, mochi, mojo. Scripts: 3. 704 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| enyo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| enyo | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 50 (16.7/s), changes 34 (11.3/s), reads 51, distinct 35, spacing median 61 p90 82 max 165 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mochi | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mochi | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mochi | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 47 (15.7/s), changes 42 (14.0/s), reads 48, distinct 43, spacing median 68 p90 83 max 112 ms (wanted 5..100/s) | cavekit-flash.md R4 |
| mojo | flash-api | **pass** | 14 ok, 0 failed | CHECK ok 22-text: count = [0] | cavekit-flash.md R4 |
| mojo | flash-input-tap | **pass** | 12 ok, 0 failed | CHECK ok 22-number: n-up = 1 (wanted 1..1) | cavekit-flash.md R5 |
| mojo | flash-tags | **pass** | 15 ok, 0 failed | CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 44 (14.7/s), changes 44 (14.7/s), reads 45, distinct 45, spacing median 69 p90 82 max 94 ms (wanted 5..100/s) | cavekit-flash.md R4 |

## The device at the end

```
DEVICE ok job-enyo: (201624/693443312) jihad (start) running, process 31573
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (201634/935562718) jihad-mochi (start) running, process 11254
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (201647/164837235) jihad-mojo (start) running, process 22794
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
DEVICE FAIL tmp-dumps: /tmp/au-a.png /tmp/au-b.png /tmp/au-c.png /tmp/au-d.png /tmp/mk-a.png /tmp/mk-b.png /tmp/mk-c.png /tmp/mk-d.png /tmp/mk-e.png /tmp/mk-f.png /tmp/mk-g.png /tmp/r3-a.png /tmp/r3-b.png /tmp/r3-c.png /tmp/rw.png /tmp/sb-probe.png /tmp/sb-probe2.png /tmp/wb.png 
DEVICE ok media-scratch: none
DEVICE FAIL overall: 21% of /tmp used
novacom: unexpected EOF from server
```

## Every test's lines

### enyo / flash-api — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, f
    WAIT ok 7-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 8-text vars = [vars a=hello]
    WAIT ok 9-text so = [so n=1]
    WAIT ok 10-text echoback = [echoback back hi]
    CHECK ok 11-text: echo = [echo hi]
    WAIT ok 12-text geturl = [geturl ok]
    WAIT ok 13-text fscommand = [fscommand jcmd jarg]
    WAIT ok 14-text data = [data true x=42]
    WAIT ok 15-text kid = [kid w=376]
    WAIT ok 17-text so = [so n=2]
    WAIT ok 18-launch: load done
    WAIT ok 19-rect m
    CHECK ok 20-text: count = [0]
    CHECK ok 22-text: count = [0]

### enyo / flash-input-tap — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 8 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-number: n-down = 1 (wanted 1..1)
    CHECK ok 11-number: n-up = 1 (wanted 1..1)
    CHECK ok 14-flash-at: the movie reported down 101,60 for stage 100,60 (within 3)
    CHECK ok 15-number: n-down = 2 (wanted 2..2)
    CHECK ok 16-number: n-up = 2 (wanted 2..2)
    WAIT ok 17-launch: load done
    WAIT ok 18-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 21-number: n-down = 1 (wanted 1..1)
    CHECK ok 22-number: n-up = 1 (wanted 1..1)

### enyo / flash-tags — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 11 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [10.3]
    CHECK ok 11-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [mime yes]
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 23-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 24-launch: load done
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 50 (16.7/s), changes 34 (11.3/s), reads 51, distinct 35, spacing median 61 p90 82 max 165 ms (wanted 5..100/s)

### mochi / flash-api — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +1 s, load done at +7 s, 9 content frames at +7 s; the GPU path, 
    WAIT ok 7-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 8-text vars = [vars a=hello]
    WAIT ok 9-text so = [so n=1]
    WAIT ok 10-text echoback = [echoback back hi]
    CHECK ok 11-text: echo = [echo hi]
    WAIT ok 12-text geturl = [geturl ok]
    WAIT ok 13-text fscommand = [fscommand jcmd jarg]
    WAIT ok 14-text data = [data true x=42]
    WAIT ok 15-text kid = [kid w=376]
    WAIT ok 17-text so = [so n=2]
    WAIT ok 18-launch: load done
    WAIT ok 19-rect m
    CHECK ok 20-text: count = [0]
    CHECK ok 22-text: count = [0]

### mochi / flash-input-tap — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-number: n-down = 1 (wanted 1..1)
    CHECK ok 11-number: n-up = 1 (wanted 1..1)
    CHECK ok 14-flash-at: the movie reported down 101,60 for stage 100,60 (within 3)
    CHECK ok 15-number: n-down = 2 (wanted 2..2)
    CHECK ok 16-number: n-up = 2 (wanted 2..2)
    WAIT ok 17-launch: load done
    WAIT ok 18-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 21-number: n-down = 1 (wanted 1..1)
    CHECK ok 22-number: n-up = 1 (wanted 1..1)

### mochi / flash-tags — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [10.3]
    CHECK ok 11-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [mime yes]
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 23-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 24-launch: load done
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 47 (15.7/s), changes 42 (14.0/s), reads 48, distinct 43, spacing median 68 p90 83 max 112 ms (wanted 5..100/s)

### mojo / flash-api — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?swf=jihad-instr-features.swf&api=1&vars=a=hello%26reset=1: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, f
    WAIT ok 7-text ready = [ready features ver=WEB 10,3,185,65 stage=400x300]
    WAIT ok 8-text vars = [vars a=hello]
    WAIT ok 9-text so = [so n=1]
    WAIT ok 10-text echoback = [echoback back hi]
    CHECK ok 11-text: echo = [echo hi]
    WAIT ok 12-text geturl = [geturl ok]
    WAIT ok 13-text fscommand = [fscommand jcmd jarg]
    WAIT ok 14-text data = [data true x=42]
    WAIT ok 15-text kid = [kid w=376]
    WAIT ok 17-text so = [so n=2]
    WAIT ok 18-launch: load done
    WAIT ok 19-rect m
    CHECK ok 20-text: count = [0]
    CHECK ok 22-text: count = [0]

### mojo / flash-input-tap — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 10 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-number: n-down = 1 (wanted 1..1)
    CHECK ok 11-number: n-up = 1 (wanted 1..1)
    CHECK ok 14-flash-at: the movie reported down 101,60 for stage 100,60 (within 3)
    CHECK ok 15-number: n-down = 2 (wanted 2..2)
    CHECK ok 16-number: n-up = 2 (wanted 2..2)
    WAIT ok 17-launch: load done
    WAIT ok 18-text ready = [ready avm1 ver=WEB 10,3,185,65 stage=400x300]
    CHECK ok 20-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 21-number: n-down = 1 (wanted 1..1)
    CHECK ok 22-number: n-up = 1 (wanted 1..1)

### mojo / flash-tags — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-instr.html?movie=avm2: connected at +1 s, load done at +6 s, 7 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (t
    WAIT ok 7-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 9-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    CHECK ok 10-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [10.3]
    CHECK ok 11-text-has: detect = [detect plugin Shockwave Flash 10.3 d185 mime yes] holds [mime yes]
    WAIT ok 12-launch: load done
    WAIT ok 13-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 15-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 16-launch: load done
    WAIT ok 17-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 19-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 20-launch: load done
    WAIT ok 21-text ready = [ready avm2 ver=WEB 10,3,185,65 stage=400x300 ei=true]
    CHECK ok 23-flash-at: the movie reported down 201,150 for stage 200,150 (within 3)
    WAIT ok 24-launch: load done
    CHECK ok 25-flash-rate: content 0,100-756,1000: 3.0 s, flips 44 (14.7/s), changes 44 (14.7/s), reads 45, distinct 45, spacing median 69 p90 82 max 94 ms (wanted 5..100/s)

