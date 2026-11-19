# Finger suite report — 2026-11-19T11:38:19

First run 2026-11-19T09:20:28; runs marked (rerun) were run again by `--merge`, and the device-check below is the last run's.

Shells: enyo, mochi, mojo. Scripts: 1. 5809 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| enyo | memory-guard-task (rerun) | **pass** | 9 ok, 0 failed | WAIT ok 23-launch: load done | cavekit-device-headroom.md R1 |
| enyo | eagler-guard-188 (rerun) | **pass** | 5 ok, 0 failed | WAIT ok 21-launch: load done | cavekit-eaglercraft.md R6 |
| mochi | memory-guard-task (rerun) | **pass** | 9 ok, 0 failed | WAIT ok 23-launch: load done | cavekit-device-headroom.md R1 |
| mojo | memory-guard-task (rerun) | **pass** | 9 ok, 0 failed | WAIT ok 23-launch: load done | cavekit-device-headroom.md R1 |

## The device at the end

```
DEVICE ok job-enyo: (10045/707790058) jihad (start) running, process 14688
DEVICE ok daemons-enyo: one daemon
DEVICE ok hand-daemon-enyo: none
DEVICE ok hooks-enyo: none
DEVICE ok adapter-gates-enyo: none
DEVICE ok proxy-enyo: network.proxy.type unset
DEVICE ok history-enyo: browser.sessionhistory.max_entries the default
DEVICE ok cards-enyo: none
DEVICE ok audio-lib-enyo: the device's own
DEVICE ok look-enyo: {"returnValue":true,"theme":null}
DEVICE ok job-mochi: (10059/626039241) jihad-mochi (start) running, process 18673
DEVICE ok daemons-mochi: one daemon
DEVICE ok hand-daemon-mochi: none
DEVICE ok hooks-mochi: none
DEVICE ok adapter-gates-mochi: none
DEVICE ok proxy-mochi: network.proxy.type unset
DEVICE ok history-mochi: browser.sessionhistory.max_entries the default
DEVICE ok cards-mochi: none
DEVICE ok audio-lib-mochi: the device's own
DEVICE ok look-mochi: {"returnValue":true,"theme":null}
DEVICE ok job-mojo: (10063/773032573) jihad-mojo (start) running, process 22801
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

### enyo / memory-guard-task — pass

    enyo card on http://192.168.1.245:8077/jihad-colors.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 15-launch: load done
    WAIT ok 16-line: [jihad-bs] memory guard (thread): this daemon 474208 kB, its players 0 kB (budget 470184), 22628 kB free (floor 65536): stopping the running content script, the page to be stopped
    WAIT ok 17-line: [jihad-bs] memory guard: the running content script stopped
    WAIT ok 18-line: [jihad-bs] memory shed: a page stopped
    WAIT ok 19-url: about:stopped?url=http%3A%2F%2F192.168.1.245%3A8077%2Fjihad-memhog.html%3Fmb%3D750%26rate%3D12%26task%3D1
    WAIT ok 20-rect sel:#link
    CHECK ok 21-text-has: sel:#where = [http://192.168.1.245:8077/jihad-memhog.html?mb=750&rate=12&task=1] holds [jihad-memhog.html]
    CHECK ok 22-gpu-calm: no new GPU fault in the kernel's log (0 lines, 0 at the start)
    WAIT ok 23-launch: load done

### enyo / eagler-guard-188 — pass

    enyo card on http://192.168.1.245:8099/eaglerx-1.8.8/unpacked/eaglercraft1-8.html: connected at +2 s, load done at +29 s, 41 content frames at +29 s; the GPU path, frames off the main thread (the expr
    WAIT ok 16-rect-ok sel:._eaglercraftX_mobile_launch_client, #link
    WAIT ok 18-dialogs-ok
    WAIT ok 19-url: about:stopped?url=http%3A%2F%2F192.168.1.245%3A8099%2Feaglerx-1.8.8%2Funpacked%2Feaglercraft1-8.html
    CHECK ok 20-gpu-calm: no new GPU fault in the kernel's log (0 lines, 0 at the start)
    WAIT ok 21-launch: load done

### mochi / memory-guard-task — pass

    mochi card on http://192.168.1.245:8077/jihad-colors.html: connected at +2 s, load done at +7 s, 5 content frames at +7 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_
    WAIT ok 15-launch: load done
    WAIT ok 16-line: [jihad-bs] memory guard (thread): this daemon 475304 kB, its players 0 kB (budget 470184), 21868 kB free (floor 65536): stopping the running content script, the page to be stopped
    WAIT ok 17-line: [jihad-bs] memory guard: the running content script stopped
    WAIT ok 18-line: [jihad-bs] memory shed: a page stopped
    WAIT ok 19-url: about:stopped?url=http%3A%2F%2F192.168.1.245%3A8077%2Fjihad-memhog.html%3Fmb%3D750%26rate%3D12%26task%3D1
    WAIT ok 20-rect sel:#link
    CHECK ok 21-text-has: sel:#where = [http://192.168.1.245:8077/jihad-memhog.html?mb=750&rate=12&task=1] holds [jihad-memhog.html]
    CHECK ok 22-gpu-calm: no new GPU fault in the kernel's log (0 lines, 0 at the start)
    WAIT ok 23-launch: load done

### mojo / memory-guard-task — pass

    mojo card on http://192.168.1.245:8077/jihad-colors.html: connected at +1 s, load done at +6 s, 5 content frames at +6 s; the GPU path, frames off the main thread (the express lane) (the job's JIHAD_G
    WAIT ok 15-launch: load done
    WAIT ok 16-line: [jihad-bs] memory guard (thread): this daemon 489160 kB, its players 0 kB (budget 470184), 29196 kB free (floor 65536): stopping the running content script, the page to be stopped
    WAIT ok 17-line: [jihad-bs] memory guard: the running content script stopped
    WAIT ok 18-line: [jihad-bs] memory shed: a page stopped
    WAIT ok 19-url: about:stopped?url=http%3A%2F%2F192.168.1.245%3A8077%2Fjihad-memhog.html%3Fmb%3D750%26rate%3D12%26task%3D1
    WAIT ok 20-rect sel:#link
    CHECK ok 21-text-has: sel:#where = [http://192.168.1.245:8077/jihad-memhog.html?mb=750&rate=12&task=1] holds [jihad-memhog.html]
    CHECK ok 22-gpu-calm: no new GPU fault in the kernel's log (0 lines, 0 at the start)
    WAIT ok 23-launch: load done

