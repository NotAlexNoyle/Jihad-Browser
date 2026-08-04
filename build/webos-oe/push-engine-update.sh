#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Push an UPDATED engine (libxul.so) + daemon (jihad-browserserver) into the deviceroot of the
# named variants (default: all three) and restart each variant's own upstart job. For the
# fast dev loop after an ARM rebuild — the full payload push is push-variant.sh.
#
# Sources are the CURRENT out-arm artifacts, stripped the same way make-device-bundle.sh strips
# them, staged to a temp dir (never stripping the build outputs in place). Every push is
# md5-verified on both sides (the 2026-08-02 zero-byte-daemon lesson).
#
# Usage: build/webos-oe/push-engine-update.sh [enyo] [mochi] [mojo]
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
DIST="$HERE/out-arm/obj-jihad-goanna-arm/dist"
DAEMON="$HERE/out-arm/jihad-browserserver-arm"
# goanna.js comes from the assembled BUNDLE, not from the dist: the dist copy is the stock
# upstream file, and the low-RAM / add-on / OMTC pref blocks are appended by
# make-device-bundle.sh. Pushing it here matters because prefs are read once at engine
# startup, so a pref-only change is an engine change — and without this the only way to
# ship one was a full variant push. (2026-08-04: extensions.update.url was fixed on desktop
# and the device silently kept the old file, which would have failed add-on installs there
# for a reason already diagnosed.)
PREFS="$HERE/device-bundle/goanna.js"
TC_DEFAULT="$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi"
STRIP="${STRIP:-$TC_DEFAULT/bin/arm-webos-linux-gnueabi-strip}"

VARIANTS="${*:-enyo mochi mojo}"
appid() { case "$1" in enyo) echo net.riverstonerelay.jihad-browser;; mochi) echo net.riverstonerelay.jihad-browser-mochi;; mojo) echo net.riverstonerelay.jihad-browser-mojo;; *) return 1;; esac; }
job()   { case "$1" in enyo) echo jihad;; mochi) echo jihad-mochi;; mojo) echo jihad-mojo;; *) return 1;; esac; }
die() { echo "ERROR: $*" >&2; exit 1; }
say() { echo "[engine-update] $*"; }

for v in $VARIANTS; do appid "$v" >/dev/null || die "unknown variant '$v'"; done
[ -f "$DAEMON" ] || die "no daemon at $DAEMON — run build-daemon-arm.sh"
[ -f "$PREFS" ] || die "no goanna.js at $PREFS — run make-device-bundle.sh"
XUL=$(readlink -f "$DIST/bin/libxul.so"); [ -f "$XUL" ] || die "no libxul at $DIST/bin/libxul.so — run build-goanna-arm.sh build"
[ -x "$STRIP" ] || die "no cross strip at $STRIP"
novacom -l >/dev/null 2>&1 || die "no device on novacom"

TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
cp -L "$XUL" "$TMP/libxul.so"; cp -L "$DAEMON" "$TMP/jihad-browserserver"
cp -L "$PREFS" "$TMP/goanna.js"
"$STRIP" "$TMP/libxul.so" "$TMP/jihad-browserserver" 2>/dev/null || true
XUL_MD5=$(md5sum "$TMP/libxul.so" | cut -c1-32)
BS_MD5=$(md5sum "$TMP/jihad-browserserver" | cut -c1-32)
PREFS_MD5=$(md5sum "$TMP/goanna.js" | cut -c1-32)
say "libxul $(du -h "$TMP/libxul.so" | cut -f1) md5=$XUL_MD5; daemon $(du -h "$TMP/jihad-browserserver" | cut -f1) md5=$BS_MD5; goanna.js md5=$PREFS_MD5"

push_verified() { # push_verified <local> <devpath> <md5>
  local ok=""
  for try in 1 2 3; do
    novacom put "file://$2" < "$1" || { say "put failed (try $try)"; continue; }
    local r; r=$(novacom run file://usr/bin/md5sum -- "$2" 2>/dev/null | cut -c1-32 || true)
    [ "$r" = "$3" ] && { ok=1; break; }
    say "md5 mismatch on $2 (device ${r:-none}, try $try)"
  done
  [ -n "$ok" ] || die "never verified: $2"
}

for v in $VARIANTS; do
  APP=/media/cryptofs/apps/usr/palm/applications/$(appid "$v")
  HL=$APP/deviceroot/hl
  J=$(job "$v")
  say "[$v] stopping $J"
  # If anything below dies, restart the job on the way out (review F14): the old binaries
  # are still in place until the atomic mv, so a failed push must not leave the variant's
  # daemon stopped.
  trap 'rm -rf "$TMP"; novacom run file://sbin/start -- "'"$J"'" >/dev/null 2>&1 || true' EXIT
  novacom run file://sbin/stop -- "$J" >/dev/null 2>&1 || true
  # Push to a temp name then move: the daemon may respawn mid-push; an atomic-ish rename keeps
  # a half-written libxul from ever being what upstart execs.
  say "[$v] pushing libxul.so"
  push_verified "$TMP/libxul.so" "$HL/.libxul.so.new" "$XUL_MD5"
  say "[$v] pushing jihad-browserserver"
  push_verified "$TMP/jihad-browserserver" "$HL/.jihad-browserserver.new" "$BS_MD5"
  say "[$v] pushing goanna.js"
  push_verified "$TMP/goanna.js" "$HL/.goanna.js.new" "$PREFS_MD5"
  novacom run file://bin/mv -- "$HL/.libxul.so.new" "$HL/libxul.so"
  novacom run file://bin/mv -- "$HL/.jihad-browserserver.new" "$HL/jihad-browserserver"
  novacom run file://bin/mv -- "$HL/.goanna.js.new" "$HL/goanna.js"
  novacom run file://bin/chmod -- 755 "$HL/jihad-browserserver"
  say "[$v] starting $J"
  novacom run file://sbin/start -- "$J" >/dev/null 2>&1 || true
  trap 'rm -rf "$TMP"' EXIT           # variant done: back to plain temp cleanup
done

sleep 3
for v in $VARIANTS; do
  J=$(job "$v")
  S=$(novacom run file://sbin/status -- "$J" 2>/dev/null || true)
  case "$S" in
    *"start/running"*|*"(start) running"*) say "[$v] $J RUNNING" ;;
    *) say "[$v] WARNING: $J status: $S" ;;
  esac
done
say "DONE"
