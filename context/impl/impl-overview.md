---
created: "2026-06-30"
last_edited: "2026-08-15"
---

# Implementation Overview

> **STALE AS A STATUS DOCUMENT — read `docs/PICKUP.md` and `context/kits/` first (2026-08-10).**
> This file stops at 2026-08-03 and therefore predates all of the Flash work: NPAPI windowless,
> input, audio, the CPU-governor fix, and the whole frame-rate/pacing investigation. Where it
> disagrees with a kit criterion or with PICKUP, **it is wrong** — it is kept for the per-session
> engineering detail it records (the causes, the traps, the measurements), not for its status.
> It also names `impl-NEXT-AGENT-START-HERE.md`, which was folded into `docs/PICKUP.md` and
> deleted; PICKUP is the only current handoff.
>
> The traps recorded below are still true and still load-bearing — the `novacom` stdin-EOF output
> loss and the WebAppMgr JS cache in particular.

## 2026-08-15 — HOST-SIDE CLOSE-OUT WAVE (no device all session)

Cavekit loop against `context/plans/build-site.md`; per-task detail in `context/impl/loop-log.md`
Iteration 3 and the new impl files it names (impl-yap-codegen, impl-audio-backend, impl-cert-store,
impl-touch-events, impl-toast-channel, impl-device-build §2026-08-15). Highlights:
- **ipc-contract R1 fully closed** — YapCodeGen vendored + patched; regeneration reproduces the
  shipped wire with zero hand edits; space-hang trap fixed.
- **Engine audio output exists in the build** — cubeb ALSA backend compiled into ARM libxul
  (`--enable-alsa`, Jessie alsa-lib staged); device listen pending; MP3/AAC/H.264 decoders still off.
- **Cert-store integration (T-135) landed and desktop-proven** — real PEM certFile, honest
  Palm-ordinal error mapping off `nsISSLStatus` (the emitted nsresult was a transport-class lie),
  three-way trust with both-ends session-serial sweep; platform writes via dlopen'd
  libPmCertificateMgr, degrade-safe.
- **Touch events (T-120) code-complete behind `JIHAD_TOUCH_EVENTS=1`** — 28-byte header re-staged,
  fence out, double-activation suppressor; two real bugs fixed en route. All ARM artifacts rebuilt;
  .ipks 1.0.4 bundle libasound (older ipks won't start the daemon).
- **Desktop verifications** — notificationbox attach/dismiss PASS on the offscreen widget path
  (gre-widgets R5 first criterion `[x]`); XPI mismatch alert fires with discriminating control;
  eleven shared-file pref rows observed (R7 second criterion `[x]`).
- **Opal config corrected** — kernel string pinned `2.6.35-palm-shortloin` (board name; `opal` is
  the product); prebuilt-input manifests landed (187-deb sysroot + adapter-deps).
- Keyboard arbitration: observable rebuilt around the address-bar screenshot; latch found to be
  DOUBLE-tap-set only, no latch log compiles (T-124 rescoped).
- Everything still open is device-, hardware-, or human-decision-gated; see build-site rows.

## 2026-08-03 (second session) — CARD DEV LOOP RESTORED, SELECT POPUP DONE ON ALL THREE (READ FIRST)

Full handoff: **`impl-NEXT-AGENT-START-HERE.md`**. Commits 512ede2f, e3de7d8a, 322f26bd, 80b7fab9,
ca4dcc52 (+ doc commits).

- **Card JS dev loop RESTORED** — `build/webos-oe/push-card-js.sh`. Two independent causes had
  broken it: (a) **`novacom run` discards output that arrives after the host's stdin hits EOF**, so
  every slow reply (anything crossing the Luna bus) came back EMPTY with exit 0 — this is what
  looked like "luna-send blackouts", dead `applicationManager/running` queries and "`enyo.log`
  stopped reaching `palm-log`"; hold stdin open (`sleep 4 | novacom run …`). (b) The **WebAppMgr
  in-process JS cache genuinely serves a stale build** after close+relaunch, md5-proven; a
  LunaSysMgr restart per cycle busts it. The script now proves every reload with a per-push stamp
  in the device log — nothing else counts.
- **`<select>` popup DONE on all three variants, user-confirmed.** The "empty popup" was a JSON
  field-name mismatch, not a rendering problem: the stock `enyo.WebView` wrapper consumes
  `onOpenSelect` itself and expects the isis `items[].text`/`isEnabled` (+ `selectedIdx`) shape,
  while the daemon wrote `label`/`enabled`. Daemon fixed; the Enyo app's own popup code deleted as
  unreachable; Mochi given its own overlay list; **Mojo needed no app code at all** (the system
  framework implements the whole path). Popup is anchored under the tapped control from a
  daemon-supplied rect. Opus review hardened the apply path (disabled / `<optgroup>` /
  out-of-range / no-op guards; fail-closed popup file; process-global ids).
  `impl-select-popup-2026-08-03.md`.
- **Mojo chrome reworked** (user-driven): title row dropped; command menu gained new card /
  history / share (history is card-local — this package registers no db8 kinds); custom icons are
  32×64 two-frame sprites. Its toolbar had overflowed the screen because **the card WebKit ignores
  unprefixed `box-sizing`** — a platform constraint now recorded in the kits, AGENTS.md and the
  variant READMEs. Diagnosed by logging real widths from inside the card, not by eyeballing a
  screenshot.
- **Start pages unified** across all three shells (logo, "Jihad Browser", engine line, hint).
- Open on this track: the `<menupopup>` overlay composite (now the top priority) and `<optgroup>`
  header rows in `<select>` lists (needs a reply-index remap).

## 2026-08-03 (first session) — SCROLL DONE, THREE VARIANTS LIVE, SELECT-POPUP CARD-BLOCKED

Full handoff: **`impl-NEXT-AGENT-START-HERE.md`**. This session:
- **Scrolling FIXED, user signed off** — overscan region paint with honest per-frame geometry,
  ≤2048-row SGX cap, direction-biased headroom, settle-gate removed, echo suppression, fit-zoom
  floored to identity blit, coverage-aware repaint + pan-cadence refresh. Opus adversarial review
  (16 findings, 3 blockers) all fixed pre-deploy. `impl-scroll-overscan-2026-08-02.md`.
- **Long-press WORKS** (user-confirmed) — the daemon `asyncCmdHitTest` gate was a stub; real
  hit-test round-trip added. **Input coord mapping fixed** — doc→viewport at the input drain.
- **All three variants LIVE** (mochi re-deployed, mojo first run); cold boot auto-starts all three,
  `device-independence-test.sh check` 24/24 → device-build **R7 verified**. `push-variant.sh` +
  `push-engine-update.sh` are the autonomous novacom deploy routes.
- **Apps renamed** Jihad Enyo/Mochi/Mojo; **Jihad logo on `about:`/`about:jihad`**; Enyo start page
  follows VKB/orientation.
- **`<select>` popup: daemon done + device-verified; card list empty, BLOCKED on a broken card JS
  dev-loop** (frozen WebAppMgr cache + `enyo.log`→`palm-log` stopped). `impl-select-popup-2026-08-03.md`.
- **`<menupopup>` DIAGNOSED** (separate 0x0 display root) — `impl-menupopup-2026-08-02.md`. **XPI
  install prompt authored, UNWIRED** (card-loop dependency).

## 2026-07-31 — THREE INDEPENDENT PACKAGES, RUN IN PLACE FROM CRYPTOFS

Two user decisions reshaped packaging: **each front-end must work entirely on its own**, and the
package must be a **good webOS citizen** that writes nothing to `/media/internal`. Both are now
requirements — cavekit-device-build.md **R7** and **R8** — with the naming contract in
`../plans/plan-variant-identity.md` and the full record in `independence-citizen-2026-07-31.md`.

**Enyo variant VERIFIED ON DEVICE, 0 failures** (commit e36c8cc): installs from a self-contained
`.ipk`, deploys its own shim + impl + upstart job, serves its own socket, and runs its daemon **out
of the app's own cryptofs `deviceroot`** — with every stock checksum unchanged and `/media/internal`
clean. Atlas was checked at the user's request and has no way around the one unavoidable rootfs
write (webOS's WebKit only scans `/usr/lib/BrowserPlugins` at boot); R8 bounds and reverses that
footprint instead, and Atlas's run-in-place model is adopted + attributed in `NOTICE`.

The device also gave up **5.3 GB** of accumulated dev cruft on the user's USB volume (bundle
tarballs, a 28 MB log, ~40 framebuffer dumps); the 285 irreplaceable on-device dev scripts were
archived into the repo first.

Four defects had to be fixed before the daemon would start at all, each of which would have
shipped: `ipkg` without `-o` installs into the **rootfs** rather than the app partition; the ARM
dist's `icudt78l.dat` is a symlink into the build container, so the bundler **silently** shipped no
ICU data and no GRE resources; `ICU_DATA` was never set, so libxul aborted in `u_init()` on every
start and upstart respawned forever; and `| grep -q` under `set -o pipefail` fails on SIGPIPE —
which made the R8 "no `/media/internal`" assertion **fail open**. That last one is endemic: it was
found independently in the adapter build scripts, the `.ipk` verifier, the Mochi payload check and
the device harness.

Also this session: **browser-services R4 (downloads) closed on desktop** — no `nsITransfer` was
registered, so `CreateTransfer` failed and *every* download was being cancelled — plus **cookie
persistence** proven across an engine restart; the **Mojo front-end promoted from skeleton to a
working browser** (new cavekit-mojo-ui.md); and the desktop **round-trip harness found broken since
2026-07-27** (the daemon adopted the real isis shmem header layout, the test stand-in never
followed), meaning every "ROUND-TRIP PASS" restated after that date was inherited rather than
reproduced — now fixed and re-verified. A fable adversarial review of the download work found two
P1s and an R8 violation; all nine findings are fixed
(`impl-review-findings-downloads.md`).

## 2026-07-29 — FULL OE BUILD-FROM-SOURCE → two self-contained `.ipk`s + Mojo skeleton (READ FIRST)

The reproducible Open webOS build (build-webos + meta-webos, 2013 "dylan" / bitbake 1.18) is
**complete**: `oe-env.sh run ". oe-init-build-env && bitbake net.riverstonerelay.jihad-browser
net.riverstonerelay.jihad-browser-mochi"` cross-compiles the whole stack **from source** into two
self-contained app `.ipk`s (Enyo 39 MB, Mochi 38 MB). Each bundles the engine (libxul) + daemon +
adapter (shim+impl) + **bundled glibc-2.23** + NSS + GRE via the new `jihad-deviceroot` recipe, with
a `postinst` that deploys the coexisting daemon/shim/upstart. A **Mojo UI skeleton** (`app-mojo/` +
recipe) scaffolds a future third front-end. New: `build/webos-oe/oe-env.sh` (chroot Ubuntu-14.04 OE
host on any Linux; no container, sudo/doas) + `docs/OE-BUILD.md`. Recipe chain fixes (pseudo/oe-core
symlink, host-gcc9, python3+UTF-8 xpt, LD override, engine `.so` closure + static-`.a` via datadir,
x86 host-tool exclusion) are in auto-memory `jihad-oe-env`. Meets device-build **R3**; the open gate
is on-device INSTALL of the `.ipk`s (**R4**). Direct-cross-build scripts remain the faster verified path.

## 2026-07-27 — ZOOM FIXED: magnify + visual-viewport pan (READ FIRST)

Rotation confirmed working on device; the next issue was zoom ("things get cut off"). ROOT:
`presShell->RenderDocument`'s internal scale cancels any engine zoom, so `SetResolution`/
`SetFullZoom` shrank the offscreen capture into a `1/zoom` quadrant. FIX: magnify in
`JihadRenderDocument` (libxul) by pre-scaling the gfxContext by Z + rendering a `w/Z × h/Z`
sub-rect (layout viewport untouched → no reflow → rotation unaffected); pan via an engine-scroll
(brings rows into the display list) + `aRect` residual split in the daemon (horizontal + vertical
bottom reachable). Verified on device: 3× magnify, horizontal + vertical pan, zoom-1 no
regression. Also built an **autonomous device-driving toolkit** (daemon inject channel + frame.ppm
capture). Full detail: `zoom-fix-2026-07-27.md` + auto-memory `jihad-input-activation-and-tiling`.

## 2026-07-26 — ROTATION RENDER-BREAK FIXED (READ FIRST)

Portrait↔landscape rotation broke the composite (3× tiling + scanlines in landscape);
Atlas did not. **Root cause: the adapter's raw `dstBuffer` blit is row-major in the card's
logical orientation and ignores the rotation transform the compositor applies.** Fixed by
compositing through the WebKit **PGContext** (`useGraphicsContext=true` + `PGSurface::wrap` +
`gc->bitblt`), the rotation-aware path Atlas uses. Compiles + links (PG symbols resolve at
load from `libWebKitLuna`); on-device visual + `dlopen_probe` confirm remains (device was
offline). New kit req **cavekit-offscreen-rendering R6**. Full detail: `rotation-fix-2026-07-26.md`.

## 2026-07-17 — DEVICE BIG-TEST RESULTS (READ FIRST)

The loading-lifecycle batch (real progress, POST adopt, UI-only watchdog — commits
df24fb8..1d9532b, codex-clean) is deployed, but the big test surfaced 5 open
interaction failures: focus-scroll pushes the page off screen, Enter shows the
overlay without visible results, **stale frames** (no invalidation-driven repaint —
the suspected shared root), VKB stuck after multi-site session, link taps show the
overlay without visible navigation. **Authoritative catalog + hypotheses + retest
plan: `context/impl/device-test-2026-07-17.md`.** Matching REPORTED flags are on
cavekit-offscreen-rendering R3, cavekit-input-bridging R2/R2a, and
cavekit-navigation-events R6.

## 2026-07-07 — SELF-CONTAINED APP, RENDERS REAL PAGES ON DEVICE (READ FIRST)

Jihad is now a **self-contained app that coexists with the stock browser** (own
MIME `application/x-jihad-browser` → `BrowserAdapterJihad.so` → daemon socket
`/tmp/yapserver.jihad-browser` → upstart job `jihad`; the app's WebView is routed
there by `app/source/JihadEngineOverride.js`). Nothing system-level is replaced.
Validated on the TouchPad: the card loads the Jihad adapter → the Jihad daemon,
`http://example.com` and `slack.com` (HTTPS) load + render on `/dev/fb1`,
load-completion fires (address-bar refresh glyph), both daemons coexist, device
stable. Authoritative detail + deploy gotchas (reboot-to-register-plugin,
`.ipk`-reinstall-to-bust-cache, content-is-on-fb1) in `docs/PICKUP.md`
(2026-07-07) and auto-memory `jihad-self-contained-arch.md`, `jihad-device-gotchas.md`.

Still open (Phase-3 hardening): tap activation (staged `buttons` fix), landscape
composite (staged rotation guard), keyboard/VKB, URL-fixup edge cases.

## 2026-07-06 — REAL on-device UI/UX state (READ FIRST)

The build pipeline and offscreen round-trip below are done, but **interactive on-device
browsing is NOT complete** — verified against the real screen (`/dev/fb1`), the daemon's
`frame.ppm` was misleading (it's the pre-composite render). See
`docs/PICKUP.md` (2026-07-06 section) for the authoritative current state.

Working on-device: portrait render (after DPR=1 fix), about:jihad/about:isis pages, the
full UA (docShell customUserAgent), URL-bar → about: navigation, DuckDuckGo default,
new-window Jihad logo, upstart-supervised daemon persistence.

Broken / unverified on-device (the real remaining work): **click activation** (taps reach
the daemon but SendMouseEvent doesn't fire links), **landscape composite** (adapter blits a
stale-orientation buffer → tiling/scanlines; daemon render is correct), **load lifecycle**
(load-complete now fires on document STOP — fixed, unverified; was the cause of the looping
load overlay + address-bar X never→refresh + partial render + broken search), **HTTPS heavy
pages** (NSS marshal-flood fixed in libxul, unverified), **keyboard/VKB** (no msgEditorFocused).

## 2026-07-04 — HEADLESS ENGINE RENDERS ON THE HP TOUCHPAD (X/GTK dropped)

The whole Phase-1 pipeline is now proven **on real ARM hardware with a fully
X/GTK-free engine**:
- **`MOZ_WIDGET_TOOLKIT=headless`** (new `widget/headless/` backend +
  `gfxPlatformHeadless` + headless gates across gfx/thebes, gfx/2d, gfx/skia,
  exthandler, widget, toolkit/library). libxul links **zero** gtk/gdk/pango/cairo/X
  — only freetype+fontconfig. Two runtime bugs fixed: null `GfxInfo` at
  `gfxPlatform::InitAcceleration` (stub registered) and a `PuppetScreen`↔fallback-hal
  infinite recursion (GetRect/GetColorDepth/GetPixelDepth answered directly).
- **Desktop**: offscreen ROUND-TRIP PASS, msgPainted 786432, `jihad-poc-render.ppm`.
- **Device (TouchPad topaz-linux)**: ARM cross-built headless libxul (29 M stripped,
  was 46 M) + GTK-free daemon (`-DJIHAD_OFFSCREEN_ONLY`) + lean bundle (**28 .so, no
  gtk/X — vs 68 before**) → deployed to `/media/internal/jihad/hl` → **on-device
  offscreen ROUND-TRIP PASS, msgPainted 786432**.
- Closes **Engine-Embedding R2**, **Offscreen-Rendering** (on-device),
  **Device-Build R2**; advances **Device-Build R4/R5**. Full recipe + gotchas in
  auto-memory `jihad-headless-toolkit.md` and `docs/PICKUP.md`.

**Remaining for "all kits complete"** (see cavekit-overview.md status column):
Mochi UI variant (`app-mochi/` skeleton → parity, largest item); device UI
integration (LunaService-enabled daemon + real NPAPI BrowserAdapter so the Enyo UI
drives the Jihad daemon on-screen — IPC R4/R5, UI-Shell R4, Device-Build R3/R4);
on-device input/gestures (Input R2/R3); download progress + SSL-accept + device cert
store (Services R4/R5); TouchPad Go / Opal (Device-Build R6).

## Domain Status
| Domain | Tasks Done | Tasks Total | Status |
|--------|-----------|-------------|--------|
| IPC Contract Preservation | R1–R3 done; R4/R5 device | 5 | T-004/T-005/T-006 build + run: real daemon links the unchanged BrowserServerBase (byte-identical YAP) with the Goanna backend; ROUND-TRIP + FREEZE-THAW PASS on desktop AND on-device. R4 device LunaService + R5 real NPAPI BrowserAdapter are the remaining device-integration items. (See the "IPC Contract / Daemon" row below for detail.) |
| Licensing & Branding | **5 (COMPLETE)** | 5 | R1 headers, R2 LICENSE/NOTICE, R5 Apache↔MPL compatibility — done. **R3 branding strip: build-goanna.sh removes Pale Moon/Basilisk/Moonchild from all.js + nsAboutRedirector + the dead nsAppRunner literals; scan verified 0 branding strings in libxul.so / jihad-browserserver / goanna.js; round-trip still passes. R4: docs/ENGINE-SOURCE.md documents the UXP origin (pinned rev b2594a4), all patches (0001-0004 + strip), and MPL source-availability; LICENSE points to it.** |
| UI Shell | 3 | 4 | T-007/T-008/T-009 done; T-004(ui) R4 pending |
| Engine Embedding & Build | **R1–R4 (R2 verified)** | 4 | libxul builds out-of-tree (R1); runtime init/shutdown once per process; **R2 repeated create/destroy: 20/20 cycles each render a full frame, no crash/leak (LEAK-CYCLE PASS)**; R3 event loop integrated (daemon tick 10ms + g_timeout, responsive, no busy-wait); R4 not vendored (git-ignored, docs/ENGINE-SOURCE.md) |
| Navigation, Loading & Events | **R1–R6 COMPLETE** | 6 | load lifecycle; back/forward (real canGo*); setHtml — NAV PASS. R3 failed-load — FAIL-EVENT PASS. R1 clearHistory + R5 getHistoryState — HISTORY PASS. **R4 url-redirected (STATE_REDIRECTING) — REDIRECT PASS (302 /a→/b via local server). R6: global-history + addUrlRedirect (POSIX-regex rules, RULES PASS: tel: handed off) + link-clicked (content-initiated nav via programmatic-load heuristic, LINK PASS: click →/b reported).** Full domain verified end-to-end |
| IPC Contract / Daemon | **ROUND-TRIP PASS + lifecycle** | — | Real daemon (libYap + unchanged BrowserServerBase + JihadBrowserServer + Goanna); ROUND-TRIP PASS (Connect+OpenUrl→…→msgPainted). **R2/R3: freeze suppresses paint + thaw reattaches buffers + resumes — FREEZE-THAW PASS; returnBuffer wired.** ~40 commands wired (nav/input/geometry/services/dialogs/downloads/history/redirect-rules/drag). findString kept safe (offscreen selection controller = future). Genuinely device-gated: R4 LunaService (device build), R5 real NPAPI BrowserAdapter rebuild. Remaining niche stubs are engine-inapplicable no-ops (plugin spotlight, spelling widget, mouse-mode, DNS/network-iface) |
| Offscreen Rendering | **R1–R3 COMPLETE (+ on-device)** | 5 | **Goanna renders real web pages** — data: page (docs/jihad-render-proof.png) AND live **https://example.com over TLS**; **GoannaRenderPage** backend (Create/LoadUrlAndWait/ReadPixels→ARGB32 shm) → **msgPainted wired through the daemon; ROUND-TRIP PASS on desktop AND on the TouchPad (786432 px)**. Now truly windowless via MOZ_WIDGET_TOOLKIT=headless (PuppetWidget, no gtk window). Geometry/resize/zoom in the row below |
| Offscreen Rendering (geometry) | **R4 + R5 COMPLETE** | 5 | R5: Resize→RESIZE PASS; ScrollTo(javascript:)+GetScrollXY→SCROLL PASS; SetZoom(nsIContentViewer::SetFullZoom via proper content-docshell)→ZOOM PASS (9×). R4 events: GetContentSize(GetRootBounds)→msgContentsSizeChanged, GetViewport(GetViewportInfo, meta-viewport pref on)→msgMetaViewportSet, pump-polled msgScrolledTo — all emit through BrowserPageGoanna→ProxySink→YAP; GEO PASS (2032×2500; init/min/max=0.5/0.5/2.0 us=1; (0,400)). All wired to the daemon. GetDocShell also repaired SetJavaScriptEnabled |
| Input Bridging | **click/key/mouse + coord-mapping (R5)** | 5 | ClickAt/KeyEvent/MouseEvent via nsIDOMWindowUtils — INPUT PASS. **R5 coord-mapping: clickAt uses content/CSS space (proven via coord_test: surface≠content at zoom); bridge now maps adapter surface coords → content (surface/zoom + scroll) for click/mouse/touch — COORDMAP PASS (surface(240,240)@2x hits the box at content(120,120)).** **R1 holdAt (contextmenu) + R4 drag scrolling (dragProcess→scroll, msgScrolledTo) — INPUT2 PASS (holdAt hits, drag scrolls 200).** TouchEvent + insertStringAtCursor wired but desktop-unverified (offscreen widget doesn't route synth touch; execCommand insertText via javascript: doesn't preserve input focus headless) — both on-device. Remaining: R3 pinch/tap gesture (on-device) |
| Browser Services | **R1 settings + R2 cache/cookies + R3 dialogs** | 5 | R1 complete: setEnableJavaScript, setUserAgent, **setMinFontSize/setBlockPopups/setAcceptCookies** (SETTINGS2 PASS — prefs applied + window.open blocked behaviorally). R2: clearCache/clearCookies (SERVICES PASS). R3: DialogService overrides `@mozilla.org/prompter;1` — alert/confirm/prompt captured + reply routed (DIALOG PASS), installed in EngineHost so the daemon never hangs. R4 downloads: DownloadService overrides `@mozilla.org/helperapplauncherdialog;1` — handoff captured (DOWNLOAD PASS). **R5 TLS: invalid cert detected via the security-module document-stop status → msgSSLConfirm(host, code) (TLS PASS: self-signed 127.0.0.1 → host+0x805a2fe3 surfaced, reject-aborts = page not loaded).** Device-gated [human-review on device]: accept-proceeds (the untrusted cert object isn't exposed headless — nsIBadCertListener2 not consulted, SSL-status/failed-chain null) + webOS cert store. Remaining: save-to-disk/progress + per-adapter blocking dialog delivery (adapter/device) |
| Desktop Build & PoC | **R1–R3 done; R4 [human-review]** | 4 | R1: single-entry build produces runnable jihad-browserserver (Goanna backend, LunaService compiled out) — DAEMON_UP. R2: jihad-adapter allocates buffers, Connect+OpenUrl, receives msgPainted, **writes out/jihad-poc-render.ppm** (→ docs/jihad-poc-render.png), returns buffer (0x150d wired). R3: real page renders through the whole pipe, lifecycle in order — ROUND-TRIP PASS. R4 (Enyo UI on desktop) recorded [human-review]: no desktop Enyo runtime → harness is the acceptance vehicle. See docs/DESKTOP-POC.md |
| Device Build & Packaging (ipk) | **R1+R2 DONE on-device; R3–R6 pending** | 6 | **R1 crosstool-NG toolchain (GCC 9.4 / glibc 2.23 softfp, min-kernel 2.6.32) — C++ ran on the TouchPad. R2 engine cross-compiles: X/GTK-free headless libxul (29 M) + GTK-free daemon cross-built, load on the device and RENDER (on-device offscreen ROUND-TRIP PASS, msgPainted 786432); lean bundle 28 .so (no gtk/X) launched via bundled ld-2.23.so on /media/internal.** Authored mozconfig.goanna-arm (now cairo-headless) + build-goanna-arm.sh + build-daemon-arm.sh + make-device-bundle.sh; OE `.bb` skeletons exist. Pending: R3 two `.ipk`s (needs Mochi UI + real BrowserAdapter rebuild) + on-device coexistence; R4 full UI-on-screen via BrowserAdapter + nav/scroll/tap on-device; R5 memory budget [human-review; 29 M libxul helps]; R6 TouchPad Go / Opal (no 2nd device). See docs/PICKUP.md |

## Completed this session (Tier-0, build-independent)
- **T-001/T-002** — LICENSE + NOTICE + license texts; Apache+MPL compatibility stated.
- **T-003** — License-header policy; verified 0/32 imported files missing Apache header.
- **T-004** — YAP interface imported & frozen; `BrowserServerBase.{h,cpp}` md5-verified byte-identical to upstream.
- **T-005** — Shared-mem framebuffer + offscreen-info contract sources imported (engine-agnostic).
- **T-006** — Daemon support sources imported (agnostic bucket); daemon/page-manager imported but flagged for de-Qt adaptation (see render/browserserver/Src/MANIFEST.md).
- **T-007** — App package rebranded (`net.riverstonerelay.jihad-browser`, title "Jihad").
- **T-008** — Verified UI `callBrowserAdapter` set + browserServer Luna URIs identical to upstream isis.
- **T-009** — Verified 24/24 forked UI `.js` retain Apache headers.

See impl-browserserver-import.md for the import detail.

## T-010 build environment (this session) — configure VALIDATED
- Authored pinned build container under `build/desktop/`: `Dockerfile`,
  `mozconfig.goanna` (xulrunner embedding target, GTK2 + basic layers, trimmed),
  `build-goanna.sh`. UXP source is mounted (read-write — autoconf regenerates
  `configure`), never vendored. Documented in `docs/TOOLCHAIN.md`.
- Discovered the hard way (this UXP master is recent, not raw ESR-52):
  - `mach` requires **Python 3.3+** (build/mach_bootstrap.py), not Python 2.
  - configure requires **GCC ≥ 9.1** (build/moz.configure FatalCheckError).
  - needs the full **X11 dev set** (libx11-xcb, xcb, xcomposite, xdamage, …).
  - podman needs **fully-qualified** image names; bionic→archive not old-releases.
  - Net: base image is **Ubuntu 20.04** (GCC 9.3, Python 3.8, autoconf2.13, yasm).
- `mach build` **SUCCEEDS** ("Your build was successful!"). Artifacts in
  `build/desktop/out/obj-jihad-goanna/dist`: **libxul.so (1.8G, unstripped),
  xpcshell, NSS/NSPR**, and the embedding headers the backend needs
  (nsIWebBrowser.h, nsIWebNavigation.h, nsIBaseWindow.h, nsIDOMWindowUtils.h,
  nsEmbedCID.h). **T-010 DONE** — the engine builds for desktop.
- Toolchain fights resolved en route (all captured in build/desktop/): podman
  fully-qualified image; bionic→archive; **Python 3.3+** (not py2); **GCC ≥ 9.1**
  → Ubuntu 20.04 base; full X11 dev set; and the real libxul blocker — js/src
  appends `-Werror=format` after the warnings list, so a command-line
  `-Wno-error=format-overflow` is re-escalated; fixed with a source pragma in
  `js/src/jit/x64/BaseAssembler-x64.h` (patches/0002, applied by build-goanna.sh).
- `Settings.{h,cpp}` reclassified: engine-coupled (QtWebKit WebSettings), needs
  reimplementation against Goanna prefs, not a mechanical de-Qt (see MANIFEST).
- Note: libxul is unstripped (-g) → 1.8G; strip for the device build (Phase 2).

## T-013 embedding runtime — smoke PASS (this session)
- `render/goanna/EngineHost.{h,cpp}` wraps XRE_InitEmbedding2/XRE_TermEmbedding +
  do_CreateInstance(NS_WEBBROWSER_CONTRACTID). `render/goanna/test/embed_smoke.cpp`
  + `build/desktop/build-embed-smoke.sh` compile against dist/ and run.
- Result: "engine runtime up → created nsIWebBrowser → destroyed → PASS", exit 0.
- Link recipe (for the daemon, T-016): `libxpcomglue_s.a -lxul libmozglue.a
  -lnspr4 -lplc4 -lplds4 -ldl -lpthread`, `-include mozilla-config.h`,
  `-fno-rtti -fno-exceptions`, frozen API (no MOZILLA_INTERNAL_API).
- Remaining for full R2: repeated create/destroy leak check; explicit profile dir.

## T-019 + load pipeline — page LOADS (this session)
- `render/goanna/test/embed_load.cpp` + `build/desktop/build-embed-load.sh`:
  minimal GTK embedder (invisible window under Xvfb) — creates nsIWebBrowser with
  a small chrome (nsIWebBrowserChrome/nsIEmbeddingSiteWindow/nsIInterfaceRequestor/
  nsIWebProgressListener/weakref), loads a data: URL, pumps the XPCOM event queue
  (`NS_ProcessNextEvent`) + GTK, and observes the full load lifecycle to STATE_STOP.
  Result: "load reached STATE_STOP → PASS", exit 0.
- Lessons baked in: event loop must pump `NS_ProcessNextEvent` (GLib-only didn't
  advance the load); the webBrowser does NOT QI to nsIWebProgress — use
  `AddWebBrowserListener(weakRef, NS_GET_IID(nsIWebProgressListener))`; tear the
  browser/baseWindow down before XRE_TermEmbedding to avoid a shutdown crash;
  desktop needs Xvfb (added to the container).
- Rendering architecture resolved (see render/goanna/PORT-MAP.md): offscreen
  render needs internal gfx/layers (build the backend in-tree) + a display; the
  frozen-API EngineHost still drives navigation/lifecycle.

## Render path — exact blocker pinned (this session)
- Attempted headless paint (map GTK window under Xvfb → expose → capture pixels).
  Crashes in `ClientLayerManager::ForwardTransaction` (null forwarder). Verified
  the GTK widget forces ClientLayerManager (OMTC pref off confirmed, no effect)
  and `XRE_InitEmbedding2` starts no compositor. Full analysis in dead-ends.md.
- So T-020 must **bring up the compositor** (CompositorThread + bridge) — the
  full-app/in-tree path — before paint. embed_load's render attempt is gated
  behind `JIHAD_TRY_RENDER`; default path proves load (exit 0).

## Next (render path)
- T-020: initialize the in-process compositor in the backend (replicate the
  relevant nsAppShell/XRE compositor setup, or build the backend in-tree where
  it's available), then map/paint.
- T-024: read the rendered layer/window back into the shmem buffer
  (`BrowserOffscreenInfo` ARGB32) → T-032 msgPainted. (Pixel-capture via GDK
  already written in embed_load, ready once paint works.)
- Map the observed load lifecycle to YAP msgLoadStarted/Progress/Stopped.
- T-016 (daemon wiring) reuses the embed link recipe; T-011 ARM toolchain parallel.
- T-011 (ARMv7 cross-toolchain) in parallel; reuse the container base. For the
  device, strip libxul and revisit jemalloc/optimize-for-size.
- Bucket-2 files (BrowserServer/Main/BrowserPageManager/Settings) compile once the
  Goanna backend provides `BrowserPage` and the event-loop integration lands.
