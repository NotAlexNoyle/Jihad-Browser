# app-mochi — Mochi (Enyo 2) UI variant

A second Jihad Browser front-end, built on **Enyo 2 + the Mochi UI library**
(LG's touch-focused, never-officially-released Enyo 2 widget set; Apache-2.0).
It is functionally equivalent to the legacy Enyo-1.0 UI in `../app`, but uses
modern Mochi controls (`Header`, `IconButton`, `Input`, `List`, `Panels`,
`Popup`, `ProgressBar`, …).

This ships as a **separate `.ipk`** (`net.riverstonerelay.jihad-browser.mochi`,
"Jihad (Mochi)") so it can coexist with the Enyo variant on the device — two
versions for the TouchPad. Both UIs drive the **same** Goanna-backed
BrowserServer through the **unchanged** BrowserAdapter contract.

> Status: **scaffold**. `index.html` + `source/` sketch the structure; the full
> port to parity with `../app` is specified in
> `../context/kits/cavekit-mochi-ui.md` and tracked in the build site.

## Why two UIs

- `../app` — the proven isis Enyo-1.0 UI (lowest risk; what isis-browser ships).
- `app-mochi` — a modern Enyo-2/Mochi UI (nicer touch UX), the same browser
  underneath.

## Framework stack (bundled at build time, not vendored here)

- **Enyo 2.x** core (`enyo/`) — from the enyo 2 framework.
- **layout** library (`lib/layout/`) — FittableRows/Columns, Panels, List.
- **Mochi** (`lib/mochi/`) — from `../../mochi` (Apache-2.0, LG).

## Contract invariant

The Mochi UI must use the identical `callBrowserAdapter(...)` method set and
`palm://com.palm.browserServer/*` URIs as `../app`. The only new UI-side piece
is an Enyo-2 `WebView`-equivalent control that binds to the same BrowserAdapter
NPAPI plugin (port task — see cavekit-mochi-ui.md R3). See
`../docs/IPC-CONTRACT.md`.
