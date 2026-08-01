---
created: "2026-07-19"
last_edited: "2026-07-31"
---
# Implementation Tracking: mochi-ui

Build site: context/plans/build-site.md

| Task | Status | Notes |
|------|--------|-------|
| T-049 | DONE (desktop) — device residue open | 2026-07-18, commit c6d0e9a. appinfo title → "Jihad (Mochi)"; icons md5-match app/; `build/webos-oe/build-mochi-ipk.sh` bundles Enyo2 core (`../mochi-sampler/enyo`) + layout (`webos-stacks/mochi/lib/layout` — mochi-sampler lib/ dirs EMPTY; `LAYOUT_SRC` override) + Mochi (`../mochi`) → 1.4 MB ipk, 394 entries, verified rebuild on merged main. Dev dirs pruned (~12 MB cut). Dual-install on device pending hardware |
| T-050 | DONE | 2026-07-19, commit 2a79d71. Apache headers on all new app-mochi files; NOTICE gained Enyo 2 core + layout + Mochi (LG, Apache-2.0) credits, confirmed inside packaged ipk (closes codex F-390) |
| T-051 | DONE (desktop) — live handshake device-gated | 2026-07-19, commit 2a79d71. JihadWebView.js: NPAPI <object type="application/x-jihad-browser">, callBrowserAdapter surface frozen (set identical to app/: findInPage/goBack/goForward/reloadPage/stopLoad; Luna URIs clearCache/clearCookies), node.eventListener wiring, arg orders checked against render/adapter/BrowserAdapter.cpp |
| T-052 | DONE (desktop) — on-device layout review open | 2026-07-19, commit 2a79d71. Shell from mochi.Header/IconButton/InputDecorator/Input/ProgressBar/Popup + Fittable layout; inline SVG data-URI glyphs; no hardcoded px beyond shared 1024x768 |
| Audit | DONE (desktop) | 2026-07-31. Adversarial re-audit of `app-mochi/` (re-derives the lost F-434..443 as F-A01..F-A22) + the Task A start-page address fix. 11 defects fixed in-tree, 9 reported, 4 audited clean — see "Audit 2026-07-31" below. Frozen adapter set re-verified unchanged; ipk rebuilds. On-device verification DEVICE-GATED (no hardware this session) |
| T-053 | PARTIAL | Shell built + polished on-device: Enyo-parity toolbar (back/forward, address with inline reload/stop, share, new-tab, history+bookmarks; PNG icon set), app-chrome crisp start page, no-autocap URL input, stageReady card-open. Remaining: bookmarks/history/downloads/find/prefs VIEWS + dialog set. Original: Feature-parity port: views (bookmarks/history/downloads/find/prefs/start page) + dialogs + full page/dialog/download callback surface + live-daemon nav wiring |

## Notes
- Framework sources stay outside repo; bundle at build time only.
- `../mochi-sampler/lib/{layout,mochi}` are empty dirs — real layout lives at
  `webos-stacks/mochi/lib/layout`; its enyo + mochi trees byte-identical to the
  packet sources (2.5.1-pre.1).

---

## Audit 2026-07-31 — adversarial review of `app-mochi/`

Re-derivation of the lost Codex cycle-2 findings F-434..443 (never written to a
file), done by re-auditing the current tree against `render/adapter/BrowserAdapter.cpp`,
`render/browserserver/`, `render/goanna/`, the Enyo 1.0 app (`app/source`,
`app/db`), the bundled Enyo 2 / Mochi framework sources (`third_party/`), and
`docs/IPC-CONTRACT.md`. Findings are renumbered **F-A01..F-A22** (the original
F-434..443 numbering is unrecoverable). Everything marked *fixed* is in the same
commit as this section; everything marked *reported* is deliberately NOT
half-fixed — each is either cross-package, daemon-side, or a judgment call.

**Contract re-verified after the fixes:** the `callBrowserAdapter` literal set is
still exactly `{findInPage, goBack, goForward, reloadPage, stopLoad}` and the
Luna URI set still `{clearCache, clearCookies}` — `grep -o` diff against
`app/source/` is empty in both directions. `node --check` passes on all 10
`app-mochi/source/*.js`; `build/webos-oe/build-mochi-ipk.sh` rebuilds the ipk
(1413 KiB, 406 entries). **No on-device verification this session (no hardware);
every behavioural claim below is desktop/static reasoning.**

### Blocking / high

| # | Sev | Finding | Disposition |
|---|-----|---------|-------------|
| **F-A01** | High | **db8 is unreachable from the Mochi package as shipped.** The kinds `net.riverstonerelay.jihad-browser.{bookmarks,history,preferences}:1` are declared in `app/db/kinds/*` with `owner: net.riverstonerelay.jihad-browser`, and `app/db/permissions/*` grants create/read/update/delete to exactly one `caller`: `net.riverstonerelay.jihad-browser` (plus read for `com.palm.launcher`). This package's app id is `net.riverstonerelay.jihad-browser.mochi` — a *different* db8 caller — and `app-mochi/` ships **no `db/` directory at all**. So: installed alone, the kinds do not exist and every call fails; installed alongside the Enyo variant, the kinds exist but db8 denies the `.mochi` caller. Bookmarks, history and preferences are all affected, i.e. most of cavekit-mochi-ui R2. | **FIXED 2026-07-31 (orchestrator, device-gated).** `app/db/permissions/*` (all 3 kinds) gained explicit full-CRUD grant objects for callers `net.riverstonerelay.jihad-browser.mochi` and `.mojo` — explicit ids, NOT the trailing wildcard (wildcard-caller matching on this db8 build is unverified; explicit callers are deterministic). Kinds stay Enyo-owned (a kind's owner must equal the registering app id, so the Mochi package can never own/ship them) → db8 features in Mochi REQUIRE the Enyo variant installed; documented in PARITY.md. `register-db-kinds.sh` pushes permissions from these same files, so a re-run applies the grant to an already-registered device. Verify on device: `luna-send -a net.riverstonerelay.jihad-browser.mochi luna://com.palm.db/find '{"query":{"from":"net.riverstonerelay.jihad-browser.history:1"}}'` must not return -3963. **SUPERSEDED 2026-08-01 — this whole row is a historical record and its ids are stale on purpose.** The permission-grant fix was itself the R7 co-ownership violation and was replaced by separate per-variant namespaces (`impl-review-findings-independence.md` F-1); the Mochi app id then lost its dot (`net.riverstonerelay.jihad-browser-mochi`) because ipkg's `<pkgid>.*` removal glob made the dotted form a child of the Enyo package (`impl-ipkg-prefix-collision.md`). The current shape is in `../plans/plan-variant-identity.md`; do not copy the ids or the `luna-send` line out of this row. |
| **F-A02** | High | **Preferences were silently reset to `false` on every launch.** `mochi.ToggleButton.valueChanged()` calls `doChange({value})` (despite its doc comment claiming it does not fire programmatically), and its `rendered()` calls `valueChanged()` **twice**. `JihadPreferences` is a `showing:false` child of the shell, so Enyo still renders it at app start → all three toggles fired `onChange` with their default `false` → `toggleChanged` ran `dbMergeQuery(preferences, key=…, {value:false})` before `load()` had ever read the stored values. | **Fixed** — `_applying` now starts `true` in `JihadPreferences.create()` and is cleared only after `load()` applies (or seeds) the stored prefs, so the render-time burst is ignored and user taps still persist. |

### Medium

| # | Sev | Finding | Disposition |
|---|-----|---------|-------------|
| **F-A03** | Med | **"New card" was a no-op.** `doNewCard` called `enyo.windows.openWindow("index.html")`. `enyo.windows` is an **Enyo 1.0 / webOS-framework** API and does not exist in the bundled Enyo 2 stack (`grep` clean across `third_party/mochi-sampler/enyo`); the `if (window.enyo && enyo.windows)` guard turned the toolbar new-tab button *and* the "New Card" menu item into silent no-ops. PARITY.md claimed "Ported". | **Fixed** — new `openCard(params)` opens the card the way `enyo.windows`' own agent does (`windows/agent.js`): `window.open(url, "", 'attributes={"window":"card"}')`, keeping `enyo.windows` as a fallback if a host provides it. Params ride in `?enyoWindowParams=<json>`, which the new `processQueryString()` reads back (BrowserApp.processQueryString parity). **Device-gated.** |
| **F-A04** | Med | **No `browserServerDisconnected` handler.** The adapter invokes it once whenever BrowserServer goes away (daemon restart/crash — a recurring event in this project's device logs). `BasicWebView` uses it to drop `_serverConnected`. `JihadWebView` did not, so after a daemon bounce the view still believed it was connected: every `_call` was fired at a dead node and lost instead of being queued, and `connect()` was never retried — the card stayed permanently dead until relaunch. | **Fixed** — added the callback (clears `_serverConnected`, emits the new `onServerDisconnected`, retries `connect()`). |
| **F-A05** | Med | **Engine-created pages were dropped.** The adapter invokes `createPage(identifier)` (`msgCreatePage`) for links with a `target` and for `window.open`. Enyo 1.0 routes it to a new card (`BasicWebView.createPage` → `Browser.openNewCardWithIdentifier`). `JihadWebView` had no such callback, so those links did nothing at all. | **Fixed** — `createPage` → `onNewPage` → `JihadBrowser.newPageRequested` → `openCard({webviewId: id})`; the new card binds the identifier via `readLaunchParams`. **Device-gated.** |
| **F-A06** | Med | **Task A — start-page URL in the address bar / history.** Reported on device 2026-07-19: the bar showed the raw `data:` URL of the start page. Root cause is historical (commit `141e28d` loaded the start page into the WebView as a `data:` document; the engine reported that URL back through `titleAndUrlChanged`/`locationChanged` and `pageInfoChanged` wrote it straight into the input, then `loadStopped` → `updateHistory` wrote it into db8). Commit `09c050c` moved the start page to app chrome, which removed the *current* trigger but left both sinks unguarded — any inline document the shell loads still lands in the bar and in history. | **Fixed** — see "Task A" below. |
| **F-A12** | Med | **Dialog answers cannot reach the engine (daemon-side).** `JihadBrowserServer`'s `ProxySink::msgSSLConfirm` sends `msgDialogSSLConfirm(mProxy, "", host, code, certFile)` — an **empty** `syncPipePath` — and `BrowserAdapter::js_sendDialogResponse` bails with `"Invalid state"` when `strlen(gDialogResponsePipe)==0`, so the SSL trust decision is discarded. Worse, no production `DialogSink` is installed at all: `render/goanna/DialogService.cpp` dispatches to `gSink`, which only the desktop test sets, so `msgDialogAlert/Confirm/Prompt/UserPassword` are **never emitted** — `alert()`/`confirm()`/`prompt()` in content are answered with the engine-side default and the app dialogs never appear. The app-side wiring is correct (F-A13); the path is not live. | **Reported.** Daemon work (`render/browserserver/JihadBrowserServer.cpp`, `render/goanna/DialogService.*`), outside this task's file scope. PARITY.md's dialog section now carries the caveat. |

### Low

| # | Sev | Finding | Disposition |
|---|-----|---------|-------------|
| **F-A07** | Low | Out-of-order engine callbacks read undefined guards: `loadProgress` compared `p >= this._lastProgress`, and `_lastProgress` was only initialised in `loadStarted` — a progress message arriving before load-start left the bar frozen at 0 for the whole load. `_loading` and `_lastSubmit` had the same lazy-init smell. | **Fixed** — all three initialised in `JihadBrowser.create()`. |
| **F-A08** | Low | The five engine-driven dialog handlers dereferenced `inEvent` unguarded (`inEvent.message`, `inEvent.host`, …), unlike every other handler in the file. A callback delivered without a payload would throw *inside* the handler, leaving content blocked on a dialog that never presents. | **Fixed** — all five guarded. |
| **F-A09** | Low | `JihadWebView._call` pushed onto `this._callQueue` after `destroy()` had set it to `null` → `TypeError` on a late adapter callback into a torn-down view. | **Fixed** — queue push guarded. |
| **F-A10** | Low | `published: {items: []}` (bookmarks/history) and `{downloads: []}` (downloads) put a **single array on the prototype**, shared by every instance — and `deleteTapped` splices `items` in place. Latent today (one instance each, `setItems` replaces the reference first), a real aliasing bug the moment a second instance exists. | **Fixed** — each `create()` takes its own copy. |
| **F-A11** | Low | `openDownload` omitted Enyo 1.0's `!d.interrupted` check (`BrowserApp.openDownloadedFile`), so an interrupted download could be handed to `applicationManager/open`. | **Fixed.** |
| **F-A14** | Low | **PARITY.md claims the code did not back.** (a) "Clear all history — Ported": Mochi clears only the db8 kind; Enyo 1.0 *also* clears the engine's own history (`viewCall("clearHistory")`), so back/forward + global history survive a Mochi clear — and adding it would widen the frozen literal set, so it is a contract-bound reduction. (b) "New card — Ported" was false (F-A03). (c) "Page/engine error dialogs — a generic info dialog exists": nothing routes errors to it — `JihadWebView` handles none of `failedLoad` / `setMainDocumentError` / `reportError`, so `openDialog` is reachable only from Share and a failed load is completely silent. (d) dialog rows, see F-A12. | **Fixed in PARITY.md** — (a) re-labelled *Simplified* with the reason, (b) updated + a new row for `createPage`, (c) re-labelled *Omitted*, (d) caveat block added, plus the F-A01 persistence caveat at the top. |
| **F-A15** | Low | cavekit-mochi-ui **R3's** criterion "No Goanna/UXP-specific identifiers appear in `app-mochi/` *(grep clean 2026-07-19)*" is no longer true: the start-page subtitle is `"Mochi UI ★ Goanna/6.9 UXP/b2594a4"` (commit `9ca2341`, a deliberate user-requested change). It is a **display string, not an API coupling**, so the code is right and the criterion's wording is stale. | **Reported.** Do **not** remove the string. Suggested rewording: "no engine-specific *API/identifier coupling*; the engine version is display-only". Kit edit is outside this task's file scope. |
| **F-A16** | Low | `refreshDownloads` **replaces** `this.downloads` with the completed-only `getAllHistory` result, discarding any in-flight session download. Enyo 1.0 merges (`insertIntoDownloads`) and keeps in-flight entries. Harmless today only because download auto-initiation is an intentional omission (nothing ever creates an in-flight record); it becomes a real bug the moment initiation lands. | **Reported** — fixing it now would be speculative against an unbuilt feature. |
| **F-A20** | Low | `setIdentifier` after the connect handshake is never pushed to the engine: `identifierChanged` is a documented no-op and only `connect()` reads `this.identifier`, so a relaunch supplying a new `webviewId` post-handshake does not bind it. Matches Enyo 1.0 (which also only sets it in `create`). | **Reported**, left alone for Enyo-1 parity. |
| **F-A21** | Low | `JihadDialogs.showSSL` stores `this._sslCertFile` and never reads it — dead state from the intentionally-omitted "View Certificate" detail. | **Reported**, kept as the seam PARITY.md's omission row refers to. |
| **F-A22** | Low | `JihadBrowser.create()` adds `webOSRelaunch` / `mojo-relaunch` listeners that `destroy()` never removes. Cannot leak in practice (card teardown destroys the document). | **Reported**, no change. |

### Informational — audited clean

| # | Finding |
|---|---------|
| **F-A13** | **The dialog answer path is app-side correct, and deliberately diverges from Enyo 1.0.** `Browser.sendDialogResponse` calls `viewCall("acceptDialog", …)` / `viewCall("cancelDialog")`; **neither name exists** in `BasicWebView` *or* in the adapter's exposed method table (`AdapterGetMethods`), so the Enyo 1.0 alert/confirm/prompt/auth answers are dead upstream — only its SSL path uses the real `sendDialogResponse`. Mochi routes all five through `sendDialogResponse`, which **is** in the table, with exactly the shape `js_sendDialogResponse` validates: 1–3 **string** args, arg0 `"1"` accept / `"0"` cancel (SSL `"1"` trust-always / `"2"` trust-once / `"0"` don't-trust), arg1 prompt text *or* username, arg2 password. Contract-safe: it goes through the variable-method `_call` path, so the frozen `callBrowserAdapter` literal set is untouched. |
| **F-A17** | **XSS / injection: clean.** Every page-controlled string (title, URL, alert/confirm/prompt text, SSL host, download filename) reaches the DOM via `setContent` on controls with the default `allowHtml:false`, and Enyo 2's `HTMLStringDelegate` escapes both content (`enyo.dom.escape`) and attribute values (`escapeAttribute`). The only `allowHtml:true` controls are the six toolbar icon buttons, whose sole dynamic writer (`setStopVisible`) interpolates a compile-time constant. Address text goes through `enyo.Input`'s value property, not markup. No `innerHTML`, `eval`, `document.write`, or `javascript:` construction anywhere in `app-mochi/source` (`normalizeUrl` explicitly routes `javascript:`/`vbscript:` to search rather than navigating). |
| **F-A18** | **db8 record shapes round-trip with the Enyo app** (subject to F-A01). bookmarks `{_kind,title,url,date,lastVisited,defaultEntry,visitCount,idx}` matches `BrowserApp.addBookmark` field-for-field (minus the intentionally-omitted thumbnail fields); history `{_kind,url,title,date}` matches `BrowserApp.updateHistory`; preferences `{_kind,key,value}` matches `gotInitialBrowserPreferences`, including the same four seeded defaults. The `delByQuery(url=…) → put` ordering matches too. Queries hit declared indexes: `dbFind(history, orderBy:"date")` → the `date` index, `dbMergeQuery(preferences, key=…)` → the `key` index. |
| **F-A19** | **`app-mochi/README.md` contradicted hard-won device knowledge and a user directive** — it advertised `mochi.Popup` among the controls in use (Popup **crashes** the card), stated the launcher title is "Jihad (Mochi)" (the directive is "Jihad Browser" for *both* variants, which `appinfo.json` already gets right), and described the app as a bare shell with the parity port still "next wave". **Fixed** — rewritten, with the Popup/icon constraints promoted to a callout. |
| — | **Adapter callback arg orders re-verified** against `BrowserAdapter.cpp`: `titleURLChange(title,url,canGoBack,canGoForward)` = args[0..3] of `msgTitleAndUrlChanged`; `urlChange(uri,canGoBack,canGoForward)` = `msgLocationChanged`; `loadProgress(p)` = 1 arg; `editorFocused(focused,fieldType,fieldActions)` = `msgEditorFocused`; `dialogSSLConfirm(host,code,certFile)` = `msgDialogSSLConfirm` (code is an INT32 variant, so `JihadDialogs.sslMessage`'s numeric banding is correct). Every WebView-internal `_call` name (`openURL`, `setPageIdentifier`, `connectBrowserServer`, `interrogateClicks`, `setShowClickedLink`, `pageFocused`, `setVisibleSize`, `sendDialogResponse`) is present in `AdapterGetMethods`. |

### Task A — start-page address (fix detail)

`JihadBrowser` now gives the start page an explicit URL identity instead of
treating whatever the engine reports as a location:

- `startPageUrl: "about:jihad"` — the name the **engine itself** uses for that
  page (`BrowserPageGoanna::jihadAboutPage` renders it as inline HTML and sets
  `mAliasUrl` so `emitLocationAndTitle` reports `about:jihad`, not the underlying
  `data:` URL).
- `_startPageDocUrl` — the **exact** URL string, recorded by
  `rememberStartPageDoc()` on every shell-initiated navigation
  (`urlChanged` + `openUrl`), whenever the shell points the view at an inline
  `data:` document. Null while the start page is app chrome, which is the
  current design.
- `addressTextFor(url)` — what the bar shows: `""` when nothing is loaded
  (matching Enyo 1.0's `BrowserApp.startPageShown → startPage.setUrl("")`),
  `"about:jihad"` when the view is showing the start-page document, the URL
  verbatim otherwise. Used by both `pageInfoChanged` and `urlChanged`.
- `isTransientUrl(url)` — true for nothing-loaded, the start-page document, and
  **any** inline `data:` document; `updateHistory` and `addCurrentBookmark` both
  bail on it, so a start page (or any megabyte-sized inline document) can never
  be written to db8. This mirrors the daemon, which already suppresses
  `msgUpdateGlobalHistory` for alias pages "to avoid a huge data: history entry".

**Deliberately NOT done:** matching on a `"starts with data:"` prefix. A page may
legitimately navigate to a `data:` URL, and aliasing that to `"about:jihad"`
would let content **spoof the address bar**. Suppression is by exact identity
against a URL the shell itself loaded — never by a guess about a URL the engine
reported.
