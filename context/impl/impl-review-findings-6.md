---
created: "2026-10-06"
last_edited: "2026-10-06"
---

# Cavekit Adversarial Review #6 — long-press menu, composed frames, padlock, Palefill (2026-10-05/06)

Owner instruction: *"make sure you are using cavekit 3.1 with codex adversarial review (cavekit
backup inspector if codex runs out of credits)"*. What ran, in order:

1. **Codex** (`gpt-5.6-sol`, `codex exec -s read-only`) on the long-press commit `657282c4` — 5 P1, 8 P2, 1 P3.
2. **Codex** on the rendering changes (composed frames, adapter guard) — 2 P1, 3 P2.
3. Codex hit its usage limit mid-run of the long-press re-review. **`ck:inspector`** took the
   re-review (0 P0/P1, 4 P2, 8 P3) and then the padlock + fix-round review (0 P0/P1, 2 P2, 4 P3).
4. **Codex** again on 2026-10-06 (credits back) on the Palefill installer and the last round of
   fixes — see "Round 4" below.

**Tooling finding (T-339):** Cavekit 3.1's `scripts/codex-review.sh` invokes `codex --approval-mode
full-auto --quiet -p …`; Codex CLI 0.160 rejects `--approval-mode`, the script catches the failure,
prints "Falling back to inspector-only review" and exits 0 — so with `codex_review=auto` and
`tier_gate_mode=severity` the gate had been passing without a review. Workaround used: the script's
own prompt, sent to `codex exec -m gpt-5.6-sol -s read-only -C <repo> -o <out> -` with the diff on
stdin (scratch wrapper; not committed). The plugin is the owner's install and was not edited.

## Round 1 — Codex on the long-press commit (657282c4)

| Severity | File | Line | Description |
|---|---|---:|---|
| P1 | render/browserserver/JihadBrowserServer.cpp | 984 | `pageCommand("cut")` runs synchronously from the unguarded YAP callback. The component subsequently dispatches an `input` event, allowing arbitrary page JS to navigate or close the page while `mInTick` is false—the teardown/reentrancy class that other input paths explicitly defer to `pump()`. |
| P1 | render/goanna/components/jihadPageCommands.js | 185 | `gHold` is global to the singleton component, and `target(win)` returns the last held window without verifying that it belongs to `win`. With multiple cards, a Copy or Select All issued by card A can operate on card B’s selection. |
| P1 | render/goanna/components/jihadPageCommands.js | 312-317 | Cut truncates clipboard content to `MAX_COPY` but then deletes the complete selection. It also deletes immediately after merely sending a notification, without confirmation that the card loaded the file or set the clipboard. Large selections or any card-side failure therefore cause irreversible data loss. |
| P1 | render/goanna/components/jihadPageCommands.js | 310 | The 1 MiB limit is applied only after `plainSelection()`/`selectedText()` has materialized the complete selection. `holdSelect()` also stringifies selections and enumerates every client rectangle. A page can create a huge selection that blocks the single daemon thread or exhausts memory before the limit helps. |
| P1 | app/source/Browser.js<br>app-mochi/source/JihadBrowser.js<br>app-mojo/app/assistants/main-assistant.js | 411<br>795<br>529 | Clipboard text is forwarded into the length-prefixed IPC and DOM without any size bound. Because another application controls the system clipboard, an oversized clipboard can exhaust the card or daemon on this memory-constrained device when Paste is chosen. |
| P2 | app/source/Browser.js<br>app-mochi/source/JihadBrowser.js<br>app-mojo/app/assistants/main-assistant.js | 365-374<br>745-747<br>405-410 | Broadcast `page-hold` replies carry no card/request identity. Enyo and Mochi accept any reply while a timestamp is pending, while Mojo accepts whichever reply arrives while its stage is active. Two pending holds or a card switch can open a menu for the wrong page and route its command through the wrong proxy. |
| P2 | render/goanna/GoannaRenderPage.cpp | 3737 | `EditableAt()` reuses `edIsEditable()`, which does not reject `readonly` or `disabled` controls. Such controls receive editing menus; `setRangeText()` can then programmatically delete a readonly value despite the control forbidding edits. |
| P2 | render/goanna/GoannaRenderPage.cpp | 3729 | Editability is hit-tested only in the top document. A point over an iframe resolves to the iframe element, so text inputs and content-editable regions inside frames are classified as plain content and cannot use Paste, Cut, or field-specific Select All. |
| P2 | render/goanna/BrowserPageGoanna.cpp | 2408 | The caret is repositioned only when the field was previously unfocused. A long-press elsewhere in an already-focused field leaves the old caret or selection intact, so Paste and Cut act at the old location despite the comment promising the finger location as the target. |
| P2 | render/goanna/BrowserPageGoanna.cpp | 2399-2400 | The return/default-prevented state of the page’s `contextmenu` event is discarded, and the browser menu is always raised afterward. Pages that call `preventDefault()` to implement their own context menu receive both menus. |
| P2 | app/source/clipboard.js | 35-41 | Clipboard invocation is outside any cleanup guard. If `select()`, `PalmSystem.paste()`, or `execCommand("paste")` throws, no timer is installed, the textarea remains attached, `enyo.keyboard.resume()` is never called, and the caller never receives a result. |
| P2 | render/browserserver/JihadBrowserServer.cpp | 972 | The frozen Copy response reports success whenever a page object exists, even when there is no selection, the component is absent, or writing/notifying fails. Callers using the existing Copy callback therefore receive a false success result. |
| P2 | render/goanna/test/enyo-selftest.js<br>render/goanna/test/mochi-selftest.js | 87<br>79 | The device test hooks write the first 160 clipboard characters to the persistent device log. This directly violates the prohibition on logging typed/copied field contents and can expose passwords or other clipboard secrets during testing. |
| P3 | render/goanna/components/jihadPageCommands.js | 245-250 | Cleanup depends solely on an in-process timer. A daemon crash or reboot cancels the timer and leaves the cleartext, world-readable script until a later page command happens to run `sweep()`, contradicting the stated fifteen-second lifetime. |

**Dispositions (fixed in `debd1934` unless noted):** P1 pageCommand in the YAP callback → queued,
drained in the guarded pump, tied to its navigation generation. P1 `gHold` global → per-top-window
state, commands check the window they were asked of. P1 Cut deleted after a truncated copy / before
confirmation → Cut is now the card's second step after the clipboard is set; refused unless the
selection is still exactly what was copied, whole, editable. P1 unbounded `String(sel)` → the
selection's size is measured by walking its text nodes, oversized ones read with a bound. P1 paste
size → bounded (and, after the inspector's finding, sent in 12000-byte pieces). P2 identity-less
`page-hold` → accepted only by the on-screen card that passed a hold on within 4 s (a finger is in
one place; the hold answer measured 0.1-0.5 s). P2 readonly/disabled → page menu. P2 iframe fields →
known gap, documented (ui-shell R7). P2 caret only when unfocused → caret follows the finger unless
inside the selection. P2 contextmenu preventDefault → system-group listener armed before the
dispatch; no menu if the page claimed the event. P2 clipboard.js cleanup → one finish path.
P2 copy callback → answered with the real result once the command ran. P2 hooks logging clipboard
contents → hooks now log only whether the clipboard equals an expected string. P3 crash leaves clip
files → swept at component start.

## Round 2 — Codex on the rendering changes

| Severity | File | Line | Description |
|---|---|---:|---|
| P1 | render/goanna/BrowserPageGoanna.cpp | 811 | `panCovered()` ignores `mSweepLeft` and strip freshness, so copied off-screen rows known to predate a page change are still considered covered. Panning into those rows suppresses repaint while sweeping is itself paused during scrolling, exposing stale pixels whose links and hit targets may no longer match the current DOM. |
| P2 | render/goanna/BrowserPageGoanna.cpp | 3694 | The sweep is armed whenever `maintKind == 0 && !smallDamage`, including ordinary composed paints with no damage at all. The missing `hadDamage` condition causes unnecessary full-region redraws after routine scrolling or commands, consuming CPU and blocking input on the single-threaded daemon. |
| P2 | render/goanna/BrowserPageGoanna.cpp | 3234 | A two-viewport-wide region roughly halves the available row count, but `panCovered()` still requires half a viewport of vertical margin. The region-placement logic normally provides only one-third—or one-sixth during a downward pan—of its spare rows above the viewport, so even the position just painted is frequently considered uncovered and every zoomed pan update triggers another expensive paint. |
| P1 | render/goanna/BrowserPageGoanna.cpp | 2489 | Pending page commands carry no navigation generation, and `pgen` is captured only when the queue is drained. Navigation paths such as back, forward, and reload increment `mNavGen` without clearing this queue, allowing a queued `copy`, `cut`, or `selectAll` from the old document to execute against the newly loaded document, potentially copying or modifying unrelated content. |
| P2 | render/goanna/DialogService.cpp | 309 | `gPageCopied` is incremented before verifying that a notification sink exists or that the card received the notification. Consequently `runPageCommand()` can report copy success when the text was only logged or otherwise never handed to the clipboard-owning card, violating the callback’s new success semantics. |

**Dispositions (fixed in `debd1934`):** P1 stale rows counted as covering → rows owed a sweep do
not; a pan over them repaints the band. P2 sweep armed with no damage → only after a change of
unknown or large extent. P2 margin vs region → the coverage margin is at most an eighth of the
region's spare rows. P1 queued commands outlive a navigation → each carries its navigation
generation. P2 `gPageCopied` counted before a sink exists → counted only when a card channel is
there.

## Round 3 — inspector on the long-press re-review and on the padlock + fixes

Inspector findings (no P0/P1) and dispositions:
- P2 paste over 16 KB dropped whole by the IPC (`kMaxMsgLen`) → confirmed in `YapDefs.h`; paste
  goes in pieces of ≤ 12000 UTF-8 bytes, never split inside a surrogate pair (checked on the host).
- P2 element-count "huge document" guard misses plain-text pages → size measured by text length.
- P2 password fields copyable → never copied, cut or reported selected.
- P2 every card acted on every `page-copied` → acted on only by the card that asked; same for
  `page-copy-empty`, `page-cut`, `page-cut-refused`; leftover cut flags cannot act on another
  card's copy.
- P3 "Cut." shown regardless → the engine reports `page-cut` / `page-cut-refused`.
- P3 silent clip-script failure → a toast; one-shot callback.
- P3 lone surrogates / stream cleanup → plain-ASCII literal (`\uXXXX`), stream closed in `finally`,
  half-written file removed.
- P3 frame padding ignored → included. P3 commands act on the focused field, not the pressed one →
  the pressed field must be the focused one, else the page menu. P3 1.5 s hold window → 4 s.
- P3 predictable clip-file names → 24 random hex digits.
- P2 adapter `pageFocused` an empty stub → sends the adapter's message; daemon clears focus on
  `false`; device-verified (T-336). P2 certificate trust state ignored → `overridden / untrusted /
  mismatch / badTime / broken` reported and shown. P3 padlock on failed loads → hidden (Enyo
  device-verified; Mochi/Mojo code only). P3 stale certificate view fields → always set. P3
  `walkSelection` walked the whole subtree → walks from the range start to its end.
- Left as known: `editable` requires the field in the top document (same as before the change);
  one `window.jihadClip` per card.

## Round 4 — Codex on the Palefill installer (2026-10-06); the inspector on the last fix round

Codex (before its credits ran out again, at 6:46 AM local they come back):

| Severity | File | Line | Description |
|---|---|---:|---|
| P1 | `render/goanna/components/jihadBundledAddons.js` | 103 | Automatically enabling Palefill exposes its `TracingListener`, which buffers every byte of matching HTML/script responses until EOF and then creates several additional full-size copies while joining, rewriting, and streaming them. A large or indefinitely streamed response from a matched site can exhaust the 512 MB target’s memory and kill the daemon; it also prevents progressive rendering. |
| P2 | `render/goanna/components/jihadBundledAddons.js` | 78 | `inst.install()` is called without rejecting `inst.addon.appDisabled`. Unlike the normal web-install path, this installs incompatible add-ons in a disabled state; during an upgrade it can replace a working older copy and then mark the incompatible replacement as offered, contradicting the claimed compatibility check. |
| P2 | `build/webos-oe/make-device-bundle.sh` | 323 | The mandatory Palefill block is conditional on the component source existing. If `jihadBundledAddons.js` is absent, packaging succeeds silently without the XPI or component even though `NOTICE` guarantees every package carries it; the component should be required just like the XPI. |
| P2 | `render/goanna/DialogService.cpp` | 317 | `gPageCopied` is incremented merely because a notification sink exists, before knowing whether `OnNotification` delivered anything. `LunaNotifier` may have zero subscribers or fail every `LSSubscriptionReply`, yet `runPageCommand()` reports copy success over IPC even though no card received the filename. |
| P3 | `build/webos-oe/push-engine-update.sh` | 80 | `--daemon-only` now unconditionally pushes the engine-internal `XPIProvider.jsm` while deliberately leaving `libxul.so` unchanged. A bundle assembled from a different engine revision can therefore pair the new provider with an old runtime and break add-on startup despite the option promising a daemon-only update. |

**Dispositions:** P1 Palefill buffers whole responses of the sites it rewrites → the add-on's
own design, the same for any user who installs it; the daemon has its low-memory watcher; left
and documented (addons R9). P2 `inst.install()` without rejecting `appDisabled` → an add-on the
manager would disable is not installed and not recorded as offered. P2 component optional in the
bundle → required, like the file. P2 `gPageCopied` counted on a sink's existence → counted only
when the sink handed the message to at least one subscriber (`OnNotification` now returns the
count). P3 provider pushed with `--daemon-only` → pushed only when the libxul on the device after
the push is the bundle's; `XPIProviderUtils.js` goes with it.

Inspector, on the last fix round (no P0/P1):
- P2 the "pressed field must be the focused one" rule passed for `<body>` (contains everything) →
  the focused element must be a text control or inside an editing host.
- P2 `deleteSelection` said "Cut." over a non-editable target → refuses unless an editing host,
  and reports true only if the selection is gone.
- P2 read-only fields had lost Copy (a control's selection is invisible to the window's) → they
  stay on the field path; `readOnly` rides the hold answer and the menus drop Paste and Cut.
  Device, Enyo: Select All → Copy of a read-only field lands; no Paste, no Cut.
- P3 script/style text in the bounded copy walk → skipped. P3 protocol 0 read as "SSL 3" when the
  call fails → unknown stays unknown. P3 `ls bundled-addons/*` killing the push script on an old
  bundle → `|| true`. P3 provider guard against dist's libxul → against the bundle's.
- P3 "delivered" means any subscriber, not the asking card; P3 a card blurring between its
  `pageFocused(true)` and its Luna request → speculative; both left as documented.
- Checked and found sound by the inspector: certificate JSON escaping, security UI presence,
  SSL status cleared on https→http, the clip file's ASCII literal and partial-write removal,
  the startup cache covering a pushed provider, ES5 in the shells, the certificate view's reuse.

## Round 5 — the inspector on the 2026-10-06 (later) diff (Codex out of credits until 6:46 AM)

Scope: the region-columns block (`BrowserPageGoanna.cpp`), the Mojo shell's `askCopy` /
`callBrowserAdapter`, the Mochi hook's `a` step, `card-selftest.sh`'s Mojo support, and the new
Mojo hook. The inspector read the framework's own `widget_submenu.js` / `widget_alert.js` /
`widget_webview.js` (the 506 build in `build/webos-oe/pdk/`) against the hook and the shell.
No P0/P1.

| Severity | File | Description |
|---|---|---|
| P2 | `render/goanna/test/mojo-selftest.js` | `a`/`d` closed the alert with `mojo.close()` (which calls `onChoose(undefined)`) and then called `onChoose(value)` again: a page `confirm()` was answered Cancel, and the second answer hit the adapter after its pipe was cleared. |
| P2 | `app-mojo/app/assistants/main-assistant.js` | The plugin road wakes every method the widget never wrapped, not only `cut` and `pageFocused`: `findInPage` too, so Find on Page in Mojo — a silent no-op until now — starts working untested; plugin-path errors (thrown strings) would propagate where the widget's wrappers swallowed theirs. |
| P3 | `mojo-selftest.js` | `m` pick: close-then-onChoose is not a tap's order; removing the element by hand leaves the widget's container/commander registered on the scene. |
| P3 | `BrowserPageGoanna.cpp` | The diff's static log gate keyed on `(regW, loX)` prints every frame of a sideways pan on content wider than two viewports. (The working tree already had the per-page `mRegionColsLogged` keyed on width.) |
| P3 | `main-assistant.js` | A copy the engine answers `false` to leaves `copyAsked` set for 5 s and shows nothing. |
| P3 | `BrowserPageGoanna.cpp` | The right-edge fix covers the band but not the adapter's own x, which may round a pixel past the floored content edge (a suspicion, not observed). |
| P3 | `render/goanna/test/mochi-selftest.js` | `a` answered whether or not an alert was up. |
| P3 | `build/webos-oe/card-selftest.sh` | Mojo push left out `app/views/**/*.html` and `index.html`. |

**Dispositions:** all applied. The hook now answers an alert through the dialog's own
`close(value)` (device: a page `confirm()` → "answered yes") and picks a submenu entry by
sending `Mojo.Event.tap` to its `x-mojo-menu-cmd` node, so the widget's own handler chooses and
cleans up. The shell's comment names the methods that take the plugin road on this framework
build (`cut`, `pageFocused`, `findInPage`); the plugin call is in `try/catch` and logged at
error level; Find on Page exercised on the device from the hook (`f` step: `find 'charlie' -> 1`,
the selection copies as "charlie"). A copy answered `false` shows "The text could not be
copied." 1.5 s later unless the engine's own word (`page-copy-empty`) got there first. The
region edge takes the adapter's x when it overshoots the band's edge by at most 2 px. The Mochi
`a` step reports whether an alert is showing and answers only then. Mojo push includes the
views and `index.html`.

**Suspicions the inspector dropped:** the region maths (the raise cannot widen a page that
fits; the clamps keep the band inside), the plugin lookup (the single `<object>`), the
`card-selftest.sh` expansion, the hook's wrappers (the adapter-call probe rethrows; the
hold probe's swallow is test-only), the copy callback's lifetime (`CopySuccessCallbackArgs`
retains and releases it; the daemon always answers).

## Round 6 — Codex on the inspector-only rounds (2026-10-06, after its credits came back; they ran out again before the second range — retry 2:53 PM)

Range reviewed: `657282c4..31c5f445` (the padlock + review-fix commits the inspector alone had
seen; 3798 diff lines). The second range (`19c5cdfa..ab07c1b7`) hit the usage limit and was not
reviewed by Codex; the inspector's round 5 stands for it.

| Severity | File | Description | Disposition |
|---|---|---:|---|
| P1 | `app/source/Browser.js`, `app-mochi/source/JihadBrowser.js` | A load start clears `loadFailed` and the padlock follows the REQUESTED url: typed `https://…` shows a padlock while the previous page is still on screen, and a tap shows that page's certificate under the new address. | Fixed: both shells track the url the engine last committed; the padlock needs the address to be committed (`pending` on Enyo's AddressInput, `_pendingUrl` in Mochi). Device: typed https → padlock gone at once, back only when the new document commits; a stalled request leaves the old page's padlock with the old page. Mojo was already committed-based (its `url` comes only from engine events). |
| P1 | `app-mojo/.../main-assistant.js` | `pageFocused` dropped by the widget (not wrapped) → another card's certificate. | Already fixed in `ab07c1b7` (plugin road). |
| P2 | `app-mojo/.../main-assistant.js` | Copy/Cut cannot reach the adapter (`copy(undefined)`, no `cut`). | Already fixed in `ab07c1b7`. |
| P1 | `render/goanna/components/jihadPageCommands.js` | Cut compares only the TEXT of the current selection with the copied text, not the element: focus moved to another field holding the same text between Copy and Cut deletes from the wrong field. | Fixed: the copy keeps a weak reference to the field it came from and its time; a cut needs the same element, the same text and a copy under 15 s old. Device: Select All → Cut in a field still cuts. |
| P2 | `jihadPageCommands.js` | `st.copied` keeps up to `MAX_COPY` characters per page indefinitely (16 pages → tens of MB). | Fixed: a 15 s timer lets the copied text go (the card asks for the cut well under a second after the copy). |
| P1 | `render/goanna/components/jihadBundledAddons.js` | Palefill's response buffering. | Accepted and documented (round 4). |
| P2 | `render/goanna/DialogService.h` | `OnNotification` now returns `int`; `services_test.cpp` and `xpi_mismatch_test.cpp` still override it as `void` and no longer compile. | Fixed: both sinks return 1. `xpi_mismatch_test` passes 8/8; `services_test` compiles and passes 7/8 — its pixel check reads a black GTK window under Xvfb before any click, as `input_test` does too: the desktop (non-offscreen) harness, not today's change; recorded in PICKUP, not chased. |
| P2 | `build/webos-oe/push-engine-update.sh` | A full push with a stale bundle installs dist's libxul, withholds the provider and restarts: new engine, old `XPIProvider`. | Fixed: a full push dies before touching the device when the bundle's libxul is not dist's. |
| P3 | `render/goanna/BrowserPageGoanna.cpp` | `contentCols` truncation at the content's right edge drops the wide region. | Already fixed in `ab07c1b7`. |

## Round 7 — Codex on `2b1e7491..20310b3e` (the personas / idle-drain / referrer / title / DNT work; 1619 diff lines)

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `render/goanna/DialogService.cpp` | `AppendLiteral(theme ? "…" : "…")`: a conditional of two literals of different lengths is a plain pointer, not an array; it compiles only because this tree has a `char*` overload. | Fixed: one literal per call. |
| P1 | `render/goanna/BrowserPageGoanna.cpp` | The re-driven navigations synthesise `Referer: <current page>`, ignoring `rel=noreferrer`, a link's `referrerpolicy` and the document's referrer policy — leaking URLs a page withheld. | Fixed: a content navigation re-sends exactly the referrer and policy the engine's own request carried (read off its `nsIHttpChannel`); a tapped link gets its own rules (rel, `referrerpolicy`, the document's last `<meta name=referrer>`), via `LoadURIWithOptions`. Gap, documented: a `Referrer-Policy` HTTP header is not visible to the tap path. **Device:** plain → full URL, `rel=noreferrer` → none, `referrerpolicy=origin` → origin, meta no-referrer → none; script navigations carry the engine's own value (kept / none). |
| P1 | `BrowserPageGoanna.cpp` | The theme's origin is read when the pump drains the event, not when it fired: dispatch-then-navigate names the wrong site on the confirm and resolves relative images against it. | Fixed: the element's own document URI and base URI are taken in the event handler and travel with the request. |
| P1 | `components/jihadPersonas.js` | Any page can PREVIEW a theme (repaint the chrome) without asking — chrome spoofing. | Fixed: Firefox 52's rule — https and the site's "install" permission; a preview is reset when its page is left (daemon tracks the previewing document). **Device:** http preview logged as ignored. |
| P1 | `jihadPersonas.js` | Previews were persisted as the current theme and the pref file flushed on every styling update. | Fixed: only the installed theme (`LWTM.currentTheme`) is remembered, and the file is written only when the value changes. |
| P1 | `BrowserPageGoanna.cpp` | Post-load title changes forwarded unbounded and uncoalesced: a page rewriting its title per frame floods the card, an oversized title overflows the 16 KiB packet. | Fixed: titles bounded to 1024 bytes on a UTF-8 boundary with control characters blanked (every title sent, not only these), at most one change per 500 ms, the latest, none when unchanged; log line bounded. **Device:** a 5000-char, 50 Hz flood → 12 sends in ~6 s, longest line 1024 + prefix, no control characters. |
| P1 | `jihadPersonas.js` | Static-theme XPIs: no size limits, extracted before the user answers, overwrite-by-name. | Fixed: manifest ≤ 64 KB, image ≤ 4 MB (zip entry `realSize`), png/jpeg/gif; unpacked to a pending file kept on yes, deleted on no (and swept at start); named by a SHA-256 of the image. **Device:** Water-Like XPI → `static-theme:0dff6242…`, painted live (see note). |
| P2 | `app/source/BrowserApp.js` | `this.$.browser` is a lazy view: a theme arriving with the start page up threw. | Fixed: the app keeps the theme and gives it to each pane view as it is selected; the painting moved into `ActionBar` so the start page's bar wears it too. **Device:** both bars painted. |
| P2 | `packaging/prefsui/content/preferences.js` | The settings page re-migrated a card's fragment payload, turning a deliberate "full DuckDuckGo" back into Lite on every open. | Fixed: only the page's own stored copy is migrated. |
| P2 | `jihadPersonas.js` | Static-theme ids collide by name. | Fixed with the hash id above. |
| P2 | `jihadPersonas.js` / `DialogService.cpp` | Theme JSON over the 6000-char notification limit becomes "null" on the card. | Fixed: the card gets only the fields it paints, each bounded (`cardTheme`). |
| P3 | `BrowserPageGoanna.cpp` | The title-change comment claims the history row is updated. | Fixed: comment now says the title bar (and tabs to come); history keeps the loaded title. |

**Note (device, Mochi):** in one run the static theme's live notification did not paint the bar
(pushed, but the bar stayed plain; a later card painted it from `getTheme`). Two later runs —
persona then XPI, each with a different theme id — painted live both times. Not reproduced; not
explained.

## Round 8 — the inspector on the uncommitted night work (Codex out of credits until Oct 7 1:53 AM; 2659 diff lines)

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `render/goanna/GoannaRenderPage.cpp` | A re-driven load with no referrer (withheld by `rel=noreferrer`/policy, or a non-HTTP content navigation such as `data:`) and no explicit principal runs as the SYSTEM principal (`nsDocShell::LoadURI`): a page could reach `file:` or a top-level `data:` document through the daemon. | Fixed: every page-initiated re-drive passes the initiating document's principal (`nsIScriptObjectPrincipal` of the old document for content navigations, of the link's document for taps) as `LoadURIWithOptions`' triggering principal; without one it fails closed to the current page's origin (as a no-referrer referrer). User-initiated loads are unchanged. **Device:** `rel=noreferrer` link to `file:///tmp/jihad-plain.html` → "Security Error: Content at http://… may not load or link to file:…"; script navigation to `data:text/html,…` → refused (`0x805303f4`); referrer cases unchanged. |
| P2 | `components/jihadPersonas.js` | The static-theme size limits trusted the zip's own declared sizes; `extract` inflates to the end of the data and recreates symlinks. | Fixed: the image is copied through the reader's stream (bounded by the declared size, CRC-checked) with a hard 4 MB cap; never extracted. **Device:** an XPI whose 6 MB image declares 1000 bytes → `NS_ERROR_FILE_CORRUPTED` at the bound, no file left, no prompt; Water-Like still installs and paints. |
| P3 | `jihadPersonas.js` | `cardTheme` could still exceed the channel (control characters escape to six characters); an oversized id made the installed theme unpersistable; a preview reusing the installed id was persisted. | Fixed: control characters blanked, fields dropped in order until it fits; "installed" decided on the theme's own id and never for the page's current preview (time-bounded mark). |
| P3 | `BrowserPageGoanna.cpp` | The URL in the location/title messages was unbounded (and the URL-as-title fallback bypassed the title bound). | Fixed: 8 KiB bound on the wire URL; the fallback goes through the title bound. |
| P3 | `GoannaRenderPage.cpp` / `jihadPersonas.js` | A theme request dispatched from an `about:srcdoc`/`about:blank` frame showed a nameless prompt. | Fixed: a theme request from a document with no site is ignored. |
| P3 | `app-mojo/.../main-assistant.js` | One download-progress throttle shared by all transfers. | Fixed: per transfer. |
| P3 | test hooks | The Enyo/Mochi `g` pinch step drives the JS gesture entry points, which widget mode had made inert. | Fixed: widget mode is the Mojo build only (`JIHAD_VARIANT`). **Device:** Enyo `g1.6` → zoom 0.69 → 0.91. |

Codex to re-review this session's range when its credits return (the owner's standing rule).

## Round 8b — the inspector on the round-8 fixes (Codex out of credits again until 12:41 PM; its run on `20310b3e..86f0eee1` stopped mid-review with no findings)

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `render/browserserver/Yap/YapPacket.cpp`, `BrowserPageGoanna.cpp` | URLs were still sent unbounded on `msgLinkClicked`, `msgUpdateGlobalHistory`, `msgFailedLoad`, `msgUrlRedirected`, `msgNewPageRequested`; an oversized string was SKIPPED by the YAP writer, the adapter read NULL and `STRINGZ_TO_NPVARIANT(NULL)` = `strlen(NULL)` in the card host (a page could crash it with `replaceState` + reload). | Fixed both ways: every URL the daemon sends goes through `jihadBoundedUri` (8 KiB); the shared YAP writer sends an empty string in place of one that does not fit (later fields stay aligned) and the reader never yields NULL (`""` on any failure) — which also covers page-controlled dialog text and popup payloads. |
| P3 | `GoannaRenderPage.cpp` | Script history traversal (`history.back()`) re-driven with the page's principal fails into file:/data:/privileged entries. | Fixed: a traversal (`loadType & 0xffff == LOAD_CMD_HISTORY`, read at STATE_START) is re-driven as before, without a page principal — docshell does not check traversals against the caller either. |
| P3 | `app/source/Browser.js`, `app-mochi/source/JihadBrowser.js` | "Open In New Card" on a long-pressed link opened file:/data: as a typed (system) load. | Fixed: web addresses only (http/https/ftp), as window.open. |
| P3 | `jihadPersonas.js` | The preview mark (60 s) outlived the engine's preview (30 s) and could block persisting a real choice. | Fixed: 30 s, cleared as soon as another theme is shown. |
| P3 | `BrowserAdapter.cpp` | The hold generation was one slot for the whole adapter. | Fixed: per request (`HitTestArgs::m_gestureGen`). |
| P3 | `BrowserAdapter.cpp` | Pass-through native gestures did not bump the generation. | Fixed: every `npPalmGestureStartEvent` bumps it. |

## Round 9 — Codex on `e7b9aa7d..075d29e5` (taps, first-paint hold, hit-test schema, personas converter; 2026-10-08, credits back)

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `app-mojo/app/assistants/main-assistant.js` | Removing the widget's `_tapHandler` is overbroad: only `_singleTapHandler` clicks; `_tapHandler` records the tap `linkClicked` reports, and Codex said `linkClicked` would throw without it. | Partly valid. The framework's `linkClicked` guards (`_lastTapEvent && _lastTapEvent.up`, else `{}`), so nothing throws; but `_tapHandler` clicks nothing and keeps the tap the framework reports, so only the single-tap handler stands down now. **Device:** Mojo start page, a tap on Wikipedia — one `clickAt (360,470) <A>`, navigates, first paint released. |
| P2 | `render/adapter/BrowserAdapter.cpp` | Parsing a rejected hit-test answer against `{}` bypasses the schema inside LunaSysMgr; make the answer conform instead. | Valid, fixed differently from the suggestion: measured on the device, the stock `/etc/palm/browser/HitTest.schema` rejects **everything** in the pbnjson this adapter links — even `{}` and every single-field object (a fresh `JSchemaFile` as well as the member) — so no answer can conform to it. The adapter now carries its own schema (isis's fields, all optional, `additionalProperties: false`, in the `optional` dialect `InteractiveWidgetRect.schema` uses) as a `JSchemaFragment`; no bypass, a rejected answer is logged and the hold falls back to the page menu. **Device:** Enyo link hold → Open In New Card / Share Link / Copy URL; image hold → Copy To Photos / Share Image / Set Wallpaper; no rejections logged. |
| P2 | `render/goanna/BrowserPageGoanna.cpp` | The first-paint hold is not tied to a document: a new suppressed document can inherit the old one's timestamp (and get no hold). | Valid, fixed: the hold state is reset with every navigation (`resetScrollForNavigation`, and the adopted-POST path). |
| P3 | `render/goanna/test/coordmap_test.cpp` | The test still clicks the surface point (240,240) at 2x and expects the daemon to divide. | Valid, fixed: it clicks document (120,120), the adapter's contract. Not run — no desktop engine build on this host. |

## Round 10 — the inspector on `20310b3e..e7b9aa7d` (the owed re-review; Codex ran out of credits mid-run, retry 10:07 PM)

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `app-mojo/app/assistants/main-assistant.js` + the Mojo WebView widget | The widget saves each page's zoom at load start and restores it at load stop (`_restoreCurrentPagePosition` → `setMagnification(item.zoom)`); under the screen-sized model `currScale` stays 1, so Back/Forward, reload or a revisit forced any page whose fit is not 1 to 1.0 behind the adapter. | Valid (device framework 506 has the same `loadStopped`), fixed: both widget methods are no-ops in this shell. **Device:** Mojo, a 1200 px page (fit 0.64) → link → Back: the daemon is at 0.6400 and the page is drawn fitted. |
| P2 | `render/adapter/BrowserAdapter.cpp` | Pinch: the header was taken off the centre AND the scroll, so every Mojo pinch drifted by (scale-1)*header. | Valid, fixed: the scroll stays ours (header included), as before e7b9aa7d. |
| P2 | `BrowserAdapter.cpp` / Mojo | A link followed from a scrolled position opened with the address bar hidden (the navigation reset's page scroll 0 kept as "header partly hidden"). | Valid, fixed: `msgLoadStarted` brings the header back when the page is at its top. **Device:** after the link above the toolbar shows. |
| P2 | `BrowserAdapter.cpp` | `mMetaViewport` outlived its page (freed only on a contents-size sentinel this daemon never sends): one `user-scalable=no` page disabled pinch and double-tap everywhere after. | Valid, fixed: dropped at `msgLoadStarted`; the next page's tag arrives after it. |
| P3 | `BrowserAdapter.cpp` | `setHeaderHeight` left the scroller's range and the daemon's scroll stale (Mojo's find bar). | Valid, fixed: range, clamp, scroller and daemon updated when the strip changes. |

## Round 11 — the inspector on the Personas work (`faa34d13`, `09ec8432`, `94dec4ee`; Codex out of credits until 10:07 PM)

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `components/jihadPersonas.js` | Every edition bump re-added the bundled Personas a user had uninstalled. | Valid, fixed: the names offered are kept (`jihad.personas.bundledNames`; a profile offered before that is taken to have been offered all); an edition adds only a Persona new to the set or one replacing its own earlier picture. **Device:** Enyo, Aero Sky uninstalled, an edition-4 bundle pushed → "offered: 0". |
| P2 | `BrowserAdapter.cpp` / `BrowserPageGoanna.cpp` | With the adapter dropping the meta viewport at every load start, the POST-adopt path (no navigation reset) never re-sent a result page's identical tag. | Valid, fixed: that path resets `mViewportSent`. Not exercised on the device. |
| P3 | `jihadPersonas.js` | A failed shrink refused the whole Persona. | Valid, fixed: the picture is kept as it came. |
| P3 | `jihadPersonas.js` | Undo could bring back a Persona uninstalled within the 30 s, without its picture. | Valid, fixed: Undo is withdrawn when its Persona is uninstalled. |
| P3 | `jihadPersonas.js` | Undo could offer a stale change seen only when a pane opened. | Already fixed in `402cc925` (outside the reviewed range): changes are noted when the manager reports them, coalesced. |
| P3 | `jihadPersonas.js` | Swapping in a worn new edition rewrote the list first, so the manager briefly wore an unrelated Persona (cards flickered, stray events). | Valid, fixed: the new edition is worn first, then the list rewritten. |
| P3 | `jihadPersonas.js` | Every redraw read every picture's header from disk; an Undo timer per redraw. | Valid, fixed: sizes cached by file (a picture's name is its hash); one Undo timer. |
| P3 | `jihadPersonas.js` | Complete (XUL) themes were dressed as Personas. | Valid, fixed: only `@personas.mozilla.org` ids are decorated. |

Checked and sound per the inspector: re-driving about: pages by their original address gives web content no new reach; Persona fields cannot inject CSS or markup; file deletion is confined to 24-hex names in `jihad-themes`; windows, timers and observers are released.

## Round 12 — the inspector on `2cc01662..392c39e3` (engine groundwork, complete themes, the gallery, the licensed bundle; Codex out of credits until 10:07 PM)

| Severity | File | Description | Disposition |
|---|---|---|---|
| P3 | `personas.html` / `preferences.css` | The empty online section always showed (`.group{display:block}` beat `[hidden]`). | Fixed (`[hidden]{display:none !important}`). **Device:** gone. |
| P3 | `jihadPersonas.js` | Re-wearing the same id (rename / details refresh) sends no styling update: cards and `jihad.theme.current` kept old details. | Fixed: `rewear()` sends the update itself. |
| P3 | `jihadPersonas.js` | Adding bundled Personas ignored the manager's 30 limit; the next change would drop them as uninstalls and delete their pictures. | Fixed: added only while there is room; what does not fit is offered later. A worn Persona's new edition replaces the old in place (no transient growth). |
| P3 | `make-personas.py` | `formerNames` of third-party entries were dropped. | Fixed. |
| P3 | `personas.js` / `jihadPersonas.js` | An https index could still bring http/file pictures. | Fixed: online pictures and installed images must be https. |
| P3 | `check-engine-api.py` | Check 1 counted a call or comment as a definition. | Fixed: comments stripped, a return type required (a call no longer matches — tested). |
| P3 | `check-engine-api.py` | Check 7 passed with no ICU file. | Fixed: FAIL. |
| P3 | `check-engine-api.py` | Check 5 searched test files. | Fixed: default-pref files only; the `git grep` fallback excludes tests. |
| P3 | `check-engine-pins.sh` | The launch-scripts ICU check matched only a comment. | Dropped. |
| P3 | `patch-queue.sh` | Predictable `/tmp/pq.$$`. | Fixed: `mktemp`, in the trap. |
| kit | `cavekit-personas-ui.md`, desktop build | R7 named the old script; the desktop dist lacked the Persona component and bundle. | Fixed (desktop `build-goanna.sh` now ships them). |

**Found on the device while verifying (not in the review):** taking a toolkit-targeted complete theme
back out left `general.skins.selectedSkin` naming the removed theme and `extensions.lwThemeToSelect`
queued as "none", so the next daemon start dropped the user's Persona. Fixed in `jihadInstallPrompt.js`
(both cleared after the removal); **device:** install + removal + daemon restart → Persona still worn,
skin prefs clean. Enyo's profile repaired by hand.

## Round 13 — Codex on `9e08c610..1fe88aa5` (the range the inspector covered in rounds 10–12; Codex credits back 2026-10-11)

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `jihadPersonas.js` | A wallpaper was decoded before shrinking with only its compressed size capped: a few KB can declare a picture that decodes to gigabytes and kills the daemon before the user is asked. | Valid, fixed: every picture's size is read from its own header first; unreadable or over 4096×4096 pixels → not kept. **Device (Enyo):** a 1.2 KB package with a 10000×10000 PNG → "the picture is 10000x10000: not kept", no prompt, daemon PID unchanged, log grew. |
| P1 | `check-engine-api.py` | Check 1 found each `jihad_*` definition but never compared its types with `EngineCompat.h`; C linkage carries no types, so a changed signature links and corrupts at run time. | Valid, fixed: return and parameter types are normalised on both sides and compared (unit-tested: a changed parameter type is caught, names and spacing ignored). All 18 match today. |
| P2 | `patch-queue.sh` | `mktemp` leaves an empty file, which older git refuses as an index. | Plausible (git 2.5x here accepts it; `--check` passed before and after), fixed anyway: the index path is in a fresh `mktemp -d`. |
| P2 | `jihadPersonas.js` | When the bundled set did not all fit under the manager's limit, the edition was still marked done, so the rest was never offered. | Valid, fixed: the edition is marked done only when all of it found room. |
| P2 | `jihadPersonas.js` | Theme-only packages were recognised by a short list of refused manifest keys; other behaviour (`chrome_url_overrides`, `commands`, protocol handlers…) passed as a Persona with its behaviour dropped. | Valid, fixed: an allowlist of the keys a theme package may carry. Checked against addons.mozilla.org's 50 most-used themes: every key they use is allowed. **Device (Enyo):** a package with `chrome_url_overrides` → "not a theme-only package"; Carbon Light still converts, asks, and is worn. |
| P2 | `check-engine-api.py` | Check 5 read only `packaging/prefs/*.js`, missing the bundler's memory prefs and those the daemon sets at run time. | Valid, fixed: also scans `build/webos-oe/*.sh`, the daemon's `*.cpp` and the components. **It found one:** `media.memory_cache_max_size` (set by the bundler) does not exist in UXP — removed. Two prefs read through a macro or a branch are listed as known. 0 FAIL, 0 WARN. |
| P3 | `main-assistant.js` (Mojo) | The ink glyph set is not applied when a Persona has no accent colour. | By design, no change: without an accent the command row keeps the framework's dark circles, where light glyphs are the readable ones. A comment now says so. |

Device left: Enyo wearing Space Fantasy again (restored through about:personas after the Carbon Light control); all three variants on bundle `7a18305f…` (daemon-only push: the daemon binary is unchanged, the chrome scripts are new).

## Round 14 — the inspector on the Persona formats and the add-on site (Codex out of credits until 4:59 PM, 2026-10-11)

### Jihad Browser, `69c44675..efa6844a` (T-390–T-392, T-409–T-411)

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `jihadPersonas.js`, the three shells, the pane | A colour-only Persona's 8×64 frame-to-toolbar gradient was drawn `cover`, so a bar showed one blended row (#3b1d5e/#7a3fa8 → about #5a2e83). | Fixed: `fit: "stretch"` (100% 100%, left top) in the component, the shells and the pane. **Device:** Enyo's bar measures #3e2164 at the top, #7c3aa8 at the bottom; Mochi and Mojo the same by eye. |
| P3 | `jihadPersonas.js` (pane) | A layered Persona's first picture was `cover` in its pane row, hiding the others. | Fixed: a placed picture keeps its own size. **Device:** the moon at its size with the hills and dots beside it (Enyo). |
| P3 | `jihadPersonas.js` | Re-installing the worn Persona under the same id used `LWTM.currentTheme =`, which sends no styling update. | Fixed: `wearTheme()` goes through `rewear()` for the worn id (three sites). **Device:** Carbon Light re-installed with a red frame repainted the bar (#cc2222). |
| P3 | `jihadPersonas.js` | Colour slots were checked for characters, not parsed: `red, 0 0 0 2000px black` reached the stylesheets. | Fixed: `canonColor()` (inIDOMUtils.colorToRGBA) → `rgb()`/`rgba()` or dropped. **Device:** `hsl(120, 60%, 30%)` arrived as `rgb(30, 122, 30)`, the shadow value was dropped. |
| P3 | `jihadPersonas.js` | The pixel budget was checked after each picture was copied, shrunk and hashed. | Fixed: from the header's size (and the size after shrinking) first; once spent, the rest are skipped unread. |
| P3 | `personas.js` | Overlapping feed or update requests could render out of order, and Check for updates twice listed each update twice. | Fixed: a sequence number per request; the page number comes from the reply. |
| P3 | `test-site-profile.sh` | `restore` deleted the backup before checking the copies and the CA. | Fixed: copies verified, the CA checked gone, only then the backup removed. |
| P3 | `test-site-profile.sh` | Files absent at setup, a user.js already there, what the test added to extensions/ and jihad-themes/, certutil missing after the daemon was stopped, an empty address. | Fixed: each. **But see below.** |
| P3 | `jihad-addon-prefs.js` | `lightweightThemes.getMoreURL` not set (R11). | Kit narrowed: unset by design — the pane's "Get Personas" opens about:personas, which browses the site. `lightweightThemes.update.enabled` now named false too. |
| kit | `cavekit-persona-formats.md` | R1/R2's about:personas card and R4's "same fields" for index Personas. | Kit narrowed: about:personas lists bundled and site Personas only; index Personas carry colour slots. |

**Found on the device while verifying (not in the review), P1 in effect:** the new clean-up in
`test-site-profile.sh restore` decided membership with `grep -qxF`; the device's BusyBox 1.17 grep
has no `-x`, every test failed, and `|| rm` removed **every** file in Enyo's `profile/jihad-themes/`
and `profile/extensions/` — twice (both Enyo restores of the day; Mochi and Mojo were never
restored). Fixed: membership in plain sh, and the list must find its own first line before
anything is removed (tested on the device in a scratch directory first). Enyo repaired: the 13
pictures its prefs name copied from Mochi under the same names (Mochi's and Mojo's copies
md5-identical), and Palefill re-offered by deleting `jihad.bundled-addons.palefill@….offered` —
1.30 installed again, active, not user-disabled; Space Fantasy painted again. The jihad-site-test
extension Enyo held from the update test is gone with it (a test add-on).

### Riverstone-Addons, `upstream/TRUNK..aeb01fb`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `index.php` | The panel ran for any raw path starting `/panel/`: `/panel/../?path=/panel/administration/` reached its login through nginx's `/` location, without the rate limit or the allowlist. | Fixed: PHP takes the panel only when nginx's `/panel/` location sets `RIVERSTONE_PANEL`. Both path tricks answer 404 (smoke test). |
| P2 | `addonDownload.php` | `?component=download&panel=1` checked passwords outside the panel. | Fixed: panel downloads are `/panel/download/`, inside the panel's login and limits. |
| P2 | `riverstone-maps.conf` | X-Frame-Options/CSP dropped for any URL whose query said `component=discover`, panel pages included. | Fixed: keyed on path and component; smoke test checks a panel URL keeps the header. |
| P2 | `classReadManifest.php` | `by-id`/`by-slug` served unreviewed add-ons publicly and to the update service. | Fixed: reviewed only. Checked: an unreviewed add-on's page and download are 404, its update check offers nothing, the panel still serves it. |
| P2 | `classReadManifest.php` | Descriptions entity-encoded before Smarty escaped them (`&amp;amp;`, growing on each save). | Fixed: plain text; the content fallback escapes it once. |
| P2 | `install.sh` | Re-running overwrote the owner's allowlist and site block. | Fixed: the allowlist is its own include written once; an edited site block is kept (`.new` beside it). |
| P2 | `classWriteManifest.php` | Validator errors (with an install.rdf's values) went into the panel page as markup. | Fixed: error pages escape everything except the code's own few tags. |
| P3 | `classPersona.php`, `personaFeed.php` | A huge page number overflowed into SQL. | Fixed: clamped; an empty page. |
| P3 | `classGenerateContent.php` | update.rdf values unescaped; `]]>` could close addon.xml's CDATA. | Fixed: escaped; CDATA split. |
| P3 | `addonSite.php` | `/search/` without terms logged a deprecation. | Fixed. |
| P3 | `fundamentals.php` | A 404 had an empty body; error pages were not in the skin. | Fixed: a page; public errors in the site's skin (device: Enyo). |
| P3 | `backup.sh`, `sync.sh`, `install.sh`, `riverstone.conf`, `DEPLOY.md` | No pipefail; no remote mkdir; the DB password in arguments; a world-readable php.log; a password on a command line. | Fixed: each. |
| P3 | `persona-admin.php` | Delete followed a symlinked folder. | Fixed: links removed as links. Checked with planted symlinks. |
| P3 | `deploy/local` | The local copy lacked the pool's open_basedir and disabled functions. | Fixed in `php-local.ini`. |
| P3 | `classPersona.php` | Empty fields left out of the feed. | Fixed: every field present as a string. |
| P3 | panel templates | Category labels and bulk-upload messages double-escaped. | Fixed. |

Codex reviews both ranges again when its credits return (owed).

## Round 15 — the inspector on the round-14 fixes (Codex ran out of credits again at 17:00 with no findings; retry 10:00 PM)

Codex was started on both full ranges at 17:00 (Jihad `69c44675..HEAD`, the site
`upstream/TRUNK..jihad`); both stopped at the usage limit before writing a finding. The inspector
reviewed what nobody had: the site's `aeb01fb..87cf077` and Jihad's `948e1c88`. Both: APPROVE.

### Riverstone-Addons, `aeb01fb..87cf077` — fixed in `c5867eb`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P3 | `riverstone-maps.conf` | `/?component=discover&component=site&path=/personas/` was frameable: nginx reads the first `component`, PHP the last. | Fixed: the exemption needs the whole query to be exactly `component=discover`. Smoke test checks that URL and that the discover pane stays frameable. |
| P3 | `persona-admin.php` | Delete followed a symlinked `personas` folder (www-data owns it; the script may run as root): reproduced deleting `victim/ssh/*`. | Fixed: the folder must resolve to exactly `<root>/personas/<id>`. Re-run in the PHP container: refused, victim intact; normal deletes unchanged. |
| P3 | `DEPLOY.md` | Optional allowlist pointed at the file install.sh rewrites. | Fixed: `riverstone-panel-allow.conf`. |
| P3 | `developer-frontpage.xhtml` | The application id's braces compiled as a Smarty expression; the page failed. | Fixed: `{ldelim}`/`{rdelim}`; compiled with the bundled Smarty. |
| P3 | `backup.sh` | A failed dump still left a valid-looking `db-*.sql.gz`. | Fixed: temporary name, moved into place when complete; a failing dump leaves nothing. |
| P3 | `install.sh` | New DB password and RS_SECRET in `sed`'s argv. | Fixed: bash string operations and `printf`; output identical. |
| P3 | `classGenerateContent.php` | gfSubst's sequential replacement undid the update.rdf escaping. | Fixed: `strtr` (one pass); update.rdf and the get API still parse. |
| P3 | `make-owner.php` | Its own text suggested the password on the command line. | Fixed. |

### Jihad Browser, `948e1c88`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `jihadPersonas.js` | `imageSize()` could under-report: a GIF's first frame enlarges it, JPEG fill bytes led the scan to a fake SOF (3000x3000 read as 1x1 / 100x100); MAX_PIXELS and the budget relied on it. | Fixed: the scan reads as the decoders do (first frame, fill bytes, SOS-before-SOF refused) and `pictureSize()` takes the larger of it and the engine's metadata decode (no pixels allocated); a picture whose content is not its extension's kind is left out. **xpcshell:** both lying files now 3000x3000; 15 bundled pictures agree both ways. **Device (Enyo):** the lying GIF package → picture left out, worn on its colours. |
| P3 | `jihadPersonas.js` | The first picture was exempt from the 6 Mpx budget (1024x16384 kept). | Fixed: the budget counts it. **Device:** left out. |
| P3 | `jihadPersonas.js` | `canonColor`: currentcolor/-moz-*/system colours came out black or invisible; `rgb(1 2 3 / 50%)` dropped. | Fixed: a bare word must be a colour name or `transparent`; `/` allowed; frame and text colours canonical too. **xpcshell:** 14 cases. |
| P3 | `jihadPersonas.js` (pane) | A tiled pattern's first picture was drawn `cover` in its row. | Fixed: a repeating first picture keeps its own size. Code. |
| P3 | `jihadPersonas.js` | Colour-only Personas from before the fix keep `fit: "cover"`. | Fixed: read as `stretch`. Code. |
| P3 | `jihadPersonas.js` | The bundled gallery's Wear still set `currentTheme` directly. | Fixed: `wearTheme()`. **Device:** Linen Light then Space Fantasy worn from the gallery. |
| P3 | `personas.js` | A second tap on Next before the reply asked for a page past the end. | Fixed: the pager is disabled while loading; a page past the end loads the last. **Device:** Next → Page 2 of 2 (a scripted double tap is taken as the zoom gesture, so the race itself is code). |
| P3 | `personas.js` | Check for updates with no answers said "up to date". | Fixed: "could not be reached" (and a count when some fail). Code. |
| P3 | `test-site-profile.sh` | No route → the script ended silently, restore included. | Fixed: the address is looked up only to set up without SITE, and cannot end the script. |
| P3 | `test-site-profile.sh` | A setup cut short could leave an empty list, which restore read as "delete everything". | Fixed by design: no lists. Setup writes whole copies to `$BAK.tmp`, marks it complete and renames it; restore refuses an incomplete backup or a short folder copy. **Device, scratch profile:** both refused, nothing changed. |
| P3 | `test-site-profile.sh` | `lightweighttheme-header/-footer`, rewritten extras and deleted pictures were not restored. | Fixed: backed up; the folders are swapped back from their copies. **Device, scratch profile:** added, deleted and rewritten files all md5-identical after restore; an interrupted restore finished on a second run. **Enyo:** two real cycles, md5-identical. |

Codex is still owed on both full ranges (10:00 PM).

## Round 16 — the inspector on the wormhole deploy (Codex ran out of credits again at 22:10 with no findings; retry 3:07 AM 2026-10-12)

Codex was started on both full ranges at 22:07 and stopped at the usage limit (~216k and ~243k
tokens) before writing a finding. The owner had meanwhile asked for every file to go over
magic-wormhole-rs: `sync.sh` became `pack.sh` + `swap-in.sh` (site `8535d3b`, `fd41f7e`, `def0c62`).
The inspector reviewed that range: APPROVE, 2 P2 and 12 P3, all fixed in `cdaed4c`.

### Riverstone-Addons, `c5867eb..def0c62` — fixed in `cdaed4c`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `swap-in.sh` | A real `datastore`/`.obj` folder in the old copy was deleted with it (only links were carried); install.sh refuses that state. | Fixed: refused with install.sh's message, nothing changed. Scratch: the planted picture survives. |
| P2 | `DEPLOY.md` | Promised certificate-expiry mail Let's Encrypt no longer sends (June 2025). | Fixed: `certbot renew --dry-run` once; the smoke test fails three weeks before expiry. Local: 34/34; near-expiry and unreachable fail. |
| P3 | `DEPLOY.md`, `pack.sh` | `tar -xzf` as root extracted members outside the copy's prefix (crafted archive could overwrite `www/` or write through the datastore link). | Fixed: extraction names the prefix (`tar -xzf X.tar.gz X/`). Scratch: a `www/index.php` member is not extracted. |
| P3 | `swap-in.sh` | An incomplete copy could go live (only `index.php` checked; a tar ending between members extracts with exit 0), and the old copy was deleted at once. | Fixed: `pack.sh` adds `.pack-manifest` (every file's SHA-256) as the archive's last member; `swap-in.sh` refuses a copy without it, with a mismatch, or with extra files. The old copy is kept as `www.prev`; its own `swap-in.sh` swaps it back. Scratch: each refused; rollback and return worked. |
| P3 | `swap-in.sh` | "Run install.sh" was guessed from what it could see (first copy, deploy/ diff). | Fixed: install.sh leaves `install.stamp` (hash of what it applies) when it finishes; swap-in compares it and checks the links. |
| P3 | `swap-in.sh` | A docs-only change asked for install.sh (nginx and FPM restart for nothing). | Fixed by the stamp: only install.sh's inputs count. Scratch: DEPLOY.md-only update → smoke test. |
| P3 | `swap-in.sh` | A run cut off after making the links left a copy every re-run refused. | Fixed: a link to the live target is accepted; any other is refused. Scratch. |
| P3 | `swap-in.sh` | FPM reload: silent when skipped, `set -e` exit hid the next step, every PHP version reloaded. | Fixed: only the version with the site's pool; a failed reload warns and the script goes on. Code (OpenRC). |
| P3 | `pack.sh` | Unknown arguments ignored (sent anyway); temp folder left on failure; any branch packed. | Fixed: usage error; trap; HEAD must carry `swap-in.sh`. |
| P3 | `DEPLOY.md` | Updating never ran the smoke test. | Fixed: swap-in names it last, and the doc says so. |
| P3 | `install.sh` | A schema change never reached an existing database (`CREATE TABLE IF NOT EXISTS`). | Fixed: `deploy/migrations/*.sql`, applied once each and recorded in `rs_migration`; a fresh database only records them. Throwaway MariaDB 10.11: all four cases. |
| P3 | `DEPLOY.md` | The Personas were sent from the working tree. | Fixed: `git status --porcelain -- render/goanna/personas` must print nothing first. |
| P3 | `DEPLOY.md` | Backups landed wherever the PC's shell was (maybe a repository). | Fixed: `--out-dir ~/riverstone-backups`, with why. |
| P3 | `install.sh` | `cron` never installed, though the backup and certbot's renewal need it. | Fixed: installed and enabled. Code. |

The inspector's open question, whether wormhole-rs's folder tar carries the folder's name as its
prefix, was checked here: a test folder arrived as `dummy/…`, owner 0/0. Not verified: the OpenRC
paths (the FPM reload, cron), which run only on the server.

## Round 17 — Codex on `69c44675..83f0e817` (credits back 2026-10-12; the site range ran out again)

Codex finished the Jihad range (1 P1, 5 P2, 4 P3; all checked against the code). On the site range
(`upstream/TRUNK..jihad`) it stopped at the usage limit again (~150k tokens, retry 12:28 PM); the
inspector reviews the round-16 fixes (`cdaed4c`) meanwhile.

### Jihad-Browser, `69c44675..83f0e817` — fixed in `df590b3b`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `test-site-profile.sh` | Restore trusted `cp`'s exit status (the device's BusyBox `cp` can report EBADF for a copy that landed), went on swapping and restarted the daemon, and the host stopped before checking the CA. | Fixed: every copy judged by md5; a restore that did not finish leaves the daemon stopped and the backup kept, and still reports whether the CA is in the profile (an unreadable database is "unknown", never "gone"). Local simulation (dash, fake novacom/upstart): a `cp` that copies and then fails with EBADF → setup and restore succeed, profile identical; a folder copy really cut short → "did not go back", daemon stopped, backup kept; restore again → identical. |
| P2 | `test-site-profile.sh` | `extensions.ini` (and `addons.json`) not backed up, though an add-on install rewrites them. | Fixed: both added (names checked in UXP's XPIProvider/AddonRepository). Simulation: a test's `addons.json` removed, `extensions.ini` put back. |
| P2 | `test-site-profile.sh` | Backup completeness was `ls DIR \| wc -l`: hidden and nested entries uncounted, contents unchecked. | Fixed: entry count by `find` and every file's md5, checked against the profile at setup and before a restore touches anything. Simulation: a changed nested file, an added hidden file, a changed backed-up file and an old-format backup each → "nothing restored", profile untouched. |
| P2 | `test-site-profile.sh` | `dev()` masked failures: a daemon that did not start after setup, or a backup that was not removed, were reported as done. | Fixed: markers for each (DAEMON-UP, BACKUP-GONE); cert9.db and user.js checked by md5 after the put; a setup stopped part-way says to run restore; each device script has its own name and removes itself. Simulation: a daemon that does not start → setup exits 1 saying so. |
| P2 | `personas.js` | The update check offered any different version (a downgrade too) and counted wrong-id or versionless answers as checked. | Fixed: `Services.vc.compare(new, installed) > 0`; an unusable answer counts as not checked. Device: round 18. |
| P2 | `jihadPersonas.js` | Updating or re-installing the worn Persona (same id) recorded no Undo. | By design, written into `cavekit-personas-ui.md` R3: that replaces the Persona after its own confirm; the version it replaced is gone from the list and, for a site Persona, its picture from the site, so Undo could not bring it back. |
| P3 | `jihadPersonas.js` | A colour-only Persona's drawn picture was opaque even for translucent colours. | Fixed: RGBA when either colour is translucent; an opaque pair is written byte for byte as before (its bytes name the Persona). Node: opaque output identical to the old function's; the translucent one decodes as RGBA with alpha 128→255. |
| P3 | `jihadPersonas.js` | A multi-picture static theme failing part-way left the pictures already moved in `jihad-themes`. | Fixed: the pictures this install put in place are removed; one that was already there stays. Code. |
| P3 | `BrowserApp.js` | The ink reload/stop glyphs for a light field were added only if the lazy browser view existed. | Fixed: asked of `ActionBar.prototype` (Enyo 1 makes the kind a global, `Oop.js:137`). Device: round 18. |
| P3 | `main-assistant.js` (Mojo) | Highlighted menu labels kept `popup_text` (set on them with `!important`), never `popup_highlight_text`. | Fixed: the highlight colour set on the labels of a selected or pressed row. Device: round 18. Enyo checked: its held class is on the item itself, the caption inherits. |

Not verified on the device: the TouchPad was not connected. Owed there: `test-site-profile.sh` on
a scratch app directory (`JIHAD_TEST_APP`) under the device's BusyBox 1.17 (the simulation used
dash and GNU tools), then the about:personas update check, the Enyo glyphs and the Mojo menu.

### Riverstone-Addons, the inspector on `cdaed4c` (the round-16 fixes) — REVISE, fixed in `b9810ad`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `install.sh` | `nginx -t && rc-service nginx restart` does not stop a `set -e` script: rules this server's nginx rejects were left in /etc/nginx, the run stamped done, and the next nginx restart (a reboot) would fail for every site on the server. | Fixed: every nginx file the run writes is kept until `nginx -t` accepts the new ones; a rejection puts them back (a file that did not exist is removed) and stops. Scratch: with an nginx that rejects everything, the old files came back and nothing restarted. |
| P2 | `install.sh`, `swap-in.sh` | The stamp was only written at the end, never cleared: a run that stopped part-way, then a rollback, was told "nothing else to set up". | Fixed: the stamp is removed when a run starts. Code (root on the server). |
| P2 | `install.sh`, `DEPLOY.md` | A rollback across a migration: no dump, no warning; and new code went live before its migration (for the whole install.sh run, or for good if it failed). | Fixed: `deploy/server/migrate.sh`; `swap-in.sh` runs the NEW copy's before the swap, with a dump first (`db-before-*`); a migration only adds and runs twice harmlessly (kit R10, DEPLOY.md, migrate.sh); a rollback names the migrations the copy going back lacks. Throwaway MariaDB 10.11 through `swap-in.sh` (`RS_SRV` + `RS_MARIADB`): applied before the swap; rollback named it; a failing migration left the live copy alone. |
| P2 | `smoke-test.sh`, `DEPLOY.md` | A failing renewal reached nobody: the smoke test runs only when the owner runs it, yet R10 was ticked. | Fixed: `watch.sh` (cron.daily): the served certificate (three weeks) and the backup (two days), to `state/health` and `/etc/motd.d`; the panel's front page shows it and says when the check stopped; install.sh stops without cron. Scratch: well, unreachable, near-expiry (an `openssl s_server` with a 10-day certificate), old and first backup; the panel on the local copy for each state, markup shown as text. |
| P3 | `install.sh` | Migrations neither atomic nor idempotent; names checked after the database was touched; schema.sql before migrations. | Fixed in `migrate.sh`: names first; each migration and its record in one session; idempotent by rule; on an existing database migrations first, then schema.sql. MariaDB: a failing migration is not recorded and applies once fixed. |
| P3 | `install.sh` | The stamp hashed the copy install.sh was in, but it applied `www`'s files. | Fixed: only the copy in place may run (except `--check`, `--inputs-hash`). Code. |
| P3 | `swap-in.sh` | Sending the live commit again replaced the way back with a second copy of it. | Fixed: refused (same manifest). Scratch. |
| P3 | `swap-in.sh` | www.prev was compared by names only: an edit made there was deleted. | Fixed: checked against its manifest too. Scratch: the edited file named, nothing changed. |
| P3 | `swap-in.sh` | A cut-off run's advice deleted the copy one back; a failure in the renames ended silently. | Fixed: it names `rm -rf www.prev && mv www.old www.prev`; a failed rename puts the live copy back or says the site is down; a failed `www.prev` step warns and goes on. Scratch for the advice; the rename failures in code. |
| P3 | `up.sh`, `smoke-test.sh` | The local 30-day test certificates would fail the new 21-day check from Oct 20. | Fixed: re-issued (the CA too) under three weeks. Scratch: 10-day → made again, 30-day → kept. |
| P3 | `up.sh` | The local copy never applied migrations (schema.sql only on a new volume). | Fixed: `up.sh` runs `migrate.sh` (TCP, after the image's restart); the init-script schema is gone. Local: restarted onto its existing database, `rs_migration` made, 34/34. |
| P3 | `install.sh`, `swap-in.sh` | A site block landing as `.new` still counted as applied. | Fixed: `swap-in.sh` names a pending `.new`. Code. |
| P3 | `pack.sh` | Any branch carrying `swap-in.sh` was sent. | Fixed: only `jihad`. `pack.sh --no-send`: 1347 entries, the new scripts 0755. |

Not verified: anything that needs root on Devuan (OpenRC's cron and nginx, `/etc/motd.d` at login).

## Round 18 — Codex on the site's deploy kit (`upstream/TRUNK..b9810ad`, `deploy/` only; credits back 12:28 PM 2026-10-12); the device checks owed by round 17

The site range has never fitted in one Codex window (it stopped at 150k–370k tokens three times), so
it was split: the deploy kit (`deploy/`, 1878 diff lines) and the PHP site (the rest). The deploy
part finished; the PHP part ran out again (retry 5:29 PM). Codex was also told to stay off the web
(round 16's run spent its window on web searches).

### Riverstone-Addons, deploy kit — 2 P1, 5 P2, 1 P3; all checked against the code, all fixed in `9e5521b`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `install.sh` | No `php-cli` in the package list, though install.sh's own config read, `make-owner.php` and `import-personas.php` run `php`. | Fixed: `php$PHPV-cli` installed by name, and listed by `--check`. Code (root on Devuan). |
| P1 | `watch.sh` | Only the database dump was checked: a night whose datastore archive failed after a good dump looked healthy for good. | Fixed: both archives checked, each named in the problem. Scratch: datastore archive stale or never made → its problem only; a migration's `db-before-*` dump does not count as the database's backup. |
| P2 | `up.sh` | The local copy never imported the Personas, so after `down.sh --wipe` it was not seeded as R9 says. | Fixed: imported at every start (stops if the import fails; unchanged entries not printed). Local restart: "0 changed". Not run on a wiped copy (it would lose the test Persona the device tests use). |
| P2 | `import-personas.php` | Live files changed before the preview and the row: the other-format header deleted first; a failed preview or SQL left files and row disagreeing. | Fixed: picture and preview made as `.new` beside the live ones; row and log in a transaction; files swapped in by rename (the ones before kept as `.old`) and only then committed; a failure rolls the row back and puts the files back; the other format goes after the commit. Local, a throwaway Persona: an update the database refuses (a trigger) and one whose preview cannot be swapped in (a directory in its place) each left picture, preview, version and log as they were; an allowed update swapped both, no `.new`/`.old` left. |
| P2 | `smoke-test.sh` | Required 401 from the panel: with the allowlist set, a test from the server gets 403 and every deploy "fails". | Fixed: 403 accepted as closed for the login page and registration (registration answered 403 too). Local: 34/34 without and with an allowlist (`deny all`). |
| P2 | `install.sh` | An unknown argument (`--chek`) ran a real install. | Fixed: usage and exit 2. Scratch: `--chek` and `--check extra` refused. |
| P2 | `make-owner.php` | Making the owner or resetting the password was not logged (R8: every change). | Fixed: a `log` row follows in the same input. Local: the row written at `up.sh`. |
| P3 | `swap-in.sh` | `extras()` saw only files and links: an empty directory or a special file in a copy went unnoticed (and was deleted with `www.prev`). | Fixed: directories with nothing listed inside and anything not a plain file are listed with find's type letter. Scratch: an empty dir, a nested one, a pipe, a link and a dir holding only an unlisted file each listed; a real archive of HEAD lists nothing; swap-in on a scratch server: first copy, the same copy refused, an empty dir refused, two updates with links. |

### Device, 2026-10-12 (the TouchPad connected; one task at a time)

- Bundle `eaa828a1…` pushed to all three (`push-engine-update.sh --daemon-only`), the id read back
  from each; Enyo `BrowserApp.js` and Mojo `main-assistant.js` pushed with `push-card-js.sh`, each
  card's boot stamp seen in the device log.
- `test-site-profile.sh` under BusyBox 1.17 on a scratch app (`JIHAD_TEST_APP=/tmp/fakeapp`, a copy
  of Enyo's profile): setup → backup made, CA in, daemon up; a test's changes (a removed, a changed
  and an added picture, a hidden nested file in each folder, prefs, two files that did not exist);
  a stray file planted in the backup → restore refused, profile untouched, daemon up; the profile
  left as a restore cut off between its moves leaves it → restore finished; the profile
  md5-identical to before setup.
- Real cycles on Enyo and Mojo: each profile (both folders, the certificate files, the Persona
  picture) md5-identical afterwards; no CA, no user.js, no backup.
- about:personas (Enyo, local copy): the test Persona installed from the site at 1.0.3; the site at
  1.0.2 → "Your Personas are up to date."; at 1.0.4 → an Update card for 1.0.4 (round 17's P2).
- Enyo glyphs: a card started on the start page, wearing a theme with a light address field: the
  theme's ink rule present, reload 2 of 2 and stop 4 of 4 drawn ink; the screenshot shows the ink ×
  (round 17's P3).
- Mojo menu: the share menu with the colour test theme: a plain row's text rgb(251, 233, 255)
  (popup_text), a row given the `selected` class rgb(42, 18, 64) on rgb(224, 168, 0)
  (popup_highlight_text on popup_highlight) — the stylesheet's answer, the class set by the hook,
  not a finger (round 17's P3).
- about:personas shows "Licence: MPL-2.0" on the five Personas drawn for the project.
- Test tooling: the Enyo hook reports the glyphs (`G`) and runs on the start page (the browser view
  may not exist yet); the Mojo hook reads a submenu's colours (`H`); `card-selftest.sh` launches
  with no address for `-`.

## Round 19 — the inspector on `7cd6a158` (Settings: Personas, Extensions and Plugins; Codex out of credits until 5:29 PM)

APPROVE with 1 P2 and 4 P3; all checked against the code and fixed in the commit after `7cd6a158`.

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `preferences.js`, `GoannaRenderPage.cpp` | Flash set to "Ask to Activate" (state 1, offered by about:addons) never plays here — nothing activates a click-to-play plugin — yet the new Plugins row said "plays when you tap it", the Content pane's Adobe Flash row (On/Off only) showed an empty list, and the cards' switch (`getFlash`, `state != 0`) said on. | Fixed: the row says it is set to Ask to Activate, which this browser cannot do, so it does not play, and to choose Always Activate; a choice row shows a stored value it does not offer as itself (the Flash row: "Ask to Activate (does not play here)"); the daemon reports only state 2 as on. Device, Enyo (daemon `4d8502ab…`, prefs.js edited with the daemon stopped and put back md5-identical): state 1 → the three panes say so, `getFlash` `enabled:false`; state 0 → "off" everywhere; unset → `enabled:true`. |
| P3 | `preferences.js` (About) | About read `navigator.plugins`, which leaves out a disabled plugin: Flash off read "not installed" while the Browser pane said "off". | Fixed: About reads the plugin host. Device, Enyo: "Shockwave Flash 10.3 d185 (off)" with Flash off. |
| P3 | kits | personas-ui R5 still described Settings → Appearance and Manage Personas; cross-references in personas-ui and addons-extensions R10 dangled; the overview counted 7 requirements in preferences-ui. Settings no longer led to the Personas manager (take off, remove, Undo). | Fixed: the kits name the new route; the overview says 8; about:personas' mention of the Personas manager is now a link that opens about:addons on its Personas list. Device, Enyo: the link lands on the Personas list (about:addons had last shown Plugins). |
| P3 | tests | The Plugins wordings recorded as checked "in node" had no test in the repository. | Fixed: `render/goanna/test/prefs-addons-rows.test.js` (8 cases, the functions read from the page itself). |
| P3 | `preferences.css` | The three navigation buttons used the red of a confirming Clear. | Fixed: their own neutral blue (`btn-nav`). Device, all three. |

Checked clean by the inspector: every name goes in as text; nothing in the group can stop the rest
of the pane from rendering; a late extension count writes to a detached node harmlessly;
about:addons opens on `extensions.ui.lastCategory` for both lists, and the Plugins category is
never hidden; the layout holds at 768 and 1024 wide.

## Round 20 — the inspector on Tier 78 (Codex out of credits until 5:29 PM, 2026-10-12)

Both ranges APPROVE (no P0/P1). Jihad `4e480610..6128e29b`: 3 P2, 8 P3; Riverstone-Addons
`9e5521b..26a9fdf`: 2 P2, 11 P3 — the site's handed to the subagent that built it.

### Jihad-Browser — fixed in the commit after `6e7b31a0`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `jihadPersonas.js` `growToList` | Every view of about:addons sits laid out in its deck, so the measure took the updates list and the detail view; a stretched list measured its own stretch, so the page only ever grew. | Fixed: the min-height cleared first; only the deck's selected panel; its visible items' extent, not its box; the detail view only when shown. Device: pending (a long list; the owner's finger). |
| P2 | `BrowserPageGoanna.cpp` `emitScrollIfChanged` | A jump the page made within 300 ms of an adapter scroll was taken as seen and never echoed (a fragment, scrollIntoView, a focus scroll right after a pan): the card stayed put over an engine that had moved. | Fixed: inside the window the echo only waits — the engine's position is not taken as seen — and the old near/far test runs once the stream pauses (pump calls it every tick). |
| P2 | `CREDITS.md` | Cutting the credits to the two bundled dropped those of three CC pictures still in the README's screenshot strip. | Fixed: a "shown in the README's screenshots" table with their licences and sources; the README's Personas text updated. |
| P3 | `personas.js` | An AMO card drawn again while its theme was fetched offered Install again (a second fetch, a second confirm); after Undo it stayed "In use". | Fixed: a card for a file being fetched says Fetching; AMO cards offer Install again after any later change. |
| P3 | `jihadPersonas.js` `fetchAmoTheme` | `createUnique` outside the try (a full /tmp left the button Fetching); a redirect to http on an allowed host passed; pictures not kept read as "not a theme". | Fixed: inside the try; the final URL must be https; a neutral message. The "worn" test stays (a re-install of the worn one under another name is a corner case, noted). |
| P3 | `jihadPersonas.js` `mine` | The ten formerly bundled Personas lost their licence lines on Your Personas' cards. | Fixed: FORMER_CREDITS by name and author. |
| P3 | `personas.js` | A bundled Persona worn from "Included" showed twice until the page reopened. | Fixed: that list is drawn again after a change. |
| P3 | `JihadWebView.js` (Mochi) | The native "flick" listener rested on an unverified premise. | Confirmed wrong and replaced: LunaSysMgr calls `Mojo.handleGesture(type, props)`, which Enyo 1 defines (compatibility/webosGesture.js) and Enyo 2 does not, so no flick ever reached Mochi's adapter. Mochi now defines it the same way (the element last touched decides). |
| P3 | `jihadPersonas.js` | addons.css and the pane's CSS in one try; the two sheets missing from the bundle's file check; addons.css applied on the desktop too. | Fixed: separate tries; checked in the bundle; addons.css only where a shell is named. |
| P3 | dead code | The pane's hidden Install-from-file row; make-personas.py's landscape helpers. | Removed. |
| P3 | kits/docs | R8 ("asks no site until a search"), R9 (third-party files moved), preferences-ui's gallery link, README, T-420, chrome-look R3's "[x] cause found". | Corrected (R3's cause criterion is [~] until a finger confirms). |

## Round 21 — Codex out again (17:3x, 389k tokens, no findings written; retry 10:29 PM); the inspector on both ranges

Codex's notes before it stopped named the keyboard-down blur and the AMO install flow as the
riskiest paths. The inspector took its place.

### Jihad-Browser `6e7b31a0..a90b7fe3` — APPROVE, 1 P2, 10 P3; fixed in the commit after `a90b7fe3`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `BrowserPageGoanna.cpp` / `GoannaRenderPage.cpp` | The keyboard-down blur ran page script (blur, focusout) inside the `setWindowSize` socket callback: a sync XHR in a blur handler spins a nested loop that can reap the page, and the callback then writes freed memory — the crash class that rebooted the device. It also nulled the type target after a handler refocused the field. | Fixed: `setWindowSize` only owes the blur (`mBlurOwed`); the pump does it after the input and change flushes; `isBusy` keeps the tick fast while owed; the type target is left to the engine's blur listener. |
| P3 | `GoannaRenderPage.cpp` | A field in a frame: the listener (top document only) never sees its blur, so the shell was never told. | Fixed: when the listener saw nothing, the daemon clears the target and lowers the editor state itself. |
| P3 | `addons.css` | `display:-moz-box !important` showed hidden and unsupported settings. | Fixed: no `!important` (the id rule outranks the skin), `:not([hidden="true"])`. |
| P3 | `addons.css` | A checkbox row with a long description and horizontal radio groups could still overflow, now clipped. | Fixed: the title box flexes (wraps); radio groups stack. Device check needs an add-on with such settings. |
| P3 | `addons.css` | A dead `.textbox-input` rule; editable drop-downs' text unthemed. | Fixed: `html\|*.menulist-editable-input { color: inherit }`, the dead rule removed. |
| P3 | `jihadPersonas.js` | Card pictures as JPEG turned PNG transparency black; 800 px drew 15:1 headers 1.65x upscaled. | Fixed: PNG kept PNG; 1400 px; the width is part of the name, so the 800 px ones are replaced and pruned. |
| P3 | `jihadPersonas.js` | Pruning deleted pictures of installed Personas past the 60 listed. | Fixed: their names are kept. |
| P3 | `jihadPersonas.js` `growToList` | A phantom pan on short detail pages; a 0 px coordinate taken as unset. | Fixed: only overflow counts; `=== null`. |
| P3 | Mochi `JihadBrowser.js` | The blur's change event submitted a half-typed address. | Fixed: the typed text is marked as already submitted before the blur. |
| P3 | Mojo `main-assistant.js` | The commit of a Jihad page opened by `openUrl` blurred the address a second time. | Fixed: `openUrl` marks it done; the commit skips it once. |
| P3 | docs | The flick comment's reason, R3's "(the adapter refuses others)", a misplaced doc comment, a dead PANE_CSS rule, CREDITS' "unmodified". | Fixed. |

Not fixed, noted for the owner's device check: a paste into a page field in Mochi with the keyboard
up (the clipboard read moves the focus; the expected order is safe).

### Riverstone-Addons `26a9fdf..d66fbc5` — APPROVE, 1 P2, 7 P3; fixed by the site agent in `2923301`..`5c1d54c`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `install.sh` | Whether to install the real-IP file was decided by a grep across /etc/nginx, wrong both ways (another site's server-level snippet or a disabled file turned ours off; a symlinked http-level one was missed and nginx -t failed every run). | Fixed (`2923301`): the ranges are included in our own server block; the grep is gone; an old conf.d copy is removed. Scratch root: 5 cases; a throwaway nginx with an http-level setting of its own passes `nginx -t`. |
| P3 | `import-personas.php` | rgb() mixing numbers and percentages in the space form passed (UXP refuses it); vertical tab taken as whitespace. | Fixed (`08265d4`), 17 cases. |
| P3 | `import-personas.php` | A colour respelled bumped the version. | Fixed: canonical comparison. |
| P3 | `import-personas.php` | A NULL browserId set later skipped the twin check. | Fixed; migration 002 adds a unique key where no duplicates exist. |
| P3 | `smoke-test.sh` | php's stderr parsed; non-integer pages broke the arithmetic silently; page/total/https unchecked; an empty feed passed. | Fixed (`cf1abe4`): 43/43 locally, bad feeds refused, an empty feed fails. |
| P3 | `smoke-test.sh` | Behind Cloudflare it read the edge certificate. | Fixed: `RS_SMOKE_CONNECT=127.0.0.1:443` on the server. |
| P3 | credits | CREDITS, the MIT text and NOTICE unreachable by visitors; Free Palestine's credit lacked the title. | Fixed (`5c1d54c`): `/credits/*.txt`, palettes' licenseURL there, "based on "Village of Battir" by Labour Palestine, CC BY 2.0" (Jihad's bundle index matches). No version bumped. |
| P3 | kits | R3 said forwarded headers are never trusted; R4 overstated one id everywhere; layered ids. | Fixed in the kits (R3 exception; R4 gaps recorded as open). |

## Round 22 — Codex out again at 22:32 (all four ranges hit the limit; retry Oct 18th, 2026 11:59 AM); the inspector on what no one had reviewed

Codex's first range ran 146k tokens and wrote nothing before the limit; the other three stopped at
once. The inspector took the ranges no review had seen yet: Jihad `a90b7fe3..0821484f` (the round-21
fixes, card pictures in the background, fonts), Jihad `23ffd9af` (the engine on RB_20261006 and its
tooling) and Riverstone-Addons `d66fbc5..jihad` (the round-21 site fixes). Tier 78 and
`6e7b31a0..a90b7fe3` had the inspector in rounds 20 and 21; Codex is owed all four ranges when its
credits return.

### Jihad-Browser `a90b7fe3..0821484f` — APPROVE, 1 P2, 11 P3; fixed in the commit after `23ffd9af`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `BrowserPageGoanna.cpp` pump | The keyboard-down blur ran before the field's `change`: the unforced flush waits while the field is still the active element, so `change` came a tick after `blur`/`focusout` (chrome-look R4 says after). | Fixed: the owed branch forces the change, then blurs. Device, Enyo: a page logging its field's events, a tap, "abc" typed through the adapter, the keyboard put away by the system (`enyo.keyboard.forceHide`; the daemon saw 942 → 651 → 942) — title `order:focus,input,change,blur`. |
| P3 | `GoannaRenderPage.cpp` `BlurFocusedEditable` | "Did the listener see the blur?" read a flag a focus event before it (a change handler) could have set. | Fixed: `PollEngineFocus()` drains it before `ClearFocus`. |
| P3 | `addons.css` | The id rule showed settings of types the manager has no binding for (it hides every `setting` but those). | Fixed: only the ten known types. The first device look caught a regression of the fix (the bool rule lost on specificity, Palefill's checkbox fell under its title); the bool rule now matches the general one's specificity. Device, Enyo: Palefill's preferences, the checkbox beside its title. |
| P3 | `preferences.js` | Font names written with `setCharPref` (UXP's narrows each character to a byte). | Fixed: `nsISupportsString` through `set/getComplexValue`, as UXP's own font preference. Device, Enyo: Georgia picked from the list → `user_pref("font.name.serif.x-western", "Georgia")`; Restore → no `font.` pref left. |
| P3 | `preferences.js` | x-unicode lists every font, so the "phantom" Noto Sans Thai comes back. | Checked on the device, and it is no phantom: `/usr/share/fonts/HeiT_nb.ttf` is Noto Sans Thai (Google, Thai only; fontconfig's cache and `fc-query` agree). The filter is right; the comment and preferences-ui R9's evidence were wrong and are corrected. |
| P3 | `preferences.js` | A size not in the list (the card's minimum of 2 px) was added at the end. | Fixed: sorted in. Device, Enyo: "None, 2 px, 9 px …". |
| P3 | `preferences.js` | The same deep link followed twice does nothing (no hashchange). | Not fixed, noted: taking the link out of the address is not possible — `replaceState` throws on an about: page (UXP's docshell asks the URI for a user:password part an about: URI cannot give) and `location.replace` to an empty fragment scrolls to the top. A first try with `replaceState` was found dead on the device and removed. |
| P3 | `jihadPersonas.js` / `personas.js` | The queue's "done" redrew every card (every picture requested again, a pending "Remove it?" reset). | Fixed: the notice says `thumbs`, and the gallery swaps only the pictures that changed. Device, Enyo: one card picture deleted, the gallery reopened — made again, no error, the cards drawn. |
| P3 | `jihadPersonas.js` | A failure outside `makeThumb` could leave the queue without its timer, and re-queueing never restarted it. | Fixed: the timer is set after the duplicate check; `nextThumb` reschedules in a `finally`. |
| P3 | `jihadPersonas.js` | Small headers decoded again after every restart; undecodable ones at every opening. | Fixed: an empty `<thumb>.small` marks a small header (kept and pruned with its picture); a failure is remembered until the daemon restarts. |
| P3 | Mojo `main-assistant.js` | `_keyboardAwayDone` was a bare flag: a pushState on the page being left used it up. | Fixed: openUrl keeps the page it opened; only an arrival on that Jihad page uses it up. Device, Mojo: address focused (window 705), about:settings opened as typed → focus on the page, window 996. |
| P3 | kit, tests | personas-ui R9's text (budget, 800 px JPEG evidence); no desktop test of the fonts section. | Kit corrected. The desktop test waits for the desktop build on RB_20261006 (docs/ENGINE.md §8). |

### Jihad-Browser `23ffd9af` (the engine update) — APPROVE, 3 P2, 8 P3; fixed in the commit after `23ffd9af`

The inspector checked every EngineCalls wrapper against the IDL and headers, every converted call
site against its old values, the six overlapping files against upstream, that `format-patch` of the
new branch is byte-identical to the queue, the generated install scripts, the tag sort, and gold's
`--icf=safe` and Thumb-2 interworking on ARM (no address-comparison risk; JIT and Flash interwork).

| Severity | File | Description | Disposition |
|---|---|---|---|
| P2 | `update-engine.sh` | Started from the queue without checking it matches the branch: an engine commit not yet `--add`ed would be left behind. | Fixed: refuses unless `third_party/uxp` is at `UXP_PIN` and `patch-queue.sh --check` passes. Run here: both pass, then "already on RB_20261006". |
| P2 | `update-engine.sh` | `--finish` rewrote the queue and removed the worktree before checking the submodule out; a failed checkout left `--abort` able to delete the only copy of the carried commits. | Fixed: checks first (worktree, a clean `third_party/uxp`, the API check), records `phase=finishing`, checks the branch out, then writes the queue and pins; a stopped run can be run again; `--abort` refuses once finishing. |
| P2 | `cavekit-engine-update.md` R8 | "All three variants" ticked with Mochi and Mojo evidence for one page each. | The missing checks run on the device: Mochi and Mojo — howsmyssl.com TLS 1.3 "Good", Palefill 1.30 enabled, about:config, about:settings, a Persona worn (toolbar painted) and taken off; Enyo — about:addons and about:settings. |
| P3 | `update-engine.sh` | On a conflict neither the overlap list nor the API check ran; the check's FAILs were cut to three lines and ignored. | Fixed: the overlap list comes from the old base and pin, before `git am`; the API check runs in full and stops on a FAIL, at the end of start and again in `--finish`. |
| P3 | `update-engine.sh` `pins()` | Rewrote the four files one by one; a miss half-way left them mixed. | Fixed: every file must hold the old or the new string, checked before any is written. `--pins` run: nothing changed. |
| P3 | `.gitignore` | `.update-in-progress` not ignored. | Fixed. |
| P3 | `check-engine-api.py` | The ICU check read the bundler, which no longer names the file. | Fixed: one ICU file in the tree; a WARN when it is not `ICU_DATA` (the update moves it). |
| P3 | `.gitmodules` and comments | `branch = jihad-engine-mods` and comments naming it. | Fixed: `jihad/uxp-RB_20261006`, rewritten by `pins()` and checked by `check-engine-pins.sh`; the recipe, `patch-queue.sh`, `EngineCompat.h` and `build/desktop/README.md` updated. |
| P3 | `EngineCalls.h` | The header said the checker covers what it does not; one positional `LoadURI` left. | Fixed: `LoadURIPlain`; the checker records `loadURI` and checks `ResizeReflowIgnoreOverride` and 60 app units to a CSS pixel. 0 FAIL, 0 WARN. |
| P3 | `docs/ENGINE.md` | Two contradicting §4.5 bullets; `--amend` for an earlier patch; the desktop step dropped from §6. | Fixed (fixup + autosquash; §6 step 4 is the desktop build and tests; §8 notes it is not done for RB_20261006). |
| P3 | `patch-queue.sh --add` | Mails without `--zero-commit --no-signature`. | Fixed. |

### Riverstone-Addons `d66fbc5..jihad` — APPROVE, 5 P3; fixed in `057dbad`

The inspector confirmed the round-21 fixes (real addresses in our own block, colours read as UXP's
parser reads them — 38 cases against `nsCSSParser.cpp`, credits at fixed paths, the parametrised twin
query, padding by characters).

| Severity | File | Description | Disposition |
|---|---|---|---|
| P3 | `migrations/002`, `migrate.sh` | Over an existing pair the key was skipped silently and the migration recorded, so it was never added. | Fixed: while the key is missing, every run tries again and names the pair. Local copy: a twin made → warning naming both; deleted → key added; a third run quiet. |
| P3 | `smoke-test.sh` | A feed ignoring `?page=` or short of its total passed. | Fixed: the page answered must be the one asked; the pages must hold the total. A fake feed of each kind fails. |
| P3 | `smoke-test.sh` | On the server most requests still went through Cloudflare (whose cache can answer for a broken server); a bare address meant port 4433. | Fixed: with `RS_SMOKE_CONNECT` every request goes to the server (`--connect-to`, the name and certificate kept); a bare address means 443. Local copy: 43/43 direct, a closed port fails. |
| P3 | nginx, `DEPLOY.md`, kit R3 | Another Cloudflare zone pointed at the server could pick the address the panel's limits see. | Fixed: the :443 block answers only its own name (`return 444` otherwise), inside our block, so no default server clashes with the server's other sites; the address is called a defence in depth; Authenticated Origin Pulls is the owner's option. A throwaway nginx: the name in any case or with a trailing dot answers, another is closed; the full configuration passes `nginx -t`. |
| P3 | `import-personas.php` | A listed Persona whose computed id another has was skipped whole. | Fixed: it keeps no id and is updated otherwise. Local copy: licence restored, id left empty, noted. |

### Found on the device while checking (not from a review)

- **The daemon** sent a page-made scroll before the page's new size, so the card clamped it to the
  page it still had: about:settings' `#section=fonts` (and any page that jumps to an anchor while it
  loads) stayed at the top ("scrolledTo 0,872" before "contentSize=768x2221"). The size now goes
  first, and without a valid one the jump waits a tick. Device: Enyo and Mochi open on Fonts & Colors.
- **Mojo** came up with the keyboard over the page when opened on one (Mojo focuses a scene's first
  text field). A card opened on a page, or for one a page opened, focuses nothing; the start page
  still focuses the address. Device: howsmyssl.com opened with the focus on the page, window 996;
  the start page with the address focused, window 705.
- **Install scripts**: one push could not put `/` back to read-only ("Device or resource busy" right
  after ls-hubd's rescan and db8's kinds; two tries a second apart), and every later push then saw
  `/` rw and left it so. The restore (postinst and prerm) is tried for about ten seconds. BusyBox:
  succeeds on the fourth try, or warns after ten with the exit trap kept; all three pushed again from
  a read-only `/` — back to read-only after each.

## Round 23 — Tier 83 (proxies and webOS's certificates); Codex out until Oct 18th, 2026 11:59 AM; the inspector

The inspector reviewed Jihad `5db7860c..e223591f` and Riverstone-Addons `057dbad..c322904`. Codex is
owed both ranges with round 22's (build site T-466).

### Jihad-Browser `5db7860c..e223591f` — REQUEST CHANGES, 1 P1, 4 P2, 10 P3; fixed in `9394c08b`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `jihadCerts.js` | The only test against a site's own certificate was `certType == CA_CERT`, i.e. NSS's `CERT_IsCACert`, which counts any self-signed v1 certificate as a CA; mozilla::pkix takes a v1 anchor. A router's v1 certificate accepted in webOS would have become a root for every site, through R4 (on by default) and R6. | Fixed: both require a v3 certificate whose own basicConstraints says cA=TRUE, read from its DER. Device, Enyo: a hand-built v1 self-signed certificate refused by the question ("not a certificate authority's certificate") and skipped by the import (notCA 1). |
| P2 | `jihadCerts.js` `dropSessions` | `logoutAndTeardown` logs out of every token, cancels every open TLS socket and forgets every temporary override (each Trust Once, the printer's). | Fixed: the daemon clears only NSS's session cache (`jihad-clear-tls-sessions` → `SSL_ClearSessionCache`, looked up in libssl3), plus idle connections. Device: "tls: session cache cleared", the next load asks. |
| P2 | `jihadCerts.js` `_caInline` | `LOAD_DOCUMENT_URI` includes frames: a hidden frame could raise "Trust …?" over the page; the certificate's name was shown unchecked. | Fixed: an HTTP CA certificate that is not the page itself is cancelled; data:/file: frames refused by a content policy; the question names who offered it; names stripped of control characters and capped. Device: a page with both kinds of frame raised nothing; "Offered by 192.168.1.245:8897". Removed roots refused. |
| P2 | `jihadCerts.js`, `make-cert-lists.py` | The Relay's CA excluded only by exact bytes from the list; a missing list let R6 accept it; the script derived the pin from the file it checked. | Fixed: fingerprint and key pinned in the component, the script refuses another; R4 never takes any squid-for-webos CA (subject); R6 says what such a certificate is. The CA is made per install by squid-for-webos's init script (research), so it is the owner's server's own. |
| P2 | `preferences.js` | The Relay's switch only in the Relay choice, though the Relay can be in use through webOS's setting or a typed proxy. | Fixed: shown wherever it is in use. Device: webOS's setting on the Relay, the switch shown. |
| P3 | `preferences.js` | WebOne decided by the mode alone; the Relay check ignored the HTTPS port; "webos" ignores the type. | Fixed the first two; "webos" left: the component rewrites the prefs at every read. |
| P3 | `preferences.js` | Ports were number fields (letters became "", then the default). | Fixed: text fields checked as numbers. |
| P3 | `preferences.js` | A locked pref could leave half a proxy written. | Fixed: checked before anything is written. |
| P3 | `preferences.js` | webOS's setting not read again when the section is shown (R1). | Fixed. |
| P3 | `jihadProxy.js` `pickEntry` | `networkTechnology` appears in no measurement. | Kept, harmless: the device's entry has none and is taken as "default". |
| P3 | `JihadLunaService.cpp` | Logged 300 characters of webOS's reply, which can carry a login. | Fixed: the length only. |
| P3 | `jihadCerts.js` | Switched off, the list and R6's "already trusted" still used the old list. | Fixed. |
| P3 | `jihadCerts.js` | An unreadable store file counted but not logged (R4). | Fixed: once per file. |
| P3 | `jihadCerts.js` | A revalidated (304) certificate was not seen. | Fixed: merged responses observed. |
| P3 | kit | HP's count 211 vs 212. | Both stated: 212 files, 211 distinct. |

Checked and fine (the inspector): no non-ASCII left in the components' strings; the 407 path uses
`AUTH_PROXY` and the channel's proxy info; about:settings is not reachable from content; R4's
temporary certificates stay in memory (UXP saves no intermediates on its own); the 1024-bit fallback
exists; the nss-ready order; the cmp fallback in the install scripts.

### Riverstone-Addons `057dbad..c322904` — APPROVE, 3 P3; fixed in `68caed1`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P3 | `install.sh` | Before a hash exists, a block older than the copy before still lands a `.new` once. | Documented in DEPLOY.md (move it in once; the next run records the hash). On the server now, the block is the copy before's, so the next run records the hash. |
| P3 | `install.sh` | `.new` deleted before `nginx -t`. | Fixed: after it passes. |
| P3 | `install.sh` | `live` not local. | Renamed. |

### Found on the device while checking (not from a review)

- JS components are read as Latin-1: non-ASCII in their strings was garbled on the card (the print
  status's "…", and the Persona importer's BOM strip never matched). Escaped (T-467).
- A reinstall that replaced an adapter LunaSysMgr had mapped left a deleted file on `/`, which kept it
  read-write after all ten tries. The install scripts now leave a byte-identical adapter alone (T-467).
- WebOne sends its CA as a download (`Content-Disposition: attachment`), which skips the engine's
  certificate import; Jihad opens a top-level CA certificate anyway (R6).
- After a pick in a card's drop-down that grows the page (Riverstone Relay in Settings), the card
  keeps its old picture until the next touch; open (T-465). **Withdrawn later the same day:** both
  pictures were taken before the pick ran (the shot at 30 s, the pick at about 30 s). Replayed
  exactly, a 30 s shot shows the old state and a 40 s one the new; 15 of 15 picks given time redrew
  within a second (Enyo, Mochi). `card-selftest.sh run` now marks hook lines logged after the shot.

## Round 24 — Codex's owed review, run by the inspector (the owner, 2026-10-13: "do the codex review with the cavekit 3.1 backup inspector agent instead")

Five inspectors in parallel, read-only, over the ranges Codex was owed (build site T-456, T-466):
Jihad `4e480610..6e7b31a0` (Tier 78), `6e7b31a0..5db7860c` (round 22), `5db7860c..9a38a708` (Tier 83);
Riverstone-Addons `9e5521b..d9359ea` (deploy kit and credits) and `upstream/TRUNK..d9359ea` (the PHP
fork). Codex is no longer owed these. Every finding was checked against the code before fixing.

### Jihad-Browser Tier 83 — REQUEST CHANGES, 1 P1, 1 P2, 5 P3; fixed in `cfbfbcf2`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P1 | `jihadCerts.js` R4 | Trust Always installs the site's own certificate in Certificate Manager's store (CertAddAuthorizedCert, which makes the hash link R4 reads). A router's or NAS box's certificate from `openssl req -x509` is v3 with cA=TRUE, so round 23's test passed it and R4 would have made it a root for every site at the next start. | Fixed: R4 takes only roots (self-issued, keyUsage with keyCertSign, no subjectAltName, no extendedKeyUsage), and never a certificate the daemon's SSL dialog installed (`jihad-platform-cert` → SHA-256 in `jihad.security.webos_certs.accepted`). Device: router with IP SAN → siteLike, without keyUsage → noCertSign, CA with keyCertSign → trusted; Trust Once logged the marker and the session serial was swept. |
| P2 | `jihadCerts.js` R6 | A blob: frame takes the Blob's type, reaches PSM's content listener, and raised the question over the page ("Offered by a file or page on this device", or a stale host). | Fixed: a one-use offer (20 s) is made only by a CA certificate opened as the page (HTTP in `_caLoad`; data:/file: in the content policy); without one the question is refused silently. Device: blob frame refused, no dialog; the HTTP link still asks. |
| P3 | `jihadCerts.js`, `make-cert-lists.py` | Removed roots matched by exact certificate only; an intermediate in a store would be an anchor. | Intermediates: fixed (notRoot). Subject+SPKI matching: tried and dropped. Mozilla removed old issues of roots it still trusts with the same name and key (GTS Root R1–R4, GlobalSign, Firmaprofesional: CCADB against CE's store), so it would refuse current roots. |
| P3 | `jihadCerts.js` `_caLoad` | Every non-document CA load cancelled, a script's fetch included. | Fixed: frames (subdocument, object) only. |
| P3 | `DialogService.cpp`, shells | "proxy " marker could come from a site named `proxy`. | Fixed: "[proxy] ". Device: Enyo, Mochi "The proxy …"; Mojo's sheet "[proxy] …". |
| P3 | `jihadProxy.js` | A failed read (daemon not yet on the bus, up to minutes) set direct. | Fixed: the last plan followed from webOS is kept (`jihad.proxy.webos.last`); direct only with none. Code: the failure cannot be brought about on demand. |
| P3 | `card-selftest.sh` | Times compared to the second; midnight. | Fixed: seconds of the day, `>=`, wrap. Local awk test across midnight; the device runs used it. |

Also fixed: the R6 question refuses a site's certificate that says cA=TRUE (device: ""192.168.1.1" is
not a certificate authority's certificate").

### Jihad-Browser round 22 — APPROVE, 2 P3; fixed in `cfbfbcf2`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P3 | `jihadPersonas.js`, `personas.js` | `jihad-personas-mine` with data "thumbs" also meant "Persona `thumbs` removed" (a site picks the id). | Fixed: its own topic, `jihad-personas-thumbs`. |
| P3 | `jihadPersonas.js` | Card pictures are decoded on the engine's thread (200–600 ms each, one per 0.7 s). | Written down (comment, personas-ui R9): once per Persona. |

### Jihad-Browser Tier 78 — APPROVE, 4 P3; fixed in `cfbfbcf2`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P3 | `jihadPersonas.js` | The bundled set was offered only when about:addons opened; a bundled Persona removed in the gallery came back there (R7). | Fixed: the gallery's list and mine requests offer it too. Code: every device profile already has edition 8, so only a fresh profile shows it. |
| P3 | `echo-server.py` | Opened the deleted `carbon-light.png`. | Fixed: read from `carbon_light-2.0.xpi`; served 200 locally. |
| P3 | `addons.css`, `personas.css` | `.button-link` (More, Undo, Restart now) and the toast's Undo under 44 px. | Fixed: 44 px. Finger round. |
| P3 | kits, comments | personas-ui R4/R5/R8 and Out of Scope, chrome-look R2, two comments described the pane before Tier 78. | Rewritten. |

### Riverstone-Addons deploy kit — APPROVE, 4 P3 + 1 kit gap; fixed in `71dd173`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P3 | `install.sh` | A block without the `riverstone-site.conf` include was overwritten silently (an owner's inlined edit); no certificate overwrote any block. | Fixed: only a missing block or the script's own challenge block is written over; any other is kept with `.new`; with no certificate the block before is kept as `.prev`. Scratch harness, seven cases. |
| P3 | `install.sh` | A stale `.new` kept when the live block was already the repository's. | Fixed: removed whenever the live block ends as the repository's, after nginx accepts. |
| P3 | `DEPLOY.md` | Step 11 said RS_SMOKE_CONNECT checks only the certificate locally; every request goes to the origin. | Fixed. |
| P3 | `catalogue/personas.json` | The five MPL Personas had no licence or source address. | Fixed: MPL's address, the credits as source; the importer updates the rows at version 1.0 (local copy). |
| gap | `cavekit-addons-site.md` R10 | Out of date. | Rewritten; R10's smoke criterion made [~] with what is the owner's. |

### Riverstone-Addons PHP fork — APPROVE, 4 P3 + 1 upstream P3; fixed in `f9bcb79`

| Severity | File | Description | Disposition |
|---|---|---|---|
| P3 | `developer.php` | Open redirect for the logged-in owner (raw REQUEST_URI after nginx normalised `//evil.example/%2E%2E%2Fpanel/addons/`). | Fixed: built from task/what/slug. Local copy: the crafted address → `/panel/administration/?task=submit&what=addon`. |
| P3 | `classDatabase.php` | A database outage answered 200. | Fixed: 503 with Retry-After. Local copy: DB stopped → 503, started → 200. |
| P3 | `classPersona.php` | Ordered by name only. | Fixed: name, id. |
| P3 | `personaFeed.php` | per_page of four digits fell back to 24. | Fixed. |
| P3 (upstream) | `classAccount.php` | Response time told whether a user name exists. | Fixed: a bcrypt check of the same cost for unknown names. Local copy: 0.22 s either way. |

Local copy after both site commits: smoke test 43/43. Live since 2026-10-14 10:26 UTC (T-470): 43/43
on the server and through Cloudflare; the five MPL Personas updated at 1.0; the feed 39 of 39.

## Round 25 — Tier 84 (one repository builds Jihad; R11); the inspector, Codex out until Oct 18th

Range `b37795fb..9ff7d71a` (T-471…T-477), read-only, launched twice: the first run died with the
PC's reboot mid-read (no verdict), the second gave **REQUEST CHANGES, 2 P1, 7 P2, 16 P3**. Every
finding was checked against the code before it was fixed. Fixes in the round-25 commit; the clean
clone (`Jihad/.jihad-cleanclone-test`) is the test bed for the clone-side ones.

| # | Severity | File | Finding | Disposition |
|---|---|---|---|---|
| 1 | P1 | `.gitmodules`, gitlink `third_party/uxp` | The engine pin `706d4782` is not on the public fork: `jihad/uxp-RB_20261006` was never pushed, so a clone anywhere else fails at the first submodule, and git's loop leaves the later ones unchecked too. | **Confirmed** (`git ls-remote`: only `jihad-engine-mods`). `tools/host/check-pins.sh` added: fetches every gitlink from its URL into a throwaway repository; it reports this one FAIL, the other ten ok. The push is the owner's call (nothing of this repository is pushed without being asked). R11 criterion 2 back to `[~]`; a criterion "pins are public" added. |
| 2 | P1 | `toolchain/build.sh` | No `--security-opt label=disable`: on an SELinux-enforcing host the toolchain container cannot read `/cfg`, `/ctng-src` or `/out`. | Fixed: the same run options as `in-container.sh`. Not exercised here (no SELinux); the kit says so. |
| 3 | P2 | `toolchain/build.sh`, `Dockerfile`, `build-toolchain.sh` | Only uid 1000 can build the toolchain: the image's `builder` is 1000 and `/home/builder` is its; the docker comment ("run as root") is wrong, ct-ng refuses root. | Fixed: `/home/builder` world-writable (ct-ng's paths unchanged, so the toolchain's bytes are), `HOME` set, podman keep-id with any uid, docker `--user`; rootless docker named unsupported. |
| 4 | P2 | `build-adapter-pdk.sh`, `build-adapter-arm.sh` | Every shipped `BrowserAdapterImpl.so` holds `~/…/qt4-extract/…/qstring.h`: `Q_ASSERT`'s `__FILE__`, active because `QT_NO_DEBUG` was not defined. The bytes depend on the checkout path, so "same adapter bytes" held on this PC only. | **Confirmed** (1 string). Fixed: `-DQT_NO_DEBUG` in both (what every release build of a Qt program has; the device's Qt is a release build, and an inline assert firing would have taken LunaSysMgr down). Rebuilt: no home string in any of the three impls (178,196 bytes each); the shims unchanged (`BrowserAdapterJihad.so` still `4f603df3`). R11 gains "builds do not depend on where the checkout is". |
| 5 | P2 | `build-all-device.sh` | `git submodule update --init` with no paths resets every submodule when one is missing. | Fixed: only the missing ones, by path; and a submodule at its pin with an empty working tree counts as missing (`--force` restores it: the clean clone had four such, see below). |
| 6 | P2 | `gen-sysroot-manifest.sh`, `build-all-device.sh` | A bad or interrupted `libpulse.so.0` transfer stays in place and the mere file counts as done; `--assemble` then fails on the digest without naming it. | Fixed: `.part` then `mv` after the sha256; the build script and `--assemble`'s `device` op both check the recipe's sha256. |
| 7 | P2 | README | The host list omits binutils (`ar` unpacks the Debian and Palm packages on the host) and xz. | Fixed in the README, DEVICE-BUILD.md and the kit. |
| 8 | P2 | `tools/review/codex-review.sh` | Fails open: a failed or empty `git diff` is reviewed as NO_FINDINGS, exit 0. | Fixed: `pipefail`, git's status checked, an empty diff refused. |
| 9 | P2 | the kit | R11 ticked on evidence that does not cover the clean-clone claim (no submodule or toolchain step ran in the clone; no SELinux, other uid or docker run). | Agreed: criteria 2 and 4 back to `[~]` with what is missing named. The clone did build the engine and the daemon from its own inputs: its daemon is byte-identical to this checkout's and the device's (`06e75a68`). |
| 10 | P3 | `build-all-device.sh` | `grep -q` on pipelines under pipefail. | Fixed: outputs captured, then tested. |
| 11 | P3 | `build-all-device.sh` | The glob copies every stale package version into `out-all`. | Fixed: only this run's three, by each appinfo.json's version; stale copies removed. (My first version read the version through `\| head -1` — the very SIGPIPE trap of #10; replaced by one awk.) |
| 12 | P3 | `gen-sysroot-manifest.sh` | Does not source `host-path.sh`, so standalone `--fetch-device-libs` misses the repository's novacom. | Fixed. |
| 13 | P3 | `build-mochi-ipk.sh` | An outside source's "last two path parts" is the login name for a checkout under `$HOME`. | Fixed: basename only. |
| 14 | P3 | `gen-sysroot-manifest.sh` | `rm -rf "$SYSROOT_OUT"` from the environment, unchecked. | Fixed: refused unless under `build/webos-oe`. |
| 15 | P3 | `in-container.sh`, `build-all-device.sh` | No environment reaches the container; `PDK_ROOT` ignored by the PDK check. | Fixed: the eight settings the scripts read are forwarded, a value outside the repository refused with the reason; the PDK check honours `PDK_ROOT`. |
| 16 | P3 | README | "Docker works too" untested. | Fixed: rootful only, and named a claim, not a record. |
| 17 | P3 | README, DEVICE-BUILD.md, tools/README.md | `sudo … novacomd &` stops on the password prompt; the `&&` chain backgrounds the build too. | Fixed: `sudo -b`, on its own line. |
| 18 | P3 | `.gitignore` | `qt4-extract.new` left by a digest mismatch shows as 2,922 untracked files. | Fixed: `qt4-extract*/`. |
| 19 | P3 | `build-novacom.sh` | A host with `gcc` but no `cc` passes the check and then probes with `cc`. | Fixed: `CC` from the compiler found, exported for both makefiles. |
| 20 | P3 | the kit | "seven device libraries": it is eight. | Fixed. |
| 21 | P3 | both fetchers | The cache-buster retry runs only when curl fails; a cached 200 with wrong bytes fails outright; curl's error hidden. | Fixed: `fetch_url` takes the pin, retries a wrong-bytes download past the cache, bounded (6 tries), `-S` keeps curl's error visible. |
| 22 | P3 | TOOLCHAIN.md | The desktop example writes to the repository root's `out/` (not ignored), without keep-id or the label option. | Fixed: `build/desktop/out`, the same options as `in-container.sh`. |
| 23 | P3 | `in-container.sh`, `toolchain/build.sh`, Dockerfiles | Images never rebuilt after a Dockerfile change; no `.containerignore` for the 6.7 GB and 498 MB contexts. | Fixed: the image carries its Dockerfile's sha256 as a label and is rebuilt when it differs (once for the images built before the label, from cache); `.containerignore` (`*`) in all three contexts, since no Dockerfile copies anything. Base images stay by tag: a digest pin would be a separate decision. |
| 24 | P3 | `build-all-device.sh` | The TouchPad check comes after the hour-long toolchain build. | Fixed: the container engine and, when libraries are needed, the device are checked first; the device-independent inputs fetched before the device ones. |
| 25 | P3 | `oe-env.sh` | In single-prompt sudo mode the build-webos checkout is root-owned inside the user's repository. | Fixed: `as_user` (`setpriv`) for the checkout, clone and pin fetch. |

**The clean clone, continued.** Its engine build died with the reboot and left `UnifiedBindings10.o`
truncated (written 08:06:08, newer than its source, no symbols): `mach build` kept it and libxul
failed to link on `MenuBoxObjectBinding::Wrap`. The 100 files written in that minute were deleted
and the build re-run: libxul and the daemon built, the daemon `06e75a68`. Then the adapter failed:
`nptypes.h`, `npruntime.h`, `npupp.h` under `adapter-include/npapi/webkit/npapi/` are symlinks into
`third_party/npapi-headers`, and in the clone that submodule (with mochi, mochi-sampler and
webos-sdk-redux) sat at its pin with an EMPTY working tree, left by the aborted first clone
(`git status` shows ` m`). `git submodule update --init --force` restored the four; the build
script now treats an emptied submodule as missing (finding 5's fix).

**Found by the R11 VM run (2026-10-14, Fedora 44, SELinux Enforcing, user uid 1001, podman 5.8.1,
KVM).** Finding 3's first fix did not hold: `--userns=keep-id` keeps the image's `USER builder` (uid
1000) as the process and only adds the host uid as a group, so the toolchain container could not
create `/out/src` ("Permission denied"; no SELinux denial). The uid-1000 PC never showed it, which
is exactly what the criterion was for. Fixed: podman runs the images that set a USER
(jihad-xtool-build, jihad-goanna-build) with `--userns=keep-id:uid=1000,gid=1000`, which maps the
invoking user onto builder; the process is builder and its files land on the host as the user
(shown in the VM). jihad-host-tools has no USER and keeps plain keep-id. The run was then started
again from the same clone.

**Second VM failure (2026-10-14):** with the uid fixed, crosstool-NG died in its own download
step (zlib: zlib.net keeps only its newest release; ct-ng 1.25.0's mirror list is from 2022). On
this PC every tarball sat in `out-toolchain/src/`, so no build here had downloaded one since the
first. Fixed: `toolchain/tarballs.manifest` (18 tarballs, sha256, size, candidate URLs, generated
from that cache) and `toolchain/fetch-tarballs.sh`, run by `build.sh` before the container; ct-ng
gets `CT_FORBID_DOWNLOAD=y`, so a gap fails at once and by name. Every URL exercised from the PC:
18 of 18 fetched and matched in 45 s. ct-ng's `build.log` is now kept as `out-toolchain/ct-ng-build.log`.
`fetch_url` moved to `build/webos-oe/fetch-lib.sh`, sourced by the three fetchers.

## Licensing audit — "ensure that all software patent and trademark licenses in the repo are being respected" (the owner, 2026-10-14)

One read-only agent audited the working tree at `8ad1ff68`: what the three packages carry (the
device bundle listing, `readelf -d`), the licence texts that reach them, trademark use against
Moonchild's legal.shtml / redist.shtml / goanna.shtml fetched live, the codecs from the built
configuration, the PDK and HP library handling. **No P0.** Dispositions:

| # | Sev | Finding | Disposition |
|---|---|---|---|
| 1 | P1 | glibc 2.23 (modified by crosstool-NG), the GCC 9.4.0 runtime (GPL-3.0 + RLE 3.1) and eight jessie libraries ship with no notice, no FreeType disclaimer, no MIT/BSD notice, no source directions. | Fixed: NOTICE "Runtime libraries bundled" section (files, versions, holders, licences, the FreeType sentence) and a corresponding-source paragraph (tarballs.manifest, the ct-ng patches, jihad.defconfig, archive.debian.org's source packages); `licenses/` gains LGPL-2.1, GPL-3.0, the GCC exception (from the pinned tarballs) and Debian's copyright files for the eight (from the sysroot's `usr/share/doc`); the packages ship all of `licenses/` (checked in the payload preflight). |
| 2 | P1 | The shipped NOTICE names only pristine UXP; recipients are not told where the modified libxul's source is (MPL-2.0 3.2). | Fixed: the fork, branch, pinned commit, base release and the patches directory, in NOTICE's UXP section; covers the LGPL parts too. |
| 3 | P1 | README and LICENSE call the packages "Apache-2.0 and MPL-2.0" only. | Fixed: both name the runtime components' licences and point to NOTICE and about:license. |
| 4 | P1 | HP-derived files changed since import without an Apache 4(b) change notice. | Fixed on 29 files (the audit's 19 and 10 more by the same test: an HP header, more than one commit, no notice): a two-line notice after the licence header, pointing at the file's git history. |
| 5 | P2 | Google's logo was the icon of the three DuckDuckGo engines; an unused Bing logo shipped. | Fixed: a drawn magnifier (`list-icon-search.png`) for DuckDuckGo; the Bing icon removed; Google's for Google only. |
| 6 | P2 | The engine's `defaults/pref/services-sync.js` ships Moonchild's Sync, key-server and trusted add-on hosts. | Fixed at bundle time: `make-device-bundle.sh` blanks the six URLs and keeps only addons.mozilla.org in the trusted list, and fails if any palemoon.org line survives; the file stays so one-argument getCharPref reads still find a value. |
| 7 | P2 | LICENSE and ENGINE-SOURCE.md cite the 404 `licensing.shtml`. | Fixed: the UXP LICENSE and TRADEMARK files, legal.shtml and redist.shtml. |
| 8 | P2 | NOTICE says "see CREDITS.md", which did not ship. | Fixed: CREDITS.md in every package. |
| 9–18 | P3 | Tagline using the Goanna mark as a slogan; R3's "no strings" wording; about:credits at an unresolvable host (patch 0010); about:rights' MPL-only sentence; the "Blazer" keyword; Lato and the hyphenation patterns unnamed; the logo's and the screenshot strip's provenance; `--disable-ffmpeg`; the HP-terms sentence and the device-binaries ignore rule; stray files (a 2.3 MB screenshot, a `.bak`, `__pycache__`). | Fixed: tagline, R3, Blazer, Lato and hyphenation in NOTICE, CREDITS (logo: supplied by the owner, to confirm; strip CC BY-SA 3.0; Carbon Light's URI), the HP sentence in fetch-pdk.sh and DEVICE-BUILD.md, the ignore rules, the stray files removed. Deferred to the next engine build, since the patches are commits on the engine branch: about:credits' URL, `--disable-ffmpeg` (the system-libavcodec dlopen path is dead on the device: no libavcodec there, none shipped). Accepted: about:rights. |

**Patents:** nothing encumbered is built or shipped: ffvpx decodes VP8, VP9 and FLAC only; no
libavcodec, openh264, GStreamer, x264, AAC or Widevine in the bundle; `MOZ_AV1=''`, `MOZ_GMP=''`,
`MOZ_WEBRTC=''`; H.264 in Flash content is the device's own Adobe plugin, not shipped.
**Trademarks:** Pale Moon / Basilisk / Moonchild identity is out of libxul (zero hits), no upstream
logo ships, "Goanna" is used nominatively, HP/Palm/webOS nominatively with the README's disclaimer,
no Adobe or HP binary is vendored.

## Round 26 — Tier 86 (content blocking); the inspector, 2026-10-15

The inspector read the kit, `jihadContentBlocker.js` (as of compiler v2), the Settings rows, the
bundle and ipk scripts, the notices and the test pages, ran its own compiler over the pinned lists
(it reproduced the device's counts exactly) and read the UXP sources the code depends on.
**Verdict: REQUEST CHANGES** — 1 P1, 6 P2, 15 P3. Every finding and what was done (component v5,
`4ccb91f4` → this commit):

| # | Sev | Finding | Disposition |
|---|---|---|---|
| F-1 | P1 | The "own host" short-circuit ran before path rules, so `\|\|developer.mozilla.org/pong/` never fired; MDN's advert stayed; the kit contradicted itself. | **Fixed in v3 before the report arrived** (path rules decided before the own-host accept; device: `blocked fetch developer.mozilla.org`, MDN clean, `cb3-mdn`). v5 orders: exceptions, path rules, then host rules with the own-host exemption. Kit R1 rewritten. |
| F-2 | P2 | Exceptions with options (1,604) were skipped, turning EasyList's breakage fixes into blocks; `$generichide`/`$elemhide`/`$document` (173, LAN addresses among them) skipped; site exceptions against generic selectors (269) ignored. | **Fixed (v5):** optioned exceptions (path, types, party, `$domain=`, `~domain`) kept per host and checked first at every level; generichide sites and `://192.168.`-style prefixes scope the generic sheet away (`@-moz-document regexp` with a negative lookahead); a generic selector with site exceptions gets its own block excluding those sites. Kit R1/R3: "an exception is never silently dropped". |
| F-3 | P2 | R5's "never while a page is loading" not implemented. | **Fixed (v5):** no update work within 20 s of a document starting (the `document-element-inserted` observer timestamps); the kit says so. |
| F-4 | P2 | Cache path read both raw lists in full for the version, rewrote and re-parsed the sheet on every start; the compile tail did heavy single-tick work; v2's sheet 4.3× larger and unmeasured. | **Fixed:** the version from the first 2 KB; the sheet file written only when compiled and registered as it is on a cache load; the exception pass indexed by domain (v3: 21.6 s → 5.9 s); site rules out of the global sheet and loaded per document (v4/v5), so the global sheet is the generic 190 KB again. Memory re-measured on the device (the A/B in the ledger). |
| F-5 | P2 | Dependent strings pin the raw list text after a compile; `_css` kept for life; three daemons compile and update in lockstep. | **Fixed (v5):** installed from the cache's own JSON parse, never from the source text; the CSS text dropped once written; a random offset on the update timer. |
| F-6 | P2 | CREDITS.md and the README's alt text described a card view not yet on disk (Free Palestine still on the Enyo card). | **Fixed** (14:52): `cardview.jpg` recomposed from the Internet Archive (Mojo), BBC News (Enyo, with the blocker) and MDN without adverts (Mochi), all at the Default look on the Frutiger Aero backdrop — what the alt text and CREDITS say. |
| F-7 | P2 | Tests could pass without testing: `doubleclick.net` is not a blocked host; a file:// page has no page host; labels wrong; 40 of the 200 images were not refused. | **Fixed:** `ad.doubleclick.net` in the kit; `jihad-blocking.html` rewritten with the rule and the expectation per line, served over http from the host for the party cases; `jihad-blocking-many.html` from twenty plain-rule hosts. |
| F-8 | P3 | Typed rules for one host merged into one mask (click.aliexpress.com). | **Fixed (v5):** a list of `{types, party}` per host. |
| F-9 | P3 | An exception on a parent domain did not beat a block on a subdomain. | **Fixed (v5):** all exception levels first. |
| F-10 | P3 | Path rules dropped `$third-party` (154) and matched case-sensitively (428 with uppercase). | **Fixed (v5):** party per prefix; prefixes and paths case folded. |
| F-11 | P3 | Type map: imageset unmapped, beacon vs ping, object-subrequest unmapped, a dead fetch bit, wrong labels, downloads blockable. | **Fixed (v5):** imageset→image, beacon and ping→ping, object-subrequest→object, fetch→xmlhttprequest, labels corrected, `TYPE_SAVEAS_DOWNLOAD` never refused; the kit's type list aligned. |
| F-12 | P3 | A site selector with unbalanced brackets could swallow the rest of the sheet; procedural selectors emitted. | **Fixed (v5):** selectors with procedural pseudo-classes, `;`, `@`, `/*` or unbalanced brackets are counted as bad and dropped; site rules are per document anyway. |
| F-13 | P3 | Updater: `_updating` cleared early; no background/anonymous/uncached flags; no truncation check; no try/catch; `lastUpdate` not flushed; future dates; 6-hourly retries; partial success. | **Fixed (v5):** `_updating` held through the compile; `mozBackgroundRequest`, `LOAD_ANONYMOUS`, `LOAD_BYPASS_CACHE`, `INHIBIT_CACHING`; a download with under half the rules is refused as truncated; try/catch around the finish; prefs saved; a future `lastUpdate` clamped; a failed attempt waits a day (`jihad.blocking.lastAttempt`); `lastUpdate` advances only on success. The button forces it. |
| F-14 | P3 | The profile's download always preferred over a newer bundled snapshot; non-atomic writes; the plain-stream comment misdiagnosed the bug. | **Fixed (v5):** the newer of the two by version; atomic writes done right (flush, then finish); the comment says what happened. |
| F-15 | P3 | The master switch left the sheet registered. | **Fixed (v5):** hiding gated on both prefs. |
| F-16 | P3 | Refusals logged the page-chosen request host, by default, hundreds a page. | **Fixed (v5):** the matched list entry is logged, `jihad.blocking.log` off by default (on for test rounds). |
| F-17 | P3 | The page host computed (eight Xray reads) on every load before any lookup. | **Fixed (v5):** lazy; computed only when a rule needs the party or the own-host check. |
| F-18 | P3 | Redirects, preconnect and DNS prefetch bypass a content policy; the kit said "before they leave the device". | **Fixed (v5):** an `http-on-modify-request` check on `LOAD_REPLACE` channels; `network.dns.disablePrefetch` and `network.http.speculative-parallel-limit=0` as defaults; the kit says what is closed. |
| F-19 | P3 | Settings: a reflow every 3 s; the button not disabled while updating; no "no lists" state; comments. | **Fixed:** `textContent` compared first; the button disabled while `state().updating`; "No lists in this build"; the row-type comment and the help text. |
| F-20 | P3 | about:jihad, CREDITS and the lists' README incomplete; the bundle compared hashes with its own constants; REQUIRED and the ipk payload check without the lists, the component, the GPL text. | **Fixed:** about:jihad names the version, the dual licence and the choice, where the text ships; CREDITS the version and `blocklists/`; the README v5's sets; the bundle greps each hash in NOTICE and the README; REQUIRED and the ipk check list all three files and `licenses/GPL-3.0.txt`. |
| F-21 | P3 | `count-blocklist.py` copies the compiler's regexes. | **Partly:** fixed test vectors with hand-counted expectations (`count-blocklist-vectors.txt`, `--check`); still the same regex family — an independent tokenizer is a later task. |

Correct by the inspector's reading, kept: one instance (category cache + `do_GetService`), prefs at
start-up, the external content types, the domain walk and party, CSS cannot be injected, failure
handling of the cache and the updater, the plain-stream fix itself, the Settings rows' safety, the
packaging and the NOTICE entry (complete), taking the lists under GPLv3+.
