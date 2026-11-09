#!/bin/bash
# screen-hold-test.sh <enyo|mochi|mojo> : T-599 (cavekit-gre-widgets.md R1) — the display's timeout is held off while a
# page's media plays. On the device: the display timeout read and set to 30 s, the shell's card launched on the
# 60 s clip with sound (it autoplays), the display's state read every 10 s — expected "on" through the clip's 50th
# second (the timeout alone would have dimmed it at 30) — then, 40 s after the clip's end, expected off or dimmed
# (the hold released); the timeout put back, the card closed. The daemon's "media playing: the card told" lines
# and the card's "[T-599] screen hold" lines are the trace. The shell's job daemon as shipped (no hand daemon).
#   tools/device/finger/screen-hold-test.sh enyo
set -u
R=/home/notalexnoyle/eclipse-workspace/Jihad/Jihad-Browser
cd "$R" || exit 2
. build/webos-oe/host-path.sh
V=${1:-enyo}
case "$V" in enyo) ID=net.riverstonerelay.jihad-browser;; mochi) ID=net.riverstonerelay.jihad-browser-mochi;; mojo) ID=net.riverstonerelay.jihad-browser-mojo;; *) echo "shell?"; exit 2;; esac
HOST=${HOST:-192.168.1.245:8077}
URL="http://$HOST/jihad-media-test.html?src=clip60av.webm&wide=1"
SCR=${TMPDIR:-/tmp}/sht.$$; mkdir -p "$SCR"; trap 'rm -rf "$SCR"' EXIT
cat > "$SCR/dev.sh" <<EOS
ID=$ID; URL='$URL'
L="luna-send -n 1"
state() { \$L palm://com.palm.display/control/status '{}' 2>/dev/null | sed -n 's/.*"state":"\\([a-z]*\\)".*/\\1/p' | head -1; }
old=\$(\$L palm://com.palm.display/control/getProperty '{"properties":["timeout"]}' 2>/dev/null | sed -n 's/.*"timeout": *\\([0-9]*\\).*/\\1/p' | head -1)
owc=\$(\$L palm://com.palm.display/control/getProperty '{"properties":["onWhenConnected"]}' 2>/dev/null | sed -n 's/.*"onWhenConnected": *\\([a-z]*\\).*/\\1/p' | head -1)
echo "display timeout was: \${old:-unknown} s, onWhenConnected \${owc:-unknown}; state \$(state)"
# the card dev loop (push-card-js.sh) keeps the display on while the USB cable is in: off for this test, or the hold
# under test could not be told from it
\$L palm://com.palm.display/control/setProperty '{"timeout":30,"onWhenConnected":false}' >/dev/null 2>&1
\$L palm://com.palm.display/control/setState '{"state":"on"}' >/dev/null 2>&1
: > /tmp/sht.mark; m0=\$(grep -ac . /var/log/messages 2>/dev/null)
\$L palm://com.palm.applicationManager/launch '{"id":"'\$ID'","params":{"target":"'\$URL'"}}' >/dev/null 2>&1
t0=\$(date +%s)
for s in 10 20 30 40 50; do sleep 10; echo "t+\$s s (\$(date +%H:%M:%S)): display \$(state)"; done
echo "--- the clip ends near 65 s; the hold lets go 3 s after the daemon hears it stop, then the 30 s timeout runs"
sleep 30; echo "t+80 s (\$(date +%H:%M:%S)): display \$(state)"
sleep 50; echo "t+130 s (\$(date +%H:%M:%S)): display \$(state) (expected off or dimmed: the hold let go)"
\$L palm://com.palm.display/control/setState '{"state":"on"}' >/dev/null 2>&1
[ -n "\$old" ] && \$L palm://com.palm.display/control/setProperty '{"timeout":'\$old'}' >/dev/null 2>&1 && echo "display timeout restored to \$old s"
[ -n "\$owc" ] && \$L palm://com.palm.display/control/setProperty '{"onWhenConnected":'\$owc'}' >/dev/null 2>&1 && echo "onWhenConnected restored to \$owc"
for pid in \$(\$L palm://com.palm.applicationManager/running '{}' 2>/dev/null | tr ',' '\\n' | grep -A1 "\$ID\\"" | grep processid | sed 's/.*"processid":"\\([0-9]*\\)".*/\\1/'); do \$L palm://com.palm.applicationManager/close '{"processId":"'\$pid'"}' >/dev/null 2>&1; echo "card \$pid closed"; done
echo "--- the daemon's media lines"; grep -a "media playing\|media silent\|video playing\|video stopped" /var/palm/jihad/$V/daemon.log 2>/dev/null | tail -n 4
echo "--- the card's hold lines"; tail -n +\$((m0+1)) /var/log/messages 2>/dev/null | grep -a "T-599" | tail -n 4 | cut -c1-160
EOS
build/webos-oe/dev-sh.sh -t 240 "$SCR/dev.sh" 2>&1
