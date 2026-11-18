#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# site-data-clean.sh up|down SHELL (host side): the shell's saved logins, form history and site permissions set aside for
# a cavekit-site-data.md script, and put back after it. Each script then starts with nothing saved (a login left by an
# earlier run meant no question was asked, and every later step failed: round 2026-11-18) and the profile is left as it
# was found. up runs before the daemon starts and down after it stops, so no file is open. An aside left by a run that
# never came down is kept as it is (it holds the profile's own files) and what is in the profile now is the dead run's.
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../../.." && pwd)"
case "$2" in enyo) A=net.riverstonerelay.jihad-browser;; mochi) A=net.riverstonerelay.jihad-browser-mochi;; mojo) A=net.riverstonerelay.jihad-browser-mojo;; *) echo "shell?"; exit 2;; esac
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
cat > "$T/clean.sh" <<EOF
P=/media/cryptofs/apps/usr/palm/applications/$A/profile; X=\$P/fl-site-data-aside
F="logins.json formhistory.sqlite formhistory.sqlite-wal formhistory.sqlite-shm permissions.sqlite permissions.sqlite-wal permissions.sqlite-shm"
cd "\$P" || exit 1
if [ "$1" = up ]; then
  if [ -d "\$X" ]; then rm -f \$F; echo "an aside from an earlier run kept; the profile's copies were that run's"
  else mkdir "\$X"; for f in \$F; do [ -e "\$f" ] && mv "\$f" "\$X/"; done; fi
else
  rm -f \$F
  if [ -d "\$X" ]; then for f in \$F; do [ -e "\$X/\$f" ] && mv "\$X/\$f" .; done; rmdir "\$X"; fi
fi
ls \$F 2>/dev/null | tr '\n' ' '
EOF
echo "site data: $1 ($2): $("$ROOT/build/webos-oe/dev-sh.sh" -t 60 "$T/clean.sh" 2>&1 | tail -1)"
