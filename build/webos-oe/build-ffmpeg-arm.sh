#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# build-ffmpeg-arm.sh — FFmpeg's H.264, AAC and MP3 decoders for the device: libavcodec.so.58 and
# libavutil.so.56, cross-built in the ARM container. The engine is built with MOZ_FFMPEG and loads
# a libavcodec 58–61 at run time (dom/media/platforms/ffmpeg/FFmpegRuntimeLinker.cpp) for the
# formats its own ffvpx lacks — VP8/VP9/Opus/Vorbis/FLAC are built in, H.264 and AAC are not, and
# until this every MP4 <video> and every H.264 Media Source stream was "format not supported".
# FFmpeg 4.4.5, LGPL v2.1 or later (no --enable-gpl, no --enable-nonfree); decoders and parsers
# only, no demuxers (the engine demuxes), no programs, no filters, no scaling. Linked against the
# toolchain's own sysroot (glibc 2.23, the device's), not the Debian one the engine takes GTK from:
# that one has no C runtime objects for this gcc (crt1.o).
#
#   build/webos-oe/in-container.sh --arm build/webos-oe/build-ffmpeg-arm.sh
#   → build/webos-oe/out-arm/ffmpeg-arm/lib/libavcodec.so.58, libavutil.so.56 (+ COPYING.LGPLv2.1)
# make-device-bundle.sh ships the two libraries when they are there.
set -euo pipefail
VER=4.4.5
SHA=f9514e0d3515aee5a271283df71636e1d1ff7274b15853bcd84e144be416ab07
SRC=/out/ffmpeg-src; OUT=/out/ffmpeg-arm
mkdir -p "$SRC" "$OUT"
cd "$SRC"
# The container has no curl: the tarball is fetched on the host by build/webos-oe/fetch-ffmpeg.sh
# (pinned sha256) into out-arm/ffmpeg-src/, which is /out/ffmpeg-src here.
if [ ! -f "ffmpeg-$VER.tar.xz" ]; then echo "ERROR: $SRC/ffmpeg-$VER.tar.xz is missing — run build/webos-oe/fetch-ffmpeg.sh on the host first" >&2; exit 2; fi
got="$(sha256sum "ffmpeg-$VER.tar.xz" | cut -d' ' -f1)"
[ "$got" = "$SHA" ] || { echo "ERROR: ffmpeg-$VER.tar.xz sha256 $got is not the pinned $SHA" >&2; exit 2; }
rm -rf "ffmpeg-$VER"; tar -xJf "ffmpeg-$VER.tar.xz"; cd "ffmpeg-$VER"
echo "== [ARM] FFmpeg $VER configure (decoders: h264 aac mp3; LGPL)"
./configure \
  --enable-cross-compile --cross-prefix=/tc/bin/arm-webos-linux-gnueabi- --arch=arm \
  --target-os=linux --prefix="$OUT" \
  --enable-shared --disable-static --enable-pic --enable-neon \
  --disable-everything \
  --enable-decoder=h264,aac,aac_latm,mp3,mp3float \
  --enable-parser=h264,aac,aac_latm,mpegaudio \
  --disable-programs --disable-doc --disable-avdevice --disable-avfilter --disable-avformat \
  --disable-swscale --disable-swresample --disable-postproc --disable-network \
  --disable-iconv --disable-zlib --disable-bzlib --disable-lzma --disable-debug \
  --extra-cflags="-march=armv7-a -mfpu=neon -mfloat-abi=softfp -O2" \
  --extra-ldflags="-Wl,-O1,--as-needed" > "$OUT/configure.log" 2>&1 || { tail -30 "$OUT/configure.log"; tail -40 ffbuild/config.log 2>/dev/null; exit 3; }
grep -E 'license|License' "$OUT/configure.log" | head -2
echo "== [ARM] FFmpeg make"
make -j"$(nproc)" > "$OUT/make.log" 2>&1 || { tail -30 "$OUT/make.log"; exit 4; }
make install > "$OUT/install.log" 2>&1
cp COPYING.LGPLv2.1 LICENSE.md "$OUT/"
echo "== built:"; ls -la "$OUT/lib/" | awk '{print $5, $9}' | grep -E 'so\.[0-9]+$|so$'
for f in "$OUT"/lib/libavcodec.so.58 "$OUT"/lib/libavutil.so.56; do
  [ -f "$f" ] || { echo "ERROR: $f missing" >&2; exit 5; }
  /tc/bin/arm-webos-linux-gnueabi-readelf -d "$f" | grep -E 'SONAME|NEEDED' | sed 's/^ *//' | tr '\n' ' '; echo
done
/tc/bin/arm-webos-linux-gnueabi-strip --strip-unneeded "$OUT"/lib/libavcodec.so.58.* "$OUT"/lib/libavutil.so.56.* 2>/dev/null || true
ls -la "$OUT/lib/" | awk '{print $5, $9}' | grep -E 'so\.[0-9]+\.[0-9]'
echo "== decoders in: $(grep -oE 'h264|aac_latm|aac|mp3float|mp3' "$OUT/configure.log" | sort -u | tr '\n' ' ')"
