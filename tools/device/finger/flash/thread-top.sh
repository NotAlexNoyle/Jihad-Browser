#!/bin/sh
# thread-top.sh SECONDS [N]: every thread's CPU over a window, the busiest first, and the whole device's idle: where a frame's time goes
W=${1:-5}; N=${2:-16}
snap() { for f in /proc/[0-9]*/task/[0-9]*/stat; do cat "$f" 2>/dev/null; echo; done | awk 'NF>10 { p=index($0,"("); q=length($0); while (substr($0,q,1)!=")" && q>0) q--; c=substr($0,p+1,q-p-1); gsub(/ /,"_",c); split(substr($0,q+2),a," "); print $1, c, a[12]+a[13], a[2]}'; }
idle() { awk '/^cpu /{print $5, $2+$3+$4+$5+$6+$7+$8}' /proc/stat; }
snap > /tmp/tt-a; set -- $(idle); i0=$1; t0=$2; sleep "$W"; snap > /tmp/tt-b; set -- $(idle); i1=$1; t1=$2
awk -v W="$W" 'NR==FNR { a[$1]=$3; next } { d=$3-a[$1]; if (d>0) printf "%6.1f%% tid %s %s\n", d/W, $1, $2 }' /tmp/tt-a /tmp/tt-b | sort -rn | head -"$N"
echo "device: busy $(awk -v i="$((i1-i0))" -v t="$((t1-t0))" 'BEGIN{printf "%.0f%%", 200*(t-i)/t}') of 200%"
rm -f /tmp/tt-a /tmp/tt-b
