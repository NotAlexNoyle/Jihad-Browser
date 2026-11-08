# Finger suite report — 2026-11-08T10:52:13

First run 2026-11-08T10:14:24; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 1. 1645 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | youtube-feed-no-preview (rerun) | **pass** | 5 ok, 0 failed | CHECK ok 17-probe-has: the probe's answer holds [playing 0] | cavekit-device-headroom.md R4 |
| enyo | youtube-no-next | **pass** | 6 ok, 0 failed | CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw | cavekit-device-headroom.md R4 |
| mochi | youtube-no-next | **pass** | 6 ok, 0 failed | CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw&pp=0gcJCVgCo7VqN5tD | cavekit-device-headroom.md R4 |
| mojo | youtube-no-next | **pass** | 6 ok, 0 failed | CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw | cavekit-device-headroom.md R4 |

## The device at the end

```
unable to find device
```

## Every test's lines

### enyo / youtube-feed-no-preview — pass

    enyo card on https://m.youtube.com/: connected at +2 s, load done at +22 s, 3 content frames at +31 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_GPU_COMPOSITOR)
    CHECK ok 10-quiet: no line play() allowed for 30 s
    CHECK ok 12-probe-has: the probe's answer holds [playing 0]
    WAIT ok 14-settled: the card's scroll 0 483
    CHECK ok 15-quiet: no line play() allowed for 10 s
    CHECK ok 17-probe-has: the probe's answer holds [playing 0]

### enyo / youtube-no-next — pass

    enyo card on https://m.youtube.com/watch?v=jNQXAC9IVRw: connected at +2 s, load done at +25 s, 5 content frames at +25 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 9-rect sel:video
    WAIT ok 11-line: [jihad-bs] js-console (no source):0:0 compat: youtube [m.youtube.com] play() allowed: 1828 ms after a click
    CHECK ok 13-probe-has: the probe's answer holds ["paused":false]
    CHECK ok 14-quiet: no line play() allowed for 70 s
    CHECK ok 16-probe-has: the probe's answer holds ["paused":true]
    CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw

### mochi / youtube-no-next — pass

    mochi card on https://m.youtube.com/watch?v=jNQXAC9IVRw: connected at +3 s, load done at +27 s, 20 content frames at +27 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD
    WAIT ok 9-rect sel:video
    WAIT ok 11-line: [jihad-bs] js-console (no source):0:0 compat: youtube [m.youtube.com] play() allowed: 1990 ms after a click
    CHECK ok 13-probe-has: the probe's answer holds ["paused":false]
    CHECK ok 14-quiet: no line play() allowed for 70 s
    CHECK ok 16-probe-has: the probe's answer holds ["paused":true]
    CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw&pp=0gcJCVgCo7VqN5tD

### mojo / youtube-no-next — pass

    mojo card on https://m.youtube.com/watch?v=jNQXAC9IVRw: connected at +1 s, load done at +24 s, 21 content frames at +24 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 9-rect sel:video
    WAIT ok 11-line: [jihad-bs] js-console (no source):0:0 compat: youtube [m.youtube.com] play() allowed: 1710 ms after a click
    CHECK ok 13-probe-has: the probe's answer holds ["paused":false]
    CHECK ok 14-quiet: no line play() allowed for 70 s
    CHECK ok 16-probe-has: the probe's answer holds ["paused":true]
    CHECK ok 17-url: load done uri=https://m.youtube.com/watch?v=jNQXAC9IVRw

