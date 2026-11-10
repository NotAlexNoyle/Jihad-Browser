#!/bin/bash
# Regenerate the ENGINE media test clips (HTML5 <video>/<audio>, nothing to do with Flash).
#
#   make-media-assets.sh <outdir>
#
# The clips are never stored: they are a few hundred KB of ffmpeg test patterns, and a clip left
# in the device's /tmp from an earlier session is not evidence of anything. Each one is checked
# after encoding (codec and frame count read back with ffprobe), because a bad asset is
# indistinguishable from a broken port -- see the SWF authoring traps in docs/PICKUP.md.
#
# Every clip is >= 10 s: the <video> decode failure fixed on 2026-10-01 took 10-60 frames to
# appear, so a 3 s clip can pass on a broken build.
set -euo pipefail
OUT="${1:?usage: make-media-assets.sh <outdir>}"
mkdir -p "$OUT"
cd "$OUT"

V="testsrc=size=320x240:rate=15:duration=10"
A="sine=frequency=440:duration=10"
ff() { ffmpeg -v error -y "$@"; }

ff -f lavfi -i "$V" -c:v libvpx     -b:v 200k -an clip10v.webm        # VP8, video only
ff -f lavfi -i "$V" -c:v libvpx-vp9 -b:v 200k -an clip10vp9.webm      # VP9, video only
# T-558 (cavekit-lowpower-uxp.md R4): the VP9 decoder A/B — ffvpx against libvpx — at YouTube's sizes, 30 fps, 30 s of
# moving detail (testsrc2 changes every pixel each frame, as video does; a static pattern would flatter the decoder)
ff -f lavfi -i "testsrc2=size=640x360:rate=30:duration=30" -c:v libvpx-vp9 -b:v 700k -an clip30vp9-360.webm
ff -f lavfi -i "testsrc2=size=854x480:rate=30:duration=30" -c:v libvpx-vp9 -b:v 1200k -an clip30vp9-480.webm
ff -f lavfi -i "$V" -c:v libtheora  -q:v 5    -an clip10.ogv          # Theora, video only
ff -f lavfi -i "$V" -c:v libx264 -pix_fmt yuv420p -an clip10h264.mp4  # H.264 (no decoder in this build)
# T-594 (cavekit-device-headroom.md R4): the hardware decoder probe (tools/device/omx-probe/) reads raw Annex-B H.264, the
# baseline profile YouTube serves this device (formats 133/134), at its two sizes, 30 s of moving detail
ff -f lavfi -i "testsrc2=size=640x360:rate=30:duration=30" -c:v libx264 -profile:v baseline -level 3.0 -pix_fmt yuv420p -b:v 700k -bsf:v h264_mp4toannexb -an -f h264 clip30h264-360.h264
ff -f lavfi -i "testsrc2=size=854x480:rate=30:duration=30" -c:v libx264 -profile:v baseline -level 3.0 -pix_fmt yuv420p -b:v 1200k -bsf:v h264_mp4toannexb -an -f h264 clip30h264-480.h264
# VP8 + Vorbis: the only clip whose <video> has an audio track, so the only one that can show
# the mute/volume controls enabled on a <video>.
ff -f lavfi -i "testsrc=size=320x240:rate=15:duration=12" -f lavfi -i "sine=frequency=440:duration=12" \
   -c:v libvpx -b:v 200k -c:a libvorbis -q:a 3 clip12av.webm
# 60 s, video only: long enough to pause and resume mid-play (play/pause, scrubber, seek tests).
ff -f lavfi -i "testsrc=size=320x240:rate=15:duration=60" -c:v libvpx -b:v 150k -an clip60v.webm
# 60 s, VP8 + Vorbis: the finger's media-controls scripts need a mute button that is enabled (an audio track)
# AND a clip that outlasts the script's waits (clip12av ended before them, 2026-11-08).
ff -f lavfi -i "testsrc=size=320x240:rate=15:duration=60" -f lavfi -i "sine=frequency=440:duration=60" \
   -c:v libvpx -b:v 150k -c:a libvorbis -q:a 3 clip60av.webm
# 640x360 @ 24 fps: the heaviest clip here, for the CPU-clock measurement (device-build R10).
ff -f lavfi -i "testsrc2=size=640x360:rate=24:duration=20" -c:v libvpx -b:v 700k -an clip20hd.webm
ff -f lavfi -i "sine=frequency=440:duration=15" -c:a libvorbis -q:a 3 tone15.ogg
# 60 s tone: long enough to work the volume controls while it is still playing (?a=tone60.ogg).
ff -f lavfi -i "sine=frequency=440:duration=60" -c:a libvorbis -q:a 3 tone60.ogg
ff -f lavfi -i "$A" -c:a libopus -b:a 48k tone10.opus
ff -f lavfi -i "$A" -c:a pcm_s16le tone10.wav
ff -f lavfi -i "$A" -c:a flac tone10.flac
ff -f lavfi -i "$A" -c:a libmp3lame -q:a 5 tone10.mp3                 # no decoder in this build
ff -f lavfi -i "$A" -c:a aac -b:a 64k tone10.m4a                      # no decoder in this build

fail=0
check() {   # check <file> <codec> [min video frames]
  local codec frames
  codec=$(ffprobe -v error -select_streams "${4:-v:0}" -show_entries stream=codec_name -of csv=p=0 "$1" | head -1)
  if [ "$codec" != "$2" ]; then echo "BAD $1: codec '$codec', wanted '$2'" >&2; fail=1; return; fi
  if [ -n "${3:-}" ]; then
    frames=$(ffprobe -v error -select_streams v:0 -count_frames -show_entries stream=nb_read_frames -of csv=p=0 "$1")
    if [ "$frames" -lt "$3" ]; then echo "BAD $1: $frames frames, wanted >= $3" >&2; fail=1; return; fi
    echo "ok  $1 ($codec, $frames frames)"
  else
    echo "ok  $1 ($codec)"
  fi
}
check clip10v.webm   vp8    150
check clip10vp9.webm vp9    150
check clip10.ogv     theora 150
check clip10h264.mp4 h264   150
check clip30h264-360.h264 h264 900
check clip30h264-480.h264 h264 900
check clip12av.webm  vp8    180
check clip60v.webm   vp8    900
check clip60av.webm  vp8    900
check clip20hd.webm  vp8    480
check clip12av.webm  vorbis "" a:0
check clip60av.webm  vorbis "" a:0
check tone15.ogg     vorbis "" a:0
check tone60.ogg     vorbis "" a:0
check tone10.opus    opus   "" a:0
check tone10.wav     pcm_s16le "" a:0
check tone10.flac    flac   "" a:0
check tone10.mp3     mp3    "" a:0
check tone10.m4a     aac    "" a:0
exit $fail
