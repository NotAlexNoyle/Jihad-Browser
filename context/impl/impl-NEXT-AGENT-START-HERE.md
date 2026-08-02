---
created: "2026-08-02"
last_edited: "2026-08-02"
status: HANDOFF — read this first, then context/kits/cavekit-overview.md
---

# START HERE — handoff to the next cavekit agent

The browser **works** on the HP TouchPad: UXP/Goanna renders real pages end to end through the
frozen YAP contract into the unmodified isis UI. `about:addons` and `about:plugins` open. What
follows is what is broken, in the order the user chose, plus the traps that cost this session time.

## Device facts you must know before touching anything

- **Screenshots come from `/dev/fb1`** (app layer), NOT `fb0` (status bar / chrome / system alerts).
  An fb0 capture shows chrome over a blank content area and reads as "rendering is broken" when it
  is fine. Wake the display first (`palm://com.palm.display/control/setState {"state":"on"}`) or
  every buffer reads black. 1024x2304 virtual, 32bpp BGRA, stride 4096: read `bs=4096 count=768`,
  interpret 1024x768 BGRA, `rotate(-90, expand)`.
- **App JS dev loop:** `novacom put` → `killall LunaSysMgr` → `palm-launch`. WebAppMgr caches app
  sources in-process, so relaunching alone runs the OLD script. **Never** use `palm-install` to
  iterate (hangs on a 44 MB ipk; two concurrent runs race the same app dir).
- **After every `novacom put`, md5-compare both sides.** A push died mid-transfer this session and
  left a **zero-byte daemon** on device; the exit status said success.
- **Only the Enyo variant is live.** Mochi worked before and is a deploy regression; Mojo has never
  run. The USB "Connected" dialog cannot be dismissed programmatically — ignore it, it is on fb0.
- The user can perform **physical taps/gestures on request** — this is the only way to exercise the
  adapter's pen path (the touchscreen is not an evdev device, so nothing can be injected).

## Work queue — the user chose this order

### 1. Scroll pan headroom (P1, most user-visible) — `impl-scroll-glitch-open.md`
Scrolling shows undrawn **grey** strips. `paintToSharedBuffer` paints exactly `w x h` = the WINDOW
size (768x942) and stamps `renderedWidth/Height` to match, so the buffer the adapter pans inside has
**zero headroom** — one pixel of pan leaves the painted area. The shm segment is ~4x a viewport
(`connect … sz=12582944` ≈ 3.15 M px vs 723 k), and the isis/Atlas adapter expects a TALL buffer
panned within (see the "tall-buffer pan" lineage in `BrowserAdapter::handlePaint`).
**Fix:** paint viewport+overscan bounded by `segSize`, report the real painted size, then
re-evaluate the 220 ms scroll-settle gate added in 9cb58c56 — it may become unnecessary.

### 2. XUL `<menupopup>` support (P1 — CONFIRMED by the user 2026-08-02)
**Tapping the `about:addons` settings button does nothing** — the tools menu never opens
(`extensions.xul:137`, `<toolbarbutton type="menu">` → `<menupopup id="utils-menu">`). This is
structural, not cosmetic, and the same popup path backs **`<select>` dropdowns, context menus and
every XUL menu** — very likely why long-press `contextmenu` never reaches the page either.
PuppetWidget *does* model popup children (`PuppetWidget.cpp:63` tests `eWindowType_popup`, `:347`
asserts it), so popups are not obviously unsupported. **Instrument popup widget creation FIRST** —
today it logs nothing, so "no popup created" and "popup created but painted nowhere" are
indistinguishable. That ambiguity is exactly what cost this session time on `holdAt` and the icons.

### 3. Chrome icon repaint latency — `impl-addons-icons-open.md`
Icons render but are **slow**. Sync decode is already on, so this is repaint DELIVERY latency.
Suspects: the ~150 ms paint rate limit, the 220 ms scroll-settle gate, and whether an
image-completion invalidation reaches the widget promptly.

### 4. Start-page centring under the VKB (user request, not yet implemented)
`app/css/browser.css` hardcodes `.startpage { height: 1024px }` and
`.startpage-placeholder-tall { height: 1024px }`, with the brand block absolutely centred in that
FIXED box — so it cannot follow the keyboard and is also wrong in landscape (centres at ~512 in a
768-tall viewport). webOS already shrinks the card when the VKB rises (768x602 portrait), so making
the container follow available height (`height:100%`, `flex:1` on the `tall` box, `position:relative`)
re-centres for keyboard up/down AND both orientations with no resize handler. Unverified — the fixed
1024 px was likely chosen because a parent lacked a height, so check the parent chain on device.

### 5. Mochi re-deploy + Mojo first run — `impl-variant-deploy-state.md`
**User decision: push the files over novacom autonomously** (slow, hours per variant, but no user
action needed). Then run the real R7 test: three daemons on their own sockets, each card reaching
only its own variant, removing one leaving the others untouched
(`build/webos-oe/device-independence-test.sh`, so far only ever run against one live variant).

### 6. R3 XPI install — `impl-r8-palemoon-basilisk.md`
Needs a daemon `amIWebInstallPrompt`; the toolkit's prompt is a modal XUL chrome window headless
cannot open, and its failure path CANCELS every install. **Follow Atlas: prompts are card-side
dialogs**, and **each variant uses its OWN framework's idiom** (Enyo 1 popup for enyo, Mochi/Enyo 2
kinds for mochi, Mojo `showAlertDialog` for mojo). The daemon stays framework-agnostic.

### 7. R7 NPAPI plugins
Configuration is done. The blocker is that **windowless NPAPI does not exist in a cairo-headless
build** (`NPNVSupportsWindowless` answers false unless Win/Mac/X11-GTK; the Unix windowless model is
X11-defined) — it must be PORTED, not enabled. Neither reference browser faced this.

## Verification standards this project learned the hard way

1. **Believe the artifact, not the exit status.** `build-goanna-arm.sh` reported success after
   `mach configure` failed; a `.ipk` install "succeeded" while the daemon was zero bytes; a bundler
   run that was never executable produced a one-line log that looked like a completed build.
2. **One timed capture cannot distinguish "never" from "late".** I concluded the icons were not
   fixed from a single frame grab; they were rendering, just after my window. That claim was wrong
   and contradicted a correct sub-agent finding.
3. **Instrument the boundary before theorising.** `mouseEvent`, `holdAt` and popup creation all log
   nothing, so absence of evidence kept getting read as evidence of absence.
4. **Restoring correct behaviour exposes bugs that were hidden.** The card→adapter "bug" was a pane
   legitimately hidden until navigation; the scroll glitching only became reachable once repaints
   started happening at all. Expect more of this.

## Uncommitted in the working tree, on purpose

`render/browserserver/JihadBrowserServer.cpp`, `render/goanna/GoannaRenderPage.{cpp,h}` — a
`$JIHAD_INJECT`-gated debug channel (`jsurl` runs privileged JS in a chrome document, `title` reads
it back). It **does compile** (the ARM daemon built with it) and is off by default. It is the right
tool for the popup investigation in item 2.
