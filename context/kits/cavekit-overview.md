---
created: "2026-06-30"
last_edited: "2026-10-06"
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

### Target platform

The TouchPad runs **webOS CE 3.1.0** since 2026-09 (it ran HP webOS 3.0.5 before). A kit's "device-verified" dated
before 2026-09-27 is 3.0.5 evidence; cavekit-device-build.md R4 lists them for re-verification. The kit files are
authoritative; the table below is counted from them.

## Domain Index
Status legend: ✅ complete · 🟢 mostly (80 % or more met) · 🟡 partial · ⬜ not started. Counted from the kit files
(2026-11-07); recount with `python3 -I tools/readme-progress.py --print`. What each status means in detail, and
what is next, is in the kit itself, `docs/PICKUP.md` (the current handoff) and `../plans/build-site.md` (the tasks).

| Domain | Cavekit File | Requirements | Status | Description |
|--------|--------------|--------------|--------|-------------|
| GPU Rendering (the pick: 2 + 4) | cavekit-gpu-rendering.md | 12 | 🟢 45 met / 3 partial / 1 open | The page composited on the Adreno 220 by the engine's OpenGL compositor in the daemon and read back into the YAP buffer (option 4), then WebGL through EGL (option 2); the software path the fallback; Tier 87 |
| The Modern Web | cavekit-modern-web.md | 6 | 🟢 27 met / 3 partial / 0 open | Site-by-site compatibility ledger on the device and the configuration fixes it drives: the slow-script guard sized for the CPU, Cloudflare's challenge (UA), GitHub's deferred parts (diagnosed), Wikipedia's layout |
| Content Blocking | cavekit-content-blocking.md | 7 | 🟢 26 met / 1 partial / 0 open | An ad and tracker blocker in the engine for all three interfaces: EasyList + EasyPrivacy (GPLv3+), host blocking by nsIContentPolicy, generic element hiding by a user sheet, 4-day updates, Settings → Privacy rows, device budget |
| UI Shell (Enyo) | cavekit-ui-shell.md | 8 | 🟢 36 met / 0 partial / 2 open | Forked/rebranded Enyo-1.0 app (`app/`) using the unchanged adapter contract |
| Mochi UI Variant | cavekit-mochi-ui.md | 7 | 🟢 37 met / 2 partial / 0 open | Second front-end on Enyo-2/Mochi (`app-mochi/`), same contract, separate .ipk |
| Mojo UI Variant | cavekit-mojo-ui.md | 7 | 🟢 38 met / 5 partial / 0 open | Third front-end on the system Mojo framework (`app-mojo/`), same contract, own independent .ipk |
| IPC Contract Preservation | cavekit-ipc-contract.md | 5 | ✅ 16 met / 0 partial / 0 open | Frozen YAP command/message interface, shmem framebuffer, daemon, LunaService |
| Engine Embedding & Build | cavekit-engine-embedding.md | 4 | ✅ 15 met / 0 partial / 0 open | Out-of-tree Goanna build, embedding runtime, event-loop integration |
| Offscreen Rendering | cavekit-offscreen-rendering.md | 9 | 🟢 35 met / 0 partial / 1 open | Headless render → shared buffer → paint protocol + geometry events + orientation-correct composite + engine-popup delivery |
| Input Bridging | cavekit-input-bridging.md | 8 | 🟢 40 met / 5 partial / 0 open | webOS pointer/key/touch/gesture → DOM events |
| Navigation, Loading & Events | cavekit-navigation-events.md | 7 | ✅ 18 met / 0 partial / 0 open | Nav commands + load/location/title/history message stream |
| Add-ons & Extensions | cavekit-addons-extensions.md | 10 | 🟢 49 met / 2 partial / 1 open | `about:addons` + classic XPI + NPAPI plugin support |
| Browser Services | cavekit-browser-services.md | 5 | ✅ 22 met / 0 partial / 0 open | Settings, cookies/cache, JS dialogs, downloads, TLS |
| Desktop Build & PoC Harness | cavekit-desktop-build.md | 4 | ✅ 11 met / 0 partial / 0 open | Phase-1 x86_64 build + YAP test client + end-to-end gate |
| Device Build & Packaging | cavekit-device-build.md | 11 | 🟢 68 met / 5 partial / 1 open | Phase-2 ARM cross-toolchain; self-contained packaging via bitbake; TouchPad + TouchPad Go |
| GRE Widget Bindings | cavekit-gre-widgets.md | 7 | 🟡 20 met / 6 partial / 0 open | Widgets the engine already gives us, used or explicitly declined |
| In-Browser Preferences | cavekit-preferences-ui.md | 9 | 🟢 39 met / 2 partial / 0 open | An engine-side settings page the three shells can share |
| Licensing & Branding | cavekit-licensing-branding.md | 5 | ✅ 12 met / 0 partial / 0 open | Apache+MPL headers, NOTICE, trademark stripping (cross-cut) |
| Tabbed Browsing | cavekit-tabbed-browsing.md | 6 | ⬜ 0 met / 0 partial / 27 open | Several pages per card with a tab strip; pages held in the daemon, tab commands over the variant's own Luna service, YAP contract untouched |
| Personas UI | cavekit-personas-ui.md | 9 | ✅ 46 met / 0 partial / 0 open | about:addons → Personas, about:personas, a licensed bundled set |
| Look of Jihad's pages | cavekit-chrome-look.md | 4 | 🟡 11 met / 4 partial / 0 open | Settings, about:personas and about:addons as part of each card |
| Engine Update Readiness | cavekit-engine-update.md | 9 | 🟢 25 met / 1 partial / 0 open | Make the UXP/Goanna interaction modular and maintainable so an engine update is a procedure, not an excavation |
| More Persona Formats | cavekit-persona-formats.md | 4 | 🟢 19 met / 3 partial / 0 open | Colour-only themes, layered pictures, more colour slots (owner's choice after the format survey) |
| Add-on Site | cavekit-addons-site.md | 13 | 🟢 56 met / 3 partial / 0 open | addons.riverstonerelay.org on Phoebus: Personas first, owner-only, Palm-era skin; about:personas reads its feed |
| Proxies & Certificates | cavekit-proxies-certs.md | 6 | 🟢 24 met / 2 partial / 0 open | Settings → Proxy (none, webOS's setting, the Relay, WebOne, own, PAC); webOS's added and patched CAs trusted; a proxy's CA asked when opened; the Relay's CA an option, off |
| Certificate Trust | cavekit-certificate-trust.md | 7 | ✅ 20 met / 0 partial / 0 open | Which certificates the browser trusts: the device's own store and the user's choices |
| Finger Emulation | cavekit-finger-emulation.md | 4 | 🟢 12 met / 2 partial / 1 open | An emulated finger that sends the touch panel's own datagrams to LunaSysMgr, and the round of finger tests it drives. |
| Device Headroom and YouTube | cavekit-device-headroom.md | 5 | 🟡 4 met / 0 partial / 11 open | The device kept reachable whatever a page does; YouTube measured, lighter, cheap to play |
| Flash, Finished | cavekit-flash.md | 14 | 🟡 27 met / 1 partial / 42 open | The device's Flash Player proven on real games, apps and videos in every shell, by the emulated finger |
| Autonomous Finger Testing | cavekit-finger-testing.md | 7 | 🟢 18 met / 0 partial / 2 open | The finger round judged by instruments a machine reads, not by an eye on a still |
| Low-Power Engine Tuning | cavekit-lowpower-uxp.md | 6 | ⬜ 0 met / 0 partial / 19 open | Each candidate lever (JS helper threads for two cores, -O2, the allocator's retained memory, memory prefs, reduced motion, lazy images, decoders, omni.ja) measured on the TouchPad against one baseline, kept only where it gains |
| The Engine's Next Release | cavekit-engine-release-2026-10.md | 4 | 🟢 11 met / 1 partial / 0 open | The engine moved to UXP's next release with the update procedure: the queue rebased, the roots re-audited, the device re-checked where the new commits touch |
| The webOS SDK at Its Latest | cavekit-webos-sdk-update.md | 3 | ⬜ 0 met / 0 partial / 7 open | The SDK submodule and its novacom built from it at the latest release, proven on the bench; the machine's own SDK refreshed |
| Global Jihad (after Jihad Browser) | cavekit-global-jihad.md | 2 | ⬜ 0 met / 0 partial / 5 open | Goanna in place of WebKit and Chromium for every app across the webOS family, replacing the system engine, never beside it. Will support: TouchPad (stock 3.0.x, CE), the 1.x/2.x phones, LuneOS, webOS OSE, rooted LG TVs. Won't: Lunacy, Open webOS, Android/iOS generally |
| The Atlas References, Current | cavekit-atlas-references.md | 3 | ⬜ 0 met / 0 partial / 9 open | The Atlas reference checkouts tracking their real upstreams, one update command, what moved weighed for Jihad |

### Driving the device

How-tos for the card-JS loop and the inject channel: `docs/DRIVING-THE-DEVICE.md`.

## Cross-Reference Map
| Domain A | Interacts With | Interaction Type |
|----------|----------------|------------------|
| UI Shell (Enyo) | IPC Contract | uses contract (client) |
| UI Shell (Enyo) | Navigation/Events | drives navigation, observes events |
| Mochi UI Variant | IPC Contract / UI Shell | same contract; parity with Enyo UI |
| Mochi UI Variant | Device Build | second .ipk packaged + on both TouchPad models |
| Mojo UI Variant | IPC Contract / UI Shell | same contract; behavioral baseline is the Enyo shell |
| Tabbed Browsing | IPC Contract / all three shells / Browser Services / Preferences | adds Luna methods on the own service (R4 precedent), never YAP; strips in each shell; held dialogs via the dialog sink; prefs on the settings page |
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
- The three UI variants share the BrowserAdapter contract and were built in parallel with the engine work.

## Changelog
- 2026-11-07: swept (the owner: "do a sweep to remove irrelevant or duplicate information in readme and kits"): the
  table recounted from the kits, the superseded state and the changelog to `archive/overview-history.md` verbatim,
  the driving how-tos to `docs/DRIVING-THE-DEVICE.md`.
