#!/bin/bash
# load-error-test.sh <enyo|mochi|mojo> [retry X Y] : T-600 (the card's own page for a failed load, from the Atlas app).
# Without `retry`: the shell's card opened on an address that does not resolve, a screenshot; then on a port of the
# host where nothing listens (refused), a screenshot; the card's and the daemon's lines about the failures. With
# `retry X Y`: a server started on that port (the test pages), the finger taps Try Again at X Y (read off the second
# screenshot), a screenshot after the load, the server stopped. Frames by eye: the panel's words, the address, the
# buttons; after Try Again the page itself.
set -u
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser
cd "$R" || exit 2
export JIHAD_REPO=$R; . build/webos-oe/host-path.sh
trap '"$R/tools/device/close-usb-alert.sh" 300 || true' EXIT   # the Connected alert, looked for on the way out
V=${1:?shell}; shift
case "$V" in enyo) ID=net.riverstonerelay.jihad-browser;; mochi) ID=net.riverstonerelay.jihad-browser-mochi;; mojo) ID=net.riverstonerelay.jihad-browser-mojo;; *) exit 2;; esac
HOSTIP=${HOSTIP:-192.168.1.245}; PORT=${PORT:-8098}
D=${ROUND_DIR:-$R/docs/rounds/refs-2026-11-09}; mkdir -p "$D"
shot() {   # shot NAME: the screen to $D/le-<shell>-NAME.jpg; a shot a USB drop emptied is taken again once the device
	# is back (and its Connected alert closed), so a drop costs time, not the picture (2026-11-09: four shots lost)
	local try
	for try in 1 2; do
		{ echo "luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{\"file\":\"/tmp/le.png\"}' >/dev/null 2>&1; sleep 2; ls /tmp/le.png >/dev/null"; sleep 5; } | novacom run file://bin/sh >/dev/null 2>&1
		novacom get file:///tmp/le.png > "$D/le-$V-$1.png" 2>/dev/null
		{ echo 'rm -f /tmp/le.png'; sleep 3; } | novacom run file://bin/sh >/dev/null 2>&1
		if python3 -c "from PIL import Image; Image.open('$D/le-$V-$1.png').rotate(-90, expand=True).convert('RGB').save('$D/le-$V-$1.jpg', quality=85)" 2>/dev/null; then
			rm -f "$D/le-$V-$1.png"; echo "shot: $D/le-$V-$1.jpg"; return 0
		fi
		rm -f "$D/le-$V-$1.png"
		echo "shot $1: empty (a USB drop?), waiting for the device"; "$R/tools/device/close-usb-alert.sh" 300
	done
	echo "shot $1: FAILED"; return 1
}
launch() {   # launch URL: the card opened on it (relaunched into the open card when one is up)
	{ echo "luna-send -n 1 palm://com.palm.applicationManager/launch '{\"id\":\"$ID\",\"params\":{\"target\":\"$1\"}}' >/dev/null 2>&1"; sleep 4; } | novacom run file://bin/sh >/dev/null 2>&1
}
if [ "${1:-}" = retry ]; then
	X=$2; Y=$3
	( cd render/goanna/test && exec python3 -m http.server "$PORT" >/dev/null 2>&1 ) & SRV=$!
	sleep 2
	{ echo "/tmp/finger tap $X $Y"; sleep 4; } | novacom run file://bin/sh 2>&1 | tail -1
	sleep 8; shot after-retry
	kill $SRV 2>/dev/null; pkill -f "http.server $PORT" 2>/dev/null   # by name too: a server left on the port made the next
	# run's "refused" case load the page (2026-11-09)
	{ echo "grep -a 'load done uri=http://$HOSTIP:$PORT' /var/palm/jihad/$V/daemon*.log 2>/dev/null | tail -n 2"; sleep 4; } | novacom run file://bin/sh 2>&1
	exit 0
fi
# the refused case needs nothing listening on the port
if ss -ltn 2>/dev/null | grep -q ":$PORT "; then
	pkill -f "http.server $PORT" 2>/dev/null; sleep 1
	ss -ltn 2>/dev/null | grep -q ":$PORT " && { echo "!! something listens on port $PORT: the refused case cannot run"; exit 1; }
fi
# the other shells' cards closed first: the screenshot must catch this shell's card in front
{ cat <<'EOS'
for id in net.riverstonerelay.jihad-browser net.riverstonerelay.jihad-browser-mochi net.riverstonerelay.jihad-browser-mojo; do
  for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 "\"$id\"" | grep processid | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
    luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'$pid'"}' >/dev/null 2>&1
  done
done
EOS
sleep 6; } | novacom run file://bin/sh >/dev/null 2>&1
m0=$({ echo 'grep -ac . /var/log/messages'; sleep 3; } | novacom run file://bin/sh 2>/dev/null | tail -1)
launch "http://jihad-no-such-host.invalid/"
sleep 12; shot unresolved
# an address never loaded before: the test server sends no cache headers, so a copy of the page the engine kept
# counts as fresh and is shown without asking the server (2026-11-09: three "refused" shots showed the page)
launch "http://$HOSTIP:$PORT/jihad-colors.html?refused=$(date +%s)"
sleep 10; shot refused
echo "--- the card's lines"
{ echo "tail -n +$((m0+1)) /var/log/messages | grep -a 'load failed\|T-600\|loadFailed\|Load failed' | tail -n 6 | cut -c1-200"; sleep 4; } | novacom run file://bin/sh 2>&1
echo "--- the daemon's"
{ echo "grep -a 'loaderr failed=1' /var/palm/jihad/$V/daemon*.log 2>/dev/null | tail -n 3"; sleep 4; } | novacom run file://bin/sh 2>&1
