#!/bin/bash
# Copyright 2026 NotAlexNoyle.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# XUL <menupopup> overlay probe (cavekit-offscreen-rendering.md R7).
#
# WHY THIS EXISTS SEPARATELY FROM build-popup-probe.sh. That probe drives a plain
# HTML <select>, which in Gecko is the COMBOBOX path (nsComboboxControlFrame +
# a dropdown view) and never enters nsXULPopupManager at all — which is why its
# `[jihad-popup] ShowPopupCallback` line never fired and the note that the popup
# "is created but never shown" conflated two different mechanisms. A real
# <menupopup> (the about:addons tools menu, context menus) DOES go through the
# popup manager, and only that path can be used to test the overlay composite.
#
# The probe loads a small XUL document with a type="menu" button, opens its popup
# with privileged JS through the daemon's $JIHAD_INJECT `jsurl` command, and dumps
# the painted frame so the popup can be seen (or not) in the pixels.
#
# Run AFTER build-adapter-roundtrip.sh (needs /out/jihad-browserserver + /out/jihad-adapter).
set -uo pipefail
DIST=/out/obj-jihad-goanna/dist
[ -x /out/jihad-browserserver ] || { echo "ERROR: run build-adapter-roundtrip.sh first"; exit 1; }

# Self-wrap under Xvfb (see build-popup-probe.sh: never `exec`, or xvfb-run becomes
# PID 1 and deadlocks on child reaping).
if [ -z "${DISPLAY:-}" ]; then
  xvfb-run -a -s "-screen 0 1024x768x24" bash "$0" "$@"
  exit $?
fi

export JIHAD_DISABLE_OMTC=1
export JIHAD_OFFSCREEN=1
export JIHAD_INJECT=1
export JIHAD_DUMP=1
export LD_LIBRARY_PATH="$DIST/bin"

# THE DOCUMENT UNDER TEST IS about:addons, not a XUL file of our own: XUL parsing is
# chrome-only in this engine, so a file:// .xul document is refused outright
# (measured: load fails, the daemon paints about:blank). about:addons is real chrome
# XUL that already renders here, and its tools menu is exactly the <menupopup> this
# requirement exists for.

DLOG=/out/menupopup-daemon.log
rm -f "$DLOG" /out/.jihad/default/frame.ppm
/out/jihad-browserserver "$DIST/bin" >"$DLOG" 2>&1 &
dpid=$!
sleep 7

export JIHAD_URL="about:addons"
export JIHAD_STAY=1
timeout 60 /out/jihad-adapter >/out/menupopup-adapter.log 2>&1 &
apid=$!
sleep 12   # connect + load + first paint

INJ=/out/.jihad/default/inject.cmd
# Open the popup through the popup manager, from privileged JS. openPopup() with
# explicit coordinates avoids depending on the button's own activation path (which
# is the XUL-input question, a different open item) — this probe is about whether an
# OPEN popup reaches the pixels.
cat > "$INJ" <<'CMD'
jsurl javascript:void(function(){try{var d=document,p=d.getElementById('utils-menu'),b=d.getElementById('header-utils-btn');var msg='popup='+(!!p)+' btn='+(!!b)+' root='+d.documentElement.tagName;if(p&&b){p.openPopup(b,'after_start',0,0,false,false,null);msg+=' state='+p.state;}d.documentElement.setAttribute('title','JPROBE '+msg);}catch(e){d.documentElement.setAttribute('title','JPROBE EXC '+e);}}())
CMD
sleep 6
echo "title" > "$INJ"    # forces a tick + a log line so timing is visible
sleep 6

kill "$apid" 2>/dev/null
kill "$dpid" 2>/dev/null
wait 2>/dev/null

echo "== popup manager / widget instrumentation =="
grep -aE "jihad-popup|jihad-widget" "$DLOG" || echo "(none — the popup never opened)"
echo "== composite =="
grep -a "composited" "$DLOG" || echo "(no popup composited into a frame)"
echo "== paints =="
grep -a "painted shmid" "$DLOG" | tail -3
echo "== frame dump =="
ls -la /out/.jihad/default/frame.ppm 2>/dev/null || echo "(no frame.ppm)"
