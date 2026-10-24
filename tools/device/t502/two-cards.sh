# T-513 (R2's fast flick and two cards in turn), run after gpu-spike-start.sh opened card 1 with
# JIHAD_INJECT=1 JIHAD_GPU_VERIFY=1: a fast flick on card 1, then the app relaunched on <url2> (a
# second card, whose test hook brings card 1 back after its wait), a fast flick on card 2, then one on
# card 1 again through "target focus". Every verify line is printed under the pan it follows.
#   dev-sh.sh -t 200 tools/device/t502/two-cards.sh <url2> ["pan 0 8000 800"]
# Needs the Enyo test hook with its A step (build/webos-oe/card-selftest.sh enyo push).
URL2="$1"; shift
PAN="${*:-pan 0 8000 800}"
ID=net.riverstonerelay.jihad-browser
STATE=/var/palm/jihad/enyo; L=$STATE/daemon-gpu.log
inject() { umask 077; printf '%s\n' "$@" > $STATE/inject.cmd; }
section() {
  echo "=== $1"
  tail -n +$(($2 + 1)) $L | grep -a 'inject\|verify\|focus:\|client connected\|FATAL\|SIGSEGV' | cut -c1-230 | head -n 40
}
m0=$(wc -l < $L)
inject "$PAN"; sleep 8
section "card 1, a fast flick ($PAN)" $m0
luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"'$ID'","params":{"target":"'"$URL2"'","jihadTest":"page:w28;A"}}' >/dev/null 2>&1
sleep 20
m1=$(wc -l < $L)
inject "target last" "$PAN"; sleep 8
section "card 2 ($URL2), a fast flick" $m1
sleep 20   # card 2's hook brings card 1 to the front
m2=$(wc -l < $L)
inject "target focus" "$PAN"; sleep 8
section "card 1 again (target focus), a fast flick" $m2
echo "=== totals"
tail -n +$((m0 + 1)) $L | grep -a 'verify slot' | awk '{for (i=1;i<=NF;i++) if ($i=="rows" && $(i+1)=="wrong:") w+=$(i-1); else if ($i=="rows" && $(i+1)=="structurally") s+=$(i-1); n++} END {printf "%d verify lines, %d rows wrong, %d rows structurally wrong\n", n, w, s}'
( cat /var/log/messages.0 /var/log/messages 2>/dev/null ) | grep 'JIHAD-TEST\] page:w28;A' | sed 's/^[0-9-]*T\([0-9:]*\)[^ ]* .*\(JIHAD-[A-Z]*\]\)/\1 \2/; s/, file:.*//' | tail -4
