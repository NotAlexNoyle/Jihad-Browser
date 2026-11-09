# The reference repositories: last read, and what moved was weighed for Jihad

The checkouts beside this repository (`../Atlas/`, `../webos-docs`, `../ref-forks/`) are read, never built or
changed. `tools/host/update-references.sh` brings them to their upstreams' heads by fast-forward and prints what
arrived (cavekit-atlas-references.md R1, R2); this file records the head each was last read at, so the next run shows
only the commits since, and says per item whether Jihad takes it (R3; the owner's standing rule of 2026-11-09: every
dependency pulled is read for things Jihad can use). An "adopt" becomes a task row on the kit it touches; an "owner's
call" is a question in the dialog batch.

## Last-read heads (2026-11-09)

| checkout | origin (default branch) | head | date | note |
|---|---|---|---|---|
| `Atlas/atlas-browser-app` | `webOS-ports/org.webosports.app.atlas` (master) | `e665566` | 2026-11-08 | tag `0.9.6`; the origin moved from Herrie82's redirected name on 2026-11-09 |
| `Atlas/BrowserAdapter` | `Herrie82/BrowserAdapter` (master) | `21c493d` | 2026-07-20 | `ancestor` = `woce/BrowserAdapter` at `84a330a`, the merge base: 22 commits of Atlas's own on top |
| `Atlas/BrowserServer` | `Herrie82/BrowserServer` (master) | `7c5e2c4` | 2026-07-08 | `ancestor` = `isis-project/BrowserServer` at `e2506f5`, the merge base: 9 of Atlas's own on top |
| `Atlas/atlas-wpe-backend` | `Herrie82/atlas-wpe-backend` (master) | `5904d54` | 2026-08-24 | — |
| `webos-docs` | `webosarchive/webos-docs` (main) | `6846ec6` | 2026-11-05 | — |
| `ref-forks/lunace-webos-ports` | `webOS-ports/luna-sysmgr` (webOS-ports/master) | `5112fcf` | 2026-09-05 | — |

The `pre3-lowmem` branches (the app, the adapter, the backend): a Pre 3 low-memory mode, 512 MB; the TouchPad has
1 GB and Jihad its own memory guard (cavekit-device-headroom.md) — not applicable.

## 2026-11-09 — the Atlas app, `e6cc687..e665566` (24 commits, 14 of them changes; the kit's reading of 2026-10-26 was at `e6cc687`)

| commits | what Atlas did | Jihad today | verdict |
|---|---|---|---|
| `6aa556d` (PR #13) | Video on the Chromium host: the screen stays on while the active tab's video plays and the card is in front; a fullscreen video covers the whole screen; a screen lock (Netflix-style) turns taps off | Nothing holds the display: the timeout dims and blanks the screen under a film with the sound going on. The daemon already knows when media plays (the audio-route poll, `BrowserPageGoanna.cpp` `jihadStartAudioPoll`) and has a door to the card (the notification subscription the `print-*` categories ride) | **adopt** the screen-on hold: the daemon tells the card when a page's media starts and stops, the card sets the window property `blockScreenTimeout` while it is in front (webOS 3, no privilege) — T-599. The screen lock: **adopt** too (the owner, 2026-11-09) — T-602 |
| `e148635` (PR #16) | Atlas's own page for a failed load, in place of the page view, with Try Again and the settings app for the cause | isis's modal dialog ("Unable to Load Page", "Unable to resolve host.") over whatever is on screen (`Browser.js` `browserError`); the daemon knows the failed URL and has a retry arm (`GetLoadError`) | **owner's call**: an in-card error panel with Retry, in each shell's look, in place of the dialog |
| `b929448` (PR #10) | A media session registered with LuneOS's `com.webos.service.mediacontroller` (`mediacontroller.query`) | No such service on webOS 3; its counterpart for the same end is `palm://com.palm.keys/media` (Bluetooth headset play/pause keys) | **adopt** (the owner, 2026-11-09) — T-601, BT media keys driving the page's media |
| `1f7981b`, `59bc79d`, `2df7255` (PR #19) | Print on the Chromium engine; the job named after the page; A4 from the system locale | Jihad prints over IPP itself: the job's `job-name` is the page title (`components/jihadPrint.js:502`) and the paper is the printer's `media-default` | n/a (already so) |
| `864af12` | Enyo loaded once (index.html loaded 0.10 and 1.0, every tap arrived twice) | `app/index.html` loads `enyo/0.10` once (checked) | n/a |
| `a49a8bd` (PR #12) | Search suggestions off until asked for; the suggest URL over HTTPS | The three shells send typed text nowhere (no suggestion service in `app/`, `app-mochi/`, `app-mojo/`) | n/a |
| `c683f68` (PR #11) | The updater sends a per-install UUID, not the device id, over HTTPS | Jihad has no self-updater (Preware) | n/a |
| `12d4af6` (PR #18) | Help links to help.luneos.org | No Help item in Jihad's menus | n/a |
| `3630d3f`, `7ba944c`, `e71fea8` (PRs #14, #15, #17) | Phone layout: address editing left on blur, bookmarks/history from the app menu, the lock halfway down the edge in landscape | Jihad is TouchPad-only | n/a |
| `ff3a2fa`; `ChromiumWebView.shouldPaint`; `sysbus/*.perm.json` | The app menu over a native page view; one rule for the native view's painting; LuneOS bus permissions | A native-view host's concerns; Jihad's engine paints into the card's plugin | n/a |

Earlier heads, weighed with this first record (they predate the kit): BrowserServer `7c5e2c4` adds `SetAutoplayWithSound`
(0x1131), a per-card command behind Atlas's "Autoplay media with sound" preference — Jihad has `jihad.youtube.stopAutoplay`
(ON) and no general autoplay toggle: **adopt** (the owner, 2026-11-09) — T-603, a Settings row. BrowserAdapter `21c493d` makes the pan buffer's height configurable
(a tall-tile buffer to come) — Jihad's GPU path has its own displayport and bands (cavekit-gpu-rendering.md): n/a.
`webos-docs` `4acbdb9..6846ec6` (an icon, the Preware link): n/a. `luna-sysmgr`: tags only (LuneOS releases): n/a.
