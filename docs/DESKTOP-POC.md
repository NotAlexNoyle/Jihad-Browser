# Jihad Browser — Desktop PoC (Phase 1)

*Goanna inside, webOS alive, inshallah.*

This is the single documented entry point for the **x86_64 Linux desktop proof of
concept**: it builds the real render daemon (`jihad-browserserver`) with the
Goanna/UXP backend, and drives it with a minimal BrowserAdapter stand-in over the
**unchanged YAP IPC contract** — no webOS stack required.

Which acceptance criteria this satisfies, and their status, are in
`context/kits/cavekit-desktop-build.md`. This file is the procedure only.

![Desktop PoC render](jihad-poc-render.png)

*The full pipe: the daemon loads the page in Goanna, paints into shared memory, and
the adapter stand-in writes it to an image — `data:` page shown; `https://example.com`
over TLS is in `jihad-render-example-com.png`.*

## Prerequisites

- `podman` (or Docker) — the pinned build container is Ubuntu 20.04 (GCC 9.3,
  Python 3.8, autoconf2.13, yasm, full X11 dev set + Xvfb). See `build/desktop/Dockerfile`.
- The UXP source submodule at `third_party/uxp` (mounted read-write; autoconf regenerates configure).
- One-time: build the image — `podman build -t jihad-goanna-build build/desktop`.

## 1. Build the engine (once, ~30–60 min)

```bash
podman run --rm --user 0 -e HOME=/out \
  -v "$PWD/third_party/uxp:/src/uxp" -v "$PWD:/jihad" \
  -v "$PWD/build/desktop/out:/out" \
  jihad-goanna-build bash /jihad/build/desktop/build-goanna.sh
```

Produces `build/desktop/out/obj-jihad-goanna/dist/bin/libxul.so` plus the frozen
embedding headers. Engine patches under `build/desktop/patches/` are applied
automatically (0002 format-overflow, 0003 OMTC-off, 0004 gfx init).

## 2. Build + run the daemon and adapter round-trip (R1 + R2 + R3)

```bash
podman run --rm --user 0 -e HOME=/out \
  -v "$PWD/third_party/uxp:/src/uxp" -v "$PWD:/jihad" \
  -v "$PWD/build/desktop/out:/out" \
  jihad-goanna-build bash /jihad/build/desktop/build-adapter-roundtrip.sh
```

This single entry point:

- **R1** — compiles + links `jihad-browserserver` from the engine-agnostic IPC
  layer (`Yap/`, `BrowserServerBase`) + the Goanna backend (`JihadBrowserServer`,
  `BrowserPageGoanna`, `GoannaRenderPage`, `EngineHost`). **LunaService is compiled
  out for desktop** — `Main.cpp` runs a plain GLib main loop; nothing links
  `libLunaService`. The daemon starts and prints `DAEMON_UP`.
- **R2** — builds `jihad-adapter`, a `YapClient` BrowserAdapter stand-in. It
  allocates the two shared buffers, sends `Connect (0x1000)` + `OpenUrl (0x1004)`,
  receives `msgPainted`, **writes the buffer to `out/jihad-poc-render.ppm`**, and
  **returns the buffer** (`ReturnBuffer 0x150d`) so rendering can continue.
- **R3** — a real page renders correctly through the whole pipe; the load-lifecycle
  messages arrive in order (`msgLoadStarted → msgLoadProgress → msgLoadStopped →
  msgLocationChanged → msgPainted`). Override the page with `JIHAD_URL=…` (e.g.
  `https://example.com`). Prints `ROUND-TRIP PASS`.

`JIHAD_POC_IMAGE=/out/foo.ppm` changes the output path.

## 3. Focused capability tests

Each `build/desktop/build-*-test.sh` builds one aspect against the engine and
asserts real behavior under Xvfb (all PASS):

| Script | Proves |
|--------|--------|
| `build-embed-load.sh` | page load reaches `STATE_STOP` |
| `build-page-driver.sh` | `BrowserPageGoanna` bridge drives paint |
| `build-input-test.sh` | click/key/mouse fire DOM handlers (INPUT PASS) |
| `build-nav-test.sh` | back/forward + `canGo*` + setHtml (NAV PASS) |
| `build-history-test.sh` | clearHistory + history-state (HISTORY PASS) |
| `build-fail-test.sh` | failed-load + global-history events |
| `build-services-test.sh` | JS toggle blocks onclick (SERVICES PASS) |
| `build-settings2-test.sh` | min-font / block-popups / accept-cookies |
| `build-dialog-test.sh` | alert/confirm/prompt interception (DIALOG PASS) |
| `build-download-test.sh` | download / MIME handoff (DOWNLOAD PASS) |
| `build-resize-test.sh` / `build-scroll-test.sh` / `build-zoom-test.sh` | surface geometry |
| `build-geo-test.sh` | contents-size / meta-viewport / scrolled-to events (GEO PASS) |
| `build-download2-test.sh` | daemon-side download lifecycle over YAP: `msgDownloadStart`/`Progress`/`Finished` (temp path + MIME) and `cancelDownload` aborting an in-progress download (DOWNLOAD-LIFECYCLE PASS) |
| `build-cookie-test.sh` | persistent cookie survives an engine restart + `cookies.sqlite` exists in the profile (COOKIE-PERSISTENCE PASS) |
| `build-prefsui-test.sh` | about:preferences notificationbox attach/render/dismiss (DOM-gone, both widget paths) + the eleven shared-file pref rows (PREFSUI PASS; negative modes via `JIHAD_PREFSUI_NEG`) |
| `build-xpi-mismatch-test.sh` | mismatched-targetApplication XPI: refusal alert names the add-on, matching-declined control shows zero alerts, page status -210 both ways (XPI-MISMATCH PASS) |

## R4 — isis Enyo UI against the desktop daemon `[human-review]`

The Enyo UI (`app/`) targets the **webOS Enyo runtime + LunaService/sysmgr**, which do not exist on
a stock Linux desktop, so there is nothing on desktop to drive the real UI with. The harness above
is what stands in. Driving the real UI happens on the device (`docs/DEVICE-BUILD.md`); how that
substitution is accounted for against the requirement is `context/kits/cavekit-desktop-build.md` R4.
