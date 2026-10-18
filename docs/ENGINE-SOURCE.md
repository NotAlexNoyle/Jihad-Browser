# Engine Source & Modifications (MPL-2.0 compliance)

*cavekit-licensing-branding R3 / R4 — engine origin, patches, and branding strip. Brought up to date 2026-10-08 (it had listed four patches under old names).*

Jihad Browser renders with the **UXP / Goanna** web engine (MPL-2.0). This
document records the engine's origin and every modification made to it, so the
MPL-2.0 obligation to keep modified files under the MPL with their source
available is met.

## Origin

- **Upstream:** UXP — https://repo.palemoon.org/MoonchildProductions/UXP, pristine at the release
  `RB_20261006` (`5f290d1097a8350b56ce2092aa9d5fc9147c6661`, 2026-10-04) — `UXP_TAG` / `UXP_BASE` in
  `build/engine/engine-version.env`. (Until 2026-10-12: master `b2594a4`, 2026-06-30.)
- **Our modified engine:** the branch `jihad/uxp-RB_20261006` of the fork `NotAlexNoyle/UXP-Jihad`,
  at `UXP_PIN` (`011aa89c2f05d0f4e2665515d486f6abb6cbe3ad`), present in this repository as the
  submodule `third_party/uxp`. The device build compiles that tree.
- **The modifications as patches:** `build/desktop/patches/` holds every change as a patch
  against `UXP_BASE`. `build/engine/patch-queue.sh --check` proves the patches, applied in
  order, produce exactly the `UXP_PIN` tree. How the two are kept in step: `docs/ENGINE.md` §2.

## Modifications to the engine (all MPL-2.0, source here)

Every modified UXP file stays under the MPL-2.0. These patches are the source of the
modifications; the same changes are the commits of `jihad/uxp-RB_20261006`, one per patch.

| Patch | Files | What it changes |
|-------|-------|-----------------|
| `0001-js-format-overflow.patch` | `js/src/jit/x64/BaseAssembler-x64.h` | GCC 9: source pragma for -Wformat-overflow (js/src re-escalates it) |
| `0002-configure-warnings.patch` | `build/moz.configure/warnings.configure` | GCC 9: warnings.configure adjustment |
| `0003-gfx-init-and-omtc.patch` | `gfx/thebes/gfxPlatform.cpp`, `gfx/thebes/gfxPlatformGtk.cpp`, `toolkit/xre/nsEmbedFunctions.cpp` | JIHAD_DISABLE_OMTC forces the in-process BasicLayerManager; gfx started inside XRE_InitEmbedding2 |
| `0004-gtk-headless-guards.patch` | `widget/gtk/gtk2drawing.c`, `widget/gtk/nsLookAndFeel.cpp`, `widget/gtk/nsNativeThemeGTK.cpp` | GTK realize/theme/look-and-feel paths guarded so libxul runs with no X server |
| `0005-puppetwidget-offscreen-zoom.patch` | `widget/PuppetWidget.cpp`, `widget/PuppetWidget.h` | PuppetWidget offscreen render, zoom and damage tracking; the jihad_offscreen_* exports |
| `0006-headless-toolkit.patch` | `embedding/components/printingui/moz.build`, `gfx/2d/2D.h`, `gfx/2d/Factory.cpp` (+19 more) | MOZ_WIDGET_TOOLKIT=headless: gfxPlatformHeadless, widget/headless, build wiring |
| `0007-nss-mainthread-marshal.patch` | `security/manager/ssl/nsNSSComponent.cpp` | NSS EnsureNSSInitialized main-thread fast path; the jihad_init_nss export |
| `0008-plugin-headless-nativehandle.patch` | `dom/plugins/ipc/PluginInstanceChild.cpp`, `dom/plugins/ipc/PluginInstanceParent.cpp`, `dom/plugins/ipc/PluginMessageUtils.h` | NativeWindowHandle for the headless platform so plugin IPC compiles |
| `0009-misc-headless.patch` | `ipc/chromium/src/base/message_loop.cc`, `toolkit/xre/nsXREDirProvider.cpp` | Headless build fixes (message loop, XRE directory provider) |
| `0010-branding-strip.patch` | `docshell/base/nsAboutRedirector.cpp`, `modules/libpref/init/all.js`, `toolkit/xre/nsAppRunner.cpp` | Pale Moon / Basilisk / Moonchild branding stripped (cavekit-licensing-branding R3) |
| `0011-appcompat-guid.patch` | `xulrunner/configure.in`, `xulrunner/confvars.sh` | The AppCompat GUID (xulrunner configure) |
| `0012-popup-instrumentation.patch` | `layout/xul/nsXULPopupManager.cpp` | XUL popup instrumentation and offscreen compositing of popups |
| `0013-xpi-install-embedding.patch` | `toolkit/mozapps/extensions/AddonManager.jsm`, `toolkit/mozapps/extensions/amInstallTrigger.js` | AddonManager / amInstallTrigger work without a browser element |
| `0014-badcertlistener-embedding.patch` | `docshell/base/nsDocShell.cpp`, `embedding/browser/nsDocShellTreeOwner.cpp` | Certificate errors reach the embedder (bad-cert listener) |
| `0015-find-embedder-no-jscontext.patch` | `embedding/components/find/nsWebBrowserFind.cpp` | Find-in-page without a JS context |
| `0016-npapi-windowless-headless.patch` | `dom/plugins/base/nsNPAPIPlugin.cpp` | NPAPI windowless plugins under the headless toolkit |
| `0017-npapi-webos-palm-host.patch` | `dom/plugins/ipc/JihadNPNInterpose.cpp`, `dom/plugins/ipc/PluginInstanceChild.cpp`, `dom/plugins/ipc/PluginInstanceChild.h` (+2 more) | The webOS (Palm) NPAPI host: NPEvent union, JihadNPNInterpose |
| `0018-npapi-child-process-loader.patch` | `ipc/glue/GeckoChildProcessHost.cpp` | The plugin child process launched through our loader |
| `0019-bhm-null-manager-guard.patch` | `xpcom/threads/BackgroundHangMonitor.cpp` | BackgroundHangMonitor null-manager guard |
| `0020-npapi-composite-without-imagebridge.patch` | `dom/plugins/ipc/PluginInstanceParent.cpp` | Plugin compositing without ImageBridge |
| `0021-npapi-windowless-invalidate-offscreen.patch` | `dom/plugins/base/nsPluginInstanceOwner.cpp` | Windowless plugin invalidation into the offscreen widget |
| `0022-npapi-recvshow-copy-frameid.patch` | `dom/plugins/ipc/PluginInstanceParent.cpp` | Plugin RecvShow copies the frame id |
| `0023-npapi-operational-logging.patch` | `dom/base/nsObjectLoadingContent.cpp`, `dom/plugins/base/nsNPAPIPlugin.cpp`, `dom/plugins/base/nsPluginHost.cpp` (+5 more) | Plugin operational logging |
| `0024-npapi-webos-plugin-input.patch` | `dom/plugins/base/npapi.h`, `dom/plugins/base/nsPluginHost.cpp`, `dom/plugins/base/nsPluginHost.h` (+6 more) | webOS plugin input, the frozen plugin YAP commands, the frame path |
| `0025-npapi-pmlog-named-semaphore-abi.patch` | `ipc/app/moz.build`, `ipc/contentproc/JihadPmLogSem.c`, `ipc/contentproc/moz.build` (+1 more) | PmLog named-semaphore ABI shim for the plugin container |
| `0026-npapi-plugin-repaint-rate.patch` | `dom/plugins/ipc/PluginInstanceChild.cpp` | Plugin child repaint rate (frame clock) |
| `0027-npapi-plugin-cpu-boost.patch` | `dom/plugins/ipc/PluginModuleParent.cpp` | CPU boost while a plugin plays |
| `0028-npapi-plugin-frame-seq.patch` | `dom/plugins/ipc/PluginInstanceParent.cpp` | Plugin frame counter export (jihad_plugin_frame_seq) |
| `0029-npapi-plugin-pull-frame.patch` | `dom/plugins/ipc/PPluginInstance.ipdl`, `dom/plugins/ipc/PluginInstanceChild.cpp`, `dom/plugins/ipc/PluginInstanceChild.h` (+1 more) | Plugin frames pulled from the daemon's clock (JihadRequestFrame IPDL) |
| `0030-cubeb-pulse-webos-virtual-sink.patch` | `media/libcubeb/src/cubeb_pulse.c` | cubeb Pulse: the webOS virtual sink (JIHAD_PULSE_SINK) |
| `0031-npapi-child-self-drive-switch.patch` | `dom/plugins/ipc/PluginInstanceChild.cpp` | JIHAD_PLUGIN_SELF_DRIVE=0 turns the plugin child's own repaint timer off |
| `0032-videocontrols-fix-scrubber-flicker-settle-interpolat.patch` | `toolkit/content/widgets/videocontrols.xml` | Video controls: scrubber flicker and settle, interpolated thumb |
| `0033-videocontrols-surface-media-errors-for-audio-only-co.patch` | `toolkit/content/widgets/videocontrols.xml` | Video controls: media errors shown for audio-only content |
| `0034-cubeb-classify-media-streams-export-a-playing-stream.patch` | `media/libcubeb/src/cubeb_pulse.c` | cubeb: media.role classification; jihad_pulse_active_streams export |
| `0035-gfx-BasicPlanarYCbCrImage-CopyData-must-not-report-f.patch` | `gfx/layers/basic/BasicImages.cpp` | gfx: BasicPlanarYCbCrImage::CopyData no longer reports failure under delayed conversion |
| `0036-extensions-a-link-or-navigation-to-an-.xpi-must-reac.patch` | `toolkit/mozapps/extensions/amContentHandler.js` | A link or navigation to an .xpi reaches the install flow without a browser window |
| `0037-plugins-an-instance-with-no-window-to-be-given-is-wi.patch` | `dom/plugins/base/nsPluginInstanceOwner.cpp` | A plugin instance with no window is windowless instead of a null dereference |
| `0038-plugin-container-do-not-hand-the-bundled-library-pat.patch` | `ipc/contentproc/plugin-container.cpp` | plugin-container does not hand the bundled library path to what the plugin runs |
| `0039-addons-an-add-on-naming-both-the-AppCompat-GUID-and-.patch` | `toolkit/mozapps/extensions/internal/XPIProvider.jsm` | XPIProvider: an add-on naming the AppCompat GUID and the toolkit matches the entry that fits |
| `0040-gpu-compositor-an-in-process-OpenGL-compositor-behin.patch` | `gfx/layers/client/ClientPaintedLayer.cpp`, `gfx/layers/composite/LayerManagerComposite.{h,cpp}`, `gfx/layers/ipc/CompositorBridgeParent.{h,cpp}`, `gfx/layers/opengl/CompositingRenderTargetOGL.{h,cpp}`, `gfx/layers/opengl/CompositorOGL.{h,cpp}`, `gfx/layers/opengl/TextureHostOGL.cpp`, `gfx/thebes/gfxPlatform.cpp`, `gfx/thebes/gfxPlatformHeadless.{h,cpp}`, `gfx/thebes/gfxPrefs.h`, `modules/libpref/init/all.js`, `widget/PuppetWidget.{h,cpp}` | T-502: with `JIHAD_GPU_COMPOSITOR` set, an in-process OpenGL compositor (an EGL pbuffer, two renderbuffer window targets) behind the offscreen widget, pulled a frame at a time by the daemon (`jihad_offscreen_compose_start/_poll/_forget`, `jihad_offscreen_compositor_seq`, `jihad_offscreen_set_view`); the layer tree's own diff drives band readbacks and scissored partial composites; the window's flipped view kept and the rows reversed in the read; tiled content layers on by default (`layers.enable-tiles`); the read thread, the GPU swizzle pass and the snapshot road are opt-in (`docs/RENDERING-OPTIONS.md`); nothing changes without the env |
| `0041-gl-the-Adreno-220-whole-texture-uploads-BGRA-uploads.patch` | `gfx/gl/GLContext.{h,cpp}`, `gfx/gl/GLContextEGL.h`, `gfx/gl/GLContextProviderEGL.cpp`, `gfx/gl/GLTextureImage.{h,cpp}`, `gfx/gl/GLUploadHelpers.cpp` | `GLRenderer::AdrenoTM220`; `CanUploadSubTextures()` false for the Adreno 2xx "(TM)" spellings and the 220 (a sub-image update costs 16x a whole upload); an EGL context prefers BGRA uploads where `GL_EXT_texture_format_BGRA8888` exists, with GLES-legal types (4x faster than RGBA when the GPU is idle); `GLContextEGL::JihadCreateSharedContext` (a 1x1 pbuffer context sharing the compositor's objects, for a readback thread); fresh texture names per upload opt-in (`JIHAD_GPU_FRESH_TEX=1`) |
| `0042-gpu-compositor-the-card-s-scroll-applied-in-the-comp.patch` | `gfx/gl/GLContext.h`, `gfx/gl/GLContextEGL.h`, `gfx/gl/GLContextProviderEGL.cpp`, `gfx/layers/Layers.cpp`, `gfx/layers/composite/AsyncCompositionManager.{h,cpp}`, `gfx/layers/composite/LayerManagerComposite.{h,cpp}`, `gfx/layers/ipc/CompositorBridgeParent.{h,cpp}`, `gfx/layers/opengl/CompositorOGL.{h,cpp}`, `layout/base/nsDisplayList.cpp`, `layout/base/nsLayoutUtils.cpp`, `layout/generic/nsGfxScrollFrame.cpp`, `widget/PuppetWidget.{h,cpp}` | T-502: the card's scroll applied by the composition manager as APZ would (root scroll metadata and the scrollport as a scroll clip for a root scroll frame with a displayport, the displayport set from layout, no intermediate surface for a scaled container); the context's pbuffer at the frame's size (the Adreno 220 bins an FBO render by the surface's size); scroll-only frames composed band by band on their own 768x256 pbuffer and FBO with the row shift behind the GPU; a done-fd that wakes the daemon when a job finishes |

### Branding strip (cavekit-licensing-branding R3) — patch 0010

Part of the engine branch since the patch series was committed (patch 0010; it was once a `sed`
step in `build-goanna.sh`). The UA product is already `Goanna` (`netwerk/protocol/http/nsHttpHandler.cpp`
`mProduct`); this removes the residual Pale Moon / Basilisk / Moonchild strings
so a `strings(1)` scan of the shipped artifacts is clean:

| File | Change |
|------|--------|
| `modules/libpref/init/all.js` | `extensions.blocklist.url` (basilisk-browser.org) → `about:blank`; `captivedetect.canonicalURL` (detectportal.palemoon.org) → example.com; two comment URLs neutralized |
| `docshell/base/nsAboutRedirector.cpp` | `about:credits` target (palemoon.org Contributors) → the Jihad credits URL |
| `toolkit/xre/nsAppRunner.cpp` | dead `MOZ_APP_NAME == "basilisk"/"palemoon"` literals (always false for the `xulrunner` app) → `"n/a"` — behavior identical, literals removed |

**How the strip is verified**, and its result, is an acceptance criterion —
`context/kits/cavekit-licensing-branding.md` R3. The check itself is a `strings` scan of
`libxul.so`, `jihad-browserserver` and `goanna.js` for `basilisk`/`palemoon`/`moonchild`.

## Trademark note

Per the UXP redistribution terms, an engine build shipped under a name other than
Pale Moon / Basilisk must strip that project's branding. "Jihad Browser" is the
product name of this fork and implies no endorsement by any upstream. See the
top-level `LICENSE` for the full component/license breakdown and the Apache-2.0 ↔
MPL-2.0 compatibility statement.
