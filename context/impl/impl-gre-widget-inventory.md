# What the GRE gives us that we do not use — XBL widget bindings

> **TWO DEFECTS IN THIS FILE, FILED BY T-140 ON 2026-08-10. Read before editing or trusting the
> back half.**
>
> **1. THIS FILE IS PHYSICALLY DUPLICATED.** Lines 274-450 are a byte-identical copy of lines
> 96-272 (177 lines, compared character for character): the whole `Chrome DOCUMENTS are available
> here` / T-110 per-page audit / T-128 correction / `Verdict for R4's first criterion` block
> appears twice, and the second copy opens mid-thought with *"That last paragraph is now partly
> STALE"* referring to a paragraph 178 lines behind it. **Nothing in the second copy is new** — do
> not read it as a later pass that reached a different verdict. **Practical consequence: any
> exact-match edit inside lines 96-450 will hit two places.** That is why the correction below is
> anchored here at the unique heading instead of being struck in situ. The fix is to delete lines
> 274-450; it was left to a session that can re-read the result rather than done blind.
>
> **2. "a XUL chrome *document*, which this embedding has never loaded" (in the T-128 correction
> section, both copies) is WRONG.** This embedding HAS loaded XUL chrome documents: `about:config`
> is `chrome://global/content/config.xul`
> (`third_party/uxp/toolkit/components/viewconfig/jar.mn:6`) and
> `render/goanna/test/xul_test.cpp:114` loads that chrome URL *directly* as its phase A, with no
> hardware; `about:addons` is a second, and its `richlistitem` rows were resolved and clicked on
> device 2026-08-04. What this embedding has never loaded is a XUL document **we wrote**. The
> difference is load-bearing: the false version turns "we chose HTML for `about:preferences`" into
> "XUL is impossible here" — the exact distinction-collapse that section was written to prevent.
> It also changes what re-opens cavekit-gre-widgets.md R4's second criterion, from "never" to "the
> day someone writes a XUL page".

**Investigated 2026-08-05**, prompted by discovering that `preferences.xml` ships unused while
writing cavekit-preferences-ui.md. The question: what else is in there.

## The inventory

**49 binding files ship in our dist** at
`dist/bin/chrome/toolkit/content/global/bindings/` (42 `.xml` in the source tree plus
generated/companion files). They are wired up in two different places, and the distinction is
the whole point of this note:

- **`chrome/toolkit/content/global/xul.css`** wires most of them for **chrome documents** —
  `button`, `checkbox`, `menulist`, `tree`, `richlistbox`, `tabbox`, `dialog`, `groupbox`,
  `listbox`, `textbox`, `toolbar`, `scale`, `notification`, `preferences`, `findbar`, and the
  rest. Any chrome page WE author gets these for free.
- **`layout/style/res/html.css`** wires two of them for **content** — i.e. for any website:
  - `videocontrols.xml#videoControls` on `<video>`/`<audio>`
  - `datetimebox.xml#{date,time}-input` on `<input type=date>` / `<input type=time>`

## Worth using

| Binding | Why it matters here |
|---|---|
| `preferences.xml` | **STRUCK 2026-08-05; this row was left stale until T-128 caught it 2026-08-10.** `<prefwindow>` is a *window* binding (dialog buttons, `instantApply`, opened as a chrome dialog) and there is no chrome WINDOW in this embedding. `about:preferences` shipped instead as a chrome-privileged HTML document. See cavekit-preferences-ui.md R1 for the decision, and the chrome-document/chrome-window section at the bottom of this file for why "no chrome window" does NOT mean "no chrome page". |
| `findbar.xml` | **This row's "caution" was WRONG and is corrected here (T-128, 2026-08-10).** There is no "missing selection controller" and `FindNext` does not SIGSEGV: the real cause was `nsWebBrowserFind::SearchInFrame` calling `nsContentUtils::SubjectPrincipal()`, which `MOZ_CRASH`es with no JSContext on the stack — an embedder calling from C++ has none, and `MOZ_CRASH` presents as SIGSEGV at address 0. Fixed by patch 0015; find works and is gated by `render/goanna/test/find_test.cpp`. The full correction is in cavekit-gre-widgets.md R6, which this row contradicted for five days. The BINDING itself is still out of scope, but for the other reason: it is chrome-WINDOW UI, and the find bar is card-side in all three variants. |
| `notification.xml` | In-content notification bars. We currently have no way to tell the user "add-on installed" / "cookies cleared" except a blocking card dialog, which is heavier than the message deserves. |
| ~~`scale.xml`, `numberbox.xml`, `spinbuttons.xml`~~ | **STRUCK 2026-08-10 (T-110) — this row was WRONG.** All three are XUL bindings wired by `xul.css`, so they need a XUL chrome *document*; the 2026-08-05 decision that struck `preferences.xml` one row up applies to them verbatim, because the only preferences page this project has is in-content HTML. The HTML equivalents are `<input type="range">` (unused — no row on `about:preferences` is a slider) and `<input type="number">` (used by 10 rows, and half of it does not work here — see the T-110 audit at the end of this file). |
| `dialog.xml`, `tabbox.xml`, `groupbox.xml`, `listbox.xml`, `richlistbox.xml`, `tree.xml` | Chrome page building blocks. `richlistbox` and `tree` are already exercised by `about:addons` and `about:config` respectively, so both are known to work in this embedding — `richlistbox` down to clicking a populated row's XBL-anonymous buttons on device 2026-08-04 (cavekit-addons-extensions.md R2), `tree` down to toggling a pref (cavekit-input-bridging.md R6). **NO CONSUMER — T-140, 2026-08-10.** All six are wired by `xul.css` and so need a XUL chrome *document*, and this project authors none: zero `.xul`/`.xhtml` files exist outside `third_party/`+`build/`, and the whole authored surface is the four HTML/no-widget rows of the T-110 audit below. They stay listed as the standing choice IF a XUL page is ever written, **not as pending work** — the near-miss is the Start Page Links editor (`preferences.js` `renderChromePane()`, `:516-580`), which is HTML on purpose. Full reasoning in cavekit-gre-widgets.md R4, second criterion. |

## Not applicable to this embedding
`browser.xml` and `remote-browser.xml` (a `<browser>` element and e10s — we ARE the browser, and
there is no second process), `autocomplete.xml` (the URL bar is card-side in all three variants),
`editor.xml`, `wizard.xml`, `colorpicker.xml`, `filefield.xml` (no file picker on this platform).

## An untested coverage gap: the two CONTENT bindings

`videocontrols.xml` and `datetimebox.xml` apply to any web page, not just to chrome we author.
**Neither has ever been tested in this browser**, and no kit covers them.

What was established 2026-08-05, on desktop:

- DOM-level support is present. `<input type=date>` keeps `type === "date"` (it is not downgraded
  to `text`), `value` round-trips an ISO date, and `<input type=range>` keeps `type === "range"`.
  `dom.forms.datetime` defaults to `true` in this engine; `dom.forms.datetime.timepicker` and
  `.others` default to `false`.
- Tapping a `<input type=date>` opened **no popup** (`popups=0` before and after).

What was **NOT** established, and must not be claimed: whether the XBL bindings visually attach.
The probe asked `GetAnonymousElementByAttribute` for anonids on the `<input>` itself, but the
structure is two levels deep — the input's frame creates a native-anonymous `<xul:datetimebox>`
child, and the XBL binding (with those anonids) is bound to THAT, not to the input. So the
`(no element)` results are a limitation of the probe, not evidence about the binding.

## VERDICT (2026-08-05): `datetimebox` — date/time input turned OFF

Resolved on device rather than by further probing, because the outcome the kit cares about
(cavekit-gre-widgets.md R2) does not depend on whether the binding attaches:

- The field rendered as a segmented `08 / 05 / 2026` control with a clear button and **no picker
  button**, and tapping it opened no popup. So it looked editable and offered no way to pick a
  date — R2 names that the worst of the three available outcomes.
- `packaging/prefs/jihad-platform-prefs.js` now sets `dom.forms.datetime` false, with the reason
  and the way back written next to the value. Both builds append that one file, so desktop and
  device cannot disagree.
- **Device-verified from a probe page that prints its own result**: before, `type=date | value=
  2026-08-05`; after, `type=text | value=2026-08-05`, rendering identically to a plain text
  input. The value survives the downgrade, and a text field is something the VKB and the
  engine's own editing keys already handle.
- The NAC-walk question is therefore moot and should NOT be re-run. It only becomes live again
  if someone turns `dom.forms.datetime` back on — at which point the picker is a XUL popup and
  goes down the cavekit-offscreen-rendering.md R7 path, or gets routed card-side the way
  `<select>` is.

**A deploy trap this exposed, worth more than the result.** The first attempt to apply the pref
silently did nothing. The device-side append was guarded with
`grep -q 'dom.forms.datetime"' "$G" || …` — and the file ALREADY contained the upstream
`pref("dom.forms.datetime", true)` at line 1098, so the guard matched and skipped the append on
every variant. It reported success. The measurement then said "STILL A DATE FIELD" and briefly
looked like the engine ignoring the pref. **Guard an idempotent append on YOUR OWN marker, never
on the name of the thing you are overriding** — upstream almost certainly sets it too, which is
why you are overriding it.

**Why this matters beyond neatness.** If the date/time bindings do attach, their picker is a XUL
popup — a separate display root, exactly like the `<select>` dropdown and the `about:addons`
tools menu, both of which needed the popup compositing and input routing work in
cavekit-offscreen-rendering.md R7 before they were usable. A date field on a real website would
go down that same path. And `<video>` controls are the entire media-playback story on this
device, which no requirement currently mentions at all.

**Next step for whoever picks this up:** reach the second level of anonymous content (walk from
the input's frame to its NAC `<xul:datetimebox>`, then query anonids on that), and separately
test `<video controls>` against a real media file — the codec question is upstream of the
controls question and is also unrecorded.

*(That last paragraph is now partly STALE — flagged 2026-08-10, left in place rather than rewritten so the sequence stays readable. The codec question WAS answered on device that day and is written up in cavekit-gre-widgets.md R1: VP8/WebM plays via the built-in ffvpx software decoder, H.264/MP4 and MP3 are refused outright, and `--disable-alsa` + `--disable-pulseaudio` left cubeb with NO backend, so nothing the ENGINE plays can make sound. The NAC-walk half of the next step is still open.)*

## Chrome DOCUMENTS are available here. Chrome WINDOWS are not. (T-128, 2026-08-10)

This distinction is the thing this file most needs to say, because three separate notes in this
project have collapsed it into "chrome doesn't work here" and one of them nearly struck a live
requirement (cavekit-gre-widgets.md R5). Establish it once:

**AVAILABLE — a chrome-privileged document loaded into the content docShell.** `about:preferences`
is one and has been since 2026-08-05. `render/goanna/components/jihadAboutPreferences.js:34`
resolves it to `chrome://jihad-prefs/content/preferences.html`; the `/content/` path segment makes
`nsChromeProtocolHandler::NewChannel2` set the channel owner to the SYSTEM principal
(`third_party/uxp/chrome/nsChromeProtocolHandler.cpp:178-192`); and the about: module omits
`URI_SAFE_FOR_UNTRUSTED_CONTENT`, the only flag that would clear that owner
(`third_party/uxp/netwerk/protocol/about/nsAboutProtocolHandler.cpp:218-224`). The daemon's own
load path supplies no triggering principal (`render/goanna/GoannaRenderPage.cpp:1590`), so
`nsDocShell` substitutes the system principal (`docshell/base/nsDocShell.cpp:1563-1571`) and
`CheckLoadURIWithPrincipal` returns `NS_OK` for it on sight
(`caps/nsScriptSecurityManager.cpp:685`) — the `URI_IS_UI_RESOURCE` gate that keeps web pages out
of `chrome://` is never reached. `render/goanna/test/xul_test.cpp` phase A asserts this without
hardware by loading `chrome://global/content/config.xul` directly.

**`about:preferences` being written in HTML is a document-LANGUAGE choice, not a security one.**
`IsChromeDoc()` is true for it; it calls `Services.prefs` and a device session proved the write
landed in `prefs.js` (cavekit-preferences-ui.md R3). "HTML not XUL" and "content not chrome" are
different axes. Do not read one as the other.

**NOT AVAILABLE — a chrome WINDOW.** No `nsIXULWindow`, no XUL `<window>` root, no `openDialog`
target, no chrome `<browser>` element above the content (which is why `amInstallTrigger` and
`AddonManager` both needed patches). THAT is what struck `preferences.xml` and `findbar.xml`, and
it does not generalise to any binding that merely lives inside a document — `notificationbox`
is the case in point.

**COSTLY BUT NOT IMPOSSIBLE — XUL popups**, which are separate display roots
(cavekit-offscreen-rendering.md R7); `<select>` is routed card-side instead.

**A trap for anyone testing a chrome page from outside:** `javascript:` URLs do NOT execute in a
chrome document here — `LoadURIWithOptions` returns `NS_OK` and nothing runs (measured 2026-08-04,
`render/goanna/GoannaRenderPage.cpp:749-755`). Drive real controls, or ship the code in the page.

---

# The about:/chrome surface we SHIP — per-page widget audit (T-110, 2026-08-10)

cavekit-gre-widgets.md **R4** asks which GRE widgets we actually depend on. This is the answer.

**METHOD: SOURCE READ ONLY. Nothing here was run — there was no device for this session.**
Every runtime claim below is either cited from an existing dated measurement elsewhere in the
context tree, or explicitly marked read-not-run. Do not upgrade any of it to "verified" without
a run.

## The complete list, and there is no fifth item

Derived from the only two installers that put anything into the GRE bundle —
`build/desktop/build-goanna.sh:60-126` and `build/webos-oe/make-device-bundle.sh:159-258` — and
cross-checked against the device bundle's own `chrome.manifest`, which carries exactly four
`jihad-*` lines (`build/webos-oe/device-bundle/chrome.manifest:66-69`).

| surface | what it is | widgets it depends on |
|---|---|---|
| `about:preferences` / `about:settings` | `packaging/prefsui/content/preferences.{html,js,css}`, served as a **chrome:// HTML document** by `render/goanna/components/jihadAboutPreferences.js` | native HTML only — full table below. **Zero XUL elements.** |
| `about:jihad` / `about:isis` | inline HTML built in C++ (`render/goanna/BrowserPageGoanna.cpp:627`, `jihadAboutPage`) and rendered through `setHTML` → `data:text/html` | **none.** `<div>/<h1>/<img>/<a>/<code>` plus one `<script>` that fills `navigator.userAgent`. No form control anywhere, so no widget dependency at all. It is a CONTENT document, so it could not carry a XUL binding even if it wanted one. |
| XPI web-install confirm | `render/goanna/components/jihadInstallPrompt.js` | **none — it routes out to the card.** Verified below. |
| `chrome://branding/` | `packaging/branding/` — `brand.dtd`, `brand.properties`, two PNGs | **none.** It exists only so the GRE's OWN `about:addons` XUL does not die on a missing DTD entity. A dependency *on* a GRE page, not a page we author. |

The three card shells (`app/`, `app-mochi/`, `app-mojo/`) are deliberately out of this audit:
they are rendered by webOS's own 2011 WebKit inside LunaSysMgr, not by the GRE, so no GRE binding
can reach them. `about:config` / `about:addons` / `about:support` are the GRE's own pages, not ours.

## `about:preferences` — the whole dependency, control by control

Every control is created in `packaging/prefsui/content/preferences.js`. There is no other DOM builder.

| widget | where | count | operable here? |
|---|---|---|---|
| `<input type="checkbox">` | `buildRow`, `row.type === "bool"` (`:443-445`) | 30 rows | **yes** — the non-editable tap path dispatches mousedown/mouseup and the engine toggles it; `GoannaRenderPage.cpp:2479` carries that history, including why the old hand-flip became a fallback. Not device-confirmed on THIS page. |
| `<select>` + `<option>` | `buildRow`, `row.type === "choice"` (`:452-459`) | 13 rows | **yes, device-proven.** `ClickAt` intercepts a dropdown `<select>` *before* any click (`GoannaRenderPage.cpp:2322-2342`) and hands it to the card via `BuildSelectPopup` → `msgPopupMenuShow`. Confirmed on this page in all three variants 2026-08-06 (cavekit-preferences-ui.md R6). |
| `<button type="button">` | pane tabs (`:642-651`), Remove (`:556`), Add Link (`:571`), Restore Defaults (`:578`), `preferences.html:33` | 5 kinds | **yes** — an injected coordinate click on the Advanced pane tab switched panes, screenshot-confirmed 2026-08-06. NB the harness's `clickid`/`DebugClickElement` does NOT fire an HTML `<button>`'s listener; drive buttons by COORDINATE or you will read a harness bug as a product failure. |
| `<input type="number">` | `buildRow`, `row.type === "int"` (`:466-468`) | 10 rows | **NO — see the section below. This is the finding.** |
| `<input type="text">` | `row.type === "string"` (`:466-468`); start-link Name (`:540-544`) | 3 rows + n | **no** — keyboard entry is MEASURED not to work on this page (cavekit-preferences-ui.md R6, 2026-08-06) and a text field has no other input method. |
| `<input type="url">` | home-button target (`:501-504`); start-link Address (`:548-552`) | 1 + n | **no** — same, and `url` is in the same `textlike` set. |
| `<label>` | `buildRow` (`:434`) | every row | works by construction: `ClickAt` resolves a tapped `<label>` to its control (`GoannaRenderPage.cpp:2347-2359`). |

**No `<textarea>` anywhere** — `intl.accept_languages` is a comma list in a plain text input.
**No `<input type="range">` anywhere**, though 10 rows are numeric; see the struck `scale.xml` row above.

**No bespoke re-implementation of any control exists in the shipped surface** — no div-based
checkbox, no hand-rolled dropdown, no custom slider. The pane switcher is `<button>`s rather than
a `<select>`, which is documented at `preferences.html:26-27` and is NOT a re-implementation: HTML
has no tab widget and `tabbox.xml` is XUL. `preferences.css:151-170` restyles the native controls
(border/background/size on `.cb`, `.sel`, `.txt`); whether that drops `-moz-appearance` native
theming was **not investigated** and does not bear on R4 — a restyled `<select>` still routes
through the daemon's card popup.

## `<input type="number">`'s spin buttons are UNREACHABLE here — read out of the source, not run

Three facts compose, and each was read this session:

1. **It is pure HTML NAC, not a XUL binding.** `nsNumberControlFrame::CreateAnonymousContent`
   (`third_party/uxp/layout/forms/nsNumberControlFrame.cpp:352-450`) builds
   `div > (input + div > (div, div))` with the `::-moz-number-spin-up`/`-spin-down` pseudos.
   `spinbuttons.xml` is never involved — which is why striking it from the table above is safe.
2. **`ElementFromPoint` CANNOT return that NAC.** `nsIDocument::GetContentInThisDocument` skips
   every frame whose content `IsInAnonymousSubtree()`
   (`third_party/uxp/dom/base/nsDocument.cpp:8211-8218`), so a tap on a spin arrow resolves to the
   `<input>` itself. `ClickAt` takes its element from exactly that call
   (`render/goanna/GoannaRenderPage.cpp:2234`).
3. **`ClickAt` treats `type="number"` as textlike** (`GoannaRenderPage.cpp:2381`), and the editable
   branch focuses the field, raises the VKB and **`return`s at `:2454` without ever dispatching
   mousedown/mouseup**.

So no click can reach a spin arrow, by construction. Combined with the already-measured fact that
typing does not work on this page, **the 10 `int` rows, the 3 `string` rows and the Browser pane's
url fields have NO working input method today** — they display a value and cannot change it. That
is exactly the decorative-control outcome `preferences.js` refuses elsewhere: it drops the
JavaScript toggle and the user-agent row for that reason (`:65-69`, `:80-88`).

**This is not an R4 violation.** The page uses the platform's control instead of re-implementing
one, which is what R4 asks. It is an INPUT-PATH gap and belongs to cavekit-preferences-ui.md R6 /
cavekit-input-bridging.md R6 — T-131 is already the right next step and needs no change. It is
recorded here because "which GRE widgets do we depend on" is R4's question, and
`<input type="number">` is the one whose honest answer is "half of it cannot be operated".

## `jihadInstallPrompt.js` routes out to a card dialog — VERIFIED at both ends (source read)

The component builds **no UI of any kind**:

- `render/goanna/components/jihadInstallPrompt.js:46-59` — fills an `nsIWritablePropertyBag2`
  (`accept=false`, `host`, `names`) and raises a SYNCHRONOUS
  `Services.obs.notifyObservers(bag, "jihad-xpi-confirm", null)`.
- `render/goanna/DialogService.cpp:195-244` — `JihadXpiConfirmObserver::Observe` sanitises the two
  content-controlled strings, calls `gSink->OnDialog(DialogKind::Confirm, …)` at `:240`, and writes
  the answer back into the bag at `:242`.
- `DialogSink` is implemented by `BrowserPageGoanna` (`render/goanna/BrowserPageGoanna.h:137`),
  whose `msgDialogConfirm` (`:78`) is the frozen YAP message each card answers in its own idiom.

Default is DENY at every missing link: bag default (`jihadInstallPrompt.js:50`),
`reply.accept = false` (`DialogService.cpp:232`), and a null sink logged as `"NONE — denying"`
(`:239`).

It exists precisely BECAUSE the toolkit's own answer (`amWebInstallListener.js`) opens a modal XUL
**chrome window**, which this embedding has none of — the same finding that struck
`preferences.xml` and `findbar.xml`. So it is R4 working in its third mode: where the platform's
widget assumes a chrome window, route out rather than re-implement.

## Correction to the framing of T-128 ("is a chrome-loaded about: page reachable AT ALL")

The question as written is already answered, and the answer is **yes for an HTML chrome document**.
`about:preferences` IS one: `jihadAboutPreferences.js:42-53` resolves it to
`chrome://jihad-prefs/content/preferences.html`; the `content jihad-prefs prefsui/content/` line
gives it the system principal (the `/content/` path segment is load-bearing — the reason is written
into `packaging/prefsui/jihad-prefsui.manifest`); and the page renders, switches panes and opens
`<select>` popups on device in all three variants.

What is genuinely untested is narrower, and is what T-128 should ask: whether a XUL element
(`<notificationbox>`) can be created and BOUND inside an HTML chrome document, or whether R5 needs
a XUL chrome *document*, which this embedding has never loaded. **Do not read the
`about:preferences` result as evidence either way — that page contains no XUL.**

## One small unrelated gap found while enumerating

`render/goanna/EngineHost.cpp:675-676` allowlists the about: pages whose JS-console output is
logged: `about:config`, `about:addons`, `about:preferences`, `about:support`. **`about:settings`
is missing**, and `jihadAboutPreferences.js` registers it as a second contract on the same class.
Impact is small today — the page's script errors carry
`chrome://jihad-prefs/content/preferences.js` as their source name and are caught by the
`chrome://` branch one line up, so only an error whose source name is the document URL would be
dropped, and `preferences.html` has no inline script. No change made (an audit should not quietly
alter daemon behaviour), but it will bite whoever adds one.

## Verdict for R4's first criterion: MET

Every surface this project ships either uses the platform's own control unmodified (native HTML on
the in-content page) or routes out to the card (the install prompt); the two credit pages have no
controls at all. No bespoke re-implementation exists anywhere in the shipped surface. R4 as a
whole stays OPEN on its second criterion (richlistbox/tree as the standing preference — T-140,
behind T-128), which this audit does not touch.

*(That last paragraph is now partly STALE — flagged 2026-08-10, left in place rather than rewritten so the sequence stays readable. The codec question WAS answered on device that day and is written up in cavekit-gre-widgets.md R1: VP8/WebM plays via the built-in ffvpx software decoder, H.264/MP4 and MP3 are refused outright, and `--disable-alsa` + `--disable-pulseaudio` left cubeb with NO backend, so nothing the ENGINE plays can make sound. The NAC-walk half of the next step is still open.)*

## Chrome DOCUMENTS are available here. Chrome WINDOWS are not. (T-128, 2026-08-10)

This distinction is the thing this file most needs to say, because three separate notes in this
project have collapsed it into "chrome doesn't work here" and one of them nearly struck a live
requirement (cavekit-gre-widgets.md R5). Establish it once:

**AVAILABLE — a chrome-privileged document loaded into the content docShell.** `about:preferences`
is one and has been since 2026-08-05. `render/goanna/components/jihadAboutPreferences.js:34`
resolves it to `chrome://jihad-prefs/content/preferences.html`; the `/content/` path segment makes
`nsChromeProtocolHandler::NewChannel2` set the channel owner to the SYSTEM principal
(`third_party/uxp/chrome/nsChromeProtocolHandler.cpp:178-192`); and the about: module omits
`URI_SAFE_FOR_UNTRUSTED_CONTENT`, the only flag that would clear that owner
(`third_party/uxp/netwerk/protocol/about/nsAboutProtocolHandler.cpp:218-224`). The daemon's own
load path supplies no triggering principal (`render/goanna/GoannaRenderPage.cpp:1590`), so
`nsDocShell` substitutes the system principal (`docshell/base/nsDocShell.cpp:1563-1571`) and
`CheckLoadURIWithPrincipal` returns `NS_OK` for it on sight
(`caps/nsScriptSecurityManager.cpp:685`) — the `URI_IS_UI_RESOURCE` gate that keeps web pages out
of `chrome://` is never reached. `render/goanna/test/xul_test.cpp` phase A asserts this without
hardware by loading `chrome://global/content/config.xul` directly.

**`about:preferences` being written in HTML is a document-LANGUAGE choice, not a security one.**
`IsChromeDoc()` is true for it; it calls `Services.prefs` and a device session proved the write
landed in `prefs.js` (cavekit-preferences-ui.md R3). "HTML not XUL" and "content not chrome" are
different axes. Do not read one as the other.

**NOT AVAILABLE — a chrome WINDOW.** No `nsIXULWindow`, no XUL `<window>` root, no `openDialog`
target, no chrome `<browser>` element above the content (which is why `amInstallTrigger` and
`AddonManager` both needed patches). THAT is what struck `preferences.xml` and `findbar.xml`, and
it does not generalise to any binding that merely lives inside a document — `notificationbox`
is the case in point.

**COSTLY BUT NOT IMPOSSIBLE — XUL popups**, which are separate display roots
(cavekit-offscreen-rendering.md R7); `<select>` is routed card-side instead.

**A trap for anyone testing a chrome page from outside:** `javascript:` URLs do NOT execute in a
chrome document here — `LoadURIWithOptions` returns `NS_OK` and nothing runs (measured 2026-08-04,
`render/goanna/GoannaRenderPage.cpp:749-755`). Drive real controls, or ship the code in the page.

---

# The about:/chrome surface we SHIP — per-page widget audit (T-110, 2026-08-10)

cavekit-gre-widgets.md **R4** asks which GRE widgets we actually depend on. This is the answer.

**METHOD: SOURCE READ ONLY. Nothing here was run — there was no device for this session.**
Every runtime claim below is either cited from an existing dated measurement elsewhere in the
context tree, or explicitly marked read-not-run. Do not upgrade any of it to "verified" without
a run.

## The complete list, and there is no fifth item

Derived from the only two installers that put anything into the GRE bundle —
`build/desktop/build-goanna.sh:60-126` and `build/webos-oe/make-device-bundle.sh:159-258` — and
cross-checked against the device bundle's own `chrome.manifest`, which carries exactly four
`jihad-*` lines (`build/webos-oe/device-bundle/chrome.manifest:66-69`).

| surface | what it is | widgets it depends on |
|---|---|---|
| `about:preferences` / `about:settings` | `packaging/prefsui/content/preferences.{html,js,css}`, served as a **chrome:// HTML document** by `render/goanna/components/jihadAboutPreferences.js` | native HTML only — full table below. **Zero XUL elements.** |
| `about:jihad` / `about:isis` | inline HTML built in C++ (`render/goanna/BrowserPageGoanna.cpp:627`, `jihadAboutPage`) and rendered through `setHTML` → `data:text/html` | **none.** `<div>/<h1>/<img>/<a>/<code>` plus one `<script>` that fills `navigator.userAgent`. No form control anywhere, so no widget dependency at all. It is a CONTENT document, so it could not carry a XUL binding even if it wanted one. |
| XPI web-install confirm | `render/goanna/components/jihadInstallPrompt.js` | **none — it routes out to the card.** Verified below. |
| `chrome://branding/` | `packaging/branding/` — `brand.dtd`, `brand.properties`, two PNGs | **none.** It exists only so the GRE's OWN `about:addons` XUL does not die on a missing DTD entity. A dependency *on* a GRE page, not a page we author. |

The three card shells (`app/`, `app-mochi/`, `app-mojo/`) are deliberately out of this audit:
they are rendered by webOS's own 2011 WebKit inside LunaSysMgr, not by the GRE, so no GRE binding
can reach them. `about:config` / `about:addons` / `about:support` are the GRE's own pages, not ours.

## `about:preferences` — the whole dependency, control by control

Every control is created in `packaging/prefsui/content/preferences.js`. There is no other DOM builder.

| widget | where | count | operable here? |
|---|---|---|---|
| `<input type="checkbox">` | `buildRow`, `row.type === "bool"` (`:443-445`) | 30 rows | **yes** — the non-editable tap path dispatches mousedown/mouseup and the engine toggles it; `GoannaRenderPage.cpp:2479` carries that history, including why the old hand-flip became a fallback. Not device-confirmed on THIS page. |
| `<select>` + `<option>` | `buildRow`, `row.type === "choice"` (`:452-459`) | 13 rows | **yes, device-proven.** `ClickAt` intercepts a dropdown `<select>` *before* any click (`GoannaRenderPage.cpp:2322-2342`) and hands it to the card via `BuildSelectPopup` → `msgPopupMenuShow`. Confirmed on this page in all three variants 2026-08-06 (cavekit-preferences-ui.md R6). |
| `<button type="button">` | pane tabs (`:642-651`), Remove (`:556`), Add Link (`:571`), Restore Defaults (`:578`), `preferences.html:33` | 5 kinds | **yes** — an injected coordinate click on the Advanced pane tab switched panes, screenshot-confirmed 2026-08-06. NB the harness's `clickid`/`DebugClickElement` does NOT fire an HTML `<button>`'s listener; drive buttons by COORDINATE or you will read a harness bug as a product failure. |
| `<input type="number">` | `buildRow`, `row.type === "int"` (`:466-468`) | 10 rows | **NO — see the section below. This is the finding.** |
| `<input type="text">` | `row.type === "string"` (`:466-468`); start-link Name (`:540-544`) | 3 rows + n | **no** — keyboard entry is MEASURED not to work on this page (cavekit-preferences-ui.md R6, 2026-08-06) and a text field has no other input method. |
| `<input type="url">` | home-button target (`:501-504`); start-link Address (`:548-552`) | 1 + n | **no** — same, and `url` is in the same `textlike` set. |
| `<label>` | `buildRow` (`:434`) | every row | works by construction: `ClickAt` resolves a tapped `<label>` to its control (`GoannaRenderPage.cpp:2347-2359`). |

**No `<textarea>` anywhere** — `intl.accept_languages` is a comma list in a plain text input.
**No `<input type="range">` anywhere**, though 10 rows are numeric; see the struck `scale.xml` row above.

**No bespoke re-implementation of any control exists in the shipped surface** — no div-based
checkbox, no hand-rolled dropdown, no custom slider. The pane switcher is `<button>`s rather than
a `<select>`, which is documented at `preferences.html:26-27` and is NOT a re-implementation: HTML
has no tab widget and `tabbox.xml` is XUL. `preferences.css:151-170` restyles the native controls
(border/background/size on `.cb`, `.sel`, `.txt`); whether that drops `-moz-appearance` native
theming was **not investigated** and does not bear on R4 — a restyled `<select>` still routes
through the daemon's card popup.

## `<input type="number">`'s spin buttons are UNREACHABLE here — read out of the source, not run

Three facts compose, and each was read this session:

1. **It is pure HTML NAC, not a XUL binding.** `nsNumberControlFrame::CreateAnonymousContent`
   (`third_party/uxp/layout/forms/nsNumberControlFrame.cpp:352-450`) builds
   `div > (input + div > (div, div))` with the `::-moz-number-spin-up`/`-spin-down` pseudos.
   `spinbuttons.xml` is never involved — which is why striking it from the table above is safe.
2. **`ElementFromPoint` CANNOT return that NAC.** `nsIDocument::GetContentInThisDocument` skips
   every frame whose content `IsInAnonymousSubtree()`
   (`third_party/uxp/dom/base/nsDocument.cpp:8211-8218`), so a tap on a spin arrow resolves to the
   `<input>` itself. `ClickAt` takes its element from exactly that call
   (`render/goanna/GoannaRenderPage.cpp:2234`).
3. **`ClickAt` treats `type="number"` as textlike** (`GoannaRenderPage.cpp:2381`), and the editable
   branch focuses the field, raises the VKB and **`return`s at `:2454` without ever dispatching
   mousedown/mouseup**.

So no click can reach a spin arrow, by construction. Combined with the already-measured fact that
typing does not work on this page, **the 10 `int` rows, the 3 `string` rows and the Browser pane's
url fields have NO working input method today** — they display a value and cannot change it. That
is exactly the decorative-control outcome `preferences.js` refuses elsewhere: it drops the
JavaScript toggle and the user-agent row for that reason (`:65-69`, `:80-88`).

**This is not an R4 violation.** The page uses the platform's control instead of re-implementing
one, which is what R4 asks. It is an INPUT-PATH gap and belongs to cavekit-preferences-ui.md R6 /
cavekit-input-bridging.md R6 — T-131 is already the right next step and needs no change. It is
recorded here because "which GRE widgets do we depend on" is R4's question, and
`<input type="number">` is the one whose honest answer is "half of it cannot be operated".

## `jihadInstallPrompt.js` routes out to a card dialog — VERIFIED at both ends (source read)

The component builds **no UI of any kind**:

- `render/goanna/components/jihadInstallPrompt.js:46-59` — fills an `nsIWritablePropertyBag2`
  (`accept=false`, `host`, `names`) and raises a SYNCHRONOUS
  `Services.obs.notifyObservers(bag, "jihad-xpi-confirm", null)`.
- `render/goanna/DialogService.cpp:195-244` — `JihadXpiConfirmObserver::Observe` sanitises the two
  content-controlled strings, calls `gSink->OnDialog(DialogKind::Confirm, …)` at `:240`, and writes
  the answer back into the bag at `:242`.
- `DialogSink` is implemented by `BrowserPageGoanna` (`render/goanna/BrowserPageGoanna.h:137`),
  whose `msgDialogConfirm` (`:78`) is the frozen YAP message each card answers in its own idiom.

Default is DENY at every missing link: bag default (`jihadInstallPrompt.js:50`),
`reply.accept = false` (`DialogService.cpp:232`), and a null sink logged as `"NONE — denying"`
(`:239`).

It exists precisely BECAUSE the toolkit's own answer (`amWebInstallListener.js`) opens a modal XUL
**chrome window**, which this embedding has none of — the same finding that struck
`preferences.xml` and `findbar.xml`. So it is R4 working in its third mode: where the platform's
widget assumes a chrome window, route out rather than re-implement.

## Correction to the framing of T-128 ("is a chrome-loaded about: page reachable AT ALL")

The question as written is already answered, and the answer is **yes for an HTML chrome document**.
`about:preferences` IS one: `jihadAboutPreferences.js:42-53` resolves it to
`chrome://jihad-prefs/content/preferences.html`; the `content jihad-prefs prefsui/content/` line
gives it the system principal (the `/content/` path segment is load-bearing — the reason is written
into `packaging/prefsui/jihad-prefsui.manifest`); and the page renders, switches panes and opens
`<select>` popups on device in all three variants.

What is genuinely untested is narrower, and is what T-128 should ask: whether a XUL element
(`<notificationbox>`) can be created and BOUND inside an HTML chrome document, or whether R5 needs
a XUL chrome *document*, which this embedding has never loaded. **Do not read the
`about:preferences` result as evidence either way — that page contains no XUL.**

## One small unrelated gap found while enumerating

`render/goanna/EngineHost.cpp:675-676` allowlists the about: pages whose JS-console output is
logged: `about:config`, `about:addons`, `about:preferences`, `about:support`. **`about:settings`
is missing**, and `jihadAboutPreferences.js` registers it as a second contract on the same class.
Impact is small today — the page's script errors carry
`chrome://jihad-prefs/content/preferences.js` as their source name and are caught by the
`chrome://` branch one line up, so only an error whose source name is the document URL would be
dropped, and `preferences.html` has no inline script. No change made (an audit should not quietly
alter daemon behaviour), but it will bite whoever adds one.

## Verdict for R4's first criterion: MET

Every surface this project ships either uses the platform's own control unmodified (native HTML on
the in-content page) or routes out to the card (the install prompt); the two credit pages have no
controls at all. No bespoke re-implementation exists anywhere in the shipped surface. R4 as a
whole stays OPEN on its second criterion (richlistbox/tree as the standing preference — T-140,
behind T-128), which this audit does not touch.

---

## 2026-08-15 — T-139 and T-111 RUN on the desktop harness (the first execution of either)

Everything above about `notificationbox` was a source read. This section is a MEASUREMENT, from
two runs of a new harness binary. Nothing here is a device result: novacom was down.

**The harness.** `render/goanna/test/prefsui_test.cpp`, built and run by
`build/desktop/build-prefsui-test.sh` inside the pinned container. It loads the real
`about:preferences` (which `render/goanna/components/jihadAboutPreferences.js` resolves to
`chrome://jihad-prefs/content/preferences.html`), drives the page's own controls with SYNTHESIZED
TAPS, and asserts through the daemon's existing debug channel — `DebugElementRect`,
`DebugElementText`, `DebugAnonNodes`, `DebugClickElement`, `DebugGetPref`. **No pixel is read**:
desktop-harness pixel readback is a recorded dead end (`dead-ends.md`), and a criterion about the
DOM should not be answered from a frame buffer anyway. 38 checks, 0 failures, exit 0 — run twice,
once on the GTK-window widget and once on the OFFSCREEN PuppetWidget path (`JIHAD_OFFSCREEN=1`),
which is the one the device runs. **Identical results on both**, including every rect.

### T-139 — the notificationbox attaches and is dismissable. All four hazards answered.

| what | measured |
|---|---|
| the element exists | `#msgbar notificationbox` rect=0,160 0x0 — `initNotifyBox()`'s `createElementNS(XUL_NS, …)` inside an HTML document works |
| **the XBL binding attached** | `NAC:1 xul:stack[1] \| xbl=1: xul:stack` — the binding's own anonymous `<stack>` |
| the METHOD gate | a `<notification>` reached the DOM, which is only possible through `notifyBox()`, whose entire body is `typeof gNotifyBox.appendNotification === "function"` |
| the bar RENDERS | `notification[value="prefs-reset"]` rect=0,145 **599x36** — a laid-out bar, not a zero-height stub |
| the label | `notification[label="Home page and start page links restored to their defaults."]` resolves |
| the platform X is suppressed | `notification[hideclose="true"]` resolves |
| the Dismiss button | `notification button.notification-button` rect=493,148 101x29 |
| **dismissed = GONE FROM THE DOM** | after a real tap on Dismiss, `sel:#msgbar notification` → `(no element)`; same after a tap on the bar itself |
| the footer mirror | `#status` reads the message before AND after dismissal |

**Hazard 1 (xul.css into an HTML document) — NOT A PROBLEM, measured.** The binding attached and
the bar laid out at 599x36 in a document whose sheets started without `xul.css`.

**Hazard 4 (frame construction, not appendChild) — NOT A PROBLEM.** The
`getBoundingClientRect()` flush in `initNotifyBox()` is enough; `xbl=1` was already true at the
first probe, before any message was raised.

**Hazard 2 (transitionend may never fire offscreen) — CONFIRMED RELEVANT, and correctly handled.**
The bar's height is 36, i.e. NON-zero, so `_showNotification`'s `skipAnimation` shortcut
(`height == 0`) did NOT apply and the append took the animated slide-in path. Dismissal still
completed synchronously, because the page only ever calls `removeNotification(item, true)`. The
bar can be raised a second time after a dismissal, which is the state the note above warns can be
corrupted. **What this run CANNOT separate, and nobody should later claim it did:**
`dismissNotification()` deliberately pairs `removeNotification(item, true)` with a `parentNode`
sweep, and from outside the page the two are indistinguishable — both end with the node detached.
What is proven is the OUTCOME the criterion names: gone from the DOM, not merely invisible.

**Hazard 3 (`javascript:` URLs do not run in a chrome document) — CONFIRMED, and BROADER than
recorded.** The 2026-08-04 measurement was against `about:addons`, a XUL document. This run
repeats it against a chrome-privileged **HTML** document and gets the same answer:
`DebugRunChromeJs("javascript:void(document.title='chrome-jsurl-ran')")` returns and the title is
unchanged. So the finding is about CHROME DOCUMENTS, not about XUL. Two consequences worth
carrying forward:
- every assert in this harness is a real tap, because the inject channel's chrome-JS entry is not
  available as an instrument here;
- **`GoannaRenderPage::ScrollTo` is a no-op on a chrome page**, because it drives
  `window.scrollTo` through exactly such a URL. That cost the first draft of this test a whole
  run: the footer's `#reset-pane` sits at y≈854, the viewport was 768 high, the scroll silently
  did nothing and the tap landed on `<null>` — which reads as "the button does nothing". The
  harness now GROWS the viewport (`Resize()`, C++ all the way down) instead.

### T-111 — the eleven row-backing prefs, observed rather than argued

All eleven read the shared `packaging/prefs/jihad-platform-prefs.js` value on the desktop dist,
and **no row on either pane renders "Not available in this build."** (`.row.unavailable .row-pref`
→ `(no element)` on both `general` and `advanced`). The three the kit names as *silently stock*
before the split are the interesting ones and all three are now correct:

    layout.frame_rate           30      (stock -1)
    browser.cache.disk.capacity 51200   (stock 256000)
    network.prefetch-next       false   (stock true)

and the other eight: `general.smoothScroll=false`, `browser.sessionhistory.max_entries=20`,
`browser.cache.disk.enable=true`, `browser.cache.disk.smart_size.enabled=false`,
`browser.cache.memory.capacity=16384`, `network.http.max-connections=32`,
`network.dns.disablePrefetch=true`, `image.animation_mode=once`.

**Read this before re-running it: the answer depends on the PROFILE, and it caught me.** The
shared desktop profile (`build/desktop/out/.jihad/default/profile/prefs.js`) carries
`user_pref("browser.cache.disk.capacity", 358400)` — left over from before
`browser.cache.disk.smart_size.enabled` was pinned false, when the cache autosized itself from
free disk. A user value beats every default, so running this test against that profile reports
358400 and reads as the pref split having failed. It has not; that is one stale profile.
`build-prefsui-test.sh` therefore `rm -rf`s a dedicated `JIHAD_STATE_DIR` on every run, because
the criterion is about what the BUILD ships.

The desktop dist was refreshed for this run by re-running `build/desktop/build-goanna.sh`'s
POST-`mach` phases only (no engine rebuild — the dist was already current). Those phases installed
the prefs UI and re-appended the shared prefs, reporting *"34 pref lines re-appended to the desktop
goanna.js, verified byte-identical to the source files"* — so last wave's hardening of that block
is now exercised against a real dist, not only against a scratch copy.

### Deliberate-failure controls (an assert that cannot fail proves nothing)

- `JIHAD_PREFSUI_NEG=139` looks for a notification value the page never emits → 1 failure, exit 4.
- `JIHAD_PREFSUI_NEG=111` adds `jihad.no.such.pref` to the eleven → 2 failures, exit 4.

Both were run. The instrument fails loudly when its premise breaks.

---

# Every `msgDialog*` producer in the daemon, classified — T-148, 2026-08-15

Prompted by `../kits/cavekit-gre-widgets.md` R5's last criterion. The question asked of each
site was **"does the user's answer change what happens next?"** — if yes it is a question and
stays modal; if no it is a statement and must not be able to stall the render daemon. Blocking
here is not a figure of speech: `BrowserPageGoanna::OnDialog` opens a FIFO and POLLS it until
the card replies or the deadline expires (`render/goanna/BrowserPageGoanna.cpp:1150-1235`).

Source-read classification; the two MOVED/ADDED rows are also harness-verified (see
`impl-toast-channel.md`).

| # | Producer | Trigger | YAP message | Verdict | Why |
|---|---|---|---|---|---|
| 1 | `JihadPrompter::Alert` / `AlertCheck` (`DialogService.cpp:99,104`) | content `window.alert()` | `msgDialogAlert` | **KEPT MODAL** | The web platform requires `alert()` to pause script until dismissed. Making it non-blocking would change content behaviour, not just chrome. |
| 2 | `JihadPrompter::Confirm` / `ConfirmCheck` / `ConfirmEx` (`:111,117,125`) | content `window.confirm()` | `msgDialogConfirm` | **KEPT MODAL** | A boolean the page then branches on. There is nothing to do but wait for it. |
| 3 | `JihadPrompter::Prompt` (`:134`) | content `window.prompt()` | `msgDialogPrompt` | **KEPT MODAL** | Returns a string the page consumes. |
| 4 | `JihadPrompter::PromptPassword` / `PromptUsernameAndPassword` (`:145,152`) | HTTP auth and content | `msgDialogPrompt` | **KEPT MODAL** | A credential request. (Both currently hard-return `false` — headless never surrenders a password — but the shape is a question.) |
| 5 | `JihadPrompter::Select` (`:160`) | `nsIPrompt::Select` | `msgDialogConfirm` | **KEPT MODAL** | A choice among items. |
| 6 | `JihadXpiConfirmObserver` (`:179`) | `jihad-xpi-confirm` from `jihadInstallPrompt.js` `confirm()` | `msgDialogConfirm` | **KEPT MODAL** | The security decision of the whole add-on path — the answer is literally install-or-cancel, and the default is deny. |
| 7 | `ProxySink::msgSSLConfirm` / `msgSSLConfirm2` (`JihadBrowserServer.cpp:44,45`) | certificate error | `msgDialogSSLConfirm` | **KEPT MODAL** | Three-way (reject / trust once / trust always) and it gates the load. |
| 8 | `jihadInstallPrompt.js` on `addon-install-failed` — the T-103 "clear reason" (was `_alert()`, now `_notify()`) | an XPI whose `targetApplication` does not match | `msgDialogAlert` | **MOVED** to the non-blocking channel | Nobody can act on it: the install was already cancelled one step earlier, inside `amWebInstallListener.checkAllDownloaded`, and the page has already been told `-210`. An alert with no possible answer was buying nothing and could park the daemon for the dialog deadline. |

**Row 8 is the only informational `msgDialog*` call site that existed.** Everything else in the
daemon is a genuine question. That is the honest answer to "move the informational call sites":
there was one.

## The kit's three named examples, checked against what actually emits

- **"cookies cleared"** — emitted NOTHING before this task. `jihad::ClearCookies` /
  `ClearCache` (`GoannaRenderPage.cpp`) did their work in silence, so a user tapped the button
  and nothing on screen changed. **ADDED**, inside those two functions rather than in either
  caller, so both the frozen YAP commands (`asyncCmdClearCache`/`asyncCmdClearCookies`) and this
  variant's Luna methods pass through the same acknowledgement and neither route can be silent.
- **"add-on installed"** — emitted NOTHING. **ADDED**: `jihadInstallPrompt.js` now also observes
  `addon-install-complete`, whose `installs` are the ones that actually installed
  (`third_party/uxp/toolkit/mozapps/extensions/amWebInstallListener.js:220`). Note the topic is
  `-complete`, not `-completed`.
- **"this page tried to open a popup"** — emits NOTHING, and was deliberately **NOT built**.
  Recorded precisely so nobody re-derives it: the engine DOES raise a trusted `DOMPopupBlocked`
  DOM event at the document (`dom/base/nsGlobalWindow.cpp:8423-8449`, reached via
  `FireAbuseEvents` at `:12521` on exactly the path `SetBlockPopups(true)` arms via
  `dom.disable_open_during_load`). Nothing in this daemon listens for it. Building it is a
  chrome-side listener plus an `isTrusted` filter — small — but it carries one design question
  that cannot be answered off-device: an ad-heavy page blocks popups in bursts, so it needs
  rate-limiting or coalescing or it becomes a toast storm. Not invented for a non-event.

## One dead branch found while reading, not fixed

`BrowserPageGoanna::OnDialog`'s `default:` arm sends `msgDialogUserPassword`
(`BrowserPageGoanna.cpp:1231`), but `DialogKind` has exactly three values —
`{Alert, Confirm, Prompt}` (`DialogService.h:25`) — and every password path above dispatches
`DialogKind::Prompt`. **So the daemon never sends `msgDialogUserPassword` at all.** The message
is part of the frozen contract and all three cards handle it, so nothing is broken and nothing
was changed; it is recorded because "the card's password dialog never appears" is otherwise a
hunt that ends here.
