#!/bin/bash
# push-card-js.sh — card-JS dev loop with PROOF of reload (impl-select-popup-2026-08-03.md).
#
# The three lessons this script encodes:
#   1. md5 of the file on disk is NOT proof the card reloaded it → a per-push stamp is sed'd
#      into the JS (@DEV@ placeholder) and must appear in the DEVICE LOG after relaunch.
#   2. The close path must use the REAL processid from applicationManager/running
#      (response field is lowercase "processid"; the close REQUEST wants camelCase "processId").
#   3. All device commands go through a pushed helper script — novacom mangles quoted args.
#
# Usage: push-card-js.sh [enyo|mochi|mojo] [file ...]
#   Default variant enyo, default file source/Browser.js (paths relative to the variant app dir).
#   Flags: --no-sysmgr   skip the LunaSysMgr restart (FASTER but the WebAppMgr
#                        in-process JS cache may serve the PREVIOUS build — proven
#                        2026-08-03: disk md5 new, card booted old stamp. The
#                        stamp check still catches it, so this flag is safe to
#                        try; rerun without it on FAIL)
#          --no-launch   push + close only
set -u
cd "$(dirname "$0")/../.."

VARIANT=enyo
RESTART_SYSMGR=1
DO_LAUNCH=1
FILES=()
for a in "$@"; do
  case "$a" in
    enyo|mochi|mojo) VARIANT=$a ;;
    --restart-sysmgr) RESTART_SYSMGR=1 ;;
    --no-sysmgr) RESTART_SYSMGR=0 ;;
    --no-launch) DO_LAUNCH=0 ;;
    *) FILES+=("$a") ;;
  esac
done
[ ${#FILES[@]} -eq 0 ] && FILES=(source/Browser.js)

case $VARIANT in
  enyo)  APPID=net.riverstonerelay.jihad-browser        APPDIR=app ;;
  mochi) APPID=net.riverstonerelay.jihad-browser-mochi  APPDIR=app-mochi ;;
  mojo)  APPID=net.riverstonerelay.jihad-browser-mojo   APPDIR=app-mojo ;;
esac
DEV_APP=/media/cryptofs/apps/usr/palm/applications/$APPID
STAMP=$(date +%m%d-%H%M%S)
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

die() { echo "FAIL: $*" >&2; exit 1; }

novacom -l | grep -q . || die "no device on novacom"

# --- device helper (subcommands; avoids novacom arg-quoting entirely) ---
cat > "$TMP/helper.sh" <<'HELPER'
#!/bin/sh
case "$1" in
  running) /usr/bin/luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>&1 ;;
  close)   /usr/bin/luna-send -n 1 palm://com.palm.applicationManager/close "{\"processId\":\"$2\"}" 2>&1 ;;
  launch)  /usr/bin/luna-send -n 1 palm://com.palm.applicationManager/launch "{\"id\":\"$2\"}" 2>&1 ;;
  md5)     md5sum "$2" 2>&1 ;;
  logsize)  wc -c < /var/log/messages ;;
  logsince) tail -c +"$2" /var/log/messages | grep "$3" ;; # short call; host loops
  wake)
    # The TouchPad dozes aggressively; a dozing device answers luna-send with
    # EMPTY output (exit 0) and freezes /var/log/messages. Hold it awake:
    # display on while USB-powered + a 15-min power activity, renewed per call.
    /usr/bin/luna-send -n 1 palm://com.palm.display/control/setState '{"state":"unlock"}' >/dev/null 2>&1
    /usr/bin/luna-send -n 1 palm://com.palm.display/control/setProperty '{"onWhenConnected":true}' >/dev/null 2>&1
    /usr/bin/luna-send -n 1 palm://com.palm.display/control/setProperty '{"timeout":3600}' >/dev/null 2>&1
    /usr/bin/luna-send -n 1 palm://com.palm.power/com/palm/power/activityStart '{"id":"net.riverstonerelay.jihad-browser.devloop","duration_ms":900000}' >/dev/null 2>&1
    # health echo: a live display service proves we are actually awake
    /usr/bin/luna-send -n 1 palm://com.palm.display/control/status '{}' 2>&1 ;;
esac
HELPER
novacom put file:///tmp/jihad-card-helper.sh < "$TMP/helper.sh" || die "helper push"
novacom run file://bin/chmod -- +x /tmp/jihad-card-helper.sh || die "helper chmod"
# `novacom run` DISCARDS command output that arrives after host stdin hits EOF
# (the channel close races the reply; proven 2026-08-03: `sleep 3 | novacom run
# probe` returns luna JSON 2/2, `< /dev/null` returns it 0/3). Fast commands
# (echo, md5) usually win the race; luna-send's bus round-trip usually loses it.
# Holding stdin open with `sleep 4 |` removes the race entirely.
dev_raw() { sleep 4 | novacom run file:///tmp/jihad-card-helper.sh -- "$@" 2>/dev/null; }
# An empty reply is still NOT "no result" (a mis-read empty `running` once made
# this script skip the close and "verify" a stale card) — retry, then fail loud.
dev() {
  local out n
  for n in 1 2 3 4 5 6; do
    out=$(dev_raw "$@")
    [ -n "$out" ] && { printf '%s\n' "$out"; return 0; }
    sleep 2
  done
  return 1
}

# The TouchPad autosleeps and a sleeping device silently no-ops luna-send (empty
# replies, frozen /var/log/messages) — wake it and hold it awake, and require the
# display service to actually answer before trusting any later luna-send.
WOKE=$(dev wake) || die "device did not answer wake after retries"
echo "$WOKE" | grep -q '"returnValue":true' || die "display service not answering: $WOKE"

# --- 1. stamp + push + md5 verify (verifies the FILE, not the reload) ---
for f in "${FILES[@]}"; do
  src=$APPDIR/$f
  [ -f "$src" ] || die "$src not found"
  sed "s/@DEV@/$STAMP/g" "$src" > "$TMP/payload"
  novacom put "file://$DEV_APP/$f" < "$TMP/payload" || die "push $f"
  h=$(md5sum "$TMP/payload" | cut -d' ' -f1)
  d=$(dev md5 "$DEV_APP/$f" | cut -d' ' -f1)
  [ "$h" = "$d" ] || die "md5 mismatch on $f (host $h device $d)"
  echo "pushed $f (md5 ok, stamp $STAMP)"
done

# --- 2. close the running card by its real processid ---
RUNNING=$(dev running) || die "running query returned nothing after retries"
echo "$RUNNING" | grep -q '"returnValue": true' || die "running query bad reply: $RUNNING"
PID=$(echo "$RUNNING" | grep -oE "\"id\": \"$APPID\", \"processid\": \"[0-9]+\"" | grep -oE '[0-9]+$' | head -1)
if [ -n "${PID:-}" ]; then
  echo "closing $APPID (processid $PID)"
  CLOSED=$(dev close "$PID") || die "close returned nothing"
  echo "$CLOSED" | grep -q '"returnValue": *true' || die "close refused: $CLOSED"
  sleep 2
  RUNNING2=$(dev running) || die "post-close running query returned nothing"
  echo "$RUNNING2" | grep -q '"returnValue": true' || die "post-close running bad reply"
  echo "$RUNNING2" | grep -q "\"$APPID\"" && die "card still running after close"
  echo "closed."
else
  echo "no running card for $APPID (query healthy — genuinely not running)"
fi

if [ "$RESTART_SYSMGR" = 1 ]; then
  echo "restarting LunaSysMgr..."
  novacom run file://usr/bin/killall -- LunaSysMgr
  sleep 25
fi

[ "$DO_LAUNCH" = 1 ] || exit 0

# --- 3. relaunch + 4. REQUIRE the stamp in the device log (the actual reload proof) ---
# Short novacom calls in a host loop: one long device-side watch dies when the
# link hiccups (seen as "unexpected EOF from server").
OFF=$(dev logsize | tr -dc 0-9)
[ -n "$OFF" ] || die "could not read log size"
echo "launching $APPID; waiting for [JIHAD-BOOT] stamp=$STAMP in /var/log/messages..."
LAUNCHED=$(dev launch "$APPID") || die "launch returned nothing"
echo "$LAUNCHED" | grep -q '"returnValue": *true' || die "launch refused: $LAUNCHED"
for i in $(seq 1 30); do
  OUT=$(dev_raw logsince "$OFF" "JIHAD-BOOT.*$STAMP")   # empty = no match yet, poll on
  [ -n "$OUT" ] && { echo "$OUT"; echo "PASS: card is running THIS push (stamp $STAMP seen in device log)"; exit 0; }
  sleep 1
done
echo "stamp NOT seen in 30s. Any JIHAD-BOOT line since launch (stale build?):"
dev logsince "$OFF" "JIHAD-BOOT" || echo "(none — card log channel dead or card did not start)"
die "card did not prove reload of stamp $STAMP"
