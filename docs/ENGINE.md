# The engine: how Jihad uses UXP/Goanna, and how to update it

Jihad Browser renders with **Goanna**, the web engine of the **UXP** platform (Pale Moon's). This
document is for the person who has to change or update that engine. Read it before touching
`third_party/uxp`, `render/goanna/` or the engine parts of `build/`.

Status: the groundwork for an update is in place (2026-10-08): one place for the pins, one list
of the engine's own exports, a patch queue that provably equals the engine branch, and a checker
that says what a candidate engine would break. **The engine has not been updated.** It is 76
upstream commits behind as of that date, a few of which touch the same files as ours (§6).

---

## 1. Where the engine is

| What | Where |
|---|---|
| Upstream | `https://repo.palemoon.org/MoonchildProductions/UXP`, pristine, at `UXP_BASE` |
| Our branch | fork `NotAlexNoyle/UXP-Jihad`, branch **`jihad-engine-mods`**, at `UXP_PIN` |
| In this repo | the submodule `third_party/uxp` (gitlink = `UXP_PIN`) |
| The OE (device image) build | `build/webos-oe/recipes-jihad/goanna/goanna_1.0.bb`, `SRCREV` = `UXP_PIN` |
| Pins and version strings | **`build/engine/engine-version.env`** — the one file to edit |

`build/engine/check-engine-pins.sh` fails when the gitlink, the recipe's `SRCREV`, the user
agent (`render/goanna/JihadUserAgent.h`), the three start pages or the ICU data file name
disagree with `engine-version.env`.

## 2. The rule: the branch is the source of truth

Every change to the engine is **a commit on `jihad-engine-mods`**. The patch queue
(`build/desktop/patches/0001…`) is *generated* from those commits, for builds that start from
pristine UXP (the desktop test build, and the MPL source record `docs/ENGINE-SOURCE.md`):

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

The first commit, `07259a27f0`, is the original patch series squashed (patches 0001–0031); the
rest are one commit each (0032–0039). Full list with files: `docs/ENGINE-SOURCE.md`.

| Area | Patches | What and why |
|---|---|---|
| Toolchain | 0001, 0002 | GCC 9 `-Wformat-overflow` |
| Offscreen rendering | 0003, 0005, 0012 | Paint into memory with no window: `JIHAD_DISABLE_OMTC`, gfx started inside `XRE_InitEmbedding2`, the PuppetWidget offscreen path and its `jihad_offscreen_*` exports, damage tracking, popups |
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
also uses internals:
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
  patch 0006), `--disable-npapi-gtk2`, audio options.
- `build-daemon-arm.sh` assumes the dist layout (`dist/{bin,include,sdk/lib}`).
- `make-device-bundle.sh` hard-codes the ICU data file (from `engine-version.env`), the dlopened
  NSS and ffvpx libraries, the GRE file list, and the component manifests.
- `push-engine-update.sh` pushes the engine JS we change as loose files; today it names only
  `XPIProvider.jsm` explicitly — an engine change to other JS/XUL needs a full push.

### 4.6 Version strings
`engine-version.env` holds them; `check-engine-pins.sh` checks the user agent and start pages
say the same. `extensions.guid.appCompatVersion` (56.9, `jihad-addon-prefs.js`) is the Firefox
version add-ons are judged against — it follows the engine's compatibility level, not its commit.

## 5. The tools

| Tool | What it tells you |
|---|---|
| `build/engine/check-engine-pins.sh` | every pin and version string agrees with `engine-version.env` |
| `build/engine/patch-queue.sh --check` | the patch queue reproduces the engine branch exactly |
| `build/engine/check-engine-api.py [--uxp TREE]` | what in TREE no longer matches what Jihad uses: our exports (§4.1), IDL signatures and implemented interfaces (§4.2) against `engine-api-baseline.json`, the engine JS and about:addons markup (§4.3), prefs (§4.4), mozconfig options and the ICU file (§4.5). Run it on a candidate before building. |

## 6. Updating the engine (procedure)

Do not update the engine casually: it is a full rebuild (hours), a full device push, and a
regression pass. When it is time:

1. **Look first.** `git -C third_party/uxp fetch origin` and read `git log
   UXP_BASE..origin/master`. Note commits touching files we change (`git diff --stat
   UXP_BASE..origin/master -- $(git diff --name-only UXP_BASE UXP_PIN)`); as of 2026-10-08 these
   were `old-configure.in` (`--disable-npapi-gtk2`), `PluginInstanceChild.{cpp,h}` (the
   `gtk2xtbin` guard) and `toolkit/moz.configure` (skia).
2. **Rebase our branch** onto the new upstream commit in the fork:
   `git rebase --onto <new> UXP_BASE jihad-engine-mods`. Resolve per commit; the commit
   messages and patch headers say why each change exists.
3. **Check the candidate before building:** `build/engine/check-engine-api.py --uxp
   third_party/uxp`. Every FAIL is a touchpoint to fix (in our code, or in the rebased branch).
4. **Update the pins** in `engine-version.env` (`UXP_BASE`, `UXP_PIN`, `UXP_SHORT`, and
   `GOANNA_VERSION` / `ICU_DATA` if they changed), then the gitlink, the recipe `SRCREV`, the user
   agent and the start pages until `check-engine-pins.sh` passes.
5. **Regenerate the queue** from the rebased commits (`git format-patch UXP_BASE..UXP_PIN`
   into `build/desktop/patches/`, keeping the numbering), then `patch-queue.sh --check`.
   Update `docs/ENGINE-SOURCE.md`.
6. **Build** the engine and daemon for ARM (`docs/DEVICE-BUILD.md`), and the desktop test build;
   run the tests in §7. Re-write the checker's baseline (`--write-baseline`) only once the build
   and tests pass on the new engine.
7. **On the device:** full engine push to all three variants (`push-engine-update.sh` without
   `--daemon-only`), then the checklist in §7. The daemon's start-up line must say all optional
   exports are present.
8. Push the fork branch and the repo (the owner does pushes).

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

- Wrap the high-risk `nsIDOMWindowUtils` / reflow / `LoadURIWithOptions` calls behind small
  functions in one file, so a signature change is one edit.
- A JS shim for the engine modules the components import (LightweightThemeManager,
  AddonManager), and for the about:addons markup the Personas pane relies on.
- One component-install script shared by `build-goanna.sh` and `make-device-bundle.sh` (their
  manifest blocks duplicate each other).
- Derive `push-engine-update.sh`'s loose-file list from the engine branch's diff instead of
  naming `XPIProvider.jsm`.
- Desktop tests for Personas, video controls and the media paths.
