#!/bin/bash
# Regenerate the image-decode test pictures (cavekit-lowpower-uxp.md R4, T-558: `image/jxl` in the Accept headers):
# one photograph-like picture in the formats a server chooses between by the Accept header, JPEG XL, WebP and JPEG,
# at the same size and a like quality, for jihad-imgdecode.html to time the engine's decode of each.
#
#   make-image-assets.sh <outdir>
#
# Never stored (a few hundred KB of ImageMagick's plasma): regenerated like the media clips.
set -euo pipefail
OUT=${1:?outdir}
cd "$OUT"
# a 1600x1200 "photograph": a plasma fractal blurred a little (smooth gradients and some noise, as a photo decodes)
magick -size 1600x1200 -seed 7 plasma:fractal -blur 0x1 -quality 85 photo.jpg
magick photo.jpg -quality 85 photo.webp
magick photo.jpg -quality 85 photo.jxl
for f in photo.jpg photo.webp photo.jxl; do
	magick identify -format "%f %m %wx%h %B bytes\n" "$f"
done
