# UXP / Goanna engine — submodule + patch queue

Jihad renders with the **UXP / Goanna** engine (Pale Moon's Gecko-ESR-52 hard fork).
The engine is **not forked and not vendored** — it is a pristine **git submodule** pinned
to an exact upstream commit, and every Jihad modification lives as an ordered **patch**
applied to that pristine tree at build time. This keeps upstream updates a clean, explicit
operation.

## Layout

| Thing | Where |
|-------|-------|
| Engine source | `third_party/uxp` — submodule → `https://repo.palemoon.org/MoonchildProductions/UXP`, pinned to a specific commit (currently `b2594a4ace…`). **Never edited in place.** |
| Jihad engine mods | `build/desktop/patches/00NN-*.patch` — one themed patch per subsystem, each a clean diff vs the pinned upstream (distinct files → they apply independently, in any order, no drift). |
| Patch applier | `build/desktop/build-goanna.sh` applies `patches/*.patch` idempotently (`patch -p1 --forward`, skips already-applied). |

## The patch queue (what each patch is)

- `0001-js-format-overflow` — the real libxul blocker: js/src re-escalates `-Werror=format`; source pragma.
- `0002-configure-warnings` — minor warnings.configure tweak.
- `0003-gfx-init-and-omtc` — force in-process BasicLayerManager + init gfxPlatform before first paint.
- `0004-gtk-headless-guards` — run libxul with no X server (headless offscreen render).
- `0005-puppetwidget-offscreen-zoom` — the offscreen render core: `JihadRenderDocument` into `mDrawTarget`, null-TabChild guards, `RENDER_CARET`, sticky dirty flag, **and pinch/fit zoom + document-relative visual-viewport pan**.
- `0006-headless-toolkit` — `MOZ_WIDGET_TOOLKIT=headless` (`gfxPlatformHeadless` + `widget/headless/`), a spike; Stage-1 GTK-bundled is the shipping fallback.
- `0007-nss-mainthread-marshal` — NSS init main-thread marshal fast-path (https-heavy pages).
- `0008-plugin-headless-nativehandle` — `NativeWindowHandle` for the headless/PGContext platform.
- `0009-misc-headless` — message_loop + XRE dir provider headless fixes.
- `0010-branding-strip` — licensing/branding (R3): strip Pale Moon/Basilisk/Moonchild pref URLs + about:credits target. (Was an inline `sed` in build-goanna.sh; now a patch so it is captured in the queue.)

Each patch is generated as `git -C third_party/uxp diff <pinned> -- <that patch's files>`. The
partition (one file → one patch) is what lets them apply independently.

**There are 29 patches now, not ten** — an earlier version of this paragraph said ten and was not
updated as the queue grew. Whether a fresh checkout plus the queue still reproduces a clean build
is an acceptance criterion, tracked in `context/kits/cavekit-engine-embedding.md` R1.

**Authoring a new patch is the part that goes wrong.** Several patches touch the same files, so by
the time you author patch N those files already carry N-1 patches: diffing against the working tree
yields nothing, and diffing against the pinned revision folds in every predecessor. Use
`build/desktop/patches/make-0029.sh` as the template — it reconstructs the correct baseline
(`git show <pinned>:` plus only the EARLIER patches that touch the same files) and diffs against
that.

## First build / fresh clone

```bash
git clone --recursive git@github.com:NotAlexNoyle/Jihad-Browser   # all submodules
# or, after a plain clone:
git submodule update --init            # third_party/{uxp,mochi,mochi-sampler,enyo-layout}

# Palm PDK (proprietary; gcc 4.3.3 + device sysroot for the NPAPI adapter) — not vendored:
build/webos-oe/fetch-pdk.sh <path/to/palm-sdk_3.0.5-*_i386.deb>   # -> build/webos-oe/pdk/
```
Other submodules: `third_party/mochi` + `third_party/mochi-sampler` (github.com/webOSArchive,
the Mochi UI `.ipk`) and `third_party/enyo-layout` (github.com/enyojs/layout @ 2.5.2, the Enyo
layout lib). They are pristine (no patches) — bumping their pin is just a checkout. The build
scripts derive their own paths (no hardcoded workspace path) and find the PDK at
`build/webos-oe/pdk/` or via `PDK_ROOT`.
The build (`build/desktop/build-goanna.sh`, mounted at `/src/uxp`) applies the patch queue to
the submodule checkout. The submodule tree becomes "dirty" during a build (patches applied
in-tree) — that is expected for patch-based vendoring; `git -C third_party/uxp checkout -- .`
restores it to pristine.

> Build invocations mount **`third_party/uxp`** as `/src/uxp` (previously the sibling `../UXP`).
> The ARM build (`build-goanna-arm.sh`) reuses the patched tree, so run the desktop patch-apply
> (or a build) once against the submodule first.

## Updating from upstream (the whole point)

```bash
cd third_party/uxp
git fetch origin
git checkout <new-upstream-commit>          # bump the pin
cd ../..
# re-apply the queue; fix any patch that upstream now conflicts with:
for p in build/desktop/patches/*.patch; do
  git -C third_party/uxp apply --3way "$p" || echo "REBASE NEEDED: $p"
done
# for any conflicted patch: resolve in-tree, then regenerate it:
#   git -C third_party/uxp diff <new-pin> -- <that patch's files> > build/desktop/patches/00NN-*.patch
git -C third_party/uxp checkout -- .        # restore pristine
git add third_party/uxp build/desktop/patches   # commit the new pin + refreshed patches
```
Only patches whose files upstream also changed need attention; the rest keep applying. There is
no fork to merge and no vendored engine bloating this repo — just a moved pin + a patch refresh.
