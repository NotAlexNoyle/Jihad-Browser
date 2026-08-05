# What the GRE gives us that we do not use — XBL widget bindings

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
| `preferences.xml` | `<prefwindow>`/`<prefpane>`/`<preference>` — the machinery cavekit-preferences-ui.md needs. Pale Moon and Basilisk build their preferences window on exactly this; only the PAGE is app-supplied. |
| `findbar.xml` | A complete find-in-page UI, unused, while cavekit-ui-shell.md R4 (findInPage) is open. **Caution before reaching for it:** it drives `nsIWebBrowserFind`/`nsITypeAheadFind`, which is the API whose `FindNext` SIGSEGVs in this offscreen configuration (re-tested 2026-08-04). The binding would give us the UI, not the missing selection controller — but it would also be a ready-made way to exercise that path. |
| `notification.xml` | In-content notification bars. We currently have no way to tell the user "add-on installed" / "cookies cleared" except a blocking card dialog, which is heavier than the message deserves. |
| `scale.xml`, `numberbox.xml`, `spinbuttons.xml` | Slider and numeric controls — the touch-friendly shapes a preferences page wants for font size and zoom. |
| `dialog.xml`, `tabbox.xml`, `groupbox.xml`, `listbox.xml`, `richlistbox.xml`, `tree.xml` | Chrome page building blocks. `richlistbox` and `tree` are already exercised by `about:addons` and `about:config` respectively, so both are known to work in this embedding. |

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
