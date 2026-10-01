<!-- Copyright 2026 NotAlexNoyle. Apache-2.0; see ../LICENSE. -->

# Mochi UI parity checklist (T-053 / cavekit-mochi-ui.md R2)

Maps every user-facing feature of the Enyo 1.0 UI (`../app/source`) to its Enyo 2
+ Mochi equivalent in `app-mochi/source`, or records an intentional omission.

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
| List saved bookmarks | `BookmarkList.js` (DbService find/subscribe) | `JihadBookmarkList.js` (`enyo.jihad.dbFind` on bookmarks kind) | Ported |
| Open a bookmark | `BookmarkList.itemClick` → `selectItem` | `JihadBookmarkList` `onSelectItem` → `JihadBrowser.navigateTo` | Ported |
| Delete a bookmark | `BookmarkList` swipe → `deleteBookmark` | row **Delete** → `enyo.jihad.dbDel` | Ported |
| Add current page as bookmark | `BrowserApp.addBookmark` | menu **Add Bookmark** → `JihadBrowser.addCurrentBookmark` (db8 put, same record shape) | Ported |
| Clear all bookmarks | `Preferences` → `BrowserApp.clearBookmarks` | Preferences **Clear Bookmarks** → `JihadBrowser.clearBookmarks` (db8 delByQuery) | Ported |
| Edit bookmark (rename / re-URL) | `BookmarkDialog.js`, `BrowserApp.editBookmark` | — | Omitted — delete + re-add covers the need; a dedicated edit dialog is deferred. |
| Bookmark thumbnails / 32-64px icons | `BrowserApp.createPageImages` (`saveViewToFile`/`generateIconFromFile`) | — | Omitted — thumbnail generation uses non-frozen engine node methods and a writable `/var/luna/data/browser/icons`; bookmarks store title + URL only. |

## History

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| List history (most recent first) | `HistoryList.js` (find orderBy date) | `JihadHistoryList.js` (`dbFind` orderBy `date`, desc) | Ported |
| Record a visit on load-stop | `BrowserApp.updateHistory` | `JihadBrowser.updateHistory` (delByQuery url → put; same kind/shape) | Ported |
| Open a history entry | `HistoryList.itemClick` | `onSelectItem` → `navigateTo` | Ported |
| Delete a history entry | `HistoryList` swipe | row **Delete** → `dbDel` | Ported |
| Clear all history | `BrowserApp.clearHistory` | header **Clear** / Preferences **Clear History** → `JihadBrowser.clearHistory` (db8 delByQuery) | Simplified — clears the db8 kind only. Enyo 1.0 additionally clears the ENGINE's own history (`Browser.clearHistory` → `viewCall("clearHistory")`), so back/forward and global history survive a Mochi clear. Adding it would widen the frozen `callBrowserAdapter` literal set (audit F-A14a). |
| Relative-date dividers ("Last Week", month names) | `HistoryList.getDivider` (`enyo.g11n`) | inline `M/D/YY` date label | Simplified — `enyo.g11n` is not in the Enyo 2 stack; a compact date replaces the grouped dividers. |

## Downloads

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| List downloads + status/progress | `DownloadList.js`, `BrowserApp` download services | `JihadDownloadList.js` + `JihadBrowser.refreshDownloads` (`getAllHistory`) | Ported |
| Open a completed download | `BrowserApp.openDownloadedFile` | `onOpenItem` → `JihadBrowser.openDownload` (`applicationManager/open`) | Ported |
| Cancel an in-progress download | `BrowserApp.cancelDownload` | `onCancelItem` → `cancelDownload` (`downloadmanager/cancelDownload`) | Ported |
| Clear the downloads list | `BrowserApp.clearDownloads` | header **Clear** → `clearDownloads` (`downloadmanager/clearHistory`) | Ported |
| Auto-start a download for a non-renderable resource | `BrowserApp.handleResource`/`downloadResource` (`onUrlRedirected`/`onFileLoad`, `resourceInfoService`) | — | Omitted — auto-initiation needs WebView resource-handoff callbacks not yet surfaced by `JihadWebView` (navigation-events scope). The list/open/cancel/clear management is functional against the download manager. |
| Retry a failed download | `DownloadList.itemRetry` | — | Omitted — follows the auto-initiation omission above. |

## Find in page

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| Find bar (query, prev, next, done) | `FindBar.js` | `JihadFindBar.js` | Ported |
| Run the search | `Browser.find` → `callBrowserAdapter("findInPage")` | `JihadFindBar` `onFind` → `JihadBrowser.findRequested` → `find` → `callBrowserAdapter("findInPage")` | Ported |
| Next / Prev match | `Browser.goToNext/goToPrevious` (no-ops in Enyo 1.0) | re-issue the current query (advances the adapter match) | Ported (matches Enyo 1.0 behaviour) |

## Preferences

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| Block Popups / Accept Cookies / Enable JavaScript toggles | `Preferences.js` ToggleButtons | `JihadPreferences.js` `mochi.ToggleButton`s | Ported (persistence) |
| Persist preferences | `BrowserApp.preferenceChanged` (db8 merge) | `JihadPreferences.toggleChanged` (`dbMergeQuery` on preferences kind) + default seeding | Ported |
| Clear Cookies / Clear Cache | `Browser.clearCookies/clearCache` (`palm://net.riverstonerelay.jihadBrowser/…`) | Preferences → `JihadBrowser.clearCookies/clearCache` (`palm://net.riverstonerelay.jihadBrowserMochi/…`) *(2026-10-01: own-service URIs on both sides)* | Ported |
| Home page + start-page shortcut EDITING *(2026-10-01)* | card Preferences rows (name, address, remove) AND `about:preferences` | NOT in this panel by design: edited in the shared `about:preferences` page (menu **Settings**), adopted back from the page's url fragment (`JihadChromePrefs.adoptFromUrl`) | Ported (single editor — cavekit-mochi-ui.md R7) |
| Settings page entry *(2026-10-01)* | app menu → `about:preferences` | menu **Settings** → `about:preferences#chrome=…` | Ported |
| Clear Bookmarks / Clear History | `Preferences` prompts | Preferences → `clearBookmarks/clearHistory` (db8) | Ported |
| **Apply** toggles to the live engine | `Browser.setEnableJavascript/setBlockPopups/setAcceptCookies` | — | **Omitted (contract)** — pushing toggle values to the engine would require new `callBrowserAdapter` methods, widening the frozen literal set (cavekit-ipc-contract R1). Values are persisted to db8; live engine application is deferred to a contract-scope change. |
| Enable Flash (system pref) | `Preferences` `flashplugins` | — | Omitted — system-preference/Flash plumbing is out of parity-views scope. |
| Default search-engine selector | `Preferences` ListSelector, `com.palm.universalsearch` | — | Omitted — the address bar uses a fixed default search (`JihadBrowser.searchUrl`, DuckDuckGo), matching the shell. |
| Confirmation prompt before a clear | `BrowserPreferencePrompt` | immediate action (button label is explicit) | Simplified — clears act immediately; the extra confirm popup is dropped. |

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
| SSL "View Certificate" detail | `CertificateDetail.js`, `Browser.viewSSLCertificate` | — | Omitted — the security-critical trust decision is fully answerable; the X.509 detail viewer is deferred. |
| Page/engine error dialogs | `Browser.browserError` | — | Omitted — `JihadBrowser.openDialog` (a generic info popup) exists but NOTHING routes engine errors to it: `JihadWebView` handles none of the adapter's `failedLoad` / `setMainDocumentError` / `reportError` callbacks, so a failed load is silent. Surfacing them is navigation-events scope (audit F-A14c). |

## Context menu, sharing, launcher, print (intentional omissions)

| Feature | Enyo 1.0 source | Status |
| --- | --- | --- |
| Long-press context menu (copy/share link, copy-to-photos, set wallpaper, share image) | `BrowserContextMenu.js`, `Browser.openContextMenu`, `saveImageAtPoint` | Omitted — needs the WebView `onMousehold`/tap-info + `saveImageAtPoint`/wallpaper node methods (not surfaced; outside the parity-views + frozen-set scope). |
| Share link sheet (email / messaging) | `ShareLinkDialog.js`, `com.palm.stservice` | Omitted — reduced to a "copy the address to share" note; the multi-channel share sheet is deferred. |
| Add to Launcher | `BrowserApp.addToLauncher` (`addLaunchPoint` + page images) | Omitted — depends on the omitted thumbnail generation and `applicationManager/addLaunchPoint`. |
| Print | `Browser.printFrame`, `PrintDialog` | Omitted — `printFrame` is a non-frozen engine node method and `PrintDialog` is not in the Mochi set. |

## R2 acceptance summary

- Address/search bar with back/forward/reload/stop — **met** (shell).
- Bookmarks, history, downloads views present and functional — **met** (downloads auto-initiation noted as omitted; management is functional).
- Find-in-page, preferences, start page present — **met**.
- Alert/confirm/prompt/auth + SSL-confirm dialogs presented and answerable — **met**.
- Parity checklist with no undocumented missing feature — **this document**.

~~On-device functional verification (rendering, live db8/download-manager/adapter round-trips) is
**DEVICE-GATED** and pending hardware.~~ **CORRECTED 2026-08-10 — that has been false since
2026-08-05.** The Mochi variant is installed and running on the TouchPad, renders live pages, and
runs alongside the Enyo and Mojo variants (`device-independence-test.sh` 24/24 with all three
installed). It was left standing after the device work landed, which would have asked a reviewer
to sign a statement the project had already disproved.

Structural correctness — kinds, the frozen adapter method set, `node --check`, grep-clean, and the
`build-mochi-ipk.sh` end-to-end build — is verified, and remains the right gate for this document.
Per-criterion status lives in `context/kits/cavekit-mochi-ui.md`, not here.

**What a sign-off of this document asserts (2026-10-01):** that every user-facing feature of the
Enyo 1.0 shell appears above as Ported, Simplified or Omitted with its reason, as of the sources on
that date. It is a product-owner judgement on the OMISSIONS (bookmark editing, thumbnails,
auto-download, share sheet, add-to-launcher, print, certificate detail, live-apply of the three
toggles, the Flash and search-engine preferences) and is not something a device run can decide.
