#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
# Build + run the mismatched-targetApplication XPI install test
# (cavekit-addons-extensions.md R3, the "clear reason" criterion / T-103).
#
# Builds BOTH add-ons from the same recipe so the only difference between them is the one
# field under test — install.rdf's <em:targetApplication><em:id>. Anything else differing
# would leave "refused for incompatibility" indistinguishable from "refused for some other
# reason", which is the whole question.
#
# JIHAD_XPI_GOOD=1 runs the negative control: the MATCHING add-on, declined at the confirm.
# It must NOT produce the incompatibility alert.
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
SRC=/jihad/render/goanna
OUT=/out/xpi_mismatch_test
[ -e "$DIST/bin/libxul.so" ] || { echo "ERROR: build the engine first"; exit 1; }

# The observer under test lives in this component. Without it in the dist the add-on is still
# refused and the user is simply never told why — i.e. the test would silently measure the
# state this task exists to change.
[ -s "$DIST/bin/components/jihadInstallPrompt.js" ] || {
  echo "ERROR: components/jihadInstallPrompt.js missing from the dist — re-run build-goanna.sh"; exit 2; }

# --- the two add-ons ------------------------------------------------------------------------
mk_addon() {   # $1 = out xpi   $2 = addon id   $3 = name   $4 = targetApplication id
  local d; d=$(mktemp -d)
  cat > "$d/install.rdf" <<RDF
<?xml version="1.0"?>
<RDF xmlns="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
     xmlns:em="http://www.mozilla.org/2004/em-rdf#">
  <Description about="urn:mozilla:install-manifest">
    <em:id>$2</em:id>
    <em:type>2</em:type>
    <em:name>$3</em:name>
    <em:version>1.0</em:version>
    <em:bootstrap>true</em:bootstrap>
    <em:targetApplication>
      <Description>
        <em:id>$4</em:id>
        <em:minVersion>1.0</em:minVersion>
        <em:maxVersion>1.0</em:maxVersion>
      </Description>
    </em:targetApplication>
  </Description>
</RDF>
RDF
  cat > "$d/bootstrap.js" <<'JS'
function startup(data, reason) { Components.utils.reportError("[JIHAD-T103] startup reason=" + reason); }
function shutdown(data, reason) {}
function install(data, reason) {}
function uninstall(data, reason) {}
JS
  rm -f "$1"
  (cd "$d" && zip -q -r "$1" install.rdf bootstrap.js) || return 1
  rm -rf "$d"
}

# JIHAD_APP_ID (render/goanna/JihadUserAgent.h) is the id an add-on must name.
mk_addon /out/jihad-t103-good.xpi     "jihad-t103-good@riverstonerelay.net"     "Jihad T103 Good" \
         "{4534aac8-d8c8-4765-95ee-7f61fd0b762d}" || exit 3
# Deliberately NOT this app, NOT the AppCompat GUID (extensions.guid.appCompatId) and NOT the
# toolkit id — so matchingTargetApplication is null and appDisabled is set.
mk_addon /out/jihad-t103-mismatch.xpi "jihad-t103-mismatch@riverstonerelay.net" "Jihad T103 Mismatch" \
         "{ffffffff-0000-0000-0000-000000000000}" || exit 3
echo "built: $(ls -l /out/jihad-t103-good.xpi /out/jihad-t103-mismatch.xpi | wc -l) test add-ons"

CXX=${CXX:-g++}
GTK_CFLAGS=$(pkg-config --cflags gtk+-2.0 glib-2.0); GTK_LIBS=$(pkg-config --libs gtk+-2.0 glib-2.0)
CXXFLAGS="-std=gnu++17 -fPIC -fno-rtti -fno-exceptions -O1 -g"
INCS="-include $DIST/include/mozilla-config.h -I$DIST/include -I$DIST/include/nspr $GTK_CFLAGS"

set -x
$CXX $CXXFLAGS $INCS -c "$SRC/EngineHost.cpp"             -o /out/EngineHost.o         || exit 10
$CXX $CXXFLAGS $INCS -c "$SRC/GoannaRenderPage.cpp"       -o /out/GoannaRenderPage.o   || exit 11
$CXX $CXXFLAGS $INCS -c "$SRC/JihadCertStore.cpp"         -o /out/JihadCertStore.o     || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/DialogService.cpp"          -o /out/DialogService.o      || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/DownloadService.cpp"        -o /out/DownloadService.o    || exit 12
$CXX $CXXFLAGS $INCS -c "$SRC/test/xpi_mismatch_test.cpp" -o /out/xpi_mismatch_test.o  || exit 13
$CXX /out/xpi_mismatch_test.o /out/GoannaRenderPage.o /out/JihadCertStore.o /out/DialogService.o \
  /out/DownloadService.o /out/EngineHost.o \
  "$DIST/sdk/lib/libxpcomglue_s.a" -L"$DIST/bin" -lxul "$DIST/sdk/lib/libmozglue.a" \
  -lnspr4 -lplc4 -lplds4 $GTK_LIBS -Wl,-rpath,"$DIST/bin" -ldl -lpthread -o "$OUT" || exit 14
set +x

grep -q 'jihad-embed' "$DIST/bin/goanna.js" 2>/dev/null || \
  echo 'pref("layers.offmainthreadcomposition.force-disabled", true); // jihad-embed' >> "$DIST/bin/goanna.js"
export JIHAD_DISABLE_OMTC=1

# A FRESH profile: an add-on left behind by an earlier run changes what the add-on manager
# does with the next one, and the shared desktop profile already holds two installs.
STATE=${JIHAD_STATE_DIR:-/out/t103-state}
rm -rf "$STATE"
mkdir -p "$STATE"
export JIHAD_STATE_DIR="$STATE"
export JIHAD_XPI_DIR=/out

LD_LIBRARY_PATH="$DIST/bin" xvfb-run -a -s "-screen 0 1024x768x24" "$OUT" "$DIST/bin"
rc=$?
echo "== xpi_mismatch_test exit: $rc =="
echo "-- profile extensions after the run --"
ls -1 "$STATE/profile/extensions" 2>/dev/null || echo "(none)"
exit $rc
