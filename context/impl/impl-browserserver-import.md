---
created: "2026-06-30"
last_edited: "2026-06-30"
---

# Impl: BrowserServer source import (T-004/005/006)

## What was done
Imported 32 engine-agnostic + adaptable source files from the isis-project
BrowserServer (SRCREV e2506f58, Apache-2.0) into
`render/browserserver/Src/`, preserving all Apache-2.0 headers. Classification
and per-file adaptation notes live in `render/browserserver/Src/MANIFEST.md`.

## Verification
- YAP interface (`BrowserServerBase.{h,cpp}`) md5-identical to upstream — the
  contract is frozen (T-004 / cavekit-ipc-contract R1).
- 0/32 imported files missing an Apache header (T-003 / cavekit-licensing R1).
- QtWebKit coupling is confined to the 4 bucket-2 files (BrowserServer.{h,cpp},
  Main.cpp, Settings.{h,cpp}); all other imports are Qt-free.

## Not done (intended boundaries, not failures)
- Nothing compiles yet — no build host wired (T-016) and no engine (T-010).
- Bucket-2 files reference the QtWebKit `BrowserPage`; they need de-Qt + rebind
  to the Goanna-backed `BrowserPage` (the Goanna backend must expose a
  `BrowserPage.h`). This is the Phase-1 integration point.
- Bucket-3 QtWebKit files (BrowserPage, BrowserOffscreenQt, WebKitEventListener,
  BrowserComboBox) intentionally not imported; Goanna backend replaces them.

## Dead ends
- None.
