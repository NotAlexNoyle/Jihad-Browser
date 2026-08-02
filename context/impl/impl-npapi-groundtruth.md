---
created: "2026-08-02"
last_edited: "2026-08-02"
status: R7 in progress — NPAPI subsystem confirmed live on device; search path is the gap
---

# R7 ground truth: NPAPI is compiled in and running; only the search path is missing

Measured 2026-08-02 on the TouchPad, before writing any plugin code.

## NPAPI IS in our ARM engine

`--disable-npapi-gtk2` in `build/webos-oe/mozconfig.goanna-arm:20` disables GTK2 **windowed**
(XEmbed) plugins only; `MOZ_ENABLE_NPAPI` is a separate switch, and it is ON. Evidence from the
shipped ARM `libxul.so` (`dist/bin/libxul.so`) — all four NPAPI entry-point symbols and the plugin
prefs are present:

```
NP_Initialize  NP_Shutdown  NP_GetMIMEDescription  NP_GetValue
plugin.disable  plugins.load_appdir_plugins
```

(75 NPAPI-related strings total. `nm -D` shows nothing because the ARM libxul is stripped — use
`strings`, not `nm`, on this binary.)

This confirms the kit's stated ground truth rather than assuming it, and means **nothing needs
enabling in the build**: windowless NPAPI is already available.

## `about:plugins` works

```
[jihad-bs] load done uri=about:plugins
[jihad-bs] titleAndUrl title=[About Plugins] uri=about:plugins
```

The page renders (fb1 capture) and reads **"No installed plugins found"**. That is the correct
result for an empty search path, and it proves `nsPluginHost` initialised and ran a scan — the
subsystem is live, not stubbed out.

So R7's remaining work is **not** "enable NPAPI" and **not** "make about:plugins work". It is:

1. a variant-scoped plugin search path that honours the R8 storage contract (under `$APP`, never
   `/media/internal`, never `/var`),
2. windowless instantiation compositing into our offscreen surface,
3. input routed through the R5 coordinate transform,
4. the `libflashplayer.so` attempt, whose outcome is to be recorded either way.

## Note for whoever does the Flash attempt

The device's own Flash (`/usr/palm/ipkgs/com.palm.app.flashplugin-001_1.0.6_all.ipk` →
`libflashplayer.so`, 8.8 MB, ARM EABI5) links against webOS's WebKit host and compositor —
`libWebKitLuna.so`, `libPiranha.so`, `libLunaSysMgrIpc.so`, `libnapp.so`, `libhal.so`,
`libpowerd.so.0`, `liblunaservice.so`, plus OpenSSL 0.9.8. Whether it functions inside Goanna
rather than LunaSysMgr's WebKit is a genuine open question. The kit requires the outcome recorded
either way, naming exactly which dependency is unsatisfiable if it fails — a documented negative
satisfies the criterion; an untested claim does not.

Silverlight is **not achievable** on this device at all: it shipped x86/x64 for Windows/macOS only,
and the Linux implementation (Moonlight) was x86-only and is long dead. The requirement there is
that our NPAPI *architecture* stays generic, verifiable on the desktop x86_64 build.
