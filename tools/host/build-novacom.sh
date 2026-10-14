#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Build novacom (the client) and novacomd (the USB daemon that talks to a TouchPad) from webOS
# Archive's SDK, the submodule third_party/webos-sdk-redux, into tools/host/bin/ (git-ignored).
# build/webos-oe/host-path.sh puts that directory first on PATH for every script here, so
# nothing has to be installed system-wide (cavekit-device-build.md R11).
#
#   tools/host/build-novacom.sh
#
# Needs a C compiler, make and libusb's 0.1 API (libusb-compat) with its headers on the host,
# because novacomd talks to USB directly:
#   apt (Debian, Ubuntu)      build-essential libusb-dev
#   dnf (Fedora, RHEL)        gcc make libusb-compat-0.1-devel   (older: libusb-devel)
#   pacman (Arch)             base-devel libusb-compat
#   zypper (openSUSE)         gcc make libusb-compat-devel
#   xbps (Void)               base-devel libusb-compat-devel
#   apk (Alpine)              build-base libusb-compat-dev
#
# Then run the daemon as root while a TouchPad is plugged in, and talk to it as yourself:
#   sudo tools/host/bin/novacomd &      (or your service manager)
#   tools/host/bin/novacom -l
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
SDK="$REPO/third_party/webos-sdk-redux"
[ -f "$SDK/novacomd/makefile" ] || { echo "!! third_party/webos-sdk-redux is not checked out: git submodule update --init third_party/webos-sdk-redux" >&2; exit 2; }
command -v cc >/dev/null || command -v gcc >/dev/null || { echo "!! no C compiler (see the package list at the top of this script)" >&2; exit 2; }
command -v make >/dev/null || { echo "!! no make (see the package list at the top of this script)" >&2; exit 2; }
if ! printf '#include <usb.h>\nint main(void){usb_init();return 0;}\n' | ${CC:-cc} -x c - -lusb -o /dev/null 2>/dev/null; then
	echo "!! libusb-compat (usb.h and -lusb) not found; see the package list at the top of this script" >&2
	exit 2
fi

# Build in a copy, so the submodule's checkout stays clean.
WORK="$HERE/build"
rm -rf "${WORK:?}"; mkdir -p "$WORK" "$HERE/bin"
cp -a "$SDK/novacomd" "$SDK/novacom" "$WORK/"
make -C "$WORK/novacomd" host
make -C "$WORK/novacom"
install -m 755 "$WORK/novacomd/build-novacomd/novacomd" "$HERE/bin/novacomd"
install -m 755 "$WORK/novacom/build-novacom/novacom" "$HERE/bin/novacom"
echo "built: $HERE/bin/novacomd, $HERE/bin/novacom"
