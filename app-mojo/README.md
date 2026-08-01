# Jihad Browser — Mojo UI variant

The third browser front-end, built on **Mojo** — the original (pre-Enyo) webOS
application framework, which webOS 3.0.5 still ships on-device at
`/usr/palm/frameworks/mojo`. It is a browser in its own right, alongside the
**Enyo 1.0** shell (`../app`) and the **Enyo 2 + Mochi** shell (`../app-mochi`).

Requirements: `../context/kits/cavekit-mojo-ui.md`. Contract:
`../docs/IPC-CONTRACT.md`. Names and paths: `../context/plans/plan-variant-identity.md`.

## Fully independent package

Per cavekit-device-build.md **R7** this variant owns a complete stack of its own —
app id `net.riverstonerelay.jihad-browser-mojo`, NPAPI MIME
`application/x-jihad-browser-mojo`, adapter shim `BrowserAdapterJihadMojo.so`,
adapter impl under `/usr/lib/jihad/mojo/`, YAP name `jihad-browser-mojo`, socket
`/tmp/yapserver.jihad-browser-mojo`, upstart job `/etc/event.d/jihad-mojo`. It shares
nothing with the Enyo or Mochi variants, and installing or removing it leaves them
untouched. It also coexists with the *stock* Mojo browser, which has its own separate
adapter/daemon pair (`application/x-palm-mojo-browser` → `/tmp/yapserver.browsermojo`).

The app id's suffix is a **hyphen, not a dot**, and that is load-bearing: `ipkg` stores
package metadata as `info/<pkgid>.{control,list,prerm}` and removes a package by globbing
`<pkgid>.*`, which also matches `<pkgid>.mojo.control`. With the old dotted id, removing
the Enyo package destroyed this package's control scripts and file list — so it could no
longer be uninstalled and its shim, impl and upstart job became permanent rootfs residue
(`../context/impl/impl-ipkg-prefix-collision.md`).

The engine, the daemon and the adapter come from the shared `jihad-deviceroot`
runtime bundle that this package's recipe installs; **nothing in `app-mojo/` touches
the engine**. This directory is the UI only.

## Uses the system Mojo framework

`index.html` loads `/usr/palm/frameworks/mojo/mojo.js` with
`x-mojo-submission="506"` — the submission webOS 3.0.5 carries as a builtin
(`/usr/palm/frameworks/mojo/builtins/palmInitFramework506.js`). No framework is
bundled (cavekit-mojo-ui.md R4). `x-mojo-version="1"`/`"2"` is deliberately **not**
used: mojo.js maps both to submission 344, which this device does not ship.

## How the engine routing works

`Mojo.Widget.WebView` renders the BrowserAdapter NPAPI `<object>`, but hard-codes its
plugin type to the stock `application/x-palm-mojo-browser` inside its own `setup()`
and offers no attribute for it. `app/models/jihad-engine-override.js` wraps that
`setup()` and, for the duration of that one call, shadows `appendChild` on the
widget's own container so the still-detached `<object>` gets this variant's MIME
before it goes live. Rewriting the type after `setup()` returns would be too late.
This is the Mojo analogue of `../app/source/JihadEngineOverride.js`.

The scene assistant re-checks the live plugin element on first activation and
surfaces an error if it is not bound to our MIME, so a broken override can never
quietly run the card on somebody else's engine.

## Contract surface

`MainAssistant.callBrowserAdapter()` is the only app-facing adapter surface. The set
of method-name literals routed through it is a strict **subset** of the Enyo-1.0
app's set:

| | method set | `palm://com.palm.browserServer/*` |
|---|---|---|
| Enyo (`../app`) | `findInPage`, `goBack`, `goForward`, `reloadPage`, `stopLoad` | `clearCache`, `clearCookies` |
| Mojo (here) | `goBack`, `goForward`, `reloadPage`, `stopLoad` | *(none)* |

No additions, no renames. Everything else the page needs — `openURL`, the connect
handshake, viewport sizing, magnification, dialog responses — is the WebView
widget's own internal traffic to the plugin, exactly as Enyo-1.0's `BasicWebView`
keeps those calls out of the app's own call set.

**Known adapter gap:** `Mojo.Widget.WebView` also exposes `clearCache()` and
`clearCookies()`, which call `adapter.clearCache()` / `adapter.clearCookies()`. The
isis BrowserAdapter (and so ours) does **not** expose those as scriptable methods —
the Enyo shell reaches them through `palm://com.palm.browserServer/*` instead. This
UI therefore never calls them. Every other adapter method the widget invokes on the
paths used here exists in the adapter.

## Files

| File | Role |
|------|------|
| `appinfo.json` | webOS app metadata (`type: web`, Mojo app id, Jihad icon set). |
| `index.html` | Loads the system Mojo framework + the app stylesheet. |
| `sources.json` | Mojo source manifest — **every** JS file must be listed here. |
| `start.html` | The page the card opens on, loaded through the engine. |
| `stylesheets/jihad-browser.css` | Scene chrome layout (relative units only). |
| `app/assistants/stage-assistant.js` | Card stage; pushes the `main` scene. |
| `app/assistants/main-assistant.js` | The browser: WebView, address bar, command menu, load state, errors. |
| `app/models/jihad-engine-override.js` | The per-variant plugin MIME swap. |
| `app/models/jihad-url.js` | URL-vs-search resolution (same rules as the other two variants). |
| `app/views/main/main-scene.html` | The `main` scene view template. |

## UI

- Address `TextField` (Enter navigates; a URL loads, anything else searches, using the
  same `looksLikeHost` rules as `../app/source/URLSearch.js`).
- `Mojo.Menu.commandMenu` with back / forward / stop-or-reload; back and forward are
  disabled until the engine reports history state, and the third button shows **stop**
  while a load is in flight and **reload** otherwise.
- `ProgressBar`, shown only during a load.
- A title row and the address field reflect the title and committed URL the engine
  reports.
- A failed load shows an error panel with the reason and a **Try Again** button rather
  than leaving a blank card.
- A page the engine creates for a `target=_blank` / `window.open` link is pushed as
  another `main` scene bound to that page identifier, so the back gesture returns to
  the opener.

Deliberately out of scope (cavekit-mojo-ui.md "Out of Scope"): bookmarks, history,
downloads and preferences views. JS `alert`/`confirm`/`prompt`/auth/SSL dialogs need
no code here — `Mojo.Widget.WebView` presents them itself from the framework's own
system templates.

**This variant therefore has NO db8 layer, deliberately** — no `db/` directory, no
kinds, no permissions, and no `com.palm.db` call anywhere in `app/` (grep-verified).
The other two variants each declare, ship and own their own kinds under their own app
id (review F-1 / cavekit-device-build.md **R7**); this one owns nothing because it
persists nothing. It used to appear in `../app/db/permissions/*` as a granted caller
on the ENYO variant's kinds — a cross-variant grant for a data layer it never used.
That grant is gone. If a Mojo front-end ever grows persistence it gets its own
`app-mojo/db/` with `net.riverstonerelay.jihad-browser-mojo.*` owned by this app id —
never a grant on another variant's kinds.

## Build

`../build/webos-oe/recipes-jihad/jihad-ui/net.riverstonerelay.jihad-browser-mojo_1.0.bb`
builds this into a self-contained `.ipk` via `jihad-app.inc`, which also installs the
composite `LICENSE` + `NOTICE` + `licenses/` into the package, exactly as it does for
the other two variants.
