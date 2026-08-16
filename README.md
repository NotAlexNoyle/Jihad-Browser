<p align="center">
  <img src="docs/jihad-browser-logo.png" alt="Jihad Browser logo" width="200">
</p>

# Jihad Browser

<p align="center"><em>Goanna inside, webOS alive, inshallah.</em></p>

A modern web browser for the **HP TouchPad** and webOS 3.0.x — the browser that still runs
**Adobe Flash** and **browser extensions** on 2011 hardware.

Jihad keeps the webOS browser you already know and swaps the engine underneath it for
**UXP / Goanna**, the Pale Moon engine. It installs **alongside** the stock browser and never
replaces it, so nothing you rely on breaks.

---

## What only Jihad does

Every one of these is a webOS first — no other browser for the platform, past or present, has
had it:

- **Browser extensions.** Install a real XPI add-on straight from a web page, manage it in
  `about:addons`, enable/disable/remove it, and keep it across restarts. Each app keeps its own
  add-ons. No webOS browser before Jihad has ever run extensions.
- **Flash *and* the modern web in one engine.** Goanna renders today's sites and loads the
  TouchPad's own `libflashplayer.so` in-process — animation, touch, keyboard, and **sound**. The
  stock browser has Flash but cannot open a modern site; every modern-engine browser dropped NPAPI,
  so none of them can run Flash at all. Jihad is the only one that does both, and a crashing
  plugin cannot take the browser down with it.
- **A real settings surface.** `about:preferences` for everyday options and `about:config` for
  everything else — the full Gecko/Goanna preference system, on a 2011 tablet.
- **Your choice of three interfaces.** The classic **Enyo 1** shell, a modern **Enyo 2 / Mochi**
  shell, or a lightweight **Mojo** shell — three independent apps that install side by side.
- **Modern TLS with the device's own trust store.** Current HTTPS, an SSL-exception dialog for an
  untrusted certificate, and accepted certificates written into the platform certificate store.

Jihad installs **alongside** the stock browser and never replaces it, runs entirely from its own
bundle, and writes nothing to your USB storage volume — so nothing you rely on breaks.

---

## What you get

- **Flash that works.** Animated content at ~30 fps composite, with audio, mouse and keyboard.
  A crashing plugin cannot take the browser down with it.
- **Real extensions.** Install an XPI from a web page, manage it in `about:addons`, enable,
  disable and remove it — and it survives a restart. Each browser keeps its own add-ons.
- **A real settings page.** `about:preferences` for everyday options, `about:config` for
  everything else.
- **The webOS gestures you expect** — pinch and fit zoom, scrolling, portrait ↔ landscape
  rotation, long-press context menus, dropdowns, and the on-screen keyboard.
- **Three interfaces, your choice** — the classic **Enyo 1** shell, a modern **Enyo 2 / Mochi**
  shell, or a lightweight **Mojo** shell. Each is a separate app; install one, or all three.
- **A good guest on your device.** Each app runs entirely from its own bundle, writes nothing to
  your USB storage volume, and removes itself cleanly. The stock browser is never touched.

## Install

Install the `.ipk` for the interface you want with **Preware** or **WebOS Quick Install**:

| Interface | Package |
|---|---|
| Enyo 1 (classic) | `net.riverstonerelay.jihad-browser` |
| Enyo 2 / Mochi | `net.riverstonerelay.jihad-browser-mochi` |
| Mojo (lightweight) | `net.riverstonerelay.jihad-browser-mojo` |

They are independent — installing, updating or removing one never affects another or the stock
browser.

## Honest status

Jihad is usable day to day on real hardware, and it is still being worked on. Two things are
known to be imperfect right now:

- **Motion is capped by software rendering.** The engine paints offscreen in software and the
  card composites the result, so animation — Flash, media-control scrubbers, CSS — tops out at
  roughly 25–30 fps. A recent compositor fix removed the frame *ghosting* that made moving
  elements skip and flash (verified on the media controls; the same paint path serves Flash), so
  what is left is an even but hardware-limited frame rate, not the uneven spacing it used to be.
- **Physical-keyboard behaviour with Flash is unverified**, because the test device has no
  keyboard to verify it with.

Per-feature status lives with the requirements in `context/kits/` — that is the single source of
truth, deliberately not restated here so the two cannot drift apart. Current work in progress is
in `docs/PICKUP.md`.

## For developers

Jihad keeps isis-browser's client–server split and holds the **YAP IPC contract byte-identical** —
the Enyo UI talks to a `BrowserAdapter` NPAPI plugin in the card, which talks over YAP plus a
shared-memory framebuffer to a headless render daemon. Only the daemon's rendering backend is
replaced: QtWebKit out, Goanna in.

| Path | Contents |
|---|---|
| `app/`, `app-mochi/`, `app-mojo/` | the three UI shells (Apache-2.0) |
| `render/browserserver/` | engine-agnostic daemon + YAP IPC (Apache-2.0) |
| `render/goanna/` | the Goanna backend (MPL-2.0) |
| `third_party/uxp` | the engine, a submodule pinned to the `jihad-engine-mods` commit (pristine UXP + our delta); the desktop build applies the same delta as patches in `build/desktop/patches/` |
| `build/desktop/`, `build/webos-oe/` | x86_64 and webOS ARMv7 build wiring |
| `context/` | Cavekit kits (requirements) and the build site (plan) |
| `docs/` | build, toolchain and IPC reference; `PICKUP.md` is the current handoff |

Building for the device: `docs/DEVICE-BUILD.md`. Engine notes: `docs/UXP.md`. The IPC contract:
`docs/IPC-CONTRACT.md`.

## Licensing

Composite work: **Apache-2.0** (UI + IPC daemon) and **MPL-2.0** (Goanna backend + engine), which
are compatible. All Pale Moon / Basilisk / Moonchild trademarks are removed from the engine build.
See `LICENSE` and `NOTICE`.
