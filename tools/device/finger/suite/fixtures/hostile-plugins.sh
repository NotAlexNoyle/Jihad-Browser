#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# hostile-plugins.sh up|down SHELL (host side): four libraries in the shell's own plugin folder (<profile>/plugins, first in
# the daemon's search path) for cavekit-flash.md R10's hostile library (T-580): random bytes, a constructor that never returns,
# a constructor that aborts, a library that is no plugin. Built here from hostile-plugins.c (the repository holds no binary);
# up before the daemon starts, down after it stops. The plugin registry is dropped both ways, so the daemon scans the folder.
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../../.." && pwd)"
. "$ROOT/build/webos-oe/host-path.sh" 2>/dev/null
case "$2" in enyo) A=net.riverstonerelay.jihad-browser;; mochi) A=net.riverstonerelay.jihad-browser-mochi;; mojo) A=net.riverstonerelay.jihad-browser-mojo;; *) echo "shell?"; exit 2;; esac
P=/media/cryptofs/apps/usr/palm/applications/$A/profile
CC="$ROOT/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi-gcc"
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
if [ "$1" = up ]; then
  "$CC" -shared -fPIC -O2 -DHANG -o "$T/libjihadhang.so" "$HERE/hostile-plugins.c" || exit 1
  "$CC" -shared -fPIC -O2 -DABORT -o "$T/libjihadabort.so" "$HERE/hostile-plugins.c" || exit 1
  "$CC" -shared -fPIC -O2 -DNOTNP -o "$T/libjihadnotnp.so" "$HERE/hostile-plugins.c" || exit 1
  head -c 8192 /dev/urandom > "$T/libjihadjunk.so"
  novacom run file://bin/mkdir -- -p $P/plugins < /dev/null
  for f in libjihadhang.so libjihadabort.so libjihadnotnp.so libjihadjunk.so; do novacom put file://$P/plugins/$f < "$T/$f"; done
  novacom run file://bin/rm -- -f $P/pluginreg.dat < /dev/null
else
  novacom run file://bin/rm -- -f $P/plugins/libjihadhang.so $P/plugins/libjihadabort.so $P/plugins/libjihadnotnp.so $P/plugins/libjihadjunk.so $P/pluginreg.dat < /dev/null
fi
echo "hostile plugins: $1 ($2)"
