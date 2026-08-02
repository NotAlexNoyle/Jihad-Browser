---
created: "2026-08-01"
last_edited: "2026-08-01"
status: RESOLVED — pages render on device
---

# Card → adapter → Goanna renders real pages on the TouchPad

**2026-08-01: confirmed end-to-end on device.** `http://example.com` loads and PAINTS —
full page content, correct fonts and link styling, inside the isis chrome with the URL bar
showing the loaded URL. Control path *and* paint path both work.

## Read this before debugging "the page is blank"

**Screenshots must come from `/dev/fb1`, not `/dev/fb0`.**

- `/dev/fb1` = the APP layer. The plugin's PGContext composite lands here. This is the page.
- `/dev/fb0` = the SYSTEM layer: status bar, the isis chrome, and system alerts.

Capturing fb0 shows the chrome with a flat `(216,216,216)` content area and *no page*, which
reads exactly like "the blit is broken". It is not — the page is on the other layer. This cost
a full round of false diagnosis: the adapter log already said `blit=1` with a valid rect while
the fb0 capture said blank, and the log was right. Trust the adapter log over a framebuffer
capture, and capture fb1.

Geometry for both: 1024×2304 virtual (triple-buffered 1024×768), 32bpp BGRA, stride 4096.
Read one frame (`bs=4096 count=768`), interpret as 1024×768 BGRA, then `rotate(-90, expand)`
for the portrait UI. Wake the display first (`palm://com.palm.display/control/setState
{"state":"on"}`) — a dimmed screen composites nothing and every buffer reads black.

## The USB "Connected / USB Drive / Close" dialog

Raised by `SystemService::msmAvail` in LunaSysMgr from storaged's `/storaged/MSMAvail`, and it
stays up for as long as USB is connected with the media volume exportable. It **cannot be
dismissed programmatically** with what the platform exposes:

- `com.palm.storage/diskmode` has only `enterMSM` / `inMSM` / `queryMSMStatus`; `enterMSM` would
  enter USB-drive mode and unmount `/media/internal`, which is the opposite of what is wanted.
- LunaSysMgr exposes `dismissModalApp` only — for modal *apps*, not system alerts.
- `killall LunaSysMgr` does NOT clear it: LunaSysMgr re-subscribes and re-raises it immediately
  (measured — identical before/after).
- Input injection is unavailable: the touchscreen is not an evdev device
  (`/proc/bus/input/devices` lists only gpio-keys, pmic8058_pwrkey, headset), so `uinput` cannot
  reach it, and `PalmSystem.simulateMouseClick` is scoped to an app's own window.

It needs a physical tap on "Close", or unplugging USB (which kills novacom). **It does not
obstruct verification**, because it composites on fb0 while the app is on fb1.

## Original finding: paint path

First on-device page load through the new stack, 2026-08-01:

```
Browser.pageTitleChanged():  http://example.com/ Example Domain false false
enyo.BasicWebView.loadStopped():
```

`Example Domain` is the real `<title>` of the fetched page. It travelled
**card → NPAPI adapter → YAP IPC → BrowserServer/Goanna → network → parse → back to the UI.**
The whole control path is live.

A framebuffer capture confirms the card renders: the isis chrome, back/forward, the URL bar
showing `http://example.com`, and the "Jihad Browser" card title all paint correctly.
**The page content area is blank.**

## `NPP_New` was never "broken" — the element was hidden and 0×0

The long-standing "NPP_New is never reached" symptom had no bug behind it. From the in-card
probe, at startup:

```
rendered node: id=browserApp_browser_view_view tag=OBJECT attr(type)=application/x-jihad-browser
rendered node: inDoc=true offset=0x0 client=0x0 rect=0x0
rendered ancestors: depth=6 hidden=DIV#browserApp_browser.basic-back[none/visible]
rendered npobject: openURL=undefined connectBrowserServer=undefined setPageIdentifier=undefined
```

An ancestor (`DIV#browserApp_browser.basic-back`) is `display:none` and the element is 0×0, so
WebKit does not instantiate the plugin. That is correct, normal behaviour: the isis UI keeps
the browser pane hidden until the user navigates. Launch with a target URL and it flips:

```
+0.5s node: inDoc=true offset=768x942 client=768x942 rect=768x942
+0.5s ancestors: depth=6 hidden=NONE
+0.5s npobject: openURL=function connectBrowserServer=function setPageIdentifier=function
```

**Do not "fix" the hidden-at-startup state.** It is the stock UI's design, and the stock
adapter behaves identically.

## Everything the probe confirmed working

- MIME registered and visible to content:
  `application/x-jihad-browser -> PRESENT plugin=Jihad Browser Adapter (enyo) file=BrowserAdapterJihad.so`
- Prototype patch reaches the kind actually instantiated:
  `WebView.chrome[0].kind===enyo.BasicWebView -> true`
- The app's real WebView gets our type: `attr(type)=application/x-jihad-browser`
- Control experiment — a manually created, correctly sized element instantiates for **both**
  adapters, so instantiation itself is sound and this is not Jihad-specific:
  ```
  control jihad: attr=application/x-jihad-browser openURL=function offset=64x64
  control stock: attr=application/x-palm-browser  openURL=function offset=64x64
  ```

## Paint path — verified working

The engine renders correctly (proven independently of the adapter by `JIHAD_DUMP=1`, which
writes the exact rendered frame to `<state>/frame.ppm`), and the adapter composites it:

```
[hp-entry] dstH=942 mOff=0x35fad368 dstBuf=(nil) rowBytes=0 gc=0x30d6fe40
[hp-pg] blit=1 src[0,0 768,942] dst[0,54 768,996] inv=1.000 rW=768 rH=942 rX=0 rY=0
```

`dstBuf=(nil)` is CORRECT on device and is not a failure: webOS selects the PGContext path
(`useGraphicsContext=true`), where WebKit hands the plugin a Piranha `PGContext*` and no raw
buffer. Only the desktop/Ubuntu fallback uses `dstBuffer`. The `gc=` field was added to
`[hp-entry]` precisely so "took the PGContext path" and "bailed at the dstBuffer guard" stop
looking identical in the log.

### Debug switches (both OFF by default; both were left off after this session)

- **Adapter paint log** — create the gate file `<state>/adapterlog`, then restart LunaSysMgr
  (the gate is resolved once per adapter process). Writes `<state>/adapter.log`.
- **Engine frame dump** — `JIHAD_DUMP=1` in the daemon's environment writes `<state>/frame.ppm`
  (768×942 P6). Costs ~2.2 MB of `fputc` per paint; never leave it on.
  The rootfs is read-only, so rather than editing `/etc/event.d/jihad`, `stop jihad` and
  hand-start the daemon with the same env plus `JIHAD_DUMP=1`.

### One trap in the daemon log

`painted shmid=… bytes=N` — `N` is **not** a byte count. `ReadPixels` returns the number of
NON-WHITE pixels (`!(r>240 && g>240 && b>240)`). example.com's background is `(238,238,238)`,
which is under that threshold, so a correctly rendered page reports `bytes=723456` = every one
of 768×942 pixels. That looks like a stride/format bug and is not one.

## Reproducing

```bash
palm-launch -d usb -p '{"target":"http://example.com"}' net.riverstonerelay.jihad-browser
# then capture /dev/fb1 (see above) — NOT fb0
```
