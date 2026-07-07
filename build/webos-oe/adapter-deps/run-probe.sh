#!/bin/sh
# On-device: run the isolated dlopen probe against the cross-built adapter.
# No LunaSysMgr involvement — pure load test.
D=/media/internal/jihad/adaptertest
cd "$D" || exit 9
# Device system libs (Qt4/pbnjson/glib/png) live in the standard paths; include
# the BrowserPlugins dir too in case any dep resolves relative to it.
export LD_LIBRARY_PATH=/usr/lib:/lib:/usr/lib/BrowserPlugins
chmod 755 dlopen_probe 2>/dev/null
./dlopen_probe ./BrowserAdapter.so > "$D/probe.log" 2>&1
echo "exit=$?" >> "$D/probe.log"
