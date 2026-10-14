#!/bin/bash
# Drive a shell's own chrome on the device without a finger, for a TEST RUN.
#
#   card-selftest.sh [mochi|enyo|mojo] push push the working-tree sources of that shell plus its
#                                          test hook (render/goanna/test/<shell>-selftest.js),
#                                          restart LunaSysMgr so the card loads them
#   card-selftest.sh [mochi|enyo|mojo] run <scenario[:arg]> [target-url] [seconds] [shot.jpg]
#                                          close the card, launch it on target-url with that
#                                          scenario, wait, print the "[JIHAD-TEST]" lines (UTC
#                                          times; those logged at or after the screenshot are marked)
#                                          and save a screenshot (portrait, already rotated); a
#                                          target-url of "-" launches with none (the start page).
#                                          The screenshot is taken `seconds` after the launch,
#                                          whether or not the scenario has finished: give the
#                                          steps time, or read the marks.
#   card-selftest.sh [mochi|enyo|mojo] restore  put the shipped manifest back, delete the hook,
#                                          restart LunaSysMgr
#
# The shell defaults to mochi.
# The hook is never packaged: it exists on the device only between `push` and `restore` (or
# until the next .ipk install, which replaces the app directory). What a scenario proves and
# what it cannot is written at the top of the hook file.
set -euo pipefail
. "$(dirname "$0")/host-path.sh"   # the repository's own novacom and palm-* first (R11)
cd "$(dirname "$0")/../.."
SHELL_NAME=mochi
case "${1:-}" in mochi|enyo|mojo) SHELL_NAME=$1; shift;; esac
# SRC_GLOBS: the shell's sources to push (working tree); HOOK: where the hook lives in the app dir.
case $SHELL_NAME in
  mochi) ID=net.riverstonerelay.jihad-browser-mochi; APPDIR=app-mochi; MANIFEST=source/package.js
         MANIFEST_SED='s|"JihadBrowser.js"|"JihadBrowser.js",\n\t"JihadSelfTest.js"|'
         SRC_GLOBS="source/*.js source/*.css index.html"; HOOK=source/JihadSelfTest.js;;
  enyo)  ID=net.riverstonerelay.jihad-browser;       APPDIR=app;       MANIFEST=depends.js
         MANIFEST_SED='s|"source/tld.js"|"source/tld.js",\n\t"source/JihadSelfTest.js"|'
         SRC_GLOBS="source/*.js source/*.css css/*.css"; HOOK=source/JihadSelfTest.js;;
  mojo)  ID=net.riverstonerelay.jihad-browser-mojo;  APPDIR=app-mojo;  MANIFEST=sources.json
         MANIFEST_SED='s|{ "source": "app/assistants/history-assistant.js" }|{ "source": "app/assistants/history-assistant.js" },\n\t{ "source": "app/assistants/JihadSelfTest.js" }|'
         SRC_GLOBS="app/assistants/*.js app/models/*.js app/views/*/*.html stylesheets/*.css index.html"; HOOK=app/assistants/JihadSelfTest.js;;
esac
DEV=/media/cryptofs/apps/usr/palm/applications/$ID
NOVACOM=(novacom)
[ -n "${JIHAD_DEVICE:-}" ] && NOVACOM=(novacom -d "$JIHAD_DEVICE")
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
dev() { ( sleep "${HOLD:-6}" | "${NOVACOM[@]}" run file://bin/sh -- "$@" ) 2>&1; }
put() { "${NOVACOM[@]}" put "file://$2" < "$1"; }
# luna-restart.sh waits for the system UI and closes the "Connected — USB Drive" alert that
# every restart leaves over the top right of the screen (it covered the Enyo toolbar in
# screenshots). It talks to the default device only; with $JIHAD_DEVICE set, fall back to a
# plain restart and a wait.
restart_sysmgr() {
  echo "restarting LunaSysMgr (card JS is cached in it)..."
  if [ -z "${JIHAD_DEVICE:-}" ]; then bash "$(dirname "$0")/luna-restart.sh" || true
  else "${NOVACOM[@]}" run file://usr/bin/killall -- LunaSysMgr || true; sleep 28; fi
}

case "${1:-}" in
push)
  STAMP=$(date +%m%d-%H%M%S)
  for f in $(for g in $SRC_GLOBS; do echo $APPDIR/$g; done) $APPDIR/$MANIFEST; do
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
  put render/goanna/test/$SHELL_NAME-selftest.js "$DEV/$HOOK"
  echo "pushed $SHELL_NAME sources + test hook (stamp $STAMP)"
  restart_sysmgr
  ;;
run)
  SC="${2:?scenario}"; URL="${3:-file:///tmp/jihad-plain.html}"; WAIT="${4:-12}"; SHOT="${5:-}"
  [ "$URL" != - ] || URL=""
  cat > "$TMP/sf.sh" <<DEVEOF
ID=$ID
luna-send -n 1 palm://com.palm.display/control/setState '{"state":"on"}' >/dev/null 2>&1
luna-send -n 1 palm://com.palm.display/control/setState '{"state":"unlock"}' >/dev/null 2>&1
for p in \$(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>&1 | tr '{' '\n' | grep "\"id\": *\"\$ID\"" | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close "{\"processId\":\"\$p\"}" >/dev/null 2>&1; done
sleep 4
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"$ID","params":{"target":"$URL","jihadTest":"$SC"}}' >/dev/null 2>&1
sleep $WAIT
SHOT_AT=\$(date -u +%H:%M:%S)
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/st.png"}' >/dev/null 2>&1; sleep 2
# /var/log/messages rotates within seconds on this device, so an offset taken before the launch
# is not reliable: take every line of THIS scenario from both generations and keep what follows
# its last "start". Each line keeps its UTC time, and a line logged in the screenshot's second or
# later says so (the capture itself takes about 2 s; midnight is allowed for): a picture taken
# before a step ran is not that step's result (T-465 was such a picture).
( zcat /var/log/messages.1.gz /var/log/messages.0.gz 2>/dev/null; cat /var/log/messages.0 /var/log/messages 2>/dev/null ) | grep 'JIHAD-TEST\] $SC: ' | sed 's/^[0-9-]*T\([0-9:]*\)[^ ]* .*\(JIHAD-[A-Z]*\]\)/\1 \2/; s/, file:.*//' | awk -v shot="\$SHOT_AT" 'function sod(t) { return substr(t,1,2)*3600 + substr(t,4,2)*60 + substr(t,7,2) } BEGIN{s=sod(shot)} /: start\$/{buf=""} {m=""; t=sod(\$0); if (t < s - 43200) t += 86400; if (t > s + 43200) t -= 86400; if (t >= s) m="(at or after the screenshot) "; buf=buf m \$0 "\n"} END{printf "%s", buf}' | cut -c1-900
echo "JIHAD-SHOT] screenshot taken at \$SHOT_AT UTC"
DEVEOF
  put "$TMP/sf.sh" /tmp/mochi-sf.sh
  OUT=$(HOLD=$((WAIT + 14)) dev /tmp/mochi-sf.sh | grep 'JIHAD-' || true)
  printf '%s\n' "$OUT"
  printf '%s\n' "$OUT" | grep -q 'JIHAD-TEST' || echo "(no [JIHAD-TEST] line — hook not loaded, or the log rotated)"
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
  "${NOVACOM[@]}" run file://bin/rm -- -f "$DEV/$HOOK"
  echo "shipped $MANIFEST restored, test hook deleted"
  restart_sysmgr
  ;;
*) sed -n 2,17p "$0"; exit 2;;
esac
