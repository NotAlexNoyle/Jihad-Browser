<!-- Copyright 2026 the Jihad Browser project. Apache-2.0; see ../LICENSE. -->

# Mochi UI parity checklist (T-053 / cavekit-mochi-ui.md R2)

Maps every user-facing feature of the Enyo 1.0 UI (`../app/source`) to its Enyo 2
+ Mochi equivalent in `app-mochi/source`, or records an intentional omission.

**Contract note.** The Mochi UI drives the engine ONLY through the frozen
`callBrowserAdapter` literal set `{findInPage, goBack, goForward, reloadPage,
stopLoad}` plus the `palm://com.palm.browserServer/{clearCache,clearCookies}`
URIs. Dialog answers use the adapter's existing scriptable `sendDialogResponse`
routed through JihadWebView's variable-method `_call` path (exactly as the Enyo
1.0 app's `viewCall(...)` fallback does), so they do **not** widen that grep-audited
literal set. Persistence uses the same Jihad-owned db8 kinds the Enyo 1.0 app
uses: `net.riverstonerelay.jihad-browser.{history,bookmarks,preferences}:1`.

Status legend: **Ported** = feature-equivalent · **Simplified** = present with a
documented reduction · **Omitted** = intentionally not built (rationale given).

## Core browsing

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| Address / search bar | `AddressInput.js`, `URLSearch.js` | `JihadBrowser.js` toolbar (`address` Input, `normalizeUrl`) | Ported (shell) |
| Back / Forward / Reload / Stop | `Browser.js`, `ActionBar.js` | `JihadBrowser.js` `goBack/goForward/reloadPage/stopLoad` → `callBrowserAdapter` | Ported (shell) |
| Load progress | `Browser.js` `loadProgress` | `JihadBrowser.js` `mochi.ProgressBar` | Ported (shell) |
| Start page | `StartPage.js` | `JihadBrowser.js` app-chrome overlay | Ported (shell) |
| New card | `Browser.openNewCard` | `JihadBrowser.doNewCard` (`enyo.windows.openWindow`) | Ported |
| Reflect title/URL + history state | `Browser.pageTitleChanged` | `JihadBrowser.pageInfoChanged` | Ported (shell) |

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
| Clear all history | `BrowserApp.clearHistory` | header **Clear** / Preferences **Clear History** → `JihadBrowser.clearHistory` | Ported |
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
| Clear Cookies / Clear Cache | `Browser.clearCookies/clearCache` | Preferences → `JihadBrowser.clearCookies/clearCache` (`browserServer` URIs) | Ported |
| Clear Bookmarks / Clear History | `Preferences` prompts | Preferences → `clearBookmarks/clearHistory` (db8) | Ported |
| **Apply** toggles to the live engine | `Browser.setEnableJavascript/setBlockPopups/setAcceptCookies` | — | **Omitted (contract)** — pushing toggle values to the engine would require new `callBrowserAdapter` methods, widening the frozen literal set (cavekit-ipc-contract R1). Values are persisted to db8; live engine application is deferred to a contract-scope change. |
| Enable Flash (system pref) | `Preferences` `flashplugins` | — | Omitted — system-preference/Flash plumbing is out of parity-views scope. |
| Default search-engine selector | `Preferences` ListSelector, `com.palm.universalsearch` | — | Omitted — the address bar uses a fixed default search (`JihadBrowser.searchUrl`, DuckDuckGo), matching the shell. |
| Confirmation prompt before a clear | `BrowserPreferencePrompt` | immediate action (button label is explicit) | Simplified — clears act immediately; the extra confirm popup is dropped. |

## Engine-driven dialogs

| Feature | Enyo 1.0 source | Mochi equivalent | Status |
| --- | --- | --- | --- |
| Alert | `Browser.showAlertDialog` (`onAlertDialog`) | `JihadWebView.dialogAlert` → `JihadDialogs.showAlert` → `sendDialogResponse(["1"])` | Ported |
| Confirm | `Browser.showConfirmDialog` | `dialogConfirm` → `showConfirm` → `["1"]`/`["0"]` | Ported |
| Prompt | `Browser.showPromptDialog` | `dialogPrompt` → `showPrompt` → `["1", text]`/`["0"]` | Ported |
| HTTP auth (user/password) | `Browser.showUserPasswordDialog` | `dialogUserPassword` → `showLogin` → `["1", user, pass]`/`["0"]` | Ported |
| SSL confirm (trust always/once/deny) | `Browser.showSSLConfirmDialog` | `dialogSSLConfirm` → `showSSL` → `["1"]`/`["2"]`/`["0"]` | Ported |
| SSL "View Certificate" detail | `CertificateDetail.js`, `Browser.viewSSLCertificate` | — | Omitted — the security-critical trust decision is fully answerable; the X.509 detail viewer is deferred. |
| Page/engine error dialogs | `Browser.browserError` | `JihadBrowser.openDialog` (generic info popup) available | Simplified — a generic info dialog exists; per-error-code mapping requires the WebView `onError` callback (navigation-events scope). |

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

On-device functional verification (rendering, live db8/download-manager/adapter
round-trips) is **DEVICE-GATED** and pending hardware. Structural correctness —
kinds, the frozen adapter method set, `node --check`, grep-clean, and the
`build-mochi-ipk.sh` end-to-end build — is verified.
