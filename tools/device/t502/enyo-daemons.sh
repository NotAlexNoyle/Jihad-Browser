HL=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/deviceroot/hl
ls -la /tmp/yapserver* 2>&1 | cut -c1-100
for p in /proc/[0-9]*; do
  c=$(tr '\0' ' ' < $p/cmdline 2>/dev/null)
  case "$c" in *jihad-browserserver*"$HL"*)
    pid=${p#/proc/}; echo "enyo daemon $pid state $(grep State $p/status | cut -f2) threads $(ls $p/task | wc -l)"
    for t in $p/task/*; do n=$(cat $t/comm); case "$n" in JihadRead*|Compositor*) printf '   %s wchan=%s\n' "$n" "$(cat $t/wchan)";; esac; done
    kill -9 $pid 2>/dev/null && echo "   killed -9"
    ;;
  esac
done
sleep 1
echo "upstart jihad: $(status jihad 2>&1)"
