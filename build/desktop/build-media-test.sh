#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
# Build + run the desktop media and video-controls test (cavekit-engine-update.md R6, T-387):
# VP8, Theora and VP9 clips play to their end with the controls' binding attached, and taps on
# the control bar pause and resume. JIHAD_MEDIA_NEG=1 must exit nonzero. JIHAD_MEDIA_SOUND=1 also
# plays a Vorbis clip (needs a sound server: pulseaudio with a null sink is started when present).
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
SRC=/jihad/render/goanna
OUT=/out/media_test
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the engine first"; exit 1; }
# the clips are made, not committed (render/goanna/test/make-media-assets.sh): a missing one would
# read as a decoder regression
for c in clip10v.webm clip10.ogv clip10vp9.webm tone15.ogg; do
  [ -s "$SRC/test/$c" ] || { echo "ERROR: $SRC/test/$c missing: run render/goanna/test/make-media-assets.sh"; exit 2; }
done

CXX=${CXX:-g++}
GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0 glib-2.0); GTK_LIBS=$(pkg-config --libs gtk+-2.0 glib-2.0)
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -O1 -g"
INCS="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr -I$DIST/include/nss $GTK_CFLAGS"

set -x
$CXX $CXXFLAGS $INCS -c "$SRC/EngineHost.cpp"        -o /out/EngineHost.o        || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/GoannaRenderPage.cpp"  -o /out/GoannaRenderPage.o  || exit 11
$CXX $CXXFLAGS $INCS -c "$SRC/JihadCertStore.cpp"    -o /out/JihadCertStore.o    || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/DialogService.cpp"     -o /out/DialogService.o     || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/DownloadService.cpp"   -o /out/DownloadService.o   || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/test/media_test.cpp" -o /out/media_test.o      || exit 13
$CXX /out/media_test.o /out/GoannaRenderPage.o /out/JihadCertStore.o /out/DialogService.o \
  /out/DownloadService.o /out/EngineHost.o \
  "$DIST/sdk/lib/libxpcomglue_s.a" -L"$DIST/bin" -lxul "$DIST/sdk/lib/libmozglue.a" \
  -lnspr4 -lplc4 -lplds4 $GTK_LIBS -Wl,-rpath,"$DIST/bin" -ldl -lpthread -o "$OUT" || exit 14
set +x

# OMTC off (engine patch 0003).
grep -q 'jihad-embed' "$DIST/bin/goanna.js" 2>/dev/null || \
  echo 'pref("layers.offmainthreadcomposition.force-disabled", true); // jihad-embed' >> "$DIST/bin/goanna.js"
export JIHAD_DISABLE_OMTC=1
STATE=${JIHAD_STATE_DIR:-/out/media-state}
rm -rf "$STATE"; mkdir -p "$STATE"
export JIHAD_STATE_DIR="$STATE"
if [ -n "${JIHAD_MEDIA_SOUND:-}" ] && command -v pulseaudio >/dev/null 2>&1; then
  pulseaudio --start --exit-idle-time=-1 -n -F /dev/null --load=module-null-sink --load=module-native-protocol-unix 2>/dev/null || true
fi
LD_LIBRARY_PATH="$DIST/bin" xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin" "$SRC/test"
rc=$?
echo "== media_test exit: $rc =="
exit $rc
