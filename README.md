<p align="center">
  <img src="docs/jihad-browser-logo.png" alt="Jihad Browser logo" width="200">
</p>

# Jihad Browser

<p align="center"><em>Goanna inside, webOS alive, inshallah.</em></p>

A web browser for **legacy Palm webOS 3.0.x** (HP TouchPad), built by porting the
**UXP / Goanna** rendering engine into the **isis-browser** front-end. The Enyo UI
shell is kept as-is; only the rendering core is swapped from QtWebKit to Goanna.

> Status: **running on the HP TouchPad as a self-contained app.** The Goanna
> engine cross-builds for webOS 3 ARMv7; the daemon renders real web pages
> on-device (e.g. `example.com`, `slack.com` over HTTPS) into the isis UI, with
> load-completion and the address-bar refresh glyph working. Jihad installs as a
> **self-contained browser that coexists with the stock webOS browser** — its own
> NPAPI MIME, adapter, render daemon, and upstart job; nothing system-level is
> replaced (see below). Remaining work: input-activation edge cases (tap/keyboard),
> landscape composite, the Mochi UI variant, and TouchPad Go.

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
**byte-for-byte identical** — no command or message is added, removed, renamed,
or re-typed. Only `BrowserPage`'s internals are reimplemented: instead of a
`QGraphicsWebView`, the Goanna backend drives an `nsIWebBrowser` rendering into
an offscreen widget, copies the result into the shared framebuffer, and emits the
same YAP messages from Goanna's progress/URI/embedding listeners.

### Self-contained coexistence (redistributable app)

Jihad is packaged as a **self-contained app that runs alongside the stock webOS
browser** — the system `BrowserAdapter.so` / `browserserver` job / stock browser
are never touched. Coexistence follows the Atlas model: four additive pieces, each
with its own name so there is no collision.

| Piece | Jihad | Stock (untouched) |
|-------|-------|-------------------|
| NPAPI MIME | `application/x-jihad-browser` | `application/x-palm-browser` |
| Adapter plugin | `BrowserAdapterJihad.so` | `BrowserAdapter.so` |
| Render daemon socket | `/tmp/yapserver.jihad-browser` | `/tmp/yapserver.browser` |
| Upstart job | `jihad` | `browserserver` |

The Enyo shell swaps **only its own** WebView's plugin type to
`application/x-jihad-browser` (`app/source/JihadEngineOverride.js`), so the card
loads the Jihad adapter → the Jihad daemon → Goanna. The adapter therefore carries
a **two-line rebrand** (its MIME string and YAP server name); the YAP command/message
interface it speaks is still byte-identical. A later, opt-in goal is to replace the
system browser engine for *all* apps — for now Jihad is app-scoped.

See `context/` (kits) for the full requirement decomposition and
`render/goanna/PORT-MAP.md` for the per-command mapping from the YAP contract to
Goanna calls.

## Repository layout

| Path                    | Contents |
|-------------------------|----------|
| `app/`                  | UI variant 1 — forked, rebranded isis-browser **Enyo 1.0** shell (Apache-2.0). |
| `app-mochi/`            | UI variant 2 — **Enyo 2 + Mochi** shell, same contract, separate `.ipk` (Apache-2.0). |
| `render/browserserver/` | BrowserServer/Adapter-derived daemon + IPC layer; engine-agnostic parts kept, QtWebKit `BrowserPage` replaced (Apache-2.0). |
| `render/goanna/`        | New Goanna backend: `nsIWebBrowser` driver, offscreen widget, YAP bridge (MPL-2.0). |
| `build/desktop/`        | x86_64 Linux build wiring (Phase 1 PoC). |
| `build/webos-oe/`       | OpenEmbedded recipes for the webOS 3 ARMv7 device build (Phase 2). |
| `context/`              | Cavekit kits + build site (the plan). |
| `docs/`                 | Design notes, IPC contract reference, toolchain notes. |
| `licenses/`             | Full Apache-2.0 and MPL-2.0 texts. |

## Roadmap

- **Phase 0 — Plan & scaffold — ✅ done:** Cavekit kits + build site; project skeleton; license setup; YAP contract captured; port map drafted.
- **Phase 1 — Desktop PoC (x86_64 Linux) — ✅ done:** Build Goanna once; bring up `BrowserServer` with the Goanna backend; render a page into the shared framebuffer; drive it from the isis UI on desktop. De-risked engine integration before the embedded toolchain.
- **Phase 2 — webOS 3 cross-build (ARMv7) — ✅ reached:** modern cross-toolchain stood up (stock gcc 4.4 cannot build UXP); Goanna + daemon cross-compiled; the self-contained adapter + daemon + upstart job deploy and **render real pages on the TouchPad**. Remaining: full OE-recipe `.ipk` for both UI variants + TouchPad Go.
- **Phase 3 — Hardening — 🔧 in progress:** input/gesture fidelity (tap activation, keyboard/VKB), landscape composite, memory tuning for 1 GB RAM, TLS/cert store, on-device performance.

## Licensing

Composite work: Apache-2.0 (UI + IPC daemon) + MPL-2.0 (Goanna backend + engine).
The two are compatible. All Pale Moon / Basilisk / Moonchild trademarks are
removed from the engine build. See `LICENSE` and `NOTICE`.
