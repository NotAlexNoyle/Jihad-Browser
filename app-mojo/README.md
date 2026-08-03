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
| `app/assistants/history-assistant.js` | The `history` scene: the visited-page list, tap to open, clear. |
| `app/models/jihad-engine-override.js` | The per-variant plugin MIME swap. |
| `app/models/jihad-url.js` | URL-vs-search resolution (same rules as the other two variants). |
| `app/models/jihad-history.js` | This variant's history store (card-local, capped, de-duplicated). |
| `app/views/main/main-scene.html` | The `main` scene view template. |
| `app/views/history/*.html` | The `history` scene, list container, row and empty templates. |
| `images/menu-icon-*.png` | Command-menu icons the framework has no glyph for (32×64 sprites). |

## UI

- Address `TextField` (Enter navigates; a URL loads, anything else searches, using the
  same `looksLikeHost` rules as `../app/source/URLSearch.js`).
- `Mojo.Menu.commandMenu` with back / forward / stop-or-reload / **new card / history /
  share**. Back and forward are disabled until the engine reports history state, the
  third button shows **stop** while a load is in flight and **reload** otherwise, and
  share is disabled on the start page. New card opens a real second card (another Mojo
  *stage*); share hands the page to the mail app with the same launch id and parameters
  as the Enyo shell's share dialog. The framework has no glyph for history or share, so
  those two carry an app-shipped icon via the menu item's `iconPath` — **32×64 two-frame
  sprites** (soft-white normal frame on top, opaque pressed frame below), because a
  32×32 image renders as a cropped, off-centre glyph.
- `ProgressBar`, shown only during a load.
- The address field reflects the committed URL. There is no separate title row: the
  card's own title bar and the address field already carry it, so a third line of the
  same text was removed.
- The start page carries the bundled logo, "Jihad Browser", the engine line and the
  same hint as the other two variants — the three shells open identically.
- A failed load shows an error panel with the reason and a **Try Again** button rather
  than leaving a blank card.
- A page the engine creates for a `target=_blank` / `window.open` link is pushed as
  another `main` scene bound to that page identifier, so the back gesture returns to
  the opener.
- `<select>` dropdowns need **no code here**: `Mojo.Widget.WebView` already implements
  the adapter's `showPopupMenu` callback and answers with `selectPopupMenuItem`. All it
  ever needed was the daemon emitting the isis option-list shape, which it now does.

Deliberately out of scope (cavekit-mojo-ui.md "Out of Scope"): bookmarks, downloads and
preferences views. JS `alert`/`confirm`/`prompt`/auth/SSL dialogs need no code here —
`Mojo.Widget.WebView` presents them itself from the framework's own system templates.

**This variant has NO db8 layer, deliberately** — no `db/` directory, no kinds, no
permissions, and no `com.palm.db` call anywhere in `app/` (grep-verified). The other two
variants each declare, ship and own their own kinds under their own app id (review F-1 /
cavekit-device-build.md **R7**); this one owns none. It used to appear in
`../app/db/permissions/*` as a granted caller on the ENYO variant's kinds — a
cross-variant grant for a data layer it never used. That grant is gone.

Its **history** (added 2026-08-03) therefore lives in the card's own storage
(`app/models/jihad-history.js`): one JSON array, newest first, capped and de-duplicated
by url, written nowhere but this app's own storage and visible to no other variant. That
keeps the R7/R8 footprint contract intact without introducing kinds. If this front-end
ever needs shared or larger persistence it gets its own `app-mojo/db/` with
`net.riverstonerelay.jihad-browser-mojo.*` kinds owned by this app id — never a grant on
another variant's kinds.

## Stylesheet constraint

`stylesheets/jihad-browser.css` is read by the **card** WebKit (~534.x), which silently
ignores unprefixed `box-sizing` and modern flexbox. Write `-webkit-box-sizing` (and the
`-webkit-box` flexbox syntax) or the declaration is dropped and padding is added to a
`width:100%` box: that is exactly why the toolbar once measured 784 px on a 768 px
screen. `start.html` is rendered by **our own Goanna engine** and needs no prefixes.

## Build

`../build/webos-oe/recipes-jihad/jihad-ui/net.riverstonerelay.jihad-browser-mojo_1.0.bb`
builds this into a self-contained `.ipk` via `jihad-app.inc`, which also installs the
composite `LICENSE` + `NOTICE` + `licenses/` into the package, exactly as it does for
the other two variants.
