#!/bin/bash
# Copyright 2026 the Jihad Browser project.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# HEADLESS round-trip: run the already-built jihad-browserserver + adapter with
# NO X SERVER at all (DISPLAY unset, no xvfb-run). Proves the JIHAD_OFFSCREEN path
# (PuppetWidget + patch 0006 headless gfxPlatformGtk) renders without X — the
# on-device configuration (the TouchPad has no X server). Run build-adapter-roundtrip.sh
# first to compile the daemon + adapter; this only re-runs them headless.
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
[ -x /out/jihad-browserserver ] || { echo "ERROR: run build-adapter-roundtrip.sh first"; exit 1; }

echo "== headless round-trip (NO X server, DISPLAY unset) =="
unset DISPLAY
export JIHAD_DISABLE_OMTC=1
export JIHAD_OFFSCREEN=1
export LD_LIBRARY_PATH="$DIST/bin"
/out/jihad-browserserver "$DIST/bin" &
dpid=$!
sleep 7
timeout 30 /out/jihad-adapter
rc=$?
kill "$dpid" 2>/dev/null; wait 2>/dev/null
echo "== headless round-trip exit: $rc =="
exit $rc
