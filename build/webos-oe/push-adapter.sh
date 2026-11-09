#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# push-adapter.sh [enyo mochi mojo]: the BrowserAdapterImpl.so build-adapter-pdk.sh just built, laid down where the
# variant's shim loads it (/usr/lib/jihad/<V>/BrowserAdapterImpl.so: root-owned, 0644, on the rootfs — the shim refuses
# anything else) and in the app's own directory (what the package carries, so a later postinst finds the two
# byte-identical and leaves them alone), the md5 read back on both; then LunaSysMgr restarted once for all of them
# (the impl is dlopen'd by the shim per card, but a loaded library stays mapped) through sysmgr-restart.sh, which
# closes the USB "Connected" alert the restart raises. The .ipk is not touched: build-variant-ipk.sh stages the same
# file from the app directory.
set -euo pipefail
. "$(dirname "$0")/host-path.sh"
HERE=$(cd "$(dirname "$0")" && pwd); REPO=$(cd "$HERE/../.." && pwd)
VS=("$@"); [ ${#VS[@]} -gt 0 ] || VS=(enyo mochi mojo)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
for V in "${VS[@]}"; do
  case $V in enyo) ID=net.riverstonerelay.jihad-browser;; mochi) ID=net.riverstonerelay.jihad-browser-mochi;; mojo) ID=net.riverstonerelay.jihad-browser-mojo;; *) echo "push-adapter.sh: unknown variant $V" >&2; exit 2;; esac
  SRC="$HERE/adapter-deps/build-pdk/$V/BrowserAdapterImpl.so"
  [ -s "$SRC" ] || { echo "push-adapter.sh: no $SRC (run build-adapter-pdk.sh $V)" >&2; exit 3; }
  H=$(md5sum "$SRC" | cut -c1-32)
  novacom put "file:///tmp/impl-$V.so" < "$SRC"
  cat > "$T/p.sh" <<EOS
mount -o rw,remount / >/dev/null 2>&1 || true
IMPL=/usr/lib/jihad/$V/BrowserAdapterImpl.so; APP=/media/cryptofs/apps/usr/palm/applications/$ID/BrowserAdapterImpl.so
cp /tmp/impl-$V.so \$IMPL.tmp && chown 0:0 \$IMPL.tmp && chmod 0644 \$IMPL.tmp && mv -f \$IMPL.tmp \$IMPL
cp /tmp/impl-$V.so \$APP.tmp && mv -f \$APP.tmp \$APP
rm -f /tmp/impl-$V.so
echo "\$(md5sum \$IMPL | cut -c1-32) \$(md5sum \$APP | cut -c1-32) \$(ls -l \$IMPL | awk '{print \$1, \$3}')"
EOS
  R=$(bash "$HERE/dev-sh.sh" -t 60 "$T/p.sh" 2>&1 | tail -1)
  set -- $R
  if [ "${1:-}" = "$H" ] && [ "${2:-}" = "$H" ]; then echo "[$V] impl $H on the rootfs and in the app dir (${3:-} ${4:-})"; else echo "[$V] PUSH FAILED: wanted $H, the device read: $R" >&2; exit 4; fi
done
echo "LunaSysMgr restarted so every card loads the new impl (the USB alert closed by the script):"
bash "$HERE/dev-sh.sh" -t 120 "$REPO/tools/device/finger/sysmgr-restart.sh" 2>&1 | tail -2
novacom run file://bin/rm -- -f /tmp/fl-restart.png < /dev/null || true
