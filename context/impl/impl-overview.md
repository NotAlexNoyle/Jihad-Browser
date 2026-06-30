---
created: "2026-06-30"
last_edited: "2026-06-30"
---

# Implementation Overview

## Domain Status
| Domain | Tasks Done | Tasks Total | Status |
|--------|-----------|-------------|--------|
| IPC Contract Preservation | 1 (partial) | 5 | T-004 done; T-005/T-006 sources imported, not yet building |
| Licensing & Branding | 3 | 5 | T-001/T-002/T-003 done |
| UI Shell | 3 | 4 | T-007/T-008/T-009 done; T-004(ui) R4 pending |
| Engine Embedding & Build | 1 (T-010 DONE) | 4 | **libxul.so built (1.8G) + embedding headers + xpcshell** in the container; T-012/T-013/T-019 next |
| Offscreen Rendering | 0 | 5 | not started |
| Input Bridging | 0 | 5 | not started |
| Navigation, Loading & Events | 0 | 6 | not started |
| Browser Services | 0 | 5 | not started |
| Desktop Build & PoC | 0 | 4 | not started |
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

## Next (engine builds; integration is now unblocked)
- T-013 (embedding runtime init + per-page nsIWebBrowser instance) against the
  built libxul/headers → then T-019 (event loop) and the Tier-2 integration core
  (offscreen render T-020/T-024/T-032, nav, input, services).
- T-016 (desktop daemon build wiring) can now link against dist/.
- T-011 (ARMv7 cross-toolchain) in parallel; reuse the container base. For the
  device, strip libxul and revisit jemalloc/optimize-for-size.
- Bucket-2 files (BrowserServer/Main/BrowserPageManager/Settings) compile once the
  Goanna backend provides `BrowserPage` and the event-loop integration lands.
