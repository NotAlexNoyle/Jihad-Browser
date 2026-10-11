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
