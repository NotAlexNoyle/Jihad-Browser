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

**db8 kinds are owned by the Enyo variant.** Bookmarks/history/preferences use
`net.riverstonerelay.jihad-browser.{bookmarks,history,preferences}:1`, which are
registered — and whose db8 permissions are granted — by `../app/db/`, to the
caller `net.riverstonerelay.jihad-browser` only. See the "Audit 2026-07-31"
section of `../context/impl/impl-mochi.md` (finding F-A01) before relying on
persistence from this package.

## Contract invariant

The Mochi UI uses the identical `callBrowserAdapter(...)` method set and
`palm://com.palm.browserServer/*` URIs as `../app`. The new UI-side piece is
`source/JihadWebView.js` — an Enyo-2 `WebView`-equivalent control that renders
the BrowserAdapter NPAPI `<object>` with the self-contained MIME
`application/x-jihad-browser` (matching `../app/source/JihadEngineOverride.js`)
and exposes the same adapter method surface (see cavekit-mochi-ui.md R3). See
`../docs/IPC-CONTRACT.md`.
