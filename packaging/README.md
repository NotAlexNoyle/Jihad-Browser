# packaging/ — self-contained on-device install

Jihad Browser installs as a **self-contained app that coexists with the stock
webOS browser**. Nothing system-level is replaced; four additive, independently
named pieces let the Jihad card use Goanna while every other app keeps the stock
engine. See `docs/DEVICE-BUILD.md` and auto-memory `jihad-self-contained-arch.md`.

| Piece | Jihad | Stock (untouched) |
|-------|-------|-------------------|
| NPAPI MIME | `application/x-jihad-browser` | `application/x-palm-browser` |
| Adapter | `/usr/lib/BrowserPlugins/BrowserAdapterJihad.so` | `BrowserAdapter.so` |
| Daemon socket | `/tmp/yapserver.jihad-browser` | `/tmp/yapserver.browser` |
| Upstart job | `/etc/event.d/jihad` | `browserserver` |

## Files here

- **`event.d/jihad`** — upstart job that runs the Goanna daemon with
  `JIHAD_BS_NAME=jihad-browser` (own socket; `respawn`). `LD_LIBRARY_PATH` is set
  via `env` on the daemon only — never exported (else `/bin/rm` etc. load the
  bundled gcc9 glibc and segfault before the daemon execs).
- **`postinst`** — lays the daemon into `/media/internal/jihad/hl`, the adapter
  into `/usr/lib/BrowserPlugins/BrowserAdapterJihad.so`, and the upstart job into
  `/etc/event.d/jihad` (one rootfs-rw window); starts `jihad`; reloads LunaSysMgr.
- **`prerm`** — reverses it, touching only our own files.

## Build inputs (from the git-excluded `build/` tree)

- Adapter: `build/webos-oe/build-adapter-pdk.sh` (gcc4.3.3 PDK) → `cp
  build-pdk/BrowserAdapter.so BrowserAdapterJihad.so`. The MIME + YAP name are
  compiled in from `ref-BrowserAdapter/BrowserAdapter.cpp`
  (`AdapterGetMIMEDescription`, `BrowserClientBase("jihad-browser", …)`).
- Daemon: the `build/webos-oe` ARM build (crosstool-NG gcc9) → `jihad-browserserver`.
- UI `.ipk`: `palm-package app/` (bundles `app/source/JihadEngineOverride.js`, which
  routes the app's WebView to `application/x-jihad-browser`).

## Two deploy gotchas (must-do)

1. **Reboot after installing a new adapter.** webOS WebKit builds its NPAPI
   MIME/plugin database at BOOT by scanning `/usr/lib/BrowserPlugins`; `killall
   LunaSysMgr` does NOT re-scan, so `application/x-jihad-browser` stays
   unregistered until the next boot. Confirm with
   `grep -l BrowserAdapterJihad /proc/*/maps` after a launch.
2. **App JS changes need a `.ipk` reinstall**, not a loose-file `novacom put` — a
   push into the installed app dir does not bust WebKit's `file://` resource cache,
   so `JihadEngineOverride.js` silently won't run.
