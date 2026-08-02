---
created: "2026-08-02"
last_edited: "2026-08-02"
status: enyo deployed and working; mochi + mojo are SHELLS
---

# Only the enyo variant is actually deployed

Measured on device 2026-08-02. This corrects any impression that "three variants are installed":
the other two have app directories but nothing that can run.

| | enyo | mochi | mojo |
|---|---|---|---|
| app dir | yes | yes | yes |
| `deviceroot/hl` | yes | yes | yes |
| **`libxul.so`** | **yes** | **NO** | **NO** |
| **`BrowserAdapterImpl.so`** | **yes** | **NO** | **NO** |
| upstart job (`/etc/event.d/jihad*`) | `jihad` only | **none** | **none** |
| plugin slot (`/usr/lib/BrowserPlugins`) | `BrowserAdapterJihad.so` | **none** | **none** |
| impl lib (`/usr/lib/jihad/<v>/`) | `enyo/` only | **none** | **none** |
| YAP socket | `/tmp/yapserver.jihad-browser` | **none** | **none** |

So R7 (per-variant independence) is **verified only in the packaging/uninstall matrix**, not by
three variants running side by side — the isolation test that matters most has not been run on
hardware because two of the three participants do not exist yet.

## Why they are not deployed, honestly

The per-variant payload is the engine bundle (~741 MB assembled, dominated by libxul), and the two
install routes both have problems:

- `palm-install` is unusable here — a 44 MB `.ipk` hung twice with no staging file and no `ipkg`
  process on the device, and two concurrent runs raced on the same app directory
  (context/impl/impl-stale-app-js.md).
- Preware / WebOS Quick Install is the supported route (cavekit-device-build.md R8) but is
  user-driven; it cannot be executed from here.

Pushing two more bundles file-by-file over `novacom put` is possible — that is effectively how the
enyo `deviceroot` was populated — but at ~741 MB each it is a long operation and should be done
deliberately, not as a side effect of another task.

## What to do

1. `build/webos-oe/build-variant-ipk.sh` already builds all three `.ipk`s; the bundle now also
   carries the branding package and the add-on pref block, so rebuild before installing.
2. Install mochi + mojo via Preware/WOQI (the supported path).
3. Then run the real R7 test: all three daemons up on their own sockets
   (`/tmp/yapserver.jihad-browser{,-mochi,-mojo}`), each card reaching only its own variant, and
   removing one leaving the others untouched — `build/webos-oe/device-independence-test.sh` exists
   for this and has only ever been exercised against one live variant.

## Not a defect: stale `/tmp` sockets

23 leftover `/tmp/yapserver.jihad-browser<pid>-<n>` per-connection sockets were present. These are
the client-side msg-server sockets, and `YapClient.cpp:154` *does* unlink them on teardown — they
accumulated only because this debugging session repeatedly `killall`ed LunaSysMgr, and SIGKILL
skips the cleanup. `/tmp` is tmpfs, so a reboot clears them. No code change made or needed.
