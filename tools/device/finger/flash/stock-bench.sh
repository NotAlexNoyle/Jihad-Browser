#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# stock-bench.sh up|down -- the stock browser under a test app id (cavekit-flash.md R3, T-573), HOST side. On webOS CE
# com.palm.app.browser's launcher sends every launch to Atlas, which has no Flash, and has no way past it; the system's files are not
# ours to change. So the app is pulled, its original launcher page put back in the copy, packaged under its own id and installed for a
# round; `down` removes it. It runs on the system's own BrowserServer and BrowserAdapter.so and the same player, which is what the bar
# is. The app manager takes no hand-laid directory: palm-install it is. Nothing of HP's is kept: the copy is made in a temporary
# directory and deleted at once.
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../.." && pwd)"; . "$ROOT/build/webos-oe/host-path.sh"
ID=net.riverstonerelay.stockbench
if [ "$1" = up ]; then
  T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
  printf 'cd /usr/palm/applications && tar cf /tmp/fl-stock.tar com.palm.app.browser\n' > "$T/t.sh"; bash "$ROOT/build/webos-oe/dev-sh.sh" -t 40 "$T/t.sh" >/dev/null 2>&1
  novacom get file:///tmp/fl-stock.tar > "$T/a.tar"; novacom run file://bin/rm -- -f /tmp/fl-stock.tar < /dev/null
  tar xf "$T/a.tar" -C "$T" || exit 1
  A="$T/com.palm.app.browser"; cp "$A/index.html.openinatlas-orig" "$A/index.html"; rm -f "$A"/*-orig
  python3 -I - "$A/appinfo.json" "$ID" <<'PY'
import json, re, sys
p, i = sys.argv[1], sys.argv[2]; s = open(p, encoding="utf-8-sig").read()
s = re.sub(r'"id"\s*:\s*"[^"]*"', '"id": "%s"' % i, s, count=1); s = re.sub(r'"title"\s*:\s*"[^"]*"', '"title": "Stock bench"', s, count=1); s = re.sub(r'"visible"\s*:\s*false', '"visible": true', s)
open(p, "w").write(s)
PY
  palm-package -o "$T" "$A" 2>&1 | tail -1 | cut -c1-120
  palm-install "$T"/${ID}_*.ipk 2>&1 | tail -1 | cut -c1-120
else
  palm-install -r "$ID" 2>&1 | tail -1 | cut -c1-120
fi
