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
# It also pushes the chrome scripts THIS PROJECT adds to the engine — components/jihad*.js with
# their manifests, the settings page (prefsui/content), and the chrome.manifest that registers
# them — from the assembled bundle.
# They are part of the engine build as far as a variant is concerned: on 2026-10-05 a new
# daemon that expected components/jihadPrint.js was on two variants and the component on one.
# (A changed script is picked up at the next start: the daemon drops the engine's compiled-
# script cache when the bundle's files change. See DropStaleStartupCache in EngineHost.cpp.)
#
# Usage: build/webos-oe/push-engine-update.sh [--daemon-only] [enyo] [mochi] [mojo]
#   --daemon-only   push just the daemon binary (and this project's chrome scripts): the loop
#                   after a daemon-only rebuild, without re-sending 45 MB of libxul per variant.
set -euo pipefail
DAEMON_ONLY=0
if [ "${1:-}" = "--daemon-only" ]; then DAEMON_ONLY=1; shift; fi

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
BUNDLE="$HERE/device-bundle"
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
# ffvpx is dlopen'd at runtime from the engine directory and is part of the engine build, so it
# travels with libxul (see make-device-bundle.sh). FLAC has no other decoder.
for f in libmozavutil.so libmozavcodec.so; do
  [ -e "$DIST/bin/$f" ] || die "no $f at $DIST/bin — run build-goanna-arm.sh build"
  cp -L "$DIST/bin/$f" "$TMP/$f"
done
# plugin-container is the NPAPI (Flash) child process: its own binary, built with the engine.
# It was not pushed until 2026-10-05, when a fix in it would otherwise have stayed on the host.
[ -e "$DIST/bin/plugin-container" ] || die "no plugin-container at $DIST/bin — run build-goanna-arm.sh build"
cp -L "$DIST/bin/plugin-container" "$TMP/plugin-container"
"$STRIP" "$TMP/libxul.so" "$TMP/jihad-browserserver" "$TMP/libmozavutil.so" "$TMP/libmozavcodec.so" "$TMP/plugin-container" 2>/dev/null || true
PC_MD5=$(md5sum "$TMP/plugin-container" | cut -c1-32)
AVU_MD5=$(md5sum "$TMP/libmozavutil.so" | cut -c1-32)
AVC_MD5=$(md5sum "$TMP/libmozavcodec.so" | cut -c1-32)
XUL_MD5=$(md5sum "$TMP/libxul.so" | cut -c1-32)
BS_MD5=$(md5sum "$TMP/jihad-browserserver" | cut -c1-32)
PREFS_MD5=$(md5sum "$TMP/goanna.js" | cut -c1-32)
say "libxul $(du -h "$TMP/libxul.so" | cut -f1) md5=$XUL_MD5; daemon $(du -h "$TMP/jihad-browserserver" | cut -f1) md5=$BS_MD5; goanna.js md5=$PREFS_MD5"

# This project's own chrome scripts, relative to the bundle root.
# ...and the engine's own script modules that this project's engine branch changes (they are
# loose files in the bundle, so a libxul push does not carry them): the add-on manager's provider.
OWN="chrome.manifest jihad-bundle.id jihad-prefsui.manifest modules/addons/XPIProvider.jsm modules/addons/XPIProviderUtils.js $(cd "$BUNDLE" && { ls components/jihad* prefsui/content/* bundled-addons/* 2>/dev/null || true; } | tr '\n' ' ')"
[ -f "$BUNDLE/chrome.manifest" ] || die "no chrome.manifest in $BUNDLE — run make-device-bundle.sh"
[ -f "$BUNDLE/jihad-bundle.id" ] || die "no jihad-bundle.id in $BUNDLE — run make-device-bundle.sh"
# The add-on manager's provider (modules/addons/*) belongs to the engine it was staged with.
# A FULL push sends dist's libxul: if the bundle was assembled from another engine build, the
# provider would be withheld and the device left running a new libxul with its old provider —
# so a stale bundle stops the push here, before anything on the device is touched. (A
# daemon-only push leaves libxul alone; its per-variant check is below.)
BUNDLE_XUL=$(md5sum "$BUNDLE/libxul.so" 2>/dev/null | cut -c1-32)
if [ "$DAEMON_ONLY" = 0 ] && [ "$BUNDLE_XUL" != "$XUL_MD5" ]; then
  die "the bundle's libxul ($BUNDLE_XUL) is not dist's ($XUL_MD5): rerun make-device-bundle.sh before a full push"
fi

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
  say "[$v] pushing jihad-browserserver"
  push_verified "$TMP/jihad-browserserver" "$HL/.jihad-browserserver.new" "$BS_MD5"
  if [ "$DAEMON_ONLY" = 0 ]; then
    say "[$v] pushing libxul.so"
    push_verified "$TMP/libxul.so" "$HL/.libxul.so.new" "$XUL_MD5"
    say "[$v] pushing goanna.js"
    push_verified "$TMP/goanna.js" "$HL/.goanna.js.new" "$PREFS_MD5"
    say "[$v] pushing ffvpx (libmozavutil, libmozavcodec)"
    push_verified "$TMP/libmozavutil.so" "$HL/.libmozavutil.so.new" "$AVU_MD5"
    push_verified "$TMP/libmozavcodec.so" "$HL/.libmozavcodec.so.new" "$AVC_MD5"
    say "[$v] pushing plugin-container"
    push_verified "$TMP/plugin-container" "$HL/.plugin-container.new" "$PC_MD5"
    novacom run file://bin/mv -- "$HL/.libxul.so.new" "$HL/libxul.so"
    novacom run file://bin/mv -- "$HL/.goanna.js.new" "$HL/goanna.js"
    novacom run file://bin/mv -- "$HL/.libmozavutil.so.new" "$HL/libmozavutil.so"
    novacom run file://bin/mv -- "$HL/.libmozavcodec.so.new" "$HL/libmozavcodec.so"
    novacom run file://bin/mv -- "$HL/.plugin-container.new" "$HL/plugin-container"
    novacom run file://bin/chmod -- 755 "$HL/plugin-container"
    novacom run file://bin/chmod -- 755 "$HL/libmozavutil.so" "$HL/libmozavcodec.so"
  fi
  novacom run file://bin/mv -- "$HL/.jihad-browserserver.new" "$HL/jihad-browserserver"
  novacom run file://bin/chmod -- 755 "$HL/jihad-browserserver"
  say "[$v] pushing this project's chrome scripts ($(echo $OWN | wc -w) files)"
  # (a variant installed before 2026-10-05 has no bundled-addons/ directory yet)
  novacom run file://bin/mkdir -- -p "$HL/bundled-addons" >/dev/null 2>&1 || true
  for f in $OWN; do
    # The add-on manager's provider belongs to the engine it was built with. A daemon-only
    # push leaves libxul alone, so the provider goes only when the device already runs the
    # bundle's libxul (else it waits for the next full push with it).
    case "$f" in modules/addons/*)
      # Compared with the BUNDLE's libxul (what the provider was staged with), not dist's:
      # a bundle staged from an older engine build must not pair its provider with a newer
      # libxul either. A full push already proved bundle == dist above; a daemon-only push
      # asks the device what it runs.
      if [ "$DAEMON_ONLY" = 1 ]; then
        DEV_XUL=$(novacom run file://usr/bin/md5sum -- "$HL/libxul.so" 2>/dev/null | cut -c1-32 || true)
      else
        DEV_XUL=$XUL_MD5
      fi
      if [ "$DEV_XUL" != "$BUNDLE_XUL" ]; then
        say "[$v] NOT pushing $f: libxul on the device after this push ($DEV_XUL) is not the bundle's ($BUNDLE_XUL); rerun make-device-bundle.sh"
        continue
      fi;;
    esac
    push_verified "$BUNDLE/$f" "$HL/$f" "$(md5sum "$BUNDLE/$f" | cut -c1-32)"
  done
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
