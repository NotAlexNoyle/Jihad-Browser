---
created: "2026-06-30"
last_edited: "2026-08-04"
---

# Cavekit Overview

## Project
Jihad Browser — port the UXP/Goanna web engine into the isis-browser webOS 3
(HP TouchPad) shell, replacing QtWebKit while keeping the
BrowserAdapter↔BrowserServer YAP IPC command/message contract byte-identical.
Phase 1 brings the engine up on desktop x86_64; Phase 2 cross-compiles for the
device. Jihad ships **self-contained, coexisting with the stock browser** — its
own MIME/adapter/daemon/upstart job, nothing system-level replaced (Atlas model;
see `jihad-self-contained-arch.md`). The adapter carries a two-line rebrand (MIME
string + YAP server name) for coexistence; the YAP command/message interface it
speaks is unchanged.

**Three independent front-ends (user decision 2026-07-31).** Enyo 1.0 (`app/`), Enyo 2 + Mochi
(`app-mochi/`), and Mojo (`app-mojo/`) each ship as a complete, standalone browser with its own
MIME / adapter shim + impl / YAP name / socket / upstart job / daemon process — installing or
removing one cannot affect another (cavekit-device-build.md **R7**). Each package runs its engine
in place from its own cryptofs `deviceroot` and writes nothing to the user's `/media/internal`
storage (**R8**).

Grounding: `context/refs/refs-overview.md`, `docs/IPC-CONTRACT.md`,
`render/goanna/PORT-MAP.md`.

## Domain Index
Status legend: ✅ complete · 🟢 mostly (device/edge items remain) · 🟡 partial · ⬜ not started.

> **Device reality check (2026-08-03, second session).** **All three variants (Enyo, Mochi, Mojo)
> are live on hardware** — each card paints through its own daemon on its own socket, cold boot
> auto-starts all three, and `device-independence-test.sh check` passes 24/24. Feature work is no
> longer Enyo-only: `<select>` popups and the start page are now verified on **all three**, and the
> Mojo shell gained its chrome actions (new card / history / share).
>
> **The card JS dev loop is RESTORED** — `build/webos-oe/push-card-js.sh` is the tool, and it
> proves each reload by a per-push stamp reaching the device log. Two independent causes had
> broken it: **`novacom run` discards output that arrives after the host's stdin hits EOF** (hold
> it open — `sleep 4 | novacom run …`; this masqueraded as luna-send "blackouts", dead
> `applicationManager/running` queries, and "`enyo.log` stopped reaching `palm-log`"), and the
> **WebAppMgr in-process JS cache really does serve a stale build** after a close+relaunch (a
> LunaSysMgr restart per cycle busts it, and is the script's default).
>
> Cross-cutting defects RESOLVED: scroll pan headroom (user signed off), long-press `contextmenu`,
> below-the-fold input landing, and the **`<select>` popup on all three variants** — the "empty
> popup" was a JSON field-name mismatch, not a rendering problem (the stock framework consumes the
> event itself and expects the isis `items[].text/isEnabled` shape; Mojo needed no app code at
> all). The **`<menupopup>` track then closed too**: engine popups are composited over the frame,
> taps/moves route into them, the row under the finger highlights, drag-and-release selects, and a
> tap outside rolls up — the `about:addons` tools menu opens, reads correctly and its items act.
> **XPI web install now works end to end, on device** (the first extension this browser has
> installed), which required fixing two engine assumptions that a browser chrome exists above the
> content, and then implementing the **DialogSink the daemon never had** — a claim the kits carried
> on the strength of a test that supplied its own sink. Still open on these tracks: `<optgroup>`
> header rows in `<select>` lists, and the SSL accept-and-reload branch. See
> `impl-select-popup-2026-08-03.md`, `impl-menupopup-2026-08-02.md`,
> `impl-scroll-overscan-2026-08-02.md`, `impl-addons-icons-open.md`.
>
> **Platform constraint found the hard way:** the webOS 3 **card** WebKit (~534.x) silently
> ignores unprefixed `box-sizing` and modern flexbox, so all card-chrome CSS must carry
> `-webkit-` prefixes (it made the Mojo toolbar 784 px wide on a 768 px screen). Pages rendered by
> our own Goanna engine are modern and unaffected.
Verified on desktop x86_64 AND cross-built + rendering on the HP TouchPad unless noted.

| Domain | Cavekit File | Requirements | Status | Description |
|--------|--------------|--------------|--------|-------------|
| UI Shell (Enyo) | cavekit-ui-shell.md | 5 | 🟢 R1–R3 ✓; R4 3/4 ACs device-verified 2026-07-20 (launch, address→openUrl, back/forward/reload); findInPage untested. `<select>` popup ✓; start page follows VKB/orientation and carries the shared logo/title/hint block; app renamed "Jihad Enyo" | Forked/rebranded Enyo-1.0 app (`app/`) using the unchanged adapter contract |
| Mochi UI Variant | cavekit-mochi-ui.md | 6 | 🟢 R1/R3/R5 ✓; card LIVE on device (loads + paints through its own daemon); **`<select>` popup device-verified 2026-08-03** (own overlay list, pick applied); start page matches the others; R2/R4 remaining on-device feature checks pending; renamed "Jihad Mochi" | Second front-end on Enyo-2/Mochi (`app-mochi/`), same contract, separate .ipk |
| Mojo UI Variant | cavekit-mojo-ui.md | 6 | 🟢 R1–R3 ✓ device-verified, R5 ✓, R4 2/3 (Opal hardware-gated); **R6 (new) chrome actions ✓** — new card / history / share, title row dropped, toolbar overflow fixed (`-webkit-box-sizing`); `<select>` popup works with **no app code** (system framework handles it); renamed "Jihad Mojo" | Third front-end on the system Mojo framework (`app-mojo/`), same contract, own independent .ipk |
| IPC Contract Preservation | cavekit-ipc-contract.md | 5 | 🟢 R1–R3 ✓; R5 ✓ on-device (adapter drives daemon; +2-line coexistence rebrand); R4 device LunaService | Frozen YAP command/message interface, shmem framebuffer, daemon, LunaService |
| Engine Embedding & Build | cavekit-engine-embedding.md | 4 | ✅ 4/4 (+ ARM cross-build) | Out-of-tree Goanna build, embedding runtime, event-loop integration |
| Offscreen Rendering | cavekit-offscreen-rendering.md | 7 | 🟢 R1–R6 ✅ desktop + on-device (R6 rotation composite + R5 zoom device-confirmed 2026-07-27); scroll pan headroom FIXED (overscan region paint, ≤2048-row SGX cap; user signed off, Opus-reviewed). **R7 (new) engine popups ✅**: `<select>` solved card-side on all three variants; `<menupopup>` composited over the frame AND interactive (taps routed in, rollover highlight, drag-select, tap-outside rollup) — the old "popup is 0x0 and never shown" diagnosis was measured on a `<select>` (the combobox path) and was wrong. Follow-up owed: F7 header frame-seq (needs an adapter rebuild) | Headless render → shared buffer → paint protocol + geometry events + orientation-correct composite + engine-popup delivery |
| Input Bridging | cavekit-input-bridging.md | 7 | 🟢 R1 ✓ + **long-press `contextmenu` works on device** (the daemon `asyncCmdHitTest` gate was a stub; real hit-test round-trip added, user-confirmed), R4 ✓, R5 ✓ + **doc→viewport coord mapping fixed** (below-the-fold taps landed a screenful low); R2 VKB jank / R3 gestures on-device; **R6 XUL input partial** — text reaches XUL fields through the engine editor, but real DOM key events need a ~2-line PuppetWidget patch + a full libxul rebuild | webOS pointer/key/touch/gesture → DOM events |
| Navigation, Loading & Events | cavekit-navigation-events.md | 7 | 🟢 R1–R7 met (R6's link-clicked AC still [~] — on-device link-tap navigation confirmed 2026-07-27, the message-emission re-test is open) | Nav commands + load/location/title/history message stream |
| Add-ons & Extensions | cavekit-addons-extensions.md | 8 | 🟡 R1 ✓, R2 ✓ (`about:addons` renders on device; branding pkg + the Jihad logo on about pages), R8 ✅, R5/R6 ✓; **R3 XPI install WORKS** — device-verified: a page's `InstallTrigger.install()` raises the confirm on the card, accept installs, and `about:addons` lists the add-on. Needed patch 0013 (`amInstallTrigger` and `AddonManager` both assume a chrome `<browser>` above the content) plus the new daemon DialogSink; **R7 reframed — windowless NPAPI does not exist in a cairo-headless build and must be PORTED, not enabled**. The tools menu now opens and is operable (cavekit-offscreen-rendering.md R7) | `about:addons` + classic XPI + NPAPI plugin support |
| Browser Services | cavekit-browser-services.md | 5 | 🟢 R1–R3 ✓ (**R3 dialogs really work now** — the daemon had NO DialogSink, so every engine dialog silently took its default; `BrowserPageGoanna` is now that sink, over the frozen FIFO contract, with a pickup deadline so an unanswered dialog cannot wedge the daemon); R4 partial; R5 SSL confirm wired with a reply path, accept-and-reload not yet end-to-end verified | Settings, cookies/cache, JS dialogs, downloads, TLS |
| Desktop Build & PoC Harness | cavekit-desktop-build.md | 4 | ✅ R1–R3; R4 [human-review] | Phase-1 x86_64 build + YAP test client + end-to-end gate |
| Device Build & Packaging | cavekit-device-build.md | 8 | 🟡 R1/R2 ✓; R3 build-produced (`.ipk`s + review items fixed); **R7 (per-variant independence) now DEVICE-VERIFIED 2026-08-03** — all three variants live, `device-independence-test.sh check` 24/24, cold-boot auto-start, own sockets, `/media/internal` clean; R8 ✓. Deploy routes: `push-variant.sh` (full payload) / `push-engine-update.sh` (fast libxul+daemon swap) / **`push-card-js.sh` (card JS/CSS/assets, stamp-proven)** — all novacom, all md5-verified. The supported Preware/WOQI `.ipk` install (R3/R4 "device-verified") is still user-gated; clean-clone reproducibility (#7/#8) + R5/R6 (memory budget, Opal) open | Phase-2 ARM cross-toolchain; self-contained packaging via bitbake; TouchPad + TouchPad Go |
| Licensing & Branding | cavekit-licensing-branding.md | 5 | ✅ 5/5 | Apache+MPL headers, NOTICE, trademark stripping (cross-cut) |

Totals: **13 domains, 77 requirements, 276 acceptance criteria — 211 met, 19 partial, 46 open**
(recounted 2026-08-04 after the engine-popup, XPI-install and dialog work; the previous "12 domains, 62 requirements" line had drifted
from the files it summarised). Closed this session on top of the earlier scroll / long-press /
coord-mapping / R7-independence work: the card dev loop, the `<select>` popup on all three
variants, the Mojo chrome actions, the Mojo toolbar overflow and the shared start page. The bulk
of what remains open sits in two kits — add-ons (22 open ACs: XPI wiring, extension effects, the
NPAPI port) and device-build (11: clean-clone reproducibility, Opal, memory budget) — plus the
XUL-input and engine-popup work. Every open item is hardware-gated, a named debug lead, or the
engine-popup-overlay track. **Current priorities — see `../impl/impl-NEXT-AGENT-START-HERE.md`
for the detailed queue:**
(1) **cookie/cache persistence** (browser-services R2) — no `cookies.sqlite` on device despite a
correct provider + prefs; the one non-hardware debug lead left;
(2) **XUL zoom on `about:addons`** — user-reported as unreliable. The fit-zoom itself is correct
(a fixed-width 980 px XUL document in a 768 px window), and the popup work does not perturb
contentSize; an injected zoom is overwritten by the card re-asserting its own, so the actual
pinch path needs a real gesture. A trace of every zoom the card requests is deployed;
(3) **chrome icon repaint latency** (addons R2 polish) — repaint-delivery latency, has a debug
lead. Re-checked 2026-08-04 and NOT reproduced on a fresh load (icons present at 3/8/16 s);
(4) **SSL accept-and-reload** (browser-services R5) — wired, not yet verified end to end; a cert
failure also raises an engine ALERT before the confirm, which wants its own look;
(5) `<select>` `<optgroup>` header rows (needs a daemon reply-index remap); F7 scroll header
frame-seq (needs an adapter rebuild); VKB jank (input R2) + gestures (input R3);
ui-shell R4 findInPage (NOT small — the engine's FindNext SIGSEGVs offscreen; needs a selection controller); device LunaService (IPC R4);
(6) clean-clone OE reproducibility (device-build #7/#8); TouchPad Go + memory budget (device-build R5/R6).
Full OE-review findings: `../impl/impl-review-findings-oe.md`.

### Working on the card UIs
`build/webos-oe/push-card-js.sh <enyo|mochi|mojo> <files…>` is the card-JS loop: it stamps the
push, md5-verifies it on both sides, closes the card by its real `processid`, restarts LunaSysMgr
(the WebAppMgr JS cache is real), relaunches, and **fails unless the new stamp appears in the
device log**. A screenshot is not proof a card is alive (fb1 holds the last painted frame) and an
on-disk md5 is not proof a card reloaded — the stamp is. Hold stdin open on every `novacom run`
whose output matters (`sleep 4 | novacom run …`).

### Milestone (2026-07-27): rotation and zoom work on the device
The portrait↔landscape render-break and the "things get cut off" zoom are both fixed and
confirmed on hardware (commit 8d7865c; `../impl/rotation-fix-2026-07-26.md`,
`../impl/zoom-fix-2026-07-27.md`, `../impl/impl-overview.md` 2026-07-27 "Rotation confirmed
working on device"). Rotation: the adapter now composites through the WebKit Piranha
**PGContext** (transform-aware) instead of a raw `dstBuffer` blit that LunaCE read at a fixed
~256px pitch — that pitch, not any daemon bug, was the portrait 3× shear. Zoom: `RenderDocument`'s
internal scale cancelled the engine zoom, so the daemon now pre-scales the gfxContext and renders
an absolute document rect (magnify + full-page pan in both axes, layout viewport untouched so
rotation stays safe). The same change made the daemon report the EFFECTIVE render scale, which
closed the long-open small-link tap miss (input-bridging R5, commit d4f0842). Closes offscreen
R6, closes device-build R4's composite AC, and supersedes the 2026-07-20 "LunaCE limitation, NOT
fixable" analysis.

### Milestone (2026-07-07): self-contained app renders real pages on the TouchPad
Re-architected from *replacing* the system browser to a **self-contained app that
coexists** with it (Atlas model: own MIME `application/x-jihad-browser` →
`BrowserAdapterJihad.so` → `/tmp/yapserver.jihad-browser` → upstart job `jihad`; the
app's WebView routed there by `app/source/JihadEngineOverride.js`). On-device the card
loads the Jihad adapter → the Jihad daemon, `example.com` + `slack.com`/HTTPS load and
render (fb1), load-completion fires (address-bar refresh glyph), and the stock browser
keeps working for all other apps. This meets IPC-Contract R5 (rebuilt adapter drives the
daemon through a full load+paint cycle) and advances Device-Build R4. Deploy gotchas
(reboot registers the plugin; `.ipk` reinstall busts the WebKit cache; content is on fb1)
in auto-memory `jihad-self-contained-arch.md` + `jihad-device-gotchas.md`.

### Milestone (2026-07-04): headless engine, no X/GTK, renders on the TouchPad
`MOZ_WIDGET_TOOLKIT=headless` — libxul links **zero** gtk/gdk/pango/cairo/X libs
(freetype+fontconfig only); ARM cross-build (29 M libxul) + GTK-free daemon → lean
device bundle (**28 .so vs 68**) → **on-device offscreen ROUND-TRIP PASS** (msgPainted
786432). This closes Engine-Embedding R2, Offscreen Rendering on-device, and
Device-Build R2, and advances Device-Build R4/R5. Detail in
`context/impl/impl-overview.md` + auto-memory `jihad-headless-toolkit.md`.

## Cross-Reference Map
| Domain A | Interacts With | Interaction Type |
|----------|----------------|------------------|
| UI Shell (Enyo) | IPC Contract | uses contract (client) |
| UI Shell (Enyo) | Navigation/Events | drives navigation, observes events |
| Mochi UI Variant | IPC Contract / UI Shell | same contract; parity with Enyo UI |
| Mochi UI Variant | Device Build | second .ipk packaged + on both TouchPad models |
| Mojo UI Variant | IPC Contract / UI Shell | same contract; behavioral baseline is the Enyo shell |
| Mojo UI Variant | Device Build | third independent .ipk (own MIME/adapter/socket/upstart/daemon) |
| IPC Contract | Offscreen Rendering | framebuffer + paint protocol |
| IPC Contract | Engine Embedding | page lifecycle / page manager |
| IPC Contract | Browser Services | LunaService surface |
| Engine Embedding | Offscreen Rendering | provides instance + event loop for painting |
| Engine Embedding | Navigation/Events | provides event loop for load progress |
| Offscreen Rendering | Input Bridging | shared transform (zoom/scroll ↔ coordinate mapping) |
| Offscreen Rendering | Navigation/Events | geometry/viewport events |
| Navigation/Events | Browser Services | redirects, MIME handoff, downloads |
| Browser Services | Device Build | device cert store / Luna integration |
| Licensing & Branding | UI / IPC / Engine / Services | cross-cutting compliance |
| Desktop Build | Engine/Render/Nav/Input | integrates them into the Phase-1 PoC |
| Device Build | Engine Embedding / Desktop Build | reuses integration, cross-compiles + packages |

## Dependency Graph
Implementation order (earlier enables later):

```
Tier 0 (foundations, parallel):
  IPC Contract Preservation
  Licensing & Branding (cross-cutting; applied throughout)

Tier 1:
  Engine Embedding & Build     (needs: build host per docs/TOOLCHAIN.md)

Tier 2 (the integration core, co-developed on Engine Embedding + IPC):
  Offscreen Rendering          (needs: IPC R2, Engine Embedding R2/R3)
  Navigation, Loading & Events (needs: IPC R1, Engine Embedding R3)
  Input Bridging               (consumes Offscreen transform)
  Browser Services             (needs: IPC R4, Engine Embedding R3, Navigation R6)

Tier 3:
  Desktop Build & PoC Harness  (Phase-1 acceptance: needs IPC + Engine + Tier 2)

Tier 4 (Phase 2):
  Device Build & Packaging     (needs: Tier 2/3 + cross-toolchain gate;
                                produces THREE independent UI .ipks, each running
                                on both TouchPad models)

Parallel UI track (depends only on the contract; packaged by Device Build):
  UI Shell (Enyo)   — forked/rebranded; live on device
  Mochi UI Variant  — Enyo-2/Mochi rewrite to parity; live on device (uses
                      Navigation/Services contracts as its behavioral reference)
  Mojo UI Variant   — built on the device's own Mojo framework; live on device
```

Notes:
- The graph is acyclic. Tier-2 domains are tightly coupled (the paint loop needs
  both the engine event loop and the buffer protocol) so they are co-developed,
  but each depends only on Tier-0/Tier-1 — bidirectional links between them are
  conceptual (see each kit's Cross-References), not build-order dependencies.
- The cross-toolchain (Device Build R1) has no engine dependency and can be
  stood up in parallel with Phase-1 integration to de-risk Phase 2.
- Both UI variants share the BrowserAdapter contract and can be developed in
  parallel with the engine work; only their on-device verification needs the
  working daemon. The Mochi rewrite was the larger of the two; as of 2026-07-20
  its shell, parity views, dialogs, and `app-mochi/PARITY.md` are built and it
  dual-installs on device (2026-07-19) — what is left is on-device functional
  verification, not construction.

## Changelog
- 2026-08-03 (second session): The card JS dev loop is restored (`push-card-js.sh`, stamp-proven
  reloads) — the two causes were `novacom run`'s stdin-EOF output race and the real WebAppMgr JS
  cache. With it, the `<select>` popup closed on **all three** variants: the "empty popup" was a
  JSON field-name mismatch (the stock framework consumes `onOpenSelect` itself and expects the
  isis `items[].text/isEnabled` + `selectedIdx` shape), so the daemon now emits that shape, the
  Enyo app's own popup code was deleted as unreachable, Mochi got an overlay list, and Mojo needed
  no app code at all. Opus review hardened the apply path (disabled/optgroup/out-of-range/no-op
  guards, fail-closed popup file, process-global ids). Mojo gained **R6** (new card / history /
  share), lost its redundant title row, and had its toolbar overflow fixed — root cause: the card
  WebKit ignores unprefixed `box-sizing`, now documented as a platform constraint. All three start
  pages share the logo/title/engine-line/hint block. Totals → ~54 met / ~9 open; priorities
  reordered around the now-unblocked queue.
- 2026-08-03: Session handoff for the next Fable run. Closed on device: scroll pan headroom
  (offscreen, overscan paint, Opus-reviewed), long-press/`contextmenu` (input, hit-test round-trip
  was a daemon stub), input coord mapping (input R5, doc→viewport), and **device-build R7** — all
  three variants live (`push-variant.sh`/`push-engine-update.sh`, `device-independence-test.sh
  check` 24/24). Apps renamed Enyo/Mochi/Mojo; Jihad logo on the about pages; Enyo start page
  follows VKB/orientation. Built but card-blocked: the `<select>` popup (daemon done + verified,
  card list empty behind a broken card JS dev-loop) and the XPI install prompt (authored, unwired).
  Totals → ~48 met / ~14 open; priorities reordered around the card-dev-loop prerequisite. New impl
  docs: `impl-scroll-overscan-2026-08-02.md`, `impl-menupopup-2026-08-02.md`,
  `impl-select-popup-2026-08-03.md`; the START-HERE handoff was rewritten.
- 2026-07-31: Reconciliation against recorded evidence. The Mochi row read "⬜ 0/5 — skeleton
  only" while cavekit-mochi-ui.md records R1 (dual-install VERIFIED ON DEVICE 2026-07-19), R3 and
  R5 as met → now 🟡 R1/R3/R5 ✓, R2/R4 [~]. The UI-Shell row now reflects R4's three
  device-verified ACs (device-test-2026-07-19 Session 4). The Offscreen row goes 5→6 requirements
  (R6, added 2026-07-26) and 6/6 (rotation composite + zoom rework device-confirmed 2026-07-27).
  The Device-Build row no longer describes the Mochi `.ipk` as broken — review items #1/#4/#5/#6/
  #9/#12 are fixed + build-verified (commits 9413d16, b1c0112); the two real gaps (clean-clone
  reproducibility, on-device install) are named instead. Totals corrected to 55 requirements
  (~43 met, ~12 open) and "Current priorities" reordered to what is actually open — the list still
  led with those fixed review items. Added the 2026-07-27 rotation/zoom milestone.
