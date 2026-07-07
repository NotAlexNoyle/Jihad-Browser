---
created: "2026-06-30"
last_edited: "2026-07-04"
---

# Cavekit: UI Shell (Enyo variant)

## Scope
The webOS front-end application forked from isis-browser (**Enyo 1.0**) and
rebranded as Jihad Browser, in `app/`. Covers app packaging/branding and the
application's use of the browser engine **through the existing BrowserAdapter
contract only**. The UI must not change in any engine-specific way — swapping the
engine is invisible to it.

This is the **Enyo (legacy) variant**. A parallel **Mochi / Enyo-2** front-end is
specified in cavekit-mochi-ui.md and ships as a separate `.ipk`; both variants
share this contract and both are built (two versions for the TouchPad).

## Requirements

### R1: Rebranded webOS application package
**Description:** The forked app is identified and presented as Jihad Browser, distinct from the stock webOS browser.
**Acceptance Criteria:**
- [x] `app/appinfo.json` declares a Jihad app id (`net.riverstonerelay.jihad-browser`), title "Jihad", and a non-HP vendor.
- [x] No remaining references to the stock id `com.palm.app.browser` inside `app/`.
- [x] App icon and splash icon are present and load.
- [x] `universalSearch` self-reference uses the new app id.
**Dependencies:** none

### R2: Engine access only via the unchanged BrowserAdapter contract
**Description:** The UI talks to the renderer exclusively through the same calls upstream isis-browser used.
**Acceptance Criteria:**
- [x] The set of `callBrowserAdapter(...)` method names used by the UI equals the upstream isis-browser set (no additions, removals, or renames).
- [x] `PalmServiceBridge` calls target the same `palm://com.palm.browserServer/*` URIs as upstream.
- [x] No engine-specific (Goanna/UXP) identifiers appear anywhere in `app/`.
**Dependencies:** cavekit-ipc-contract.md (R1, R5)

### R3: Apache-2.0 provenance preserved on forked UI files
**Description:** Forking the UI does not alter its licensing.
**Acceptance Criteria:**
- [x] Every forked source file retains its original HP Apache-2.0 header unmodified.
- [x] New UI files (if any) carry an Apache-2.0 header.
**Dependencies:** cavekit-licensing-branding.md (R1)

### R4: UI drives the new render daemon unchanged
**Description:** With the Goanna-backed daemon running, the UI behaves as the browser UI did with QtWebKit.
**Acceptance Criteria:**
- [ ] App launches to its start page.
- [ ] Entering a URL in the address bar issues an `openUrl` through the adapter and the page is requested.
- [ ] Back/forward/reload/stop controls invoke the corresponding adapter calls.
- [ ] Find-in-page issues `findInPage`.
**Dependencies:** cavekit-navigation-events.md, cavekit-ipc-contract.md

## Out of Scope
- Any rendering, IPC transport, or engine concern (other domains).
- Redesigning the UI/UX — this is a fork that preserves the isis browser UX.
- New webOS DB kinds for bookmarks/history (reuse existing schemas; coexistence-vs-replacement of the stock browser is a packaging decision in cavekit-device-build.md).

## Cross-References
- See also: cavekit-mochi-ui.md (the Enyo-2/Mochi sibling variant), cavekit-ipc-contract.md, cavekit-navigation-events.md, cavekit-licensing-branding.md

## Changelog
- 2026-06-30: Initial draft.
- 2026-07-04: Reconciled — R1 rebrand, R2 unchanged adapter contract (method set + Luna URIs = upstream), R3 Apache headers all verified. R4 is proven contract-correct against the STOCK QtWebKit BrowserServer on-device (openURL->loadStarted->pageTitleChanged->documentLoadFinished); driving the JIHAD Goanna daemon on-device is pending (needs the LunaService daemon + real BrowserAdapter).
