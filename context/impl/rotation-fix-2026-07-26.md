---
created: "2026-07-26"
last_edited: "2026-07-26"
---

# Rotation render-break — diagnosis + fix (2026-07-26)

**Symptom (user):** Jihad Browser (Goanna/UXP) cannot rotate between portrait and
landscape without the page rendering breaking (3× tiling + scanlines / shear in
landscape). **Atlas Browser rotates fine.** Kit: cavekit-offscreen-rendering (new R6).

## Root cause

The daemon renders the new orientation CORRECTLY (setWindowSize → `GoannaRenderPage::Resize`
→ `ResizeReflowIgnoreOverride`), AND the adapter's raw blit into `dstBuffer` is itself clean 1:1
(both proven on-device 2026-07-20 via simultaneous `JIHAD_DUMP` + `/dev/fb1` + `adapter.log`
capture — see auto-memory `jihad-input-activation-and-tiling`). The break is in **how LunaCE
composites the raw plugin surface**:

- Jihad's adapter ran with `AdapterBase(instance, true, false)` → **useGraphicsContext=false**,
  so WebKit handed it a raw linear `dstBuffer` and `handlePaint` memcpy-blitted into it.
- **LunaCE reads that raw plugin draw surface at a FIXED ~256px (1024-byte) stride** regardless
  of the real surface width. 768-wide (portrait) = 3×256 → the frame tiles 3× (tile[0] repeated
  / "scanlines"); 1024-wide (landscape) = 4×256 → clean. So the rotation break is orientation-
  specific: rotating into portrait tiles, landscape is fine. No adapter write-stride can fit 768
  content into a 256px read (tried + reverted 2026-07-20).
- The prior "rotation guard" (white frame / hold while `renderedWidth != window.width`) was a
  **band-aid** — it only blanked the page, never fixed the composite.

The raw `dstBuffer` path CANNOT fix this (LunaCE's fixed-stride read of the linear buffer). The
**PGContext path composites THROUGH Piranha's own surface handling** (transform + stride aware),
so LunaCE never misreads a raw buffer — the exact "option (b)" the 2026-07-20 deep dive named as
the real fix. That session believed the Piranha headers were "closed" (why Jihad took the raw
path); this session DISPROVED that (the headers are public in WOCE build-support, Atlas uses this
path in production, and the minimal ABI is reconstructed in PGContext.h/PGSurface.h).

## Why Atlas works (reference solution, Apache-2.0)

Atlas runs `AdapterBase(instance, true, true)` → **useGraphicsContext=true**. WebKit then
hands a Piranha **PGContext*** (and no dstBuffer) that **already carries the card's
rotation/scale transform**. Atlas wraps its offscreen raster as a **PGSurface** (zero-copy)
and `gc->bitblt(...)` **through** the context, so the compositor performs the logical→physical
mapping including rotation. Correct in both orientations, no stride/rotation artifacts.
`PGContext`/`PGSurface` are exported by the device `libWebKitLuna.so` (the adapter is dlopen'd
into the WebKit process; symbols resolve at load — the link is deliberately not `--no-undefined`).

## The fix (this session)

`Jihad-Browser/render/adapter/BrowserAdapter.cpp`:
1. Ctor flag `false → true` (useGraphicsContext).
2. `handlePaint`: added the **PGContext/PGSurface primary branch** (ported from Atlas,
   adapted to Goanna's viewport-sized buffer contract: `renderedX/Y = adapter scroll`,
   `contentZoom = engine zoom`; source-row clamp + dest shrink to avoid past-edge smear).
   The raw dstBuffer blit stays as the **no-context fallback** (desktop/Ubuntu). The stale
   white-frame rotation guard is superseded (left only in the dead fallback).
3. New minimal ABI headers `render/adapter/PGContext.h`, `PGSurface.h` — the Jihad project's
   own reconstruction of the webOS Piranha interface (NOT copied from HP or Atlas), declaring
   only `PGContext::bitblt` (8-coord), `PGSurface::wrap`, `PGSurface::releaseRef`.

## Verification

- **Compiles** clean with the PDK GCC 4.3.3 (`build-adapter-pdk.sh`), links `BrowserAdapterImpl.so`.
- After the Codex fix (below), the ONLY Piranha undefined (load-time) symbols are the two that
  Codex verified byte-for-byte against the real webOS SDK headers (WOCE staged tree):
  - `_ZN9PGContext6bitbltEP9PGSurfaceiiiiiiii` → `PGContext::bitblt(PGSurface*, int×8)`
  - `_ZN9PGSurface4wrapEjjPKhb` → `PGSurface::wrap(unsigned int, unsigned int, unsigned char const*, bool)`
  `PGSurface::releaseRef` is now resolved INLINE (via the modeled `PGShared` base) — no bogus UND.
  No PG vtable/typeinfo orphans; `operator delete`/`__cxa_*` resolve from libstdc++/libc (NEEDED).
- **Device-gated (remaining):** on-device visual confirm of a portrait↔landscape rotate, and
  `dlopen_probe` (RTLD_NOW) to confirm the two PG symbols resolve on the real `libWebKitLuna`.
  Device was offline this session (`novacom -l` → failed to connect).

## Codex 5.6-sol adversarial review (2026-07-26) — findings + disposition

Codex independently located the REAL webOS `PGContext.h`/`PGShared.h`/`PGSurface.h` (WOCE
build-support) and validated the ABI. 8 findings; disposition:

- **F1 CRITICAL — `PGSurface::releaseRef()` bogus symbol → RTLD_NOW load failure.** FIXED.
  releaseRef is an INLINE method of the `PGShared` base (no exported symbol); declaring it on
  PGSurface linked an unresolvable `_ZN9PGSurface10releaseRefEv`. `PGSurface.h` now models
  `PGShared` (inline addRef/releaseRef/refCount + virtual dtor + m_refCount) with `PGSurface :
  public PGShared`. Rebuilt; the UND is gone. This would otherwise have blocked the whole plugin.
- **F4 HIGH — PG path could pass out-of-bounds source rects / bypassed the stale-geometry guard.**
  FIXED. Now clamps the source rect on BOTH axes with conservative `ceil` dest-shrink and a
  both-dimension validity check, and re-adds the unity-zoom stale-width guard (hold last frame
  mid-rotation instead of transforming a wrong-size buffer).
- **F2 HIGH — daemon `setScrollPosition` never updates `mAdapterScrollX/Y`** (buffer advertises
  stale `renderedX/Y`; double-applies scroll; passes zoomed px to the CSS-px engine). DEFERRED —
  pre-existing DAEMON scroll bug, independent of rotation compositing. Logged for a scroll-path
  follow-up; do NOT fold into the rotation change.
- **F3 HIGH — daemon 250 ms in-flight reclaim vs the adapter's zero-copy `PGSurface`.** DEFERRED —
  pre-existing daemon flow-control; the raw path already read the shared buffer during
  `handlePaint`, so zero-copy adds no NEW cross-process window (wrap→bitblt→releaseRef is
  synchronous within one paint). Worth tightening the `returnBuffer` contract later.
- **F5 MEDIUM — raw fallback unreachable on a no-Piranha host.** ACCEPTED/scoped — this NPAPI
  adapter runs ONLY on-device (Piranha always present); desktop round-trips use the separate test
  adapter, not this plugin. The fallback stays for source clarity; not a deployment bug.
- **F6 MEDIUM — partial-damage src origin not scaled under live zoom.** DEFERRED — inherited
  verbatim from Atlas's working zoom path; `inv==1` for rotation so it does not affect this fix.
  Revisit with on-device pinch-zoom + partial-damage testing (changing it risks the zoom path).
- **F7 MEDIUM — dead `handlePaintInFrozenState` would crash if ever wired (QPainter* cast).**
  NOTED — `AdapterBase::PrvPaint` only ever calls `handlePaint`; the frozen helper has no caller,
  so this is latent, not a regression. Left as-is with this note; if freeze paint is ever wired,
  it must composite via PGSurface/PGContext, not QPainter.
- **F8 LOW — scroll indicator absent on the PG path.** NOTED — matches Atlas; cosmetic. Add via
  PGContext fills later if wanted (more PG ABI surface).

## Follow-ups / notes

- F2, F3, F6, F7, F8 above are the tracked follow-ups (none block rotation).
- If a PG symbol fails to resolve on-device (dlopen_probe undefined), adjust the parameter types
  in `PGContext.h`/`PGSurface.h` to match the device lib's exported signatures.
