# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# device-check.sh: the device as a finger round should leave it (cavekit-finger-testing.md R5; T-564) — the three
# jobs running on their upstart daemons, no hand daemon (one with the inject channel in its environment), no test
# hook in an app directory, no adapter gate, no round gate file or device script in /tmp, no dump or screenshot in
# /tmp, no round scratch under /media/internal, the Default look (each daemon's getTheme answers none), No proxy (no
# proxy user pref). One "DEVICE ok|FAIL item: detail" line each; exit 1 on any FAIL. Runs on the device (dev-sh.sh).
fail=0
ok() { echo "DEVICE ok $1: $2"; }
bad() { echo "DEVICE FAIL $1: $2"; fail=1; }
A=/media/cryptofs/apps/usr/palm/applications
for v in enyo mochi mojo; do
  case $v in enyo) id=net.riverstonerelay.jihad-browser; job=jihad; svc=net.riverstonerelay.jihadBrowser;;
             mochi) id=net.riverstonerelay.jihad-browser-mochi; job=jihad-mochi; svc=net.riverstonerelay.jihadBrowserMochi;;
             mojo) id=net.riverstonerelay.jihad-browser-mojo; job=jihad-mojo; svc=net.riverstonerelay.jihadBrowserMojo;; esac
  HL=$A/$id/deviceroot/hl
  st=$(status $job 2>&1); case "$st" in *running*) ok "job-$v" "$st";; *) bad "job-$v" "$st";; esac
  n=0; hand=0
  for p in /proc/[0-9]*; do c=$(cat $p/cmdline 2>/dev/null | tr '\0' ' '); case "$c" in *jihad-browserserver*"$HL"*) n=$((n+1)); cat $p/environ 2>/dev/null | tr '\0' '\n' | grep -q '^JIHAD_INJECT=' && hand=$((hand+1));; esac; done
  [ $n = 1 ] && ok "daemons-$v" "one daemon" || bad "daemons-$v" "$n daemons"
  [ $hand = 0 ] && ok "hand-daemon-$v" "none" || bad "hand-daemon-$v" "$hand with the inject channel"
  h=$(find $A/$id -iname '*selftest*' 2>/dev/null | wc -l); [ "$h" = 0 ] && ok "hooks-$v" "none" || bad "hooks-$v" "$h test hook(s) in the app directory"
  g=$(ls /var/palm/jihad/$v/ 2>/dev/null | grep -c '^adapterlog$\|^opaque$\|^pgcache$\|^alphawrap$'); [ "$g" = 0 ] && ok "adapter-gates-$v" "none" || bad "adapter-gates-$v" "$g gate file(s) in /var/palm/jihad/$v"
  # No proxy: network.proxy.type absent or 0 (the host lines of the proxy rounds are inert without it)
  pt=$(sed -n 's/.*user_pref("network.proxy.type", *\([0-9]*\)).*/\1/p' $A/$id/profile/prefs.js 2>/dev/null | head -1); [ -z "$pt" ] || [ "$pt" = 0 ] && ok "proxy-$v" "network.proxy.type ${pt:-unset}" || bad "proxy-$v" "network.proxy.type $pt"
  th=$(luna-send -n 1 palm://$svc/getTheme '{}' 2>&1 | tr -d '\n' | cut -c1-120)
  case "$th" in *'"theme"'*|*'"id"'*|*'"persona"'*) case "$th" in *'null'*|*'""'*|*'"none"'*) ok "look-$v" "$th";; *) bad "look-$v" "a persona: $th";; esac;; *) ok "look-$v" "$th";; esac
done
gf=$(ls /tmp/threadstat.on /tmp/renice.main /tmp/jihad-owner-testing 2>/dev/null | tr '\n' ' '); [ -z "$gf" ] && ok "gate-files" "none in /tmp" || bad "gate-files" "$gf"
ds=$(ls /tmp/dev-sh.* 2>/dev/null | grep -v "^$0$" | tr '\n' ' '); [ -z "$ds" ] && ok "device-scripts" "none left in /tmp" || bad "device-scripts" "$ds"
du=$(ls /tmp/frame.raw /tmp/frames.raw /tmp/*.png /tmp/fl-region* /tmp/x.* 2>/dev/null | tr '\n' ' '); [ -z "$du" ] && ok "tmp-dumps" "none" || bad "tmp-dumps" "$du"
ms=$(ls -d /media/internal/jihad-finger /media/internal/jihad-stream 2>/dev/null | tr '\n' ' '); [ -z "$ms" ] && ok "media-scratch" "none" || bad "media-scratch" "$ms"
echo "DEVICE $( [ $fail = 0 ] && echo ok || echo FAIL ) overall: $(df /tmp | tail -1 | awk '{print $5}') of /tmp used"
exit $fail
