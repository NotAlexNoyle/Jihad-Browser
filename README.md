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

## Why Jihad

The TouchPad has three browsers worth considering. They are good at different things.

| | **stock / isis** | **Atlas** | **Jihad** |
|---|---|---|---|
| Engine | QtWebKit (2011) | WPE WebKit 2.52 | UXP / Goanna |
| Modern sites | ✗ | ✓ | ✓ |
| **Browser extensions** | ✗ | ✗ | **✓ — install real XPI add-ons** |
| **Adobe Flash** | ✓ (system plugin) | ✗ | **✓ — the device's own Flash, in a modern engine** |
| Full preferences UI | ✗ | ✗ | **✓ — `about:preferences` + `about:config`** |
| Choice of interface | Enyo 1 | Enyo 1 | **Enyo 1, Enyo 2/Mochi, or Mojo** |
| Keeps your USB volume clean | — | engine on `/media/internal` | **nothing written to `/media/internal`** |
| Coexists with stock browser | — | ✓ | ✓ |

**Where Atlas is stronger:** it has a JIT'd ES2022 JavaScript engine and more mature text
selection and clipboard handling. If you mainly want raw speed on modern JavaScript, look at
[Atlas](https://github.com/Herrie82) — it is an excellent browser and Jihad borrowed its
good-citizen install model and its Piranha compositing approach outright.

**Where Jihad is unique:** it is the only modern-engine TouchPad browser that runs **Flash**
and **extensions**. Modern WebKit dropped NPAPI years ago, so Flash content — the games, the
players, the embedded video that make up a lot of what is left of the 2011 web — cannot run
there at all. Jihad loads the TouchPad's own `libflashplayer.so` inside Goanna: it renders
animation, takes touch and keyboard input, and **plays sound**.

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
| Enyo 2 / Mochi | `net.riverstonerelay.jihad-browser.mochi` |
| Mojo (lightweight) | `net.riverstonerelay.jihad-browser.mojo` |

They are independent — installing, updating or removing one never affects another or the stock
browser.

## Honest status

Jihad is usable day to day on real hardware, and it is still being worked on. Two things are
known to be imperfect right now:

- **Flash animation is not perfectly smooth yet.** It runs at the right average frame rate, but
  frame *spacing* is still uneven enough to notice next to the stock browser.
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
| `third_party/uxp` | the engine, as a pinned submodule; our changes are patches in `build/desktop/patches/` |
| `build/desktop/`, `build/webos-oe/` | x86_64 and webOS ARMv7 build wiring |
| `context/` | Cavekit kits (requirements) and the build site (plan) |
| `docs/` | build, toolchain and IPC reference; `PICKUP.md` is the current handoff |

Building for the device: `docs/DEVICE-BUILD.md`. Engine notes: `docs/UXP.md`. The IPC contract:
`docs/IPC-CONTRACT.md`.

## Licensing

Composite work: **Apache-2.0** (UI + IPC daemon) and **MPL-2.0** (Goanna backend + engine), which
are compatible. All Pale Moon / Basilisk / Moonchild trademarks are removed from the engine build.
See `LICENSE` and `NOTICE`.
