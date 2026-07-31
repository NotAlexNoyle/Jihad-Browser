---
created: "2026-06-30"
last_edited: "2026-07-31"
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
- [x] `app-mochi/appinfo.json` declares a distinct app id (`net.riverstonerelay.jihad-browser.mochi`) and title ("Jihad Browser"). *(2026-07-18, T-049. 2026-07-20: title is "Jihad Browser" — per user directive BOTH variants are titled "Jihad Browser" (never "Jihad (Mochi)"/"Jihad (Enyo)"); the distinct app id keeps them coexisting. Confirmed on-device: the card status bar reads "Jihad Browser".)*
- [x] Installing it does not collide with or replace the Enyo variant (`net.riverstonerelay.jihad-browser`); both can be installed at once. *(VERIFIED ON DEVICE 2026-07-19: `palm-install` of the Mochi ipk succeeded with the Enyo variant present; `palm-install -l` lists both `net.riverstonerelay.jihad-browser 1.0.0` and `net.riverstonerelay.jihad-browser.mochi 1.0.0`.)*
- [x] Uses the Jihad Browser icon set. *(md5-identical to `app/icon*.png`, verified 2026-07-18.)*
**Dependencies:** none

### R2: Feature parity with the Enyo UI
**Description:** The Mochi UI offers the same user-facing browser features as the Enyo-1.0 UI.
**Acceptance Criteria:**
- [~] Address/search bar with navigation (back, forward, reload, stop) works. *(2026-07-31 reconciliation: `../../app-mochi/PARITY.md` "R2 acceptance summary" records this as **met (shell)** — the toolbar Input + `goBack/goForward/reloadPage/stopLoad` route through the frozen `callBrowserAdapter` set (T-051/T-052, commit 2a79d71). Stays `[~]`: PARITY.md states on-device FUNCTIONAL verification (live adapter round-trips) is **DEVICE-GATED and pending hardware**; only the structural side — `node --check`, the frozen method set, and the `build-mochi-ipk.sh` end-to-end build — is verified.)*
- [~] Bookmarks, history, and downloads views are present and functional. *(2026-07-31 reconciliation: PARITY.md "R2 acceptance summary" records **met**, with downloads auto-initiation listed as an intentional omission and list/open/cancel/clear functional against the download manager. Views built by commits 9a8997c/9418599 (`JihadBookmarkList.js`, `JihadHistoryList.js`, `JihadDownloadList.js` + the Enyo-2 Luna-service helper) and wired into the shell by a244eed. Stays `[~]` on the same device gate — live db8/download-manager round-trips are unverified on hardware.)*
- [~] Find-in-page, preferences, and the start page are present. *(2026-07-31 reconciliation: PARITY.md "R2 acceptance summary" records **met** — `JihadFindBar.js` + `JihadPreferences.js` (commit dbeafc6) and the app-chrome start page (09c050c, device-screenshot-confirmed 2026-07-19 per `../impl/device-test-2026-07-19.md` Session 2). Stays `[~]`: PARITY.md documents that Preferences persists toggles to db8 but does **not** apply them to the live engine (intentional contract omission — applying would widen the frozen `callBrowserAdapter` set), and find/prefs are not device-functional-verified.)*
- [~] Alert/confirm/prompt/auth and SSL-confirm dialogs are presented and answerable. *(2026-07-20: implemented in `JihadDialogs.js` + the overflow menu / generic dialog in `JihadBrowser.js`, but NOT as `mochi.Popup` — a floating/modal `mochi.Popup` CRASHES the Mochi card on this engine (pressing Share crashed the app). Converted to plain Control overlays (a scrim + centered box toggled by `showing`, `jihad-btn` divs instead of `mochi.Button`) — the same pattern the parity list-views use. The share crash is gone and the card opens cleanly (verified on-device 2026-07-20). Dialog PRESENTATION/answering still needs on-device verification with a page that raises alert/confirm.)*
- [~] A parity checklist against `../app` source shows no missing user-facing feature (or documents intentional omissions). [human-review] *(2026-07-31 reconciliation: the checklist EXISTS and is complete — `../../app-mochi/PARITY.md` (commit 0d2fb82) maps every user-facing Enyo-1.0 feature to Ported / Simplified / Omitted with a rationale per omission (bookmark-edit dialog, thumbnails, download auto-initiation + retry, context menu, share sheet, add-to-launcher, print, cert detail viewer, search-engine selector, live-engine pref application). Stays `[~]` because the `[human-review]` sign-off on that checklist is not recorded anywhere.)*
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
- [x] UI is composed from Mochi controls (e.g. Header, IconButton, Input, List, Panels, ProgressBar — NOT Popup, which crashes on this engine; see the deviation note) rather than ad-hoc markup. *(2026-07-19, T-052: mochi.Header + FittableColumns toolbar (IconButton nav + InputDecorator/Input address), mochi.ProgressBar. 2026-07-20 DEVIATION: `mochi.Popup` is NOT usable — a floating/modal Popup crashes the card on this engine (Goanna/ESR52 host). The overflow menu + all dialogs use plain Control overlays instead (scrim + box toggled by `showing`); `mochi.Input`/`Header`/`ProgressBar` etc. are fine. Documented in PARITY.md + [[jihad-input-activation-and-tiling]].)*
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
- 2026-07-31: Reconciliation against recorded evidence. R2's first three ACs moved `[ ]`→`[~]`: `app-mochi/PARITY.md` ("R2 acceptance summary", commit 0d2fb82) records them as structurally met — views built (a244eed, 9a8997c, 9418599, dbeafc6), contract-clean against the frozen `callBrowserAdapter` set, ipk builds — while the same document states on-device functional verification is DEVICE-GATED and pending hardware; NOT marked `[x]`. The parity-checklist AC also moved to `[~]` (the checklist exists and documents every omission; the `[human-review]` sign-off is unrecorded). R1/R3/R5 were already `[x]` on cited evidence (R1 dual-install VERIFIED ON DEVICE 2026-07-19); no status was raised without a citation.
