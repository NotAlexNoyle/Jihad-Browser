---
created: "2026-06-30"
last_edited: "2026-06-30"
---

# Dead Ends / Hard-won findings

Failed approaches and their root causes, so they are not retried.

## ✅ RESOLVED (2026-06-30): headless rendering now WORKS

The "rendering needs a full in-tree rewrite" conclusion below was **too
pessimistic**. Headless rendering works with **two small engine patches** +
Xvfb, no backend rewrite:
- **patches/0003** — `gfxPlatform::UsesOffMainThreadCompositing()` honors
  `JIHAD_DISABLE_OMTC` env → force the in-process **BasicLayerManager** (CPU
  paint), sidestepping the compositor entirely (fixes crash #1 below).
- **patches/0004** — call `gfxPlatform::GetPlatform()` in `XRE_InitEmbedding2`
  so `gfxVars`/`gfxConfig` are initialized before first paint (fixes the
  `gfxVars::UseXRender` null crash that appeared after 0003).

Result: `embed_load` (default) paints a real page into the offscreen GTK window
and GDK-captures it — `docs/jihad-render-proof.png` (blue #224488 bg + white
heading, 728k non-white px, exit 0). The detailed crash-progression notes below
are kept for history; they are no longer blockers.

---


## Headless rendering via the minimal external embedder (frozen API) — does NOT work for paint

**What was tried:** drive paint by giving the `nsIWebBrowser` (created via the
frozen `XRE_InitEmbedding2` embedding API) a real GTK top-level window under
Xvfb, mapping it (`gtk_widget_show_all`) so the first expose paints, then reading
the window pixels back with GDK.

**Result:** segfault on the first expose, every time:
`mozilla::layers::ClientLayerManager::ForwardTransaction` dereferences a null
forwarder (gfx/layers/client/ClientLayerManager.cpp:374) via
`nsWebBrowser::PaintWindow` → `nsWindow::OnExposeEvent`.

**Root cause:** on this UXP/Linux (GTK2) build the widget **always** uses a
compositor-backed `ClientLayerManager`, which forwards paint transactions to a
compositor over IPC. The pref `layers.offmainthreadcomposition.enabled=false`
(verified read: "OMTC enabled pref = 0") does **not** switch it to the
in-process `BasicLayerManager` — the GTK widget forces compositing. And
`XRE_InitEmbedding2` does **not** bring up the in-process compositor
(CompositorThread + CompositorBridgeChild/Parent) that the ClientLayerManager
needs, so its forwarder is null → crash.

**Conclusion / what to do instead:** rendering requires the full compositor
bring-up that normal app startup (XRE_main / nsAppShell) performs, not the
minimal `XRE_InitEmbedding2` embedding path. The render backend must either
(a) be built/run with the full app + compositor initialization (the in-tree
component route in render/goanna/PORT-MAP.md), or (b) explicitly initialize the
compositor infrastructure before creating the widget. The frozen-API
`EngineHost` remains correct for **navigation/lifecycle** (load works headless —
see embed_load); only **paint** needs the compositor.

**Still works (not a dead end):** `XRE_InitEmbedding2` + `nsIWebBrowser` +
event loop + `AddWebBrowserListener` → a page **loads to STATE_STOP** headless
(embed_load default path, exit 0). Just don't map/paint the window without a
compositor. The render attempt is gated behind `JIHAD_TRY_RENDER`.

## Headless rendering via the print path (nsIWebBrowserPrint → PDF/PS) — HANGS

**What was tried:** after the page loads, render it to a file via the print path
(which uses its own print device context, not the screen compositor):
`nsIWebBrowserPrint::Print(settings, nullptr)` with `printToFile=true`,
`outputFormat=kOutputFormatPDF`, `printSilent=true`, `showPrintProgress=false`.
Hope: bypass the ClientLayerManager screen-paint crash entirely.

**Result:** `Print()` **hangs** — it spins an internal nested event loop that
never returns in a headless embedder (no output, container had to be killed).
Our own pump loop after `Print()` is never reached.

**Likely cause:** the print engine waits on UI/presentation state (print
progress, a focused presentation, or a printer/params round-trip) that never
arrives headlessly. Not pursued further.

**Conclusion:** reinforces the same verdict — there is no quick frozen-API render
path from the minimal embedder. Rendering goes through the in-tree/full-app
compositor route. The render attempt in embed_load is gated off by default; the
print path was removed (kept this note instead of dead code).

## Why the screen-paint crash can't be fixed from the external embedder (deep diagnosis)

Traced to bedrock. Two coupled internal init-ordering problems:

1. **Can't force BasicLayerManager.** `nsBaseWidget::ShouldUseOffMainThreadCompositing`
   = `gfxPlatform::UsesOffMainThreadCompositing()` =
   `!gfxPrefs::LayersOffMainThreadCompositionForceDisabled()` (GTK). That gfxPref
   is `DECL_GFX_PREF(Once, ...)` — snapshotted **once** at gfx init. In the
   `XRE_InitEmbedding2` flow gfx initializes around/before greprefs (goanna.js)
   apply, so the snapshot keeps the **default (false)** even though the live pref
   service correctly reports `force-disabled=1` (verified at runtime:
   `OMTC.enabled=0 force-disabled=1 accel.force=0`, yet ClientLayerManager is
   still chosen). So OMTC stays on.

2. **Compositor never connects.** With OMTC on, the widget uses a
   `ClientLayerManager` that needs a compositor connection. The first paint
   crashes in `ClientLayerManager::ForwardTransaction` dereferencing a null
   `mTransactionIdAllocator` (gfx/layers/client/ClientLayerManager.cpp:636) —
   the in-process CompositorBridge for the widget was never established by the
   minimal embedder.

**Net:** rendering requires controlling gfx/pref init ordering (set the Once
pref before gfx init) **or** standing up the widget compositor — both are
internal, in-tree concerns. The frozen-API external embedder cannot reach them.
This is the concrete task for T-020: build the render backend in-tree (or extend
the embedding init sequence in UXP) so the layer/compositor path is set up before
paint, then read the BasicLayerManager/compositor buffer into the shmem.

## Other notes
- `-Wno-error=format-overflow` via warnings.configure does not reach js/src
  (js appends `-Werror=format` after the warnings list) — fixed with a source
  pragma instead (patches/0002). See git log T-010.

## webOS audio is POLICY-gated — a working cubeb/ALSA/pulse stream is still silent (2026-08-15)

The cubeb ALSA backend is built into ARM libxul and DEMONSTRABLY reaches pulseaudio on device:
`<audio>` reports `err=0 readyState=4`, and `pactl list` shows our sink-inputs at
`s16le 2ch 44100Hz` ("ALSA Playback"). Yet no sound. The pulse sinks stay `State: SUSPENDED` and
our sink-inputs read `Volume: 0% / -inf dB`. The hardware PCM (`/proc/asound/card0/pcm0p`) is owned
by Android's `mediaserver` HAL, and webOS un-suspends/routes a sink only when its audio POLICY
manager grants an app a playback "scenario" over LunaService (`com.palm.audio` / audiod). Do NOT
chase this as a cubeb, ALSA-config, or decoder bug — the pipe works; the app is unmuted only after a
policy request. Same family as "webOS Flash isn't a generic NPAPI plugin". Corollaries proven the
same day: `paplay --device=pmedia` from a shell HANGS identically; `pactl suspend-sink pcm_output 0`
returns `Failure: Invalid argument`; and killing `mediaserver` does not free the PCM (it respawns
and re-grabs). Also: the "bundle Jessie's libasound" plan is wrong for this hardware — the Jessie
1.0.28 lib gives `err=3` against the device's 0.9.8 pulse plugin ABI, the DEVICE's own
`/usr/lib/libasound.so.2` gives `err=0`, and cryptofs forbids the bundling symlink anyway.

## A `file://` XPI navigation does NOT trigger the install flow on device (2026-08-15)

Navigating the card to `file:///tmp/foo.xpi` loads it as a document (`STOP`), with no
`InstallTrigger`, no `addon-install-*` — so the T-103 install-refusal observer is never reached that
way. The 2026-08-03 device XPI proof used the card's own install affordance / an http-served
trigger. To exercise the install path on device, serve the XPI over http or drive the card's install
UI; a bare `file://` navigation is not an install trigger. (Desktop harness reproduces the observer
because it drives the install through AddonManager directly.)

## `javascript:` URLs are dead in ALL chrome documents, not just XUL ones (2026-08-15)

The 2026-08-04 measurement ("javascript: does not execute in a chrome document",
GoannaRenderPage.cpp:749-755) was taken on about:addons (XUL). Re-measured 2026-08-15 on a chrome
HTML document (about:preferences): same silent no-op. So the dead end covers chrome documents
generally, whatever the markup language. Consequence nobody had noticed: anything the daemon
drives THROUGH a javascript: URL is a silent no-op on a chrome page — concretely
`GoannaRenderPage::ScrollTo` (drives `window.scrollTo` that way). This cost one full harness run:
a button at y≈854 in a 768-tall viewport could not be scrolled to, the tap landed on `<null>`, and
it read as "the button does nothing". Use the C++ `Resize()` path (or element-relative taps) on
chrome pages; do not "fix" it by trying another javascript: variant.

## Stale desktop profile fakes a prefs regression (2026-08-15)

`build/desktop/out/.jihad/default/profile/prefs.js` accumulates `user_pref` lines from old runs
(e.g. `browser.cache.disk.capacity=358400` from before `smart_size` was pinned off) that OVERRIDE
the shared platform prefs and read as "the shared file did not take". Any prefs assertion on the
desktop harness must run with a fresh, dedicated `JIHAD_STATE_DIR` (both 2026-08-15 runners
`rm -rf` theirs per run). Same family as "an empty log passes every check": the instrument's own
leftovers impersonate the defect.

## input2_test holdAt/insert pixel checks — desktop harness paint (pre-existing, 2026-07-18)

input2_test reports `holdAt green=-1` / `insertStringAtCursor green=-1` = ZERO
`painted shmid` emissions across the whole run (harness-wide, not logic-specific).
Confirmed NOT a regression from the 2026-07 loading-screen/focus work: `git diff
8b993a1..HEAD -- BrowserPageGoanna.cpp` touches none of paintToSharedBuffer /
attachShm / jihadShmResolve / mActiveKey / mBufSize. link_test (navigation) and
the new focus_test (VKB emissions) both PASS against the same desktop libxul, so
navigation + input + focus logic are fine — only the pixel-readback paint in the
desktop Xvfb harness is dead. Prime suspect: the desktop libxul in
build/desktop/out is stale vs the current UXP tree (this session only rebuilt ARM
libxul). Rebuild desktop libxul before trusting input2/scroll/geo pixel asserts.
Does not affect device (ARM libxul is current + the daemon renders on /dev/fb1).

## OE bitbake rebuilds the engine from a DRIFTED patch queue, not the working tree (2026-08-16)

The `goanna` recipe cloned pristine UXP `b2594a4` (SRCREV) and rebuilt the entire jihad engine
delta with `do_apply_jihad_patches` (`patch -p1 --forward < "$p" || true`). Two silent traps:
(1) the desktop patch queue in `build/desktop/patches/` has DRIFTED from pristine UXP — 6/13 hunks
of the PuppetWidget offscreen-zoom patch, plus PuppetWidget.h / moz.build / three gtk files, no
longer apply; (2) the `|| true` swallowed every failure, so the OE clone compiled a HALF-patched
`PuppetWidget.cpp` (popup code present, its `nsXULPopupManager`/`nsIFrame`/`nsLayoutUtils` includes
missing) and `do_compile` died with "not declared" errors. The desktop HOST build (build-goanna-arm.sh)
hid this completely: it compiles the already-patched `third_party/uxp` WORKING TREE, where the mods
live as uncommitted dirt, and its own patch loop is dry-run-gated so it skips them as "already applied".
So "the host build compiles this file fine" is NOT evidence the OE build will — they build different
source (committed pristine + drifted queue vs. dirty working tree). Do NOT chase the compile error by
adding includes to the working-tree file: the OE clone never sees the working tree. The fix is to make
the OE source == the host source: commit the working-tree delta durably (branch `jihad-engine-mods`,
`07259a27`), point SRCREV there with `nobranch=1`, and dry-run-gate the apply loop. Meta-lesson: a
`|| true` on a patch loop turns "patch no longer applies" into a silent half-apply that only shows up
as a compile error 300 tasks later.
