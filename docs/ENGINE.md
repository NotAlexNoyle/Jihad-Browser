# The engine: how Jihad uses UXP/Goanna, and how to update it

Jihad Browser renders with **Goanna**, the web engine of the **UXP** platform (Pale Moon's). This
document is for the person who has to change or update that engine. Read it before touching
`third_party/uxp`, `render/goanna/` or the engine parts of `build/`.

Status (2026-10-12): **the engine is on UXP's release `RB_20261006`** (it was on master
`b2594a4` of 2026-06-30, five releases and about 170 upstream changes behind). The update was done
with the procedure in §6, which is now two commands (`build/engine/update-engine.sh`): our 39
commits carried across with git's three-way merge and no conflict, six files changed on both sides
read by hand, `check-engine-api.py` 0 FAIL. The build is a release build in Thumb-2 (§4.5).

---

## 1. Where the engine is

| What | Where |
|---|---|
| Upstream | `https://repo.palemoon.org/MoonchildProductions/UXP`, pristine, at the release tag `UXP_TAG` (`UXP_BASE`) |
| Our branch | fork `NotAlexNoyle/UXP-Jihad`, branch **`jihad/uxp-<UXP_TAG>`** (one per release; `jihad-engine-mods` is the old one, on `b2594a4`), at `UXP_PIN` |
| In this repo | the submodule `third_party/uxp` (gitlink = `UXP_PIN`) |
| The OE (device image) build | `build/webos-oe/recipes-jihad/goanna/goanna_1.0.bb`, `SRCREV` = `UXP_PIN` |
| Pins and version strings | **`build/engine/engine-version.env`** — the one file to edit |

`build/engine/check-engine-pins.sh` fails when the gitlink, the recipe's `SRCREV`, the user
agent (`render/goanna/JihadUserAgent.h`), the three start pages or the bundler's ICU data file
name disagree with `engine-version.env`.

## 2. The rule: the branch is the source of truth

Every change to the engine is **a commit on the engine branch**, one commit per patch. The patch
queue (`build/desktop/patches/0001…`, git mails) is *generated* from those commits, for builds that
start from pristine UXP (the desktop test build, the MPL source record `docs/ENGINE-SOURCE.md`) and
to carry the changes to the next release (§6):

```bash
build/engine/patch-queue.sh --add <commit>   # after committing an engine change: next patch
build/engine/patch-queue.sh --check          # the queue applied to UXP_BASE == UXP_PIN's tree?
```

`--check` applies the whole queue to `UXP_BASE` in a scratch git index and compares the result
with `UXP_PIN`'s tree (seconds, no checkout). It passes today: 39 patches reproduce the pin
exactly. (Until 2026-10-08 the queue had drifted: eight commits and part of the squashed series
were missing, so the desktop build lacked, among others, the switch that turns the Flash child's
own repaint timer off.)

Never author an engine change as a patch file by hand, and never edit `third_party/uxp`
without committing there: the device build compiles the committed tree.

## 3. What our branch changes (the delta)

One commit per patch, 0001–0042 (until 2026-10-12 the first 31 were one squashed commit; they
were split when the engine moved to `RB_20261006`, so each carries across on its own). Full list
with files: `docs/ENGINE-SOURCE.md`.

| Area | Patches | What and why |
|---|---|---|
| Toolchain | 0001, 0002 | GCC 9 `-Wformat-overflow` |
| Offscreen rendering | 0003, 0005, 0012 | Paint into memory with no window: `JIHAD_DISABLE_OMTC`, gfx started inside `XRE_InitEmbedding2`, the PuppetWidget offscreen path and its `jihad_offscreen_*` exports, damage tracking, popups |
| GPU: the Adreno 220 | 0041 | whole-texture uploads on the Adreno 2xx (`CanUploadSubTextures`), BGRA uploads on EGL, a shared pbuffer context for a readback thread |
| GPU compositor (T-502) | 0040, 0042 | `JIHAD_GPU_COMPOSITOR`: an in-process OpenGL compositor on an EGL pbuffer behind the offscreen widget, pulled a frame at a time by the daemon (`jihad_offscreen_compose_start/_poll`), band readbacks from the layer tree's diff, tiled layers by default; the card's scroll applied in the compositor as APZ would (0042: root scroll metadata and the scrollport as a scroll clip for a root scroll frame with a displayport, the displayport set from layout, no intermediate surface for a scaled container, two read bands and a row shift per request, scroll-only frames composed into a band-sized target); off without the env (`docs/RENDERING-OPTIONS.md`, "T-502, the spike") |
| Headless toolkit | 0004, 0006, 0008, 0009 | Run with no X server: GTK guards, `MOZ_WIDGET_TOOLKIT=headless`, plugin IPC types |
| Embedding without a browser window | 0007, 0013, 0014, 0015, 0019, 0036, 0039 | NSS start-up on the main thread, add-on installs and certificate errors reach the embedder, null guards, `.xpi` links, AppCompat matching |
| Branding | 0010, 0011 | Pale Moon / Basilisk branding removed; the AppCompat GUID |
| NPAPI / Flash | 0016–0018, 0020–0029, 0031, 0037, 0038 | Palm's NPAPI host: the `NPEvent` union, input, the child loader, compositing without ImageBridge, frame pull and pacing, CPU boost, the PmLog semaphore shim |
| Media | 0030, 0032–0035 | Pulse sink and stream count for webOS audio; video controls fixes; a YCbCr copy fix |

## 4. Where Jihad touches the engine (touchpoints)

Ordered by how likely an update is to break them. "Silent" means a break would not stop the
build — those are what `check-engine-api.py` exists for.

### 4.1 Our own engine exports — `render/goanna/EngineCompat.h`
The daemon calls 18 `jihad_*` C functions that only our branch defines (offscreen paint,
popups, plugin pacing, NSS start-up, audio). They are declared **only** in `EngineCompat.h`:
seven required (a libxul without them fails to load) and eleven optional (weak; the feature
falls back). At start-up the daemon logs which optional ones the running libxul lacks
(`[jihad-bs] engine: N of 11 optional jihad_* exports present`). A rebase that drops one is
otherwise **silent**.

### 4.2 Engine internals the daemon calls — `render/goanna/*.cpp`
The daemon is built "frozen-API" style (external strings, `libxpcomglue_s.a`, `-lxul`), but it
also uses internals. The positional calls below go through **`render/goanna/EngineCalls.h`**
(since 2026-10-12): one wrapper each, every argument named, the daemon's fixed values set once — a
signature change upstream is one edit there. Headers can move too: since `RB_20261006`,
`mozilla/StyleSheet.h` (reached through `nsIPresShell.h`) holds a `RefPtr<nsIDocument>` with inline
users, so `GoannaRenderPage.cpp` includes `nsIDocument.h` first — the daemon did not build without.
- **High:** `nsIDOMWindowUtils` with long positional argument lists (`sendMouseEventToWindow` 13
  args, `sendKeyEvent`, `sendTouchEventToWindow`, `getViewportInfo`, `nodesFromRect`);
  `nsIPresShell::ResizeReflowIgnoreOverride` (60 app units per pixel assumed); `GetPresShell` /
  `GetContentViewer`; `nsIWidget` into `InitWindow`.
- **High, silent:** `JihadAppInfo` implements all of `nsIXULAppInfo` / `nsIXULRuntime` /
  `nsIPlatformInfo` (`EngineHost.cpp`) — a new attribute upstream breaks it; `nsIClipboardHelper`
  is redeclared by hand with a copied IID (`EngineHost.cpp`).
- **Medium:** `nsIWebNavigation::LoadURIWithOptions` (8 arguments); `XRE_InitEmbedding2` /
  `XRE_NotifyProfile` / `XRE_TermEmbedding`; the directory provider's keys; the prompter,
  `nsITransfer` and helper-app dialog we implement; ~30 `nsIDOM*` interfaces.
- **Low:** observer topics (`http-on-modify-request`, `memory-pressure`).

### 4.3 Engine JavaScript our components depend on — `render/goanna/components/`
- `jihadPersonas.js`: `LightweightThemeManager` (`parseTheme`, `previewTheme`, `currentTheme`,
  `usedThemes`, the pref `lightweightThemes.usedThemes`), `imgITools`, `nsIZipReader`, and the
  **markup of about:addons** (ids, XBL `anonid`s, `gViewController`, the `ViewChanged` event) that
  the Personas pane decorates.
- `jihadInstallPrompt.js`: overrides `amIWebInstallPrompt`; reads `addon-install-*` topics.
- `jihadBundledAddons.js`: `AddonManager.getInstallForFile` / `getAddonByID`.
- All use `Cu.import` of `Services.jsm` / `XPCOMUtils.jsm` (would need porting if UXP moves to
  ES modules).

### 4.4 Engine prefs we set — `packaging/prefs/*.js`, `make-device-bundle.sh`, `EngineHost.cpp`
A renamed pref is **silently** ignored. Two prefs have no engine default on purpose (listed in
the checker): DNT (UXP dropped it; `EngineHost.cpp` sends the header) and `plugin.state.flash`.

### 4.5 The build — `build/desktop/`, `build/webos-oe/`
- mozconfigs (`mozconfig.goanna*`): `--enable-application=xulrunner`, `cairo-headless` (our
  patch 0006), audio options; the device's `mozconfig.goanna-arm` is a **release** build
  (`--enable-release --enable-gold`: per-function sections, the linker's dead-code removal and
  identical-code folding, all off in "developer" mode) in **Thumb-2** (`--with-thumb=yes`), at
  `-Os`. (`--disable-npapi-gtk2` went with `RB_20261006`: upstream no longer offers GTK2 NPAPI to a
  headless toolkit.)
- `build-daemon-arm.sh` assumes the dist layout (`dist/{bin,include,sdk/lib}`).
- `make-device-bundle.sh` reads the ICU data file's name from `engine-version.env` (`ICU_DATA`) and
  hard-codes the dlopened NSS and ffvpx libraries, the GRE file list, and the component manifests.
- `push-engine-update.sh` pushes the engine JS we change as loose files; today it names only
  `XPIProvider.jsm` explicitly — an engine change to other JS/XUL needs a full push.

### 4.6 Version strings
`engine-version.env` holds them; `check-engine-pins.sh` checks the user agent and start pages
say the same. `extensions.guid.appCompatVersion` (56.9, `jihad-addon-prefs.js`) is the Firefox
version add-ons are judged against — it follows the engine's compatibility level, not its commit.

## 5. The tools

| Tool | What it tells you |
|---|---|
| `build/engine/update-engine.sh [TAG] / --finish / --abort / --pins` | moves the engine to an upstream release: the queue carried across with a three-way merge, the overlaps listed, the API checked, every pin rewritten (§6) |
| `build/engine/check-engine-pins.sh` | every pin and version string agrees with `engine-version.env` |
| `build/engine/patch-queue.sh --check` | the patch queue reproduces the engine branch exactly |
| `build/engine/check-engine-api.py [--uxp TREE]` | what in TREE no longer matches what Jihad uses: our exports (§4.1), IDL signatures and implemented interfaces (§4.2) against `engine-api-baseline.json`, the engine JS and about:addons markup (§4.3), prefs (§4.4), mozconfig options and the ICU file (§4.5). Run it on a candidate before building. |

## 6. Updating the engine (procedure)

Do not update the engine casually: it is a full rebuild (an hour or more), a full device push and
a regression pass. When it is time (the owner decides):

1. **Carry the changes across:** `build/engine/update-engine.sh [TAG]` (default: the newest
   `RB_*` release). It refuses unless `third_party/uxp` is at `UXP_PIN` and the queue reproduces
   it (an engine commit not yet in the queue would be left behind: `patch-queue.sh --add` it, then
   `--pins`). It fetches upstream, lists the files upstream **also** changed since our base,
   applies the queue to TAG in a worktree beside this repository with `git am -3`, and runs
   `check-engine-api.py` on the result (a FAIL stops it). A conflict stops it too: resolve in the
   worktree, `git add`, `git am --continue`, until done; `--finish` runs the API check then.
   (`--abort` gives up, until `--finish` has checked the branch out.)
2. **Read the overlaps** it lists: `git -C third_party/uxp diff UXP_BASE TAG -- <file>` against
   what our commit does there. A clean merge can still meet code that now behaves differently
   (2026-10-12: upstream moved the GTK2 NPAPI option under GTK toolkits; our mozconfig named it).
   Fix in the worktree with a new commit, or fold the fix into the commit it belongs to:
   `git commit --fixup <that commit>`, then `git rebase -i --autosquash <TAG>` (`--amend` only
   reaches the last one).
3. **Finish:** `build/engine/update-engine.sh --finish` checks the worktree (the API check too) and
   that `third_party/uxp` has no local changes before it changes anything; then it checks the
   branch out there, writes the queue again from it (so it applies to the new base next time),
   brings every pin along (`engine-version.env`, the OE recipe, the user agent, the three start
   pages and `.gitmodules`' branch) and runs `patch-queue.sh --check` and `check-engine-pins.sh`
   (the gitlink check passes once committed). Stopped part-way, it can be run again.
   Bump `JIHAD_APP_BUILD_ID` in `render/goanna/JihadUserAgent.h` so add-on data is checked again.
4. **Build and test on the desktop:** the desktop engine and the tests in §7
   (`build/desktop/build-*-test.sh`); they guard the touchpoints the device checklist does not
   reach one by one.
5. **Build** for ARM: the engine (`build/webos-oe/build-goanna-arm.sh all` in the build container,
   `docs/DEVICE-BUILD.md`), then the daemon, `make-device-bundle.sh`, and
   `build-variant-ipk.sh` for each variant. Keep the old object directory for a rollback (on ZFS a
   `cp -a` costs nothing).
6. **On the device:** a **full** variant push (`push-variant.sh <variant>`, each in turn) — not
   `push-engine-update.sh`, which sends libxul but not the engine's loose JS, CSS and chrome that
   the release changed with it. Then the checklist in §7. The daemon's start-up line must say all
   optional exports are present.
7. Re-write the checker's baseline (`check-engine-api.py --write-baseline`) once the desktop
   tests and the device checks pass; update `docs/ENGINE-SOURCE.md`; commit the queue, the pins and the gitlink
   together. Pushing the fork branch and the repository is the owner's.

## 7. What guards what

Desktop tests (`build/desktop/build-*-test.sh`, against the desktop engine build):

| Touchpoint | Tests |
|---|---|
| Engine start, page load | `embed-smoke`, `embed-load` |
| Page driving, window utils | `page-driver`, `coordmap`, `link`, `redirect`, `fail`, `rules`, `geo`, `find`, `focus`, `input2` |
| Input, reflow, offscreen zoom (0005) | `input`, `touch`, `coord`, `zoom`, `scroll`, `resize`, `xul` |
| Dialogs, downloads | `dialog`, `download`, `download2` |
| Profile, NSS (0014) | `cookie`, `tls` |
| Prefs, settings page | `services`, `settings2`, `prefsui` |
| App info, AppCompat (0011, 0039) | `xpi-mismatch` |
| History, lifecycle | `nav`, `history`, `leak` |
| Popups (0012) | `popup-probe`, `menupopup-probe` |
| Headless + offscreen exports | `headless-daemon`, `headless-roundtrip` |
| The whole daemon / NPAPI | `adapter-roundtrip`, `test-plugin` |

**Not covered by any desktop test** (device only): Personas and the about:addons pane, video
controls, the Pulse/cubeb path, the YCbCr fix, `.xpi` link handling, Palm NPAPI/Flash.

Device checklist after an engine change (each shell): a page loads and paints; taps and
long-press land; pinch and double-tap zoom; an https page shows TLS 1.3 (howsmyssl.com); a
Flash movie plays with sound; a video plays with sound; an XPI installs (Palefill is
enabled); a Persona installs and paints the toolbar; about:addons, about:config and
about:settings open; printing finds the test sink (`render/goanna/test/ipp-sink.py`).

## 8. Next steps (not done yet)

- A JS shim for the engine modules the components import (LightweightThemeManager,
  AddonManager), and for the about:addons markup the Personas pane relies on.
- One component-install script shared by `build-goanna.sh` and `make-device-bundle.sh` (their
  manifest blocks duplicate each other).
- Derive `push-engine-update.sh`'s loose-file list from the engine branch's diff instead of
  naming `XPIProvider.jsm`.
- Desktop tests for Personas, video controls and the media paths.
- The desktop engine build and its tests (§6 step 4) on `RB_20261006`: the update of 2026-10-12
  was checked on the device only (all three variants); the desktop build still runs the old base.
