#!/bin/bash
# Copyright 2026 the Jihad Browser project. Apache-2.0.
#
# Assemble the on-device bundle for the TouchPad: the transitive .so closure of
# jihad-browserserver-arm + libxul, plus the GRE files (goanna.js, omni.ja, ...),
# all as REAL files named by soname (VFAT on /media/internal has no symlinks),
# launched via the bundled glibc-2.23 loader. Runs on the host (readelf reads ARM).
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
DIST="$HERE/out-arm/obj-jihad-goanna-arm/dist"
SYS="$HERE/arm-sysroot/root"
TCS="$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/arm-webos-linux-gnueabi/sysroot"
TCLIB="$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/arm-webos-linux-gnueabi/lib"
DAEMON="$HERE/out-arm/jihad-browserserver-arm"
OUT="$HERE/device-bundle"
STRIP="$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi-strip"

rm -rf "$OUT"; mkdir -p "$OUT"
SEARCH=("$DIST/bin" "$SYS/usr/lib/arm-linux-gnueabi" "$SYS/lib/arm-linux-gnueabi" "$TCS/lib" "$TCS/usr/lib" "$TCLIB")

find_lib(){ local s="$1" d; for d in "${SEARCH[@]}"; do [ -e "$d/$s" ] && { readlink -f "$d/$s"; return; }; done; }

declare -A done
queue=()
add(){ local real="$1" name="$2"; [ -n "${done[$name]:-}" ] && return; done[$name]=1
       cp -L "$real" "$OUT/$name"; queue+=("$OUT/$name"); }

# seed: daemon + libxul
cp -L "$DAEMON" "$OUT/jihad-browserserver"; queue+=("$OUT/jihad-browserserver")
add "$(readlink -f "$DIST/bin/libxul.so")" "libxul.so"

i=0
while [ $i -lt ${#queue[@]} ]; do
  f="${queue[$i]}"; i=$((i+1))
  for need in $(readelf -d "$f" 2>/dev/null | awk '/NEEDED/{gsub(/[][]/,"",$5); print $5}'); do
    [ -n "${done[$need]:-}" ] && continue
    real=$(find_lib "$need")
    if [ -n "$real" ]; then add "$real" "$need"
    else echo "  (unresolved, expected on-device or bundled-glibc: $need)"; fi
  done
done

# glibc loader (invoked explicitly) + ensure core glibc sonames present
cp -L "$(find_lib ld-2.23.so 2>/dev/null || echo "$TCS/lib/ld-2.23.so")" "$OUT/ld-2.23.so" 2>/dev/null || \
  cp -L "$TCS/lib/ld-linux.so.3" "$OUT/ld-2.23.so"
for s in libc.so.6 libm.so.6 libpthread.so.0 libdl.so.2 librt.so.1 libgcc_s.so.1 libstdc++.so.6; do
  [ -n "${done[$s]:-}" ] && continue; r=$(find_lib "$s"); [ -n "$r" ] && cp -L "$r" "$OUT/$s" && done[$s]=1
done

# glibc name-service (NSS) + resolver modules — getaddrinfo dlopen's these at runtime
# (they are NOT in any NEEDED list), and they must match the bundled glibc 2.23, not the
# device's 2.8. Without them DNS fails and http:// URLs never navigate (about:blank).
for s in libnss_dns.so.2 libnss_files.so.2 libresolv.so.2; do
  cp -L "$TCS/lib/$s" "$OUT/$s" 2>/dev/null && echo "  NSS: $s"
done
# NSS runtime-dlopen'd modules (NOT in any NEEDED list): the CA roots (libnssckbi)
# and the crypto tokens (libsoftokn3 + libfreebl3, loaded by NSS_Init). Without softokn3/
# freebl3, NSS_NoDB_Init fails (-1) and https can't create a TLS socket
# (NS_ERROR_UNKNOWN_SOCKET_TYPE). NOTE: the run script MUST set LD_LIBRARY_PATH=<bundle>
# so NSPR's PR_LoadLibrary finds these (the ld-2.23.so --library-path does not cover it).
for s in libnssckbi.so libsoftokn3.so libfreebl3.so libfreeblpriv3.so; do
  [ -e "$DIST/bin/$s" ] && cp "$DIST/bin/$s" "$OUT/$s" && echo "  NSS module: $s"
done

# GRE resource files the engine loads from greDir (= the bundle dir)
for f in goanna.js omni.ja dependentlibs.list platform.ini icudt78l.dat; do
  [ -e "$DIST/bin/$f" ] && cp "$DIST/bin/$f" "$OUT/"
done
# Ensure the OMTC-off pref (headless CPU paint) is present in goanna.js
grep -q 'offmainthreadcomposition.force-disabled' "$OUT/goanna.js" 2>/dev/null || \
  echo 'pref("layers.offmainthreadcomposition.force-disabled", true);' >> "$OUT/goanna.js"

# strip everything to shrink for the device
for so in "$OUT"/*.so "$OUT"/*.so.* "$OUT/jihad-browserserver" "$OUT/libxul.so"; do
  [ -f "$so" ] && "$STRIP" "$so" 2>/dev/null || true
done

echo "=== bundle assembled: $OUT ==="
echo "files: $(ls "$OUT" | wc -l), size: $(du -sh "$OUT" | cut -f1)"
echo "libxul: $(du -h "$OUT/libxul.so" | cut -f1)"
