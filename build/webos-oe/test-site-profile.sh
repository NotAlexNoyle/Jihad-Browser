#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Point one variant at a LOCAL copy of the add-on site for a test, and put it back afterwards
# (context/kits/cavekit-addons-site.md R9). The local copy (../Riverstone-Addons/deploy/local/up.sh)
# serves https with its own test certificate authority; this script:
#
#   setup   stops the variant's daemon and backs up, inside the app directory, every profile file a
#           test changes (cert9.db, key4.db, pkcs11.txt, prefs.js, permissions.sqlite, extensions.json,
#           extensions.ini, addons.json, the persisted Persona pictures lightweighttheme-header/-footer)
#           and WHOLE copies of jihad-themes/ and extensions/, with the md5 of every file and the
#           number of entries in each folder (hidden and nested ones too). The backup is written under
#           a temporary name, checked against the profile and marked complete before anything is
#           changed; only then does it add the local test CA to the certificate database (certutil on
#           this PC), write a user.js that points the gallery, the site's Personas page, the install
#           permission, the discover pane and the update service at the local copy, check both
#           arrived whole, and start the daemon again;
#   restore stops the daemon, checks the backup is complete and every file in it is as it was
#           backed up, puts every file back (deleting those that did not exist), swaps jihad-themes/
#           and extensions/ for their copies (so whatever a test added, changed or removed there
#           comes back as it was), removes user.js, checks all of that by md5, starts the daemon,
#           checks the test CA is gone — and only then deletes the backup. When anything did not go
#           back, the daemon is left STOPPED (nothing runs on a half-restored profile) and the backup
#           kept: run restore again.
#
# (Until 2026-10-11 restore removed "what the test added" by matching names against a list with
# grep -x, which the device's BusyBox 1.17 grep lacks: every name looked unlisted, and two restores
# emptied Enyo's jihad-themes/ and extensions/. Nothing here decides what to delete by matching.
# The device's cp can report EBADF for a copy that landed, so no copy is judged by cp's status: each
# is compared by md5 (Codex 2026-10-12, round 17). The device side uses find, md5sum, wc and the
# shell only.)
#   status  says whether a test setup is in place and whether the CA is in the profile.
#
# Usage: build/webos-oe/test-site-profile.sh setup|restore|status [enyo|mochi|mojo]   (default enyo)
#   SITE=https://<addr>:8443  the local copy (default: this PC's LAN address, port 8443)
#   CA=<file>                 its CA (default: ../Riverstone-Addons/deploy/local/state/tls/ca.crt)
set -euo pipefail
. "$(dirname "$0")/host-path.sh"   # the repository's own novacom and palm-* first (R11)
cd "$(dirname "$0")/../.."
ACTION="${1:?setup|restore|status}"; VARIANT="${2:-enyo}"
case "$VARIANT" in
  enyo)  ID=net.riverstonerelay.jihad-browser;       JOB=jihad;;
  mochi) ID=net.riverstonerelay.jihad-browser-mochi; JOB=jihad-mochi;;
  mojo)  ID=net.riverstonerelay.jihad-browser-mojo;  JOB=jihad-mojo;;
  *) echo "variant: enyo, mochi or mojo" >&2; exit 2;;
esac
# The LAN address is needed only to set up, and only when SITE is not given; with no route it is
# empty rather than ending the script, so a restore always runs (inspector 2026-10-11, round 15).
if [ "$ACTION" = setup ] && [ -z "${SITE:-}" ]; then
  ADDR="$(ip -4 route get 1.1.1.1 2>/dev/null | awk '{for (i = 1; i < NF; i++) if ($i == "src") print $(i + 1)}' || true)"
  [ -n "$ADDR" ] || { echo "no LAN address found: set SITE=https://<addr>:8443" >&2; exit 1; }
  SITE="https://$ADDR:8443"
fi
CA="${CA:-$(cd .. && pwd)/Riverstone-Addons/deploy/local/state/tls/ca.crt}"
# JIHAD_TEST_APP: run against a scratch copy of an app directory instead (to test this script).
APP="${JIHAD_TEST_APP:-/media/cryptofs/apps/usr/palm/applications/$ID}"
PROF=$APP/profile
BAK=$APP/jihad-site-test-backup
FILES="cert9.db key4.db pkcs11.txt prefs.js permissions.sqlite extensions.json extensions.ini addons.json lightweighttheme-header lightweighttheme-footer"
DIRS="jihad-themes extensions"
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
# Whether the profile trusts the test CA: "in", "gone", or "unknown" when its database could not be
# fetched or read (never taken for "gone").
ca_state() {
  local f l
  mkdir -p "$TMP/db"
  for f in cert9.db key4.db pkcs11.txt; do novacom get "file://$PROF/$f" > "$TMP/db/$f" || { echo unknown; return; }; done
  l=$(certutil -L -d "sql:$TMP/db") || { echo unknown; return; }
  case "$l" in *"LOCAL TEST CA"*) echo in;; *) echo gone;; esac
}
# Put a file on the device and check it arrived whole (md5), or stop.
put_checked() {
  novacom put "file://$2" < "$1" || { echo "could not put $2" >&2; return 1; }
  local want got
  want=$(md5sum < "$1" | cut -c1-32)
  got=$(dev "sum $2" 4 | grep -o '^[0-9a-f]\{32\}' | head -n 1 || true)
  [ "$got" = "$want" ] || { echo "$2 did not arrive whole" >&2; return 1; }
}
# Run a script on the device, its output back. Its output is all that is believed: every outcome
# that matters is a marker line the caller looks for. Each run has its own file name (and removes
# it), so a put that failed cannot run an earlier script instead.
dev() {
  local f="/tmp/jihad-site-test.$$.$(date +%s%N).sh"
  { echo 'trap '"'"'rm -f "$0"'"'"' EXIT'; printf '%s\n' "$DEVVARS" "$DEVLIB" "$1"; } > "$TMP/run.sh"
  novacom put "file://$f" < "$TMP/run.sh" >/dev/null 2>&1 || { echo "novacom put failed"; return 0; }
  ( sleep "${2:-6}" | novacom run file://bin/sh -- "$f" ) 2>&1 | grep -v "unexpected EOF" || true
}
DEVVARS="PROF=$PROF; BAK=$BAK; FILES='$FILES'; DIRS='$DIRS'; JOB=$JOB"
# The device side's helpers (a quoted here-doc: nothing in it is expanded on this PC).
DEVLIB=$(cat <<'EOF'
sum() { [ -f "$1" ] || return 0; set -- $(md5sum < "$1"); echo "$1"; }
up() { case "$(status "$1" 2>/dev/null)" in *"start/running"*|*"(start) running"*) return 0;; esac; return 1; }
# lists ROOT DIR OUT: write OUT.count (entries under ROOT/DIR, hidden and nested ones too) and
# OUT.md5 (each file's md5 and path); fails on a file it cannot read.
lists() {
  n=$(cd "$1" && find "$2" | wc -l) || return 1
  echo $((n)) > "$3.count" || return 1
  (cd "$1" && find "$2" -type f | while read -r f; do s=$(sum "$f"); [ -n "$s" ] || exit 1; echo "$s $f"; done) > "$3.md5"
}
# treeok ROOT DIR LIST: ROOT/DIR holds as many entries as LIST.count and every file in LIST.md5 as it was.
treeok() {
  n=$(cd "$1" && find "$2" | wc -l) || return 1
  [ "$((n))" = "$(cat "$3.count" 2>/dev/null)" ] || return 1
  [ -f "$3.md5" ] || return 1
  (cd "$1" && while read -r s f; do [ "$(sum "$f")" = "$s" ] || exit 1; done) < "$3.md5"
}
EOF
)

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
  say "stopping $JOB; backing up $FILES and $DIRS"
  # Written as $BAK.tmp and renamed only when every copy succeeded: a setup cut short leaves no
  # backup that restore could take for a whole one. On any failure the daemon is started again and
  # nothing in the profile has been changed.
  out=$(dev "$(cat <<'EOF'
stop $JOB >/dev/null 2>&1; sleep 2
T=$BAK.tmp
fail() { rm -rf $T; start $JOB >/dev/null 2>&1; echo "BACKUP-FAILED: $1"; exit 1; }
rm -rf $T; mkdir -p $T || fail mkdir
: > $T/absent.list || fail absent.list
: > $T/files.md5 || fail files.md5
for f in $FILES; do
  if [ -f $PROF/$f ]; then
    cp -p $PROF/$f $T/$f
    s=$(sum $PROF/$f); [ -n "$s" ] && [ "$(sum $T/$f)" = "$s" ] || fail $f
    echo "$s $f" >> $T/files.md5 || fail files.md5
  else echo $f >> $T/absent.list || fail absent.list; fi
done
for d in $DIRS; do
  if [ -d $PROF/$d ]; then
    lists $PROF $d $T/$d || fail "$d (reading it)"
    cp -a $PROF/$d $T/$d
    treeok $T $d $T/$d || fail "$d (the copy differs)"
  else echo absent > $T/$d.count || fail $d.count; fi
done
echo complete > $T/COMPLETE || fail COMPLETE
mv $T $BAK; [ -f $BAK/COMPLETE ] && [ ! -e $T ] || fail rename
echo BACKUP-OK
EOF
)" 20)
  echo "$out" | grep -v BACKUP-OK || true
  echo "$out" | grep -q BACKUP-OK || { echo "the backup could not be made: nothing was changed" >&2; exit 1; }
  # From here the profile is being changed: stopping part-way leaves the daemon stopped, for restore.
  trap 'rm -rf "$TMP"; echo "setup stopped part-way: $JOB is left stopped; run  $0 restore $VARIANT" >&2' EXIT
  mkdir -p "$TMP/db"
  for f in cert9.db key4.db pkcs11.txt; do novacom get "file://$PROF/$f" > "$TMP/db/$f"; done
  certutil -A -n "Riverstone Add-ons LOCAL TEST CA" -t "C,," -i "$CA" -d "sql:$TMP/db"
  case "$(certutil -L -d "sql:$TMP/db")" in *"LOCAL TEST CA"*) ;; *) echo "the CA did not go in" >&2; exit 1;; esac
  put_checked "$TMP/db/cert9.db" "$PROF/cert9.db"
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
  put_checked "$TMP/user.js" "$PROF/user.js"
  trap 'rm -rf "$TMP"' EXIT
  say "CA added, user.js written ($SITE); starting $JOB"
  out=$(dev 'start $JOB >/dev/null 2>&1; sleep 3; status $JOB; up $JOB && echo DAEMON-UP' 8)
  echo "$out" | grep -v DAEMON-UP || true
  echo "$out" | grep -q DAEMON-UP || { echo "the test is set up but $JOB did not start: start it, or run restore" >&2; exit 1; }
  ;;
restore)
  dev "test -d $BAK || echo NO-BACKUP" 4 | grep -q NO-BACKUP && { echo "no test setup to restore for $VARIANT" >&2; exit 1; }
  say "stopping $JOB; putting the profile back"
  out=$(dev "$(cat <<'EOF'
stop $JOB >/dev/null 2>&1; sleep 2
# Nothing is touched unless the whole backup is there and every file in it is as it was backed up
# (a backup made before 2026-10-12 has no md5 lists: refused too).
bad=""
[ -f $BAK/COMPLETE ] || bad="the backup is not complete"
if [ -z "$bad" ]; then
  [ -f $BAK/files.md5 ] && (cd $BAK && while read -r s f; do [ "$(sum "$f")" = "$s" ] || exit 1; done) < $BAK/files.md5 || bad="a file in the backup is not as it was backed up"
fi
for d in $DIRS; do
  [ -z "$bad" ] || break
  c=$(cat $BAK/$d.count 2>/dev/null)
  [ "$c" = absent ] || treeok $BAK $d $BAK/$d || bad="the backup of $d is not what was there"
done
if [ -n "$bad" ]; then start $JOB >/dev/null 2>&1; echo "$bad: nothing restored"; echo RESTORE-REFUSED; exit 1; fi
ok=1
for f in $FILES; do
  [ -f $BAK/$f ] || continue
  cp -p $BAK/$f $PROF/$f
  [ "$(sum $PROF/$f)" = "$(sum $BAK/$f)" ] || { ok=0; echo "$f did not go back"; }
done
while read -r f; do
  [ -n "$f" ] || continue
  rm -f $PROF/$f; [ ! -e $PROF/$f ] || { ok=0; echo "$f could not be removed"; }
done < $BAK/absent.list
for d in $DIRS; do
  c=$(cat $BAK/$d.count)
  # A restore cut off between its moves: the profile's own folder is the .test-old one.
  [ -d $PROF/$d.test-old ] && [ ! -d $PROF/$d ] && mv $PROF/$d.test-old $PROF/$d
  rm -rf $PROF/$d.test-old
  if [ "$c" = absent ]; then
    rm -rf $PROF/$d; [ ! -e $PROF/$d ] || { ok=0; echo "$d could not be removed"; }
    continue
  fi
  if [ -e $PROF/$d ]; then mv $PROF/$d $PROF/$d.test-old; [ ! -e $PROF/$d ] || { ok=0; echo "$d could not be moved aside"; continue; }; fi
  cp -a $BAK/$d $PROF/$d
  if treeok $PROF $d $BAK/$d; then rm -rf $PROF/$d.test-old; echo "$d: as it was ($c entries)"
  else rm -rf $PROF/$d; [ -d $PROF/$d.test-old ] && mv $PROF/$d.test-old $PROF/$d; ok=0; echo "$d did not go back"; fi
done
rm -f $PROF/user.js; [ ! -e $PROF/user.js ] || { ok=0; echo "user.js could not be removed"; }
if [ $ok = 1 ]; then
  start $JOB >/dev/null 2>&1; sleep 3; status $JOB
  up $JOB && echo DAEMON-UP
  echo COPIED-BACK
else
  echo "$JOB is left stopped: nothing runs on a half-restored profile"
  echo COPY-FAILED
fi
EOF
)" 30)
  echo "$out" | grep -v -e COPIED-BACK -e COPY-FAILED -e RESTORE-REFUSED -e DAEMON-UP || true
  # The CA is looked for whatever happened, so a restore that did not finish says whether the
  # profile still trusts it.
  ca=$(ca_state)
  if echo "$out" | grep -q RESTORE-REFUSED; then
    echo "nothing was restored: the backup is not usable (see above), so the test setup is still in place and the test CA is $([ "$ca" = gone ] && echo "gone" || echo "still in the profile (or unreadable)")" >&2
    exit 1
  fi
  if ! echo "$out" | grep -q COPIED-BACK; then
    echo "the restore did not finish (see above); the test CA is $([ "$ca" = gone ] && echo "gone" || echo "STILL in the profile (or unreadable)"); the backup is kept at $BAK: run restore again" >&2
    exit 1
  fi
  [ "$ca" = gone ] || { echo "the test CA is STILL in the profile (or unreadable): the backup is kept at $BAK" >&2; exit 1; }
  echo "$out" | grep -q DAEMON-UP || echo "WARNING: $JOB did not come back up: start it" >&2
  out=$(dev 'rm -rf $BAK; [ -e $BAK ] || echo BACKUP-GONE' 6)
  echo "$out" | grep -q BACKUP-GONE || { echo "restored, but the backup at $BAK could not be removed: remove it by hand" >&2; exit 1; }
  say "restored: no test CA, no user.js, nothing the test added; backup removed"
  ;;
status)
  dev "test -d $BAK && echo 'test setup: IN PLACE' || echo 'test setup: none'; test -f $PROF/user.js && echo 'user.js: present' || echo 'user.js: none'" 4
  case "$(ca_state)" in in) echo "test CA: in the profile";; gone) echo "test CA: not in the profile";; *) echo "test CA: could not be read";; esac
  ;;
*) echo "setup|restore|status" >&2; exit 2;;
esac
