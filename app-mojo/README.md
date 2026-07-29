# Jihad Browser — Mojo UI (skeleton)

A **scaffold for a future Mojo port** of the Jihad Browser front-end. Mojo is the original
(pre-Enyo) webOS application framework; this is a third UI variant alongside the shipping
**Enyo 1.0** (`app/`) and **Enyo 2 + Mochi** (`app-mochi/`) shells.

## What already works (for free)

This variant packages the **same self-contained runtime bundle** as the other two — the Goanna
engine (`libxul.so`), the render daemon, the NPAPI adapter, and the bundled glibc-2.23 — via the
shared `jihad-deviceroot` recipe. When the `.ipk` installs, its postinst lays the daemon + adapter
shim + upstart job into place exactly like the Enyo variants. **A Mojo port does not touch the
engine at all.**

## What a real port has to build (only the UI)

The whole trick of the Jihad browser is a WebView whose plugin MIME is swapped to
`application/x-jihad-browser`, which routes the card through the Jihad adapter → daemon → Goanna
instead of the stock engine. In Enyo that swap is `app/source/JihadEngineOverride.js`. A Mojo port
must do the equivalent with Mojo's web widget:

1. In the main scene, instantiate a Mojo web widget (`Mojo.Widget.WebView` / the PalmSysMgr web
   control) and set its plugin type to `application/x-jihad-browser` before first load.
2. Build the chrome: address bar, back / forward / reload / stop, progress, on-screen-keyboard focus.
3. Wire the WebView + address bar to the same contract the Enyo shell uses —
   `callBrowserAdapter(...)` + `palm://com.palm.browserServer/*` (the byte-identical YAP interface).

## Files

| File | Role |
|------|------|
| `appinfo.json` | webOS app metadata (`type: web`, Mojo). |
| `index.html` | Bootstraps the Mojo framework. |
| `sources.json` | Mojo source manifest (stage + main scene). |
| `app/assistants/stage-assistant.js` | Stage assistant — pushes the main scene. |
| `app/assistants/main-assistant.js` | **Where the WebView + chrome go** (TODOs). |
| `app/views/main/main-scene.html` | Main scene markup (placeholder). |

## Build

The recipe `build/webos-oe/recipes-jihad/jihad-ui/net.riverstonerelay.jihad-browser.mojo_1.0.bb`
builds this into a self-contained `.ipk` just like the other two variants (it `require`s
`jihad-app.inc`). It is a **skeleton**: it installs and the engine works, but the UI is a stub until
the TODOs above are done.
