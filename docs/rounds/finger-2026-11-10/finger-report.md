# Finger suite report — 2026-11-10T14:15:55

Shells: mojo. Scripts: 1. 68 s.

| shell | test | verdict | checks | evidence | kit |
|---|---|---|---|---|---|
| mojo | download-list | **fail** | 0 ok, 0 failed | the session did not reach its end (rc 255) | cavekit-device-build.md R4 |

## The device at the end

```
unable to find device
```

## Every test's lines

### mojo / download-list — fail

    mojo card on http://192.168.1.245:8077/jihad-long-static.html: connected at +3 s, load done at +12 s, 3 content frames at +12 s; the GPU path, frames off the main thread (the express lane) (the job's 
    the session did not reach its end (rc 255)

