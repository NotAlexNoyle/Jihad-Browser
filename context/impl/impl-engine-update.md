---
created: "2026-10-18"
last_edited: "2026-10-18"
---
# Implementation Tracking: engine-update

Build site: context/plans/build-site.md

| Task | Status | Notes |
|------|--------|-------|
| T-385 | DONE | `render/goanna/EngineCalls.h` (Tier 81) wraps ElementFromPoint, GetViewportInfo, LoadURI, LoadURIWithOptions, NodesFromRect, ResizeReflowIgnoreOverride, SendKeyEvent, SendMouseEventToWindow, SendTouchEventToWindow. `build/engine/check-engine-calls.sh` (new) fails on a direct call in `render/` (proved by a planted call); fixed `render/goanna/test/embed_load.cpp:290`. 19 call sites. |
