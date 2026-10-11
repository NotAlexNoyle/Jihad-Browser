#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Point one variant at a LOCAL copy of the add-on site for a test, and put it back afterwards
# (context/kits/cavekit-addons-site.md R9). The local copy (../Riverstone-Addons/deploy/local/up.sh)
# serves https with its own test certificate authority; this script:
#
#   setup   stops the variant's daemon, backs up the profile files it changes (cert9.db, key4.db,
#           pkcs11.txt, prefs.js, permissions.sqlite) inside the app directory, adds the local test CA
#           to a copy of the certificate database (certutil on this PC), writes a user.js that points
#           the gallery, the site's Personas page, the install permission, the discover pane and the
#           update service at the local copy, and starts the daemon again;
#   restore stops the daemon, puts every backed-up file back, deletes the ones that did not exist
#           at setup, removes what the test added to jihad-themes/ and extensions/ (and restores
#           extensions.json), removes user.js, starts the daemon, checks the test CA is gone — and only
#           then deletes the backup: the profile is as it was.
#   status  says whether a test setup is in place and whether the CA is in the profile.
#
# Usage: build/webos-oe/test-site-profile.sh setup|restore|status [enyo|mochi|mojo]   (default enyo)
#   SITE=https://<addr>:8443  the local copy (default: this PC's LAN address, port 8443)
#   CA=<file>                 its CA (default: ../Riverstone-Addons/deploy/local/state/tls/ca.crt)
set -euo pipefail
cd "$(dirname "$0")/../.."
ACTION="${1:?setup|restore|status}"; VARIANT="${2:-enyo}"
case "$VARIANT" in
  enyo)  ID=net.riverstonerelay.jihad-browser;       JOB=jihad;;
  mochi) ID=net.riverstonerelay.jihad-browser-mochi; JOB=jihad-mochi;;
  mojo)  ID=net.riverstonerelay.jihad-browser-mojo;  JOB=jihad-mojo;;
  *) echo "variant: enyo, mochi or mojo" >&2; exit 2;;
esac
ADDR="$(ip -4 route get 1.1.1.1 2>/dev/null | awk '{for (i = 1; i < NF; i++) if ($i == "src") print $(i + 1)}')"
[ -n "${SITE:-}" ] || [ -n "$ADDR" ] || { echo "no LAN address found: set SITE=https://<addr>:8443" >&2; exit 1; }
SITE="${SITE:-https://$ADDR:8443}"
CA="${CA:-$(cd .. && pwd)/Riverstone-Addons/deploy/local/state/tls/ca.crt}"
APP=/media/cryptofs/apps/usr/palm/applications/$ID
PROF=$APP/profile
BAK=$APP/jihad-site-test-backup
FILES="cert9.db key4.db pkcs11.txt prefs.js permissions.sqlite extensions.json"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
# certutil from the engine's own NSS (3.90): a newer system certutil writes PKCS#11 v3.2 trust objects,
# which this NSS does not read — the CA is then in the database but not trusted (measured 2026-10-11).
NSSBIN="$(pwd)/build/desktop/out/obj-jihad-goanna/dist/bin"
UXP_CERTUTIL="$(pwd)/build/desktop/out/obj-jihad-goanna/security/nss/cmd/certutil/certutil_certutil/certutil"
certutil() {
  [ -x "$UXP_CERTUTIL" ] || { echo "no engine-built certutil at $UXP_CERTUTIL (build the desktop engine)" >&2; exit 1; }
  LD_LIBRARY_PATH="$NSSBIN" "$UXP_CERTUTIL" "$@"
}
say() { echo "[site-test] [$VARIANT] $*"; }
dev() {   # run a script on the device, its output back
  printf '%s\n' "$1" > "$TMP/run.sh"
  novacom put file:///tmp/jihad-site-test.sh < "$TMP/run.sh" >/dev/null 2>&1
  ( sleep "${2:-6}" | novacom run file://bin/sh -- /tmp/jihad-site-test.sh ) 2>&1 | grep -v "unexpected EOF" || true
}

case "$ACTION" in
setup)
  [ -f "$CA" ] || { echo "no CA at $CA (start the local copy first)" >&2; exit 1; }
  [ -x "$UXP_CERTUTIL" ] || { echo "no engine-built certutil at $UXP_CERTUTIL (build the desktop engine)" >&2; exit 1; }
  if dev "test -f $PROF/user.js && echo USERJS-EXISTS" 4 | grep -q USERJS-EXISTS; then
    echo "$PROF/user.js already exists: this test would replace it. Move it away first." >&2; exit 1
  fi
  if dev "test -d $BAK && echo BACKUP-EXISTS" 4 | grep -q BACKUP-EXISTS; then
    echo "a test setup is already in place for $VARIANT: restore first" >&2; exit 1
  fi
  say "stopping $JOB; backing up $FILES"
  # Each file is copied, or noted as absent (restore deletes it); what jihad-themes/ and extensions/
  # hold is listed, so restore can remove what the test adds there.
  dev "stop $JOB >/dev/null 2>&1; sleep 2; mkdir -p $BAK && : > $BAK/absent.list && for f in $FILES; do if [ -f $PROF/\$f ]; then cp -p $PROF/\$f $BAK/\$f; else echo \$f >> $BAK/absent.list; fi; done; ls $PROF/jihad-themes > $BAK/jihad-themes.list 2>/dev/null; ls $PROF/extensions > $BAK/extensions.list 2>/dev/null; ls $BAK | tr '\n' ' '" 8
  mkdir -p "$TMP/db"
  for f in cert9.db key4.db pkcs11.txt; do novacom get "file://$PROF/$f" > "$TMP/db/$f"; done
  certutil -A -n "Riverstone Add-ons LOCAL TEST CA" -t "C,," -i "$CA" -d "sql:$TMP/db"
  certutil -L -d "sql:$TMP/db" | grep -q "LOCAL TEST CA" || { echo "the CA did not go in" >&2; exit 1; }
  novacom put "file://$PROF/cert9.db" < "$TMP/db/cert9.db"
  cat > "$TMP/user.js" <<EOF
// Jihad Browser: a LOCAL add-on site test (build/webos-oe/test-site-profile.sh). Removed by "restore".
user_pref("jihad.personas.galleryURL", "$SITE/?component=personas");
user_pref("jihad.personas.siteURL", "$SITE/personas/");
user_pref("xpinstall.whitelist.add", "$SITE");
user_pref("extensions.webservice.discoverURL", "$SITE/?component=discover");
user_pref("extensions.getAddons.browseAddons", "$SITE/");
user_pref("extensions.getAddons.get.url", "$SITE/?component=integration&type=internal&request=get&addonguid=%IDS%&os=%OS%&version=%VERSION%");
user_pref("extensions.getAddons.search.url", "$SITE/?component=integration&type=internal&request=search&q=%TERMS%&locale=%LOCALE%&os=%OS%&version=%VERSION%");
// The local CA is not one the engine ships, and add-on downloads and update checks insist on one by
// default (the real site's Let's Encrypt root is built in): allowed for this test profile only.
user_pref("extensions.install.requireBuiltInCerts", false);
user_pref("extensions.update.requireBuiltInCerts", false);
user_pref("extensions.update.url", "$SITE/?component=aus&reqVersion=%REQ_VERSION%&id=%ITEM_ID%&version=%ITEM_VERSION%&maxAppVersion=%ITEM_MAXAPPVERSION%&status=%ITEM_STATUS%&appID=%APP_ID%&appVersion=%APP_VERSION%&appOS=%APP_OS%&appABI=%APP_ABI%&locale=%APP_LOCALE%&currentAppVersion=%CURRENT_APP_VERSION%&updateType=%UPDATE_TYPE%&compatMode=%COMPATIBILITY_MODE%");
EOF
  novacom put "file://$PROF/user.js" < "$TMP/user.js"
  say "CA added, user.js written ($SITE); starting $JOB"
  dev "start $JOB >/dev/null 2>&1; sleep 3; status $JOB" 8
  ;;
restore)
  dev "test -d $BAK || echo NO-BACKUP" 4 | grep -q NO-BACKUP && { echo "no test setup to restore for $VARIANT" >&2; exit 1; }
  say "stopping $JOB; putting the profile back"
  out=$(dev "stop $JOB >/dev/null 2>&1; sleep 2
ok=1
for f in $FILES; do if [ -f $BAK/\$f ]; then cp -p $BAK/\$f $PROF/\$f || ok=0; fi; done
while read f; do [ -n \"\$f\" ] && rm -f $PROF/\$f; done < $BAK/absent.list
# Membership in plain sh: the device's BusyBox 1.17 grep has no -x, and a grep that fails made
# \"not listed\" true for every file — two restores emptied Enyo's jihad-themes/ and extensions/
# (2026-10-11). The list must also find its own first line, or nothing is removed.
inlist() { while IFS= read -r l; do [ \"\$l\" = \"\$1\" ] && return 0; done < \"\$2\"; return 1; }
prune() {   # prune DIR LIST: remove what DIR holds that LIST did not
  [ -f \"\$2\" ] && cd \"\$1\" 2>/dev/null || return 0
  first=\$(head -n 1 \"\$2\"); if [ -n \"\$first\" ] && ! inlist \"\$first\" \"\$2\"; then echo \"PRUNE-CHECK-FAILED \$1\"; return 0; fi
  for f in *; do [ -e \"\$f\" ] || continue; inlist \"\$f\" \"\$2\" || { echo \"removing \$1/\$f\"; rm -rf \"./\$f\"; }; done
}
prune $PROF/jihad-themes $BAK/jihad-themes.list
prune $PROF/extensions $BAK/extensions.list
rm -f $PROF/user.js
start $JOB >/dev/null 2>&1; sleep 3; status $JOB
[ \$ok = 1 ] && echo COPIED-BACK || echo COPY-FAILED" 12)
  echo "$out" | grep -v -e COPIED-BACK -e COPY-FAILED || true
  echo "$out" | grep -q COPIED-BACK || { echo "a file could not be copied back: the backup is kept at $BAK" >&2; exit 1; }
  mkdir -p "$TMP/db"
  for f in cert9.db key4.db pkcs11.txt; do novacom get "file://$PROF/$f" > "$TMP/db/$f"; done
  if certutil -L -d "sql:$TMP/db" | grep -q "LOCAL TEST CA"; then echo "the test CA is STILL in the profile: the backup is kept at $BAK" >&2; exit 1; fi
  dev "rm -rf $BAK" 4 >/dev/null
  say "restored: no test CA, no user.js, nothing the test added; backup removed"
  ;;
status)
  dev "test -d $BAK && echo 'test setup: IN PLACE' || echo 'test setup: none'; test -f $PROF/user.js && echo 'user.js: present' || echo 'user.js: none'" 4
  mkdir -p "$TMP/db"
  for f in cert9.db key4.db pkcs11.txt; do novacom get "file://$PROF/$f" > "$TMP/db/$f"; done
  certutil -L -d "sql:$TMP/db" | grep -q "LOCAL TEST CA" && echo "test CA: in the profile" || echo "test CA: not in the profile"
  ;;
*) echo "setup|restore|status" >&2; exit 2;;
esac
