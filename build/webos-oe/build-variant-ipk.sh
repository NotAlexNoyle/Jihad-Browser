#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Build a SELF-CONTAINED, INSTALLABLE .ipk for one Jihad Browser variant — without bitbake.
#
# WHY THIS EXISTS: `palm-package app/` produces a UI-only .ipk (no deviceroot, and no
# postinst/prerm at all — palm-package writes a control.tar.gz containing only ./control).
# Until now the self-contained package existed ONLY on the OE/bitbake path, whose chroot needs
# an interactive sudo password and so cannot be driven unattended. This script produces the same
# payload shape as the OE product recipe (build/webos-oe/recipes-jihad/jihad-ui/jihad-app.inc),
# so the two paths stay equivalent and either can feed the on-device acceptance matrix.
#
# Payload (identical in both paths):
#     <app files>                                   app/ | app-mochi/+frameworks | app-mojo/
#     BrowserAdapterImpl.so                         app root; postinst copies it to the
#                                                   root-owned /usr/lib/jihad/<variant>/
#     deviceroot/hl/…                               engine + daemon + bundled glibc-2.23,
#                                                   RUN IN PLACE from cryptofs (never copied)
#     deviceroot/BrowserPlugins/<variant shim>.so   postinst → /usr/lib/BrowserPlugins/
#     deviceroot/event.d/<variant job>              postinst → /etc/event.d/
#     LICENSE, NOTICE, licenses/
#   control.tar.gz: ./control + ./postinst + ./prerm   (packaging/<variant>/{postinst,prerm})
#
# The control-script injection follows the repack pattern from the webOS knowledge base
# (`webos://knowledge/postinst-packaging`, "The Repack Pattern"), including its two traps:
# build a FRESH archive with `ar rc` rather than updating one in place, and verify the result
# with `ar t` + `tar -tzf <(ar p … control.tar.gz)` before calling it done. Both verifications
# run here, so a malformed .ipk fails at build time instead of on the device.
#
# INSTALL VIA PREWARE OR WEBOS QUICK INSTALL — `palm-install` runs as a non-root user and does
# NOT execute control scripts, so the daemon/shim/upstart would never be laid down.
#
# Usage:
#   build/webos-oe/build-variant-ipk.sh                 all three variants (default)
#   build/webos-oe/build-variant-ipk.sh enyo mochi      just these
# Output: build/webos-oe/out-ipk/<app id>_<version>_all.ipk
#
# Inputs are CONSUMED, never rebuilt here (build them first with build-all-device.sh):
#   engine+daemon bundle   make-device-bundle.sh (re-run here so the .ipk gets the current
#                          out-arm/ daemon + libxul, not a stale device-bundle/)
#   adapter shim + impl    build-adapter-pdk.sh → adapter-deps/build-pdk/{<shim>.so,<V>/…}
#   upstart job + control  packaging/event.d/<job>, packaging/<V>/{postinst,prerm}
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)
OUT="$HERE/out-ipk"                       # git-ignored by /build/**/out-*/
WORK="$HERE/out-ipk/work"
PARTS="$HERE/adapter-deps/build-pdk"      # build-adapter-pdk.sh output tree

die() { echo "ERROR: $*" >&2; exit 1; }
say() { echo; echo "=== $* ==="; }

# ---- variant table: mirrors context/plans/plan-variant-identity.md and packaging/ ------------
# <variant> : <app id> : <shim .so> : <upstart job> : <app source dir>
# The YAP name / socket / impl path / state dir are NOT repeated here — they are compiled into
# the adapter and written into packaging/<V>/*, which this script only copies.
variant_set() {
  case "$1" in
    enyo)  V_APPID="net.riverstonerelay.jihad-browser";       V_SHIM="BrowserAdapterJihad.so";      V_JOB="jihad";       V_SRC="app"       ;;
    mochi) V_APPID="net.riverstonerelay.jihad-browser.mochi"; V_SHIM="BrowserAdapterJihadMochi.so"; V_JOB="jihad-mochi"; V_SRC="app-mochi" ;;
    mojo)  V_APPID="net.riverstonerelay.jihad-browser.mojo";  V_SHIM="BrowserAdapterJihadMojo.so";  V_JOB="jihad-mojo";  V_SRC="app-mojo"  ;;
    *) die "unknown variant '$1' (expected: enyo mochi mojo)" ;;
  esac
}

VARIANTS="${*:-enyo mochi mojo}"
for v in $VARIANTS; do variant_set "$v"; done   # validate the whole list before doing any work

for t in palm-package ar tar rsync; do command -v "$t" >/dev/null || die "$t not on PATH"; done

# ---- 1. the variant-agnostic engine/daemon bundle, assembled ONCE ----------------------------
# make-device-bundle.sh is the supported assembler (soname-named real files, bundled glibc-2.23
# loader, NSS modules, GRE resources, low-RAM goanna.js prefs, required-file manifest). Assemble
# it fresh from the current out-arm/ artifacts rather than reusing a stale device-bundle/.
HL="$HERE/device-bundle"
say "engine+daemon bundle: make-device-bundle.sh -> $HL"
bash "$HERE/make-device-bundle.sh"
[ -x "$HL/jihad-browserserver" ] && [ -f "$HL/ld-2.23.so" ] && [ -f "$HL/libxul.so" ] \
  || die "device bundle incomplete at $HL"

mkdir -p "$OUT"

for V in $VARIANTS; do
  variant_set "$V"
  say "[$V] staging $V_APPID"
  STAGE="$WORK/$V"
  rm -rf "$STAGE"; mkdir -p "$STAGE"

  # ---- 2. the app tree ------------------------------------------------------------------
  # Mochi's tree is app-mochi/ PLUS three bundled framework trees (Enyo 2 + layout + Mochi) that
  # are resolved, pruned, provenance-stamped and QA'd by build-mochi-ipk.sh — reuse that, do not
  # reimplement it. The other two variants are a straight copy of their app dir, minus repo docs
  # (a shipped .ipk once carried stray CLAUDE.md/README.md).
  if [ "$V" = mochi ]; then
    bash "$HERE/build-mochi-ipk.sh" --stage-only "$STAGE"
  else
    rsync -a --exclude=CLAUDE.md --exclude=README.md "$REPO/$V_SRC/" "$STAGE/"
  fi
  [ -f "$STAGE/appinfo.json" ] || die "[$V] no appinfo.json in the staged tree"
  grep -q "\"$V_APPID\"" "$STAGE/appinfo.json" || die "[$V] appinfo.json does not declare id $V_APPID"

  # ---- 3. this variant's adapter impl (app root) ----------------------------------------
  # The shim dlopens the impl from the ROOT-OWNED /usr/lib/jihad/<V>/, which postinst populates
  # from here; cryptofs reports every file 0777, so the app-bundle copy is a delivery vehicle,
  # never a load path (plan-variant-identity.md rule 5).
  [ -f "$PARTS/$V/BrowserAdapterImpl.so" ] \
    || die "[$V] missing $PARTS/$V/BrowserAdapterImpl.so — run build-adapter-pdk.sh $V"
  install -m 0644 "$PARTS/$V/BrowserAdapterImpl.so" "$STAGE/BrowserAdapterImpl.so"

  # ---- 4. deviceroot: engine/daemon run in place + this variant's shim and upstart job ----
  # NOTHING here is copied to /media/internal at install time (R8) — postinst execs it in place
  # from the app's own cryptofs directory.
  install -d "$STAGE/deviceroot/BrowserPlugins" "$STAGE/deviceroot/event.d"
  rsync -a "$HL/" "$STAGE/deviceroot/hl/"
  [ -f "$PARTS/$V_SHIM" ] || die "[$V] missing $PARTS/$V_SHIM — run build-adapter-pdk.sh $V"
  install -m 0755 "$PARTS/$V_SHIM" "$STAGE/deviceroot/BrowserPlugins/$V_SHIM"
  install -m 0644 "$REPO/packaging/event.d/$V_JOB" "$STAGE/deviceroot/event.d/$V_JOB"

  # ---- 5. licence/attribution payload ---------------------------------------------------
  install -m 0644 "$REPO/LICENSE" "$STAGE/LICENSE"
  install -m 0644 "$REPO/NOTICE"  "$STAGE/NOTICE"
  if [ -d "$REPO/licenses" ]; then
    install -d "$STAGE/licenses"; install -m 0644 "$REPO"/licenses/*.txt "$STAGE/licenses/"
  fi

  # ---- 6. payload preflight: exactly what this variant's postinst will demand ------------
  # Same list, same order as packaging/<V>/postinst's own preflight — catch a bad payload here,
  # not on the device.
  for f in deviceroot/hl/jihad-browserserver deviceroot/hl/ld-2.23.so \
           "deviceroot/BrowserPlugins/$V_SHIM" "deviceroot/event.d/$V_JOB" \
           BrowserAdapterImpl.so appinfo.json LICENSE NOTICE; do
    [ -e "$STAGE/$f" ] || die "[$V] staging is missing $f (postinst would abort on-device)"
  done

  # ---- 7. palm-package -------------------------------------------------------------------
  say "[$V] palm-package"
  palm-package "$STAGE" -o "$OUT"
  IPK=$(ls -1t "$OUT/${V_APPID}"_*_all.ipk 2>/dev/null | head -n1 || true)
  [ -n "$IPK" ] || die "[$V] palm-package did not produce ${V_APPID}_*_all.ipk"

  # ---- 8. repack to inject postinst/prerm ------------------------------------------------
  # webos://knowledge/postinst-packaging, "The Repack Pattern". palm-package's control.tar.gz
  # holds only ./control, so extract the three ar members, add the scripts to the control tree,
  # re-gzip it, and build a FRESH ar archive (`ar rc`) — updating the existing one in place is
  # the documented trap.
  say "[$V] injecting postinst/prerm (repack)"
  RW=$(mktemp -d); trap 'rm -rf "$RW"' EXIT
  ( cd "$RW" && ar x "$IPK" )
  for m in debian-binary control.tar.gz data.tar.gz; do
    [ -f "$RW/$m" ] || die "[$V] palm-package .ipk has no $m — cannot repack"
  done
  mkdir -p "$RW/ctrl"
  tar -xzf "$RW/control.tar.gz" -C "$RW/ctrl"
  install -m 0755 "$REPO/packaging/$V/postinst" "$RW/ctrl/postinst"
  install -m 0755 "$REPO/packaging/$V/prerm"    "$RW/ctrl/prerm"
  ( cd "$RW/ctrl" && tar -czf "$RW/control.tar.gz" . )
  ( cd "$RW" && rm -f repacked.ipk && ar rc repacked.ipk debian-binary control.tar.gz data.tar.gz )
  mv "$RW/repacked.ipk" "$IPK"

  # ---- 9. verify the repacked .ipk (build-time, so the device never sees a broken one) ----
  say "[$V] verifying $IPK"
  MEMBERS=$(ar t "$IPK")
  for m in debian-binary control.tar.gz data.tar.gz; do
    case $'\n'"$MEMBERS"$'\n' in *$'\n'"$m"$'\n'*) : ;; *) die "[$V] ar member $m missing from the repacked .ipk" ;; esac
  done
  CTRL=$(tar -tzf <(ar p "$IPK" control.tar.gz))
  for m in ./control ./postinst ./prerm; do
    case $'\n'"$CTRL"$'\n' in *$'\n'"$m"$'\n'*) : ;; *) die "[$V] control.tar.gz is missing $m" ;; esac
  done
  # the control scripts must be executable inside the archive, or ipkg will not run them.
  #
  # NB: none of these checks may pipe into `grep -q` while `set -o pipefail` is in effect.
  # `-q` exits at the FIRST match, the writer upstream gets SIGPIPE, and pipefail promotes that
  # 141 to the pipeline's status — so the check fails precisely when it should PASS. It is also
  # load-bearingly non-deterministic: it depends on whether the match arrives before the writer
  # finishes, so it passed silently while the payload was ~700 entries and started failing the
  # moment the GRE resources took it past 2400. (Same defect class the adapter build scripts hit
  # on 2026-07-31.) Capture first, then match without a pipe.
  CTRLMODES=$(ar p "$IPK" control.tar.gz | tar -tvzf - ./postinst ./prerm)
  case "$CTRLMODES" in
    *-rwxr-xr-x*) : ;;
    *) die "[$V] postinst/prerm are not 0755 inside control.tar.gz" ;;
  esac
  # and the payload must carry the runtime, not just the UI
  DATA=$(ar p "$IPK" data.tar.gz | tar -tzf -)
  for f in "deviceroot/hl/jihad-browserserver" "deviceroot/hl/ld-2.23.so" \
           "deviceroot/BrowserPlugins/$V_SHIM" "deviceroot/event.d/$V_JOB" \
           "BrowserAdapterImpl.so" "LICENSE" "NOTICE"; do
    case $'\n'"$DATA"$'\n' in
      *"/$f"$'\n'*) : ;;
      *) die "[$V] .ipk payload is missing $f" ;;
    esac
  done
  # R8, checked in the shipped artifact: no control script may write to the user's USB volume.
  # Captured, not piped into `grep -q`: under pipefail a SIGPIPE'd producer would make this
  # check report "clean" for a script that DOES touch /media/internal — failing open on the one
  # assertion R8 exists to enforce.
  CTRLTEXT=$(ar p "$IPK" control.tar.gz | tar -xzOf - ./postinst ./prerm | grep -vE '^[[:space:]]*#' || true)
  case "$CTRLTEXT" in
    *"/media/internal"*) die "[$V] a control script still touches /media/internal" ;;
  esac
  rm -rf "$RW"; trap - EXIT
  echo "  ar members : $(echo "$MEMBERS" | tr '\n' ' ')"
  echo "  control    : $(echo "$CTRL" | tr '\n' ' ')"
  echo "  size       : $(( $(stat -c %s "$IPK") / 1024 )) KiB"
  echo "  md5        : $(md5sum "$IPK" | cut -c1-32)"
done

say "DONE — install via Preware or WebOS Quick Install (NOT palm-install)"
for V in $VARIANTS; do
  variant_set "$V"
  ls -1 "$OUT/${V_APPID}"_*_all.ipk 2>/dev/null | tail -n1 | sed 's/^/  /'
done
