#!/bin/bash
# Drive a shell's own chrome on the device without a finger, for a TEST RUN.
#
#   card-selftest.sh [mochi|enyo] push     push the working-tree sources of that shell plus its
#                                          test hook (render/goanna/test/<shell>-selftest.js),
#                                          restart LunaSysMgr so the card loads them
#   card-selftest.sh [mochi|enyo] run <scenario[:arg]> [target-url] [seconds] [shot.jpg]
#                                          close the card, launch it on target-url with that
#                                          scenario, wait, print the "[JIHAD-TEST]" lines and
#                                          save a screenshot (portrait, already rotated)
#   card-selftest.sh [mochi|enyo] restore  put the shipped manifest back, delete the hook,
#                                          restart LunaSysMgr
#
# The shell defaults to mochi.
# The hook is never packaged: it exists on the device only between `push` and `restore` (or
# until the next .ipk install, which replaces the app directory). What a scenario proves and
# what it cannot is written at the top of the hook file.
set -euo pipefail
cd "$(dirname "$0")/../.."
SHELL_NAME=mochi
case "${1:-}" in mochi|enyo) SHELL_NAME=$1; shift;; esac
case $SHELL_NAME in
  mochi) ID=net.riverstonerelay.jihad-browser-mochi; APPDIR=app-mochi; MANIFEST=source/package.js
         MANIFEST_SED='s|"JihadBrowser.js"|"JihadBrowser.js",\n\t"JihadSelfTest.js"|';;
  enyo)  ID=net.riverstonerelay.jihad-browser;       APPDIR=app;       MANIFEST=depends.js
         MANIFEST_SED='s|"source/tld.js"|"source/tld.js",\n\t"source/JihadSelfTest.js"|';;
esac
DEV=/media/cryptofs/apps/usr/palm/applications/$ID
NOVACOM=(novacom)
[ -n "${JIHAD_DEVICE:-}" ] && NOVACOM=(novacom -d "$JIHAD_DEVICE")
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
dev() { ( sleep "${HOLD:-6}" | "${NOVACOM[@]}" run file://bin/sh -- "$@" ) 2>&1; }
put() { "${NOVACOM[@]}" put "file://$2" < "$1"; }
restart_sysmgr() { echo "restarting LunaSysMgr (card JS is cached in it)..."; "${NOVACOM[@]}" run file://usr/bin/killall -- LunaSysMgr || true; sleep 28; }

case "${1:-}" in
push)
  STAMP=$(date +%m%d-%H%M%S)
  for f in $APPDIR/source/*.js $APPDIR/source/*.css $APPDIR/$MANIFEST; do
    [ -f "$f" ] || continue
    rel=${f#$APPDIR/}; n=$(basename "$f")
    if [ "$rel" = "$MANIFEST" ]; then
      # the hook loads last, after the shell's own sources
      sed "$MANIFEST_SED" "$f" > "$TMP/$n"
      grep -q JihadSelfTest "$TMP/$n" || { echo "could not add the hook to $MANIFEST" >&2; exit 1; }
    else
      sed "s/@DEV@/$STAMP/g" "$f" > "$TMP/$n"
    fi
    put "$TMP/$n" "$DEV/$rel"
  done
  put render/goanna/test/$SHELL_NAME-selftest.js "$DEV/source/JihadSelfTest.js"
  echo "pushed $SHELL_NAME sources + test hook (stamp $STAMP)"
  restart_sysmgr
  ;;
run)
  SC="${2:?scenario}"; URL="${3:-file:///tmp/jihad-plain.html}"; WAIT="${4:-12}"; SHOT="${5:-}"
  cat > "$TMP/sf.sh" <<DEVEOF
ID=$ID
luna-send -n 1 palm://com.palm.display/control/setState '{"state":"on"}' >/dev/null 2>&1
luna-send -n 1 palm://com.palm.display/control/setState '{"state":"unlock"}' >/dev/null 2>&1
for p in \$(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>&1 | tr '{' '\n' | grep "\"id\": *\"\$ID\"" | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close "{\"processId\":\"\$p\"}" >/dev/null 2>&1; done
sleep 4
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"$ID","params":{"target":"$URL","jihadTest":"$SC"}}' >/dev/null 2>&1
sleep $WAIT
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/st.png"}' >/dev/null 2>&1; sleep 2
# /var/log/messages rotates within seconds on this device, so an offset taken before the launch
# is not reliable: take every line of THIS scenario from both generations and keep what follows
# its last "start".
cat /var/log/messages.0 /var/log/messages 2>/dev/null | grep 'JIHAD-TEST\] $SC: ' | sed 's/.*\(JIHAD-[A-Z]*\]\)/\1/; s/, file:.*//' | awk '/: start\$/{buf=""} {buf=buf \$0 "\n"} END{printf "%s", buf}' | cut -c1-900
DEVEOF
  put "$TMP/sf.sh" /tmp/mochi-sf.sh
  HOLD=$((WAIT + 14)) dev /tmp/mochi-sf.sh | grep 'JIHAD-' || echo "(no [JIHAD-TEST] line — hook not loaded, or the log rotated)"
  if [ -n "$SHOT" ]; then
    "${NOVACOM[@]}" get file:///tmp/st.png > "$TMP/st.png"
    python3 -c "
from PIL import Image
Image.open('$TMP/st.png').rotate(-90, expand=True).convert('RGB').resize((576,768)).save('$SHOT', quality=85)"
    echo "screenshot: $SHOT"
  fi
  "${NOVACOM[@]}" run file://bin/rm -- -f /tmp/st.png >/dev/null 2>&1 || true
  ;;
restore)
  put $APPDIR/$MANIFEST "$DEV/$MANIFEST"
  "${NOVACOM[@]}" run file://bin/rm -- -f "$DEV/source/JihadSelfTest.js"
  echo "shipped $MANIFEST restored, test hook deleted"
  restart_sysmgr
  ;;
*) sed -n 2,17p "$0"; exit 2;;
esac
