---
created: "2026-08-01"
last_edited: "2026-08-01"
status: RESOLVED
---

# RESOLVED — app JS changes need a LunaSysMgr restart (WebAppMgr caches app sources)

**Root cause: `WebAppMgr` caches an app's JS/HTML in-process.** Changing the file on disk and
relaunching the card is not enough; the cached script keeps running. `killall LunaSysMgr`
(upstart respawns LunaSysMgr *and* WebAppMgr) makes the new code run on the very next launch.

No reboot, no version bump, no reinstall, and no packaging change is required. This satisfies
the user's constraint: *"you shouldnt need to reboot to install it. at most, restarting
lunasysmgr should be enough."*

## How it was proven

The anomaly was that two **adjacent statements** in `JihadEngineOverride.js` behaved
differently — the unguarded `log("WebView engine -> …")` appeared in `palm-log` every launch
while the statement immediately after it never did. It was reproduced in three different code
shapes to eliminate every alternative:

| Shape tried | Result | What it ruled out |
|---|---|---|
| `log("probe-canary: JIHAD_PROBE=" + …)` | absent | — |
| plain words, no `:` / `=`, wrapped in `try/catch` | absent | message SHAPE, and a thrown exception |
| buffered `stamp()` + `flushStamps()` | absent | logger-not-ready (stamps survive `enyo` being unloaded) |

With, at the same time:

- device file **md5-identical** to the local file (checked immediately before each launch);
- **only one** copy of the file on the device — `grep -rl` over `source/` and `mock/` of all
  three variant app dirs; mochi and mojo do not even contain this file;
- no shadow copy in any of the six `ApplicationPath` scan dirs;
- `palm-log` timestamps inside the launch window, so the lines were from the current launch.

After `killall LunaSysMgr`, first launch:

```
[Jihad] WebView engine -> application/x-jihad-browser
[Jihad] stamps at-patch Atrue,Btrue,Etrue
[Jihad] probe canary alive
[Jihad] stamps at-end Atrue,Btrue,Etrue,F,C,Dtrue
```

Every stamp present, `JIHAD_PROBE` true at every point — i.e. the code had been correct all
along and the device had simply never executed it.

## Two real defects found on the way, both worth keeping in mind

1. **Concurrent `palm-install` runs race on the app directory.** A `palm-install` from an
   earlier turn was still running (>20 min, never completed) when a second one was started
   against the same app id. Two installers were writing
   `/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser`
   simultaneously. **Every earlier "installed it, still stale" datapoint was collected inside
   that window and is untrustworthy** — including the `killall LunaSysMgr` ×5 and full-reboot
   attempts recorded in the previous version of this file, which is why the correct fix looked
   like it had already been ruled out.
2. **`palm-install` is unusable for iteration here.** 44 MB over novacom hung twice with no
   staging file and no `ipkg` process on the device. `novacom put` of the single changed file
   takes ~10 s and is md5-verifiable.

## The dev loop to use

```bash
D=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/source/X.js
novacom put "file://$D" < app/source/X.js     # ~10 s, verify with md5sum both sides
novacom run file://usr/bin/killall -- LunaSysMgr
palm-launch -d usb net.riverstonerelay.jihad-browser
```

Check for a leftover `palm-install` / `webos-tools.jar` process before starting any install.

## Note on `getAppInfo` version

`getAppInfo` reports a stale `"version"` (1.0.0 while `appinfo.json` on disk said 1.0.2). That
is the app-registry cache, a *different* staleness from this bug, and it is a red herring for
code freshness: the registry's `main` already pointed at the correct on-disk path, and the code
went live after a LunaSysMgr restart without the registry version ever changing.
