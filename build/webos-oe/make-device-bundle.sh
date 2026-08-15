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

# plugin-container: the OOP NPAPI host, and the ONLY path in UXP that can render a windowless
# plugin. In-process has no rendering path at all — nsPluginFrame::PaintPlugin is an empty stub
# ("we no longer support in-process plugins or synchronous painting") and
# PluginPRLibrary::GetImageContainer returns NS_ERROR_NOT_IMPLEMENTED, so only
# PluginInstanceParent can produce the ImageContainer the plugin layer is built from.
# Seeded into the dependency queue like the daemon so ITS shared libraries get bundled too.
#
# Shipped as the REAL ELF binary under the name XRE looks for. It is NOT exec'able on its own —
# its PT_INTERP is /lib/ld-linux.so.3, webOS's glibc 2.8, while everything we cross-build targets
# the bundled 2.23 — so the daemon exports MOZ_CHILD_PROCESS_LOADER/_PATH and XRE launches it
# through our own loader, exactly the way /etc/event.d/jihad launches the daemon.
# It was briefly a /bin/sh wrapper that re-exec'd the binary through that loader; that CANNOT
# work and cost this port a session. The child inherits LD_LIBRARY_PATH pointing at the bundled
# glibc (the upstart job sets it, and GeckoChildProcessHost sets it again from gGREBinPath), so
# the wrapper's own /bin/sh — a 2.8 binary — resolved OUR 2.23 libc and segfaulted before running
# a single line. Verified on device: `LD_LIBRARY_PATH=$HL /bin/sh -c 'echo hi'` segfaults.
if [ -f "$DIST/bin/plugin-container" ]; then
  cp -L "$DIST/bin/plugin-container" "$OUT/plugin-container"
  queue+=("$OUT/plugin-container")
else
  echo "!! no plugin-container in $DIST/bin — NPAPI plugins cannot render out-of-process" >&2
  exit 31
fi

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

# --- about:preferences / about:settings ---------------------------------------------------------
# Pale Moon organises settings as a prefwindow of panes; Basilisk serves the same material
# in-content. This embedding has no chrome window to host a prefwindow, so it takes Basilisk's
# shape with Pale Moon's panes: an nsIAboutModule (a JS component, exactly like the XPI install
# prompt above) resolving both about: names to chrome://jihad-prefs/content/preferences.html.
# chrome://, not resource://, because the page writes prefs and therefore needs the system
# principal; content cannot reach it because the module does not set URI_SAFE_FOR_UNTRUSTED_CONTENT.
PREFSUI="$REPO/packaging/prefsui"
ABOUT_JS="$REPO/render/goanna/components/jihadAboutPreferences.js"
if [ -d "$PREFSUI" ] && [ -f "$ABOUT_JS" ]; then
	rm -rf "$OUT/prefsui"
	mkdir -p "$OUT/prefsui" "$OUT/components"
	cp -R "$PREFSUI/content" "$OUT/prefsui/"
	cp -L "$PREFSUI/jihad-prefsui.manifest" "$OUT/"
	cp -L "$ABOUT_JS" "$OUT/components/jihadAboutPreferences.js"
	cat > "$OUT/components/jihad-about-preferences.manifest" <<'MANIFEST'
# Jihad Browser — about:preferences / about:settings. Copyright 2026 NotAlexNoyle. MPL-2.0.
# Both names resolve to the same document (chrome://jihad-prefs/content/preferences.html):
# about:preferences is what Basilisk users type, about:settings is the other name asked for.
component {0f2b8f4e-6c31-4a7b-9f2d-5c1e73a4b806} jihadAboutPreferences.js
contract @mozilla.org/network/protocol/about;1?what=preferences {0f2b8f4e-6c31-4a7b-9f2d-5c1e73a4b806}
contract @mozilla.org/network/protocol/about;1?what=settings {0f2b8f4e-6c31-4a7b-9f2d-5c1e73a4b806}
MANIFEST
	# Idempotent, same reason as the branding line above.
	if ! grep -q '^manifest jihad-prefsui.manifest$' "$OUT/chrome.manifest" 2>/dev/null; then
		echo 'manifest jihad-prefsui.manifest' >> "$OUT/chrome.manifest"
	fi
	if ! grep -q '^manifest components/jihad-about-preferences.manifest$' "$OUT/chrome.manifest" 2>/dev/null; then
		echo 'manifest components/jihad-about-preferences.manifest' >> "$OUT/chrome.manifest"
	fi
	# Fail the BUILD, not the device: a half-landed package means about:preferences resolves to
	# nothing, and "unknown protocol" points nowhere near this block.
	for f in jihad-prefsui.manifest prefsui/content/preferences.html \
	         prefsui/content/preferences.js prefsui/content/preferences.css \
	         components/jihadAboutPreferences.js components/jihad-about-preferences.manifest; do
		[ -s "$OUT/$f" ] || { echo "ERROR: prefs-ui file missing/empty in bundle: $f" >&2; exit 1; }
	done
	echo "  prefs ui: about:preferences + about:settings registered (chrome://jihad-prefs/)"
else
	# FAIL LOUD, do not skip. A guard that quietly passes when its inputs are missing is how
	# this build drifts: the bundle would ship with no about:preferences and say nothing.
	echo "ERROR: prefs-ui inputs missing ($PREFSUI / $ABOUT_JS)" >&2
	exit 1
fi

# --- XPI web-install confirm (browser-services R3 / addons R3) -----------------------------------
# The toolkit's own install confirm is a modal XUL chrome window (amWebInstallListener.js) that a
# headless daemon cannot open — and its failure path CANCELS the install, so without this every web
# install is silently refused. amWebInstallListener looks for an application-provided
# "@mozilla.org/addons/web-install-prompt;1" FIRST, so ours takes that hook and routes the decision
# to the CARD over the existing dialog contract (DialogService's jihad-xpi-confirm observer).
#
# The C++ observer has shipped since 2026-08-03; this is the JS half plus the manifest line that
# actually registers it. It was deliberately left unregistered until a card dialog could answer,
# because a blocking confirm with no reply path would hang the daemon — that reply path is proven
# now (the <select> popup round-trip and the restored card dev loop use it).
PROMPT_JS="$REPO/render/goanna/components/jihadInstallPrompt.js"
if [ -f "$PROMPT_JS" ]; then
	mkdir -p "$OUT/components"
	cp -L "$PROMPT_JS" "$OUT/components/jihadInstallPrompt.js"
	# Registered from the GRE root manifest, next to the engine's own component manifests.
	cat > "$OUT/components/jihad-install-prompt.manifest" <<'MANIFEST'
# Jihad Browser — XPI web-install confirm. Copyright 2026 NotAlexNoyle. MPL-2.0.
# Claims the hook amWebInstallListener checks before falling back to its own modal XUL
# window, which cannot open in this headless embedding.
component {5cf8e2a6-91a1-44f5-9d33-8ab6f2c40d17} jihadInstallPrompt.js
contract @mozilla.org/addons/web-install-prompt;1 {5cf8e2a6-91a1-44f5-9d33-8ab6f2c40d17}
MANIFEST
	if ! grep -q '^manifest components/jihad-install-prompt.manifest$' "$OUT/chrome.manifest" 2>/dev/null; then
		echo 'manifest components/jihad-install-prompt.manifest' >> "$OUT/chrome.manifest"
	fi
	for f in components/jihadInstallPrompt.js components/jihad-install-prompt.manifest; do
		[ -s "$OUT/$f" ] || { echo "ERROR: xpi-prompt file missing/empty in bundle: $f" >&2; exit 1; }
	done
	echo "  xpi prompt: @mozilla.org/addons/web-install-prompt;1 registered (card-side confirm)"
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
grep -q 'JIHAD add-on prefs' "$OUT/goanna.js" 2>/dev/null || cat "$REPO/packaging/prefs/jihad-addon-prefs.js" >> "$OUT/goanna.js"
# Platform prefs (what this device IS — touch, chiefly). Same shared-file rule as the add-on
# prefs above: one source, appended by both builds, so desktop and device cannot drift.
grep -q 'JIHAD platform prefs' "$OUT/goanna.js" 2>/dev/null || cat "$REPO/packaging/prefs/jihad-platform-prefs.js" >> "$OUT/goanna.js"

# WHAT IS LEFT IN THIS HEREDOC IS DEVICE-ONLY ON PURPOSE, and the test is one line long: nothing
# below backs a row in packaging/prefsui/content/preferences.js. These are 512 MB / ARM numbers
# (decoded-image surfacecache, JS GC water marks, media cache, socket counts), and appending them
# to the DESKTOP harness would be wrong on its own terms - a harness that GCs at a 32 MB heap and
# caps its surface cache at 32 MB is not standing in for the device, it is simulating a memory
# ceiling the host does not have, and it would hide the eviction behaviour this tuning exists to
# avoid on device.
#
# The ELEVEN prefs that DO back about:preferences rows moved to
# packaging/prefs/jihad-platform-prefs.js on 2026-08-10 (T-111, cavekit-gre-widgets.md R7), which
# BOTH builds append. Do NOT re-add one of them here: goanna.js is last-wins and this heredoc is
# appended AFTER that file, so a duplicate here silently un-shares the value and restores exactly
# the desktop/device drift R7 exists to prevent.
grep -q 'JIHAD low-RAM tuning' "$OUT/goanna.js" 2>/dev/null || cat >> "$OUT/goanna.js" <<'JIHADPREFS'
// --- JIHAD low-RAM tuning (512 MB floor) ---
// Device-only by design: nothing in this block backs an about:preferences row. The ones that do
// live in packaging/prefs/jihad-platform-prefs.js, appended by BOTH builds (gre-widgets R7).
pref("image.mem.surfacecache.max_size_kb", 32768);        // 32 MB decoded-image cache (was 1 GB)
pref("image.mem.surfacecache.size_factor", 8);            // cap at RAM/8, not RAM/4
pref("image.mem.surfacecache.discard_factor", 1);         // drop the whole cache on memory pressure
pref("image.mem.discardable", true);
pref("image.mem.animated.discardable", true);
pref("browser.sessionhistory.max_total_viewers", 0);      // no bfcache page viewers in RAM
pref("browser.cache.memory.enable", true);
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
// the single-core-class device; still plenty for one page. The other three that used to be
// here - network.http.max-connections, network.prefetch-next, network.dns.disablePrefetch -
// back about:preferences rows and are now in packaging/prefs/jihad-platform-prefs.js. This one
// backs no row, so it stays device-only.
pref("network.http.max-persistent-connections-per-server", 4);
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
// browser.cache.disk.{enable,capacity,smart_size.enabled} all back about:preferences rows, so
// the VALUES moved to packaging/prefs/jihad-platform-prefs.js (T-111). The reasoning above is
// kept here because it is device reasoning - where the cache lives and why 50 MB - and the
// shared file cites it rather than repeating it.
JIHADPREFS

# strip everything to shrink for the device
for so in "$OUT"/*.so "$OUT"/*.so.* "$OUT/jihad-browserserver" "$OUT/libxul.so" \
          "$OUT/plugin-container"; do
  [ -f "$so" ] && "$STRIP" "$so" 2>/dev/null || true
done

# review #9: required-file manifest — a bundle missing anything essential is a hard failure, not a
# silent success. Covers the daemon, libxul, the bundled glibc loader/core, the NSS TLS modules
# (dlopen'd, not NEEDED), and the GRE resources the engine loads from greDir.
# (omni.ja is NOT required — this xulrunner embedding build ships loose resources + goanna.js,
# not a packed omni.ja; the direct build treats it as optional too.)
REQUIRED=(jihad-browserserver libxul.so ld-2.23.so libc.so.6 libstdc++.so.6 libpthread.so.0 \
          libnss3.so libnssutil3.so libsoftokn3.so libfreebl3.so libnss_dns.so.2 libnss_files.so.2 \
          libmozsqlite3.so goanna.js plugin-container)
_missing=""
for r in "${REQUIRED[@]}"; do [ -e "$OUT/$r" ] || _missing="$_missing $r"; done
[ -z "$_missing" ] || { echo "!! device bundle is missing required files:$_missing" >&2; exit 30; }

echo "=== bundle assembled: $OUT ==="
echo "files: $(ls "$OUT" | wc -l), size: $(du -sh "$OUT" | cut -f1)"
echo "libxul: $(du -h "$OUT/libxul.so" | cut -f1)"
