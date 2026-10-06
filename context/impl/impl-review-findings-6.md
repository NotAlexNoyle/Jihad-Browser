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
