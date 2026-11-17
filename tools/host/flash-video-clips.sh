#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# flash-video-clips.sh -- the clips of the codec and size table (cavekit-flash.md R8, T-578), made into the corpus cache: Sorenson
# Spark and H.264 (constrained baseline), each at 240, 360 and 480 lines, 20 s at 24 frames a second, a key frame a second (a seek
# lands within one), a 440 Hz tone in MP3. The picture is ffmpeg's testsrc2: every pixel changes each frame, as video does, and its
# clock makes a frame's time readable in a kept picture. VP6 has no free encoder: its sizes are the corpus's three samples.
# The clips are never stored in the repository; each is read back with ffprobe after encoding.
set -euo pipefail
OUT="${JIHAD_FLASH_CACHE:-$HOME/.cache/jihad-flash}/video"; mkdir -p "$OUT"
ff() { ffmpeg -v error -y "$@"; }
for size in 426x240:400k 640x360:700k 854x480:1200k; do
  wh=${size%%:*}; br=${size##*:}; h=${wh##*x}
  src=(-f lavfi -i "testsrc2=size=$wh:rate=24:duration=20" -f lavfi -i "sine=frequency=440:duration=20")
  ff "${src[@]}" -c:v flv -b:v "$br" -g 24 -c:a libmp3lame -ar 44100 -b:a 64k -flvflags add_keyframe_index "$OUT/gen-spark-$h.flv"
  ff "${src[@]}" -c:v libx264 -profile:v baseline -level 3.0 -pix_fmt yuv420p -b:v "$br" -g 24 -c:a libmp3lame -ar 44100 -b:a 64k -flvflags add_keyframe_index "$OUT/gen-h264-$h.flv"
  for c in spark h264; do
    f="$OUT/gen-$c-$h.flv"
    n=$(ffprobe -v error -select_streams v:0 -count_packets -show_entries stream=nb_read_packets -of csv=p=0 "$f")
    k=$(ffprobe -v error -select_streams v:0 -show_entries stream=codec_name -of csv=p=0 "$f")
    [ "$n" = 480 ] || { echo "FAIL $f: $n frames, 480 wanted"; exit 1; }
    echo "gen-$c-$h.flv $k $wh $n frames $(stat -c %s "$f") bytes"
  done
done
