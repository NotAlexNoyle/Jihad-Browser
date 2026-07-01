---
created: "2026-06-30"
last_edited: "2026-06-30"
---

# Implementation Overview

## Domain Status
| Domain | Tasks Done | Tasks Total | Status |
|--------|-----------|-------------|--------|
| IPC Contract Preservation | 1 (partial) | 5 | T-004 done; T-005/T-006 sources imported, not yet building |
| Licensing & Branding | **5 (COMPLETE)** | 5 | R1 headers, R2 LICENSE/NOTICE, R5 Apache↔MPL compatibility — done. **R3 branding strip: build-goanna.sh removes Pale Moon/Basilisk/Moonchild from all.js + nsAboutRedirector + the dead nsAppRunner literals; scan verified 0 branding strings in libxul.so / jihad-browserserver / goanna.js; round-trip still passes. R4: docs/ENGINE-SOURCE.md documents the UXP origin (pinned rev b2594a4), all patches (0001-0004 + strip), and MPL source-availability; LICENSE points to it.** |
| UI Shell | 3 | 4 | T-007/T-008/T-009 done; T-004(ui) R4 pending |
| Engine Embedding & Build | **R1–R4 (R2 verified)** | 4 | libxul builds out-of-tree (R1); runtime init/shutdown once per process; **R2 repeated create/destroy: 20/20 cycles each render a full frame, no crash/leak (LEAK-CYCLE PASS)**; R3 event loop integrated (daemon tick 10ms + g_timeout, responsive, no busy-wait); R4 not vendored (git-ignored, docs/ENGINE-SOURCE.md) |
| Navigation, Loading & Events | **R1–R6 COMPLETE** | 6 | load lifecycle; back/forward (real canGo*); setHtml — NAV PASS. R3 failed-load — FAIL-EVENT PASS. R1 clearHistory + R5 getHistoryState — HISTORY PASS. **R4 url-redirected (STATE_REDIRECTING) — REDIRECT PASS (302 /a→/b via local server). R6: global-history + addUrlRedirect (POSIX-regex rules, RULES PASS: tel: handed off) + link-clicked (content-initiated nav via programmatic-load heuristic, LINK PASS: click →/b reported).** Full domain verified end-to-end |
| IPC Contract / Daemon | **ROUND-TRIP PASS + lifecycle** | — | Real daemon (libYap + unchanged BrowserServerBase + JihadBrowserServer + Goanna); ROUND-TRIP PASS (Connect+OpenUrl→…→msgPainted). **R2/R3: freeze suppresses paint + thaw reattaches buffers + resumes — FREEZE-THAW PASS; returnBuffer wired.** ~40 commands wired (nav/input/geometry/services/dialogs/downloads/history/redirect-rules/drag). findString kept safe (offscreen selection controller = future). Genuinely device-gated: R4 LunaService (device build), R5 real NPAPI BrowserAdapter rebuild. Remaining niche stubs are engine-inapplicable no-ops (plugin spotlight, spelling widget, mouse-mode, DNS/network-iface) |
| Offscreen Rendering | **T-020+T-024 render→shmem** | 5 | **Goanna renders real web pages** — data: page (docs/jihad-render-proof.png) AND live **https://example.com over TLS** (docs/jihad-render-example-com.png); consolidated into the reusable **GoannaRenderPage** backend class (Create/LoadUrlAndWait/ReadPixels→ARGB32 shm); msgPainted wiring in the daemon next |
| Offscreen Rendering (geometry) | **R4 + R5 COMPLETE** | 5 | R5: Resize→RESIZE PASS; ScrollTo(javascript:)+GetScrollXY→SCROLL PASS; SetZoom(nsIContentViewer::SetFullZoom via proper content-docshell)→ZOOM PASS (9×). R4 events: GetContentSize(GetRootBounds)→msgContentsSizeChanged, GetViewport(GetViewportInfo, meta-viewport pref on)→msgMetaViewportSet, pump-polled msgScrolledTo — all emit through BrowserPageGoanna→ProxySink→YAP; GEO PASS (2032×2500; init/min/max=0.5/0.5/2.0 us=1; (0,400)). All wired to the daemon. GetDocShell also repaired SetJavaScriptEnabled |
| Input Bridging | **click/key/mouse + coord-mapping (R5)** | 5 | ClickAt/KeyEvent/MouseEvent via nsIDOMWindowUtils — INPUT PASS. **R5 coord-mapping: clickAt uses content/CSS space (proven via coord_test: surface≠content at zoom); bridge now maps adapter surface coords → content (surface/zoom + scroll) for click/mouse/touch — COORDMAP PASS (surface(240,240)@2x hits the box at content(120,120)).** **R1 holdAt (contextmenu) + R4 drag scrolling (dragProcess→scroll, msgScrolledTo) — INPUT2 PASS (holdAt hits, drag scrolls 200).** TouchEvent + insertStringAtCursor wired but desktop-unverified (offscreen widget doesn't route synth touch; execCommand insertText via javascript: doesn't preserve input focus headless) — both on-device. Remaining: R3 pinch/tap gesture (on-device) |
| Navigation, Loading & Events | 0 | 6 | not started |
| Browser Services | **R1 settings + R2 cache/cookies + R3 dialogs** | 5 | R1 complete: setEnableJavaScript, setUserAgent, **setMinFontSize/setBlockPopups/setAcceptCookies** (SETTINGS2 PASS — prefs applied + window.open blocked behaviorally). R2: clearCache/clearCookies (SERVICES PASS). R3: DialogService overrides `@mozilla.org/prompter;1` — alert/confirm/prompt captured + reply routed (DIALOG PASS), installed in EngineHost so the daemon never hangs. R4 downloads: DownloadService overrides `@mozilla.org/helperapplauncherdialog;1` — handoff captured (DOWNLOAD PASS). **R5 TLS: invalid cert detected via the security-module document-stop status → msgSSLConfirm(host, code) (TLS PASS: self-signed 127.0.0.1 → host+0x805a2fe3 surfaced, reject-aborts = page not loaded).** Device-gated [human-review on device]: accept-proceeds (the untrusted cert object isn't exposed headless — nsIBadCertListener2 not consulted, SSL-status/failed-chain null) + webOS cert store. Remaining: save-to-disk/progress + per-adapter blocking dialog delivery (adapter/device) |
| Desktop Build & PoC | **R1–R3 done; R4 [human-review]** | 4 | R1: single-entry build produces runnable jihad-browserserver (Goanna backend, LunaService compiled out) — DAEMON_UP. R2: jihad-adapter allocates buffers, Connect+OpenUrl, receives msgPainted, **writes out/jihad-poc-render.ppm** (→ docs/jihad-poc-render.png), returns buffer (0x150d wired). R3: real page renders through the whole pipe, lifecycle in order — ROUND-TRIP PASS. R4 (Enyo UI on desktop) recorded [human-review]: no desktop Enyo runtime → harness is the acceptance vehicle. See docs/DESKTOP-POC.md |
| Device Build & Packaging | 0 | 5 | not started |

## Completed this session (Tier-0, build-independent)
- **T-001/T-002** — LICENSE + NOTICE + license texts; Apache+MPL compatibility stated.
- **T-003** — License-header policy; verified 0/32 imported files missing Apache header.
- **T-004** — YAP interface imported & frozen; `BrowserServerBase.{h,cpp}` md5-verified byte-identical to upstream.
- **T-005** — Shared-mem framebuffer + offscreen-info contract sources imported (engine-agnostic).
- **T-006** — Daemon support sources imported (agnostic bucket); daemon/page-manager imported but flagged for de-Qt adaptation (see render/browserserver/Src/MANIFEST.md).
- **T-007** — App package rebranded (`net.riverstonerelay.jihad`, title "Jihad").
- **T-008** — Verified UI `callBrowserAdapter` set + browserServer Luna URIs identical to upstream isis.
- **T-009** — Verified 24/24 forked UI `.js` retain Apache headers.

See impl-browserserver-import.md for the import detail.

## T-010 build environment (this session) — configure VALIDATED
- Authored pinned build container under `build/desktop/`: `Dockerfile`,
  `mozconfig.goanna` (xulrunner embedding target, GTK2 + basic layers, trimmed),
  `build-goanna.sh`. UXP source is mounted (read-write — autoconf regenerates
  `configure`), never vendored. Documented in `docs/TOOLCHAIN.md`.
- Discovered the hard way (this UXP master is recent, not raw ESR-52):
  - `mach` requires **Python 3.3+** (build/mach_bootstrap.py), not Python 2.
  - configure requires **GCC ≥ 9.1** (build/moz.configure FatalCheckError).
  - needs the full **X11 dev set** (libx11-xcb, xcb, xcomposite, xdamage, …).
  - podman needs **fully-qualified** image names; bionic→archive not old-releases.
  - Net: base image is **Ubuntu 20.04** (GCC 9.3, Python 3.8, autoconf2.13, yasm).
- `mach build` **SUCCEEDS** ("Your build was successful!"). Artifacts in
  `build/desktop/out/obj-jihad-goanna/dist`: **libxul.so (1.8G, unstripped),
  xpcshell, NSS/NSPR**, and the embedding headers the backend needs
  (nsIWebBrowser.h, nsIWebNavigation.h, nsIBaseWindow.h, nsIDOMWindowUtils.h,
  nsEmbedCID.h). **T-010 DONE** — the engine builds for desktop.
- Toolchain fights resolved en route (all captured in build/desktop/): podman
  fully-qualified image; bionic→archive; **Python 3.3+** (not py2); **GCC ≥ 9.1**
  → Ubuntu 20.04 base; full X11 dev set; and the real libxul blocker — js/src
  appends `-Werror=format` after the warnings list, so a command-line
  `-Wno-error=format-overflow` is re-escalated; fixed with a source pragma in
  `js/src/jit/x64/BaseAssembler-x64.h` (patches/0002, applied by build-goanna.sh).
- `Settings.{h,cpp}` reclassified: engine-coupled (QtWebKit WebSettings), needs
  reimplementation against Goanna prefs, not a mechanical de-Qt (see MANIFEST).
- Note: libxul is unstripped (-g) → 1.8G; strip for the device build (Phase 2).

## T-013 embedding runtime — smoke PASS (this session)
- `render/goanna/EngineHost.{h,cpp}` wraps XRE_InitEmbedding2/XRE_TermEmbedding +
  do_CreateInstance(NS_WEBBROWSER_CONTRACTID). `render/goanna/test/embed_smoke.cpp`
  + `build/desktop/build-embed-smoke.sh` compile against dist/ and run.
- Result: "engine runtime up → created nsIWebBrowser → destroyed → PASS", exit 0.
- Link recipe (for the daemon, T-016): `libxpcomglue_s.a -lxul libmozglue.a
  -lnspr4 -lplc4 -lplds4 -ldl -lpthread`, `-include mozilla-config.h`,
  `-fno-rtti -fno-exceptions`, frozen API (no MOZILLA_INTERNAL_API).
- Remaining for full R2: repeated create/destroy leak check; explicit profile dir.

## T-019 + load pipeline — page LOADS (this session)
- `render/goanna/test/embed_load.cpp` + `build/desktop/build-embed-load.sh`:
  minimal GTK embedder (invisible window under Xvfb) — creates nsIWebBrowser with
  a small chrome (nsIWebBrowserChrome/nsIEmbeddingSiteWindow/nsIInterfaceRequestor/
  nsIWebProgressListener/weakref), loads a data: URL, pumps the XPCOM event queue
  (`NS_ProcessNextEvent`) + GTK, and observes the full load lifecycle to STATE_STOP.
  Result: "load reached STATE_STOP → PASS", exit 0.
- Lessons baked in: event loop must pump `NS_ProcessNextEvent` (GLib-only didn't
  advance the load); the webBrowser does NOT QI to nsIWebProgress — use
  `AddWebBrowserListener(weakRef, NS_GET_IID(nsIWebProgressListener))`; tear the
  browser/baseWindow down before XRE_TermEmbedding to avoid a shutdown crash;
  desktop needs Xvfb (added to the container).
- Rendering architecture resolved (see render/goanna/PORT-MAP.md): offscreen
  render needs internal gfx/layers (build the backend in-tree) + a display; the
  frozen-API EngineHost still drives navigation/lifecycle.

## Render path — exact blocker pinned (this session)
- Attempted headless paint (map GTK window under Xvfb → expose → capture pixels).
  Crashes in `ClientLayerManager::ForwardTransaction` (null forwarder). Verified
  the GTK widget forces ClientLayerManager (OMTC pref off confirmed, no effect)
  and `XRE_InitEmbedding2` starts no compositor. Full analysis in dead-ends.md.
- So T-020 must **bring up the compositor** (CompositorThread + bridge) — the
  full-app/in-tree path — before paint. embed_load's render attempt is gated
  behind `JIHAD_TRY_RENDER`; default path proves load (exit 0).

## Next (render path)
- T-020: initialize the in-process compositor in the backend (replicate the
  relevant nsAppShell/XRE compositor setup, or build the backend in-tree where
  it's available), then map/paint.
- T-024: read the rendered layer/window back into the shmem buffer
  (`BrowserOffscreenInfo` ARGB32) → T-032 msgPainted. (Pixel-capture via GDK
  already written in embed_load, ready once paint works.)
- Map the observed load lifecycle to YAP msgLoadStarted/Progress/Stopped.
- T-016 (daemon wiring) reuses the embed link recipe; T-011 ARM toolchain parallel.
- T-011 (ARMv7 cross-toolchain) in parallel; reuse the container base. For the
  device, strip libxul and revisit jemalloc/optimize-for-size.
- Bucket-2 files (BrowserServer/Main/BrowserPageManager/Settings) compile once the
  Goanna backend provides `BrowserPage` and the event-loop integration lands.
