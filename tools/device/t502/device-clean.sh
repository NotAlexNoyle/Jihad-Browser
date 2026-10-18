HL=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/deviceroot/hl
stop jihad >/dev/null 2>&1; sleep 2
for p in /proc/[0-9]*; do c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null); case "$c" in *jihad-browserserver*"$HL"*) kill -9 ${p#/proc/} 2>/dev/null && echo "killed enyo daemon ${p#/proc/}";; esac; done
n=0; for s in /tmp/yapserver.jihad-browser[0-9]*-*; do [ -S "$s" ] && rm -f "$s" && n=$((n+1)); done; echo "removed $n stale page sockets"
for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 'net.riverstonerelay.jihad-browser"' | grep processid | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"$pid"'"}' >/dev/null 2>&1; echo "closed card $pid"; done
echo "upstart jihad: $(status jihad 2>&1)"; echo "LunaSysMgr: $(pidof LunaSysMgr)"; df -h /tmp | tail -n 1
