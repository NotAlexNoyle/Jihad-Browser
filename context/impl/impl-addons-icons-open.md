---
created: "2026-08-02"
last_edited: "2026-08-02"
status: RESOLVED by adc06f5f (the dirty-loop fix). Icons render; they are SLOW to appear.
---

# RESOLVED — `about:addons` icons render (the dirty-loop fix WAS the cause)

**Correction, 2026-08-02 (user, on device): the icons work.** They are slow to appear, but they do
render. So commit adc06f5f — draining the child PuppetWidget chain so post-load repaints happen —
**was** the fix, and my earlier conclusion that it "did not fix the icons" was WRONG.

Why I got it wrong: I captured the daemon's frame ~35 s after launch and saw no icons, and treated
that as proof. But the chrome PNGs load and decode after the load-driven paints, and the repaint
that carries them is subject to the ~150 ms paint rate limit and (since 9cb58c56) the scroll-settle
gate — so on this ~1.2 GHz device they arrive later than my capture window, and later still if
anything is panning. A single timed capture cannot distinguish "never renders" from "renders late",
and I asserted the former. The sub-agent's original claim was right; my re-measurement was the flawed
instrument.

**Remaining, and it is a real defect: the icons are SLOW.** Sync decode is already on
(`PAINT_SYNC_DECODE_IMAGES`), so this is latency in *delivering the repaint*, not in decoding.
Suspects, in order: the ~150 ms paint rate limit in `BrowserPageGoanna`; the 220 ms scroll-settle
gate added in 9cb58c56 (which delays any repaint that follows a pan); and whether an image-completion
invalidation actually reaches the widget promptly or waits for the next unrelated tick.

Everything below is the original investigation, kept because the ruled-out list is still accurate and
saved real time.

## Original notes: everything ruled out, and the live clue

**Symptom.** `about:addons` renders correctly except that the category rows
("Extensions/Themes/Plugins") show text with **no icons**, and the toolbar buttons top-left are
blank rectangles. The icon **box is reserved** — labels are indented ~16 px — so layout has the
frame and its size. This is a PAINT / display-list problem for that frame, not layout and not
loading.

Reproduced in the **daemon's own frame** (`JIHAD_DUMP=1` → `/var/palm/jihad/enyo/frame.ppm`,
768×942), so it is NOT the card→adapter blit.

## Ruled out — with evidence. Do not re-test these.

1. **Files** — `category-extensions.png` (1615 B), `category-plugins.png` (1334 B),
   `category-themes.png` (2094 B) on device, byte-identical to the build output. 157 PNGs under the
   device `chrome/` tree, **zero** zero-length.
2. **Skin registration** — `skin mozapps classic/1.0 toolkit/skin/classic/mozapps/` in
   `chrome/toolkit.manifest`.
3. **CSS** — `#category-extension > .category-icon { list-style-image: url("chrome://mozapps/skin/
   extensions/category-extensions.png"); }` present; every referenced filename exists.
4. **Content image decode/paint** — a `data:image/png;base64,…` `<img>` renders on device
   (framebuffer-verified between a red and a blue CSS block).
5. **chrome:// image load + decode + paint** — navigating straight to the icon URL gives
   `title=[category-extensions.png (PNG Image, 32 × 32 pixels)]` and a framebuffer scan finds
   exactly one 32×32 painted region. It renders.
6. **XUL `list-style-image` / `nsImageBoxFrame` per se** — proven working on device with the real
   `extensions.css`, the real `#category-extension > .category-icon` rule, the real richlistitem
   XBL binding and a real `toolbarbutton`, through the daemon's exact render path.
7. **Sync image decode** — already enabled. `PuppetWidget::JihadRenderDocument`
   (`third_party/uxp/widget/PuppetWidget.cpp:679`) passes `flags = nsIPresShell::RENDER_CARET`
   only, and `nsPresShell.cpp:4574-4575` sets `PAINT_SYNC_DECODE_IMAGES` whenever
   `RENDER_ASYNC_DECODE_IMAGES` is **not** passed.
8. **The inert dirty loop** — real bug, found and FIXED (commit adc06f5f: invalidations landed on a
   child PuppetWidget while the daemon polled the parent). Post-load paints now demonstrably occur
   (two after `load done` on an about:addons load). **This did not fix the icons** — verified on the
   fixed libxul (device md5 == bundle md5).
9. Missing branding chrome — separately fixed; `about:addons` opens and reaches its list view's
   empty state, so `AddonManager` fully initialises.

## The live clue — start here

Inside about:addons, an in-page **`ctx.drawWindow` painted the icon correctly** (ink=741, identical
to desktop) **while the daemon's frame stayed blank**. Same document, same presShell, same computed
style (`icon-lsi=url(...category-extensions.png)`, 32×32 box, all four rules matched via
`inIDOMUtils`).

Both paths end in `PresShell::RenderDocument`. So the difference is in *how* they call it:

- `CanvasRenderingContext2D::DrawWindow` — `dom/canvas/CanvasRenderingContext2D.cpp:~5480`, its
  `renderDocFlags`, the rect/matrix, and which presShell it resolves.
- `PuppetWidget::JihadRenderDocument` — `third_party/uxp/widget/PuppetWidget.cpp:679`:
  `flags = RENDER_CARET`, plus the zoom/pan matrix and clip rect it builds; the daemon resolves the
  docshell via `GetDocShell(mChrome->mBrowser)` (`render/goanna/GoannaRenderPage.cpp:2146-2159`).

Directions not yet eliminated (verify, do not assume):
- a **different/nested presShell**: is the icon content in a subdocument the daemon is not
  rendering, while `drawWindow` on the window catches it?
- `RENDER_IGNORE_VIEWPORT_SCROLLING` / `RENDER_DOCUMENT_RELATIVE` interacting with the zoom matrix;
- display-list construction differences (`PAINT_IGNORE_SUPPRESSION`, paint suppression during load);
- XBL anonymous content reaching the display list for one builder but not the other — note the XBL
  **text** does paint, so any theory must explain why the `<image>` differs from its sibling label.

## Verification bar for any claimed fix

On the **device**, from the daemon's own `JIHAD_DUMP` frame (not the screen, not desktop, not
`drawWindow`): a `frame.ppm` of `about:addons` that visibly contains the category icons, plus the
before/after attributable to the change. A previous fix was reported on weaker evidence and did not
reproduce when measured this way — that is why this bar exists.

Repro: `sh /tmp/rundump.sh` on device (stops the upstart job, runs the daemon with `JIHAD_DUMP=1`),
`palm-launch -d usb -p '{"target":"about:addons"}' net.riverstonerelay.jihad-browser`, wait ~30 s,
then `novacom run file://bin/cat -- /var/palm/jihad/enyo/frame.ppm`.
