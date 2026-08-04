<p align="center">
  <img src="docs/jihad-browser-logo.png" alt="Jihad Browser logo" width="200">
</p>

# Jihad Browser

<p align="center"><em>Goanna inside, webOS alive, inshallah.</em></p>

A web browser for **legacy Palm webOS 3.0.x** (HP TouchPad), built by porting the
**UXP / Goanna** rendering engine into the **isis-browser** front-end. The Enyo UI
shell is kept as-is; only the rendering core is swapped from QtWebKit to Goanna.

> Status: **interactively usable on the HP TouchPad as a self-contained app, on all
> three front-ends.** The Goanna engine cross-builds for webOS 3 ARMv7; the daemon
> renders real web pages on-device (http + HTTPS) into the isis UI. Working
> on-device: address-bar navigation, back/forward/reload/stop, link + button taps,
> typing via the on-screen keyboard (Enter-to-search / form submit), engine-driven
> repaint (no more stale frames), **portrait ↔ landscape rotation**, **pinch/fit zoom
> (real magnification + full-page visual-viewport pan in both axes)**, **scrolling**,
> **long-press context menus**, **`<select>` dropdowns**, and load completion. All
> three UI variants ship and all three are live on hardware: the **Enyo 1.0** shell,
> an **Enyo 2 + Mochi** shell, and a **Mojo** shell — each its own `.ipk` with its own
> daemon, and each opening on the same branded start page. Jihad installs as a
> **self-contained browser that coexists with the stock webOS browser** — its own
> NPAPI MIME, adapter, render daemon, and upstart job; nothing system-level is
> replaced (see below).
>
> **Add-ons infrastructure (2026-08)** — `about:addons` and `about:plugins` both open and render
> on-device. `about:addons` had been dead on a missing `chrome://branding/` package (the branding
> package is supplied by the *application* in Firefox/Pale Moon, and this build embeds the GRE with
> no application above it), fixed by shipping our own. The add-on pref set the reference forks rely
> on now ships, and Pale Moon's dual-GUID `UXP_APPCOMPAT_GUID` mechanism is built in — without it our
> frozen app ID is named by zero existing extensions, so every real XPI would arrive `appDisabled`.
> NPAPI is compiled in and `nsPluginHost` scans, but **windowless NPAPI does not exist in a
> cairo-headless build** and must be ported, so binary plugins (Flash) are not yet possible.
>
> **Extensions work end to end (2026-08).** A page's `InstallTrigger.install()` raises a confirm
> on the card, accepting it installs the XPI into that variant's own profile, `about:addons` lists
> it, and the add-on's effect is visible on real pages. Enable, disable and remove all work from
> the real `about:addons` controls and survive a restart. Extensions are per-variant: each app has
> its own profile, and nothing lands on the user's storage volume. Getting there needed two engine
> assumptions removed — `amInstallTrigger` and `AddonManager` both expect a chrome `<browser>`
> above the content, which this embedding does not have — and the XUL `<menupopup>` overlay
> composite, since a popup is a **separate display root** the offscreen capture never contains and
> the content document cannot hit-test.
>
> **Known issues, on-device:** the fit-zoom on `about:addons` is unreliable under a real pinch
> (the fit-zoom itself is correct; the card re-asserts its own zoom, so this needs a gesture, not
> an injected value). `<optgroup>` header rows in `<select>` popups need a reply-index remap.
> Windowless NPAPI still does not exist in a cairo-headless build, so binary plugins (Flash)
> remain impossible without a port. Cookies now persist across restarts, and the chrome-icon
> latency did not reproduce on re-test.
>
> **Deployment reality:** all three variants are live on the test device simultaneously — cold boot
> auto-starts three daemons on three sockets, and the independence harness
> (`device-independence-test.sh`) passes 24/24. Per-variant independence is therefore verified both
> in the packaging/uninstall matrix and with all three running side by side.
>
> A latent daemon crash (a `tick()` re-entrancy use-after-free that made complex
> pages "overload" into a stuck loading screen) was root-caused from an on-device
> core dump and fixed. History, bookmarks, and downloads are wired to the app's own
> db8 kinds + the system download manager in the two Enyo-family shells; the Mojo
> shell registers no db8 kinds and keeps its own history in card-local storage.
> Remaining work: the VKB viewport "snap"/white-band on keyboard toggle, the windowless
> NPAPI port, and TouchPad Go hardware verification.
>
> **The daemon shuts down cleanly (2026-08-04).** It had no SIGTERM handler — the signal
> `stop <job>` sends — so it was killed before the engine's deferred savers (the add-on database,
> prefs) could flush, and a change made in the UI could be lost. Fixing that exposed two crashes
> on a teardown path the process had literally never taken.
>
> **Rotation** and **zoom** are both fixed (2026-07): the landscape "3× tiling" was a
> LunaCE fixed-stride read of the raw plugin surface — resolved by compositing the
> offscreen through the device's transform-aware Piranha `PGContext` (the Atlas
> approach). Zoom now magnifies in the offscreen `RenderDocument` capture (gfxContext
> scale) and pans the full page via document-relative rendering. **Scrolling** followed
> in 2026-08: the daemon paints viewport + direction-biased overscan (capped at the
> SGX 2048-row texture limit) and stamps honest per-frame geometry, so panning no
> longer exposes undrawn strips. All three were verified on the TouchPad and
> adversarially reviewed before deploy.

## Why

isis-browser (the open-webOS browser) renders with an ancient QtWebKit that no
longer handles the modern web. Goanna (the Pale Moon engine, a hard fork of
Gecko ESR-52) is far more capable while still buildable for constrained ARM
devices. Jihad Browser keeps the familiar webOS browser UX and replaces the
engine underneath it.

## Architecture

The original isis architecture is a client–server split:

```
isis-browser (Enyo JS UI)
   │  callBrowserAdapter(method, args)  /  PalmServiceBridge → palm://com.palm.browserServer/*
   ▼
BrowserAdapter (NPAPI plugin in the sysmgr card)
   │  YAP RPC over a socket  +  shared-memory framebuffer (double-buffered)
   ▼
BrowserServer (headless render daemon)
   │  [ QtWebKit backend ]   ← removed
   │  [ Goanna  backend  ]   ← added by Jihad Browser
```

The **YAP IPC contract** (≈80 async commands, 1 sync command, ≈60 async
messages, plus a shared-memory framebuffer signalled by `msgPainted`) is held
**byte-for-byte identical** — no command or message is added, removed, renamed,
or re-typed. Only `BrowserPage`'s internals are reimplemented: instead of a
`QGraphicsWebView`, the Goanna backend drives an `nsIWebBrowser` rendering into
an offscreen widget, copies the result into the shared framebuffer, and emits the
same YAP messages from Goanna's progress/URI/embedding listeners.

### Three independent apps, coexisting with the stock browser

Jihad ships as **three standalone browsers** — an Enyo 1.0 shell, an Enyo 2 + Mochi
shell, and a Mojo shell — each a complete, self-contained `.ipk`. They run alongside
the stock webOS browser and alongside *each other*: the system `BrowserAdapter.so`,
the `browserserver` job, and the stock browser are never touched, and **no component
is shared between the three variants**, so installing, upgrading, or removing any one
of them cannot affect another.

| Piece | Enyo | Mochi | Mojo | Stock (untouched) |
|-------|------|-------|------|-------------------|
| NPAPI MIME | `application/x-jihad-browser` | `…-mochi` | `…-mojo` | `application/x-palm-browser` |
| Adapter plugin | `BrowserAdapterJihad.so` | `…Mochi.so` | `…Mojo.so` | `BrowserAdapter.so` |
| Daemon socket | `/tmp/yapserver.jihad-browser` | `…-mochi` | `…-mojo` | `/tmp/yapserver.browser` |
| Upstart job | `jihad` | `jihad-mochi` | `jihad-mojo` | `browserserver` |

Each shell swaps **only its own** WebView's plugin type to its own MIME, so its card
loads its own adapter → its own daemon → Goanna. The adapter therefore carries a
**two-line rebrand** (MIME string + YAP server name); the YAP command/message interface
it speaks is still byte-identical.

**Good-citizen install contract.** Following Atlas, each package runs its engine and
daemon **in place from its own app bundle** on `/media/cryptofs`, and writes **nothing**
to `/media/internal` — that volume is the user's USB storage and stays free of app
internals. The only files placed outside the app directory are the ones webOS forces:
webOS's WebKit scans `/usr/lib/BrowserPlugins` for NPAPI plugins at boot, so each variant
installs its own small adapter shim there, its own adapter impl under `/usr/lib/jihad/<variant>/`,
and its own upstart job in `/etc/event.d/`. Runtime state (log, engine profile/cache) lives
under `/var/palm/jihad/<variant>/`. Every one of those paths is variant-namespaced and removed
exactly by `prerm` — verified by a before/after filesystem and checksum diff
(`build/webos-oe/device-citizen-audit.sh`). Full contract: `context/kits/cavekit-device-build.md`
R7/R8, with the naming table in `context/plans/plan-variant-identity.md`.

See `context/` (kits) for the full requirement decomposition and
`render/goanna/PORT-MAP.md` for the per-command mapping from the YAP contract to
Goanna calls.

## Repository layout

| Path                    | Contents |
|-------------------------|----------|
| `app/`                  | UI variant 1 — forked, rebranded isis-browser **Enyo 1.0** shell (Apache-2.0). |
| `app-mochi/`            | UI variant 2 — **Enyo 2 + Mochi** shell, same contract, separate `.ipk` (Apache-2.0). |
| `app-mojo/`             | UI variant 3 — **Mojo** shell on the system framework, same contract, separate `.ipk` (Apache-2.0). |
| `packaging/`            | Per-variant `postinst`/`prerm`/upstart jobs, generated from one template + one identity table (`gen-variant-scripts.sh --check` proves the checked-in copies have not drifted). |
| `render/browserserver/` | BrowserServer/Adapter-derived daemon + IPC layer; engine-agnostic parts kept, QtWebKit `BrowserPage` replaced (Apache-2.0). |
| `render/goanna/`        | New Goanna backend: `nsIWebBrowser` driver, offscreen widget, YAP bridge (MPL-2.0). |
| `third_party/`          | Out-of-tree upstreams as pinned **git submodules** (populate with `git submodule update --init`): **`uxp`** = UXP/Goanna engine (Pale Moon; Jihad's engine mods are patches in `build/desktop/patches/`, see `docs/UXP.md`); **`mochi`** + **`mochi-sampler`** = webOSArchive Mochi/Enyo-2 for the Mochi UI `.ipk`; **`enyo-layout`** = `enyojs/layout` 2.5.2. All updatable from upstream by bumping the pin. |
| `build/desktop/`        | x86_64 Linux build wiring (Phase 1 PoC). |
| `build/webos-oe/`       | webOS 3 ARMv7 device build: the direct cross-build scripts (verified pipeline) **and** the full OpenEmbedded path — `oe-env.sh` stands up the OE "dylan" host on any Linux via `chroot` (no container; sudo/doas), see `docs/OE-BUILD.md`. |
| `context/`              | Cavekit kits + build site (the plan). |
| `docs/`                 | Design notes, IPC contract reference, toolchain notes. |
| `licenses/`             | Full Apache-2.0 and MPL-2.0 texts. |

## Roadmap

- **Phase 0 — Plan & scaffold — ✅ done:** Cavekit kits + build site; project skeleton; license setup; YAP contract captured; port map drafted.
- **Phase 1 — Desktop PoC (x86_64 Linux) — ✅ done:** Build Goanna once; bring up `BrowserServer` with the Goanna backend; render a page into the shared framebuffer; drive it from the isis UI on desktop. De-risked engine integration before the embedded toolchain.
- **Phase 2 — webOS 3 cross-build (ARMv7) — ✅ reached:** modern cross-toolchain stood up (stock gcc 4.4 cannot build UXP); Goanna + daemon cross-compiled; the self-contained adapter + daemon + upstart job deploy and **render real pages on the TouchPad**. Both `.ipk`s build via a single entry (`build/webos-oe/build-all-device.sh`); TouchPad Go (Opal) machine config authored (shared ARMv7 softfp binary; install pending hardware).
- **Phase 2.5 — Three independent packages + install-footprint contract — ✅ all three verified on device:** each front-end ships as a standalone `.ipk` with its own MIME/adapter/socket/upstart job/daemon (nothing co-owned, so nothing to refcount); the engine runs **in place from the app's cryptofs bundle** and nothing is written to the user's `/media/internal` volume; `build/webos-oe/build-variant-ipk.sh` produces all three without bitbake, and `device-citizen-audit.sh` + `device-independence-test.sh` (24/24 with all three installed and running) are the acceptance evidence.
- **Phase 3.5 — Add-ons + engine-repaint correctness — 🔧 in progress (2026-08):** `about:addons`/`about:plugins` open; branding package, add-on prefs and the dual-GUID AppCompat mechanism ship; the engine-driven repaint loop was found **inert** (invalidations landed on a child `PuppetWidget` the daemon never polled) and fixed, which restored all post-load repainting — SPA updates, animations, late images, XHR content; input dispatch moved off the YAP socket callback into the guarded pump and was **verified on hardware with a physical tap** (one tap → one activation, no double form POST). Adversarially reviewed against Pale Moon and Basilisk (cavekit-addons-extensions R8).
- **Phase 3 — Hardening — 🔧 in progress:** DONE — daemon crash fix (tick re-entrancy), engine-driven repaint (no stale frames), engine-driven VKB focus, crash-safe form submission, history/bookmarks/downloads storage, branded start pages on all three shells, self-drive test harness, **portrait ↔ landscape rotation (PGContext composite)**, **pinch/fit zoom — magnify + full-page visual-viewport pan (document-relative render)**, **scroll pan headroom (viewport+overscan paint)**, **long-press `contextmenu`** (the daemon's hit-test round-trip, which the adapter gates every long-press on, had been a stub), **input coordinate mapping** (below-the-fold taps landed a screenful low), **`<select>` dropdowns on all three shells**, the **card-JS dev loop** (`push-card-js.sh`, reloads proven by a runtime stamp), and the Mojo shell's chrome actions (new card / history / share). REMAINING — the **`<menupopup>` overlay composite** (tools menu + context menus), XPI install wiring, chrome-icon repaint latency, cookie/cache persistence across restarts, VKB viewport "snap"/white-band, memory-budget measurement, and TouchPad Go on-device verification.

## Licensing

Composite work: Apache-2.0 (UI + IPC daemon) + MPL-2.0 (Goanna backend + engine).
The two are compatible. All Pale Moon / Basilisk / Moonchild trademarks are
removed from the engine build. See `LICENSE` and `NOTICE`.
