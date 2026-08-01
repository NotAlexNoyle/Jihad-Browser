---
created: "2026-07-31"
last_edited: "2026-07-31"
---

# Plan: per-variant identity + install footprint

Implements cavekit-device-build.md **R7** (three independent packages) and **R8** (good-citizen
install footprint). This is the single source of truth for every name and path the three variants
use; every task in Tier 9 of `build-site.md` (T-055..T-061) must agree with this table.

## Why

User decisions (2026-07-31):
1. "each app (mojo, enyo, mochi) needs to work entirely on its own, self contained" →
   **fully independent per app**, no co-owned component, no refcount.
2. "without modifying system files … it should be a good webOS citizen" +
   "follow atlas in not copying anything to /media/internal".

Atlas was checked for a way to avoid the one unavoidable rootfs write and has none: webOS's WebKit
scans `/usr/lib/BrowserPlugins` at boot, and Atlas's own README calls that "the **only** path the
app loads it from". So the plan **bounds** that footprint (a tiny shim + the impl + one upstart job,
all variant-namespaced) and reverses it exactly, instead of pretending it away.

## Verified platform facts (this device, 2026-07-31)

- `/media/cryptofs` is `fuse.cryptofs (rw,nosuid,nodev)` — **not** `noexec`. Exec from an app's
  install dir was proven live (busybox copied into the app dir ran). So the daemon + bundled
  `ld-2.23.so` + `libxul.so` can run **in place** from `deviceroot/`.
- `/media/internal` is the user's **vfat** USB mass-storage volume. Off limits for app internals.
- `/var/palm` exists, `root:root 0755`, on the ext3 rootfs — suitable for root-owned runtime state.
- `/` is `ext3 rw` but treated as read-only; writes need a `remount,rw` window.

## Identity table

| | **Enyo 1.0** | **Enyo 2 / Mochi** | **Mojo** |
|---|---|---|---|
| variant token `V` | `enyo` | `mochi` | `mojo` |
| app id | `net.riverstonerelay.jihad-browser` | `net.riverstonerelay.jihad-browser.mochi` | `net.riverstonerelay.jihad-browser.mojo` |
| source dir | `app/` | `app-mochi/` | `app-mojo/` |
| NPAPI MIME | `application/x-jihad-browser` | `application/x-jihad-browser-mochi` | `application/x-jihad-browser-mojo` |
| shim (rootfs) | `/usr/lib/BrowserPlugins/BrowserAdapterJihad.so` | `…/BrowserAdapterJihadMochi.so` | `…/BrowserAdapterJihadMojo.so` |
| impl (rootfs) | `/usr/lib/jihad/enyo/BrowserAdapterImpl.so` | `/usr/lib/jihad/mochi/BrowserAdapterImpl.so` | `/usr/lib/jihad/mojo/BrowserAdapterImpl.so` |
| YAP name (`JIHAD_BS_NAME`) | `jihad-browser` | `jihad-browser-mochi` | `jihad-browser-mojo` |
| YAP socket | `/tmp/yapserver.jihad-browser` | `/tmp/yapserver.jihad-browser-mochi` | `/tmp/yapserver.jihad-browser-mojo` |
| upstart job | `/etc/event.d/jihad` | `/etc/event.d/jihad-mochi` | `/etc/event.d/jihad-mojo` |
| engine + daemon | `$APP/deviceroot/hl/` — **run in place** | same | same |
| runtime state dir | `/var/palm/jihad/enyo/` | `/var/palm/jihad/mochi/` | `/var/palm/jihad/mojo/` |
| daemon log | `/var/palm/jihad/enyo/daemon.log` | `…/mochi/daemon.log` | `…/mojo/daemon.log` |

`$APP` = `/media/cryptofs/apps/usr/palm/applications/<app id>`.

The Enyo variant keeps the unsuffixed names it already has on-device — renaming it would break the
one deployment that is known to work, for no gain.

## Rules that follow from the table

1. **No glob may span variants.** `rm -f /tmp/yapserver.jihad-browser*` deletes the Mochi and Mojo
   sockets too. Every socket/job/file reference is an exact path.
2. **No refcount.** Nothing is co-owned, so `prerm` unconditionally removes exactly this variant's
   files: its shim, its `/usr/lib/jihad/$V/` directory, its upstart job, its `/var/palm/jihad/$V/`
   state. It removes `/usr/lib/jihad` itself only with `rmdir` (succeeds only when empty).
3. **Nothing is written to `/media/internal`** by install, by the daemon, by the adapter, or by the
   shim. Debug channels that used it move to `/var/palm/jihad/$V/` and stay off by default.
4. **The rootfs rw window** wraps only the rootfs writes and is closed by a `trap … EXIT` so a
   failure mid-install still leaves `/` read-only.
5. **The impl stays on the rootfs**, not in the app bundle: the shim dlopens it into privileged
   LunaSysMgr and enforces root-owned + not-group/world-writable. cryptofs reports every file as
   `777`, so an app-bundle impl can never pass that check. (This is why the impl path is under
   `/usr/lib/jihad/$V/` and not `$APP/`.)
6. **The daemon runs in place** from `$APP/deviceroot/hl`, launched by that variant's upstart job
   via the bundled `ld-2.23.so --library-path $APP/deviceroot/hl`.

## Build-time parametrization

One source, three builds. Both adapter halves take the variant on the compiler command line —
no per-variant source copies:

- `render/adapter/BrowserAdapterShim.cpp` — `JIHAD_MIME`, `JIHAD_IMPL_PATH`, `JIHAD_VARIANT`.
- the adapter impl (`ref-BrowserAdapter/BrowserAdapter.cpp`) — `JIHAD_MIME`, `JIHAD_YAP_NAME`.
- the daemon reads `JIHAD_BS_NAME` from the environment (already) and derives its state dir from it.

## Out of scope here
- Changing the YAP command/message wire format. It stays byte-identical; only the *service name*
  differs per variant, exactly as the stock browser vs. `browsermojo` already differ.
