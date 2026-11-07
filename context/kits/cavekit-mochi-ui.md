---
created: "2026-06-30"
last_edited: "2026-11-07"
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
- [x] `app-mochi/appinfo.json` declares a distinct app id (`net.riverstonerelay.jihad-browser-mochi`) and a launcher title. *(2026-07-18, T-049. Since 2026-08-03 the launcher titles are Jihad Enyo / Jihad Mochi / Jihad Mojo (user directive, all three live at once); every start page still says "Jihad Browser".)*
- [x] Installing it does not collide with or replace the Enyo variant (`net.riverstonerelay.jihad-browser`); both can be installed at once. *(Device 2026-07-19: the Mochi ipk installed with the Enyo variant present and `palm-install -l` listed both. That run used the old dotted id; since 2026-08-01 the id is `net.riverstonerelay.jihad-browser-mochi` (`../impl/impl-ipkg-prefix-collision.md`); re-verify under the new ids.)*
- [x] Uses the Jihad Browser icon set. *(md5-identical to `app/icon*.png`, verified 2026-07-18.)*
**Dependencies:** none

### R2: Feature parity with the Enyo UI
**Description:** The Mochi UI offers the same user-facing browser features as the Enyo-1.0 UI.
**Acceptance Criteria:**
- [x] Address/search bar with navigation (back, forward, reload, stop) works. *(`../../app-mochi/PARITY.md` (T-051/T-052, commit 2a79d71); device 2026-08-03: the Mochi card drives its own daemon over its own socket and loads pages end to end.)*
- [x] Bookmarks, history, and downloads views are present and functional. *(Device 2026-08-05: Mochi's db8 kinds had not been registered (`kind not registered: 'net.riverstonerelay.jihad-browser-mochi.history:1'`), because the OS installer does not register an app's `db/kinds`. The variant `postinst` now registers kinds and permissions (`putKind`/`putPermissions`, under the variant's own app id) and `prerm` removes them; after a supported reinstall `com.palm.db/find` on history and bookmarks returns `{"returnValue":true,"results":[]}`. A row was first seen on a device 2026-10-02 (bookmark editing, below).)*
- [x] Find-in-page, preferences, and the start page are present. *(Device 2026-08-05: LunaSysMgr calls a global `Mojo` namespace on every app (Enyo 1.0 defines it, Enyo 2 does not), so every launch threw `Mojo is not defined` and a relaunch with a target had nowhere to land. `app-mochi/index.html` defines `relaunch`, `stageActivated`, `stageDeactivated`, `show`, `hide`, `keyboardShown`, `positiveSpaceChanged` and `lowMemoryNotification` before the frameworks load; a relaunch with a target reaches the daemon as `openUrl http://example.com/RELAUNCHTEST`.)*
- [x] Alert/confirm/prompt/auth and SSL-confirm dialogs are presented and answerable. *(`JihadDialogs.js`, as plain Control overlays because `mochi.Popup` crashes the card on this engine (R4). Device 2026-08-05: an `alert()` page logged `dialog alert -> card (pipe /var/palm/jihad/mochi/dialog-1.fifo)`, `dialog alert: answered after 11406 ms`, `dialog alert -> ACCEPT`, the daemon still running.)*
- [x] **The long-press menu on any page content** (cavekit-ui-shell.md R7), the **padlock** (R8) and **Palefill** (cavekit-addons-extensions.md R9) are in this shell too. *(2026-10-05, device: the scripted long-press flows, the padlock's certificate dialog, Palefill installed and active in this variant's profile.)*
- [~] A parity checklist against `../app` source shows no missing user-facing feature (or documents intentional omissions). [human-review] *(`../../app-mochi/PARITY.md` maps every Enyo-1.0 feature. Owner review 2026-10-02 (the [human-review]): no omission or simplification is accepted, and each became a criterion below. Since 2026-10-05 every row is Ported, Print last. Open: the finger steps of the long-press menu.)*
- [x] **Bookmark editing.** A saved bookmark's title and address can be changed from the bookmarks view and the change is stored (db8 merge on this package's own kind). The address is stored as typed — not HTML-escaped (the Enyo dialog escapes it, which turns `a=1&b=2` into `&amp;`; that is not copied). *(MET 2026-10-02, CE 3.1.0, Mochi card, through the shell's own handlers by `build/webos-oe/card-selftest.sh`; a finger on the control is on the human checklist, here and in the criteria below that cite `card-selftest.sh`. Edit raises the dialog (`../impl/evidence-2026-10-02/mochi_settings-panel_clear-confirm_bookmark-editor.jpg`); title `Edited & saved <b>` and address `http://example.com/?a=1&b=2` are stored exactly. Fixed on the way: the list had never shown a row on a device (it ordered by `date`, which has no index: `-3965 db: no index for query`; now `_rev`), and a panel hidden at layout time is now reflowed when opened.)*
- [x] **Bookmark thumbnails.** Adding a bookmark captures a thumbnail and icon of the page and the bookmarks view shows it.  *(MET 2026-10-02, CE 3.1.0, `card-selftest.sh`. A card may load a local image only from its own app directory and the adapter's `saveViewToFile` writes only under `/var` or `/tmp`, so the variant's Luna service has `saveThumbnail`/`deleteThumbnail`/`clearThumbnails` (own card only) and the daemon writes a 192x144 PNG of the next real paint to `<appdir>/thumbnails/t-….png`. Add Bookmark → the row shows it (`../impl/evidence-2026-10-02/mochi_bookmark-with-thumbnail.jpg`); deleting the bookmark removes the file; uninstall purges the folder.)*
- [x] **Downloads start from a page.** Navigating to a resource the engine cannot show starts a download that appears in the downloads view with its progress and its result, and a failed one can be retried from the view. Exactly ONE copy of the file is fetched (the engine already saves the file itself and also announces it; a card that answers the announcement with a second download fetches everything twice). *(MET 2026-10-02, CE 3.1.0: launched on `http://…/jihad-testfile.bin`, the engine saved it, the downloads view lists it `completed` with Open (`../impl/evidence-2026-10-02/mochi_share-opens-email-and-messaging_downloads-view.jpg`), and the server logged one `GET`. An injected failed record shows Retry, which fetched the file again. Not seen: a progress percentage (file too small). An engine download has no Cancel (no adapter command). The Enyo shell answers the engine's hand-off with a second, download-manager fetch, hence its `name-1`, `name-2` copies.)*
- [~] **Long-press menu.** Holding on a link offers open in new card, share link and copy address; holding on an image offers save, share and set as wallpaper; each does what it says. A tap is unaffected. A save lands in the download carve-out of cavekit-device-build.md R8, nowhere else on the user volume. *(2026-10-02, CE 3.1.0: given a hit-test result for a link and an image through `eventFired`, the card raises Open in New Card, Share Link, Copy Address, Save Image, Share Image, Set as Wallpaper (`../impl/evidence-2026-10-02/mochi_history-groups_clear-confirm_share-sheet_hold-menu.jpg`); a tap or a hold on plain content is not claimed. Open in New Card opened a second card (`../impl/evidence-2026-10-02/mochi_open-in-new-card.jpg`); Copy Address → clipboard; Save Image → `/media/internal/downloads` through the system download manager (the daemon's `saveImageAtPoint` is a stub, so an inline `data:` image cannot be saved). 2026-10-05, at the owner's request (`card-selftest.sh mochi run holdpick:wallpaper|share`): Set as Wallpaper changed the system `wallpaper` preference, then put back; Share Image opened Email with the picture (`../impl/evidence-2026-10-05/mochi_share-image-opens-email.jpg`). Open then: the real hold. 2026-10-24, emulated finger (T-526): Paste from a field's hold sheet now clicks the held point first so the text lands; the link sheet and the text hold verified by finger.)* *(suite: `long-press-menu`, `long-press-text-field`, `long-press-image`; `docs/FINGER-TESTS.md`)*
- [x] **Share and Add to Launcher.** The share entry offers email and messaging and hands the page title and address to the chosen app; Add to Launcher creates a launch point that opens the page in this variant. *(MET 2026-10-02, CE 3.1.0: the share button raises Email / Text Message / Copy Address / Add to Launcher; Email and Messaging open (`applicationManager/open` → `returnValue:true`; no account on the device, so no draft seen); Add to Launcher → `{"returnValue":true,"launchPointId":"…"}` (removed after). No Facebook entry: that app is not on webOS CE.)*
- [x] **Print.** *(MET 2026-10-05, CE 3.1.0, by a different road than the original wording: the system print stack discards every printer that is not HP (it logged "not HP printer and doing printer filtering" for the owner's Epson ET-2800 at 192.168.1.253), and the Enyo shell's stock Print printed nothing. The daemon finds IPP printers itself and prints the page as JPEG sheets (`render/goanna/components/jihadPrint.js`, commit e0ab6b26; the mDNS query must go out FROM port 5353, as the firewall drops answers to an ephemeral port); the shell has its own dialog (`app-mochi/source/JihadPrintDialog.js`; the Enyo shell uses the same). Device (`card-selftest.sh mochi run print[:address]`): the dialog listed `EPSON ET-2800 Series · 192.168.1.253` and a Mac-shared ET-2720 (`evidence-2026-10-05/mochi_print-dialog-lists-the-epson.jpg`); a test sink (`render/goanna/test/ipp-sink.py`) received the three-sheet test page as three sheets, nothing missing or repeated. One sheet reached the Epson `completed-successfully` (job 31), sent by the daemon's print call, not this dialog. Not covered: a person looking at a sheet printed from this dialog, and a finger on it. Only the variant's own card may find printers or print; the frozen YAP contract is untouched.)* *(Original criterion follows.)* **Print.** The page can be sent to a printer through the system print dialog.
- [x] **Certificate details.** The SSL warning offers a view of the certificate (subject, issuer, validity, serial, algorithms) before the user answers it. *(MET 2026-10-02, CE 3.1.0, `card-selftest.sh`: self-signed https server; View Certificate shows `Issued to: jihad-test.invalid`, issuer, validity, serial and SHA-256 fingerprint (`../impl/evidence-2026-10-02/mochi_ssl-warning_certificate-details.jpg`). The Enyo route cannot work on CE (`com.palm.certificatemanager` answers a card "Service does not exist", the daemon is parked on the reply pipe, the card cannot read a local file), so the daemon writes the details as a one-call script into `<appdir>/certinfo/` while the warning is open.)*
- [x] **Search engine choice.** The engine used for a non-address entry in the address bar can be chosen from a list in the card's settings and the choice persists. *(MET 2026-10-02, CE 3.1.0, `card-selftest.sh`: three engines listed; Google → `hello world` resolves to `https://www.google.com/search?q=hello%20world` and survives a relaunch; DuckDuckGo → `https://duckduckgo.com/?q=hello%20world`. The choice is the card's own (local storage), not the system search setting.)*
- [x] **Flash switch.** The card's settings carry a Flash switch that really turns the plugin on and off — the same stored value the settings page shows (cavekit-addons-extensions.md R7). The Enyo shell's switch writes a system preference nothing in this browser reads; that is not copied. *(MET 2026-10-02, CE 3.1.0, `card-selftest.sh`: switch off → `getFlash` `"enabled":false`, kept across a relaunch; on → `true`. Off, a Flash page instantiates no plugin; on, the movie plays (`../impl/evidence-2026-10-02/mochi-flash-from-system-folder_switch-on_switch-off.jpg`). `luna-send` and another variant's app id are refused by the setters.)*
- [x] **Clear History clears the engine too.** After Clear History the card can no longer go back or forward to the cleared pages, and its back/forward controls show that at once. *(MET 2026-10-02, CE 3.1.0, `card-selftest.sh`: two pages loaded, Clear History → Back and Forward disabled at once; a `goBack` sent to the engine afterwards stays on the same address; db8 find returns 0 rows.)*
- [x] **Confirmation before a clear.** Clear History, Clear Bookmarks, Clear Cookies and Clear Cache each ask first and do nothing on cancel. *(MET 2026-10-02, CE 3.1.0, `card-selftest.sh`: Clear Cookies asks "Would you like to clear your browser cookies and local plug-in data?" and Cancel sends nothing; the history view's Clear clears only on OK (8 rows → 0). Screenshots in `../impl/evidence-2026-10-02/`. Its own overlay, not the engine-dialog confirm, whose buttons answer the page.)*
- [x] **Grouped history dates.** The history view groups entries under Today, Yesterday, weekday names, weeks and month names. (The Enyo view's week bucket never appears because of a misspelt constant; the bug is not copied.) *(MET 2026-10-02, CE 3.1.0, `card-selftest.sh`: visits aged 0, 1, 3, 9, 20, 60 and 400 days are headed Today, Yesterday, Tuesday, Last Week, 2 Weeks Ago, August, 2025 (`../impl/evidence-2026-10-02/mochi_history-groups_clear-confirm_share-sheet_hold-menu.jpg`); 14 of 14 bucket cases pass on the host.)*
- [x] **The upper-right menu is Atlas's and the Enyo shell's** (owner 2026-10-06: "Make sure the upper right menu has the same options on all 3 variants (see atlas for reference)"): Share ▾ = Add Bookmark · Share Link (Email · Text Message · Copy Address) · Add to Launcher; New Card; Bookmarks ▾ = Bookmarks · History · Downloads. The app menu (the card's title bar) is Find on Page · Settings · Print, and opens on the system's own tap: sysmgr relaunches the card with `{"palm-command":"open-app-menu"}`, which `index.html`'s `Mojo.relaunch` hands to the shell (Enyo 2 has no such mapping). *(Device 2026-10-06: hook `Mshare`/`Mlibrary`/`Mapp` + `K`/`R` — each menu's rows, Share Link → its sheet, Bookmarks → the panel, Find → the find bar; a real `open-app-menu` relaunch opened the menu at the left.)*
**Dependencies:** cavekit-navigation-events.md, cavekit-browser-services.md

### R3: Drives the unchanged BrowserAdapter contract
**Description:** The Mochi UI talks to the engine only through the existing contract; no engine/IPC change.
**Acceptance Criteria:**
- [x] An Enyo-2 WebView-equivalent control binds to a BrowserAdapter NPAPI plugin speaking the same contract the Enyo UI uses. *(2026-07-19, T-051: `app-mochi/source/JihadWebView.js` renders an `<object>` of this variant's own MIME `application/x-jihad-browser-mochi`; device 2026-08-03: `client connected` on `/tmp/yapserver.jihad-browser-mochi`, then paints. 2026-10-01: the plugin node is not scriptable until `adapterInitialized`, after the adapter has connected, so an engine-created card came up empty (`connect … id=-1`); the adapter re-sends its connect state when a real identifier arrives late, and `JihadBrowser.create()` hands it over early.)*
- [x] The `callBrowserAdapter(...)` method set and `palm://com.palm.browserServer/*` URIs used are identical to the Enyo variant (no additions/renames). *(2026-07-19: methods {findInPage, goBack, goForward, reloadPage, stopLoad} and URIs {clearCache, clearCookies}, diff against Enyo empty. Since 2026-10-02 the shell also calls `clearHistory`, the image and thumbnail methods and the download callbacks, all already used by the Enyo shell on the unchanged adapter table.)*
- [x] No Goanna/UXP-specific identifiers appear in `app-mochi/`. *(grep clean 2026-07-19.)* *(Exceptions, 2026-10-01: the start page prints "Mochi UI ★ Goanna/6.9 UXP/…" (`app-mochi/source/JihadBrowser.js`), user-facing branding as in cavekit-mojo-ui.md R3; the card's Luna calls go to this variant's own service (`jihadBrowserMochi/{clearCookies,clearCache,notifications}`), cavekit-ipc-contract.md R4.)*
**Dependencies:** cavekit-ipc-contract.md (R1, R5), cavekit-ui-shell.md (R2)

### R4: Built with Mochi controls; layout fits both TouchPad models
**Description:** The UI uses Mochi widgets and renders correctly on TouchPad and TouchPad Go.
**Acceptance Criteria:**
- [x] UI is composed from Mochi controls (e.g. Header, IconButton, Input, List, Panels, ProgressBar — NOT Popup, which crashes on this engine; see the deviation note) rather than ad-hoc markup. *(2026-07-19, T-052: mochi.Header, FittableColumns toolbar, IconButton, InputDecorator/Input, mochi.ProgressBar. 2026-07-20: `mochi.Popup` crashes the card on this engine; menus and dialogs are plain Control overlays (PARITY.md, [[jihad-input-activation-and-tiling]]).)*
- [x] Layout is usable on the TouchPad (Topaz) screen. *(Topaz device 2026-08-03: toolbar, start page and `<select>` popup render correctly; shared 1024×768 fittable layout, no hard-coded px. 2026-08-05: the address text ran under the inline reload glyph because `mochi.Input` puts the class on a wrapper; the padding is now on the `<input>` itself, measured in the card: `input.right=609 padRight=44 textEdge=565 glyph.left=579 overlap=-14` (a `[JIHAD-URLGEOM]` probe logs it at card start). TouchPad Go gates live in cavekit-device-build.md R6.)*
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
- [x] Both are edited in ONE place, and it is not this panel: `about:preferences`, reached from this variant's overflow menu ("Settings"). *(Changed 2026-08-06: Mochi's own editor removed; `JihadChromePrefs` is a read cache whose `adoptFromUrl()` takes edits published by the page, gated on the url path. 2026-08-16: `safeUrl` (the `https?` / `about:(preferences|settings|jihad|isis|blank)` allowlist) is applied in `cleanLinks`, `loadHome` and `saveHome` of `app-mochi/source/JihadChromePrefs.js`, identical to Enyo `ChromePrefs.js`, Mojo and `preferences.js`, so a hand-typed `about:preferences#chrome=…` cannot put a `javascript:` url on the home button.)* ~~Both are editable in this variant's own preferences panel:~~ a row per shortcut (name, address, remove), plus add and restore-defaults, and a home-page field where blanking it restores the default rather than leaving home pointing nowhere.
- [x] They are stored in the card's own `localStorage` (`source/JihadChromePrefs.js`), **not** in the db8 preferences kind this package owns. Reason: the start page reads them while it is being built, and db8 is asynchronous where `localStorage` is not. This registers no new kind and adds nothing to the frozen adapter set.
- [x] The copy is this variant's own CACHE of the shared value — the three shells are deliberately not a shared installation, so each keeps its own copy, but there is now exactly one WRITER (the settings page). *(Resolved 2026-08-06 by removing Mochi's editor; cavekit-preferences-ui.md R5.)*
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
- 2026-11-07: sweep; long evidence notes shortened, history moved verbatim to `archive/mochi-ui-history.md`.
- 2026-11-06 (T-565's whole runs): a tap on the page takes the focus off the card's own fields, so the keyboard goes down, as on Enyo (`keyboard-address`). T-569 closed: the card's size sent again no longer re-lays a page out; desktop Wikipedia keeps its declared width and the fit zoom here too (`wikipedia-zoom`, `wikipedia-keyboard-zoom`).
- 2026-10-06 (night): R2 — the upper-right menu and the app menu match Atlas and the Enyo shell; the title-bar tap opens the app menu.
- 2026-10-06: R2 gains the long-press menu, the padlock and Palefill (device-verified 2026-10-05, scripted).
- 2026-10-05: R2 Print `[ ]`→`[x]`, by the daemon's own IPP path and the shell's own dialog. Long-press menu: Share Image and Set as Wallpaper run on the device at the owner's request.
- 2026-10-01: target platform is now webOS CE 3.1.0; R3 amended (branding carve-out, own service name). 3.0.5-era device boxes are listed in cavekit-device-build.md R4's re-verification sweep.
- 2026-08-05: R7 added (home button, start-page links) and met; stored in `localStorage` because the start page reads them synchronously.
- 2026-08-03: device gate cleared for this variant; R6 (card-side `<select>`) added and device-verified; platform-constraints section added.
- 2026-06-30: Initial draft (added per request: second UI variant + .ipk).
- 2026-07-04: status check; `app-mochi/` was a skeleton.
- 2026-07-31: reconciliation against recorded evidence (`app-mochi/PARITY.md`, commit 0d2fb82).
