#!/bin/bash
# three pan runs for the bisect: whole reads, two bands on the full target, the band target
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser; S=$(dirname "$0")
cd $R
for cfg in "whole JIHAD_GPU_WHOLE=1" "twoband JIHAD_GPU_NO_BAND_TARGET=1" "bandtarget"; do
  set -- $cfg; name=$1; shift; words="$*"
  $R/build/webos-oe/dev-sh.sh -t 90 $S/device-clean.sh > /dev/null 2>&1
  $S/pan-run.sh bisect-$name http://192.168.1.245:8077/jihad-long-static.html "IOF=100 $words" > $S/pan-$name.out 2>&1
  echo "=== $name: $(grep 'paint pipeline' $S/pan-$name.out | tail -n 2 | sed 's/.*(\(.*\))/\1/' | tr '\n' ' ')"
  grep '^frame' $S/pan-$name.out | wc -l
  $R/build/webos-oe/dev-sh.sh -t 60 $S/gpu-spike-collect-stop.sh > $S/pan-$name-stop.txt 2>&1
done
