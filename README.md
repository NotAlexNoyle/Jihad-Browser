# Jihad Browser

A web browser for **legacy Palm webOS 3.0.x** (HP TouchPad), built by porting the
**UXP / Goanna** rendering engine into the **isis-browser** front-end. The Enyo UI
shell is kept as-is; only the rendering core is swapped from QtWebKit to Goanna.

> Status: **planning + scaffolding**. No engine build is wired up yet. See
> `context/` for the Cavekit kits and build site that drive implementation.

## Why

isis-browser (the open-webOS browser) renders with an ancient QtWebKit that no
longer handles the modern web. Goanna (the Pale Moon engine, a hard fork of
Gecko ESR-52) is far more capable while still buildable for constrained ARM
devices. Jihad Browser keeps the familiar webOS browser UX and replaces the
engine underneath it.

## Architecture

The original isis architecture is a client–server split:

```
isis-browser (Enyo JS UI)
   │  callBrowserAdapter(method, args)  /  PalmServiceBridge → palm://com.palm.browserServer/*
   ▼
BrowserAdapter (NPAPI plugin in the sysmgr card)
   │  YAP RPC over a socket  +  shared-memory framebuffer (double-buffered)
   ▼
BrowserServer (headless render daemon)
   │  [ QtWebKit backend ]   ← removed
   │  [ Goanna  backend  ]   ← added by Jihad Browser
```

The **YAP IPC contract** (≈80 async commands, 1 sync command, ≈60 async
messages, plus a shared-memory framebuffer signalled by `msgPainted`) is held
**byte-for-byte identical**. The UI and the BrowserAdapter do not change. Only
`BrowserPage`'s internals are reimplemented: instead of a `QGraphicsWebView`,
the Goanna backend drives an `nsIWebBrowser` rendering into an offscreen widget,
copies the result into the shared framebuffer, and emits the same YAP messages
from Goanna's progress/URI/embedding listeners.

See `context/` (kits) for the full requirement decomposition and
`render/goanna/PORT-MAP.md` for the per-command mapping from the YAP contract to
Goanna calls.

## Repository layout

| Path                    | Contents |
|-------------------------|----------|
| `app/`                  | Forked, rebranded isis-browser Enyo UI shell (Apache-2.0). |
| `render/browserserver/` | BrowserServer/Adapter-derived daemon + IPC layer; engine-agnostic parts kept, QtWebKit `BrowserPage` replaced (Apache-2.0). |
| `render/goanna/`        | New Goanna backend: `nsIWebBrowser` driver, offscreen widget, YAP bridge (MPL-2.0). |
| `build/desktop/`        | x86_64 Linux build wiring (Phase 1 PoC). |
| `build/webos-oe/`       | OpenEmbedded recipes for the webOS 3 ARMv7 device build (Phase 2). |
| `context/`              | Cavekit kits + build site (the plan). |
| `docs/`                 | Design notes, IPC contract reference, toolchain notes. |
| `licenses/`             | Full Apache-2.0 and MPL-2.0 texts. |

## Roadmap

- **Phase 0 — Plan & scaffold (this milestone):** Cavekit kits + build site; project skeleton; license setup; YAP contract captured; port map drafted.
- **Phase 1 — Desktop PoC (x86_64 Linux):** Build Goanna once; bring up `BrowserServer` with the Goanna backend; render a page into the shared framebuffer; drive it from the isis UI on desktop. De-risks engine integration before the embedded toolchain.
- **Phase 2 — webOS 3 cross-build (ARMv7):** Stand up a modern cross-toolchain (stock CodeSourcery gcc 4.4 cannot build UXP); cross-compile Goanna + BrowserServer; package `.ipk` via OE; run on the TouchPad.
- **Phase 3 — Hardening:** memory tuning for 1 GB RAM, input/gesture fidelity, TLS/cert store, plugin/MIME handling, on-device performance.

## Licensing

Composite work: Apache-2.0 (UI + IPC daemon) + MPL-2.0 (Goanna backend + engine).
The two are compatible. All Pale Moon / Basilisk / Moonchild trademarks are
removed from the engine build. See `LICENSE` and `NOTICE`.
