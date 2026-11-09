#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# cert-checks.sh [out.txt]: the certificate kit's device checks on the Enyo variant (cavekit-certificate-trust.md
# R4/R6/R7; the 2026-10-25 round did them by hand). The daemon is started by hand with the inject channel
# (tools/device/t502/gpu-spike-start.sh: the shipped job has no JIHAD_INJECT), each URL is sent to the card
# through it, and the daemon log's load, loaderr, dialog and padlock lines since the mark are the answer; the
# Enyo job is put back after (t502/enyo-restore-job.sh). A page that raises a dialog waits the daemon's 60 s
# default when nothing answers it, so the list is short. Expected: tls-v1-0 refused (the TLS 1.2 floor),
# tls-v1-2, sha256 and Wikipedia loaded with certErr=0, expired / wrong.host / self-signed / untrusted-root
# failed with a certificate error (certErr=1 or an ssl-confirm), never a silent load; revoked.badssl.com LOADS (its CA
# revokes by CRL only and UXP fetches no CRLs: cavekit-certificate-trust.md R4, recorded out of scope).
set -u
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser; cd $R; . build/webos-oe/host-path.sh
OUT=${1:-/dev/stdout}
# JIHAD_DIALOG_MS=4000: an alert or ssl-confirm nobody answers takes its default after 4 s, not 60 (the daemon blocks on it)
build/webos-oe/dev-sh.sh -t 90 tools/device/t502/gpu-spike-start.sh "http://192.168.1.245:8077/jihad-long-static.html" "JIHAD_INJECT=1 JIHAD_DIALOG_MS=4000" 2>&1 | grep 'screenshot at\|FATAL' | tail -n 1
URLS=${URLS:-"https://tls-v1-0.badssl.com:1010/ https://tls-v1-2.badssl.com:1012/ https://sha256.badssl.com/ https://expired.badssl.com/ https://wrong.host.badssl.com/ https://self-signed.badssl.com/ https://untrusted-root.badssl.com/ https://revoked.badssl.com/ https://en.wikipedia.org/wiki/HP_TouchPad"}
S=/tmp/claude-cert-checks.sh
# The daemon's inject and load lines are stdout (buffered when redirected to the log), its dialog lines stderr
# (unbuffered): a per-URL tail of the log mixes old and new lines. So: each URL sent, a fixed wait (alerts take
# their default after JIHAD_DIALOG_MS), and the whole log read once the daemon has exited (its buffers flushed).
cat > $S <<'EOS'
STATE=/var/palm/jihad/enyo
LOG=$STATE/daemon-gpu.log
sleep 8
wc -c < $LOG > /tmp/cert-mark
for u in URLS; do
  umask 077; echo "url $u" > $STATE/inject.cmd
  sleep 14
done
cat /tmp/cert-mark
EOS
sed -i "s|URLS|$URLS|" $S
MARK=$(build/webos-oe/dev-sh.sh -t 300 $S | tail -n 1)
rm -f $S
build/webos-oe/dev-sh.sh -t 90 tools/device/t502/enyo-restore-job.sh 2>&1 | grep 'running\|killing' | sed 's/^/restore: /'
# the hand daemon's log, from the mark: the inject, load, loaderr, dialog and ssl-confirm lines in the order written
novacom get file:///var/palm/jihad/enyo/daemon-gpu.log 2>/dev/null | tail -c +$(( ${MARK:-0} + 1 )) | grep -a 'inject url\|load done\|loaderr\|dialog \|ssl-confirm\|certs:' | cut -c1-240 | tee "$OUT"
