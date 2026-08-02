#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Popup instrumentation probe (menupopup investigation, 2026-08-02): load a page with a
# plain <select>, drive a click into it through the $JIHAD_INJECT channel, and report every
# [jihad-widget]/[jihad-popup] line the instrumented libxul emits. Answers the question the
# device could not: does a popup OPEN headless (widget created/shown), or does the
# activation die before the popup machinery runs?
#
# Run AFTER build-adapter-roundtrip.sh (needs /out/jihad-browserserver + /out/jihad-adapter).
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
[ -x /out/jihad-browserserver ] || { echo "ERROR: run build-adapter-roundtrip.sh first"; exit 1; }

unset DISPLAY
export JIHAD_DISABLE_OMTC=1
export JIHAD_OFFSCREEN=1
export JIHAD_INJECT=1
export LD_LIBRARY_PATH="$DIST/bin"

DLOG=/tmp/popup-daemon.log
rm -f "$DLOG"
/out/jihad-browserserver "$DIST/bin" >"$DLOG" 2>&1 &
dpid=$!
sleep 7

export JIHAD_URL="data:text/html,<body style='margin:0'><select style='position:absolute;left:100px;top:100px;width:220px;height:44px;font-size:24px'><option>alpha</option><option>beta</option><option>gamma</option></select></body>"
export JIHAD_STAY=1
timeout 40 /out/jihad-adapter >/tmp/popup-adapter.log 2>&1 &
apid=$!
sleep 12   # connect + load + first paint

INJ=/out/.jihad/default/inject.cmd
echo "click 200 120 1" > "$INJ"     # tap the select (content coords)
sleep 5
echo "click 200 190 1" > "$INJ"     # tap where the open dropdown's 2nd row would be
sleep 5

kill "$apid" 2>/dev/null
kill "$dpid" 2>/dev/null
wait 2>/dev/null

echo "== inject lines =="
grep -a "inject" "$DLOG" || true
echo "== instrumentation =="
grep -aE "jihad-widget|jihad-popup" "$DLOG" || echo "(no popup/widget lines at all)"
echo "== clickAt lines =="
grep -a "clickAt" "$DLOG" | tail -6 || true
