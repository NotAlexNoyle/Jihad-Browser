#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# T-556 -O2 + PGO (the owner's "redone", 2026-11-11; docs/research/pgo-review-2026-11-11.md section 3): the profiling
# session on the TouchPad with the cheap gen engine (mozconfig.goanna-arm-pgo, phase gen). Every daemon life writes its
# profile on "pgo-dump" before it is stopped (gpu-spike-stop.sh with /tmp/spike-pgo-dump), into GCOV_PREFIX on the
# app's own volume (not the FAT media partition), and libgcov merges each dump into the files. Then the tree is pulled
# beside the gen objects and the use build starts. docs/research/lowpower-uxp.md section 7 is the procedure.
#
#   build/webos-oe/in-container.sh --arm build/webos-oe/build-goanna-arm.sh all 4 mozconfig.goanna-arm-pgo gen   # 4 jobs: earlyoom killed cc1plus at 8 (2026-11-12)
#   tools/device/pgo-profile.sh            (the TouchPad on novacom, nothing else running on it; 3-4 hours)
#   ROUND_DIR=docs/rounds/<dir>            where the rows and logs go (default docs/rounds/pgo-<date>)
#   PGO_WORK=<dir>                         the stripped libraries, the pulled tar, the use build's log
#                                          (default build/webos-oe/out-arm/pgo-work)
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser
S=${PGO_WORK:-$R/build/webos-oe/out-arm/pgo-work}; mkdir -p "$S"
cd "$R" || exit 2
. build/webos-oe/host-path.sh
export JIHAD_REPO=$R
say() { echo; echo "##### $(date +%H:%M:%S) $*"; }
dsh() { { echo "$1"; sleep "${2:-6}"; } | novacom run file://bin/sh 2>&1; }
APP=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser
APPD=$APP/deviceroot
PROF=$APP/pgo-prof
O=build/webos-oe/out-arm/obj-jihad-goanna-arm-pgo
ST=build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi-strip
RD=${ROUND_DIR:-$R/docs/rounds/pgo-$(date +%Y-%m-%d)}
HOST=${HOST:-192.168.1.245:8077}
ENVW="XULDIR=$APPD/hl-pgo JIHAD_TEARDOWN_SEC=300 GCOV_PREFIX=$PROF GCOV_PREFIX_STRIP=0"
cleanup_device() { dsh "rm -rf $APPD/hl-pgo $PROF /media/internal/jihad-pgo2.tar; rm -f /tmp/spike-pgo-dump /tmp/spike-stop-wait; echo cleaned" 10 | tail -1; }

say "the daemon with pgo-dump (all three shells)"
bash build/webos-oe/push-engine-update.sh --daemon-only > $S/push-engine.log 2>&1; echo "daemon push rc=$?"; tail -1 $S/push-engine.log | cut -c1-140
tools/device/close-usb-alert.sh 300

rm -rf $S/libs; mkdir -p $S/libs
for f in $O/dist/bin/*.so; do cp -L "$f" $S/libs/ && $ST $S/libs/$(basename "$f"); done
nm -D $S/libs/libxul.so | grep -q ' T __gcov_flush' || { echo "!! the gen libxul exports no __gcov_flush: stop"; exit 1; }
say "the instrumented libraries ($(ls $S/libs | wc -l), $(du -sm $S/libs | cut -f1) MB stripped) to $APPD/hl-pgo; the profile under $PROF"
dsh "df -k /media/cryptofs | tail -1; mount | grep -m1 ' /media/cryptofs '; rm -rf $APPD/hl-pgo $PROF; mkdir -p $APPD/hl-pgo $PROF; echo 300 > /tmp/spike-stop-wait; touch /tmp/spike-pgo-dump" 6 | tail -3
bad=0
for f in $S/libs/*.so; do
	b=$(basename $f); novacom put file://$APPD/hl-pgo/$b < $f
	want=$(md5sum $f | cut -c1-32); got=$(dsh "md5sum $APPD/hl-pgo/$b" 6 | cut -c1-32)
	[ "$want" = "$got" ] || { echo "!! $b did not arrive whole ($want / $got)"; bad=1; }
done
[ $bad = 0 ] || { cleanup_device; exit 1; }
echo "all $(ls $S/libs | wc -l) arrived whole"
tools/device/close-usb-alert.sh 300

say "a smoke run: one Wikipedia load, then the profile's count (the page must load this time)"
ROUND_DIR=$RD RUNS=1 ONLY="wiki" LEVER=pgo2-smoke EXTRA_ENV="$ENVW" WAIT_SCALE=4 STOP_T=400 tools/device/lowpower-bench.sh 2>&1 | grep -a 'row:\|away\|usb-alert\|!!\|pgo-dump' | cut -c1-200
grep -a 'engine up after\|pgo-dump\|symbol lookup\|load done' $RD/lp-r1-pgo2-smoke-wiki.log | head -5 | cut -c1-160
n=$(dsh "find $PROF -name '*.gcda' | wc -l" 20 | tr -dc 0-9)
echo "profile files after the smoke run: $n"
[ "${n:-0}" -gt 100 ] || { echo "!! the smoke run wrote no profile: stop here"; cleanup_device; tools/device/close-usb-alert.sh 300; exit 1; }

say "the workload, part 1: the bench's own pages twice (Wikipedia load/rest/pan, jsbench, the long page's pan, start cold and warm)"
ROUND_DIR=$RD RUNS=2 ONLY="wiki jsbench long start" LEVER=pgo2-profile EXTRA_ENV="$ENVW" WAIT_SCALE=4 STOP_T=400 tools/device/lowpower-bench.sh 2>&1 | grep -a 'row:\|away\|usb-alert\|!!\|pgo-dump' | cut -c1-200

say "the workload, part 2: the pages no bench row covers, once each (lazy images, image decode, CSS animation, transitions, a form, modern-web, apple.com, animate.style, a YouTube watch page)"
for pg in "http://$HOST/jihad-lazy-images.html" "http://$HOST/jihad-imgdecode.html" "http://$HOST/jihad-anim-css.html" \
		"http://$HOST/jihad-transition.html" "http://$HOST/jihad-tall-input.html" "http://$HOST/modern-web.html" \
		"https://www.apple.com/" "https://animate.style/" "https://m.youtube.com/watch?v=ZgZM0ewM4lU"; do
	nm=$(printf '%s' "$pg" | sed 's#https\?://##; s#[^A-Za-z0-9]#_#g' | cut -c1-40)
	PAGE="$pg" ROUND_DIR=$RD RUNS=1 ONLY="wiki" LEVER=pgo2-$nm EXTRA_ENV="$ENVW" WAIT_SCALE=4 STOP_T=400 tools/device/lowpower-bench.sh 2>&1 | grep -a 'row:\|away\|usb-alert\|!!\|pgo-dump' | cut -c1-160
done

say "the profile: pulled beside the gen objects"
dsh "rm -f /tmp/spike-pgo-dump /tmp/spike-stop-wait; echo files: \$(find $PROF -name '*.gcda' | wc -l); du -sk $PROF; cd $PROF && tar cf /media/internal/jihad-pgo2.tar out && ls -la /media/internal/jihad-pgo2.tar" 90 | tail -3
rm -f $S/jihad-pgo2.tar
novacom get file:///media/internal/jihad-pgo2.tar > $S/jihad-pgo2.tar
find $O -name '*.gcda' -delete
tar xf $S/jihad-pgo2.tar -C build/webos-oe/out-arm --strip-components=1 && echo "pulled: $(find $O -name '*.gcda' | wc -l) gcda files beside the objects"
cleanup_device
tools/device/close-usb-alert.sh 300
if [ "$(find $O -name '*.gcda' | wc -l)" -gt 100 ]; then
	# The gen phase writes some objects under the use phase's .o name (xpcom/base: LinuxUtils, SystemMemoryReporter, its
	# two unified units, every round of 2026-11-12/13), and make then takes them as current: the use link failed on
	# __gcov_init. Any .o that carries the profiler is deleted so the use phase compiles it again.
	for f in $(find $O -name '*.o' -size +0 -print0 | xargs -0 grep -l -a '__gcov_init' 2>/dev/null); do echo "instrumented .o removed: ${f#$O/}"; rm -f "$f"; done
	bash build/webos-oe/in-container.sh --arm build/webos-oe/build-goanna-arm.sh build 4 mozconfig.goanna-arm-pgo use > $S/use-build.log 2>&1
	echo "pgo2-use rc=$?"
	echo "use-phase profile notes: mismatch $(grep -ac 'does not match its profile data' $S/use-build.log), not in profile $(grep -ac 'not found in profile data' $S/use-build.log), no data file $(grep -ac 'profile count data file not found' $S/use-build.log)"
else echo "!! too few profile files: no use build"; fi
