# Device Handoff — pick up the `.ipk` / on-device track here

*Written 2026-07-01 for the next session (which will have the `webos-mcp`
knowledge loaded). Load `webos://knowledge/all` first, then read this.*

## TL;DR

The **desktop backend is complete + verified + adversarially reviewed (4× Codex)**
— 52 commits. The remaining work is the **device build**. The TouchPad is now
connected and the **Jihad Enyo UI is already installed on it**. The one hard
blocker for a full Goanna-on-device build is a **modern GCC-9+ ARMv7
cross-toolchain** targeting the device's 2011 glibc; the PDK's gcc 4.4 cannot
build UXP.

## What is DONE (Phase 1, desktop — see impl-overview.md for the test names)

Engine embed + real-web render, full YAP round-trip + PoC image, input
(click/key/mouse + coord-mapping + holdAt + drag), navigation R1–R6 (incl
failed-load, redirect, addUrlRedirect, link-clicked, history), geometry R4–R5,
services (settings/cache/cookies/dialogs/downloads/TLS-detect), freeze/thaw,
create/destroy leak cycle, the **branding strip** (0 Pale Moon/Basilisk strings
in shipped artifacts), and licensing docs. All proven by `build/desktop/build-*-test.sh`.

## Device facts (verified on-device this session, via novacom)

- **HP webOS 3.0.5**, `Nova-HP-Topaz` build 86 (2011-12-21), **ARMv7l**, kernel
  `2.6.35-palm-tenderloin`. It's the **TouchPad (topaz/tenderloin)**.
- `novacom -l` → `topaz-linux` connected over USB; `novacomd` running.
- Stock render stack present + running: `/usr/bin/BrowserServer -d 30000`
  (PID seen), `/usr/bin/BrowserServerMojo`, `/usr/lib/BrowserPlugins/BrowserAdapter.so`
  (Dec 2011 stock). This is the exact BrowserAdapter↔BrowserServer architecture
  Jihad targets — the YAP contract is byte-identical.
- Installed browser apps coexist: `com.omww.com.android.browser`,
  `org.webosinternals.browser-tls13`, and now **`net.riverstonerelay.jihad`**.

## What was DONE on-device this session

- Packaged the Enyo UI: `palm-package app/ -o <out>` →
  `net.riverstonerelay.jihad_1.0.0_all.ipk` (776 KB). **Builds cleanly.**
- `palm-install`ed it on the TouchPad; **it installs and coexists** with the
  other browsers (device-build R3, "installs + can coexist"). Launched with
  `palm-launch` (process started).
- **Not yet visually confirmed rendering** — verifying the UI actually drives the
  running `BrowserServer` and paints a page on-screen is the next concrete step
  (device-build R4). The user rejected a raw `/dev/fb0` grab; use a proper webOS
  screen-capture path (see the webos-mcp knowledge) or drive + read `palm-log`.

## Toolchains present / absent

- **PalmSDK (JS/Enyo)** at `/opt/PalmSDK` — `palm-package/install/launch/log`,
  `novacom`. WORKS (used it to build + install the UI .ipk).
- **PDK / arm-gcc** — a full-disk search (`find / -xdev -name 'arm-none-linux-gnueabi-gcc'`)
  found **nothing**. The user says the SDK includes an arm-gcc; if so, get its
  path. NOTE: that toolchain is **CodeSourcery gcc 4.4.3 (C++98)** and **cannot
  build UXP/Goanna** (needs GCC ≥ 9.1 / C++14). Its value is the **device sysroot**
  + building gcc-4.4-era native pieces (e.g. the BrowserAdapter).
- **`webos-mcp`** — installed globally (`claude mcp add webos-mcp -s user -- npx -y webos-mcp`),
  `✔ Connected`. Documentation-only (does NOT ship a toolchain).

## The device-build decision the user made

The user chose **"Use the PDK (tell me the path)"** — i.e. pull the device sysroot
from the PDK + build the gcc-4.4-compatible native pieces (BrowserAdapter). **Still
need the PDK path from the user** (search came up empty).

## Next steps (in order)

1. **Get the webOS knowledge** (`webos://knowledge/all`) — do device work grounded
   in the real docs, not assumptions.
2. **Verify the installed Jihad UI on-device (device-build R4):** launch it, drive
   a URL, and confirm it renders via the running `BrowserServer` (use a proper
   screen-capture service per the webos-mcp docs, and `palm-log -f
   net.riverstonerelay.jihad`). This closes UI-Shell R4 too (app launches, URL →
   openUrl, back/fwd/reload/stop, findInPage) — all without cross-compiling.
3. **Get the PDK path** from the user → pull the **device sysroot**; optionally
   rebuild the BrowserAdapter against the Jihad daemon (R5), and compile a trivial
   C++ PDK binary to confirm the toolchain runs on-device (R1 feasibility).
4. **Also pull the sysroot straight off the device** if the PDK is unavailable:
   `novacom get file:///usr/lib/...`, `/lib/...`, and headers if present. A modern
   GCC-9 ARMv7 cross-compiler + that sysroot is the path to cross-building libxul
   (`build/webos-oe/mozconfig.goanna-arm` is authored for this).
5. **Goanna daemon cross-build (the big one):** stand up a modern GCC-9 ARMv7
   cross-toolchain matching the device glibc (crosstool-ng, or a Bootlin/Linaro
   armv7-eabihf toolchain pinned to the device's glibc), then run the OE recipes
   in `build/webos-oe/recipes-jihad/` (or a direct cross-build mirroring
   `build/desktop/build-daemon.sh`). Deploy `jihad-browserserver` + `libxul.so` to
   the device and point the adapter at it. This is what makes the FULL Jihad
   browser (Goanna rendering) run on the TouchPad.

## Key paths + commands

- UI source: `Jihad-Browser/app/` (Enyo 1.0), `Jihad-Browser/app-mochi/` (Mochi).
- Engine build (desktop, works): `build/desktop/` + the pinned podman image
  `jihad-goanna-build`; UXP checkout at `../UXP` (pinned rev in
  `docs/ENGINE-SOURCE.md`).
- ARM device build scaffolding: `build/webos-oe/mozconfig.goanna-arm` +
  `build/webos-oe/recipes-jihad/`; full track in `docs/DEVICE-BUILD.md`.
- Remove the app if needed: `palm-install -r net.riverstonerelay.jihad`.
