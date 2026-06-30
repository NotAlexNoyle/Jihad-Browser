---
created: "2026-06-30"
last_edited: "2026-06-30"
---

# Dead Ends / Hard-won findings

Failed approaches and their root causes, so they are not retried.

## Headless rendering via the minimal external embedder (frozen API) — does NOT work for paint

**What was tried:** drive paint by giving the `nsIWebBrowser` (created via the
frozen `XRE_InitEmbedding2` embedding API) a real GTK top-level window under
Xvfb, mapping it (`gtk_widget_show_all`) so the first expose paints, then reading
the window pixels back with GDK.

**Result:** segfault on the first expose, every time:
`mozilla::layers::ClientLayerManager::ForwardTransaction` dereferences a null
forwarder (gfx/layers/client/ClientLayerManager.cpp:374) via
`nsWebBrowser::PaintWindow` → `nsWindow::OnExposeEvent`.

**Root cause:** on this UXP/Linux (GTK2) build the widget **always** uses a
compositor-backed `ClientLayerManager`, which forwards paint transactions to a
compositor over IPC. The pref `layers.offmainthreadcomposition.enabled=false`
(verified read: "OMTC enabled pref = 0") does **not** switch it to the
in-process `BasicLayerManager` — the GTK widget forces compositing. And
`XRE_InitEmbedding2` does **not** bring up the in-process compositor
(CompositorThread + CompositorBridgeChild/Parent) that the ClientLayerManager
needs, so its forwarder is null → crash.

**Conclusion / what to do instead:** rendering requires the full compositor
bring-up that normal app startup (XRE_main / nsAppShell) performs, not the
minimal `XRE_InitEmbedding2` embedding path. The render backend must either
(a) be built/run with the full app + compositor initialization (the in-tree
component route in render/goanna/PORT-MAP.md), or (b) explicitly initialize the
compositor infrastructure before creating the widget. The frozen-API
`EngineHost` remains correct for **navigation/lifecycle** (load works headless —
see embed_load); only **paint** needs the compositor.

**Still works (not a dead end):** `XRE_InitEmbedding2` + `nsIWebBrowser` +
event loop + `AddWebBrowserListener` → a page **loads to STATE_STOP** headless
(embed_load default path, exit 0). Just don't map/paint the window without a
compositor. The render attempt is gated behind `JIHAD_TRY_RENDER`.

## Headless rendering via the print path (nsIWebBrowserPrint → PDF/PS) — HANGS

**What was tried:** after the page loads, render it to a file via the print path
(which uses its own print device context, not the screen compositor):
`nsIWebBrowserPrint::Print(settings, nullptr)` with `printToFile=true`,
`outputFormat=kOutputFormatPDF`, `printSilent=true`, `showPrintProgress=false`.
Hope: bypass the ClientLayerManager screen-paint crash entirely.

**Result:** `Print()` **hangs** — it spins an internal nested event loop that
never returns in a headless embedder (no output, container had to be killed).
Our own pump loop after `Print()` is never reached.

**Likely cause:** the print engine waits on UI/presentation state (print
progress, a focused presentation, or a printer/params round-trip) that never
arrives headlessly. Not pursued further.

**Conclusion:** reinforces the same verdict — there is no quick frozen-API render
path from the minimal embedder. Rendering goes through the in-tree/full-app
compositor route. The render attempt in embed_load is gated off by default; the
print path was removed (kept this note instead of dead code).

## Other notes
- `-Wno-error=format-overflow` via warnings.configure does not reach js/src
  (js appends `-Werror=format` after the warnings list) — fixed with a source
  pragma instead (patches/0002). See git log T-010.
