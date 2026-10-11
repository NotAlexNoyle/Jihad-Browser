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
#   restore stops the daemon, puts every backed-up file back, removes user.js and the backup, and
#           starts the daemon again: the profile is as it was, the test CA gone.
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
SITE="${SITE:-https://$ADDR:8443}"
CA="${CA:-$(cd .. && pwd)/Riverstone-Addons/deploy/local/state/tls/ca.crt}"
APP=/media/cryptofs/apps/usr/palm/applications/$ID
PROF=$APP/profile
BAK=$APP/jihad-site-test-backup
FILES="cert9.db key4.db pkcs11.txt prefs.js permissions.sqlite"
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
  if dev "test -d $BAK && echo BACKUP-EXISTS" 4 | grep -q BACKUP-EXISTS; then
    echo "a test setup is already in place for $VARIANT: restore first" >&2; exit 1
  fi
  say "stopping $JOB; backing up $FILES"
  dev "stop $JOB >/dev/null 2>&1; sleep 2; mkdir -p $BAK && for f in $FILES; do [ -f $PROF/\$f ] && cp -p $PROF/\$f $BAK/\$f; done; ls $BAK | tr '\n' ' '" 8
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
  dev "stop $JOB >/dev/null 2>&1; sleep 2; for f in $FILES; do [ -f $BAK/\$f ] && cp -p $BAK/\$f $PROF/\$f; done; rm -f $PROF/user.js; rm -rf $BAK; start $JOB >/dev/null 2>&1; sleep 3; status $JOB" 10
  mkdir -p "$TMP/db"
  for f in cert9.db key4.db pkcs11.txt; do novacom get "file://$PROF/$f" > "$TMP/db/$f"; done
  if certutil -L -d "sql:$TMP/db" | grep -q "LOCAL TEST CA"; then echo "the test CA is STILL in the profile" >&2; exit 1; fi
  say "restored: no test CA, no user.js"
  ;;
status)
  dev "test -d $BAK && echo 'test setup: IN PLACE' || echo 'test setup: none'; test -f $PROF/user.js && echo 'user.js: present' || echo 'user.js: none'" 4
  mkdir -p "$TMP/db"
  for f in cert9.db key4.db pkcs11.txt; do novacom get "file://$PROF/$f" > "$TMP/db/$f"; done
  certutil -L -d "sql:$TMP/db" | grep -q "LOCAL TEST CA" && echo "test CA: in the profile" || echo "test CA: not in the profile"
  ;;
*) echo "setup|restore|status" >&2; exit 2;;
esac
