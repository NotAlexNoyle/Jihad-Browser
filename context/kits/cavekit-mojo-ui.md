---
created: "2026-07-31"
last_edited: "2026-07-31"
---

# Cavekit: Mojo UI Variant

## Scope
A third browser front-end (directory `app-mojo/`) implemented on **Mojo**, the original
(pre-Enyo) webOS application framework that ships with webOS 3.0.5 at
`/usr/palm/frameworks/mojo`. It is functionally a browser in its own right — not a
demonstration — and drives the **same** BrowserAdapter contract as the Enyo-1.0 shell
(cavekit-ui-shell.md) and the Enyo-2/Mochi shell (cavekit-mochi-ui.md).

It ships as its own `.ipk` (`net.riverstonerelay.jihad-browser.mojo`) and, per
cavekit-device-build.md **R7**, is a fully independent package: its own NPAPI MIME type,
adapter shim, adapter impl, YAP service name, socket, upstart job, and daemon process.
Installing or removing it has no effect on the Enyo or Mochi variants.

This domain is a presentation layer only — the engine, the daemon, and the YAP/Luna
contract are unchanged.

Reference: `app-mojo/README.md`, `docs/IPC-CONTRACT.md`, the Mojo framework docs
(`webos://knowledge/mojo`), the Enyo-1.0 shell in `app/` as the behavioral baseline.

## Requirements

### R1: Separate, coexisting Mojo application package
**Description:** The Mojo UI is packaged as its own webOS app, installable alongside both other variants.
**Acceptance Criteria:**
- [x] `app-mojo/appinfo.json` declares a distinct app id (`net.riverstonerelay.jihad-browser.mojo`) and the Jihad Browser title/icon set.
- [ ] Installing it does not collide with, overwrite, or replace either the Enyo (`net.riverstonerelay.jihad-browser`) or Mochi (`…​.mochi`) variant; all three can be installed at once and all three launch.
- [ ] Removing it leaves the other two fully functional (launch + load a page).
**Dependencies:** cavekit-device-build.md (R7)

### R2: Working browser front-end on Mojo
**Description:** The Mojo variant is a usable browser, not a scaffold.
**Acceptance Criteria:**
- [ ] A Mojo scene hosts a web-render surface bound to this variant's NPAPI MIME type and displays a real rendered page.
- [ ] Address entry navigates: typing a URL (or a search term) loads the corresponding page.
- [ ] Back, forward, reload, and stop are present and drive the adapter.
- [ ] Load state is visible to the user (progress indication + a stop/reload affordance that reflects whether a load is in flight).
- [ ] The page title and the committed URL are reflected in the UI as the engine reports them.
- [ ] A failed load surfaces an error to the user rather than leaving a blank card.
**Dependencies:** cavekit-navigation-events.md, cavekit-ipc-contract.md (R1)

### R3: Drives the unchanged BrowserAdapter contract
**Description:** The Mojo UI talks to the engine only through the existing contract; no engine or IPC change.
**Acceptance Criteria:**
- [ ] The `callBrowserAdapter(...)` method set and the `palm://com.palm.browserServer/*` URIs it uses are a subset of the set the Enyo variant uses — no additions, no renames.
- [ ] The render surface is created with THIS variant's MIME type only; it never loads another variant's adapter or the stock `application/x-palm-browser`.
- [ ] No Goanna/UXP-specific identifiers appear in `app-mojo/`.
**Dependencies:** cavekit-ipc-contract.md (R1, R5), cavekit-ui-shell.md (R2)

### R4: Built with Mojo framework idiom; layout fits both TouchPad models
**Description:** The UI is a real Mojo app, using the framework's own structures.
**Acceptance Criteria:**
- [ ] The app is composed from Mojo's own constructs (stage + scene assistants, `sources.json` manifest, Mojo widgets and view templates) rather than framework-agnostic markup with hand-rolled behavior.
- [ ] It runs against the device's system Mojo framework rather than bundling a copy of it.
- [ ] Layout is usable on the TouchPad (Topaz) and TouchPad Go (Opal) screens. [human-review on device]
**Dependencies:** cavekit-device-build.md (R3, R6)

### R5: Licensing and attribution
**Description:** The Mojo variant respects all licenses.
**Acceptance Criteria:**
- [ ] New `app-mojo/` source files carry Apache-2.0 headers.
- [ ] The package ships the composite `LICENSE` + `NOTICE` (it bundles the MPL-2.0 engine like the other variants).
**Dependencies:** cavekit-licensing-branding.md (R1, R2)

## Out of Scope
- Any change to the rendering engine, BrowserServer, or the YAP/Luna contract.
- Feature parity with the Enyo shell's long tail (bookmarks/history/downloads/preferences views).
  R2 defines the bar: a working browser. Anything beyond it is YAGNI until asked for.
- Replacing either other variant — all three ship.

## Cross-References
- See also: cavekit-ui-shell.md (Enyo 1.0 variant), cavekit-mochi-ui.md (Enyo 2/Mochi variant),
  cavekit-ipc-contract.md, cavekit-navigation-events.md, cavekit-device-build.md (R7 independence,
  R8 install footprint), cavekit-licensing-branding.md

## Changelog
- 2026-07-31: Initial draft. Created when the user required all three front-ends (Enyo, Mochi, Mojo)
  to work standalone; `app-mojo/` had been a documented skeleton tracked only as one acceptance
  criterion under cavekit-device-build.md R3.
