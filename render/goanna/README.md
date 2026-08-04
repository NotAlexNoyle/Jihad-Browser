# render/goanna — Goanna (UXP) rendering backend

New code (MPL-2.0) that drives the **UXP / Goanna** engine to satisfy the
BrowserServer `BrowserPage` contract. This is the replacement for the QtWebKit
core. See `PORT-MAP.md` for the per-command mapping and
`../../docs/IPC-CONTRACT.md` for the contract being satisfied.

## Pieces (built — renders on desktop and on the TouchPad)

As built, the backend consolidated into fewer files than first sketched — the
offscreen widget is `MOZ_WIDGET_TOOLKIT=headless` (a real toolkit built into
libxul via patches, so no in-tree `OffscreenWidget` is needed), and the frame
sink + listeners live inside the two render classes:

| File | Role |
|---|---|
| `BrowserPageGoanna.{h,cpp}` | Per-page façade over the YAP contract: routes every command, owns the shared framebuffer + double-buffer flow control, the `pump()` tick, VKB/editor state, and emits the `msg…` messages. |
| `GoannaRenderPage.{h,cpp}` | The engine driver: `nsIWebBrowser`/`nsIWebNavigation` + the `PageChrome` listener (`nsIWebProgressListener`/`nsIWebBrowserChrome`/`nsIBadCertListener2`/`nsIDOMEventListener`), offscreen render → ARGB32 readback, input synthesis via `nsIDOMWindowUtils`, crash-safe form submission, engine-driven focus/blur. |
| `EngineHost.{h,cpp}` | One-time XRE embedding startup, the profile-dir provider (cookie/cache persistence), pref overrides (headless viewport, UA, low-RAM), and installs the dialog + download service overrides. |
| `DialogService.{h,cpp}` | Overrides `@mozilla.org/prompter;1` so alert/confirm/prompt are captured to the sink instead of opening a chrome window (which is absent headless). |
| `DownloadService.{h,cpp}` | Overrides `@mozilla.org/helperapplauncherdialog;1` so a download/MIME-handoff is surfaced to the daemon → app → `com.palm.downloadmanager`. |
| `JihadUserAgent.h` | The Jihad UA string (`Goanna/6.9 UXP/… JihadBrowser/1.0`). |

Branding-strip and the headless toolkit are engine-side patches applied by the
build (`build/desktop/patches/`, `build/webos-oe/mozconfig.goanna-arm`), not files
here.

## The DEBUG surface (`$JIHAD_INJECT`), and its one sharp edge

`GoannaRenderPage.cpp` exposes a small set of `Debug*` helpers the daemon's inject channel maps
to commands. They exist so acceptance criteria can be met without a human touching the screen,
and they are gated on `$JIHAD_INJECT`, which is off unless set.

**`DebugRunChromeJs` (`jsurl`) does NOT execute in a chrome document.** It loads a `javascript:`
URL with the system principal, which works in content. In `about:addons` and friends,
`LoadURIWithOptions` returns NS_OK — so the daemon prints `ok=1` — and no code runs: no effect,
no exception, nothing on the console. Measured 2026-08-04 three ways (`document.title`,
`Components.utils.reportError`, a `gViewController` call). **`ok=1` from `jsurl` means the load
was issued, never that the script ran.**

Drive the chrome UI by clicking its real controls instead. `DebugElementRect` /
`DebugClickElement` (`rect` / `clickid`) resolve an element and click its own centre in viewport
space — independent of zoom and scroll, which raw coordinates are not. Three lookup forms:

| Form | Resolves via | Use for |
|---|---|---|
| `<id>` | `getElementById` | content pages |
| `sel:<css>` | `querySelector` | XUL nodes keyed by attribute — an about:addons row is `richlistitem[value="<addon id>"]` |
| `anon:<css>\|<anonid>` | `querySelector` + `nsIDOMDocumentXBL::GetAnonymousElementByAttribute` | XBL **anonymous** content — a row's `enable-btn`/`disable-btn`/`remove-btn`, which no id can reach |

Clicking the real button is also the better test: it goes through the same command path a finger
does.

## Dialogs block, on a deadline sized for a person

`BrowserPageGoanna` is the `DialogSink`: it creates a FIFO, emits `msgDialog*(syncPipePath, …)`,
and blocks until the card answers. **The card opens the reply pipe only at the moment the user
taps** (`BrowserAdapter::js_sendDialogResponse`), so there is NO signal distinguishing "the dialog
is on screen" from "nobody is there" — do not add a liveness heuristic based on the pipe. A
5-second version of exactly that idea silently defaulted every dialog a human answered in normal
time, while every harness passed. One 60 s deadline now; `JIHAD_DIALOG_MS` overrides it for
tests. The wait is timed in the log (card-picked-up, answered) because the engine side is ~2 ms
from click to dialog emitted, so any visible lag belongs to the front-end.

## Engine dependency

UXP/Goanna is **built out-of-tree** from the upstream UXP source
(`third_party/uxp` (submodule), pinned rev in `../../docs/ENGINE-SOURCE.md`) and linked; it is
**not vendored** in this repo. The build wiring lives in `../../build/`. libxul is
built with `MOZ_WIDGET_TOOLKIT=headless` (a new in-tree toolkit added by the Jihad
patches) so it links **no** GTK/X — only freetype + fontconfig — and renders
offscreen with no display server. The ARM cross-build (`build/webos-oe/`) produces
a ~29 MB stripped libxul that loads and renders on the TouchPad.

## Why this is the load-bearing work

Goanna has no first-class embedding product (unlike WebKit's QtWebKit port).
The embedding API (`nsIWebBrowser`) and the puppet/offscreen widget machinery
exist in-tree, but wiring them into a headless render-to-shmem server was the
novel, highest-risk part of the port. It was sequenced first on desktop x86_64
(Phase 1) to de-risk it before the ARM toolchain fight (Phase 2) — both are now
done: the engine renders real pages into the shared framebuffer on the TouchPad.
The current work here is interaction/render fidelity (input, VKB, repaint,
lifecycle), not standing the pipeline up.
