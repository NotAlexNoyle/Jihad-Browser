---
created: "2026-06-30"
last_edited: "2026-07-04"
---

# Cavekit: Mochi UI Variant

## Scope
A second browser front-end (directory `app-mochi/`) implemented in **Enyo 2 + the
Mochi UI library**, functionally equivalent to the legacy Enyo-1.0 UI
(cavekit-ui-shell.md) and driving the **same** BrowserAdapter contract. It ships
as a separate `.ipk` so both UIs can coexist on the device. The rendering engine
and IPC are unchanged — this domain is purely an alternative presentation layer.

Reference: `app-mochi/README.md`, the Mochi library at `../mochi`, the Enyo-2
sampler at `../mochi-sampler`, `docs/IPC-CONTRACT.md`.

## Requirements

### R1: Separate, coexisting Mochi application package
**Description:** The Mochi UI is packaged as its own webOS app, installable alongside the Enyo variant.
**Acceptance Criteria:**
- [x] `app-mochi/appinfo.json` declares a distinct app id (`net.riverstonerelay.jihad-browser.mochi`) and title ("Jihad (Mochi)"). *(2026-07-18, T-049: title reconciled to kit value.)*
- [~] Installing it does not collide with or replace the Enyo variant (`net.riverstonerelay.jihad-browser`); both can be installed at once. *(Structural side verified 2026-07-18 from the ipk: distinct package name, no services.json. Actual dual install DEVICE-GATED — device offline.)*
- [x] Uses the Jihad Browser icon set. *(md5-identical to `app/icon*.png`, verified 2026-07-18.)*
**Dependencies:** none

### R2: Feature parity with the Enyo UI
**Description:** The Mochi UI offers the same user-facing browser features as the Enyo-1.0 UI.
**Acceptance Criteria:**
- [ ] Address/search bar with navigation (back, forward, reload, stop) works.
- [ ] Bookmarks, history, and downloads views are present and functional.
- [ ] Find-in-page, preferences, and the start page are present.
- [ ] Alert/confirm/prompt/auth and SSL-confirm dialogs are presented and answerable.
- [ ] A parity checklist against `../app` source shows no missing user-facing feature (or documents intentional omissions). [human-review]
**Dependencies:** cavekit-navigation-events.md, cavekit-browser-services.md

### R3: Drives the unchanged BrowserAdapter contract
**Description:** The Mochi UI talks to the engine only through the existing contract; no engine/IPC change.
**Acceptance Criteria:**
- [x] An Enyo-2 WebView-equivalent control binds to the same BrowserAdapter NPAPI plugin the Enyo UI uses. *(2026-07-19, T-051: `app-mochi/source/JihadWebView.js` renders `<object type="application/x-jihad-browser">` (self-contained MIME per JihadEngineOverride.js), `node.eventListener=this`, callback arg orders verified against render/adapter/BrowserAdapter.cpp. Live daemon handshake DEVICE-GATED.)*
- [x] The `callBrowserAdapter(...)` method set and `palm://com.palm.browserServer/*` URIs used are identical to the Enyo variant (no additions/renames). *(Verified 2026-07-19: set {findInPage, goBack, goForward, reloadPage, stopLoad} + URIs {clearCache, clearCookies} — diff empty both.)*
- [x] No Goanna/UXP-specific identifiers appear in `app-mochi/`. *(grep clean 2026-07-19.)*
**Dependencies:** cavekit-ipc-contract.md (R1, R5), cavekit-ui-shell.md (R2)

### R4: Built with Mochi controls; layout fits both TouchPad models
**Description:** The UI uses Mochi widgets and renders correctly on TouchPad and TouchPad Go.
**Acceptance Criteria:**
- [x] UI is composed from Mochi controls (e.g. Header, IconButton, Input, List, Panels, Popup, ProgressBar) rather than ad-hoc markup. *(2026-07-19, T-052: mochi.Header + FittableColumns toolbar (IconButton nav + InputDecorator/Input address), mochi.ProgressBar, mochi.Popup scaffolding.)*
- [~] Layout is usable on the TouchPad (Topaz) and TouchPad Go (Opal) screen. [human-review on device] *(Fittable/relative layout, shared 1024x768, no hardcoded px; stays [~] until on-device review.)*
- [x] Enyo 2 core + layout + Mochi are bundled into the package at build time (not vendored in this repo). *(2026-07-18, T-049: `build/webos-oe/build-mochi-ipk.sh` stages Enyo 2 core from `../mochi-sampler/enyo`, layout from `webos-stacks/mochi/lib/layout` (mochi-sampler's lib/ dirs are empty; overridable via `LAYOUT_SRC`), Mochi from `../mochi` → palm-package → 1.4 MB ipk, 394 entries; output git-ignored, nothing vendored.)*
**Dependencies:** cavekit-device-build.md (R3, R6)

### R5: Licensing and attribution
**Description:** The Mochi variant respects all licenses.
**Acceptance Criteria:**
- [x] New `app-mochi/` source files carry Apache-2.0 headers. *(2026-07-19, T-050.)*
- [x] Bundled Mochi (LG, Apache-2.0) and Enyo 2 are attributed in `NOTICE`. *(2026-07-19, T-050: Enyo 2 core + layout (LG 2012-2014) + Mochi (LG 2014) in NOTICE; confirmed inside packaged ipk — closes codex F-390.)*
**Dependencies:** cavekit-licensing-branding.md (R1, R2)

## Out of Scope
- Any change to the rendering engine, BrowserServer, or the YAP/Luna contract.
- New browser capabilities beyond parity with the Enyo UI (YAGNI).
- Replacing the Enyo variant — both ship.

## Cross-References
- See also: cavekit-ui-shell.md (Enyo variant), cavekit-ipc-contract.md, cavekit-navigation-events.md, cavekit-browser-services.md, cavekit-device-build.md, cavekit-licensing-branding.md

## Changelog
- 2026-06-30: Initial draft (added per request: second UI variant + .ipk).
- 2026-07-04: Status check — app-mochi/ is a skeleton (appinfo.json + index.html + source/ + icons, ~9 files); no requirement met yet. R1–R5 pending: the Mochi/Enyo-2 UI has not been built to parity. Largest remaining kit.
