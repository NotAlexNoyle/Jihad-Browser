#!/bin/sh
# OFFLINE font/text diagnosis — runs the Jihad daemon + the standalone YAP adapter
# (jihad-adapter-arm), no LunaSysMgr involved. Renders a data: page with big black
# text on white and dumps the first painted frame to frame.ppm. If the text is in
# the PPM, the engine renders fonts on-device; if blank, it's a fontconfig/freetype gap.
cd /media/internal/jihad/hl || exit 9
chmod 755 ld-2.23.so jihad-browserserver jihad-adapter-arm 2>/dev/null
export JIHAD_OFFSCREEN=1 JIHAD_DISABLE_OMTC=1 FONTCONFIG_PATH=/etc/fonts
export LD_LIBRARY_PATH=/media/internal/jihad/hl
export JIHAD_DUMP=/media/internal/jihad/frame.ppm
export JIHAD_URL='https://duckduckgo.com/html/'
rm -f /media/internal/jihad/od-d.log /media/internal/jihad/od-a.log /media/internal/jihad/frame.ppm
( ./ld-2.23.so --library-path /media/internal/jihad/hl ./jihad-browserserver /media/internal/jihad/hl >/media/internal/jihad/od-d.log 2>&1 & )
sleep 16
( ./ld-2.23.so --library-path /media/internal/jihad/hl ./jihad-adapter-arm >/media/internal/jihad/od-a.log 2>&1 & )
sleep 35
/usr/bin/pkill -9 jihad-adapter 2>/dev/null
/usr/bin/pkill -9 jihad-browser 2>/dev/null
echo "offline dump done"
ls -la /media/internal/jihad/frame.ppm 2>/dev/null
