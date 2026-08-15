#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
# Build + run the about:preferences harness test (cavekit-gre-widgets.md R5 T-139 +
# R7 T-111). Asserts are DOM/text-level only — desktop-harness PIXEL readback is a
# recorded dead end (context/impl/dead-ends.md), so nothing here looks at a frame.
#
# JIHAD_PREFSUI_NEG=139|111 runs a deliberate-failure variant; both MUST exit nonzero.
# JIHAD_PREFSUI_OFFSCREEN=1 runs the device's PuppetWidget path instead of the GTK window.
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
SRC=/jihad/render/goanna
OUT=/out/prefsui_test
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the engine first"; exit 1; }

# The prefs UI has to be IN THE DIST or this test measures nothing. build-goanna.sh
# installs it after `mach build`; fail loud here rather than reporting "page did not load".
for f in prefsui/content/preferences.html prefsui/content/preferences.js \
         components/jihadAboutPreferences.js jihad-prefsui.manifest; do
  [ -s "$DIST/bin/$f" ] || { echo "ERROR: $f missing from the dist — re-run build-goanna.sh"; exit 2; }
done

# And it has to be the CURRENT copy. cavekit-gre-widgets.md R5 records the device version of
# this trap: push-engine-update.sh ships libxul and the daemon but NOT packaging/prefsui/, so a
# test run after one of those measures the OLD page and reads as "the box does not render". The
# desktop dist has the same shape — build-goanna.sh copies the files in, and nothing re-copies
# them when you edit the source. Compare, do not assume.
for f in content/preferences.html content/preferences.js content/preferences.css; do
  cmp -s "/jihad/packaging/prefsui/$f" "$DIST/bin/prefsui/$f" || {
    echo "ERROR: $DIST/bin/prefsui/$f differs from packaging/prefsui/$f — the dist copy is STALE."
    echo "       Re-run build/desktop/build-goanna.sh (its post-mach phases re-install the prefs UI)."
    exit 2; }
done
cmp -s /jihad/render/goanna/components/jihadAboutPreferences.js \
       "$DIST/bin/components/jihadAboutPreferences.js" || {
  echo "ERROR: the dist's jihadAboutPreferences.js is STALE — re-run build-goanna.sh"; exit 2; }

CXX=${CXX:-g++}
GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0 glib-2.0); GTK_LIBS=$(pkg-config --libs gtk+-2.0 glib-2.0)
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -O1 -g"
INCS="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr $GTK_CFLAGS"

set -x
$CXX $CXXFLAGS $INCS -c "$SRC/EngineHost.cpp"        -o /out/EngineHost.o        || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/GoannaRenderPage.cpp"  -o /out/GoannaRenderPage.o  || exit 11
$CXX $CXXFLAGS $INCS -c "$SRC/JihadCertStore.cpp"    -o /out/JihadCertStore.o    || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/DialogService.cpp"     -o /out/DialogService.o     || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/DownloadService.cpp"   -o /out/DownloadService.o   || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/test/prefsui_test.cpp" -o /out/prefsui_test.o      || exit 13
$CXX /out/prefsui_test.o /out/GoannaRenderPage.o /out/JihadCertStore.o /out/DialogService.o \
  /out/DownloadService.o /out/EngineHost.o \
  "$DIST/sdk/lib/libxpcomglue_s.a" -L"$DIST/bin" -lxul "$DIST/sdk/lib/libmozglue.a" \
  -lnspr4 -lplc4 -lplds4 $GTK_LIBS -Wl,-rpath,"$DIST/bin" -ldl -lpthread -o "$OUT" || exit 14
set +x

# OMTC off (engine patch 0003).
grep -q 'jihad-embed' "$DIST/bin/goanna.js" 2>/dev/null || \
  echo 'pref("layers.offmainthreadcomposition.force-disabled", true); // jihad-embed' >> "$DIST/bin/goanna.js"
export JIHAD_DISABLE_OMTC=1

# A FRESH profile every run, on purpose. The point of T-111 is what the BUILD ships; a
# profile carrying a user_pref from an earlier session (the shared desktop profile has
# browser.cache.disk.capacity=358400 left over from before smart_size was pinned off)
# would answer a different question and read as a failure of the pref split.
STATE=${JIHAD_STATE_DIR:-/out/prefsui-state}
rm -rf "$STATE"
mkdir -p "$STATE"
export JIHAD_STATE_DIR="$STATE"

[ -n "${JIHAD_PREFSUI_OFFSCREEN:-}" ] && export JIHAD_OFFSCREEN=1
LD_LIBRARY_PATH="$DIST/bin" xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin"
rc=$?
echo "== prefsui_test exit: $rc =="
exit $rc
