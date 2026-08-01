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

## Where the engine's state belongs — measured partition sizes (2026-08-01)

Grounded in `webos://knowledge/system-internals` ("Where persistent state lives") and `df` on the
device. The engine has two distinct storage needs and they do **not** belong on the same partition:

| Path | Free | FS | Verdict |
|---|---|---|---|
| `/var` | **49.6 MB** | ext3 | Cookies/prefs only. Far too small for a browser disk cache, and shared with system state. |
| `/var/file-cache` | 110.8 MB | ext3, encrypted (`store-cryptofilecache`) | **Off limits.** It is the filecache *service's* managed store with its own accounting and GC; squatting there is exactly the bad-citizen behaviour R8 forbids. |
| `/var/db` | 216.8 MB | ext3, encrypted (`store-cryptodb`) | Off limits — mojodb's. |
| `/media/cryptofs` | **10.2 GB** | fuse.cryptofs | The app partition; the engine already executes from here. Right home for the disposable cache. |
| `/media/internal` | 10.2 GB | vfat | The user's USB volume. Barred for app internals; the sole carve-out is finished downloads. |
| `/media/ram` | 459 MB | tmpfs | Volatile and costs RAM on a 1 GB device. No. |

### What isis and Atlas actually do — checked, and they agree

Both prior implementations of this exact browser on this exact device put **cache AND cookies on
cryptofs**, and downloads on the user's volume. This is not a judgement call; it is settled prior
art, and one of the two spells out the hard technical reason.

**isis** — our own upstream, still in the fork at `render/browserserver/Src/Settings.cpp:65-74`
(and identically in `ref-BrowserServer`):
```
CacheEnabled                      true
CacheMaxSize                      "50M"
CachePath                         /media/cryptofs/.browser/cache
CookieJarPath                     /media/cryptofs/.browser/cookies
DownloadPath                      /media/internal/downloads
WebSettings/PersistentStoragePath /media/cryptofs/.browser
```

**Atlas** — `atlas-wpe-backend/BrowserPageWPE.cpp:730`, verbatim, and it is the *why*:
> "Network session data (IndexedDB / localStorage / ServiceWorkers) + disk cache + cookies must NOT
> live on /media/internal: it is VFAT (no hard links, no real file locking). That breaks WebKit's
> NetworkCache (endless 'Failed to create hard link ...') AND, critically, the SQLite-backed website
> storage that heavy apps need… Put them on cryptofs (create/rename/lock work; PROVEN…)"

It routes `netdata`, `netcache` and `cookies.db` to its app deviceroot on cryptofs (through a
`/var/atlas252` bridge symlink that exists only because WPE bakes in a length-limited prefix —
not a constraint we have).

**This independently explains our own 2026-07-20 cookie failure.** `cookies.sqlite` never appeared
because the profile was on VFAT, which gives SQLite no real locking. Moving to ext3 (`/var`) fixed
it — but cryptofs would have fixed it equally, and is where both predecessors put it.

### The split to implement

- `ProfD` (`NS_APP_USER_PROFILE_50_DIR`) → the app's own `profile/` **on cryptofs** — cookies,
  prefs, permissions, cert overrides. Matches isis's `CookieJarPath` and Atlas's `cookies.db`.
- `ProfLD` (`NS_APP_USER_PROFILE_LOCAL_50_DIR`) → the app's own `cache/` **on cryptofs** —
  `cache2`, `startupCache`. Matches isis's `CachePath`. Gecko puts `cache2` under the local key, so
  `EngineHost` needs no new mechanism; it already registers both.
- **Cache cap: 50 MB** — isis's own `CacheMaxSize`, chosen by the people who shipped this browser
  on this hardware. Set it in the low-RAM `goanna.js` block, not at runtime (the "Once" prefs are
  snapshotted before a runtime `SetIntPref` would land — Codex F-235).
- `/var/palm/jihad/<variant>/` keeps ONLY the daemon log and debug channels — operational state for
  a root daemon, kilobytes, correctly on ext3. **Not** the engine profile: `/var` has 49.6 MB free.
- Downloads → `/media/internal/downloads` — isis's exact default, and the user's decision.

Tradeoff, stated deliberately: cryptofs is FUSE, so many-small-file cache I/O is slower than ext3.
Both predecessors accepted it, and the alternative is a cache that fills a 62 MB system partition.
R8 consequence: the profile and cache trees are **untracked** by `ipkg`, so `prerm` must remove them
explicitly or they become residue and break the exact-reversal criterion that is currently green.

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
| app id | `net.riverstonerelay.jihad-browser` | `net.riverstonerelay.jihad-browser-mochi` | `net.riverstonerelay.jihad-browser-mojo` |
| source dir | `app/` | `app-mochi/` | `app-mojo/` |
| NPAPI MIME | `application/x-jihad-browser` | `application/x-jihad-browser-mochi` | `application/x-jihad-browser-mojo` |
| shim (rootfs) | `/usr/lib/BrowserPlugins/BrowserAdapterJihad.so` | `…/BrowserAdapterJihadMochi.so` | `…/BrowserAdapterJihadMojo.so` |
| impl (rootfs) | `/usr/lib/jihad/enyo/BrowserAdapterImpl.so` | `/usr/lib/jihad/mochi/BrowserAdapterImpl.so` | `/usr/lib/jihad/mojo/BrowserAdapterImpl.so` |
| YAP name (`JIHAD_BS_NAME`) | `jihad-browser` | `jihad-browser-mochi` | `jihad-browser-mojo` |
| YAP socket | `/tmp/yapserver.jihad-browser` | `/tmp/yapserver.jihad-browser-mochi` | `/tmp/yapserver.jihad-browser-mojo` |
| upstart job | `/etc/event.d/jihad` | `/etc/event.d/jihad-mochi` | `/etc/event.d/jihad-mojo` |
| engine + daemon | `$APP/deviceroot/hl/` — **run in place** | same | same |
| runtime state dir (log + debug only) | `/var/palm/jihad/enyo/` | `/var/palm/jihad/mochi/` | `/var/palm/jihad/mojo/` |
| daemon log | `/var/palm/jihad/enyo/daemon.log` | `…/mochi/daemon.log` | `…/mojo/daemon.log` |
| engine profile — `ProfD` (durable: cookies, prefs, permissions) | `$APP/profile/` | same shape | same shape |
| engine local profile — `ProfLD` (disposable: `cache2`, `startupCache`) | `$APP/cache/` | same shape | same shape |
| downloads (USER data; the one R8 carve-out) | `/media/internal/downloads` | same | same |

`$APP` = `/media/cryptofs/apps/usr/palm/applications/<app id>` — i.e.
`…/net.riverstonerelay.jihad-browser`, `…/net.riverstonerelay.jihad-browser-mochi`,
`…/net.riverstonerelay.jihad-browser-mojo`.

The Enyo variant keeps the unsuffixed names it already has on-device — renaming it would break the
one deployment that is known to work, for no gain.

**The suffixed app ids use a HYPHEN, never a dot** (`…jihad-browser-mochi`, not
`…jihad-browser.mochi`). This is not cosmetic and it is not negotiable. webOS's `/usr/bin/ipkg`
stores package metadata as `info/<pkgid>.{control,list,prerm}` and cleans up on removal with a glob
on `<pkgid>.*` — which also matches `<pkgid>.child.control`. With dotted ids the two suffixed
variants were *dot-children* of the Enyo id, so **removing the Enyo package deleted Mochi's and
Mojo's control/list/prerm**, leaving them un-uninstallable and their shim, impl and upstart job as
permanent rootfs residue — a direct P1 against R7 and R8. Proven on-device 2026-08-01 with two
pairs of minimal packages: with a dot the child's metadata is destroyed, with a hyphen it survives
intact (`../impl/impl-ipkg-prefix-collision.md`). Any FUTURE variant id must be a hyphen suffix for
the same reason.

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
