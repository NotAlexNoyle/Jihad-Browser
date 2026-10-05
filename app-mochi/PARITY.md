<!-- Copyright 2026 NotAlexNoyle. Apache-2.0; see ../LICENSE. -->

# Mochi UI parity checklist (T-053 / cavekit-mochi-ui.md R2)

Maps every user-facing feature of the Enyo 1.0 UI (`../app/source`) to its Enyo 2
+ Mochi equivalent in `app-mochi/source`, or records an intentional omission.

**Refreshed 2026-10-05: Print is built; no row is open.** Both shells print through the engine daemon's own IPP path and their own dialog, because webOS's print dialog accepts HP printers only (see the Print row).

**Refreshed 2026-10-02 after the owner's review.** The owner was shown every Omitted and Simplified row and accepted none: each was to be built. Rows changed by that work are marked *(2026-10-02)*; the one row still open is Print, which cannot be verified without a printer the device can reach.

**Refreshed 2026-10-01 against `app/source` and `app-mochi/source` as they are today** (the
previous revision was 2026-08-10 and predated the own-service URIs, the toast channel, the home
control, the stored start-page shortcuts, the shared settings page and the `<select>` list).
Rows added or corrected in this pass are marked *(2026-10-01)*.

**Contract note.** The Mochi UI drives the engine ONLY through the `callBrowserAdapter` literal
set `{findInPage, goBack, goForward, reloadPage, stopLoad, selectPopupMenuItem}` — the last one
is the reply to the adapter's own `showPopupMenu` callback, the same pair the stock Enyo 1.0
WebView uses — plus THIS VARIANT'S OWN Luna service,
`palm://net.riverstonerelay.jihadBrowserMochi/{clearCache,clearCookies,notifications}`
*(2026-10-01: not `com.palm.browserServer`, which is the stock daemon — calling it clears the
wrong browser; cavekit-ipc-contract.md R4)*. Dialog answers use the adapter's existing scriptable `sendDialogResponse`
routed through JihadWebView's variable-method `_call` path (exactly as the Enyo
1.0 app's `viewCall(...)` fallback does), so they do **not** widen that grep-audited
literal set. Persistence uses THIS PACKAGE'S OWN db8 kinds:
`net.riverstonerelay.jihad-browser-mochi.{history,bookmarks,preferences}:1`.
*(2026-10-02: the parity work added `clearHistory` and `pageFocused` (through the variable-method `_call` path, like `sendDialogResponse`), the adapter's download callbacks and its `eventFired` hold callback, and six methods on this variant's own Luna service — `getFlash`, `enableFlash`, `disableFlash`, `saveThumbnail`, `deleteThumbnail`, `clearThumbnails`. Every adapter method and callback is one the Enyo shell already uses on the unchanged adapter; nothing was added to the YAP wire.)*

> **Persistence ownership (audit F-A01, superseded by review F-1 on 2026-08-01).**
> F-A01's fix was a permission grant: the kinds stayed owned by the Enyo package
> and `../app/db/permissions/*` granted the Mochi/Mojo app ids CRUD. That
> made db8 features here REQUIRE the Enyo variant to be installed — which is the
> cross-variant coupling cavekit-device-build.md **R7** forbids, and it failed in
> both directions (Mochi alone: kinds never registered; Enyo removed: this
> variant's data destroyed with a package it does not own). Since a db8 kind's
> owner must equal the registering app id, the fix is a SEPARATE NAMESPACE, not a
> broader grant. This package now declares, ships and owns
> `…jihad-browser-mochi.*` in `db/{kinds,permissions}/`, the cross-variant grants
> are removed from `../app/db/permissions/*`, and no variant depends on another.
> History/bookmarks are therefore per variant, by design. After a dev
> `palm-install`, run `build/webos-oe/register-db-kinds.sh mochi`. Live db8
> round-trips remain device-gated.

Status legend: **Ported** = feature-equivalent · **Simplified** = present with a
documented reduction · **Omitted** = intentionally not built (rationale given).

## Core browsing

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| Address / search bar | `AddressInput.js`, `URLSearch.js` | `JihadBrowser.js` toolbar (`address` Input, `normalizeUrl`) | Ported (shell) |
| Back / Forward / Reload / Stop | `Browser.js`, `ActionBar.js` | `JihadBrowser.js` `goBack/goForward/reloadPage/stopLoad` → `callBrowserAdapter` | Ported (shell) |
| Load progress | `Browser.js` `loadProgress` | `JihadBrowser.js` `mochi.ProgressBar` | Ported (shell) |
| Start page | `StartPage.js` | `JihadBrowser.js` app-chrome overlay; address bar stays empty for it (`addressTextFor`), never recorded in history (`isTransientUrl`) | Ported (shell) |
| New card | `Browser.openNewCard` | `JihadBrowser.openCard` (`window.open` + webOS `attributes=`) | Ported [device-gated] — Enyo 2 has no `enyo.windows`; see audit F-A03 |
| Engine-created page (link `target` / `window.open`) | `BasicWebView.createPage` → `Browser.openNewCardWithIdentifier` | `JihadWebView.createPage` → `onNewPage` → `JihadBrowser.newPageRequested` → `openCard({webviewId})` | Ported [device-gated] — see audit F-A05 |
| Reflect title/URL + history state | `Browser.pageTitleChanged` | `JihadBrowser.pageInfoChanged` | Ported (shell) |
| Home control *(2026-10-01)* | `ActionBar.js` home `ToolButton` → `ChromePrefs.loadHome` | `JihadBrowser.js` `home` nav button, immediately right of forward → `goHome` → `enyo.jihadChrome.loadHome()` | Ported |
| Start-page shortcuts from the stored list *(2026-10-01)* | `StartPage.js` + `ChromePrefs.loadLinks` | `JihadBrowser.js` start-page overlay + `JihadChromePrefs.loadLinks` (card `localStorage`, `safeUrl` allowlist on read and write) | Ported |
| `<select>` list *(2026-10-01)* | stock `enyo.WebView` popup (framework) | `JihadWebView.showPopupMenu` → app overlay list → `selectPopupMenuItem(id, idx\|-1)` — Enyo 2 has no framework popup, so the list is app code | Ported |
| Transient notices ("Cookies cleared.", add-on installed/incompatible) *(2026-10-01)* | `Browser.js` toast fed by the daemon's `notifications` subscription | `JihadToast.js` + `JihadBrowser.js` subscription to `jihadBrowserMochi/notifications` | Ported |

## Bookmarks

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| List saved bookmarks | `BookmarkList.js` (DbService find/subscribe) | `JihadBookmarkList.js` (`enyo.jihad.dbFind` on bookmarks kind, newest first by `_rev`) | Ported *(2026-10-02: until then this list was always EMPTY on a device — it ordered by `date`, which the kind has no index for, and db8 refuses such a query)* |
| Open a bookmark | `BookmarkList.itemClick` → `selectItem` | `JihadBookmarkList` `onSelectItem` → `JihadBrowser.navigateTo` | Ported |
| Delete a bookmark | `BookmarkList` swipe → `deleteBookmark` | row **Delete** → `enyo.jihad.dbDel` | Ported |
| Add current page as bookmark | `BrowserApp.addBookmark` | menu **Add Bookmark** → `JihadBrowser.addCurrentBookmark` (db8 put, same record shape) | Ported |
| Clear all bookmarks | `Preferences` → `BrowserApp.clearBookmarks` | Preferences **Clear Bookmarks** → `JihadBrowser.clearBookmarks` (db8 delByQuery) | Ported |
| Edit bookmark (rename / re-URL) *(2026-10-02)* | `BookmarkDialog.js`, `BrowserApp.editBookmark` | row **Edit** → overlay dialog in `JihadBookmarkList.js` → `enyo.jihad.dbMerge`. Title and address are stored as typed (the Enyo dialog HTML-escapes them, which corrupts `&` in an address; not copied) | Ported |
| Bookmark thumbnails *(2026-10-02)* | `BrowserApp.createPageImages` (`saveViewToFile`/`generateIconFromFile`) | `JihadBrowser.addCurrentBookmark` → `markActive` + own service `saveThumbnail` → the daemon writes a 192x144 PNG of the page into this app's `thumbnails/` folder; the row shows it; deleted with the bookmark | Ported — by a different route: the adapter path writes only under `/var` or `/tmp`, which a card may not load from, and its icon helpers are compiled out, so the Enyo shell gets no thumbnails on this engine at all. No separate 32/64 px icons. |

## History

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| List history (most recent first) | `HistoryList.js` (find orderBy date) | `JihadHistoryList.js` (`dbFind` orderBy `date`, desc) | Ported |
| Record a visit on load-stop | `BrowserApp.updateHistory` | `JihadBrowser.updateHistory` (delByQuery url → put; same kind/shape) | Ported |
| Open a history entry | `HistoryList.itemClick` | `onSelectItem` → `navigateTo` | Ported |
| Delete a history entry | `HistoryList` swipe | row **Delete** → `dbDel` | Ported |
| Clear all history *(2026-10-02)* | `BrowserApp.clearHistory` | header **Clear** / Preferences **Clear History** → confirmation → `JihadBrowser.clearHistory` (db8 delByQuery + the adapter's `clearHistory`; back/forward controls disabled at once) | Ported |
| Relative-date dividers *(2026-10-02)* | `HistoryList.getDivider` (`enyo.g11n`) | `JihadHistoryList.groupLabel`: Today, Yesterday, weekday, Last Week / N Weeks Ago, month, year — built without `enyo.g11n`. The Enyo view's week bucket never appears (misspelt constant); here it does | Ported |

## Downloads

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| List downloads + status/progress | `DownloadList.js`, `BrowserApp` download services | `JihadDownloadList.js` + `JihadBrowser.refreshDownloads` (`getAllHistory`) | Ported |
| Open a completed download | `BrowserApp.openDownloadedFile` | `onOpenItem` → `JihadBrowser.openDownload` (`applicationManager/open`) | Ported |
| Cancel an in-progress download | `BrowserApp.cancelDownload` | `onCancelItem` → `cancelDownload` (`downloadmanager/cancelDownload`) | Ported |
| Clear the downloads list | `BrowserApp.clearDownloads` | header **Clear** → `clearDownloads` (`downloadmanager/clearHistory`) | Ported |
| Download started by a page *(2026-10-02)* | `BrowserApp.handleResource`/`downloadResource` | the engine saves the file itself and reports it; `JihadWebView.download*` → `JihadBrowser.engineDownload*` → the downloads view (progress, result, kept across launches) | Ported — and fetched ONCE: the Enyo shell answers the engine's hand-off with a download-manager fetch of the same URL, so there every such file is fetched twice |
| Retry a failed download *(2026-10-02)* | `DownloadList.itemRetry` | row **Retry** → `JihadBrowser.retryDownload` (re-drives the address; download-manager items are fetched again) | Ported. An engine download cannot be CANCELLED from a card — the adapter has no command for it — so its row offers no Cancel |

## Find in page

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| Find bar (query, prev, next, done) | `FindBar.js` | `JihadFindBar.js` | Ported |
| Run the search | `Browser.find` → `callBrowserAdapter("findInPage")` | `JihadFindBar` `onFind` → `JihadBrowser.findRequested` → `find` → `callBrowserAdapter("findInPage")` | Ported |
| Next / Prev match | `Browser.goToNext/goToPrevious` (no-ops in Enyo 1.0) | re-issue the current query (advances the adapter match) | Ported (matches Enyo 1.0 behaviour) |

## Preferences

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| Block Popups / Accept Cookies / Enable JavaScript toggles *(2026-10-02)* | removed from `Preferences.js` | removed from `JihadPreferences.js` | Removed in BOTH shells (owner decision): the settings page owns the three values; **More Settings** opens it |
| Persist preferences | `BrowserApp.preferenceChanged` (db8 merge) | search engine: the card's own store; Flash: the daemon | Ported *(2026-10-02: the db8 preferences kind is still owned and registered but nothing is stored in it now)* |
| Clear Cookies / Clear Cache | `Browser.clearCookies/clearCache` (`palm://net.riverstonerelay.jihadBrowser/…`) | Preferences → `JihadBrowser.clearCookies/clearCache` (`palm://net.riverstonerelay.jihadBrowserMochi/…`) *(2026-10-01: own-service URIs on both sides)* | Ported |
| Home page + start-page shortcut EDITING *(2026-10-01)* | card Preferences rows (name, address, remove) AND `about:preferences` | NOT in this panel by design: edited in the shared `about:preferences` page (menu **Settings**), adopted back from the page's url fragment (`JihadChromePrefs.adoptFromUrl`) | Ported (single editor — cavekit-mochi-ui.md R7) |
| Settings page entry *(2026-10-01)* | app menu → `about:preferences` | menu **Settings** → `about:preferences#chrome=…` | Ported |
| Clear Bookmarks / Clear History | `Preferences` prompts | Preferences → `clearBookmarks/clearHistory` (db8) | Ported |
| **Apply** toggles to the live engine | `Browser.setEnableJavascript/setBlockPopups/setAcceptCookies` | — | Not applicable since 2026-10-02: the toggles are gone from both shells |
| Enable Flash *(2026-10-02)* | `Preferences` switch → own service `enableFlash`/`disableFlash` | `JihadPreferences` switch → own service `getFlash`/`enableFlash`/`disableFlash` | Ported — a real switch in both shells now (it used to write the system preference `flashplugins`, which nothing in this browser reads) |
| Default search-engine selector *(2026-10-02)* | `Preferences` ListSelector, `com.palm.universalsearch` | `JihadPreferences` engine rows (DuckDuckGo, Google, Wikipedia — the Enyo shell's fallback list) → `enyo.jihad.saveSearchEngine`; `JihadBrowser.searchUrl` uses it | Ported — the choice is this card's own; it does not change the system-wide search engine |
| Confirmation prompt before a clear *(2026-10-02)* | `BrowserPreferencePrompt` | `JihadPreferences` / `JihadHistoryList` confirmation overlay, Enyo's wording | Ported |

## Engine-driven dialogs

> **Answer path — live end to end since 2026-08 (corrected 2026-10-01).** The previous revision of
> this note (audit F-A12) said the daemon installed no `DialogSink` and that the SSL sink passed an
> empty `syncPipePath`. Both were fixed on the daemon side: `BrowserPageGoanna` is the production
> dialog sink over the frozen FIFO contract (cavekit-browser-services.md R3) and the SSL confirm
> has carried a reply pipe since 2026-08-03. What remains true: Mochi is more correct than Enyo 1.0
> here — `Browser.sendDialogResponse` calls `viewCall("acceptDialog"/"cancelDialog")`, names that
> exist in neither `BasicWebView` nor the adapter's method table. Device evidence on THIS variant
> is an `alert()` reaching the card (2026-08); confirm / prompt / auth / SSL on Mochi are
> code-equivalent and have not each been watched on a device.

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| Alert | `Browser.showAlertDialog` (`onAlertDialog`) | `JihadWebView.dialogAlert` → `JihadDialogs.showAlert` → `sendDialogResponse(["1"])` | Ported |
| Confirm | `Browser.showConfirmDialog` | `dialogConfirm` → `showConfirm` → `["1"]`/`["0"]` | Ported |
| Prompt | `Browser.showPromptDialog` | `dialogPrompt` → `showPrompt` → `["1", text]`/`["0"]` | Ported |
| HTTP auth (user/password) | `Browser.showUserPasswordDialog` | `dialogUserPassword` → `showLogin` → `["1", user, pass]`/`["0"]` | Ported |
| SSL confirm (trust always/once/deny) | `Browser.showSSLConfirmDialog` | `dialogSSLConfirm` → `showSSL` → `["1"]`/`["2"]`/`["0"]` | Ported |
| SSL "View Certificate" detail *(2026-10-02)* | `CertificateDetail.js` (`com.palm.certificatemanager`) | `JihadDialogs` **View Certificate** → details the daemon writes into this app's `certinfo/` folder while the warning is open | Ported — by a different route: on webOS CE a card cannot reach the certificate manager at all, so the Enyo button never enables there |
| Page/engine error dialogs | `Browser.browserError` | the engine's own alert, presented by `JihadDialogs.showAlert` | Ported in effect *(corrected 2026-10-02: a failed load is NOT silent — the engine raises an alert naming the failure and the card shows it; seen on the device 2026-10-01)* |

## Context menu, sharing, launcher, print (intentional omissions)

| Feature | Enyo 1.0 source | Status |
| --- | --- | --- |
| Long-press context menu *(2026-10-02)* | `BrowserContextMenu.js`, `Browser.openContextMenu`, `saveImageAtPoint` | Ported — `JihadWebView.eventFired` (hold on a link or image) → action sheet: Open in New Card, Share Link, Copy Address; Save Image, Share Image, Set as Wallpaper. Images are fetched by the system download manager into the downloads folder (the daemon's `saveImageAtPoint` is a stub), so an inline `data:` image cannot be saved. |
| Share link sheet *(2026-10-02)* | `ShareLinkDialog.js` | Ported — toolbar share → Email, Text Message, Copy Address (no Facebook entry: the app it launched is not on webOS CE). |
| Add to Launcher *(2026-10-02)* | `BrowserApp.addToLauncher` | Ported — in the share sheet; `applicationManager/addLaunchPoint` with this app's icon. |
| Print *(2026-10-05)* | was `Browser.printFrame` + the system `PrintDialog`; now `JihadPrintDialog.js` in BOTH shells | Ported — menu → Print opens `JihadPrintDialog`: printers the daemon finds are listed as each answers, an address can be typed for one discovery cannot see, a tap prints, the dialog shows progress and the result. Not the system dialog on purpose: webOS's print daemon discards every printer that is not HP (it found the owner's Epson and logged "not HP printer and doing printer filtering"), and the daemon's `printFrame` is a stub, so the Enyo shell's stock Print could not print on anything either — it now uses the same dialog. The daemon prints the page as JPEG sheets over IPP (`render/goanna/components/jihadPrint.js`). |

## R2 acceptance summary

- Address/search bar with back/forward/reload/stop — **met** (shell).
- Bookmarks, history, downloads views present and functional — **met** (downloads auto-initiation noted as omitted; management is functional).
- Find-in-page, preferences, start page present — **met**.
- Alert/confirm/prompt/auth + SSL-confirm dialogs presented and answerable — **met**.
- Parity checklist with no missing feature — **met as of 2026-10-05**: every Enyo feature is Ported, Print last.

~~On-device functional verification (rendering, live db8/download-manager/adapter round-trips) is
**DEVICE-GATED** and pending hardware.~~ **CORRECTED 2026-08-10 — that has been false since
2026-08-05.** The Mochi variant is installed and running on the TouchPad, renders live pages, and
runs alongside the Enyo and Mojo variants (`device-independence-test.sh` 24/24 with all three
installed). It was left standing after the device work landed, which would have asked a reviewer
to sign a statement the project had already disproved.

Structural correctness — kinds, the frozen adapter method set, `node --check`, grep-clean, and the
`build-mochi-ipk.sh` end-to-end build — is verified, and remains the right gate for this document.
Per-criterion status lives in `context/kits/cavekit-mochi-ui.md`, not here.

**2026-10-05.** Print was built and exercised on the device the same way (`context/impl/evidence-2026-10-05/`, `build/webos-oe/card-selftest.sh mochi run print[:address]`), against the owner's real printers for discovery and against a test sink for the sheets. Share Image and Set as Wallpaper were also run through the long-press sheet's own row handler on the device: the wallpaper was set (and put back), and Share Image fetched the picture and opened Email.

**Owner review, 2026-10-02.** The owner reviewed the omissions and simplifications this document listed on 2026-10-01 and accepted none of them; all but Print have since been built and exercised on the device (`context/impl/evidence-2026-10-02/`, driven by `build/webos-oe/mochi-selftest.sh`). What that evidence does not cover is a finger reaching each control — those steps are on the human checklist.
