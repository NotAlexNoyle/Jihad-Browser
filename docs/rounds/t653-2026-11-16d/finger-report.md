# Finger suite report — 2026-11-16T08:30:35

Shells: enyo, mochi, mojo. Scripts: 9. 1736 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| enyo | flash-sound | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| enyo | frame-field-hold | **fail** | 13 ok, 1 failed | CHECK FAIL 35-wait-line: no line hold: editable=true within the bound; frames: kept/enyo-frame-field-hold-field-menu_.jpg, kept/enyo-frame-field-hold-35-wait-line_.jpg | cavekit-ui-shell.md R7 |
| enyo | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| enyo | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| enyo | rotate | **pass** | 10 ok, 0 failed | CHECK ok 28-card-scroll-min: the card's scroll y 640 (at least 400); frames: kept/enyo-rotate-landscape-left_.jpg, kept/enyo-rotate-landscape-right_.jpg | cavekit-device-build.md R4 |
| enyo | volume-keys | **fail** | 9 ok, 1 failed | CHECK FAIL 28-volume: the media volume 22, wanted higher (marked 66); frames: kept/enyo-volume-keys-28-volume_.jpg | cavekit-finger-testing.md R7 |
| mochi | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| mochi | flash-sound | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2955 rms 2056 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| mochi | frame-field-hold | **fail** | 13 ok, 1 failed | CHECK FAIL 35-wait-line: no line hold: editable=true within the bound; frames: kept/mochi-frame-field-hold-field-menu_.jpg, kept/mochi-frame-field-hold-35-wait-line_.jpg | cavekit-ui-shell.md R7 |
| mochi | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mochi | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| mochi | rotate | **pass** | 10 ok, 0 failed | CHECK ok 28-card-scroll-min: the card's scroll y 650 (at least 400); frames: kept/mochi-rotate-landscape-left_.jpg, kept/mochi-rotate-landscape-right_.jpg | cavekit-device-build.md R4 |
| mochi | volume-keys | **fail** | 9 ok, 1 failed | CHECK FAIL 28-volume: the media volume 22, wanted higher (marked 66); frames: kept/mochi-volume-keys-28-volume_.jpg | cavekit-finger-testing.md R7 |
| mojo | audio-heard | **pass** | 6 ok, 0 failed | CHECK ok 14-sound: sound at the output sink: peak 2955 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-device-build.md R4 |
| mojo | flash-sound | **pass** | 8 ok, 0 failed | CHECK ok 16-sound: sound at the output sink: peak 2952 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz | cavekit-engine-update.md R8 |
| mojo | frame-field-hold | **fail** | 13 ok, 1 failed | CHECK FAIL 35-wait-line: no line hold: editable=true within the bound; frames: kept/mojo-frame-field-hold-field-menu_.jpg, kept/mojo-frame-field-hold-35-wait-line_.jpg | cavekit-ui-shell.md R7 |
| mojo | frame-field | **pass** | 10 ok, 0 failed | CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused) | cavekit-chrome-look.md R4 |
| mojo | long-press-menu | **pass** | 4 ok, 0 failed | CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html | cavekit-ui-shell.md R7 |
| mojo | mojo-address-typing | **pass** | 4 ok, 0 failed | CHECK ok 16-url: load done uri=https://lite.duckduckgo.com/lite/?q=webos | cavekit-mojo-ui.md R6 |
| mojo | mojo-launch-from-app | **pass** | 4 ok, 0 failed | CHECK ok 13-content: region 0,150-756,600: colours 373 hash f5159163 rows 450 uniform 164 light 150-593 blue none; sel:body on the screen at 0,73-764,20561 | cavekit-chrome-look.md R4 |
| mojo | rotate | **pass** | 10 ok, 0 failed | CHECK ok 28-card-scroll-min: the card's scroll y 573 (at least 400); frames: kept/mojo-rotate-landscape-left_.jpg, kept/mojo-rotate-landscape-right_.jpg | cavekit-device-build.md R4 |
| mojo | volume-keys | **fail** | 9 ok, 1 failed | CHECK FAIL 28-volume: the media volume 22, wanted higher (marked 66); frames: kept/mojo-volume-keys-28-volume_.jpg | cavekit-finger-testing.md R7 |

## The device at the end

```
DEVICE ok job-enyo: (361586/166284328) jihad (start) running, process 13533
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (361590/845864770) jihad-mochi (start) running, process 8363
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (361596/828472177) jihad-mojo (start) running, process 10009
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
DEVICE ok overall: 16% of /tmp used
```

## Every test's lines

### enyo / audio-heard — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +1 s, load done at +7 s, 13 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31139 rms 21984 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### enyo / flash-sound — pass

    enyo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +7 s, 14 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31139 rms 21984 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### enyo / frame-field-hold — fail

    enyo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +2 s, load done at +6 s, 10 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect f
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 151 landed on document y 88)
    WAIT ok 12-line: [jihad-bs] hold at 77,336 editable=0 wasFocused=0
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=false readOnly=false selected=true
    WAIT ok 15-still f
    WAIT ok 18-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=0
    WAIT ok 19-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false
    WAIT ok 22-line: [jihad-bs] notify: page-hold: {"editable":true,"readOnly":false,"selected":false} (pushed)
    WAIT ok 23-differs f
    CHECK ok 24-region-changed: region 13,323-756,528: colours 299 hash 49e48b9b rows 205 uniform 0 light 324-526 blue none against the sample's hash 261e5084
    CHECK ok 25-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 30-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 32-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 34-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=1
    CHECK FAIL 35-wait-line: no line hold: editable=true within the bound

### enyo / frame-field — pass

    enyo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +3 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect f
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 151 landed on document y 88)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    CHECK ok 11-visible: f on the screen at 13,322-759,529 (the reach ends at 600, keyboard 1)
    WAIT ok 14-differs f
    CHECK ok 15-region-changed: region 13,323-756,528: colours 203 hash 9f55f686 rows 205 uniform 0 light 324-526 blue none against the sample's hash 2a543c72
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)
    WAIT ok 18-still f
    WAIT ok 20-still f
    CHECK ok 21-region-equal: region 13,323-756,528: colours 202 hash af7f74a8 rows 205 uniform 0 light 324-526 blue none = the sample's hash
    CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser, or the daemon's editorFocused)

### enyo / long-press-menu — pass

    enyo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 7 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 113 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### enyo / rotate — pass

    enyo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 4 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (enyo): the page's top on the screen 82 (the table said 82, the tap at y 182 landed on document y 100)
    WAIT ok 13-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x686 mZoom=1.3333
    WAIT ok 14-content 0,150,756,900
    WAIT ok 17-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x686 mZoom=0.7500
    WAIT ok 18-content 0,150,756,900
    WAIT ok 20-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x686 mZoom=1.3333
    WAIT ok 21-content 0,150,756,900
    WAIT ok 24-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x686 mZoom=0.7500
    WAIT ok 25-settled: the card's scroll 0 0
    CHECK ok 26-content: region 0,150-756,600: colours 421 hash 727e4039 rows 450 uniform 173 light 150-599 blue none; sel:body on the screen at 0,82-764,20570
    CHECK ok 28-card-scroll-min: the card's scroll y 640 (at least 400)

### enyo / volume-keys — fail

    enyo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +2 s, load done at +8 s, 21 content frames at +8 s; the GPU path, frames off the main thread
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2975 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume: the media volume 66, higher (marked 44)
    CHECK ok 17-sound: sound at the output sink: peak 2964 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 20-volume: the media volume 44, lower (marked 66)
    CHECK ok 21-sound: sound at the output sink: peak 2953 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 25-volume: the media volume 0, zero (marked 66)
    CHECK ok 26-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2958 rms 2060 frames 45056 hz 440): no sound out
    CHECK FAIL 28-volume: the media volume 22, wanted higher (marked 66)
    CHECK ok 29-sound: sound at the output sink: peak 2954 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### mochi / audio-heard — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +2 s, load done at +8 s, 7 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31144 rms 21986 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2956 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mochi / flash-sound — pass

    mochi card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +1 s, load done at +7 s, 18 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31139 rms 21985 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2955 rms 2056 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mochi / frame-field-hold — fail

    mochi card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's J
    WAIT ok 9-rect f
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 147 landed on document y 88)
    WAIT ok 12-line: [jihad-bs] hold at 77,336 editable=0 wasFocused=0
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=false readOnly=false selected=true
    WAIT ok 15-still f
    WAIT ok 18-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=0
    WAIT ok 19-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false
    WAIT ok 22-line: [jihad-bs] notify: page-hold: {"editable":true,"readOnly":false,"selected":false} (pushed)
    WAIT ok 23-differs f
    CHECK ok 24-region-changed: region 13,319-756,524: colours 218 hash c90db968 rows 205 uniform 0 light none blue none against the sample's hash 22936c02
    CHECK ok 25-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 30-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 32-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 34-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=1
    CHECK FAIL 35-wait-line: no line hold: editable=true within the bound

### mochi / frame-field — pass

    mochi card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +6 s, 10 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's 
    WAIT ok 7-rect f
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 147 landed on document y 88)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    CHECK ok 11-visible: f on the screen at 13,318-759,525 (the reach ends at 600, keyboard 1)
    WAIT ok 14-differs f
    CHECK ok 15-region-changed: region 13,319-756,524: colours 229 hash 4e051f6c rows 205 uniform 0 light 320-522 blue none against the sample's hash 47ef556c
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)
    WAIT ok 18-still f
    WAIT ok 20-still f
    CHECK ok 21-region-equal: region 13,319-756,524: colours 228 hash ea9cee95 rows 205 uniform 0 light 320-522 blue none = the sample's hash
    CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mochi, or the daemon's editorFocused)

### mochi / long-press-menu — pass

    mochi card on http://192.168.1.245:8077/jihad-card-events.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 109 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mochi / rotate — pass

    mochi card on http://192.168.1.245:8077/jihad-long-static.html: connected at +2 s, load done at +7 s, 5 content frames at +8 s; the GPU path, frames off the main thread (the express lane) (the job's J
    calibrated (mochi): the page's top on the screen 78 (the table said 78, the tap at y 178 landed on document y 100)
    WAIT ok 13-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x690 mZoom=1.3333
    WAIT ok 14-content 0,150,756,900
    WAIT ok 17-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x690 mZoom=1.0000
    WAIT ok 18-content 0,150,756,900
    WAIT ok 20-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x946 mZoom=1.0000
    WAIT ok 21-content 0,150,756,900
    WAIT ok 24-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x946 mZoom=0.7500
    WAIT ok 25-settled: the card's scroll 0 0
    CHECK ok 26-content: region 0,150-756,600: colours 361 hash 20376874 rows 450 uniform 169 light 150-598 blue none; sel:body on the screen at 0,78-764,20566
    CHECK ok 28-card-scroll-min: the card's scroll y 650 (at least 400)

### mochi / volume-keys — fail

    mochi card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +1 s, load done at +7 s, 18 content frames at +7 s; the GPU path, frames off the main threa
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2975 rms 2058 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume: the media volume 66, higher (marked 44)
    CHECK ok 17-sound: sound at the output sink: peak 2964 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 20-volume: the media volume 44, lower (marked 66)
    CHECK ok 21-sound: sound at the output sink: peak 2953 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 25-volume: the media volume 0, zero (marked 66)
    CHECK ok 26-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2958 rms 2060 frames 45056 hz 440): no sound out
    CHECK FAIL 28-volume: the media volume 22, wanted higher (marked 66)
    CHECK ok 29-sound: sound at the output sink: peak 2954 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

### mojo / audio-heard — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +1 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31138 rms 21983 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    WAIT ok 13-text status = [playing]
    CHECK ok 14-sound: sound at the output sink: peak 2955 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mojo / flash-sound — pass

    mojo card on http://192.168.1.245:8077/jihad-flash-tone.html: connected at +1 s, load done at +6 s, 4 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIH
    WAIT ok 9-rect sel:embed
    CHECK ok 10-audio: the movie's audio: open running audible
    CHECK ok 11-sound: sound at the output sink: peak 31144 rms 21986 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz
    WAIT ok 12-launch: load done
    CHECK ok 13-sound: no sound at the output sink: peak 0 rms 0 frames 45056 hz 0
    WAIT ok 14-launch: load done
    WAIT ok 15-text status = [playing]
    CHECK ok 16-sound: sound at the output sink: peak 2952 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 23, the pitch wanted 440 Hz

### mojo / frame-field-hold — fail

    mojo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +5 s, 5 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 9-rect f
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 142 landed on document y 88)
    WAIT ok 12-line: [jihad-bs] hold at 77,336 editable=0 wasFocused=0
    WAIT ok 13-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=false readOnly=false selected=true
    WAIT ok 15-still f
    WAIT ok 18-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=0
    WAIT ok 19-line: [jihad-bs] js-console (no source):0:0 page: hold: editable=true readOnly=false selected=false
    WAIT ok 22-line: [jihad-bs] notify: page-hold: {"editable":true,"readOnly":false,"selected":false} (pushed)
    WAIT ok 23-differs f
    CHECK ok 24-region-changed: region 13,314-756,519: colours 609 hash 4f7ab6f1 rows 205 uniform 0 light 326-444 blue none against the sample's hash ce1bdacc
    CHECK ok 25-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 30-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 32-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 34-line: [jihad-bs] hold at 117,396 editable=1 wasFocused=1
    CHECK FAIL 35-wait-line: no line hold: editable=true within the bound

### mojo / frame-field — pass

    mojo card on http://192.168.1.245:8077/jihad-frame-field.html: connected at +1 s, load done at +5 s, 7 content frames at +5 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    WAIT ok 7-rect f
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 142 landed on document y 88)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 11-visible: f on the screen at 13,313-759,520 (the reach ends at 600, keyboard 1)
    WAIT ok 14-differs f
    CHECK ok 15-region-changed: region 13,314-756,519: colours 203 hash 3245e2d2 rows 205 uniform 0 light 315-517 blue none against the sample's hash 14b1cd9a
    CHECK ok 17-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 18-still f
    WAIT ok 20-still f
    CHECK ok 21-region-equal: region 13,314-756,519: colours 202 hash ba939f24 rows 205 uniform 0 light 315-517 blue none = the sample's hash
    CHECK ok 23-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)

### mojo / long-press-menu — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +5 s, 3 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    WAIT ok 10-differs 0,300,756,900
    WAIT ok 12-still 0,300,756,900
    WAIT ok 14-load
    CHECK ok 15-url: load done uri=http://192.168.1.245:8077/jihad-long-static.html

### mojo / mojo-address-typing — pass

    mojo card on http://192.168.1.245:8077/jihad-card-events.html: connected at +1 s, load done at +6 s, 6 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 104 landed on document y 31)
    CHECK ok 10-keyboard: the keyboard is up (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    CHECK ok 14-nav-paint: the new page's first paint 5 ms after the load started (at most 6000)
    WAIT ok 15-load
    CHECK ok 16-url: load done uri=https://lite.duckduckgo.com/lite/?q=webos

### mojo / mojo-launch-from-app — pass

    mojo card on about:blank: connected at +1 s, load done at +4 s, 0 content frames at +34 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    WAIT ok 10-url: http://192.168.1.245:8077/jihad-long-static.html
    CHECK ok 11-keyboard: the keyboard is down (the window server's last IME line for net.riverstonerelay.jihad-browser-mojo, or the daemon's editorFocused)
    WAIT ok 12-content 0,150,756,600
    CHECK ok 13-content: region 0,150-756,600: colours 373 hash f5159163 rows 450 uniform 164 light 150-593 blue none; sel:body on the screen at 0,73-764,20561

### mojo / rotate — pass

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JI
    calibrated (mojo): the page's top on the screen 73 (the table said 73, the tap at y 173 landed on document y 100)
    WAIT ok 13-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x740 mZoom=1.3333
    WAIT ok 14-content 0,150,756,900
    WAIT ok 17-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x996 mZoom=1.0000
    WAIT ok 18-content 0,150,756,900
    WAIT ok 20-line: [jihad-bs] emitGeometry contentSize=1024x20488 win=1024x740 mZoom=1.3333
    WAIT ok 21-content 0,150,756,900
    WAIT ok 24-line: [jihad-bs] emitGeometry contentSize=768x20488 win=768x996 mZoom=0.7500
    WAIT ok 25-settled: the card's scroll 0 0
    CHECK ok 26-content: region 0,150-756,600: colours 373 hash f5159163 rows 450 uniform 164 light 150-593 blue none; sel:body on the screen at 0,73-764,20561
    CHECK ok 28-card-scroll-min: the card's scroll y 573 (at least 400)

### mojo / volume-keys — fail

    mojo card on http://192.168.1.245:8077/jihad-media-test.html?src=clip60av.webm&wide=1&sound=1: connected at +1 s, load done at +6 s, 9 content frames at +6 s; the GPU path, frames off the main thread 
    WAIT ok 9-text status = [playing]
    CHECK ok 13-sound: sound at the output sink: peak 2955 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 16-volume: the media volume 66, higher (marked 44)
    CHECK ok 17-sound: sound at the output sink: peak 2959 rms 2057 frames 45056 hz 440, the speaker amp on, the media volume 66, the pitch wanted 440 Hz
    CHECK ok 20-volume: the media volume 44, lower (marked 66)
    CHECK ok 21-sound: sound at the output sink: peak 2956 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 44, the pitch wanted 440 Hz
    CHECK ok 25-volume: the media volume 0, zero (marked 66)
    CHECK ok 26-sound: the route muted (the speaker amp on, the media volume 0) with the signal still at the sink (peak 2959 rms 2058 frames 45056 hz 440): no sound out
    CHECK FAIL 28-volume: the media volume 22, wanted higher (marked 66)
    CHECK ok 29-sound: sound at the output sink: peak 2955 rms 2059 frames 45056 hz 440, the speaker amp on, the media volume 22, the pitch wanted 440 Hz

