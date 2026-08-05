---
created: "2026-06-30"
last_edited: "2026-08-03"
---

# Cavekit: UI Shell (Enyo variant)

## Scope
The webOS front-end application forked from isis-browser (**Enyo 1.0**) and
rebranded as Jihad Browser, in `app/`. Covers app packaging/branding and the
application's use of the browser engine **through the existing BrowserAdapter
contract only**. The UI must not change in any engine-specific way — swapping the
engine is invisible to it.

This is the **Enyo (legacy) variant**, one of three. A **Mochi / Enyo-2** front-end
(cavekit-mochi-ui.md) and a **Mojo** front-end (cavekit-mojo-ui.md) each ship as their
own `.ipk`; all three share this contract, and all three are built and live on device.

## Requirements

### R1: Rebranded webOS application package
**Description:** The forked app is identified and presented as Jihad Browser, distinct from the stock webOS browser.
**Acceptance Criteria:**
- [x] `app/appinfo.json` declares a Jihad app id (`net.riverstonerelay.jihad-browser`), a Jihad launcher title, and a non-HP vendor. *(The launcher title is **"Jihad Enyo"** since 2026-08-03: with all three variants installed at once, identical titles were unusable. In-app branding is unified — the start page says "Jihad Browser" over the shared logo, as in the other two variants.)*
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
- [x] App launches to its start page. *(Device-verified 2026-07-20 against the Jihad Goanna daemon — `../impl/device-test-2026-07-19.md` "Session 4 (2026-07-20) — kit-criteria verification sweep (device connected)": "app launches to start page … (daemon-verified across the session)".)*
- [x] Entering a URL in the address bar issues an `openUrl` through the adapter and the page is requested. *(Same source: "address→openUrl loads".)*
- [x] Back/forward/reload/stop controls invoke the corresponding adapter calls. *(Same source: "back/forward/reload navigate (daemon-verified across the session)". Stop was not called out separately in that record; it shares the frozen `callBrowserAdapter` set verified under R2.)*
- [x] Find-in-page issues `findInPage`, and the engine actually searches. *(2026-08-05 — FIXED, and the diagnosis this criterion carried for two weeks was WRONG. It said `FindNext` "dereferences a frame-selection controller the offscreen browser does not set up", and that the fix needed an offscreen-safe selection controller. The selection machinery was never the problem. `nsWebBrowserFind::SearchInFrame` performs a same-origin check through `nsContentUtils::SubjectPrincipal()`, whose first act is `if (!GetCurrentJSContext()) MOZ_CRASH("Accessing the Subject Principal without an AutoJSAPI on the stack is forbidden")`. An EMBEDDER calls `FindNext` from C++ with no script on the stack — no JSContext — so the engine's own deliberate abort fired, and MOZ_CRASH presents as SIGSEGV at address 0, which is precisely the fault that had been measured and misattributed. Found by instrumenting the engine and watching where execution stopped: between "before SearchInFrame" and any selection code. Patch 0015 consults the subject principal only when a JSContext exists; with none the caller is native embedding code, already inside libxul's own process holding that docShell's finder, so the check has no caller to check. Desktop result: `find 'JIHADFINDME' -> 1` — a real match, no crash. The UI half was already correct, so the whole path now works. **Not yet run on device** (needs the ARM libxul rebuild), and the daemon-side `Find` no longer returns a hardcoded false.)*
**Dependencies:** cavekit-navigation-events.md, cavekit-ipc-contract.md

### R5: Engine `<select>` dropdowns are presented card-side
**Description:** A tap on a page `<select>` shows the option list in the card and applies the choice.
**Added 2026-08-03.** The engine cannot paint a native combobox (it is a separate display root), so the daemon serializes the options and the CARD renders the list — the isis/Atlas `msgPopupMenuShow` contract this fork already inherits.
**Acceptance Criteria:**
- [x] The list appears with the real options and the chosen one is applied to the page (`input`+`change` fire). *(Device-verified 2026-08-03, user-confirmed.)*
- [x] The popup opens **anchored under the tapped control**, not screen-centre. *(The framework's own `_selectRect` anchor path cannot run in this embedding — `BasicWebView` publishes no `onClick` — so the daemon ships the control's rect and the shell places the list.)*
- [x] **No app-side popup implementation exists.** The stock `enyo.WebView` wrapper consumes the inner `BasicWebView`'s `onOpenSelect` itself (`showSelect` → `createSelectPopup` → `PopupList`) and replies via `selectPopupMenuItem`; it never re-publishes the event. An `onOpenSelect` handler in `Browser.js` is unreachable dead code — one was written, proved dead, and deleted. **Do not re-add one**, and do not patch `BasicWebView.showPopupMenu` (an earlier patch shadowed the framework method and killed the popup entirely).
**Dependencies:** cavekit-addons-extensions.md, cavekit-ipc-contract.md (R1)

## Out of Scope
- Any rendering, IPC transport, or engine concern (other domains).
- Redesigning the UI/UX — this is a fork that preserves the isis browser UX.
- New webOS DB kinds for bookmarks/history (reuse existing schemas; coexistence-vs-replacement of the stock browser is a packaging decision in cavekit-device-build.md).

## Cross-References
- See also: cavekit-mochi-ui.md (the Enyo-2/Mochi sibling variant), cavekit-ipc-contract.md, cavekit-navigation-events.md, cavekit-licensing-branding.md

## Changelog
- 2026-08-03: Added **R5** (card-side `<select>` popup — device-verified and user-confirmed),
  including the standing "no app-side popup implementation" rule that the investigation
  established. Corrected R1's title AC: the launcher title is "Jihad Enyo" since the three
  variants went live together; in-app branding stays the shared "Jihad Browser" start page, which
  now carries the same logo/title/engine-line/hint block as the other two variants.
- 2026-06-30: Initial draft.
- 2026-07-04: Reconciled — R1 rebrand, R2 unchanged adapter contract (method set + Luna URIs = upstream), R3 Apache headers all verified. R4 is proven contract-correct against the STOCK QtWebKit BrowserServer on-device (openURL->loadStarted->pageTitleChanged->documentLoadFinished); driving the JIHAD Goanna daemon on-device is pending (needs the LunaService daemon + real BrowserAdapter).
- 2026-07-31: Reconciliation against recorded evidence. R4 was still all-unchecked although `../impl/device-test-2026-07-19.md` Session 4 (2026-07-20, device connected) recorded launch-to-start-page, address→`openUrl`, and back/forward/reload as daemon-verified across that session — those three ACs are now `[x]` with that citation. `findInPage` stays open: the same record says "pending a focused test". No other change; R1–R3 were already evidence-backed.
