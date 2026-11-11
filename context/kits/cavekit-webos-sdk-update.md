---
created: "2026-10-26"
last_edited: "2026-11-10"
complexity: quick
---

# Cavekit: The webOS SDK at Its Latest

## Scope
The owner, 2026-10-26: **"update the webos sdk to the latest."**

What "the webOS SDK" is in this repository (read 2026-10-26):
- **The SDK's command-line tools** (palm-package, palm-install, palm-launch, palm-log; Java wrappers around
  `webos-tools.jar`) and **novacom/novacomd**: the submodule `third_party/webos-sdk-redux` (webOS Archive's
  community-maintained SDK, https://github.com/webOSArchive/webos-sdk-redux), pinned at `f165ba9` (2026-10-09, the
  0.4 tree). `build/webos-oe/host-path.sh` puts its `Current/bin` first on PATH; `tools/host/build-novacom.sh` builds
  novacom/novacomd from it into `tools/host/bin/`. Upstream main is `5349208` (2026-10-19), **release 0.4.1**: a
  Windows novacomd and setup wizard, macOS driver packages, and a +293-line `novacomd/src/host/usb-linux.c` (upstream
  A/B-tested it on Linux against a CE 3.1.0 TouchPad, no regressions). Known upstream issue: novacomd does not compile
  with gcc 15 (C23 `bool`); `-std=gnu17` works around it (this host has gcc 14.2.1).
- **The Palm PDK** (CodeSourcery gcc 4.3.3, the adapter's compiler): HP's final 3.0.5 build,
  `palm-sdk_3.0.5-svn528736-pho676_i386.deb`, sha256-pinned in `build/webos-oe/fetch-pdk.sh`. Nothing newer exists;
  the adapter's ABI is tied to it. It stays.
- Not the SDK: the project's own cross toolchain (crosstool-NG 1.25.0) and sysroot, and the webos-mcp docs server.

Reference: the research brief of 2026-10-26 (this session), `docs/DEVICE-BUILD.md`, cavekit-device-build.md R11.

## Requirements

### R1: The SDK submodule at upstream's latest release
**Acceptance Criteria:**
- [x] `third_party/webos-sdk-redux` is at upstream's 0.4.1 commit (`5349208` or the newest release commit on the day it
  is done), committed as the gitlink; `Current` resolves to the 0.4.1 tree *(2026-11-09: checked out at `5349208`,
  `Current -> ./0.4.1`; 2026-11-10: the owner's gitlink commit `5e4ace62`)*
- [x] The PDK pin (`fetch-pdk.sh`, its sha256) is unchanged and the adapter still builds with it
- [x] `docs/DEVICE-BUILD.md` and cavekit-device-build.md R11 name 0.4.1
**Dependencies:** none

### R2: The tools rebuilt and proven on the device
**Acceptance Criteria:**
- [x] `tools/host/build-novacom.sh` rebuilds novacom/novacomd from 0.4.1, passing `-std=gnu17` so a gcc 15 host also
  builds it
- [x] On the bench (CE 3.1.0): `novacom -l` lists the device; a `novacom run`, a `put`/`get` of a 40 MB file and a
  full `push-variant.sh` all succeed (one session at a time); `palm-package`, `palm-install` and `palm-launch` build,
  install and launch the three packages *(2026-11-09, T-549: all of it, the 0.4.1 client against the host's 0.4 novacomd,
  whose Linux code 0.4.1 did not change; the daemon swap is the owner's root step)*
- [ ] Stock 3.0.5 is not on the bench: its check is the owner's (asked before it is claimed)
**Dependencies:** R1

### R3: The machine's own SDK (outside the repository), if the owner wants it
**Acceptance Criteria:**
- [x] *(The owner, 2026-10-26: **yes, refresh it.**)* If the owner says so, `/opt/PalmSDK` is refreshed with 0.4.1's installer (a root step: the owner runs the command
  the session hands over), and `/usr/local/bin/palm-package` reports it; otherwise left as it is and noted *(2026-11-10,
  T-550: the owner ran the installer (choices 1/1): `/opt/PalmSDK/Current -> 0.4.1`, 17 `palm-*` links in `/usr/local/bin`,
  `palm-package` resolves into 0.4.1 (its tool version 3.0.5b38, as 0.4 shipped); 0.4 and older left beside it. The host's
  novacomd swapped for the 0.4.1 build the same evening (`/opt/nova/bin/novacomd`, byte-identical to
  `tools/host/bin/novacomd`), the TouchPad back online at once)*
**Dependencies:** R1

## Out of Scope
- A replacement for the proprietary PDK compiler (a crosstool-NG gcc 4.3 build): a separate decision
- The Windows and macOS parts of 0.4.1

## Cross-References
- cavekit-device-build.md R11 — the host tools and their pins
- cavekit-finger-emulation.md — its tools run over novacom

## Changelog
- 2026-11-10: R1 and R3 met (the owner's gitlink commit `5e4ace62`; T-550, the owner's two root steps: the installer and the novacomd swap). R2's stock 3.0.5 check waits for the stock TouchPad.
- 2026-11-09: R1 met but for the gitlink (the owner's), R2 met on the bench (T-548, T-549); R3's root command handed over (T-550).
- 2026-10-26: created from the owner's request and the day's research.
