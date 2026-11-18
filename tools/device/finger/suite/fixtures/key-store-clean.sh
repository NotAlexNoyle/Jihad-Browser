#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# key-store-clean.sh up|down SHELL (host side): the shell's NSS key store (key4.db: the key that encrypts saved logins,
# and the master password over it) set aside for a script that sets a master password (site-logins-master), and put
# back after it; the engine makes a new, empty store at its start. A run that dies with the master password set leaves
# it on the test store only, never on the profile's own. Used with site-data-clean (the logins encrypted by the set-aside
# key go aside with it). up runs before the daemon starts, down after it stops. An aside left by a run that never came
# down is kept as it is (it holds the profile's own store).
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../../.." && pwd)"
case "$2" in enyo) A=net.riverstonerelay.jihad-browser;; mochi) A=net.riverstonerelay.jihad-browser-mochi;; mojo) A=net.riverstonerelay.jihad-browser-mojo;; *) echo "shell?"; exit 2;; esac
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
cat > "$T/clean.sh" <<EOS
P=/media/cryptofs/apps/usr/palm/applications/$A/profile; X=\$P/fl-key-store-aside
F="key4.db key4.db-journal key4.db-wal key4.db-shm"
cd "\$P" || exit 1
if [ "$1" = up ]; then
  if [ -d "\$X" ]; then rm -f \$F; echo "an aside from an earlier run kept; the profile's store was that run's"
  else mkdir "\$X"; for f in \$F; do [ -e "\$f" ] && mv "\$f" "\$X/"; done; fi
else
  rm -f \$F
  if [ -d "\$X" ]; then for f in \$F; do [ -e "\$X/\$f" ] && mv "\$X/\$f" .; done; rmdir "\$X"; fi
fi
ls \$F 2>/dev/null | tr '\n' ' '
EOS
echo "key store: $1 ($2): $("$ROOT/build/webos-oe/dev-sh.sh" -t 60 "$T/clean.sh" 2>&1 | tail -1)"
