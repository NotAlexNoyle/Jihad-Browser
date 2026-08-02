#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Push one staged variant payload to the device over novacom and lay it down exactly the way
# Preware would: extract into the app's own cryptofs directory, then run this variant's REAL
# packaging/<V>/postinst on-device (as root, which novacom provides). This is the user-approved
# autonomous deploy route for the Mochi/Mojo variants (context/impl/impl-variant-deploy-state.md)
# — palm-install is unusable here (hangs on large .ipks, races concurrent runs, and never runs
# control scripts anyway) and Preware/WOQI needs the user.
#
# Every transfer is md5-verified on BOTH sides before it is trusted (a push died mid-transfer on
# 2026-08-02 and left a zero-byte daemon while novacom's exit status said success), and the
# tarball travels as ONE stream because per-file novacom round-trips over ~2400 GRE files cost
# more than the payload itself.
#
# Usage: build/webos-oe/push-variant.sh <enyo|mochi|mojo>
# Needs: build/webos-oe/build-variant-ipk.sh already run (its out-ipk/work/<V>/ staging is the
# payload source of truth — same bytes the .ipk would carry).
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)

V=${1:?usage: push-variant.sh <enyo|mochi|mojo>}
case "$V" in
  enyo)  APPID="net.riverstonerelay.jihad-browser"       ;;
  mochi) APPID="net.riverstonerelay.jihad-browser-mochi" ;;
  mojo)  APPID="net.riverstonerelay.jihad-browser-mojo"  ;;
  *) echo "ERROR: unknown variant '$V'" >&2; exit 1 ;;
esac

STAGE="$HERE/out-ipk/work/$V"
APP=/media/cryptofs/apps/usr/palm/applications/$APPID
TARBALL_DEV="$APP/.push-payload.tar.gz"
die() { echo "ERROR($V): $*" >&2; exit 1; }
say() { echo "[push-$V] $*"; }

[ -f "$STAGE/appinfo.json" ] || die "no staging at $STAGE — run build-variant-ipk.sh $V first"
novacom -l >/dev/null 2>&1 || die "no device on novacom"

# ── 1. pack the staged tree ──────────────────────────────────────────────────────────────────
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
say "packing $STAGE"
tar -czf "$TMP/payload.tar.gz" -C "$STAGE" .
LOCAL_MD5=$(md5sum "$TMP/payload.tar.gz" | cut -c1-32)
SIZE_KB=$(( $(stat -c %s "$TMP/payload.tar.gz") / 1024 ))
say "payload ${SIZE_KB} KiB md5=$LOCAL_MD5"

# ── 2. push + verify (retry x3) ──────────────────────────────────────────────────────────────
# The device end must exist; the app dir does (all three variants are installed shells).
novacom run file://bin/mkdir -- -p "$APP" >/dev/null
ok=""
for try in 1 2 3; do
  say "push attempt $try (${SIZE_KB} KiB — this is the long part)"
  if ! novacom put "file://$TARBALL_DEV" < "$TMP/payload.tar.gz"; then
    say "novacom put reported failure; retrying"; continue
  fi
  REMOTE_MD5=$(novacom run file://usr/bin/md5sum -- "$TARBALL_DEV" 2>/dev/null | cut -c1-32 || true)
  if [ "$REMOTE_MD5" = "$LOCAL_MD5" ]; then ok=1; break; fi
  say "md5 mismatch (device: '${REMOTE_MD5:-none}') — retrying"
done
[ -n "$ok" ] || die "payload never verified after 3 pushes"
say "payload verified on device"

# ── 3. extract in place ──────────────────────────────────────────────────────────────────────
# --no-same-owner: cryptofs ownership is cosmetic, but tar as root would try to restore the
# host uid; the enyo deploy established the device tar accepts the flag.
say "extracting into $APP"
novacom run file://bin/tar -- -xzf "$TARBALL_DEV" -C "$APP" --no-same-owner \
  || die "on-device extract failed"
novacom run file://bin/rm -- -f "$TARBALL_DEV"

# ── 4. spot-verify the heavy artifacts byte-for-byte ─────────────────────────────────────────
for f in deviceroot/hl/jihad-browserserver deviceroot/hl/libxul.so BrowserAdapterImpl.so; do
  [ -f "$STAGE/$f" ] || continue
  L=$(md5sum "$STAGE/$f" | cut -c1-32)
  R=$(novacom run file://usr/bin/md5sum -- "$APP/$f" 2>/dev/null | cut -c1-32 || true)
  [ "$L" = "$R" ] || die "post-extract md5 mismatch on $f (local $L device ${R:-none})"
  say "verified $f"
done

# ── 5. run the variant's REAL postinst on-device ─────────────────────────────────────────────
# The same bytes Preware would execute: shim -> /usr/lib/BrowserPlugins, impl ->
# /usr/lib/jihad/<V>/, upstart job -> /etc/event.d, state dir, daemon (re)start. novacom runs it
# as root, exactly like the installer. Its output lands here for the log.
say "running packaging/$V/postinst on device"
# md5-verify BEFORE root-executing it (review 2026-08-02 F14): a truncated postinst runs
# as root and can execute a destructive half without its restore half — the same
# silent-truncation failure the payload push already guards against.
PI_MD5=$(md5sum "$REPO/packaging/$V/postinst" | cut -c1-32)
pi_ok=""
for try in 1 2 3; do
  novacom put "file://$APP/.push-postinst.sh" < "$REPO/packaging/$V/postinst" || continue
  R=$(novacom run file://usr/bin/md5sum -- "$APP/.push-postinst.sh" 2>/dev/null | cut -c1-32 || true)
  [ "$R" = "$PI_MD5" ] && { pi_ok=1; break; }
  say "postinst md5 mismatch (device ${R:-none}, try $try)"
done
[ -n "$pi_ok" ] || die "postinst never verified on device — NOT executing it"
novacom run file://bin/sh -- "$APP/.push-postinst.sh" || die "postinst failed on device"
novacom run file://bin/rm -- -f "$APP/.push-postinst.sh"

say "DONE — $APPID deployed. NOTE: the NPAPI shim registers with WebKit only on a full REBOOT."
