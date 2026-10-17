#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# fetch-ffmpeg.sh — the FFmpeg source for build-ffmpeg-arm.sh, on the host (the ARM container has no
# network tools), checked against the pinned sha256. Then:
#   build/webos-oe/in-container.sh --arm build/webos-oe/build-ffmpeg-arm.sh
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
. "$HERE/fetch-lib.sh"
VER=4.4.5
SHA=f9514e0d3515aee5a271283df71636e1d1ff7274b15853bcd84e144be416ab07
mkdir -p "$HERE/out-arm/ffmpeg-src"
fetch_url "https://ffmpeg.org/releases/ffmpeg-$VER.tar.xz" "$HERE/out-arm/ffmpeg-src/ffmpeg-$VER.tar.xz" "$SHA"
echo "ffmpeg-$VER.tar.xz in $HERE/out-arm/ffmpeg-src/ (sha256 $SHA)"
