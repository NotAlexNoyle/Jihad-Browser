# Engine Source & Modifications (MPL-2.0 compliance)

*cavekit-licensing-branding R3 / R4 — engine origin, patches, and branding strip.*

Jihad Browser renders with the **UXP / Goanna** web engine (MPL-2.0). This
document records the engine's origin and every modification made to it, so the
MPL-2.0 obligation to keep modified files under the MPL with their source
available is met.

## Origin (not vendored)

- **Upstream:** UXP — https://repo.palemoon.org/MoonchildProductions/UXP
- **Pinned revision:** `b2594a4ace4556b0a953c079a8c1bc350fc095ec` (master, 2026-06-30)
- The UXP source tree is **not vendored** into this repository. It is built
  out-of-tree from a local checkout mounted into the pinned build container
  (see `build/desktop/`). Engine object/build dirs are git-ignored
  (`build/desktop/out/`). This satisfies cavekit-engine-embedding R4.

## Modifications to the engine (all MPL-2.0, source here)

All engine modifications live in this repository under `build/desktop/` and are
applied at build time by `build/desktop/build-goanna.sh` against the upstream
checkout. The modified UXP files remain under the MPL-2.0; the patches/edits that
produce them are the "source" of those modifications and are reproduced here.

### Toolchain / embedding patches — `build/desktop/patches/`

| Patch | File(s) touched | Purpose |
|-------|-----------------|---------|
| `0001-warnings-no-error-format-overflow.patch` | `warnings.configure` | relax a GCC 9 `-Werror=format` escalation |
| `0002-baseassembler-x64-format-overflow-pragma.patch` | `js/src/jit/x64/BaseAssembler-x64.h` | source pragma for `-Wformat-overflow` (js/src re-appends `-Werror=format`) |
| `0003-gfxplatform-jihad-disable-omtc-env.patch` | `gfx/thebes/gfxPlatform.cpp` | `JIHAD_DISABLE_OMTC` env forces the in-process BasicLayerManager (headless CPU paint) |
| `0004-xre-initembedding-gfx-init.patch` | `toolkit/xre/nsEmbedFunctions.cpp` | init `gfxPlatform` in `XRE_InitEmbedding2` before first paint |

### Branding strip (cavekit-licensing-branding R3) — in `build-goanna.sh`

Applied as an idempotent `sed` step (guard file `.jihad-branding-stripped-v2`).
The UA product is already `Goanna` (`netwerk/protocol/http/nsHttpHandler.cpp`
`mProduct`); this removes the residual Pale Moon / Basilisk / Moonchild strings
so a `strings(1)` scan of the shipped artifacts is clean:

| File | Change |
|------|--------|
| `modules/libpref/init/all.js` | `extensions.blocklist.url` (basilisk-browser.org) → `about:blank`; `captivedetect.canonicalURL` (detectportal.palemoon.org) → example.com; two comment URLs neutralized |
| `docshell/base/nsAboutRedirector.cpp` | `about:credits` target (palemoon.org Contributors) → the Jihad credits URL |
| `toolkit/xre/nsAppRunner.cpp` | dead `MOZ_APP_NAME == "basilisk"/"palemoon"` literals (always false for the `xulrunner` app) → `"n/a"` — behavior identical, literals removed |

**Verification:** after the strip + rebuild, `strings libxul.so`,
`strings jihad-browserserver`, and `goanna.js` each contain **0**
`basilisk`/`palemoon`/`moonchild` strings; the adapter round-trip still passes.

## Trademark note

Per the UXP redistribution terms, an engine build shipped under a name other than
Pale Moon / Basilisk must strip that project's branding. "Jihad Browser" is the
product name of this fork and implies no endorsement by any upstream. See the
top-level `LICENSE` for the full component/license breakdown and the Apache-2.0 ↔
MPL-2.0 compatibility statement.
