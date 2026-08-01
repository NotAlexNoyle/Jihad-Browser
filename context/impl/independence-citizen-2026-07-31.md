---
created: "2026-07-31"
last_edited: "2026-07-31"
---

# Per-variant independence + good-citizen install (2026-07-31)

Implements cavekit-device-build.md **R7** + **R8** and the new cavekit-mojo-ui.md.
Naming/path contract: `../plans/plan-variant-identity.md`. Build site: Tier 9, T-055..T-061.

## The two user decisions that drove this

1. **"each app (mojo, enyo, mochi) needs to work entirely on its own, self contained"** — three
   fully independent packages. Retires the 2026-07-29 shared-runtime + `prerm` refcount model
   (review items #4/#5): nothing is co-owned, so there is nothing to refcount.
2. **"without modifying system files … it should be a good webOS citizen"**, refined to
   **"follow atlas in not copying anything to /media/internal"**.

## What the Atlas investigation actually found (asked for explicitly)

Atlas was checked for a way to avoid the rootfs write. **It has none, and does the same thing we
do**: `Atlas/atlas-browser-app/packaging/ipk-postinst.sh` remounts `/` rw and installs
`/usr/lib/BrowserPlugins/BrowserAdapterAtlas.so` + `/etc/event.d/atlas`, and its README calls that
plugin path "the **only** path the app loads it from". webOS's WebKit scans that directory for
NPAPI plugins at boot; there is no app-local plugin path. So R8 **bounds and reverses** the
footprint rather than eliminating it.

Atlas **is** stricter than Jihad was in one respect, and that is the part we adopted: it runs its
engine **in place from the app's cryptofs `deviceroot`** and copies nothing to `/media/internal`,
with the comment "that vfat partition is the user's USB storage and must stay free of app
internals". Jihad had been copying its whole runtime to `/media/internal/jihad/hl`.

## Platform facts established on hardware this session

| Fact | How it was established | Why it matters |
|---|---|---|
| `/media/cryptofs` is `fuse.cryptofs (rw,nosuid,nodev)` — **not noexec** | `mount` on device | The in-place model is only possible if the daemon can exec from the app bundle |
| A binary in an app's install dir **executes** | copied busybox into the Enyo app dir, ran it, got its "applet not found" — i.e. it ran | Proves the above rather than assuming it |
| `/var/palm` exists, `root:root 0755`, ext3 | `ls -ld` | Gives runtime state a root-owned home off the user's volume |
| cryptofs reports every file `0777` | `ls -la` of the app dir | **The adapter impl cannot live in the app bundle** — the shim's trust check (root-owned, not group/world-writable) can never pass there. This is why the impl belongs on the rootfs under `/usr/lib/jihad/<variant>/` |
| `/usr/bin/ipkg` is present | `ls` | `palm-install` does not run `postinst`; installs must go through ipkg for the deploy to happen |

## Device cleanup performed (the "before" side of R8 evidence)

The device carried a development layout that violated R8 by a wide margin:

- **`/media/internal/jihad` was 5.3 GB** on the user's USB volume — two bundle tarballs (43 MB +
  41 MB), a **28 MB** daemon log, ~40 × 9.4 MB framebuffer dumps, the running daemon tree, and 285
  ad-hoc dev shell scripts.
- `/usr/lib/BrowserPlugins` held two hand-made backups (`BrowserAdapter.so.stock`,
  `BrowserAdapterJihad.so.prejihadshim`) alongside the shim.

Actions, in order, all scripted so they are auditable and repeatable:

1. The 285 on-device dev scripts (the only content not reproducible from the repo) were archived to
   `build/webos-oe/device-scratch-archive/jihad-devscripts-2026-07-31.tgz` (46 KB).
2. `build/webos-oe/device-purge-legacy.sh --apply` removed the legacy layout. It **refuses** to
   delete `BrowserAdapter.so.stock` unless that file is byte-identical to the live stock adapter —
   if they differ, the live one might not be stock and the backup is the only original.
   They were identical (`6014eabf…`), so the backup was provably redundant.
3. `build/webos-oe/device-citizen-audit.sh snap baseline` captured the clean state.

**Result (`diff as-is baseline`):** all five stock checksums unchanged
(`BrowserAdapter.so`, `BrowserAdapterMojo.so`, `RemoteAdapter.so`, `/etc/event.d/browserserver`,
`/etc/event.d/browserservermojo`, plus `/usr/bin/BrowserServer{,Mojo}`); the only removals were our
own files; `/media/internal` no longer contains anything Jihad. Stock `BrowserServer` (pid 26610,
up since Jul 18) and `BrowserServerMojo` (pid 1942, up since Jul 16) were **never restarted or
disturbed** by any of it.

## Evidence tooling added

| Script | Role |
|---|---|
| `build/webos-oe/device-citizen-audit.sh` | `snap`/`diff` filesystem+checksum snapshots. R8's "no stock file modified" and "no residue" criteria are before/after comparisons, so they need a comparable artifact, not a human eyeball. |
| `build/webos-oe/device-independence-test.sh` | The R7 matrix: install/remove the three variants in sequence, asserting after each step that every other variant still has its own shim, impl, upstart job, socket, daemon, and in-place runtime — and that a full install→remove cycle returns the filesystem exactly to baseline. |
| `build/webos-oe/device-purge-legacy.sh` | One-time migration off the pre-R8 layout (dry-run by default). |

## Also closed this session (desktop, independent of the above)

- **browser-services R4 downloads — DOWNLOAD-LIFECYCLE PASS** (commit 8953eab). Root cause of the
  long-standing gap: only the `nsIHelperAppLauncherDialog` handoff was captured, so
  `nsExternalAppHandler::CreateTransfer` failed and **cancelled every download**. Registering an
  `@mozilla.org/transfer;1` (`nsITransfer`) implementation is mandatory; its
  `init`/`onProgressChange64`/`onStateChange(STOP)` now drive `msgDownloadStart/Progress/Finished`,
  and the `nsICancelable` it receives is what `cancelDownload` aborts (verified:
  `msgDownloadError 0x804b0002 = NS_BINDING_ABORTED`, never `Finished`).
- **browser-services R2 cookie persistence — COOKIE-PERSISTENCE PASS on desktop** (same commit):
  a persistent cookie survives a full engine teardown/restart and `cookies.sqlite` exists in the
  profile. This reframes the 2026-07-20 on-device failure: the design is right, the failure was
  VFAT-specific — and R8 moves the profile to ext3 (`/var/palm/jihad/<variant>/`) anyway, which may
  resolve it outright. The device retest must target the new path.

## Incidental finding: the desktop round-trip harness had been broken (and the docs did not know)

Found while verifying T-057, and worth recording separately because it invalidates a claim the
tracking documents carried:

`render/browserserver/test/adapter_client.cpp` — the desktop YAP stand-in for the real
BrowserAdapter — allocated its shared-memory segment as `W*H*4`. But the daemon writes the **isis
shmem layout**, `[BrowserOffscreenInfo header][ARGB32 pixels]`, and `paintToSharedBuffer()`
silently `return`s if the segment is too small for header+pixels. So the daemon painted nothing,
the harness waited forever, and the run ended in a timeout (exit 124) with no `msgPainted`.

- **Pre-existing, not caused by this session's work**: pristine `HEAD` was built into a scratch
  tree (via `git archive`, leaving the working tree untouched) and failed identically.
- **When it broke:** the daemon adopted the real header layout in the rotation/zoom work
  (`35ad1c1`, then `8d7865c` 2026-07-27); the harness was last touched at `36e8513`, long before,
  and was never updated to match. The daemon was right; the *test* was stale.
- **Consequence for the record:** every "desktop ROUND-TRIP PASS" restated after 2026-07-27 was
  inherited from an older run, not reproduced. The device round-trip is unaffected (the real
  adapter always allocated header+pixels — that is why the layout change worked on hardware while
  the desktop harness quietly stopped).
- **Fixed** as part of this session: the harness now sizes the segment for header+pixels, reads
  pixels at `base+sizeof(BrowserOffscreenInfo)`, and logs the header geometry. ROUND-TRIP PASS is
  now genuine again — `msgPainted … 685624 non-white px`, `painted=1 verified=1`, exit 0.

Lesson worth keeping: a harness that "still passes" because nobody re-ran it is indistinguishable
from one that passes. The build scripts under `build/desktop/` are the acceptance vehicle for most
of Phase 1, so a stale one silently converts verified requirements into asserted ones.

## Open at time of writing

T-055..T-060 are in flight. T-061 (the on-device independence matrix) cannot run until the three
`.ipk`s exist. Nothing in this document is claimed as device-verified for the new packaging — only
the cleanup, the baseline, and the platform facts above are.
