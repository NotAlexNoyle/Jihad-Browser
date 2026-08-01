---
created: "2026-08-01"
last_edited: "2026-08-01"
---

# App JS changes do not take effect on this device by ANY method tried

**Severity: P1.** This blocks iterating on `app/` at all, and it silently invalidated part of the
card→adapter investigation, which was reasoning about code the device was not running.

## The anomaly, stated precisely

`app/source/JihadEngineOverride.js` contains two **adjacent statements** inside the same block:

```js
log("WebView engine -> application/x-jihad-browser");
if (JIHAD_PROBE) { log("probe-canary: JIHAD_PROBE=" + JIHAD_PROBE); probeInstall(proto); }
```

Both call the **same** `log()` function. On every launch, `palm-log` shows the first and **never**
the second. The canary is deliberately short and uses the proven-working emitter, so this is not a
message-shape or transport problem.

The only consistent explanation is that the card is executing a **stale copy** of the file —
one that predates the probe block entirely.

## What was verified, so this is not re-guessed

- The on-device file **does** contain the canary and the probe: `grep -c probe-canary` = 1,
  `grep -c JihadProbe` = 4, at `/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/source/JihadEngineOverride.js`.
- Its md5 matched the local file exactly after install.
- **Only one** file in the app mentions the MIME or `_jihadPatched` — there is no second patcher
  winning a race and setting the guard first (that was the leading alternative hypothesis).
- `depends.js` lists it **first**, exactly once. `index.html` pulls in only the Enyo framework.
- No rootfs copy of the app exists (`/usr/palm/applications/...` absent), so nothing shadows it.
- No `extractfs` entry for the app exists.

## Every method tried to make the update take effect — all failed

| Method | Result |
|---|---|
| `ipkg -o /media/cryptofs/apps install` (the harness path) | stale |
| Version bump 1.0.0 → 1.0.1 → 1.0.2 (webOS's documented lever for forcing a re-register) | stale |
| `killall LunaSysMgr` (respawns WebAppMgr too), ×5 | stale |
| Full device reboot | stale |
| `palm-launch -c` to close the card, then relaunch | stale |
| `luna-send palm://com.palm.appinstaller/installNoVerify` — the service Preware/WOQI actually use | stale |

The `appinstaller` run did update the on-disk payload (version and canary both present afterwards),
so the install itself works; what does not happen is the running card picking it up.

## Why this matters beyond the immediate bug

1. **It invalidates evidence.** Part of the card→adapter diagnosis was reasoning about the current
   `app/` source while the device ran something older. Any conclusion about app-side behaviour drawn
   before this was found must be re-checked against what the device actually executes.
2. **It is another silent-instrument failure**, the same family as the ten fail-open defects found
   this session: the system looked like "the probe never ran" when it was really "the probe is not
   there yet". The canary exists specifically to tell those two apart, and it is what proved this.
3. **The user's requirement stands unmet**: *"you shouldnt need to reboot to install it. at most,
   restarting lunasysmgr should be enough."* Right now not even a reboot is enough for app JS.

## Open leads, in the order worth trying

- Find where webOS caches app resources for a launched card. It is not `extractfs` and not the app
  directory; something between the two is serving an older parse.
- Check whether the app's HTML/JS is being read from a **`.jail`** view or a per-app copy made at
  registration time (`/var/palm/jail/<appid>/`), which would be populated once and reused.
- Confirm by content, not by inference: read the *exact bytes* the card loaded, e.g. by making the
  override log a build stamp that changes every build, and comparing it to the file on disk.
- Establish whether `com.palm.appinstaller/installNoVerify` completed or errored — the call was
  issued with `subscribe:true` and the harness timed out waiting; its response was never read.
