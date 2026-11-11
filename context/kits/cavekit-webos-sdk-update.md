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

### R4: novacomd from the owner's fork
*(The owner, 2026-11-11: "at the next milestone where nothing is running on the touchpad I want to use
NotAlexNoyle/novacomd 1.2.1"; "current novacomd is installed at /usr/local/bin/novacomd and also exists in jihad browser
build make sure those get removed or replaced".)* The daemon comes from https://git.trueog.net/NotAlexNoyle/novacomd
(release 1.2.1, tag at `b8c48ef`). The client, novacom, stays the SDK's.
**Acceptance Criteria:**
- [x] `tools/host/build-novacom.sh` builds novacomd from the fork at tag 1.2.1, stopping if the tag is not at
  `b8c48ef`, stamped `novacomd-1.2.1`; novacom from the SDK submodule as before; each binary moved into `tools/host/bin/`
  by rename, so a running script never finds it missing *(2026-11-11)*
- [ ] The host's daemon is 1.2.1 on libusb-compat: `/opt/nova/bin/novacomd` (which `/usr/local/bin/novacomd` links to)
  is novacomd 1.2.1 linked against libusb-compat; the older novacomd copies in `/usr/local/bin` removed. A root step for
  the owner, run while nothing is running on the TouchPad *(2026-11-11: the release's `novacomd-1.2.1-linux-x86_64`
  installed at 04:29 and the two backups removed; at 04:36, under the Enyo self-test, it hit a USB timeout and never
  answered again: its USB thread blocked in `USBDEVFS_REAPURB` with no timeout, the main thread waiting on the channel
  open behind it (the capture: `/proc/<pid>/task/*/{wchan,syscall,stack}`). The cause is the original libusb 0.1.12
  that the release's Linux binaries have built in: after a timeout it cancels the transfer and then waits, with no
  timeout, for whichever transfer finishes next, while novacomd reads and writes from two threads. The same 1.2.1 built
  here against Void's libusb-compat 0.1.8 (`tools/host/bin/novacomd`, run by the owner since 04:49) passed the self-test
  three times through four USB timeouts. Open: `/opt/nova/bin/novacomd` replaced by that build until a 1.2.2 release
  carries libusb-compat; the fork's fix is staged for the owner (install script and release binaries on libusb-compat))*
- [x] On the bench with the 1.2.1 daemon: `novacom -l` lists the TouchPad; a `novacom run`, a `put`/`get` of a 40 MB file
  and a full `push-variant.sh` succeed *(2026-11-11 through the release binary: the 40 MB file both ways byte-identical
  in 10 s / 13 s, `push-variant.sh enyo` in 148 s with its payload's md5 checked; then `default-check.sh enyo` ×3
  through the libusb-compat build, faults 0)*
- [x] `tools/host/build-novacom.sh` refuses the original libusb 0.1.12: libusb-compat calls into libusb 1.0 and the
  original does not, so the library's text tells them apart (both report 0.1.12 to pkg-config); checked refusing on
  Debian 13's `libusb-dev` and passing on Void *(2026-11-11; dropped once the pin is a release whose install script
  builds libusb-compat itself)*
**Dependencies:** R2

## Out of Scope
- A replacement for the proprietary PDK compiler (a crosstool-NG gcc 4.3 build): a separate decision
- The Windows and macOS parts of 0.4.1

## Cross-References
- cavekit-device-build.md R11 — the host tools and their pins
- cavekit-finger-emulation.md — its tools run over novacom

## Changelog
- 2026-11-11 (morning): R4's swap done and undone in part: the release binary's built-in libusb 0.1.12 hung novacomd after a USB timeout; the libusb-compat build runs instead; `build-novacom.sh` refuses 0.1.12.
- 2026-11-11: R4 added (the owner: novacomd from NotAlexNoyle/novacomd 1.2.1, the old copies removed or replaced); `build-novacom.sh` builds it from the tag.
- 2026-11-10 (late): upstream re-read: no release after 0.4.1 (the newest tag `0.4.1-Release`); main (`755de99`, the same on GitHub and the Gitea mirror) is three untagged commits past the pin (2026-11-04/05), all in the SDK's own copies of the Enyo 1 and Mojo frameworks (`0.4.1/share/framework`, Lunacy's patches 0013-0015: the spinner as a ring at high density, scrolling at 120 Hz, a WebView in a frame hearing its own events and its views going with the frame). Weighed for Jihad: not applicable — the Enyo card runs on the device's own framework, the Mochi card on `third_party/mochi-sampler`, and nothing here builds from the SDK's copies; the TouchPad's screen is 60 Hz. R1 stays on the release.
- 2026-11-10: R1 and R3 met (the owner's gitlink commit `5e4ace62`; T-550, the owner's two root steps: the installer and the novacomd swap). R2's stock 3.0.5 check waits for the stock TouchPad.
- 2026-11-09: R1 met but for the gitlink (the owner's), R2 met on the bench (T-548, T-549); R3's root command handed over (T-550).
- 2026-10-26: created from the owner's request and the day's research.
