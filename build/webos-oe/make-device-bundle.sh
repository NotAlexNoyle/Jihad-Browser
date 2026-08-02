#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Assemble the on-device bundle for the TouchPad: the transitive .so closure of
# jihad-browserserver-arm + libxul, plus the GRE files (goanna.js, omni.ja, ...),
# all as REAL files named by soname (VFAT on /media/internal has no symlinks),
# launched via the bundled glibc-2.23 loader. Runs on the host (readelf reads ARM).
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
REPO="${JIHAD_REPO:-$(cd "$HERE/../.." && pwd)}"   # repo root: source of the in-tree ICU data
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

# GRE resource files the engine loads from greDir (= the bundle dir).
#
# `cp -L` and the REQUIRED check are both load-bearing (found on-device 2026-07-31, the hard way):
# in the ARM dist, `bin/icudt78l.dat` is a SYMLINK to /src/uxp/config/external/icu/data/… — a path
# that exists only INSIDE the build container. Assembled on the host, `[ -e ]` follows that link,
# finds nothing, and the old code SILENTLY SKIPPED the file. The resulting bundle looked complete
# (39 files, no error) and the daemon then aborted on every start with
#   ###!!! ABORT: u_init() failed: file /src/uxp/xpcom/build/XPCOMInit.cpp, line 698
# i.e. ICU could not initialize, which upstart dutifully respawned forever. A missing required
# GRE file must fail the BUILD, not the device.
REQUIRED_GRE="goanna.js dependentlibs.list platform.ini icudt78l.dat"
for f in $REQUIRED_GRE; do
  src="$DIST/bin/$f"
  # Fall back to the in-tree source for a dist entry that is a container-only dangling symlink.
  if [ ! -e "$src" ] && [ "$f" = "icudt78l.dat" ]; then
    src="$REPO/third_party/uxp/config/external/icu/data/$f"
  fi
  [ -e "$src" ] || { echo "ERROR: required GRE file missing: $f (looked in $DIST/bin)" >&2; exit 1; }
  cp -L "$src" "$OUT/$f" || { echo "ERROR: failed to copy $f" >&2; exit 1; }
done
# omni.ja is OPTIONAL: it exists only in a packaged (jar'd) build. This dist ships the GRE
# resources as loose directories instead, copied below.
[ -e "$DIST/bin/omni.ja" ] && cp -L "$DIST/bin/omni.ja" "$OUT/"

# Loose GRE resource directories. When the engine is not packaged into omni.ja it loads chrome
# manifests, XPCOM component manifests, default prefs and JS modules from these directories under
# greDir; without them XPCOM registration fails.
#
# These cannot simply be `cp -rL`'d on the host. A mach build tree links its resources straight
# back at the SOURCE tree, so most entries here are symlinks to /src/uxp/… — the path where the
# build CONTAINER mounts third_party/uxp. On the host every one of them dangles, `cp -rL` fails
# mid-copy, and a naive `|| cp -r` fallback both leaves the dangling links in place and (because
# the destination already half-exists) nests the tree as res/res/…. palm-package then dies trying
# to read them (FilteredFileContents.readContents → NPE in the regex matcher).
#
# So: copy the structure preserving links, then rewrite each dangling /src/uxp/… link into the
# real in-tree file. The container mount is exactly $REPO/third_party/uxp, so the mapping is a
# prefix substitution. Anything still dangling afterwards is dropped rather than shipped —
# palm-package cannot read it and the engine would not find it either.
UXP_SRC="$REPO/third_party/uxp"
for d in chrome components defaults dictionaries hyphenation modules res; do
  [ -d "$DIST/bin/$d" ] || continue
  rm -rf "${OUT:?}/$d"
  cp -a "$DIST/bin/$d" "$OUT/$d"
  # Resolve container-only links against the in-tree source.
  while IFS= read -r link; do
    [ -n "$link" ] || continue
    tgt=$(readlink "$link")
    case "$tgt" in
      /src/uxp/*) real="$UXP_SRC/${tgt#/src/uxp/}" ;;
      *)          real="" ;;
    esac
    if [ -n "$real" ] && [ -e "$real" ]; then
      rm -f "$link"; cp -L "$real" "$link"
    else
      rm -f "$link"          # unresolvable: ship nothing rather than a broken entry
    fi
  done <<EOF
$(find "$OUT/$d" -xtype l 2>/dev/null)
EOF
  # Build bookkeeping that must never reach a package.
  find "$OUT/$d" -name '.mkdir.done' -delete 2>/dev/null || true
  find "$OUT/$d" -type d -empty -delete 2>/dev/null || true
done
[ -e "$DIST/bin/chrome.manifest" ] && cp -L "$DIST/bin/chrome.manifest" "$OUT/"

# --- our own chrome://branding/ package ---------------------------------------------------------
# toolkit's about:addons opens with <!ENTITY % brandDTD SYSTEM "chrome://branding/locale/brand.dtd">.
# In Firefox/Pale Moon that package comes from the APPLICATION (browser/branding); this build embeds
# the GRE with no application above it, and the branding strip removed the vendor's, so the DTD
# resolved to nothing — a hard XML parse error that killed about:addons before it rendered
# ("No chrome package registered for chrome://branding/locale/brand.dtd"). The GRE's own
# chrome/en-US/.../global/brand.dtd is EMPTY and registered under chrome://global/, so it does not
# satisfy this. Ship ours instead — our names, none of the upstream vendor's trademarked assets.
BRANDING="$REPO/packaging/branding"
if [ -d "$BRANDING" ]; then
	rm -rf "$OUT/branding"
	mkdir -p "$OUT/branding"
	cp -R "$BRANDING/content" "$BRANDING/locale" "$OUT/branding/"
	cp -L "$BRANDING/jihad-branding.manifest" "$OUT/"
	# Idempotent: re-running the bundler must not append the line twice (a duplicate `manifest`
	# directive re-registers the package and the engine warns on every start).
	if ! grep -q '^manifest jihad-branding.manifest$' "$OUT/chrome.manifest" 2>/dev/null; then
		echo 'manifest jihad-branding.manifest' >> "$OUT/chrome.manifest"
	fi
	# Fail the BUILD, not the device: a branding package that silently did not land means
	# about:addons breaks again, and the symptom (an XML parse error) points nowhere near here.
	for f in jihad-branding.manifest branding/locale/en-US/brand.dtd \
	         branding/locale/en-US/brand.properties branding/content/icon32.png; do
		[ -s "$OUT/$f" ] || { echo "ERROR: branding file missing/empty in bundle: $f" >&2; exit 1; }
	done
	echo "  branding: chrome://branding/ registered (about:addons brand.dtd)"
fi
# Ensure the OMTC-off pref (headless CPU paint) is present in goanna.js
grep -q 'offmainthreadcomposition.force-disabled' "$OUT/goanna.js" 2>/dev/null || \
  echo 'pref("layers.offmainthreadcomposition.force-disabled", true);' >> "$OUT/goanna.js"
# Low-RAM (512 MB target: TouchPad Go, Palm Pre 3) memory + repaint tuning. These MUST be in
# goanna.js (loaded before gfxPlatform snapshots the "Once" gfx prefs) — a runtime SetIntPref in
# EngineHost::Init is too late (Codex F-235). Appended prefs override the stock defaults (last-wins).
# The stock surfacecache cap is 1 GB, catastrophic on 512 MB: the decoded-image surface memory grows
# until the OS evicts the live render to near-blank (the "renders full then goes blank" degradation).
# Add-on / plugin prefs (cavekit-addons-extensions R3/R5/R6/R7). Every value below was chosen
# against Pale Moon's and Basilisk's app prefs and the engine defaults they override; see
# context/impl/impl-r8-palemoon-basilisk.md for the per-item comparison and citations.
grep -q 'JIHAD add-on prefs' "$OUT/goanna.js" 2>/dev/null || cat >> "$OUT/goanna.js" <<'JIHADADDONS'
// --- JIHAD add-on prefs ---
// The GRE's own code default is gStrictCompatibility=true (AddonManager.jsm:610). Both Pale Moon
// and Basilisk turn it OFF in app prefs — a default we never inherit, because an embedding has no
// application prefs file. Left on, an extension must satisfy min AND max against the app version,
// so anything with a maxVersion below ours is rejected even after the AppCompat GUID work below.
// Off, only minVersion is enforced for default-compatible types (XPIProvider.jsm:6382-6410).
pref("extensions.strictCompatibility", false);
// Deliberately NOT setting extensions.minCompatibleAppVersion: unset skips the check entirely,
// which is what we want. The forks set "1.5"/"4.0" in the Firefox version namespace, which is
// meaningless for ours.

// SCOPE_PROFILE|SCOPE_APPLICATION. The engine default is SCOPE_ALL (XPIProvider.jsm:1987-1988),
// which enables SCOPE_USER -> "XREUSysExt" -> nsXREDirProvider::GetSysUserExtensionsDirectory,
// which EnsureDirectoryExists on $HOME/.mozilla/extensions. Our $HOME is the daemon state dir on
// /var, so SCOPE_ALL mkdir'd into the 62 MB /var partition on every single startup (violating the
// R6 storage contract) AND made a $HOME-shared directory a cross-variant leak channel (violating
// R5 isolation). This is a DELIBERATE divergence from both forks, which accept all scopes because
// on a desktop OS those locations are a feature; three independent variants on one device is a
// webOS packaging requirement neither of them has.
pref("extensions.enabledScopes", 5);
// Anything discovered in a non-profile scope starts disabled, as both forks do.
pref("extensions.autoDisableScopes", 15);

// Install gating. xpinstall.whitelist.required defaults TRUE (XPIProvider.jsm:3655) and a blocked
// install surfaces ONLY as an observer notification that a browser front-end would turn into a
// doorhanger — we have no such front-end, so leaving it on makes a blocked install silently do
// nothing. Pale Moon ships exactly this set.
pref("xpinstall.whitelist.required", false);
pref("xpinstall.whitelist.directRequest", true);
pref("xpinstall.whitelist.fileRequest", true);
// CONSCIOUS CHOICE, matching Pale Moon: permits installs from http origins. This device's browsing
// is largely legacy http, and requiring a secure origin would block most real sources; the risk is
// accepted and recorded here rather than left implicit.
pref("extensions.install.requireSecureOrigin", false);

// No AMO, no update server, no blocklist server — we operate none, and the GRE ships no defaults
// for these URLs (they are app-supplied in both forks). Left enabled, the daily background update
// timer and the version-change blocklist validation burn cycles on a 512 MB device and throw
// per-addon against URLs that do not exist. Out of scope per the kit.
pref("extensions.update.enabled", false);
pref("extensions.blocklist.enabled", false);
pref("extensions.getAddons.cache.enabled", false);

// Dual-GUID compatibility (Pale Moon's UXP_APPCOMPAT_GUID feature). Our app ID is frozen and named
// by zero existing extensions, so without this every real XPI arrives appDisabled and the web
// install path treats appDisabled as a failed install (amWebInstallListener.js:112-115) — i.e. R3
// is unreachable with any extension not authored for Jihad specifically.
//
// ORDERING IS LOAD-BEARING: XPIProvider.jsm reads these with a one-argument getCharPref at module
// top level, so with the UXP_APPCOMPAT_GUID build define present and these prefs ABSENT the import
// throws and takes the whole add-ons manager with it. Shipping the prefs FIRST is harmless (they
// are simply unread until the define lands) and makes that failure mode unreachable. The build
// define is a separate change to third_party/uxp/xulrunner/configure.in.
pref("extensions.guid.appCompatId", "{ec8030f7-c20a-464f-9b0e-13a3a9e97384}");
pref("extensions.guid.appCompatVersion", "56.9");

// NPAPI: this daemon is single-process and plugin-container is not in the bundle, so an
// out-of-process plugin load would stall trying to spawn it. RunPluginOOP defaults TRUE on a
// non-GTK build (nsNPAPIPlugin.cpp:229-303). Whether OOP is viable on this device at all is an
// open R7 question, not a settled one.
pref("dom.ipc.plugins.enabled", false);
JIHADADDONS

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
// Disk offload: a persistent HTTP cache keeps repeat loads off the network AND
// lets the RAM cache stay small. WHERE it lives is decided in
// render/goanna/JihadRuntimePaths.h, not here — Gecko's own ProfD/ProfLD split,
// with BOTH halves in the app's own directory on cryptofs:
//   ProfD  (durable: cookies.sqlite, prefs.js, permissions) -> $APP/profile
//   ProfLD (disposable: cache2, startupCache)               -> $APP/cache
//
// That is where both prior implementations of this browser on this device put
// them. isis — our own upstream, still in the fork at Src/Settings.cpp:65-74 —
// ships CachePath=/media/cryptofs/.browser/cache and
// CookieJarPath=/media/cryptofs/.browser/cookies. Atlas routes netdata, netcache
// and cookies.db to its app deviceroot on cryptofs, and says why: /media/internal
// is VFAT, with no hard links and no real file locking, which breaks both the
// network cache and SQLite-backed storage. (That is also why OUR cookies.sqlite
// was never created on-device on 2026-07-20.) /var is not used for either half:
// 49.6 MB free, shared with system state.
//
// THE CAP IS NOT OPTIONAL, and it is why smart-sizing stays off: cache2 sizes
// itself from FREE SPACE when smart_size is enabled, so on a 10.2 GB partition it
// would claim GBs — the room existing is not a reason to use it. 50 MB is not a
// number invented here: it is isis's own CacheMaxSize ("50M"), chosen by the
// people who shipped this browser on this hardware, and it is ~3x the 16 MB RAM
// cache in front of it.
//
// CAUTION, cryptofs: it is FUSE, so cache2's many-small-file I/O is slower there
// than ext3 would be. Both predecessors accepted that trade, and the alternative
// is a cache that fills a 62 MB system partition and takes the rest of webOS with
// it. The memory cache absorbs the hot set either way.
//
// cookies.sqlite is NOT disposable: force mozStorage to synchronous=FULL so the
// SQLite rollback journal is fsync-ordered and a hard power loss mid-write rolls
// back cleanly instead of corrupting the whole cookie DB (an OOM SIGKILL alone
// doesn't tear writes — the kernel completes queued I/O — but battery pulls
// happen).
pref("toolkit.storage.synchronous", 2);                   // FULL fsync for the cookie DB
pref("browser.cache.disk.enable", true);
pref("browser.cache.disk.capacity", 51200);               // 50 MB — isis's own CacheMaxSize
pref("browser.cache.disk.smart_size.enabled", false);     // never autosize from free space
JIHADPREFS

# strip everything to shrink for the device
for so in "$OUT"/*.so "$OUT"/*.so.* "$OUT/jihad-browserserver" "$OUT/libxul.so"; do
  [ -f "$so" ] && "$STRIP" "$so" 2>/dev/null || true
done

# review #9: required-file manifest — a bundle missing anything essential is a hard failure, not a
# silent success. Covers the daemon, libxul, the bundled glibc loader/core, the NSS TLS modules
# (dlopen'd, not NEEDED), and the GRE resources the engine loads from greDir.
# (omni.ja is NOT required — this xulrunner embedding build ships loose resources + goanna.js,
# not a packed omni.ja; the direct build treats it as optional too.)
REQUIRED=(jihad-browserserver libxul.so ld-2.23.so libc.so.6 libstdc++.so.6 libpthread.so.0 \
          libnss3.so libnssutil3.so libsoftokn3.so libfreebl3.so libnss_dns.so.2 libnss_files.so.2 \
          libmozsqlite3.so goanna.js)
_missing=""
for r in "${REQUIRED[@]}"; do [ -e "$OUT/$r" ] || _missing="$_missing $r"; done
[ -z "$_missing" ] || { echo "!! device bundle is missing required files:$_missing" >&2; exit 30; }

echo "=== bundle assembled: $OUT ==="
echo "files: $(ls "$OUT" | wc -l), size: $(du -sh "$OUT" | cut -f1)"
echo "libxul: $(du -h "$OUT/libxul.so" | cut -f1)"
