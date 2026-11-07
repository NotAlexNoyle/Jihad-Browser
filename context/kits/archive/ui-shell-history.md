# cavekit-ui-shell.md history, moved out of the kit by the 2026-11-07 sweep; verbatim.

## R2, criterion 2 (removed 2026-11-07)

*(**AMENDED 2026-10-01 — the criterion as worded has been deliberately untrue since 2026-08-04, and the kit never said so.** `clearCookies` / `clearCache` go to this variant's OWN service, `palm://net.riverstonerelay.jihadBrowser/…` (`app/source/Browser.js`), because the upstream URI is the STOCK daemon and calling it clears the wrong browser — cavekit-ipc-contract.md R4 owns that decision. The card also subscribes to that service's `notifications` method for toasts (cavekit-gre-widgets.md R5). What this criterion now requires: the METHOD names and payloads are upstream's; only the service NAME differs, and it is this variant's own.)*

## R2, criterion 3 (removed 2026-11-07)

*(**AMENDED 2026-10-01:** a grep is no longer literally clean — the start page prints an engine line, "Enyo UI ★ Goanna/6.9 UXP/…" (`app/source/StartPage.js`). That is user-facing branding text, not an API identifier, and the Mojo kit has carried the same carve-out since it was written (cavekit-mojo-ui.md R3). The criterion is about the card not depending on the engine; it does not.)*

## R3, misplaced note (moved to the Changelog, not removed) (removed 2026-11-07)

*(2026-10-25, T-540: a dialog's reply wait ends when the card's socket hangs up (the card closed with the dialog up) or a send to it failed — it used to hold the main thread for the whole 60 s deadline, and no new card could connect meanwhile.)*

## R4, criterion 4 (find) (removed 2026-11-07)

*(2026-08-05 — FIXED, and the diagnosis this criterion carried for two weeks was WRONG. It said `FindNext` "dereferences a frame-selection controller the offscreen browser does not set up", and that the fix needed an offscreen-safe selection controller. The selection machinery was never the problem. `nsWebBrowserFind::SearchInFrame` performs a same-origin check through `nsContentUtils::SubjectPrincipal()`, whose first act is `if (!GetCurrentJSContext()) MOZ_CRASH("Accessing the Subject Principal without an AutoJSAPI on the stack is forbidden")`. An EMBEDDER calls `FindNext` from C++ with no script on the stack — no JSContext — so the engine's own deliberate abort fired, and MOZ_CRASH presents as SIGSEGV at address 0, which is precisely the fault that had been measured and misattributed. Found by instrumenting the engine and watching where execution stopped: between "before SearchInFrame" and any selection code. Patch 0015 consults the subject principal only when a JSContext exists; with none the caller is native embedding code, already inside libxul's own process holding that docShell's finder, so the check has no caller to check. Desktop result: `find 'JIHADFINDME' -> 1` — a real match, no crash. The UI half was already correct, so the whole path now works. **DEVICE-VERIFIED the same day**, after the ARM libxul rebuild: on a page containing the token, `find 'JIHADFINDME' -> 1`, and on a token that is absent, `find 'NOTPRESENTXYZ' -> 0` — a positive and a negative, with the daemon alive and no fault report either side. The daemon-side `Find` no longer returns a hardcoded false.)* *(2026-10-01 (third session) — the hit was INVISIBLE on screen in all three shells, fixed by pref. The engine found the word (`find 'FINDTOKENQ7' -> 1`, `-> 0` for an absent one) and drew it as a solid white box with the text gone: with no native theme the stand-in palette answers white for every text-selection colour, so selected text was white on white. `packaging/prefs/jihad-platform-prefs.js` now sets `ui.textSelectBackground` / `Foreground` and the attention and highlight colours; screenshot after: the hit is readable.)*

## R6, criterion 8 (removed 2026-11-07)

*(Added 2026-08-06 after a self-inflicted regression: repointing the single app-menu "Preferences" entry at the engine page left the native panel with no route in, removing the default-search selector, the JavaScript/Flash toggles and all four clear-data actions from the product — and a block delete took five preference handlers with it. The menu now carries "Settings" (the shared page) and "Browser Settings" (this card's panel). The panel goes away when R5 absorbs what only it can do, not before.)*

## R8, criterion 5 (removed 2026-11-07)

*(Device, 2026-10-06, by the shells' own handlers driven from the test hooks: Mochi over `https://no-such-host.invalid/` — padlock showing before the engine's alert is answered, gone after it, and a tap then says "No certificate information is available for this page."; Mojo over Wikipedia — `handleAddressTap` at x=12 on the field brings the Certificate alert with the issued-to/by, validity, serial, SHA-256 fingerprint and connection lines, and `pageFocused` reaches the daemon ("focus: Luna requests are now answered for the card that just said it has the focus"). Before this day Mojo's `pageFocused` and `cut` went nowhere: the WebView widget wraps neither, and the shell only called wrapped methods — see cavekit-mojo-ui.md R3.)*

## Changelog 2026-10-01 (removed 2026-11-07)

- 2026-10-01: **Target platform is now webOS CE 3.1.0.** R2: two criteria amended, no box changed
  state — the service-URI criterion now records the own-service-name decision (ipc R4), and the
  "no engine identifiers" criterion gains the start-page branding carve-out the Mojo kit already
  had. R4–R6's device-verified boxes are 3.0.5 evidence and are listed in
  cavekit-device-build.md R4's re-verification sweep.

## Changelog 2026-08-05 (removed 2026-11-07)

- 2026-08-05: **R6 added** (home button, customizable start-page links) and met. Stored in the
  card's own `localStorage`, so nothing was added to the frozen adapter set. Note this panel is
  not the final home for them — cavekit-preferences-ui.md R5 merges the per-variant panels into
  `about:preferences`.

## Changelog 2026-08-03 (removed 2026-11-07)

- 2026-08-03: Added **R5** (card-side `<select>` popup — device-verified and user-confirmed),
  including the standing "no app-side popup implementation" rule that the investigation
  established. Corrected R1's title AC: the launcher title is "Jihad Enyo" since the three
  variants went live together; in-app branding stays the shared "Jihad Browser" start page, which
  now carries the same logo/title/engine-line/hint block as the other two variants.

## Changelog 2026-07-04 (removed 2026-11-07)

- 2026-07-04: Reconciled — R1 rebrand, R2 unchanged adapter contract (method set + Luna URIs = upstream), R3 Apache headers all verified. R4 is proven contract-correct against the STOCK QtWebKit BrowserServer on-device (openURL->loadStarted->pageTitleChanged->documentLoadFinished); driving the JIHAD Goanna daemon on-device is pending (needs the LunaService daemon + real BrowserAdapter).

## Changelog 2026-07-31 (removed 2026-11-07)

- 2026-07-31: Reconciliation against recorded evidence. R4 was still all-unchecked although `../impl/device-test-2026-07-19.md` Session 4 (2026-07-20, device connected) recorded launch-to-start-page, address→`openUrl`, and back/forward/reload as daemon-verified across that session — those three ACs are now `[x]` with that citation. `findInPage` stays open: the same record says "pending a focused test". No other change; R1–R3 were already evidence-backed.
