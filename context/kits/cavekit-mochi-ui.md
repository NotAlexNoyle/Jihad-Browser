---
created: "2026-06-30"
last_edited: "2026-08-03"
---

# Cavekit: Mochi UI Variant

## Scope
A second browser front-end (directory `app-mochi/`) implemented in **Enyo 2 + the
Mochi UI library**, functionally equivalent to the legacy Enyo-1.0 UI
(cavekit-ui-shell.md) and driving the **same** BrowserAdapter contract. It ships
as a separate `.ipk` so all three UIs (this one, the Enyo-1.0 shell and the Mojo
shell) coexist on the device. The rendering engine
and IPC are unchanged — this domain is purely an alternative presentation layer.

Reference: `app-mochi/README.md`, the Mochi library at `../mochi`, the Enyo-2
sampler at `../mochi-sampler`, `docs/IPC-CONTRACT.md`.

## Requirements

### R1: Separate, coexisting Mochi application package
**Description:** The Mochi UI is packaged as its own webOS app, installable alongside the Enyo variant.
**Acceptance Criteria:**
- [x] `app-mochi/appinfo.json` declares a distinct app id (`net.riverstonerelay.jihad-browser-mochi`) and a launcher title. *(2026-07-18, T-049. **Superseded 2026-08-03 by a later user directive**: with all three variants live at once, identical launcher titles were unusable, so the apps are now titled **Jihad Enyo / Jihad Mochi / Jihad Mojo**. The in-app branding stayed unified — every start page says "Jihad Browser" over the shared logo.)*
- [x] Installing it does not collide with or replace the Enyo variant (`net.riverstonerelay.jihad-browser`); both can be installed at once. *(VERIFIED ON DEVICE 2026-07-19: `palm-install` of the Mochi ipk succeeded with the Enyo variant present; `palm-install -l` lists both `net.riverstonerelay.jihad-browser 1.0.0` and `net.riverstonerelay.jihad-browser.mochi 1.0.0`. **That transcript is quoted verbatim and predates the 2026-08-01 rename** — the Mochi id is now `net.riverstonerelay.jihad-browser-mochi`, because the dotted form made it an ipkg dot-child of the Enyo package whose metadata was destroyed when Enyo was removed (`../impl/impl-ipkg-prefix-collision.md`). Coexistence is unaffected in principle, but re-verify under the new ids.)*
- [x] Uses the Jihad Browser icon set. *(md5-identical to `app/icon*.png`, verified 2026-07-18.)*
**Dependencies:** none

### R2: Feature parity with the Enyo UI
**Description:** The Mochi UI offers the same user-facing browser features as the Enyo-1.0 UI.
**Acceptance Criteria:**
- [x] Address/search bar with navigation (back, forward, reload, stop) works. *(Structural evidence per `../../app-mochi/PARITY.md` (T-051/T-052, commit 2a79d71); the device gate cleared 2026-08-03 — the Mochi card is live on hardware, drives its own daemon over its own socket, and loads pages end to end (`example.com` and local documents), with the adapter round-trip visible in the daemon log.)*
- [x] Bookmarks, history, and downloads views are present and functional. *(2026-08-05 — the "live db8 round-trip on hardware" this was waiting for is done, and it found a real defect on the way. Mochi's db8 kinds were **NOT REGISTERED** on the device: every read logged `kind not registered: 'net.riverstonerelay.jihad-browser-mochi.history:1'`, so the views were running against a database that did not exist — installed, launched, and silently broken. The cause is a packaging gap, not a UI one: the OS installer does not register an app's `db/kinds`, and Enyo's only worked because a dev helper (`register-db-kinds.sh`) had been run by hand months earlier. The variant `postinst` now registers kinds and permissions itself (`putKind`/`putPermissions`, idempotent upserts, under the variant's own app id since db8 requires owner == registrant), and `prerm` de-registers them so removal is exact. Verified after a supported reinstall: all three Mochi kinds registered by the postinst, and `com.palm.db/find` on history and bookmarks now returns `{"returnValue":true,"results":[]}` instead of an error — a real, queryable, empty store.)*
- [x] Find-in-page, preferences, and the start page are present. *(2026-08-05 — device-verified, and a real defect was fixed to get there. Every Mochi card launch threw `Uncaught ReferenceError: Mojo is not defined`, which this app's own entry point had dismissed in a comment as "unrelated legacy". It is not legacy: **LunaSysMgr calls into a global `Mojo` namespace on every app**, and Enyo 1.0 defines those entry points deliberately — its source says so, "LunaSysMgr calls use Mojo namespace atm" (`framework/source/palm/system/windows/events.js`) — which is exactly why the Enyo variant never saw it and this bundled Enyo 2 + Mochi app did. The cost was not cosmetic: `Mojo.relaunch()` is how the system hands a running card a new launch target, and Enyo 1.0's own comment says it must RETURN TRUE "otherwise it'll try to focus the app". So "open this URL in Jihad Mochi" while the card was already open had nowhere to land. `app-mochi/index.html` now defines the namespace BEFORE the frameworks load (a handler that does not exist yet is the bug) — `relaunch`, `stageActivated`, `stageDeactivated`, `show`, `hide`, `keyboardShown`, `positiveSpaceChanged`, `lowMemoryNotification` — with `relaunch` parsing `PalmSystem.launchParams` and navigating. Measured after the fix: zero occurrences of the error, and a relaunch with a target reaches the daemon as `openUrl http://example.com/RELAUNCHTEST`. Preferences also persist for real now that the db8 kinds are registered (criterion above).)*
- [x] Alert/confirm/prompt/auth and SSL-confirm dialogs are presented and answerable. *(2026-07-20: implemented in `JihadDialogs.js` + the overflow menu / generic dialog in `JihadBrowser.js`, but NOT as `mochi.Popup` — a floating/modal `mochi.Popup` CRASHES the Mochi card on this engine (pressing Share crashed the app). Converted to plain Control overlays (a scrim + centered box toggled by `showing`, `jihad-btn` divs instead of `mochi.Button`) — the same pattern the parity list-views use. The share crash is gone and the card opens cleanly (verified on-device 2026-07-20). **DEVICE-VERIFIED 2026-08-05**: the Mochi card was launched at a page whose script calls `alert()`, and the daemon log shows the whole round trip — `dialog alert -> card (pipe /var/palm/jihad/mochi/dialog-1.fifo)`, then `dialog alert: answered after 11406 ms`, then `dialog alert -> ACCEPT`, with the daemon still running afterwards. So the card presented it and something card-side replied through the FIFO, which is what this criterion asks. Honest limit: from the daemon it cannot be told whether a person tapped OK or the card answered by itself — either way the path works end to end. The 11.4 s is worth keeping too: it is well past the 5 s deadline the daemon used to enforce, so this exact test would have been silently defaulted before 2026-08-04 (cavekit-browser-services.md R3) — a real-card confirmation of that fix rather than a scripted one.)*
- [~] A parity checklist against `../app` source shows no missing user-facing feature (or documents intentional omissions). [human-review] *(2026-07-31 reconciliation: the checklist EXISTS and is complete — `../../app-mochi/PARITY.md` (commit 0d2fb82) maps every user-facing Enyo-1.0 feature to Ported / Simplified / Omitted with a rationale per omission (bookmark-edit dialog, thumbnails, download auto-initiation + retry, context menu, share sheet, add-to-launcher, print, cert detail viewer, search-engine selector, live-engine pref application). Stays `[~]` because the `[human-review]` sign-off on that checklist is not recorded anywhere.)*
**Dependencies:** cavekit-navigation-events.md, cavekit-browser-services.md

### R3: Drives the unchanged BrowserAdapter contract
**Description:** The Mochi UI talks to the engine only through the existing contract; no engine/IPC change.
**Acceptance Criteria:**
- [x] An Enyo-2 WebView-equivalent control binds to a BrowserAdapter NPAPI plugin speaking the same contract the Enyo UI uses. *(2026-07-19, T-051: `app-mochi/source/JihadWebView.js` renders an `<object>` whose type is the Jihad MIME, `node.eventListener=this`, callback arg orders verified against render/adapter/BrowserAdapter.cpp. The R7 reopening is CLOSED: the control binds to this variant's own `application/x-jihad-browser-mochi`, and the live daemon handshake was verified on device 2026-08-03 (`client connected` on `/tmp/yapserver.jihad-browser-mochi`, then paints).)*
- [x] The `callBrowserAdapter(...)` method set and `palm://com.palm.browserServer/*` URIs used are identical to the Enyo variant (no additions/renames). *(Verified 2026-07-19: set {findInPage, goBack, goForward, reloadPage, stopLoad} + URIs {clearCache, clearCookies} — diff empty both.)*
- [x] No Goanna/UXP-specific identifiers appear in `app-mochi/`. *(grep clean 2026-07-19.)*
**Dependencies:** cavekit-ipc-contract.md (R1, R5), cavekit-ui-shell.md (R2)

### R4: Built with Mochi controls; layout fits both TouchPad models
**Description:** The UI uses Mochi widgets and renders correctly on TouchPad and TouchPad Go.
**Acceptance Criteria:**
- [x] UI is composed from Mochi controls (e.g. Header, IconButton, Input, List, Panels, ProgressBar — NOT Popup, which crashes on this engine; see the deviation note) rather than ad-hoc markup. *(2026-07-19, T-052: mochi.Header + FittableColumns toolbar (IconButton nav + InputDecorator/Input address), mochi.ProgressBar. 2026-07-20 DEVIATION: `mochi.Popup` is NOT usable — a floating/modal Popup crashes the card on this engine (Goanna/ESR52 host). The overflow menu + all dialogs use plain Control overlays instead (scrim + box toggled by `showing`); `mochi.Input`/`Header`/`ProgressBar` etc. are fine. Documented in PARITY.md + [[jihad-input-activation-and-tiling]].)*
- [x] Layout is usable on the TouchPad (Topaz) screen. *(2026-08-05: the URL text in the address bar overlapped the inline reload glyph — user-reported and fixed. The rule already said `padding-right` on `.jihad-address`, but `mochi.Input` puts that class on a WRAPPER and renders the real `<input>` inside it, so the reserve landed on the wrapper while the field's text ran straight under the icon. Now set on the element that matters (`.jihad-address input, input.jihad-address`), and MEASURED rather than eyeballed, from inside the card: `input.right=609 padRight=44 textEdge=565 glyph.left=579 overlap=-14` — i.e. 14 px of clearance where there had been ~30 px of overlap. A one-shot `[JIHAD-URLGEOM]` probe logs those numbers at card start, because this is the second time a card-WebKit layout bug has been invisible to inspection and obvious to arithmetic.)* *(**Topaz verified 2026-08-03** — toolbar, start page and the `<select>` popup all render correctly on hardware. Fittable/relative layout, shared 1024×768, no hardcoded px. **TouchPad Go moved out of this criterion 2026-08-04**: every Opal gate now lives in cavekit-device-build.md R6, so the same missing hardware is not re-litigated in four kits. Note the prefix constraint below: unprefixed `box-sizing`/flexbox is silently dropped by the card WebKit, which is a layout bug waiting to happen, not a cosmetic detail.)*
- [x] Enyo 2 core + layout + Mochi are bundled into the package at build time (not vendored in this repo). *(2026-07-18, T-049: `build/webos-oe/build-mochi-ipk.sh` stages Enyo 2 core from `../mochi-sampler/enyo`, layout from `webos-stacks/mochi/lib/layout` (mochi-sampler's lib/ dirs are empty; overridable via `LAYOUT_SRC`), Mochi from `../mochi` → palm-package → 1.4 MB ipk, 394 entries; output git-ignored, nothing vendored.)*
**Dependencies:** cavekit-device-build.md (R3, R6)

### R5: Licensing and attribution
**Description:** The Mochi variant respects all licenses.
**Acceptance Criteria:**
- [x] New `app-mochi/` source files carry Apache-2.0 headers. *(2026-07-19, T-050.)*
- [x] Bundled Mochi (LG, Apache-2.0) and Enyo 2 are attributed in `NOTICE`. *(2026-07-19, T-050: Enyo 2 core + layout (LG 2012-2014) + Mochi (LG 2014) in NOTICE; confirmed inside packaged ipk — closes codex F-390.)*
**Dependencies:** cavekit-licensing-branding.md (R1, R2)

### R6: Engine `<select>` dropdowns are presented card-side
**Description:** A tap on a page `<select>` shows the option list in the card and applies the choice.
**Added 2026-08-03** (the engine cannot paint a native combobox — it is a separate display root; the daemon serializes the options and the card renders the list, which is the isis/Atlas IPC contract we already inherit).
**Acceptance Criteria:**
- [x] The adapter's `showPopupMenu(id, json)` callback is handled (Enyo 2 has no framework equivalent of the Enyo-1.0 `enyo.WebView` wrapper that would do it for us) and surfaced to the shell.
- [x] The list is presented with the overlay idiom this variant already uses for menus and dialogs — **not** `mochi.Popup`, which crashes the card on this engine (R4) — and is anchored under the tapped control from the rect the daemon ships.
- [x] Exactly one reply per popup: the chosen index, or `-1` on dismissal **and** on any payload the shell cannot present, so the daemon always releases the element it is holding.
- [x] Device-verified: a tap lists the real options and the pick reaches the page (`popupMenuSelect id=… idx=…`, and the test page's `onchange` fired).
**Dependencies:** cavekit-addons-extensions.md, cavekit-ipc-contract.md (R1)

### R7: Chrome-owned settings
**Description:** The two settings this shell owns rather than the engine — the home button's target and the start page's shortcut list — are stored, rendered and editable.
**Added 2026-08-05 (user requests: a configurable home button; customizable start-page links).**
**Acceptance Criteria:**
- [x] A **home** control sits immediately right of forward and opens the configured page. Never disabled — unlike back/forward it does not depend on page history. Default `https://start.duckduckgo.com/`. *(Device-verified 2026-08-05.)*
- [x] The start page renders the stored shortcut list rather than a hard-coded one, and redraws when the list is edited without relaunching the card.
- [x] Both are edited in ONE place, and it is not this panel: `about:preferences`, reached from this variant's overflow menu ("Settings"). *(Changed 2026-08-06. Previously read "editable in this variant's own preferences panel" with a row-per-shortcut editor — which made Mochi a second writer for settings the engine page also owns. The editor is removed; `JihadChromePrefs` is now a read cache with an `adoptFromUrl()` that takes edits published by the page, gated on the url PATH so `about:blank` cannot be used to inject one.)* **CORRECTION 2026-08-10, found while porting the same code into Mojo (T-112): the source comment on `JihadChromePrefs.adoptFromUrl` claims "Same rule as the Enyo shell's ChromePrefs" and only HALF of it is.** The path gate is the same. The Enyo shell's second half — `safeUrl()`, the `https?` / `about:(preferences|settings|jihad|isis|blank)` allowlist applied to every value that comes out of the payload — has **no counterpart** in `app-mochi/source/JihadChromePrefs.js`, whose `cleanLinks` and `saveHome` only `trim()`. Not exploitable through the shipped path (the settings page runs the allowlist itself before publishing, and `about:preferences` is unreachable from content — `jihadAboutPreferences.js` withholds `URI_SAFE_FOR_UNTRUSTED_CONTENT`), so this is defence-in-depth that is MISSING rather than a live hole; but a user who hand-types `about:preferences#chrome=…` can put a `javascript:` url on Mochi's home button. Fix is one line each: `u = safeUrl(...)` in `cleanLinks`, and `safeUrl` in `loadHome`/`saveHome`. **NOT done — T-112 was scoped to Mojo.** ~~Both are editable in this variant's own preferences panel:~~ a row per shortcut (name, address, remove), plus add and restore-defaults, and a home-page field where blanking it restores the default rather than leaving home pointing nowhere.
- [x] They are stored in the card's own `localStorage` (`source/JihadChromePrefs.js`), **not** in the db8 preferences kind this package owns. Reason: the start page reads them while it is being built, and db8 is asynchronous where `localStorage` is not. This registers no new kind and adds nothing to the frozen adapter set.
- [x] The copy is this variant's own CACHE of the shared value — the three shells are deliberately not a shared installation, so each keeps its own copy, but there is now exactly one WRITER (the settings page). *(Resolved 2026-08-06 by removing Mochi's editor; the note below is kept because it records why the original design had to change.)* *(Was downgraded 2026-08-06: This was the right design before the settings merge and is the wrong one after it: `about:preferences` is now the single editing surface, and Mochi shipping its OWN editor over its OWN store makes it a second writer for the same two settings — the exact state cavekit-preferences-ui.md R5 exists to end. Either remove Mochi's editor and read the shared value, or accept it and say so in R5. Not left as-is.)*
**Dependencies:** cavekit-preferences-ui.md (R5 — where this panel is headed), cavekit-ui-shell.md

## Out of Scope
- Any change to the rendering engine, BrowserServer, or the YAP/Luna contract.
- New browser capabilities beyond parity with the Enyo UI (YAGNI).
- Replacing the Enyo variant — both ship.

## Platform constraints learned on device
- **The card WebKit (~534.x) ignores unprefixed `box-sizing` and modern flexbox.** Use
  `-webkit-box-sizing` and the `-webkit-box` flexbox syntax in `JihadBrowser.css`. An
  unprefixed declaration is dropped silently, so a `width:100%` row with padding overflows
  its card (measured on the sibling Mojo variant: 784 px on a 768 px screen). Pages rendered
  by our own Goanna engine are modern and unaffected.
- `mochi.Popup` remains unusable (R4) — every overlay in this variant is a plain Control.

## Cross-References
- See also: cavekit-ui-shell.md (Enyo variant), cavekit-ipc-contract.md, cavekit-navigation-events.md, cavekit-browser-services.md, cavekit-device-build.md, cavekit-licensing-branding.md

## Changelog
- 2026-08-05: **R7 added** (home button, customizable start-page links) and met. The storage
  choice is the point worth keeping: these two live in the card's `localStorage` rather than the
  db8 preferences kind this package already owns, because the start page reads them
  synchronously while building. Note this panel is not the final home for them —
  cavekit-preferences-ui.md R5 merges the per-variant panels into `about:preferences`.
- 2026-08-03: Device gate cleared for this variant. R2's navigation AC and R3's WebView-binding AC
  move to `[x]` (the card is live on hardware, bound to its OWN MIME — closing the R7 reopening —
  and loads pages through its own daemon); R4's layout AC records Topaz verified, Opal not. R1's
  title AC is corrected: the "both variants titled Jihad Browser" directive was superseded when
  all three went live, and the apps are now Jihad Enyo / Jihad Mochi / Jihad Mojo (the in-app
  branding is still the shared "Jihad Browser" start page). Added **R6** (card-side `<select>`
  popup, device-verified) and a platform-constraints section — the card WebKit needs `-webkit-`
  prefixes.
- 2026-06-30: Initial draft (added per request: second UI variant + .ipk).
- 2026-07-04: Status check — app-mochi/ is a skeleton (appinfo.json + index.html + source/ + icons, ~9 files); no requirement met yet. R1–R5 pending: the Mochi/Enyo-2 UI has not been built to parity. Largest remaining kit.
- 2026-07-31: Reconciliation against recorded evidence. R2's first three ACs moved `[ ]`→`[~]`: `app-mochi/PARITY.md` ("R2 acceptance summary", commit 0d2fb82) records them as structurally met — views built (a244eed, 9a8997c, 9418599, dbeafc6), contract-clean against the frozen `callBrowserAdapter` set, ipk builds — while the same document states on-device functional verification is DEVICE-GATED and pending hardware; NOT marked `[x]`. The parity-checklist AC also moved to `[~]` (the checklist exists and documents every omission; the `[human-review]` sign-off is unrecorded). R1/R3/R5 were already `[x]` on cited evidence (R1 dual-install VERIFIED ON DEVICE 2026-07-19); no status was raised without a citation.
