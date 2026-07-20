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

## Engine dependency

UXP/Goanna is **built out-of-tree** from the upstream UXP source
(`../../../UXP`, pinned rev in `../../docs/ENGINE-SOURCE.md`) and linked; it is
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
