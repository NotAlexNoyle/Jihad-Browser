# Copyright 2026 NotAlexNoyle. Apache-2.0.
# scoreboard-device.sh APPID URL TAG (device side of tools/device/scoreboard.sh, T-629): one browser on one page, read
# the same way whichever engine draws it. The frame sampler (fbstream) reads the screen from 1 s before the launch to
# the end of six finger drags; a screenshot at rest 30 s after the launch; the resident memory of every process before
# the launch and at rest, so the growth is the browser's whole cost (its engine processes, its card in LunaSysMgr)
# whatever it is made of; the card closed at the end.
ID=$1; URL=$2; TAG=$3
SD=/media/internal/jihad-stream; mkdir -p $SD; rm -f $SD/sb-$TAG.fbs $SD/sb-$TAG.fin $SD/sb-$TAG.done   # not .log: the host opened it as this script's stdout (a glob here unlinked it, and every row lost its growth)
[ -x /tmp/finger ] && [ -x /tmp/fbstream ] || { echo "FATAL: /tmp/finger or /tmp/fbstream missing"; touch $SD/sb-$TAG.done; exit 1; }
closeit() {
  for p in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>&1 | grep -oE "\"id\": \"$ID\", \"processid\": \"[0-9]+\"" | grep -oE '"[0-9]+"$' | tr -d '"'); do
    luna-send -n 1 palm://com.palm.applicationManager/close "{\"processId\":\"$p\"}" >/dev/null 2>&1
  done
}
snap() { for p in /proc/[0-9]*; do r=$(awk '/^VmRSS/{print $2}' $p/status 2>/dev/null); [ -n "$r" ] && echo "${p#/proc/} $r $(tr '\0' ' ' < $p/cmdline 2>/dev/null | cut -c1-70)"; done; }
closeit; sleep 4
snap > /tmp/sb-m0.txt
/tmp/fbstream $SD/sb-$TAG.fbs -s 50 -z 15 > /tmp/sb-fbs.out 2>&1 &
SP=$!
sleep 1
luna-send -n 1 palm://com.palm.applicationManager/launch "{\"id\":\"$ID\",\"params\":{\"target\":\"$URL\"}}" >/dev/null 2>&1
echo "launched $ID"
sleep 30
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot "{\"file\":\"/tmp/sb-$TAG.png\"}" >/dev/null 2>&1
snap > /tmp/sb-m1.txt
sleep 2
i=0; while [ $i -lt 6 ]; do FINGER_TRACE=$SD/sb-$TAG.fin /tmp/finger drag 480 900 480 250 1000; sleep 1; i=$((i + 1)); done
wait $SP
echo "fbstream exit $?"; cat /tmp/sb-fbs.out | tail -2; ls -la $SD/sb-$TAG.fbs $SD/sb-$TAG.fin 2>&1
awk 'NR == FNR { a[$1] = $2; next } { d = $2 - (($1 in a) ? a[$1] : 0); if (d > 2048) { s += d; $2 = ""; printf "  +%d kB%s\n", d, $0 } } END { print "growth " s + 0 " kB" }' /tmp/sb-m0.txt /tmp/sb-m1.txt
closeit
rm -f /tmp/sb-m0.txt /tmp/sb-m1.txt /tmp/sb-fbs.out
touch $SD/sb-$TAG.done
