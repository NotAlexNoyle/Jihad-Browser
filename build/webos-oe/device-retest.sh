#!/bin/bash
# Copyright 2026 the Jihad Browser project. Apache-2.0.
#
# T1–T5 retest bring-up (context/impl/device-test-2026-07-17.md): run when the
# TouchPad reappears on novacom. Verifies the deployed trio still matches the
# local build (they were md5-verified live before the 2026-07-18 host power cut:
# libxul f7969264*, daemon 33a1aaa0, goanna.js b7db3f58), restarts the daemon if
# asked, then streams the daemon log filtered to the T1–T5 signals.
# (*libxul md5 is of the DEPLOYED stripped copy — local dist libxul is unstripped,
#  so only daemon + goanna.js md5s are compared host-vs-device here.)
#
# Usage: device-retest.sh [--restart] [--logs-only]
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)

RESTART=0 LOGS_ONLY=0
for a in "$@"; do case "$a" in
	--restart) RESTART=1;; --logs-only) LOGS_ONLY=1;;
	*) echo "unknown flag: $a" >&2; exit 2;;
esac; done

if [ -z "$(novacom -l 2>/dev/null)" ]; then
	echo "NO DEVICE: novacom -l lists nothing (check USB/power)."; exit 1
fi
echo "=== device ==="; novacom -l

if [ "$LOGS_ONLY" = 0 ]; then
	echo "=== deployed vs local md5 ==="
	novacom run file://usr/bin/md5sum -- \
		/media/internal/jihad/hl/jihad-browserserver \
		/media/internal/jihad/hl/goanna.js \
		/media/internal/jihad/hl/libxul.so 2>/dev/null || echo "  (md5 probe failed)"
	echo "  local daemon   : $(md5sum "$HERE/out-arm/jihad-browserserver-arm" | cut -c1-32)"
	echo "  local goanna.js: $(md5sum "$HERE/device-bundle/goanna.js" | cut -c1-32)"
	echo "  expected deployed: daemon 33a1aaa0…, goanna.js b7db3f58…, libxul f7969264…"

	echo "=== daemon process ==="
	novacom run file://bin/ps -- aux 2>/dev/null | grep -E "jihad-browserserver" | grep -v grep || echo "  daemon NOT running"
	if [ "$RESTART" = 1 ]; then
		echo "=== restarting jihad daemon ==="
		novacom run file://sbin/stop -- jihad 2>/dev/null || true
		sleep 2
		novacom run file://sbin/start -- jihad
	fi
fi

cat <<'CHECKLIST'
=== T1–T5 retest checklist (drive on the touchscreen; watch the log below) ===
 1. T3 stale-frame: load a page; content must appear WITHOUT tap/drag nudges
    (patch 0012 dirty-repaint). Watch for periodic paints during load.
 2. T2 Enter-search: google.com → tap box → type → Enter. Expect results page,
    NO full-card overlay, NO daemon respawn in log (previous root: SIGSEGV).
 3. T5 link/button taps: tap results + a JS-onclick button. Expect nav, no overlay.
 4. T4 VKB: google → ddg → html.duckduckgo.com in ONE card; field tap must
    raise VKB each time (watch vkb tag= / editorFocused= lines).
 5. T1 scroll-push: focus google search box; note JIHAD_T1_LOG scroll lines
    (engine scroll before/after resize + editable-focused).
=== streaming daemon log (Ctrl-C to stop) ===
CHECKLIST
exec novacom run file://bin/sh -- -c "tail -f /media/internal/jihad/upstart-daemon.log" 2>/dev/null \
	| grep --line-buffered -E "state top|clickAt|vkb tag=|editorFocused|JIHAD_T1|scroll|respawn|SIGSEGV|paint|dirty" || true
