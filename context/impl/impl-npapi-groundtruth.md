---
created: "2026-08-02"
last_edited: "2026-08-15"
status: R7 in progress — NPAPI subsystem confirmed live on device; search path is the gap.
        2026-08-15 appendix: keyboard-arbitration observable + procedure (T-107)
---

# R7 ground truth: NPAPI is compiled in and running; only the search path is missing

Measured 2026-08-02 on the TouchPad, before writing any plugin code.

## NPAPI IS in our ARM engine

`--disable-npapi-gtk2` in `build/webos-oe/mozconfig.goanna-arm:20` disables GTK2 **windowed**
(XEmbed) plugins only; `MOZ_ENABLE_NPAPI` is a separate switch, and it is ON. Evidence from the
shipped ARM `libxul.so` (`dist/bin/libxul.so`) — all four NPAPI entry-point symbols and the plugin
prefs are present:

```
NP_Initialize  NP_Shutdown  NP_GetMIMEDescription  NP_GetValue
plugin.disable  plugins.load_appdir_plugins
```

(75 NPAPI-related strings total. `nm -D` shows nothing because the ARM libxul is stripped — use
`strings`, not `nm`, on this binary.)

This confirms the kit's stated ground truth rather than assuming it, and means **nothing needs
enabling in the build**: windowless NPAPI is already available.

## `about:plugins` works

```
[jihad-bs] load done uri=about:plugins
[jihad-bs] titleAndUrl title=[About Plugins] uri=about:plugins
```

The page renders (fb1 capture) and reads **"No installed plugins found"**. That is the correct
result for an empty search path, and it proves `nsPluginHost` initialised and ran a scan — the
subsystem is live, not stubbed out.

So R7's remaining work is **not** "enable NPAPI" and **not** "make about:plugins work". It is:

1. a variant-scoped plugin search path that honours the R8 storage contract (under `$APP`, never
   `/media/internal`, never `/var`),
2. windowless instantiation compositing into our offscreen surface,
3. input routed through the R5 coordinate transform,
4. the `libflashplayer.so` attempt, whose outcome is to be recorded either way.

## Note for whoever does the Flash attempt

The device's own Flash (`/usr/palm/ipkgs/com.palm.app.flashplugin-001_1.0.6_all.ipk` →
`libflashplayer.so`, 8.8 MB, ARM EABI5) links against webOS's WebKit host and compositor —
`libWebKitLuna.so`, `libPiranha.so`, `libLunaSysMgrIpc.so`, `libnapp.so`, `libhal.so`,
`libpowerd.so.0`, `liblunaservice.so`, plus OpenSSL 0.9.8. Whether it functions inside Goanna
rather than LunaSysMgr's WebKit is a genuine open question. The kit requires the outcome recorded
either way, naming exactly which dependency is unsatisfiable if it fails — a documented negative
satisfies the criterion; an untested claim does not.

Silverlight is **not achievable** on this device at all: it shipped x86/x64 for Windows/macOS only,
and the Linux implementation (Moonlight) was x86-only and is long dead. The requirement there is
that our NPAPI *architecture* stays generic, verifiable on the desktop x86_64 build.

---

# 2026-08-15 (T-107): keyboard-arbitration ground truth, read out of the adapter source

No device this session (novacom down). This is what the source says about the arbitration path,
established so the device run — whenever a key can be delivered — is not spent rediscovering it.
The procedure that follows from it lives in `context/kits/cavekit-addons-extensions.md`, R7's
keyboard criterion, annotation dated 2026-08-15. What changed: the observable was `window.pageYOffset`
and is now a screenshot of the card's URL FIELD.

## The gate, and what is reachable from where

`BrowserAdapter::handleKeyDown` / `handleKeyUp` (`render/adapter/BrowserAdapter.cpp:1610-1622`):

```
asyncCmdKeyDown(event->rawkeyCode, event->rawModifier, event->chr);          // :1613 — ALWAYS
return event->rawkeyCode != ESC_KEY && mPageFocused && (bEditorFocused || mFlashGestureLock);
```

The forward to the daemon is unconditional and sits ABOVE the return, so the engine and any plugin
see the key whichever way the gate goes. Only the CARD's behaviour differs. Three things follow:

1. **No page-side or plugin-side observable can measure arbitration.** The SWF turning red, a DOM
   `keydown` counter, `#kc` — all of them fire identically in the pass and the leak case. The only
   thing that differs is what the chrome does, and the chrome is the Enyo card, not our content.
2. **The daemon's inject channel can never test this.** `render/browserserver/JihadBrowserServer.cpp`
   dispatches `key`, `click`, `clickid`, `clickoff`, `dblclickid` and `touch` by calling `BrowserPage`
   and `jihad::Debug*` directly in the DAEMON process. `handleKeyDown` runs in LunaSysMgr and is
   never entered. Recorded again here because it is the shortcut that looks like it should work.
3. **Escape is exempt unconditionally** (`:1614`), so it always falls through to the card and always
   looks like a leak. Never test with it.

## `mFlashGestureLock`: a DOUBLE-tap, not a tap

Sets true — exactly two sites:

| site | path | reachable by |
|---|---|---|
| `:1583` | `handlePenDoubleClick`, double-tap inside a plugin rect | a real `NpPalmPenEvent` from LunaSysMgr — a human finger |
| `:3119` | `js_smartZoom` | card JS `adapter.smartZoom(x, y)` — **no in-tree caller** |

Clears — `:1432` (`handlePenDown`, tap outside a plugin rect), `:1576` (double-tap outside while
locked), `:1868` (`doGestureStart`), `:1993` (`sendGestureStart`), `:3113` (`js_smartZoom` outside).

`handlePenDown` only ever CLEARS. So a single tap inside the SWF does not engage the plugin, and
the wording carried in both kits — "set when a tap lands inside a plugin rect" — is wrong about the
gesture. The behaviour it describes is right; the arming input is a double-tap.

Grep for the scriptable route: `smartZoom` appears nowhere in `app/`, `app-mochi/` or `app-mojo/`
(the only hits in the tree are `third_party/enyo-layout/imageview/source/PanZoomView.js`, a
different method on a different kind). So today step 5 of the procedure needs a human. A three-line
card-JS shim calling `adapter.smartZoom` would make it dev-loop scriptable — a card change, left as
a flagged option rather than done here.

## The latch has no log, but it has a behaviour

`render/adapter/Debug.h:31` leaves `DEBUG` commented out, so `TRACEF` is `(void)0` in the shipped
adapter and `"Updating flashGestureLock status to : %d"` (`:5590`) never reaches syslog. The one
piece of this state machine that DOES log is `handleFocus`'s `g_message` (`:1360`), which prints
`mPageFocused` — useful, and the reason the procedure names it for diagnosing an unexpected leak.

The latch is instead confirmed behaviourally: with it set, a drag starting inside the plugin rect
stops panning the card, because `handlePenMove`'s `passToFlash` requires it (`:1507`). That is what
`#sy` is for now.

## Two traps the procedure exists to defeat

- **The latch-setting double-tap scrolls the page.** `handlePenDoubleClick` falls through to
  `prvSmartZoom` (`:1586`) after setting the latch, so `pageYOffset` moves with no keystroke
  involved. That is a false POSITIVE for the old observable, on top of the false negative already
  recorded (a touch OS need not scroll on arrow keys at all). Hence: baseline screenshot AFTER the
  double-tap, never before.
- **The editor confound.** The gate is `bEditorFocused || mFlashGestureLock`. `jihad-keyarb.html`
  contains an `<input>` (deliberately — it is the control for the OTHER half of R7), and if it
  holds focus the keys are swallowed for the wrong reason and the run silently proves nothing.
  `#fo` now reports `document.activeElement` so this is measured rather than assumed.

## Test page changes (`build/webos-oe/stage-test-pages.sh`)

`jihad-keyarb.html` gained `#id` (magenta `JIHAD-KEYARB` banner — the wrong-card guard for
`takeScreenShot`, which captures the FOREGROUND card and after a double-launch is often the wrong
one), `#kc` (count/last keyCode from a capturing window listener — a per-run delivery counter that a
reload resets, unlike the SWF's one-shot green→red) and `#fo` (the confound above). `#sy` is kept
and repurposed. All four are `gettext`-readable. Generated HTML verified locally: balanced tags, no
duplicate ids, JS parses; `bash -n` clean.
