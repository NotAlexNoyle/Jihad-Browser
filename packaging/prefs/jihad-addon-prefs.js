// Jihad Browser — add-on / plugin prefs. Copyright 2026 NotAlexNoyle. MPL-2.0.
//
// THE single source of these values: appended to goanna.js by BOTH the device bundler
// (build/webos-oe/make-device-bundle.sh) and the desktop build (build/desktop/build-goanna.sh).
// They used to live only in the device bundler, which meant the desktop loop ran with engine
// defaults and could not exercise the install path at all — an InstallTrigger.install() there
// threw NS_ERROR_UNEXPECTED purely because the whitelist prefs below were absent, which reads
// like a broken feature rather than a missing config (measured 2026-08-03).
//
// Every value was chosen against Pale Moon's and Basilisk's app prefs and the engine defaults
// they override; see context/impl/impl-r8-palemoon-basilisk.md for the per-item comparison.
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
