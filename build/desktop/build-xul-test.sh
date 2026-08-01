#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
# Build + run the XUL/chrome input test (cavekit-input-bridging R6, T-067/T-068).
# Reproduces the device-only XUL SendMouseEvent SIGSEGV on the desktop engine so it
# can be diagnosed in a local loop instead of 5-minute device round-trips.
#
# JIHAD_HEADLESS=1 builds against the TRUE HEADLESS libxul (obj-jihad-headless,
# MOZ_WIDGET_TOOLKIT=headless) and runs with NO X server at all. That is what actually
# reproduces the device: the headless widget module registers only five contracts
# (appshell, screenmanager, transferable, formatconverter, gfxinfo) where the GTK2 module
# registers the full set, so the GTK desktop build silently has widget components the
# device does not. A XUL repro on the GTK build is therefore NOT a repro of the device.
set -uo pipefail
DIST=${JIHAD_HEADLESS:+/out/obj-jihad-headless/dist}
DIST=${DIST:-/out/obj-jihad-goanna/dist}
SRC=/jihad/render/goanna
OUT=/out/xul_test
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the engine first"; exit 1; }

CXX=${CXX:-g++}
GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0 glib-2.0); GTK_LIBS=$(pkg-config --libs gtk+-2.0 glib-2.0)
GLIB_CFLAGS=$(pkg-config --cflags glib-2.0);         GLIB_LIBS=$(pkg-config --libs glib-2.0)
# -g: the whole point is a resolvable backtrace out of the daemon-side code too.
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -O1 -g"
if [ -n "${JIHAD_HEADLESS:-}" ]; then
  # Exactly the device daemon's compile flags (build/webos-oe/build-daemon-arm.sh): GTK-free,
  # offscreen-only. Linking GTK would need a DISPLAY and put an X server back in the picture.
  CXXFLAGS="$CXXFLAGS -DJIHAD_OFFSCREEN_ONLY"
  GTK_CFLAGS=$GLIB_CFLAGS; GTK_LIBS=$GLIB_LIBS
fi
INCS="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr $GTK_CFLAGS"

set -x
$CXX $CXXFLAGS $INCS -c "$SRC/EngineHost.cpp"       -o /out/EngineHost.o       || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/DialogService.cpp"    -o /out/DialogService.o    || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/DownloadService.cpp"  -o /out/DownloadService.o  || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/GoannaRenderPage.cpp" -o /out/GoannaRenderPage.o || exit 11
$CXX $CXXFLAGS $INCS -c "$SRC/test/xul_test.cpp"    -o /out/xul_test.o         || exit 12
$CXX /out/xul_test.o /out/GoannaRenderPage.o /out/EngineHost.o /out/DialogService.o /out/DownloadService.o \
  "$DIST/sdk/lib/libxpcomglue_s.a" -L"$DIST/bin" -lxul "$DIST/sdk/lib/libmozglue.a" \
  -lnspr4 -lplc4 -lplds4 $GTK_LIBS -Wl,-rpath,"$DIST/bin" -ldl -lpthread -o "$OUT" || exit 13
set +x

if ! grep -q 'jihad-embed' "$DIST/bin/goanna.js" 2>/dev/null; then
  echo 'pref("layers.offmainthreadcomposition.force-disabled", true); // jihad-embed' >> "$DIST/bin/goanna.js"
fi
export JIHAD_DISABLE_OMTC=1
# The device runs the offscreen PuppetWidget path exclusively (JIHAD_OFFSCREEN_ONLY);
# the desktop default is the GTK window, which does NOT reproduce a PuppetWidget fault.
# Forcing it here is what makes this a real repro of the device configuration.
# JIHAD_XUL_ONSCREEN=1 runs the legacy GTK-window widget path instead of the offscreen
# PuppetWidget. It is the CONTROL for every input result here: the device only ever runs
# offscreen, so anything that works on-screen and not offscreen is a PuppetWidget defect,
# not an engine one — and that distinction is not visible from a single run.
if [ -z "${JIHAD_XUL_ONSCREEN:-}" ]; then export JIHAD_OFFSCREEN=1; else unset JIHAD_OFFSCREEN; fi
if [ -n "${JIHAD_HEADLESS:-}" ]; then
  # No xvfb, no DISPLAY — the device has no X server, and running under one would
  # re-introduce the very difference this build exists to eliminate.
  unset DISPLAY
  LD_LIBRARY_PATH="$DIST/bin" "$OUT" "$DIST/bin"
else
  LD_LIBRARY_PATH="$DIST/bin" xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin"
fi
# F-5: `echo "... $?"` USED TO BE THE LAST COMMAND, so the script's own exit status was echo's
# (always 0) and even a SIGSEGV (139) was reported to the caller as success. Capture the status
# FIRST, then exit with it — a runner that cannot fail makes every "harness passed" line
# worthless, and this was the ninth such fail-open in this repo's verification code.
rc=$?
echo "== xul_test exit: $rc =="
exit $rc
