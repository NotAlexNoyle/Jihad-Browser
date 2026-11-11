#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Build novacom (the client) and novacomd (the USB daemon that talks to a TouchPad) into tools/host/bin/ (git-ignored).
# novacom comes from webOS Archive's SDK (release 0.4.1), the submodule third_party/webos-sdk-redux. novacomd comes
# from the owner's fork, NotAlexNoyle/novacomd at release 1.2.1 (the owner, 2026-11-11), fetched from Gitea at that tag
# and checked against the tag's commit. build/webos-oe/host-path.sh puts tools/host/bin first on PATH for every script
# here, so nothing has to be installed system-wide (cavekit-device-build.md R11).
#
#   tools/host/build-novacom.sh
#
# Needs a C compiler, make and libusb's 0.1 API (libusb-compat) with its headers on the host,
# because novacomd talks to USB directly:
#   apt (Debian, Ubuntu)      build-essential, and libusb-compat built from source (Debian packages only the original
#                             libusb 0.1.12, which this script refuses)
#   dnf (Fedora, RHEL)        gcc make libusb-compat-0.1-devel   (older: libusb-devel)
#   pacman (Arch)             base-devel libusb-compat
#   zypper (openSUSE)         gcc make libusb-compat-devel
#   xbps (Void)               base-devel libusb-compat-devel
#   apk (Alpine)              build-base libusb-compat-dev
#
# Then run the daemon as root while a TouchPad is plugged in, and talk to it as yourself:
#   sudo -b tools/host/bin/novacomd     (in the background; or your service manager)
#   tools/host/bin/novacom -l
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
SDK="$REPO/third_party/webos-sdk-redux"
[ -f "$SDK/novacom/makefile" ] || { echo "!! third_party/webos-sdk-redux is not checked out: git submodule update --init third_party/webos-sdk-redux" >&2; exit 2; }
CC="${CC:-$(command -v cc || command -v gcc || command -v clang || true)}"
[ -n "$CC" ] || { echo "!! no C compiler (see the package list at the top of this script)" >&2; exit 2; }
export CC   # the probe below and both builds use the compiler that was found
command -v make >/dev/null || { echo "!! no make (see the package list at the top of this script)" >&2; exit 2; }
if ! printf '#include <usb.h>\nint main(void){usb_init();return 0;}\n' | "$CC" -x c - -lusb -o /dev/null 2>/dev/null; then
	echo "!! libusb-compat (usb.h and -lusb) not found; see the package list at the top of this script" >&2
	exit 2
fi
# Not the original libusb 0.1.12 (the libusb-dev of Debian, Ubuntu and Raspberry Pi OS): after a USB timeout its wait for
# the cancelled transfer can block for good, and novacomd stops answering (seen 2026-11-11 with the 1.2.1 release
# binary, which has it built in). libusb-compat calls into libusb 1.0, linked or loaded at run time; the original does
# not, so the library's text tells them apart (both report version 0.1.12 to pkg-config). Drop this check once the
# novacomd pin is 1.2.2 or later, whose install script refuses the original itself and builds libusb-compat instead.
usblib=$("$CC" -print-file-name=libusb.so)
[ -f "$usblib" ] || usblib=$("$CC" -print-file-name=libusb.a)
if [ -f "$usblib" ] && ! grep -qa libusb_init "$usblib"; then
	echo "!! $usblib is the original libusb 0.1.12, which can hang novacomd; it needs libusb-compat. Debian, Ubuntu and" >&2
	echo "   Raspberry Pi OS do not package it: build libusb-compat 0.1.9 from its release tarball" >&2
	echo "   (https://github.com/libusb/libusb-compat-0.1/releases; needs libusb-1.0-0-dev and pkg-config) and install it." >&2
	exit 2
fi

# The SDK's novacom: its platform.h declares its own `typedef int bool`, which C23 (gcc 15's default) refuses; the
# SDK's GCC15-C23-BOOL.md names -std=gnu17 as the fix, and every gcc back to 4.x takes it.
# The fork's novacomd builds at the compiler's own default standard.
NOVACOMD_URL=https://git.trueog.net/NotAlexNoyle/novacomd.git
NOVACOMD_TAG=1.2.1
NOVACOMD_COMMIT=b8c48efe9e3dfa9f3b8e7f9194cc4d41a36f3d82
command -v git >/dev/null || { echo "!! no git (novacomd is fetched from $NOVACOMD_URL)" >&2; exit 2; }

# Build in a work directory, so the submodule's checkout stays clean.
WORK="$HERE/build"
rm -rf "${WORK:?}"; mkdir -p "$WORK" "$HERE/bin"
cp -a "$SDK/novacom" "$WORK/"
git -c advice.detachedHead=false clone -q --depth 1 --branch "$NOVACOMD_TAG" "$NOVACOMD_URL" "$WORK/novacomd"
got=$(git -C "$WORK/novacomd" rev-parse HEAD)
[ "$got" = "$NOVACOMD_COMMIT" ] || { echo "!! novacomd tag $NOVACOMD_TAG is at $got, not $NOVACOMD_COMMIT: stopping" >&2; exit 1; }
make -C "$WORK/novacom" CC="$CC -std=gnu17"
CC="$CC" BUILDVERSION="$NOVACOMD_TAG" bash "$WORK/novacomd/install-novacomd.sh" "$WORK/novacomd"   # unprivileged: builds only
# Each into place by rename, so a script running novacom meanwhile never finds it missing.
put() { install -m 755 "$1" "$HERE/bin/.$2.new" && mv -f "$HERE/bin/.$2.new" "$HERE/bin/$2"; }
put "$WORK/novacomd/build-novacomd-host/novacomd" novacomd
put "$WORK/novacom/build-novacom/novacom" novacom
echo "built: $HERE/bin/novacomd ($("$HERE/bin/novacomd" -V 2>&1 | sed -n 's/.*novacomd version //p' | head -1)), $HERE/bin/novacom"
