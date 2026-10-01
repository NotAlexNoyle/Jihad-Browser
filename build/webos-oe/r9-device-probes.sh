#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# The two device items left on cavekit-device-build.md R9's "remaining cross-ABI surfaces"
# criterion. Both are headless: no human, no card.
#
#   r9-device-probes.sh build     host only: build the mutex probe with the PDK (gcc 4.3.3),
#                                 DYNAMICALLY linked, so on the device it runs against the device's
#                                 own glibc 2.8 libpthread — not the 2.5 PDK proxy qemu gave us and
#                                 not our bundled 2.23.
#   r9-device-probes.sh mutex     device: push the probe, run it under the device loader, print
#                                 what glibc 2.8 writes into pthread_mutex_t.__kind for a
#                                 PROCESS_SHARED mutex and what it does with a foreign __kind=128.
#   r9-device-probes.sh closure   device + host: pull libflashplayer.so and every library in its
#                                 DT_NEEDED closure off the device, then scan each one's dynamic
#                                 symbols for the named-semaphore and process-shared entry points
#                                 (the surfaces that cross the 2.8 / 2.23 boundary).
#   r9-device-probes.sh all       build, mutex, closure.
#
# ONE DEVICE: run nothing else against it while `mutex` or `closure` is running.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../.." && pwd)
PDK="${PDK_ROOT:-$HERE/pdk/opt/PalmPDK}"
TC="$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin/arm-webos-linux-gnueabi"
PROBE="$HERE/out-arm/mtx_dump_pdk"
PULL="${R9_PULL_DIR:-$HERE/out-arm/r9-closure}"
FLASH="${R9_FLASH:-/media/cryptofs/apps/usr/lib/BrowserServerPlugins/libflashplayer.so}"
# Where the device resolves libraries from, in ld.so order of interest.
SEARCH="/usr/lib /lib /usr/lib/BrowserServerPlugins /media/cryptofs/apps/usr/lib"

build() {
  [ -d "$PDK/arm-gcc" ] || { echo "!! PDK not found at $PDK (set PDK_ROOT)" >&2; exit 2; }
  mkdir -p "$(dirname "$PROBE")"
  "$PDK/arm-gcc/bin/arm-none-linux-gnueabi-gcc" --sysroot="$PDK/arm-gcc/sysroot" \
      -O0 -std=gnu99 -o "$PROBE" "$ROOT/render/goanna/test/mtx_dump.c" -lpthread
  "$TC-readelf" -d "$PROBE" | grep NEEDED
  echo "== built $PROBE ($(md5sum "$PROBE" | cut -c1-32)) =="
}

mutex() {
  [ -f "$PROBE" ] || build
  novacom put file:///tmp/mtx_dump_pdk < "$PROBE"
  novacom run file://bin/chmod -- 755 /tmp/mtx_dump_pdk
  echo "== device glibc: =="
  sleep 3 | novacom run file://bin/ls -- -la /lib/libc.so.6 /lib/libpthread.so.0 || true
  echo "== mtx_dump on the device (device loader, device libpthread): =="
  sleep 4 | novacom run file://tmp/mtx_dump_pdk
}

closure() {
  rm -rf "$PULL"; mkdir -p "$PULL"
  local queue=("$FLASH") seen=" " f base need d found
  while [ ${#queue[@]} -gt 0 ]; do
    f="${queue[0]}"; queue=("${queue[@]:1}")
    base=$(basename "$f")
    case "$seen" in *" $base "*) continue;; esac
    seen="$seen$base "
    novacom get "file://$f" > "$PULL/$base" 2>/dev/null || { echo "  (could not get $f)"; rm -f "$PULL/$base"; continue; }
    [ -s "$PULL/$base" ] || { echo "  (empty: $f)"; rm -f "$PULL/$base"; continue; }
    for need in $("$TC-readelf" -d "$PULL/$base" 2>/dev/null | sed -n 's/.*(NEEDED).*\[\(.*\)\]/\1/p'); do
      case "$seen" in *" $need "*) continue;; esac
      found=""
      for d in $SEARCH; do
        if sleep 1 | novacom run file://bin/ls -- "$d/$need" >/dev/null 2>&1; then found="$d/$need"; break; fi
      done
      if [ -n "$found" ]; then queue+=("$found"); else echo "  UNRESOLVED on device: $need (needed by $base)"; fi
    done
  done
  echo "== closure: $(ls "$PULL" | wc -l) libraries in $PULL =="
  echo "== libraries importing or defining a cross-ABI entry point: =="
  local hit=0
  for f in "$PULL"/*; do
    syms=$("$TC-readelf" --dyn-syms -W "$f" 2>/dev/null \
            | grep -E ' (sem_open|sem_close|sem_unlink|sem_wait|sem_post|pthread_mutexattr_setpshared|pthread_condattr_setpshared|pthread_rwlockattr_setpshared|shm_open)(@|$)' \
            | awk '{print $7 "=" $8}' | sort -u | tr '\n' ' ') || true
    if [ -n "$syms" ]; then echo "  $(basename "$f"): $syms"; hit=1; fi
  done
  [ $hit = 1 ] || echo "  (none)"
  (cd "$PULL" && sha256sum * > SHA256SUMS)
}

case "${1:-}" in
  build) build ;;
  mutex) mutex ;;
  closure) closure ;;
  all) build; mutex; closure ;;
  *) sed -n '3,22p' "$0"; exit 1 ;;
esac
