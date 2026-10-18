---
created: "2026-10-18"
last_edited: "2026-10-18"
---
# Implementation Tracking: engine-update

Build site: context/plans/build-site.md

| Task | Status | Notes |
|------|--------|-------|
| T-385 | DONE | `render/goanna/EngineCalls.h` (Tier 81) wraps ElementFromPoint, GetViewportInfo, LoadURI, LoadURIWithOptions, NodesFromRect, ResizeReflowIgnoreOverride, SendKeyEvent, SendMouseEventToWindow, SendTouchEventToWindow. `build/engine/check-engine-calls.sh` (new) fails on a direct call in `render/` (proved by a planted call); fixed `render/goanna/test/embed_load.cpp:290`. 19 call sites. |
| T-386 | DONE | `render/goanna/modules/JihadEngine.jsm` (SURFACE JSON + accessors), `components/jihadPersonas.js` and `jihadBundledAddons.js` through it, installed by `make-device-bundle.sh`, `build-goanna.sh`, `push-engine-update.sh`; `check-engine-api.py` derives JS_NEEDS from the surface and adds check 4b; `test/jihad-engine-shim-test.js`; device Enyo about:addons + Personas view verified (screenshots). Not in scope: `packaging/prefsui/content/addons.css` also names about:addons ids (a style sheet, not the components). |
