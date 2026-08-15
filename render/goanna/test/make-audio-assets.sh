#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Jihad Browser — regenerate every Flash audio test asset from scratch, then parse each one back
# out and refuse to ship a broken one.
#
# WHY THIS EXISTS. The .swf files were checked in but the MP3 streams behind them were only ever
# written to /tmp, so the repo could not reproduce its own assets and nobody could tell which
# flags an existing one carried. Three separate asset bugs in this tree each looked exactly like a
# port defect: a DefineSound reusing a DefineShape's character ID, a SeekSamples of 0 on a looped
# MP3, and a StartSound with SyncNoMultiple=0 on a timeline that loops. The verify pass at the end
# is the point of the script, not a formality.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$HERE"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

command -v ffmpeg >/dev/null || { echo "need ffmpeg (libmp3lame)" >&2; exit 1; }

# A BARE frame stream, no ID3 and no Xing/LAME header frame: make-audio-swf.py rejects an ID3 tag,
# and a Xing frame would decode as an extra 1152 samples of silence at the head of every loop.
mp3() {   # $1 out  $2 expr  $3 rate  $4 seconds  [$5 = stereo]
  local ch="${5:+|$2}"
  ffmpeg -v error -y -f lavfi -i "aevalsrc=${2}${ch}:s=$3:d=$4" \
         -codec:a libmp3lame -b:a 64k -write_xing 0 -id3v2_version 0 "$1"
}

SINE440='sin(2*PI*440*t)'

mp3 "$TMP/mono22k.mp3"   "$SINE440" 22050 1
mp3 "$TMP/stereo44k.mp3" "$SINE440" 44100 1 stereo
mp3 "$TMP/sine5s.mp3"    "$SINE440" 22050 5
mp3 "$TMP/sine30s.mp3"   "$SINE440" 22050 30

# The control asset: uncompressed PCM, no codec in the path at all.
python3 make-audio-swf.py -o jihad-audio.swf       --rate 11025 --seconds 1
python3 make-audio-swf.py -o jihad-audio-sine.swf  --rate 22050 --seconds 1 --mp3 "$TMP/mono22k.mp3"
python3 make-audio-swf.py -o jihad-audio-mp3.swf   --rate 22050 --seconds 1 --mp3 "$TMP/mono22k.mp3"
python3 make-audio-swf.py -o jihad-audio-44k.swf   --rate 44100 --seconds 1 --stereo \
                                                   --mp3 "$TMP/stereo44k.mp3"
python3 make-audio-swf.py -o jihad-audio-5s.swf    --rate 22050 --seconds 5 --mp3 "$TMP/sine5s.mp3"

# THE ASSET TO LISTEN TO. Every looped MP3 here clicks once per iteration, and it is not a port
# defect: Flash restarts its MP3 decoder at the loop point and the granule overlap-add has no
# history to work from, so the first samples of each pass are a transient. SeekSamples hides the
# encoder-delay silence but not this. Device-proven 2026-08-10 three ways — the click rate tracks
# the sound's LENGTH (1 s asset clicks ~1/s, 5 s asset ~1/5 s), the PCM asset never clicks because
# it needs no decoder, and the STOCK webOS browser playing the same SWF clicks identically.
# A single-shot sound has no loop point at all, so it is the only MP3 asset that can answer a
# question about audio QUALITY rather than about looping.
python3 make-audio-swf.py -o jihad-audio-long.swf  --rate 22050 --seconds 30 --loops 1 \
                                                   --mp3 "$TMP/sine30s.mp3"

echo
echo "== verify =="
fail=0
for f in jihad-audio.swf jihad-audio-sine.swf jihad-audio-mp3.swf jihad-audio-44k.swf \
         jihad-audio-5s.swf jihad-audio-long.swf; do
  out="$(python3 dump-swf.py "$f")"
  echo "$out"
  if echo "$out" | grep -q -- '<--'; then
    echo "   ^^ $f FAILED verification" >&2
    fail=1
  fi
done
exit $fail
