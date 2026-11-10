#!/bin/bash
# build.sh: omx-probe for the TouchPad with the project's cross toolchain, against UXP's OpenMAX IL 1.1.2 headers.
# Output: tools/device/omx-probe/out/omx-probe (dynamic, glibc 2.23: run it under the daemon's ld-2.23.so).
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd); REPO=$(cd "$HERE/../../.." && pwd)
TC=$REPO/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi-gcc
mkdir -p "$HERE/out"
"$TC" -O2 -Wall -march=armv7-a -mfpu=neon -mfloat-abi=softfp -I"$REPO/third_party/uxp/media/openmax_il/il112" \
  -o "$HERE/out/omx-probe" "$HERE/omx-probe.c" -ldl -lpthread
echo "built: $HERE/out/omx-probe"
