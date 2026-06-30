---
created: "2026-06-30"
last_edited: "2026-06-30"
---

# Plan Overview

## Build Sites
| Site | File | Tasks | Done | Status |
|------|------|-------|------|--------|
| Jihad Browser port | build-site.md | 48 | 0 | READY |

Phase 1 (desktop x86_64 PoC): all tasks except the device track.
Phase 2 (webOS ARMv7): T-011, T-018, T-046, T-047, T-048.

Next: `/ck:make` to implement (auto-parallelizes Tier-0 tasks). The engine
build (T-010) and cross-toolchain (T-011) are the long poles — start them first.
