# app-mochi — Mochi (Enyo 2) UI variant

A second Jihad Browser front-end, built on **Enyo 2 + the Mochi UI library**
(LG's touch-focused, never-officially-released Enyo 2 widget set; Apache-2.0).
It is functionally equivalent to the legacy Enyo-1.0 UI in `../app`, and uses
Mochi controls (`Input`, `InputDecorator`, `ProgressBar`, `ToggleButton`, …)
over a Fittable layout.

> **`mochi.Popup` is UNUSABLE here** — a floating/modal Popup crashes the app
> card on this engine (device, 2026-07-20). Every menu and dialog is a plain
> `enyo.Control` overlay (a scrim + a box toggled by `showing`). Likewise the
> engine renders no mochi sprite icons, CSS-background SVGs, CSS circles, or the
> needed font glyphs, so toolbar icons are PNG data URIs. Do not "modernise"
> these back — see `PARITY.md` and `../context/impl/impl-mochi.md`.

This ships as a **separate `.ipk`** (`net.riverstonerelay.jihad-browser.mochi`)
so it can coexist with the Enyo variant on the device — two versions for the
TouchPad. Both UIs drive the **same** BrowserServer through the **unchanged**
BrowserAdapter contract.

> Status: **parity port landed, device verification pending**. `source/`
> implements the shell (NPAPI-bound `JihadWebView`, toolbar, load progress,
> app-chrome start page) plus the parity views — bookmarks, history, downloads,
> find-in-page, preferences — and the engine-driven dialog set (alert / confirm /
> prompt / auth / SSL). Feature-by-feature status, including every intentional
> omission, is in `PARITY.md`; requirements are in
> `../context/kits/cavekit-mochi-ui.md`.

## Why two UIs

- `../app` — the proven isis Enyo-1.0 UI (lowest risk; what isis-browser ships).
- `app-mochi` — a modern Enyo-2/Mochi UI (nicer touch UX), the same browser
  underneath.

## Framework stack (bundled at build time, not vendored here)

- **Enyo 2.x** core (`enyo/`) — from the enyo 2 framework.
- **layout** library (`lib/layout/`) — FittableRows/Columns, Panels, List.
- **Mochi** (`lib/mochi/`) — from `../../mochi` (Apache-2.0, LG).

## Packaging

`build/webos-oe/build-mochi-ipk.sh` builds the `.ipk`. It stages this directory
(`appinfo.json`, `index.html`, icons, `source/`) together with the three
external framework trees above into a layout matching `index.html`'s
`<script src>` paths, then runs `palm-package`. The frameworks are pulled from
outside the repo at build time (never committed); the staging tree and the
`.ipk` land under `build/webos-oe/out-mochi/` (git-ignored). Source roots are
overridable via `ENYO_SRC` / `LAYOUT_SRC` / `MOCHI_SRC`; the layout library is
resolved from the first candidate that actually contains `package.js`.

The app id is `net.riverstonerelay.jihad-browser.mochi` — distinct from the Enyo
variant (`net.riverstonerelay.jihad-browser`), so both `.ipk`s install and
coexist. The launcher **title is "Jihad Browser"** for BOTH variants (user
directive, cavekit-mochi-ui.md R1 — never "Jihad (Mochi)"/"Jihad (Enyo)"); the
distinct app id is what keeps them apart. Icons are the shared Jihad Browser set
(identical bytes to `../app/icon*.png`).

**This package owns its own db8 kinds.** Bookmarks/history/preferences use
`net.riverstonerelay.jihad-browser.mochi.{bookmarks,history,preferences}:1`,
declared in `db/{kinds,permissions}/` HERE, owned by this app id, and registered
by the appinstaller out of this `.ipk`. Nothing is shared with the Enyo variant.

Until review F-1 they were the ENYO variant's kinds, shipped only in that package
with a permission grant extended to this app id — so installing Mochi alone
registered no kinds at all and every db8 call failed, and removing the Enyo
package destroyed this variant's data. That is the co-ownership
cavekit-device-build.md **R7** forbids. A db8 kind's `owner` must equal the app id
that registers it, so independence means a separate namespace, not a broader
grant; there are deliberately no cross-variant permissions. The consequence is by
design: **history and bookmarks are per variant** — this browser does not see the
Enyo browser's history, exactly as it does not share its engine profile.

After a dev `palm-install` (which does not run the appinstaller's kind
registration) run `build/webos-oe/register-db-kinds.sh mochi` once.

## Contract invariant

The Mochi UI uses the identical `callBrowserAdapter(...)` method set and
`palm://com.palm.browserServer/*` URIs as `../app`. The new UI-side piece is
`source/JihadWebView.js` — an Enyo-2 `WebView`-equivalent control that renders
the BrowserAdapter NPAPI `<object>` with **this variant's own** self-contained
MIME `application/x-jihad-browser-mochi` (the Enyo variant keeps
`application/x-jihad-browser`; see `../context/plans/plan-variant-identity.md`)
and exposes the same adapter method surface (see cavekit-mochi-ui.md R3). See
`../docs/IPC-CONTRACT.md`.
