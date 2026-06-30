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
| Engine Embedding & Build | 0 (T-010 env ready) | 4 | build container + mozconfig authored; build run pending host with docker |
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

## T-010 build environment (this session)
- Authored pinned build container under `build/desktop/`: `Dockerfile`
  (Ubuntu 18.04 — Python 2.7 / autoconf2.13 / GCC 7 / yasm), `mozconfig.goanna`
  (xulrunner embedding target, GTK2 + basic layers, trimmed), `build-goanna.sh`.
- Reason: this Void host (Python 3.14 / autoconf 2.72 / GCC 14) cannot run the
  ESR-52 `mach`. Container gives the era-correct baseline; UXP source is mounted,
  never vendored. Documented in `docs/TOOLCHAIN.md`.
- **Blocked on**: running the build needs docker/podman on the host. Build run +
  mozconfig validation (xulrunner target may need fallback) is the remaining
  T-010 work.

## Next (need build host / engine)
- Run the container to build Goanna (T-010), then T-013 (embedding runtime) →
  unblocks the integration core (offscreen/nav/input/services).
- T-011 (ARMv7 cross-toolchain) can start in parallel; reuse the same container base.
- Bucket-2 files (BrowserServer/Main/BrowserPageManager/Settings) compile only once
  the Goanna backend provides `BrowserPage` and the event-loop integration lands.
