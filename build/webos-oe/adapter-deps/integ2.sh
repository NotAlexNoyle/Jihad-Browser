#!/bin/sh
# Full device UI integration (v2): gcc4 host-native adapter + Jihad Goanna daemon.
# Fixes v1: kill ANY prior jihad daemon + stale sockets before starting one (v1 left
# a second daemon owning the socket), single clean daemon, fresh launch.
BP=/usr/lib/BrowserPlugins
echo "== install gcc4 adapter (backup stock once) =="
mount -o remount,rw / 2>/dev/null
[ -f "$BP/BrowserAdapter.so.stock" ] || cp "$BP/BrowserAdapter.so" "$BP/BrowserAdapter.so.stock"
cp /media/internal/jihad/BrowserAdapter.so "$BP/BrowserAdapter.so"; chmod 755 "$BP/BrowserAdapter.so"
echo "  installed: $(ls -la $BP/BrowserAdapter.so | awk '{print $5}') bytes"
echo "== clean slate: stop stock server, kill any jihad daemon, drop stale sockets =="
/sbin/stop browserserver 2>/dev/null | tail -1
/usr/bin/pkill -9 -f '/usr/bin/BrowserServer' 2>/dev/null
/usr/bin/pkill -9 -f 'jihad-browserserver' 2>/dev/null
sleep 1
rm -f /tmp/yapserver.browser /tmp/yapserver.browser*-* 2>/dev/null
echo "== start ONE fresh Jihad daemon as 'browser' =="
cd /media/internal/jihad/hl
chmod 755 ld-2.23.so jihad-browserserver 2>/dev/null
export JIHAD_OFFSCREEN=1 JIHAD_DISABLE_OMTC=1 FONTCONFIG_PATH=/etc/fonts JIHAD_BS_NAME=browser
export JIHAD_DUMP=/media/internal/jihad/frame.ppm
rm -f /media/internal/jihad/integ-d.log
( export LD_LIBRARY_PATH=/media/internal/jihad/hl; ./ld-2.23.so --library-path /media/internal/jihad/hl ./jihad-browserserver /media/internal/jihad/hl >/media/internal/jihad/integ-d.log 2>&1 & ) &
i=0; while [ $i -lt 30 ]; do grep -q "serving YAP 'browser'" /media/internal/jihad/integ-d.log 2>/dev/null && break; sleep 1; i=$((i+1)); done
echo "  daemon up in ${i}s"
echo "== launch Jihad UI fresh =="
/usr/bin/luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"net.riverstonerelay.jihad-browser","params":{"target":"data:text/html,<h1>JIHAD BROWSER WORKS</h1><p>Goanna rendering on the TouchPad 12345</p>"}}' 2>&1
sleep 22
echo "== daemon log (expect: client connected -> connect -> openUrl -> msgPainted) =="
grep -vE 'Fontconfig' /media/internal/jihad/integ-d.log 2>/dev/null | tail -25
