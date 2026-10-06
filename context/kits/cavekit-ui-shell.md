---
created: "2026-06-30"
last_edited: "2026-10-06"
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
- [x] `PalmServiceBridge` calls target the same `palm://com.palm.browserServer/*` URIs as upstream. *(**AMENDED 2026-10-01 — the criterion as worded has been deliberately untrue since 2026-08-04, and the kit never said so.** `clearCookies` / `clearCache` go to this variant's OWN service, `palm://net.riverstonerelay.jihadBrowser/…` (`app/source/Browser.js`), because the upstream URI is the STOCK daemon and calling it clears the wrong browser — cavekit-ipc-contract.md R4 owns that decision. The card also subscribes to that service's `notifications` method for toasts (cavekit-gre-widgets.md R5). What this criterion now requires: the METHOD names and payloads are upstream's; only the service NAME differs, and it is this variant's own.)*
- [x] No engine-specific (Goanna/UXP) identifiers appear anywhere in `app/`. *(**AMENDED 2026-10-01:** a grep is no longer literally clean — the start page prints an engine line, "Enyo UI ★ Goanna/6.9 UXP/…" (`app/source/StartPage.js`). That is user-facing branding text, not an API identifier, and the Mojo kit has carried the same carve-out since it was written (cavekit-mojo-ui.md R3). The criterion is about the card not depending on the engine; it does not.)*
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
- [x] Find-in-page issues `findInPage`, and the engine actually searches. *(2026-08-05 — FIXED, and the diagnosis this criterion carried for two weeks was WRONG. It said `FindNext` "dereferences a frame-selection controller the offscreen browser does not set up", and that the fix needed an offscreen-safe selection controller. The selection machinery was never the problem. `nsWebBrowserFind::SearchInFrame` performs a same-origin check through `nsContentUtils::SubjectPrincipal()`, whose first act is `if (!GetCurrentJSContext()) MOZ_CRASH("Accessing the Subject Principal without an AutoJSAPI on the stack is forbidden")`. An EMBEDDER calls `FindNext` from C++ with no script on the stack — no JSContext — so the engine's own deliberate abort fired, and MOZ_CRASH presents as SIGSEGV at address 0, which is precisely the fault that had been measured and misattributed. Found by instrumenting the engine and watching where execution stopped: between "before SearchInFrame" and any selection code. Patch 0015 consults the subject principal only when a JSContext exists; with none the caller is native embedding code, already inside libxul's own process holding that docShell's finder, so the check has no caller to check. Desktop result: `find 'JIHADFINDME' -> 1` — a real match, no crash. The UI half was already correct, so the whole path now works. **DEVICE-VERIFIED the same day**, after the ARM libxul rebuild: on a page containing the token, `find 'JIHADFINDME' -> 1`, and on a token that is absent, `find 'NOTPRESENTXYZ' -> 0` — a positive and a negative, with the daemon alive and no fault report either side. The daemon-side `Find` no longer returns a hardcoded false.)* *(2026-10-01 (third session) — the hit was INVISIBLE on screen in all three shells, fixed by pref. The engine found the word (`find 'FINDTOKENQ7' -> 1`, `-> 0` for an absent one) and drew it as a solid white box with the text gone: with no native theme the stand-in palette answers white for every text-selection colour, so selected text was white on white. `packaging/prefs/jihad-platform-prefs.js` now sets `ui.textSelectBackground` / `Foreground` and the attention and highlight colours; screenshot after: the hit is readable.)*
**Dependencies:** cavekit-navigation-events.md, cavekit-ipc-contract.md

### R5: Engine `<select>` dropdowns are presented card-side
**Description:** A tap on a page `<select>` shows the option list in the card and applies the choice.
**Added 2026-08-03.** The engine cannot paint a native combobox (it is a separate display root), so the daemon serializes the options and the CARD renders the list — the isis/Atlas `msgPopupMenuShow` contract this fork already inherits.
**Acceptance Criteria:**
- [x] The list appears with the real options and the chosen one is applied to the page (`input`+`change` fire). *(Device-verified 2026-08-03, user-confirmed.)*
- [x] The popup opens **anchored under the tapped control**, not screen-centre. *(The framework's own `_selectRect` anchor path cannot run in this embedding — `BasicWebView` publishes no `onClick` — so the daemon ships the control's rect and the shell places the list.)*
- [x] **No app-side popup implementation exists.** The stock `enyo.WebView` wrapper consumes the inner `BasicWebView`'s `onOpenSelect` itself (`showSelect` → `createSelectPopup` → `PopupList`) and replies via `selectPopupMenuItem`; it never re-publishes the event. An `onOpenSelect` handler in `Browser.js` is unreachable dead code — one was written, proved dead, and deleted. **Do not re-add one**, and do not patch `BasicWebView.showPopupMenu` (an earlier patch shadowed the framework method and killed the popup entirely).
**Dependencies:** cavekit-addons-extensions.md, cavekit-ipc-contract.md (R1)

### R6: Chrome-owned settings
**Description:** The two settings this shell owns rather than the engine — the home button's target and the start page's shortcut list — are stored, rendered and editable.
**Added 2026-08-05 (user requests: a configurable home button; customizable start-page links).**
**Acceptance Criteria:**
- [x] A **home** `ToolButton` sits immediately right of forward in the action bar and opens the configured page, by exactly the path a typed address takes (`ActionBar` `onLoad` -> `goClick`) rather than a new one. Never disabled. Default `https://start.duckduckgo.com/`. *(Device-verified 2026-08-05.)*
- [x] Its icon is a **32x64 two-frame sprite** matching the rest of `app/images/chrome/` — grey normal frame over blue-glow pressed frame — not a flat single-state image.
- [x] The start page renders the stored shortcut list rather than a hard-coded one, and the shell redraws it when Preferences saves, without relaunching the card.
- [x] Both are editable in Preferences: a row per shortcut (name, address, remove), plus add and restore-defaults, and a home-page field where blanking it restores the default. *(Device-verified 2026-08-05.)*
- [x] They are stored in the card's own `localStorage` (`app/source/ChromePrefs.js`) — no new db8 kind, and nothing added to the frozen `callBrowserAdapter` set that R2 holds byte-identical.
- [x] The redraw is dispatched **before** the shell's `applyPreference()`, which switches to the browser view as a side effect and would otherwise yank the user out of Preferences mid-edit.
- [x] The editor keeps its own copy UNCLEANED so a half-finished row (a name typed, no address yet) does not vanish under the cursor; only what reaches the start page is cleaned.
- [x] The card's own preferences panel stays REACHABLE while `about:preferences` cannot yet replace it. *(Added 2026-08-06 after a self-inflicted regression: repointing the single app-menu "Preferences" entry at the engine page left the native panel with no route in, removing the default-search selector, the JavaScript/Flash toggles and all four clear-data actions from the product — and a block delete took five preference handlers with it. The menu now carries "Settings" (the shared page) and "Browser Settings" (this card's panel). The panel goes away when R5 absorbs what only it can do, not before.)*
**Dependencies:** cavekit-preferences-ui.md (R5 — where this panel is headed)

## Out of Scope
- Any rendering, IPC transport, or engine concern (other domains).
- Redesigning the UI/UX — this is a fork that preserves the isis browser UX.
- New webOS DB kinds for bookmarks/history (reuse existing schemas; coexistence-vs-replacement of the stock browser is a packaging decision in cavekit-device-build.md).

## Cross-References
- See also: cavekit-mochi-ui.md (the Enyo-2/Mochi sibling variant), cavekit-ipc-contract.md, cavekit-navigation-events.md, cavekit-licensing-branding.md

### R7: A long-press on page content raises a context menu, as a right-click does on a desktop
**Description:** Holding a finger on a page does something everywhere, not only on links and images. The menu is the desktop browser's: editing commands on a field, the page's actions elsewhere.
**Added 2026-10-05 (owner: "long pressing on the page doesn't do anything but it should, like on a desktop browser").** Applies to all three shells (cavekit-mochi-ui.md R2, cavekit-mojo-ui.md R6).
**Acceptance Criteria:**
- [x] On text: the word under the finger is selected; the menu offers Copy, Select All, Back/Forward (when there is somewhere to go), Reload, Add Bookmark, Find on Page. On empty space: the same without Copy. *(Device, Enyo + Mochi, scripted holds through the adapter's own `mouseHoldAt`: 2026-10-05.)*
- [x] On a text field: the field takes the focus with the caret under the finger, and the menu offers Paste, Select All, and Copy/Cut once something in it is selected. A read-only or disabled field, or a password field's contents, is never offered for editing or copying. *(Device, Enyo + Mochi, 2026-10-05.)*
- [x] A press inside what is already selected keeps the selection, so Select All → press → Copy copies the page. *(Device, 2026-10-05.)*
- [x] A page that handles its own `contextmenu` event (preventDefault) gets no second menu. *(Device, 2026-10-05.)*
- [x] Cut deletes only once the card confirms the text is on the clipboard, and only what was copied, whole; the card is told how it ended ("Cut.", or why not). Paste is bounded and sent in pieces the IPC can carry (one message holds 16 KB). *(Device, 2026-10-05.)*
- [x] All of it inside the frozen contract: the adapter's own `selectAll` / `copy` / `cut` / `insertStringAtCursor` (they were stubs in the daemon), one bundled component (`components/jihadPageCommands.js`) and the notification channel; copied text reaches the card as a one-call script in the app's own `clip/` directory, under a random name, in plain ASCII. Commands act on the page they were asked of and run from the daemon's guarded tick.
- [ ] A real finger: hold on text, on a field, on a link, in all three shells; Mojo's menu has no test hook at all. [human at the device]
- [ ] Fields inside iframes get the page's menu, not the editing one (editability is hit-tested in the top document). Known gap; decide whether it matters. [decision]
**Dependencies:** cavekit-ipc-contract.md (R1), cavekit-gre-widgets.md (R5 — the notification channel)

### R8: The padlock shows whose certificate the page was served with
**Description:** Tapping the address bar's padlock shows the loaded page's certificate (subject, issuer, validity, serial, fingerprint), the connection (TLS version, cipher, key size) and whether the certificate was believed.
**Added 2026-10-05 (owner: "add clicking on the lock to view certificate information").**
**Acceptance Criteria:**
- [x] The daemon's own Luna service answers `pageCertificate` to this variant's own card only, in the reply itself (not on the public notification channel), from the docshell's security UI: certificate fields, connection, and `broken` / `overridden` / `untrusted` / `mismatch` / `badTime`. *(Device, 2026-10-05: *.wikipedia.org, YE2, TLS 1.3, AES-GCM, 128-bit keys.)*
- [x] Enyo's existing padlock is the button; the certificate view it opens is the one the SSL warning uses, with fingerprint, connection and trust rows added. Mochi and Mojo, which had no padlock, get one in the address field (Mojo's drawn as the field's background, tapped at its left edge). *(Enyo + Mochi device-verified; Mojo drawn, tap untested.)*
- [x] No padlock over a failed load: what is on screen is an error page, not that site. *(Enyo, device, 2026-10-05.)*
- [x] With several cards open the answer is for the card that asked: the adapter's `pageFocused` — an empty stub until now, on which thumbnails and printing already relied — sends the daemon the adapter's existing message, and the daemon clears the focus when a card gives it up. *(Device, 2026-10-05: a card's own call seen reaching the daemon right before its request.)*
- [x] Mojo: the padlock tap; Mochi: the padlock after a failed load. *(Device, 2026-10-06, by the shells' own handlers driven from the test hooks: Mochi over `https://no-such-host.invalid/` — padlock showing before the engine's alert is answered, gone after it, and a tap then says "No certificate information is available for this page."; Mojo over Wikipedia — `handleAddressTap` at x=12 on the field brings the Certificate alert with the issued-to/by, validity, serial, SHA-256 fingerprint and connection lines, and `pageFocused` reaches the daemon ("focus: Luna requests are now answered for the card that just said it has the focus"). Before this day Mojo's `pageFocused` and `cut` went nowhere: the WebView widget wraps neither, and the shell only called wrapped methods — see cavekit-mojo-ui.md R3.)* A finger on the glyph itself is still the owner's. [human at the device]
**Dependencies:** cavekit-gre-widgets.md (the SSL warning's certificate view), cavekit-ipc-contract.md (R1)

## Changelog
- 2026-10-06 (later): R8 "Mojo padlock tap; Mochi padlock after a failed load" `[ ]`→`[x]` (device, from the test hooks). R7 unchanged in boxes; its Mojo path was found broken and fixed (cavekit-mojo-ui.md R3/R6).
- 2026-10-06: Added R7 (long-press menu) and R8 (padlock → certificate) for the 2026-10-05 work, with the device evidence and what still needs a finger.
- 2026-10-01: **Target platform is now webOS CE 3.1.0.** R2: two criteria amended, no box changed
  state — the service-URI criterion now records the own-service-name decision (ipc R4), and the
  "no engine identifiers" criterion gains the start-page branding carve-out the Mojo kit already
  had. R4–R6's device-verified boxes are 3.0.5 evidence and are listed in
  cavekit-device-build.md R4's re-verification sweep.
- 2026-08-05: **R6 added** (home button, customizable start-page links) and met. Stored in the
  card's own `localStorage`, so nothing was added to the frozen adapter set. Note this panel is
  not the final home for them — cavekit-preferences-ui.md R5 merges the per-variant panels into
  `about:preferences`.
- 2026-08-03: Added **R5** (card-side `<select>` popup — device-verified and user-confirmed),
  including the standing "no app-side popup implementation" rule that the investigation
  established. Corrected R1's title AC: the launcher title is "Jihad Enyo" since the three
  variants went live together; in-app branding stays the shared "Jihad Browser" start page, which
  now carries the same logo/title/engine-line/hint block as the other two variants.
- 2026-06-30: Initial draft.
- 2026-07-04: Reconciled — R1 rebrand, R2 unchanged adapter contract (method set + Luna URIs = upstream), R3 Apache headers all verified. R4 is proven contract-correct against the STOCK QtWebKit BrowserServer on-device (openURL->loadStarted->pageTitleChanged->documentLoadFinished); driving the JIHAD Goanna daemon on-device is pending (needs the LunaService daemon + real BrowserAdapter).
- 2026-07-31: Reconciliation against recorded evidence. R4 was still all-unchecked although `../impl/device-test-2026-07-19.md` Session 4 (2026-07-20, device connected) recorded launch-to-start-page, address→`openUrl`, and back/forward/reload as daemon-verified across that session — those three ACs are now `[x]` with that citation. `findInPage` stays open: the same record says "pending a focused test". No other change; R1–R3 were already evidence-backed.
