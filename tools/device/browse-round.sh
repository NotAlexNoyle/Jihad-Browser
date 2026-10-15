#!/bin/sh
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# A pretend user's half hour on the TouchPad (cavekit-modern-web.md R6). Runs ON THE DEVICE through
# build/webos-oe/dev-sh.sh with a hold of ~2400 s, with tools/device/browse-round-sites.txt put to
# /tmp/round2-sites.txt first (name|url|dwell seconds, one per line):
#   novacom put file:///tmp/round2-sites.txt < tools/device/browse-round-sites.txt
#   build/webos-oe/dev-sh.sh -t 2400 tools/device/browse-round.sh > round.log
# It stops the Mochi job, starts the daemon by hand with JIHAD_JS_CONSOLE=all (page errors reach
# the log; off by default because page text would persist there), visits each site for its dwell,
# polls for the first paint and the first "load done", reads the paint pipeline's frame rate, the
# worst tick, the daemon's RSS and the load average, shoots /tmp/shot-r2-<name>.png, prints one
# "@@" line per visit with the page's distinct console errors under it, then puts the job back.
# Pull the shots with novacom get (one session at a time) and delete them: /tmp is 40 MB.
closeall() {
  for p in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>&1 | tr '{' '\n' | grep '"id": *"net.riverstonerelay.jihad-browser' | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
    luna-send -n 1 palm://com.palm.applicationManager/close "{\"processId\":\"$p\"}" >/dev/null 2>&1
  done
  sleep 4
}
# ── Round 2: a pretend user's half hour in Mochi, the console bridge on all sources ──
L=/var/palm/jihad/mochi/daemon.log
since() { tail -n +$1 $L; }
closeall; stop jihad-mochi >/dev/null 2>&1; sleep 6
sed -n '/^script/,/^end script/p' /etc/event.d/jihad-mochi | sed -e '1d' -e '$d' -e 's/exec env /exec env JIHAD_JS_CONSOLE=all /' > /tmp/jihad-dbg.sh
cd /; sh /tmp/jihad-dbg.sh >/dev/null 2>&1 &
sleep 25; grep 'js-console bridge on' $L | tail -1
rm -f /tmp/shot-r2-*.png
while IFS='|' read -r name url wait; do
  [ -z "$name" ] && continue
  m=$(wc -l < $L); t0=$(date +%s)
  closeall
  host=$(echo "$url" | sed 's|https\?://||; s|/.*||')
  luna-send -n 1 palm://com.palm.applicationManager/launch "{\"id\":\"net.riverstonerelay.jihad-browser-mochi\",\"params\":{\"target\":\"$url\"}}" >/dev/null 2>&1
  # performance: poll once a second for the first paint and the first "load done" of this visit
  tpaint=-; tload=-; i=0
  while [ $i -lt "$wait" ]; do
    sleep 1; i=$((i+1))
    if [ "$tpaint" = "-" ] && since $m | grep -q 'painted shmid'; then tpaint=$i; fi
    if [ "$tload" = "-" ] && since $m | grep -q "load done uri=[^ ]*$host"; then tload=$i; fi
  done
  luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot "{\"file\":\"/tmp/shot-r2-$name.png\"}" >/dev/null 2>&1; sleep 2
  new=$(since $m)
  fps=$(echo "$new" | grep -o '([0-9.]* fps)' | tr -d '() fps' | awk '{n++; s+=$1; if(min==""||$1<min)min=$1} END{if(n) printf "avg %.1f min %.1f n=%d", s/n, min, n; else print "-"}')
  work=$(echo "$new" | grep -o 'work avg=[0-9]* max=[0-9]*' | awk -F'max=' '{if($2>m)m=$2} END{print m+0}')
  pid=$(ps | grep '[j]ihad-browser-mochi/deviceroot/hl/jihad-browserserver' | awk '{print $1}' | head -1); [ -z "$pid" ] && pid=$(ps | grep '[.]/jihad-browserserver' | awk '{print $1}' | head -1)
  rss=$(grep VmRSS /proc/$pid/status 2>/dev/null | awk '{print $2}')
  echo "@@ $name | $url | $(( $(date +%s) - t0 ))s | shot=$(wc -c < /tmp/shot-r2-$name.png 2>/dev/null) | load-done=$(echo "$new" | grep -c 'load done') painted=$(echo "$new" | grep -c 'painted shmid') blocked=$(echo "$new" | grep -c 'blocker: blocked') errors=$(echo "$new" | grep 'js-console' | grep -c -E 'Error|error:') | first-paint=${tpaint}s load=${tload}s fps=$fps worst-tick=${work}ms rss=${rss}kB load1=$(cut -d' ' -f1 /proc/loadavg) | last: $(echo "$new" | grep -o 'load done uri=[^ ]*' | tail -1 | cut -c15-110)"
  echo "$new" | grep 'js-console' | grep -v -E 'blocker:|bundled add-ons|certs:|bridge on|nsSidebar|Content Security Policy' | sed 's/^\[jihad-bs\] js-console //' | cut -c1-260 | sort | uniq -c | sort -rn | head -6 | sed 's/^/   /'
  echo "$new" | grep -E 'crash|SIGSEGV|out of memory|memory pressure|low memory' | head -2 | sed 's/^/   !! /'
  df /tmp | tail -1 | awk '{print "   /tmp used " $3 " of " $2}'
done < /tmp/round2-sites.txt
closeall
for p in $(ps | grep '[j]ihad-browser-mochi' | awk '{print $1}'); do kill $p 2>/dev/null; done; sleep 4
for p in $(ps | grep '[j]ihad-browser-mochi' | awk '{print $1}'); do kill -9 $p 2>/dev/null; done; sleep 2
start jihad-mochi >/dev/null; sleep 10; status jihad-mochi; grep 'js-console bridge on' $L | tail -1
echo "@@ END"
