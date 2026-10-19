#!/bin/bash
# default-check.sh <enyo|mochi|mojo> [url]: T-510 (cavekit-gpu-rendering R1). On the variant's own
# upstart daemon (the shipped default: the GPU path with the express lane), the card loads a page,
# pans, pinches in and out (its own self-test hook's gesture calls) and is closed; then the daemon
# log's lines since the run: the path it took, the frames each lane published, faults, fallbacks.
set -u
V=$1; URL=${2:-https://en.wikipedia.org/wiki/HP_TouchPad}
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser; D=$R/docs/rounds/t502-2026-10-17; S=$R/tools/device/t502
cd $R; . build/webos-oe/host-path.sh
case $V in
  enyo)  STEPS="page:w12;g2.46,384,470;w3;t300,400;w3;g0.41,384,470;w3"; ST=enyo; ID=net.riverstonerelay.jihad-browser;;
  mochi) STEPS="page:w12;d384,700,0,-500;w2;f0,-1500;w3;g2.46,384,470;w3;g0.41,384,470;w3"; ST=mochi; ID=net.riverstonerelay.jihad-browser-mochi;;
  mojo)  STEPS="page:w12;D384,700,0,-500;w2;Z2.0;w3;Z0.5;w3"; ST=mojo; ID=net.riverstonerelay.jihad-browser-mojo;;
esac
cat > $S/default-close.sh <<EOS
for pid in \$(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\n' | grep -A1 '"$ID"' | grep processid | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
  luna-send -n 1 palm://com.palm.applicationManager/close '{"processId":"'"\$pid"'"}' >/dev/null 2>&1; echo "closed card \$pid"
done
sleep 3
grep -a 'rendering:' /var/palm/jihad/$ST/daemon.log | tail -n 1
EOS
bash build/webos-oe/card-selftest.sh $V push 2>&1 | tail -n 1
mark=$(novacom run file:///usr/bin/wc -- -c /var/palm/jihad/$ST/daemon.log 2>/dev/null | awk '{print $1}')
timeout 300 bash build/webos-oe/card-selftest.sh $V run "$STEPS" "$URL" 60 $D/default-$V.jpg 2>&1 | tail -n 2
build/webos-oe/dev-sh.sh -t 60 $S/default-close.sh 2>&1 | grep 'closed card\|rendering:' | sed "s/^/$V: /"
rm -f $S/default-close.sh
novacom get file:///var/palm/jihad/$ST/daemon.log 2>/dev/null | tail -c +$(( ${mark:-0} + 1 )) > $D/default-$V-daemon.log
L=$D/default-$V-daemon.log
echo "$V: painted $(grep -a -c 'painted shmid' $L), express frames $(grep -a -c 'express frame' $L), zooms $(grep -a -c 'setZoomAndScroll zoom' $L), disconnects $(grep -a -c 'client disconnected' $L), faults $(grep -a -c 'FATAL SIGNAL' $L), GPU off $(grep -a -c 'GPU path is off' $L), express off $(grep -a -c 'express lane off' $L)"
bash build/webos-oe/card-selftest.sh $V restore 2>&1 | tail -n 1
