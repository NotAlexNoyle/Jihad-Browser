---
created: "2026-07-17"
last_edited: "2026-07-17"
---

# Device test 2026-07-17 — reported failures (post loading-screen batch)

User big-test on the TouchPad against daemon `871720dd` (commits df24fb8..1d9532b:
incremental progress, POST adopt, UI-only watchdog, Enter validation, VKB focus
reconcile). What the test reported, mapped to kits, with current hypotheses.
These entries are the AUTHORITATIVE open-issue list for the interaction domain —
kits carry matching REPORTED flags.

## T1 — Focus scroll pushes the page up off the screen (google)

**Report:** search box tappable; when focused, the whole page is pushed upward
off screen. Typing works.
**Kit:** cavekit-input-bridging R2/R2a (tap-to-focus behaviour).
**Hypothesis:** the tap path's scroll-restore (F-247: restore only when tapped
field is in the upper 55% of the viewport) is not holding on the REAL device
once the VKB resize lands: the app/adapter resizes the card (setWindowSize
smaller height) → reflow → engine or app scrolls the focused field "into view"
again AFTER our one-shot restore ran. Stock isis scrolls using field-rect info
the daemon never emits, so the app-side autoscroll may be operating on garbage.
**Next:** log scroll offsets around focus on-device; consider re-asserting the
restore after the VKB resize settles, or emitting the focused-field rect.
**INSTRUMENTED (2026-07-18, commit 8ebca68):** setWindowSize now logs engine
scroll before/after each resize + editable-focused state — the next repro shows
whether the push is the VKB-shrink reflow (engine) or pre-resize (adapter/app).

## T2 — Enter → full-card loading overlay, search never executes (google, ddg full)

**Report:** typing works; Enter shows the full loading overlay and "doesn't
work" — no results page. Same on duckduckgo.com.
**Kit:** cavekit-input-bridging R2a (Enter submit); cavekit-navigation-events R6.
**Hypothesis (two-part):**
1. On google/ddg FULL (JS-heavy) sites, the submit runs page JS which navigates
   via script (or pushState) — either a content-nav that our re-drive/adopt path
   mishandles on-device, or an IN-PLACE SPA update that produces NO load events
   at all. In the SPA case the daemon emits nothing and never repaints → looks
   dead (see T3 — likely the same root).
2. The overlay itself = msgLoadStarted from the GET re-drive/POST adopt with no
   completion reaching the app before the 12 s watchdog.
**Next:** on-device daemon log during an Enter press (state-change lines will
show whether a content nav STARTs at all); T3 paint fix first — it may reveal
that the result page actually rendered.
**ROOT CAUSE FOUND (2026-07-18, commit 11839bc):** HandleEnter clicked the
submit button via nsIDOMHTMLElement::DOMClick(), which the engine forbids from
native code — nsGenericHTMLElement::Click() → IsCallerChrome() →
SubjectPrincipal() MOZ_CRASHes with no JSContext on the stack. So Enter-to-submit
SIGSEGV'd the daemon; upstart respawned it, the app saw the socket drop = the
stuck full-card loading overlay with no results. Reproduced deterministically in
focus_test (gdb backtrace). Fixed: submit now via ClickElementSynthetic (mouse
events at the button rect). Retest T2 on device.

## T3 — Stale frame: old page sticks around until a tap/drag forces a repaint

**Report:** "the old page seems to stick around for longer than it should.
sometimes I have to put a finger down and tap or drag to see the current
rendering state."
**Kit:** cavekit-offscreen-rendering (paint delivery).
**Root (HIGH CONFIDENCE — architectural):** painting is gated on `mNeedsPaint`,
which is only set by: load completion, geometry change, queued edit actions, and
explicit input. There is NO invalidation-driven repaint: engine-internal changes
(incremental page render during load, async image decode after load-done, ALL
SPA/JS DOM updates, CSS animation) never set `mNeedsPaint`, so the shared buffer
holds the old frame until the user scrolls (drag → paint). This single root
plausibly underlies the visible half of T2 and T5.
**Fix direction:** invalidation-driven or deadline-driven repaint — see the
survey (Atlas paints on WPE frame-ready; stock BrowserServer painted on WebKit
invalidation callbacks). Cheap daemon-only interim: repaint (rate-limited) while
a load is in flight, for a settle window after completion, and after any
input/click; correct engine fix: expose PuppetWidget invalidation (dirty flag)
from the offscreen widget shim and paint on dirty.

## T4 — VKB won't come up on html.duckduckgo.com after visiting google + ddg

**Report:** after testing google and ddg in the same session, tapping the
html.ddg search field raises no keyboard; only a browser restart fixes it.
**Kit:** cavekit-input-bridging R2 (VKB raise); msgEditorFocused lifecycle.
**Hypothesis:** state-machine desync across page loads in one card. Candidates:
(a) the F-326 change lowers the VKB when the focus manager reports a non-text
focused element — if html.ddg's focus lands somewhere the check misreads (body,
anchor), every tap immediately emits editorFocused(false); (b) app-side
PalmSystem editorFocused state stuck from the previous page (we emit
true→false→true across navs; a missed transition wedges it). Order-dependence
(google/ddg first) points at carried state, not html.ddg itself.
**Next:** on-device `vkb tag=` + `editorFocused=` log lines while reproducing;
compare Atlas's editorFocused/keyboard lifecycle handling.
**FIX IMPLEMENTED (2026-07-18, commit 8ebca68, pending retest):** engine-driven
editorFocused — capture-phase focus/blur listener on the top document feeds the
VKB state machine (change-only emission; blur gives the app clean false
transitions to unwedge on; Atlas autofocus gate so load-time autofocus can't
grab the VKB before the first tap). Typing also retargets to the field the
ENGINE says is focused, not just the tapped one.

## T5 — Link clicks: full-card overlay, no navigation

**Report:** clicking links doesn't work; causes the loading overlay.
**Kit:** cavekit-navigation-events R6 (link re-drive); ipc R1.
**Hypothesis:** the tap IS detected (overlay = msgLinkClicked/openUrl →
msgLoadStarted), so the GET re-drive fires — but either the re-driven load never
completes on-device (overlay till watchdog) or it completes and renders while
the buffer stays stale (T3) so the user still sees the old page = "doesn't
work". Desktop link_test passes; device differs. T3 fix first, then re-test;
if still broken, capture the daemon state-change log for one link tap.
**PARTIAL ROOT (2026-07-18, commit 11839bc):** a link tap that landed on a
NON-anchor element (or an anchor wrapping other elements where the hit-test
resolved to a child) hit the non-editable ClickAt path, which called DOMClick()
= the same SubjectPrincipal MOZ_CRASH as T2. Real <a href> taps use the href
path (unaffected — link_test always passed), but taps on button/JS-onclick
"links" crashed the daemon. Fixed with the DOMClick removal. Retest with T3
paint fix + this.

## Explicitly deferred

- Login form (POST) end-to-end — user: "too ambitious" until the above are fixed.

## Architectural finding (2026-07-18) — SendMouseEvent doesn't fire DEFAULT actions

Root of both the DOMClick crash AND why removing it wasn't enough:
`nsIDOMWindowUtils::SendMouseEvent(mousedown+mouseup)` in this offscreen
embedding synthesizes the `click` EVENT (so onclick/JS handlers run) but does
NOT trigger the click DEFAULT ACTION (form submission, checkbox toggle). That is
why the original code called DOMClick() — which MOZ_CRASHes from native code
(no JSContext → SubjectPrincipal crash). Fix: for form submission, dispatch a
cancelable `submit` event + form->Submit() (FireFormSubmit — crash-safe, runs
onsubmit + validation). Applied to Enter (HandleEnter) and tapped submit
controls (ClickAt). Links keep working via the separate href path
(TakeClickNav→openUrl). Validated desktop: focus_test D (Enter) + E (tap submit)
both navigate. **Remaining default-action gaps (not yet handled): checkbox/radio
toggle on tap, `<label for>` activation** — add if the device test shows them.

## Test order for the next device session

1. T3 paint fix in place → re-run google Enter (T2), link tap (T5) — expect both
   may just start "working" visually.
2. T4 VKB repro with `palm-log` + daemon stderr capture.
3. T1 scroll capture: GetScrollXY before focus / after focus / after VKB resize.
