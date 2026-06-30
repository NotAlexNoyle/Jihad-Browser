# render/goanna — Goanna (UXP) rendering backend

New code (MPL-2.0) that drives the **UXP / Goanna** engine to satisfy the
BrowserServer `BrowserPage` contract. This is the replacement for the QtWebKit
core. See `PORT-MAP.md` for the per-command mapping and
`../../docs/IPC-CONTRACT.md` for the contract being satisfied.

## Pieces (to be built in Phase 1)

| File | Role |
|---|---|
| `BrowserPageGoanna.{h,cpp}` | Implements the `BrowserPage` public surface using `nsIWebBrowser` + `nsIWebNavigation`. Entry point for every YAP command. |
| `OffscreenWidget.{h,cpp}` | Custom `nsIWidget` (modeled on UXP `widget/PuppetWidget`) that owns no native window and paints into the shared framebuffer. |
| `GoannaFrameSink.{h,cpp}` | Readback/composite of the dirty region into the shmem ARGB32 buffer; emits `msgPainted`, honors `returnBuffer`. |
| `GoannaListeners.{h,cpp}` | `nsIWebProgressListener`, `nsIWebBrowserChrome`, `nsIURIContentListener`, prompt/cert/download listeners → YAP `msg…` emissions. |
| `InputBridge.{h,cpp}` | webOS key/mouse/gesture/touch → `nsIDOMWindowUtils` synthesized DOM events. |
| `EngineHost.{h,cpp}` | One-time XPCOM/embedding startup (`NS_InitEmbedding`-equivalent for UXP), profile dir, pref overrides, GLib↔Goanna event-loop bridge. |
| `Branding/` | Pref + resource overrides that strip Pale Moon/Basilisk branding and set Jihad Browser app name/UA. |

## Engine dependency

UXP/Goanna is **built out-of-tree** from the upstream UXP source
(`../../../UXP`) and linked; it is **not vendored** in this repo. The build
wiring lives in `../../build/`. A `.mozconfig`-style configuration that produces
a minimal embedding-capable Goanna (no full browser front-end, basic layers
first) is part of the Phase 1 build site.

## Why this is the load-bearing work

Goanna has no first-class embedding product (unlike WebKit's QtWebKit port).
The embedding API (`nsIWebBrowser`) and the puppet/offscreen widget machinery
exist in-tree, but wiring them into a headless render-to-shmem server is the
novel, highest-risk part of the port. It is sequenced first on desktop x86_64
(Phase 1) precisely to de-risk it before the ARM toolchain fight (Phase 2).
