# Jihad Browser — Device Build & `.ipk` Packaging (Phase 2)

*cavekit-device-build — the webOS 3 ARMv7 build for the HP TouchPad (Topaz/
tenderloin) and TouchPad Go (Opal).*

This is the **last** milestone: cross-compile the verified Phase-1 backend for
the device and package it as installable `.ipk`s. The scaffolding is authored
here; the parts that need the physical hardware / device sysroot / OE build
environment are marked **[gated]** and are the only things that cannot be
completed off-device.

## What is done (Phase 1, verified on desktop)

Everything the daemon does is built + adversarially reviewed (3× Codex) + proven
by runnable tests on x86_64: engine embed + real-web render, the full YAP
round-trip, input (click/key/mouse + coord-mapping), navigation (R1–R6),
geometry (R4–R5), services (settings/cache/cookies/dialogs/downloads/TLS-detect),
freeze/thaw, the branding strip, and the desktop PoC image. See
`docs/DESKTOP-POC.md` and `context/impl/impl-overview.md`.

The **`.ipk` build reuses those exact sources cross-targeted** — no new engine
integration is needed, only a toolchain + packaging.

## Layout (authored)

```
build/webos-oe/
  mozconfig.goanna-arm                 # ARMv7 hard-float/NEON engine mozconfig
  recipes-jihad/
    goanna/goanna_1.0.bb               # UXP engine, cross build (heavy)
    jihad-browserserver/…_1.0.bb       # daemon (Goanna backend), LunaService ON
    jihad-ui/net.riverstonerelay.jihad_1.0.bb        # Enyo 1.0 UI .ipk (app/)
    jihad-ui/net.riverstonerelay.jihad.mochi_1.0.bb  # Mochi UI .ipk (app-mochi/)
```

The recipes mirror the upstream isis recipes (`browserserver`, `browser-adapter`,
`com.palm.app.browser`) with the engine dependency swapped from
`webkit-webos`+`qt4-webos` to `goanna`. The **BrowserAdapter is reused unchanged**
(it only speaks YAP + blits shmem) — IPC-contract R5.

## The gates (what needs the hardware / SDK)

| Gate | Requirement | Kit |
|------|-------------|-----|
| **Toolchain sysroot** | A modern **C++14** cross-toolchain (stock webOS 3 gcc 4.4 cannot build UXP) that links against the **TouchPad's glibc/kernel ABI** so binaries don't need a newer glibc than the device has. Needs the webOS 3 device sysroot. | R1 |
| **Engine cross-build** | Run the `goanna` recipe with that toolchain (ARMv7-A + NEON, hard-float). Heavy (hours). Verifying "loads without missing-symbol/ABI errors" needs the device. | R2 |
| **OE environment** | An OpenEmbedded/`meta-webos` tree to `bitbake` the recipes into `.ipk`s (both UI variants). | R3 |
| **Physical device** | Install + run each `.ipk` on the **TouchPad** and **TouchPad Go**; verify launch, render-on-screen via the adapter, navigation, scroll, tap. Cert/dialog/download flows with device services. | R4, R6 `[human-review on device]` |
| **Memory budget** | Render-process memory within the 1 GB device budget; freeze/purge reclaim; no OOM in a browsing scenario. | R5 `[human-review on device]` |

None of these can be executed in this environment: there is **no device sysroot,
no OE tree, no network for the cross-toolchain, and no TouchPad**. They require
the user's hardware + SDK. This is the one milestone gated on the device in hand.

## Machine configs (R6)

Two ARMv7 webOS-3 machines, both supported by upstream isis:
- **TouchPad** — Topaz / `tenderloin` (APQ8060, 1024×768).
- **TouchPad Go** — Opal (smaller screen). Model-specific screen geometry /
  machine config is captured in the OE machine conf, not assumed identical.

## Runtime constraints

Goanna is heavier than the old QtWebKit; on a 1 GB device the mozconfig builds
`-Os`, strips libxul, and disables jemalloc for the platform allocator. Aggressive
`freeze`/`purgePage` (wired: freeze suppresses paint, thaw reattaches) reclaims
backgrounded cards. Deeper memory tuning is Phase 3.

## To build (once the gates are satisfied)

1. Stand up the cross-toolchain against the device sysroot (R1); verify a trivial
   C++14 binary runs on the device/emulator.
2. `bitbake goanna` (engine), then `bitbake jihad-browserserver`.
3. `bitbake net.riverstonerelay.jihad net.riverstonerelay.jihad.mochi` → two `.ipk`s.
4. Install both on the TouchPad + TouchPad Go; run the on-device checklist (R4/R6).
