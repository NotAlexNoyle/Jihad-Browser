#!/bin/bash
# Copyright 2026 the Jihad Browser project. Apache-2.0.
#
# Assemble the on-device bundle for the TouchPad: the transitive .so closure of
# jihad-browserserver-arm + libxul, plus the GRE files (goanna.js, omni.ja, ...),
# all as REAL files named by soname (VFAT on /media/internal has no symlinks),
# launched via the bundled glibc-2.23 loader. Runs on the host (readelf reads ARM).
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
# All inputs are env-overridable so the OE product recipe can point them at bitbake-built
# artifacts (jihad-deviceroot recipe). Defaults = the direct cross-build layout.
TC="${JIHAD_TC:-$HERE/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi}"
DIST="${DIST:-$HERE/out-arm/obj-jihad-goanna-arm/dist}"
SYS="${SYS:-$HERE/arm-sysroot/root}"
TCS="${TCS:-$TC/arm-webos-linux-gnueabi/sysroot}"
TCLIB="${TCLIB:-$TC/arm-webos-linux-gnueabi/lib}"
DAEMON="${DAEMON:-$HERE/out-arm/jihad-browserserver-arm}"
OUT="${OUT:-$HERE/device-bundle}"
STRIP="${STRIP:-$TC/bin/arm-webos-linux-gnueabi-strip}"

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
# Low-RAM (512 MB target: TouchPad Go, Palm Pre 3) memory + repaint tuning. These MUST be in
# goanna.js (loaded before gfxPlatform snapshots the "Once" gfx prefs) — a runtime SetIntPref in
# EngineHost::Init is too late (Codex F-235). Appended prefs override the stock defaults (last-wins).
# The stock surfacecache cap is 1 GB, catastrophic on 512 MB: the decoded-image surface memory grows
# until the OS evicts the live render to near-blank (the "renders full then goes blank" degradation).
grep -q 'JIHAD low-RAM tuning' "$OUT/goanna.js" 2>/dev/null || cat >> "$OUT/goanna.js" <<'JIHADPREFS'
// --- JIHAD low-RAM tuning (512 MB floor) ---
pref("image.mem.surfacecache.max_size_kb", 32768);        // 32 MB decoded-image cache (was 1 GB)
pref("image.mem.surfacecache.size_factor", 8);            // cap at RAM/8, not RAM/4
pref("image.mem.surfacecache.discard_factor", 1);         // drop the whole cache on memory pressure
pref("image.mem.discardable", true);
pref("image.mem.animated.discardable", true);
pref("browser.sessionhistory.max_total_viewers", 0);      // no bfcache page viewers in RAM
pref("browser.sessionhistory.max_entries", 20);           // bound history depth
pref("browser.cache.memory.enable", true);
pref("browser.cache.memory.capacity", 16384);             // 16 MB in-RAM HTTP cache
pref("layout.frame_rate", 30);                            // cap the refresh driver at 30 Hz
pref("general.smoothScroll", false);                      // no smooth-scroll compositing
pref("image.animation_mode", "once");                     // play animated GIFs once, not forever
pref("nglayout.initialpaint.delay", 100);                 // paint sooner on slow pages
// JS heap tuning for a small-RAM/slow-CPU device, following the low-spec Goanna
// forks: Arctic Fox ships high_water_mark=128 + slice=10 for old Macs; Mypal68
// uses slice=5 for old XP boxes. 512 MB floor wants an even lower water mark.
pref("javascript.options.mem.high_water_mark", 32);       // GC when a zone's malloc'd heap hits 32 MB
pref("javascript.options.mem.gc_incremental_slice_ms", 5); // short GC slices keep input responsive
pref("javascript.options.mem.gc_low_frequency_heap_growth", 120); // grow the heap slowly (default 150%)
pref("javascript.options.mem.gc_high_frequency_high_limit_mb", 40); // treat >40 MB heaps as "large"
// Media: never buffer half a video in RAM on a 512 MB device.
pref("media.cache_size", 32768);                          // 32 MB media cache (kB; default 500 MB)
pref("media.memory_cache_max_size", 4096);                // 4 MB in-memory media cache (kB)
// Network: fewer parallel sockets = less buffer memory + less CPU contention on
// the single-core-class device; still plenty for one page.
pref("network.http.max-connections", 32);
pref("network.http.max-persistent-connections-per-server", 4);
pref("network.prefetch-next", false);                     // no speculative page prefetch
pref("network.dns.disablePrefetch", true);
// Disk offload: persistent HTTP cache on the (large) internal media partition
// keeps repeat loads off the network AND lets the RAM cache stay small. Cookies
// and the rest of the profile live in the daemon profile dir (see EngineHost).
// VFAT CAUTION (inspector P2): /media/internal is VFAT — no journaling. The
// cache2 disk cache is DISPOSABLE by design (entries are checksummed; corruption
// = cache miss + auto-rebuild), so it is safe there. cookies.sqlite is NOT
// disposable: force mozStorage to synchronous=FULL so the SQLite rollback
// journal is fsync-ordered and a hard power loss mid-write rolls back cleanly
// instead of corrupting the whole cookie DB (an OOM SIGKILL alone doesn't tear
// writes — the kernel completes queued I/O — but battery pulls happen).
pref("toolkit.storage.synchronous", 2);                   // FULL fsync for sqlite (cookies on VFAT)
pref("browser.cache.disk.enable", true);
pref("browser.cache.disk.capacity", 65536);               // 64 MB on /media/internal
pref("browser.cache.disk.smart_size.enabled", false);     // never autosize into GBs
JIHADPREFS

# strip everything to shrink for the device
for so in "$OUT"/*.so "$OUT"/*.so.* "$OUT/jihad-browserserver" "$OUT/libxul.so"; do
  [ -f "$so" ] && "$STRIP" "$so" 2>/dev/null || true
done

echo "=== bundle assembled: $OUT ==="
echo "files: $(ls "$OUT" | wc -l), size: $(du -sh "$OUT" | cut -f1)"
echo "libxul: $(du -h "$OUT/libxul.so" | cut -f1)"
