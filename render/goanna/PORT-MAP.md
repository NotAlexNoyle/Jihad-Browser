# Port map: YAP contract → Goanna (UXP)

How each part of the BrowserServer contract (see `../../docs/IPC-CONTRACT.md`)
is satisfied by the Goanna engine. This replaces the QtWebKit `BrowserPage`
internals; the YAP interface and shared-buffer plumbing are unchanged.

## Engine entry points (UXP)

| Need | UXP/Goanna API | Notes |
|---|---|---|
| Embed a page | `nsIWebBrowser` (`embedding/browser/nsIWebBrowser.idl`) | Created via `nsIComponentManager`/`do_CreateInstance("@mozilla.org/embedding/browser/nsWebBrowser;1")`. |
| Host window | `nsIBaseWindow` + custom `nsIWidget` | Use an **offscreen widget** modeled on `widget/PuppetWidget.{h,cpp}` so all painting goes to a buffer, not a native window. |
| Navigation | `nsIWebNavigation` (QI from the webBrowser) | `loadURI`, `goBack`, `goForward`, `reload`, `stop`, `canGoBack/Forward`, session history. |
| Load/progress events | `nsIWebProgressListener` via `nsIWebProgress` | → `msgLoadStarted/Progress/Stopped`, `msgLocationChanged`, `msgFailedLoad`. |
| Title/URI/chrome callbacks | `nsIWebBrowserChrome`, `nsIEmbeddingSiteWindow`, `nsIURIContentListener` | → `msgTitleChanged`, `msgLinkClicked`, MIME handoff, popups, dialogs. |
| Painting | Layers/compositor → readback, **or** `nsIWebBrowser` + `nsIDOMWindowUtils::renderDocument`/`drawWindow` into a `gfxContext` over the shared buffer | The offscreen widget owns the surface; flush into shmem then `msgPainted`. |
| Input | `nsIDOMWindowUtils::sendMouseEvent / sendKeyEvent / sendTouchEvent / sendNativeMouseEvent` | Synthesize from `clickAt`/`keyDown`/`mouseEvent`/`gestureEvent`/`touchEvent`. |
| Zoom / viewport | `nsIDOMWindowUtils::setResolutionAndScaleTo`, full-zoom via `nsIContentViewer.fullZoom`, meta-viewport via mobile viewport handling | → honor `setZoomAndScroll`, emit `msgMetaViewportSet`. |
| Find | `nsIWebBrowserFind` (`toolkit/components/typeaheadfind` / `nsITypeAheadFind`) | `findString`. |
| Selection/clipboard | `nsISelectionController`, `nsIClipboard`, `nsICommandManager` (`cmd_copy/cut/paste/selectAll`) | copy/cut/paste/selectAll, caret bounds via selection range rects. |
| Cookies / cache | `nsICookieManager2`, `nsICacheStorageService` | `clearCookies`, `clearCache`, `setAcceptCookies`. |
| User agent | `nsIHttpProtocolHandler` userAgent override / pref `general.useragent.override` | `setUserAgent`. |
| Downloads | `nsIWebBrowserPersist` / `nsIExternalHelperAppService` / `nsIHelperAppLauncherDialog` | download start/progress/error/finished, MIME not-supported handoff. |
| TLS/cert prompts | `nsICertOverrideService`, `nsIBadCertListener2` | `msgDialogSSLConfirm` (bridge to webOS CertificateMgr as the QtWebKit path did). |
| JS dialogs | `nsIPromptService` / `nsIPrompt` override | `msgDialogAlert/Confirm/Prompt/UserPassword`. |
| Network interface / DNS | proxy + `nsIDNSService` / interface binding via socket transport | `setNetworkInterface`, `setDNSServers` (may need engine patch for bind-to-iface). |

## Command → Goanna action (selected; full list in IPC-CONTRACT.md)

| YAP async command | Goanna action |
|---|---|
| `connect` / `thaw` | Attach shmem buffers to the offscreen widget's surface; create `nsIWebBrowser` if needed; resize. |
| `setWindowSize` / `setVirtualWindowSize` | Resize offscreen widget + set CSS viewport / `setCSSViewport`. |
| `openUrl` | `nsIWebNavigation.loadURI`. |
| `setHtml(url,body)` | `loadURI` with a `data:`/stream, or `nsIWebBrowserStream`. |
| `back/forward/reload/stop` | corresponding `nsIWebNavigation` calls. |
| `clickAt(cx,cy,n)` | `sendMouseEvent("mousedown"/"mouseup", …, clickCount=n)` at content coords. |
| `keyDown/keyUp` | `sendKeyEvent` (map webOS keycodes+mods → DOM `KeyboardEvent`). |
| `mouseEvent/gestureEvent/touchEvent` | `sendMouseEvent`/`sendTouchEvent`; pinch/rotate → resolution change + synthesized touch. |
| `setScrollPosition` / `scrollLayer` | `nsIDOMWindowUtils.scrollToCSSPixels` / async scroll APIs. |
| `setZoomAndScroll` | `setResolutionAndScaleTo` + scroll. |
| `findString` | `nsIWebBrowserFind.searchString` + `findNext`. |
| `copy/cut/paste/selectAll` | `nsICommandManager.doCommand("cmd_*")`. |
| `clearCache/clearCookies` | cache storage clear / cookie manager `removeAll`. |
| `setEnableJavaScript/setBlockPopups/setAcceptCookies/setMinFontSize/setUserAgent` | set corresponding `nsIPrefBranch` prefs / per-docshell `nsIDocShell.allowJavascript`. |
| hit-test family | `nsIDOMWindowUtils.elementFromPoint` + element geometry → matching `msg…Response`. |

## Paint loop (the hard part)

1. Goanna invalidates a region (via the offscreen widget's `Invalidate`).
2. Backend schedules a paint on the BrowserServer paint timer (keep the existing
   GSource cadence so input/paint interleave like the QtWebKit path).
3. On paint: composite/readback the dirty region into the inactive shmem buffer
   in ARGB32, matching `BrowserOffscreenInfo` stride/format.
4. Emit `msgPainted(shmKey)`; wait for `returnBuffer` before reusing that buffer
   (double-buffered with key1/key2).

Open questions to resolve in Phase 1:
- Use the **basic (non-GL) layer manager** with `drawWindow`/`renderDocument`
  readback (simplest, CPU paint — best first target), vs. an OpenGL/EGL
  compositor reading back via `glReadPixels` (needed for video/WebGL later).
- Whether to run Goanna on the BrowserServer main GLib loop or spin Goanna's
  own `nsIThreadManager` event loop and bridge. Likely: integrate Goanna's
  event loop into GLib via `nsIEventTarget` + a GSource pump.

These are tracked as build-site tasks; see `../../context/`.

## Rendering architecture (resolved 2026-06-30)

Findings from bringing libxul up (T-010) and the embedding smoke test (T-013):

- **Embedding works via the frozen API**: `XRE_InitEmbedding2` + `nsIWebBrowser`
  (`EngineHost`) bring Goanna up headless and create a browser instance. Proven.
- **But offscreen *rendering* needs more than the frozen SDK.** This UXP
  revision (ESR-52 base) has **no headless widget backend** (that landed in
  Gecko 56+), and `nsIDOMWindowUtils` (drawWindow/renderDocument) is **internal**
  — not in `dist/include` frozen headers. So pixel readback can't be done purely
  as an external glue consumer.
- Consequences for the Goanna `BrowserPage` backend:
  1. It needs a **widget + a display/compositor** to lay out and paint a page.
     On desktop that means an (invisible) GTK window under a virtual display
     (**Xvfb**); on webOS it targets the device framebuffer/EGL.
  2. **Pixel readback** into the shared buffer needs internal gfx/layers/presShell
     access. Two viable routes:
     a. Build the backend **in-tree** as part of the UXP build (a new directory
        compiled with `MOZILLA_INTERNAL_API`), like the e10s content-process
        PuppetWidget renders into a shared surface. Most faithful to BrowserServer.
     b. Expose the needed internal headers/libs to an out-of-tree backend.
- **Decision**: pursue route (a) — the render backend is an in-tree-compiled
  component using an offscreen widget + basic (CPU) layer manager, reading the
  layer buffer back into the BrowserServer shmem (`BrowserOffscreenInfo` format).
  The frozen-API `EngineHost` (navigation, lifecycle, listeners) still drives it.

This refines kit cavekit-offscreen-rendering.md R1/R2: "offscreen widget" =
windowless GTK/native widget under a virtual display, not a from-scratch
PuppetWidget, and readback uses the internal layer manager.

### Confirmed by experiment (embed_load + dead-ends.md)

The GTK widget on this UXP/Linux **forces a `ClientLayerManager`** (compositor-
backed) — `layers.offmainthreadcomposition.enabled=false` does NOT downgrade it
to BasicLayerManager. And `XRE_InitEmbedding2` does **not** start the in-process
compositor the ClientLayerManager forwards to, so the first paint crashes in
`ClientLayerManager::ForwardTransaction` (null forwarder).

→ The render backend must run with the **compositor brought up** (CompositorThread
+ CompositorBridgeChild/Parent), i.e. the full-app/in-tree path, before any
paint. Navigation/lifecycle via the frozen `EngineHost` already works headless
(page loads to STATE_STOP); only paint needs the compositor. T-020 starts by
initializing the compositor, then reading the rendered layer/window back into the
shmem buffer.

