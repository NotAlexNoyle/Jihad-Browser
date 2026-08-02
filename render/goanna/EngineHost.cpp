/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — Goanna embedding runtime host (T-013). See EngineHost.h.
 */
#include "EngineHost.h"

#include "nsXULAppAPI.h"             // XRE_InitEmbedding2 / XRE_TermEmbedding
#include "nsIFile.h"
#include "nsIWebBrowser.h"
#include "nsComponentManagerUtils.h" // do_CreateInstance
#include "nsXPCOM.h"                  // NS_NewNativeLocalFile
#include "nsStringGlue.h"            // nsCString / nsDependentCString (frozen API)
#include "nsIPrefBranch.h"           // default mobile prefs
#include "nsServiceManagerUtils.h"   // do_GetService
#include "nsThreadUtils.h"           // NS_IsMainThread
#include "DialogService.h"           // InstallDialogService (dialog interception)
#include "DownloadService.h"         // InstallDownloadService (download handoff)
#include "JihadUserAgent.h"          // JIHAD_USER_AGENT (shared UA string)
#include "JihadRuntimePaths.h"       // the ONE runtime-state dir (T-057 / R8)
#include "JihadCrashReport.h"        // re-arm the fatal-signal dump after engine init
#include "nsIObserver.h"             // per-domain UA override (http-on-modify-request)
#include "nsIObserverService.h"
#include "nsIDirectoryService.h"     // nsIDirectoryServiceProvider (profile dir — cookie persistence)
#include "nsIXULAppInfo.h"           // app identity: the add-on compatibility contract
#include "nsIXULRuntime.h"
#include "nsIPlatformInfo.h"
#include "nsXPCOMCIDInternal.h"      // XULAPPINFO_SERVICE_CONTRACTID / XULRUNTIME_…
#include "nsIComponentRegistrar.h"   // RegisterFactory (override the stock app-info)
#include "nsIFactory.h"
#include "nsServiceManagerUtils.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIComponentManager.h"     // GetClassObject (evict the stock app-info factory)  // do_GetService (app-info readback probe)
#include "nsIHttpChannel.h"
#include "nsIConsoleService.h"       // chrome-JS error visibility (T-067 / R6)
#include "nsIConsoleListener.h"
#include "nsIScriptError.h"
#include "nsIURI.h"
#include "nsISupportsImpl.h"         // NS_IMPL_ISUPPORTS
#ifndef JIHAD_OFFSCREEN_ONLY
#include <gdk/gdk.h>                 // gdk_display_get_default (widget-probe display guard)
#endif
#include <string>
#include <cstring>                   // strstr/strchr for JihadPerDomainUaForUrl
#include <cstdio>                    // /proc/meminfo poll (memory-pressure watcher)
#include <cstdlib>                   // getenv/atol (JIHAD_MEM_LOW_KB, profile dir)
#include <ctime>                     // clock_gettime (watcher rate-limit)
#include <malloc.h>                  // malloc_trim (memchute pattern)
#include <cctype>                    // tolower (case-insensitive host match)

// From nsEmbedCID.h; inlined to avoid include-path churn across SDK layouts.
#define JIHAD_NS_WEBBROWSER_CONTRACTID "@mozilla.org/embedding/browser/nsWebBrowser;1"

namespace jihad {

// --- per-domain User-Agent overrides --------------------------------------------------------
// Many modern sites serve degraded or endlessly-reloading content to the Goanna/Firefox-52 UA
// (google.com is the worst offender) but work when shown a newer Firefox UA. Gecko has this
// (UserAgentOverrides.jsm + general.useragent.override.<host> prefs), but that machinery is wired
// up by nsBrowserGlue, which this bare embedding does not run — so the prefs never take effect.
// Instead we register our OWN http-on-modify-request observer that rewrites the outgoing
// User-Agent header per host. The domain→Firefox-version choices are ported from Basilisk /
// Pale-Moon's uaoverrides.inc (MPL-2.0, © Moonchild Productions / the Pale Moon team — see NOTICE);
// the version is the site-compat token, high enough that the site serves a working page but not so
// high it assumes engine features UXP lacks. SiteSpecificUserAgent-style sub-domain walk-up is done
// in jihadUaForHost so google.com also covers mail/drive/docs.google.com, reddit.com covers www., etc.
struct JihadUaRule { const char* domain; const char* ua; };
static const JihadUaRule kJihadUaTable[] = {
  { "google.com",     "Mozilla/5.0 (X11; Linux armv7l; rv:71.0) Gecko/20100101 Firefox/71.0" },
  { "gstatic.com",    "Mozilla/5.0 (X11; Linux armv7l; rv:71.0) Gecko/20100101 Firefox/71.0" },
  { "googleapis.com", "Mozilla/5.0 (X11; Linux armv7l; rv:61.9) Gecko/20100101 Firefox/61.9" },
  { "youtube.com",    "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "bing.com",       "Mozilla/5.0 (X11; Linux armv7l; rv:52.9) Gecko/20100101 Firefox/52.9" },
  { "yahoo.com",      "Mozilla/5.0 (X11; Linux armv7l; rv:99.9) Gecko/20100101 Firefox/99.9" },
  { "live.com",       "Mozilla/5.0 (X11; Linux armv7l; rv:52.9) Gecko/20100101 Firefox/52.9" },
  { "outlook.com",    "Mozilla/5.0 (X11; Linux armv7l; rv:52.9) Gecko/20100101 Firefox/52.9" },
  { "instagram.com",  "Mozilla/5.0 (X11; Linux armv7l; rv:68.0) Gecko/20100101 Firefox/68.0" },
  { "dropbox.com",    "Mozilla/5.0 (X11; Linux armv7l; rv:68.9) Gecko/20100101 Firefox/68.9" },
  { "reddit.com",     "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "facebook.com",   "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "twitter.com",    "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "x.com",          "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "github.com",     "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "amazon.com",     "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "wikipedia.org",  "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "linkedin.com",   "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "twitch.tv",      "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "netflix.com",    "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "tiktok.com",     "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "discord.com",    "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "wordpress.com",  "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "zoom.us",        "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "slack.com",      "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "jit.si",         "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "jitsi.org",      "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "whatsapp.com",   "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "telegram.org",   "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "spotify.com",    "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "gitlab.com",     "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "microsoft.com",  "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
  { "office.com",     "Mozilla/5.0 (X11; Linux armv7l; rv:78.0) Gecko/20100101 Firefox/78.0" },
};

// Return the override UA for a host, walking up the sub-domain chain (www.reddit.com -> reddit.com),
// or nullptr if no rule matches. Matches on a full label boundary so "notgoogle.com" never matches.
static const char* jihadUaForHost(const nsACString& hostCStr) {
  std::string h(hostCStr.BeginReading(), hostCStr.Length());
  if (!h.empty() && h[h.size() - 1] == '.') h.erase(h.size() - 1);   // FQDN trailing dot (google.com.)
  while (!h.empty()) {
    for (size_t i = 0; i < sizeof(kJihadUaTable) / sizeof(kJihadUaTable[0]); ++i)
      if (h == kJihadUaTable[i].domain) return kJihadUaTable[i].ua;
    size_t dot = h.find('.');
    if (dot == std::string::npos) break;
    h.erase(0, dot + 1);
  }
  return nullptr;
}

// Public (see EngineHost.h): extract the host from a URL string and return its per-domain UA
// override, so LoadUrl can pin the docShell customUserAgent (navigator.userAgent) to the same value
// the request-header observer sends — otherwise client-side UA gating sees a stale FF52 (F-240).
const char* JihadPerDomainUaForUrl(const char* url) {
  if (!url) return nullptr;
  const char* p = strstr(url, "://");
  p = p ? p + 3 : url;
  // The authority ends at the first '/', '?' or '#'. Only an '@' WITHIN the authority is userinfo —
  // an '@' in the query (e.g. ?email=a@b) is not (Codex F-269).
  const char* end = p;
  while (*end && *end != '/' && *end != '?' && *end != '#') ++end;
  for (const char* q = p; q < end; ++q) if (*q == '@') { p = q + 1; break; }
  // host = authority minus any :port, lowercased to match the normalized ascii host the observer
  // sees (host matching must be case-insensitive: HTTPS://GOOGLE.COM should match — F-269).
  std::string host;
  for (const char* q = p; q < end && *q != ':'; ++q) host += (char)tolower((unsigned char)*q);
  if (host.empty()) return nullptr;
  nsDependentCString h(host.c_str());
  return jihadUaForHost(h);
}

class JihadUaOverride final : public nsIObserver {
public:
  NS_DECL_ISUPPORTS
  NS_IMETHOD Observe(nsISupports* aSubject, const char* aTopic, const char16_t*) override {
    nsCOMPtr<nsIHttpChannel> chan = do_QueryInterface(aSubject);
    if (!chan) return NS_OK;
    nsCOMPtr<nsIURI> uri; chan->GetURI(getter_AddRefs(uri));
    if (!uri) return NS_OK;
    nsAutoCString host; uri->GetAsciiHost(host);
    if (host.IsEmpty()) return NS_OK;
    const char* ua = jihadUaForHost(host);
    if (ua) chan->SetRequestHeader(NS_LITERAL_CSTRING("User-Agent"), nsDependentCString(ua), false);
    return NS_OK;
  }
private:
  ~JihadUaOverride() {}
};
NS_IMPL_ISUPPORTS(JihadUaOverride, nsIObserver)

static void jihadRegisterUaOverride() {
  static bool s_registered = false;
  if (s_registered) return;
  nsCOMPtr<nsIObserverService> obs = do_GetService("@mozilla.org/observer-service;1");
  if (!obs) return;
  nsCOMPtr<nsIObserver> o = new JihadUaOverride();
  // ownsWeak=false: the observer service keeps a strong ref for the process lifetime.
  if (NS_SUCCEEDED(obs->AddObserver(o, "http-on-modify-request", false))) s_registered = true;
}

EngineHost::~EngineHost()
{
  Shutdown();
}

// Defined below (with the app-info service itself); declared here because the
// directory provider is the earliest hook we own and calls it. See GetFile().
static void InstallAppInfoServiceEarly();

// ── profile directory provider ──────────────────────────────────────────────
// Without a profile dir ("ProfD"/"ProfLD"), the cookie service, permission
// manager, and disk cache have nowhere to persist — cookies become memory-only
// and die with the daemon. That is exactly the failure Atlas hit on WPE: every
// restart lost consent/login cookies, so consent-gated modern sites bounced to
// their consent wall on each launch. Provide a writable profile dir so
// cookies.sqlite + the disk cache survive restarts. Both halves are derived and
// validated by JihadRuntimePaths.h — $APP/profile and $APP/cache on cryptofs,
// where isis and Atlas both put them ($JIHAD_PROFILE_DIR / $JIHAD_CACHE_DIR
// override, R8-guarded). See the block in Init() for why there is no longer a
// <greDir>/../profile fallback.
class JihadDirProvider final : public nsIDirectoryServiceProvider
{
public:
  NS_DECL_ISUPPORTS
  // Two directories, not one. Gecko already splits the profile into a DURABLE
  // half and a DISPOSABLE half, and we answer each from the storage that suits
  // it (see RuntimeCacheDir() in JihadRuntimePaths.h for the measured numbers):
  //   ProfD  -> $APP/profile   durable: cookies.sqlite, prefs.js, permissions
  //   ProfLD -> $APP/cache     disposable: cache2, startupCache
  // Both on cryptofs, where isis and Atlas both put them (see JihadRuntimePaths.h
  // for the citations and the VFAT file-locking reason). /var is NOT used for
  // either: 49.6 MB free, shared with system state. aLocal falls back to aProfile
  // when there is no separate cache dir, so behaviour degrades rather than breaks.
  JihadDirProvider(nsIFile* aProfile, nsIFile* aLocal)
      : mProfile(aProfile), mLocal(aLocal ? aLocal : aProfile) {}

  NS_IMETHOD GetFile(const char* aProp, bool* aPersistent, nsIFile** aResult) override
  {
    // EARLIEST EMBEDDER HOOK. This is the only callback we own that runs INSIDE
    // NS_InitXPCOM2, i.e. before XRE_InitEmbedding2 fires "app-startup" and lets
    // chrome JS run. That ordering matters for exactly one thing: Services.jsm
    // caches `Services.appinfo` on first access (defineLazyGetter), and whatever
    // resolves it first wins for the life of the process — so registering our
    // nsIXULAppInfo after app-startup is too late for JS even though the
    // component manager is correct (measured: do_GetService returns ours while
    // AddonManager still reads undefined). Registering here gets in first.
    // RETRIED on every call, not attempted once: the FIRST GetFile happens before
    // the component manager exists (NS_GetComponentRegistrar returns
    // NS_ERROR_NOT_INITIALIZED, measured), so a one-shot attempt always lost the
    // race. GetFile is called many times during startup; this takes the first one
    // that can succeed, which is still comfortably before app-startup JS runs.
    // Costs one already-registered bool test per call afterwards.
    InstallAppInfoServiceEarly();
    *aPersistent = true;
    // The four profile keys nsXULAppAPI.h documents for embedders that pick the
    // profile BEFORE XRE_InitEmbedding. "ProfDS"/"ProfLDS" are the *startup*
    // aliases nsXREDirProvider falls back to for consumers that run before a
    // profile is "selected" — the startup cache is one (StartupCache::Init asks
    // for ProfLDS), so omitting them left it homeless.
    nsIFile* src = nullptr;
    if (!strcmp(aProp, "ProfD") ||          // NS_APP_USER_PROFILE_50_DIR
        !strcmp(aProp, "ProfDS")) {         // NS_APP_PROFILE_DIR_STARTUP
      src = mProfile;
    } else if (!strcmp(aProp, "ProfLD") ||  // NS_APP_USER_PROFILE_LOCAL_50_DIR
               !strcmp(aProp, "ProfLDS")) { // NS_APP_PROFILE_LOCAL_DIR_STARTUP
      src = mLocal;
    }
    if (src) {
      nsCOMPtr<nsIFile> f;
      if (NS_SUCCEEDED(src->Clone(getter_AddRefs(f))) && f) {
        f.forget(aResult);
        return NS_OK;
      }
    }
    return NS_ERROR_FAILURE;
  }

private:
  ~JihadDirProvider() {}
  nsCOMPtr<nsIFile> mProfile;   // ProfD  — durable  ($APP/profile)
  nsCOMPtr<nsIFile> mLocal;     // ProfLD — disposable ($APP/cache)
};
NS_IMPL_ISUPPORTS(JihadDirProvider, nsIDirectoryServiceProvider)

// Held (deliberately leaked) for the process lifetime — XRE keeps a raw pointer
// to the provider, so it must outlive the embedding.
static JihadDirProvider* sJihadDirProvider = nullptr;

// ── application identity: nsIXULAppInfo + nsIXULRuntime ─────────────────────
//
// WHY THIS EXISTS AT ALL — it is the fix for a hard device crash, and the
// prerequisite for add-ons.
//
// UXP registers a stock nsXULAppInfo (toolkit/xre/nsAppRunner.cpp, static module
// "Apprunner"), so `@mozilla.org/xre/app-info;1` RESOLVES even in an embedding.
// But its interface map is conditional:
//
//     NS_INTERFACE_MAP_ENTRY_CONDITIONAL(nsIXULAppInfo, gAppData || XRE_IsContentProcess())
//
// and `gAppData` is only ever set by XRE_main() from application.ini. We come up
// through XRE_InitEmbedding2(), which never touches it — so QI(nsIXULAppInfo)
// returns NS_NOINTERFACE and the service exposes nsIXULRuntime ONLY.
//
// Services.jsm swallows exactly that (toolkit/modules/Services.jsm:21-33: get the
// nsIXULRuntime, try to QI to nsIXULAppInfo, ignore NS_NOINTERFACE), so
// `Services.appinfo` exists but every nsIXULAppInfo attribute on it is undefined.
// AddonManager.jsm:770-781 then does:
//
//     try { oldAppVersion = Services.prefs.getCharPref(PREF_EM_LAST_APP_VERSION);
//           appChanged = Services.appinfo.version != oldAppVersion; } catch (e) {}
//     if (appChanged !== false)
//       Services.prefs.setCharPref(PREF_EM_LAST_APP_VERSION, Services.appinfo.version);
//
// On a fresh profile the getCharPref throws (no default argument), so appChanged
// stays `undefined`, `undefined !== false` is true, and it walks into
// setCharPref(pref, undefined) — a null `char*` — which is the
// NS_ERROR_ILLEGAL_VALUE [nsIPrefBranch.setCharPref] logged on both desktop and
// device. On the TouchPad the failure downstream of it is a SIGSEGV inside
// XRE_NotifyProfile()'s DoStartup(), which killed the daemon before it could
// serve (device runs 2026-08-01: last breadcrumb "XRE_NotifyProfile", EXIT=139;
// with the notify skipped it comes up and serves normally).
//
// So the daemon supplies its OWN app identity, unconditionally implementing
// nsIXULAppInfo + nsIPlatformInfo + nsIXULRuntime and taking over both contract
// IDs. Every value comes from JihadUserAgent.h — the same place the UA product
// token comes from — because appinfo.version and the UA's product version are the
// same fact, and an add-on's <em:targetApplication> range is checked against it.
//
// NOT solved by leaving $JIHAD_NO_PROFILE_NOTIFY set: XRE_NotifyProfile is what
// arms the profile keys, and without it nsCookieService never opens cookies.sqlite
// — i.e. it would cost exactly the persistence the profile work exists to deliver.
class JihadAppInfo final : public nsIXULAppInfo, public nsIXULRuntime
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPLATFORMINFO
  NS_DECL_NSIXULAPPINFO
  NS_DECL_NSIXULRUNTIME
  JihadAppInfo() : mLogConsoleErrors(false) {}
private:
  ~JihadAppInfo() {}
  bool mLogConsoleErrors;
};

// nsIXULAppInfo inherits nsIPlatformInfo, so the ambiguous nsISupports has to be
// disambiguated exactly as the stock nsXULAppInfo does.
NS_IMPL_ADDREF(JihadAppInfo)
NS_IMPL_RELEASE(JihadAppInfo)
NS_INTERFACE_MAP_BEGIN(JihadAppInfo)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIXULAppInfo)
  NS_INTERFACE_MAP_ENTRY(nsIPlatformInfo)
  NS_INTERFACE_MAP_ENTRY(nsIXULAppInfo)
  NS_INTERFACE_MAP_ENTRY(nsIXULRuntime)
NS_INTERFACE_MAP_END

// --- nsIXULAppInfo: the add-on compatibility contract -----------------------
NS_IMETHODIMP JihadAppInfo::GetVendor(nsACString& r)  { r.AssignLiteral(JIHAD_APP_VENDOR);  return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetName(nsACString& r)    { r.AssignLiteral(JIHAD_APP_NAME);    return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetID(nsACString& r)      { r.AssignLiteral(JIHAD_APP_ID);      return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetVersion(nsACString& r) { r.AssignLiteral(JIHAD_APP_VERSION); return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetAppBuildID(nsACString& r) { r.AssignLiteral(JIHAD_APP_BUILD_ID); return NS_OK; }
// UAName is what Gecko would splice into a default UA. Ours is fully overridden
// (general.useragent.override), so this is only ever read by chrome JS; keep it
// equal to the UA's product token rather than letting a second spelling exist.
NS_IMETHODIMP JihadAppInfo::GetUAName(nsACString& r)  { r.AssignLiteral(JIHAD_APP_NAME);    return NS_OK; }

// --- nsIPlatformInfo: the ENGINE's version, not the product's ---------------
// MOZILLA_VERSION comes from the engine's own mozilla-config.h (6.9.0 here), so
// it tracks whatever UXP this daemon was built against and cannot drift.
// Extensions range against this separately from the app version.
NS_IMETHODIMP JihadAppInfo::GetPlatformVersion(nsACString& r) { r.AssignLiteral(MOZILLA_VERSION); return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetPlatformBuildID(nsACString& r) { r.AssignLiteral(JIHAD_APP_BUILD_ID); return NS_OK; }

// --- nsIXULRuntime ----------------------------------------------------------
NS_IMETHODIMP JihadAppInfo::GetInSafeMode(bool* r)  { *r = false; return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetLogConsoleErrors(bool* r) { *r = mLogConsoleErrors; return NS_OK; }
NS_IMETHODIMP JihadAppInfo::SetLogConsoleErrors(bool v)  { mLogConsoleErrors = v; return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetOS(nsACString& r) { r.AssignLiteral("Linux"); return NS_OK; }
// TARGET_XPCOM_ABI is the engine build's own ABI tag ("arm-eabi-gcc3" on device,
// "x86_64-gcc3" on the desktop harness). It gates XPIs carrying binary XPCOM
// components, so it must be the real one, not a guess.
NS_IMETHODIMP JihadAppInfo::GetXPCOMABI(nsACString& r) {
#ifdef TARGET_XPCOM_ABI
  r.AssignLiteral(TARGET_XPCOM_ABI); return NS_OK;
#else
  r.Truncate(); return NS_ERROR_NOT_AVAILABLE;
#endif
}
// The engine is built --enable-default-toolkit=cairo-headless and the daemon
// renders through PuppetWidget offscreen; report that rather than a GTK it does
// not have.
NS_IMETHODIMP JihadAppInfo::GetWidgetToolkit(nsACString& r) {
#if defined(MOZ_WIDGET_TOOLKIT)
  r.AssignLiteral(MOZ_WIDGET_TOOLKIT);
#else
  r.AssignLiteral("headless");
#endif
  return NS_OK;
}
// Single process, always: e10s is off in this embedding, so there is no content
// child to be and nothing to spawn.
NS_IMETHODIMP JihadAppInfo::GetProcessType(uint32_t* r) { *r = 0 /* GeckoProcessType_Default */; return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetProcessID(uint32_t* r)   { *r = (uint32_t)getpid(); return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetUniqueProcessID(uint64_t* r) { *r = (uint64_t)getpid(); return NS_OK; }
NS_IMETHODIMP JihadAppInfo::EnsureContentProcess()      { return NS_ERROR_NOT_IMPLEMENTED; }
NS_IMETHODIMP JihadAppInfo::GetAccessibilityEnabled(bool* r) { *r = false; return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetIs64Bit(bool* r) { *r = (sizeof(void*) == 8); return NS_OK; }
// No compatibility.ini and no updater: there is no "restart" for us to invalidate
// caches across. Report success rather than failing a caller that is only asking
// us to remember something for next boot — the startup cache is rebuilt from the
// (disposable) ProfLD tree anyway.
NS_IMETHODIMP JihadAppInfo::InvalidateCachesOnRestart() { return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetReplacedLockTime(PRTime* r) { *r = 0; return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetLastRunCrashID(nsAString& r) { r.Truncate(); return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetIsReleaseOrBeta(bool* r)  { *r = true;  return NS_OK; }
// "Official branding" means the vendor's trademarked branding, which this build
// deliberately does not carry (cavekit-licensing-branding.md R3 stripped it).
NS_IMETHODIMP JihadAppInfo::GetIsOfficialBranding(bool* r) { *r = false; return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetIsOfficial(bool* r)        { *r = false; return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetDefaultUpdateChannel(nsACString& r) { r.AssignLiteral("default"); return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetDistributionID(nsACString& r) { r.Truncate(); return NS_OK; }
NS_IMETHODIMP JihadAppInfo::GetWindowsDLLBlocklistStatus(bool* r) { *r = false; return NS_OK; }

// We register under the STOCK app-info CID, {95d89e3e-a169-41a3-8e56-719978e15b12}
// (toolkit/xre/nsAppRunner.cpp APPINFO_CID), rather than minting our own.
//
// WHY, and this is the whole reason the first attempt failed: re-pointing the
// CONTRACT id is not enough to reach chrome JS. `Cc["@mozilla.org/xre/app-info;1"]`
// builds an nsJSCID, and nsJSCID::NewID (js/xpconnect/src/XPCJSID.cpp) resolves a
// contract string to a CID *eagerly* — `registrar->ContractIDToCID(str, &cid)`
// then `InitWithName(*cid, str)` — so from that moment the JS object is pinned to
// a CID and getService() never consults the contract table again. Remapping the
// contract therefore fixes C++ callers and silently misses JS ones. Measured
// exactly that: with only the contract remapped, do_GetService() returned our
// object at both probe points while AddonManager still read undefined, and our
// GetVersion() was never called from JS at all.
//
// Owning the CID makes both routes land here. It means UNREGISTERING the stock
// factory first (the component manager refuses a duplicate CID with
// NS_ERROR_FACTORY_EXISTS), which is safe this early: the stock service is a
// gAppData-less stub that cannot answer nsIXULAppInfo, nothing has had a reason
// to hold it, and any cached service instance is dropped with its factory entry.
#define JIHAD_APPINFO_CID \
  { 0x95d89e3e, 0xa169, 0x41a3, \
    { 0x8e, 0x56, 0x71, 0x99, 0x78, 0xe1, 0x5b, 0x12 } }
static const nsCID kJihadAppInfoCID = JIHAD_APPINFO_CID;

// Factory returning the one process-wide instance (same shape as the dialog
// service's singleton factory in DownloadService.cpp).
class JihadAppInfoFactory final : public nsIFactory {
public:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aResult) override {
    if (!aResult) return NS_ERROR_NULL_POINTER;
    if (aIID.Equals(NS_GET_IID(nsISupports)) || aIID.Equals(NS_GET_IID(nsIFactory))) {
      *aResult = static_cast<nsIFactory*>(this);
      AddRef();
      return NS_OK;
    }
    *aResult = nullptr;
    return NS_NOINTERFACE;
  }
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override  { return 2; }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override { return 1; }
  NS_IMETHOD CreateInstance(nsISupports* aOuter, const nsIID& aIID, void** aResult) override {
    if (aOuter) return NS_ERROR_NO_AGGREGATION;
    if (!mInstance) { mInstance = new JihadAppInfo(); NS_ADDREF(mInstance); }
    return mInstance->QueryInterface(aIID, aResult);
  }
  NS_IMETHOD LockFactory(bool) override { return NS_OK; }
private:
  JihadAppInfo* mInstance = nullptr;   // deliberately leaked: process lifetime
};
static JihadAppInfoFactory& AppInfoFactory() { static JihadAppInfoFactory f; return f; }

// Re-point BOTH contract IDs at our implementation. Registering a contract that
// already exists re-maps it (the same override the helper-app dialog and the
// transfer service use), so the stock gAppData-less nsXULAppInfo stops being
// reachable through either name. MUST run before XRE_NotifyProfile(): that is
// what starts the add-on manager, and Services.appinfo is a lazy getter that
// caches the first object it resolves.
static bool InstallAppInfoService()
{
  static bool sRegistered = false;
  if (sRegistered) return true;          // the early hook may have done it already
  // Not an error during the earliest retries — the component manager simply does
  // not exist yet (NS_ERROR_NOT_INITIALIZED). Silent: the caller retries.
  nsCOMPtr<nsIComponentRegistrar> reg;
  if (NS_FAILED(NS_GetComponentRegistrar(getter_AddRefs(reg))) || !reg) return false;
  // Evict the stock factory so the CID is free (see the CID comment above).
  nsCOMPtr<nsIComponentManager> cm;
  if (NS_SUCCEEDED(NS_GetComponentManager(getter_AddRefs(cm))) && cm) {
    nsCOMPtr<nsIFactory> stock;
    if (NS_SUCCEEDED(cm->GetClassObject(kJihadAppInfoCID, NS_GET_IID(nsIFactory),
                                        getter_AddRefs(stock))) && stock) {
      reg->UnregisterFactory(kJihadAppInfoCID, stock);
    }
  }
  // First call registers the CID and points the app-info contract at it.
  nsresult rv1 = reg->RegisterFactory(kJihadAppInfoCID, "Jihad App Info",
                                      XULAPPINFO_SERVICE_CONTRACTID, &AppInfoFactory());
  // The second contract must be aliased with a NULL factory. Passing the factory
  // again re-registers the same CID, which nsComponentManagerImpl rejects with
  // NS_ERROR_FACTORY_EXISTS ("if a null factory is passed in, this call just
  // wants to reset the contract ID to point to an existing CID entry" —
  // xpcom/components/nsComponentManager.cpp).
  nsresult rv2 = reg->RegisterFactory(kJihadAppInfoCID, "Jihad App Info",
                                      XULRUNTIME_SERVICE_CONTRACTID, nullptr);
  if (NS_FAILED(rv1) || NS_FAILED(rv2)) {
    fprintf(stderr, "[jihad-bs] appinfo: RegisterFactory failed (app-info=0x%x runtime=0x%x)\n",
            (unsigned)rv1, (unsigned)rv2);
    return false;
  }

  sRegistered = true;
  return true;
}

// One-shot wrapper for the earliest hook (JihadDirProvider::GetFile). Silent on
// failure: at that point in XPCOM startup a diagnostic write is not necessarily
// safe, and Init()'s explicit call reports properly a moment later.
static void InstallAppInfoServiceEarly()
{
  static bool sDone = false;
  if (sDone) return;
  sDone = InstallAppInfoService();
  if (sDone) fprintf(stderr, "[jihad-bs] appinfo: registered early (before app-startup JS)\n");
}

// READ IT BACK, exactly the way chrome JS does. Registering a factory over an
// existing contract id is an override, and an override that silently did not take
// is invisible until something downstream crashes — the same reason the adapter
// build reads its own identity strings back out of the .so.
//
// This replicates Services.jsm's getter verbatim (toolkit/modules/Services.jsm:21-33):
// getService(nsIXULRuntime) on the app-info contract, then QI to nsIXULAppInfo.
// If that sequence yields our identity here but `Services.appinfo.version` is still
// undefined in JS, the divergence is on the JS side, not in the component manager —
// which is precisely the distinction worth being able to make from a log.
static void ProbeAppInfo(const char* when)
{
  nsCOMPtr<nsIXULRuntime> rt = do_GetService(XULAPPINFO_SERVICE_CONTRACTID);
  if (!rt) {
    fprintf(stderr, "[jihad-bs] appinfo[%s]: contract does not resolve at all\n", when);
    return;
  }
  nsCOMPtr<nsIXULAppInfo> ai = do_QueryInterface(rt);
  if (!ai) {
    fprintf(stderr, "[jihad-bs] appinfo[%s]: resolves, but QI(nsIXULAppInfo) FAILS "
                    "(this is the stock gAppData-less nsXULAppInfo)\n", when);
    return;
  }
  nsCString v, id;
  ai->GetVersion(v);
  ai->GetID(id);
  fprintf(stderr, "[jihad-bs] appinfo[%s]: id=%s version=%s platform=%s\n",
          when, id.get(), v.get(), MOZILLA_VERSION);
}

// ── nsIClipboardHelper (T-067 / cavekit-input-bridging R6) ──────────────────────────────────
//
// The headless widget module (build/desktop/patches/0006) registers five contracts and its own
// comment concedes "theme, clipboard, idle, … are still TODO". One of those omissions is not
// cosmetic: `@mozilla.org/widget/clipboardhelper;1` is fetched at TOP LEVEL by
// `chrome://global/content/config.js`:
//
//     const gClipboardHelper = Components.classes[nsClipboardHelper_CONTRACTID].getService(...)
//
// With no registration, `Components.classes[...]` is `undefined`, the `.getService` throws a
// TypeError, and the whole script aborts at line 21 — so `gPrefHash`, `ShowPrefs` and every other
// definition below it never come into existence. That, not the input path, is why about:config's
// button did nothing even once its `oncommand` was firing (measured: the click reached
// `ShowPrefs@config.js:346`, which then died on `gPrefHash is undefined`).
//
// Registering it here rather than in the engine keeps this a daemon change with no libxul
// rebuild. The CID is ours: nothing in a headless build claims that contract, so this is a fresh
// registration, not the kind of override that the app-info work found JS caches defeat.
//
// SCOPE, stated honestly: this stores the copied text in-process. webOS clipboard integration is
// a separate concern with no requirement behind it yet, so nothing here pretends to reach the
// system clipboard — the point is that chrome which merely *asks for the service* stops dying.
// The interface is declared here rather than included: nsIClipboardHelper.idl carries a
// `%{C++ #include "nsString.h"}` block, and nsString.h is an INTERNAL string header that this
// frozen-API daemon cannot compile against ("Internal string headers are not available from
// external-linkage code"). Two pure virtuals in IDL order after nsISupports is the entire ABI,
// and `const nsAString&` is a pointer either way — the same external/internal string equivalence
// DialogService already relies on to implement nsIPromptService.
#define JIHAD_NS_ICLIPBOARDHELPER_IID \
  { 0x438307fd, 0x0c68, 0x4d79, { 0x92, 0x2a, 0xf6, 0xcc, 0x95, 0x50, 0xcd, 0x02 } }

class nsIClipboardHelper : public nsISupports {
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(JIHAD_NS_ICLIPBOARDHELPER_IID)
  NS_IMETHOD CopyStringToClipboard(const nsAString& aString, int32_t aClipboardID) = 0;
  NS_IMETHOD CopyString(const nsAString& aString) = 0;
};
NS_DEFINE_STATIC_IID_ACCESSOR(nsIClipboardHelper, JIHAD_NS_ICLIPBOARDHELPER_IID)

static nsCString& JihadClipboardText() { static nsCString s; return s; }

class JihadClipboardHelper final : public nsIClipboardHelper {
public:
  NS_DECL_ISUPPORTS
  NS_IMETHOD CopyStringToClipboard(const nsAString& aString, int32_t) override {
    JihadClipboardText() = NS_ConvertUTF16toUTF8(aString);
    return NS_OK;
  }
  NS_IMETHOD CopyString(const nsAString& aString) override {
    JihadClipboardText() = NS_ConvertUTF16toUTF8(aString);
    return NS_OK;
  }
private:
  ~JihadClipboardHelper() {}
};
NS_IMPL_ISUPPORTS(JihadClipboardHelper, nsIClipboardHelper)

// {0f0d1e2a-7c44-4b8e-9d3a-5b6c7e8f9a01} — Jihad-owned; no headless build registers this contract.
#define JIHAD_CLIPBOARDHELPER_CID \
  { 0x0f0d1e2a, 0x7c44, 0x4b8e, { 0x9d, 0x3a, 0x5b, 0x6c, 0x7e, 0x8f, 0x9a, 0x01 } }
static const nsCID kJihadClipboardHelperCID = JIHAD_CLIPBOARDHELPER_CID;

class JihadClipboardHelperFactory final : public nsIFactory {
public:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aResult) override {
    if (!aResult) return NS_ERROR_NULL_POINTER;
    if (aIID.Equals(NS_GET_IID(nsISupports)) || aIID.Equals(NS_GET_IID(nsIFactory))) {
      *aResult = static_cast<nsIFactory*>(this);
      AddRef();
      return NS_OK;
    }
    *aResult = nullptr;
    return NS_NOINTERFACE;
  }
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override  { return 2; }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override { return 1; }
  NS_IMETHOD CreateInstance(nsISupports* aOuter, const nsIID& aIID, void** aResult) override {
    if (aOuter) return NS_ERROR_NO_AGGREGATION;
    if (!mInstance) { mInstance = new JihadClipboardHelper(); NS_ADDREF(mInstance); }
    return mInstance->QueryInterface(aIID, aResult);
  }
  NS_IMETHOD LockFactory(bool) override { return NS_OK; }
private:
  JihadClipboardHelper* mInstance = nullptr;   // deliberately leaked: process lifetime
};
static JihadClipboardHelperFactory& ClipboardHelperFactory()
{ static JihadClipboardHelperFactory f; return f; }

static void InstallClipboardHelper()
{
  nsCOMPtr<nsIClipboardHelper> have = do_GetService("@mozilla.org/widget/clipboardhelper;1");
  if (have) return;                     // a toolkit that already provides one wins (desktop GTK)
  nsCOMPtr<nsIComponentRegistrar> reg;
  if (NS_FAILED(NS_GetComponentRegistrar(getter_AddRefs(reg))) || !reg) return;
  nsresult rv = reg->RegisterFactory(kJihadClipboardHelperCID, "Jihad Clipboard Helper",
                                     "@mozilla.org/widget/clipboardhelper;1",
                                     &ClipboardHelperFactory());
  fprintf(stderr, "[jihad-bs] clipboardhelper: %s (headless widget module does not provide one)\n",
          NS_SUCCEEDED(rv) ? "registered" : "REGISTRATION FAILED");
}

// ── chrome-JS console bridge (T-067 / cavekit-input-bridging R6) ────────────────────────────
//
// WHY: nothing in this embedding listens to the console service, so EVERY error thrown by
// chrome JavaScript is discarded. That is not a small gap — `about:addons` renders a
// `<parsererror>`, `about:config`'s warning button does nothing, and in both cases the engine
// knows exactly why and we were throwing the explanation away. Two device round-trips were spent
// guessing at symptoms a single console line would have named.
//
// SCOPE, deliberate: only messages whose source is OUR chrome are printed. Errors from web content
// are the site's business, would be high-volume on real browsing, and could echo page data into the
// log, which F-163 forbids. $JIHAD_JS_CONSOLE=all lifts the filter for a debugging session; =0
// turns the bridge off entirely.
//
// F-8 tightening — the first version of this filter passed `about:` wholesale and passed EVERY
// empty source, and both are reachable by content: `about:blank` (and `about:srcdoc`) are the
// source name of script in a content-created iframe, and a script error raised from eval/data:
// carries no source name at all. Either one echoes page-controlled text into the persistent device
// log, which is the exact leak class the filter exists to prevent. So: chrome:// and resource://
// always; `about:` only for the specific chrome pages this browser drives (content cannot navigate
// to those); and an empty source only when the message is NOT an nsIScriptError — a plain
// nsIConsoleMessage comes from XPCOM internals (the chrome registry, NSS), never from page JS.
class JihadConsoleListener final : public nsIConsoleListener
{
public:
  NS_DECL_ISUPPORTS
  NS_IMETHOD Observe(nsIConsoleMessage* aMessage) override
  {
    if (!aMessage) return NS_OK;
    nsCOMPtr<nsIScriptError> err = do_QueryInterface(aMessage);
    nsAutoString src;
    if (err) err->GetSourceName(src);
    NS_ConvertUTF16toUTF8 src8(src);
    if (!sAll) {
      const char* s = src8.get();
      bool ours;
      if (!s || !*s) {
        ours = !err;                       // internal console message, not script (F-8)
      } else if (!strncmp(s, "chrome://", 9) || !strncmp(s, "resource://", 11)) {
        ours = true;
      } else {
        // The chrome about: pages we actually drive. NOT a prefix match on "about:" — that
        // admits about:blank, i.e. content (F-8).
        ours = !strcmp(s, "about:config") || !strcmp(s, "about:addons") ||
               !strcmp(s, "about:preferences") || !strcmp(s, "about:support");
      }
      if (!ours) return NS_OK;
    }
    // nsIConsoleMessage::message is `wstring`, not AString — it hands back a malloc'd
    // char16_t* the caller owns (unlike nsIScriptError::sourceName, which is AString).
    char16_t* raw = nullptr;
    aMessage->GetMessageMoz(&raw);
    uint32_t line = 0;
    if (err) err->GetLineNumber(&line);
    fprintf(stderr, "[jihad-bs] js-console %s:%u %s\n",
            src8.get() && *src8.get() ? src8.get() : "(no source)", line,
            raw ? NS_ConvertUTF16toUTF8(nsDependentString(raw)).get() : "");
    if (raw) NS_Free(raw);
    return NS_OK;
  }
  static bool sAll;
private:
  ~JihadConsoleListener() {}
};
bool JihadConsoleListener::sAll = false;
NS_IMPL_ISUPPORTS(JihadConsoleListener, nsIConsoleListener)

static void InstallConsoleListener()
{
  const char* e = getenv("JIHAD_JS_CONSOLE");
  if (e && *e && !strcmp(e, "0")) return;
  JihadConsoleListener::sAll = (e && !strcmp(e, "all"));
  nsCOMPtr<nsIConsoleService> cs = do_GetService("@mozilla.org/consoleservice;1");
  if (!cs) { fprintf(stderr, "[jihad-bs] js-console: no console service\n"); return; }
  RefPtr<JihadConsoleListener> l = new JihadConsoleListener();
  cs->RegisterListener(l);
  fprintf(stderr, "[jihad-bs] js-console bridge on (%s)\n",
          JihadConsoleListener::sAll ? "ALL sources" : "chrome/resource/about only");
}

// T-067 (cavekit-input-bridging R6): which widget components does this build actually have?
//
// The headless toolkit (build/desktop/patches/0006) registers FIVE widget contracts — app shell,
// screen manager, transferable, format converter, gfxinfo — and its own comment says "theme,
// clipboard, idle, … are still TODO". The desktop GTK2 build registers all of them. That is the
// single structural difference between the configuration where synthesized XUL input works
// (desktop) and the one where it SIGSEGV'd (device), so the missing set is evidence, not trivia:
// XUL frames are the heaviest users of nsITheme and the drag service, which is why HTML pages
// never noticed. Printed once at init, unconditionally, because a device round-trip costs five
// minutes and this is the cheapest possible way to make the difference visible in the daemon log.
static void ProbeWidgetComponents()
{
#ifndef JIHAD_OFFSCREEN_ONLY
  // Desktop GTK2 build with NO X server: getting the drag-service/clipboard
  // contracts CONSTRUCTS GTK widgets against a null display → gdk_window_new
  // assertion → SIGSEGV (found 2026-08-02: the no-X round-trip had not run since
  // this probe landed in T-067). The probe is diagnostics, not a dependency —
  // skip it rather than crash the daemon it exists to diagnose. The device
  // (JIHAD_OFFSCREEN_ONLY, headless toolkit) has no GTK and probes safely.
  // Checked on the OPENED display, not just $DISPLAY (review F15): DISPLAY=:99
  // with no Xvfb behind it crashes identically.
  const char* disp = getenv("DISPLAY");
  if (!disp || !*disp || !gdk_display_get_default()) {
    fprintf(stderr, "[jihad-bs] widget-probe skipped (GTK build, no usable display)\n");
    return;
  }
#endif
  static const char* kContracts[] = {
    "@mozilla.org/chrome/chrome-native-theme;1",   // nsITheme — every -moz-appearance XUL widget
    "@mozilla.org/widget/dragservice;1",           // EventStateManager drag-gesture path
    "@mozilla.org/widget/clipboard;1",
    "@mozilla.org/widget/lookandfeel;1",
    "@mozilla.org/widget/idleservice;1",
    "@mozilla.org/widget/appshell/headless;1",     // present: the control for this probe
    "@mozilla.org/gfx/screenmanager;1",            // present: the control for this probe
    nullptr
  };
  for (int i = 0; kContracts[i]; ++i) {
    nsCOMPtr<nsISupports> s = do_GetService(kContracts[i]);
    fprintf(stderr, "[jihad-bs] widget-probe %-46s %s\n", kContracts[i], s ? "PRESENT" : "ABSENT");
  }
}

// ── startup breadcrumbs ─────────────────────────────────────────────────────
// The TouchPad has no debugger and no core dumps: when the daemon dies during
// engine bring-up, the ONLY evidence is what reached the log before it went. A
// SIGSEGV inside this function is otherwise indistinguishable from one three
// steps later, which costs a device round-trip per hypothesis. Unbuffered stderr,
// one line per step, so the last line printed names the step that was running.
// Cheap enough to leave on permanently (a dozen writes, once per process).
static void initStep(const char* what) {
  fprintf(stderr, "[jihad-bs] init: %s\n", what);
  fflush(stderr);
}

bool
EngineHost::Init(const char* greDir)
{
  if (mInited) {
    return true;
  }
  if (!greDir || !*greDir) {
    return false;
  }

  initStep("resolving greDir");
  nsCOMPtr<nsIFile> dir;
  nsresult rv = NS_NewNativeLocalFile(nsDependentCString(greDir),
                                      /* followLinks */ true,
                                      getter_AddRefs(dir));
  if (NS_FAILED(rv) || !dir) {
    return false;
  }

  // greDir doubles as the application directory for this headless embedder.
  // Provide a PROFILE dir ("ProfD"/"ProfLD") so cookies.sqlite, permissions,
  // and the disk cache persist across daemon restarts — without it the cookie
  // service is memory-only and consent/login cookies die with the process
  // (the Atlas WPE consent-bounce lesson).
  //
  // F-5: $JIHAD_PROFILE_DIR was the ONE path environment variable that bypassed
  // JihadRuntimePaths.h entirely — taken on trust, with no R8 user-volume check
  // and no ownership/mode validation, while $JIHAD_DUMP, $JIHAD_INJECT and
  // $JIHAD_DOWNLOAD_DIR all went through the shared helpers. Both halves are now
  // derived there and nothing but the leaf names appears at this call site:
  //   RuntimeProfileDir()  ProfD  — durable: cookies.sqlite, prefs, permissions
  //   RuntimeCacheDir()    ProfLD — disposable: cache2, startupCache
  // Both resolve to the app's own directory on cryptofs, which is where isis
  // (CookieJarPath/CachePath under /media/cryptofs/.browser) and Atlas (netdata/
  // netcache/cookies.db in its app deviceroot) both put them — see the header for
  // the VFAT locking reason that decided it. Each honours its env override
  // through RuntimeResolvePath (canonicalised, R8-guarded) and is created and
  // validated before it is handed to the engine: a symlink planted at either path
  // is refused, and /media/internal is refused unconditionally.
  //
  // There is deliberately NO fallback to <greDir>/../profile any more. That was
  // the pre-T-057 location, and now that the daemon RUNS IN PLACE from the app
  // bundle it would resolve to $APP/deviceroot/profile — next to the engine
  // binaries, where a profile has no business being. Degrading to memory-only
  // cookies with a loud line is the honest outcome.
  initStep("resolving the profile + cache dirs");
  {
    const std::string& profDir  = jihad::RuntimeProfileDir();
    const std::string& cacheDir = jihad::RuntimeCacheDir();
    fprintf(stderr, "[jihad-bs] profile ProfD=%s ProfLD=%s\n",
            profDir.empty()  ? "(none — cookies memory-only)" : profDir.c_str(),
            cacheDir.empty() ? "(none — disk cache off)"      : cacheDir.c_str());
    nsCOMPtr<nsIFile> prof, local;
    if (!profDir.empty() &&
        NS_SUCCEEDED(NS_NewNativeLocalFile(nsDependentCString(profDir.c_str()),
                                           true, getter_AddRefs(prof))) && prof) {
      prof->Normalize();
      if (!cacheDir.empty() &&
          NS_SUCCEEDED(NS_NewNativeLocalFile(nsDependentCString(cacheDir.c_str()),
                                             true, getter_AddRefs(local))) && local) {
        local->Normalize();
      } else {
        local = nullptr;   // provider falls back to ProfD; cache2 shares the profile tree
      }
      sJihadDirProvider = new JihadDirProvider(prof, local);
      NS_ADDREF(sJihadDirProvider);
    }
  }
  initStep("XRE_InitEmbedding2");
  rv = XRE_InitEmbedding2(dir, dir, sJihadDirProvider);
  mInited = NS_SUCCEEDED(rv);
  initStep(mInited ? "XRE_InitEmbedding2 OK" : "XRE_InitEmbedding2 FAILED");
  // RE-ARM the fatal-signal dump. SpiderMonkey installs its own SIGSEGV handler
  // during engine init (the wasm/asm.js bounds-check trap) and replaces ours, so
  // the install in main() no longer covers anything from here on — which is
  // exactly the window the device crash lives in. Measured: without this, a fault
  // after engine init produced no report at all. See JihadCrashReport.h.
  jihad::JihadInstallCrashHandler();

  // Override "@mozilla.org/prompter;1" so content dialogs (alert/confirm/prompt)
  // are captured by our sink instead of trying to open a chrome dialog window,
  // which is absent in the headless daemon and would otherwise hang the load.
  // With no sink installed the default is deny/OK — the engine never blocks.
  if (mInited) {
    initStep("InstallDialogService");
    InstallDialogService();
    initStep("InstallDownloadService");
    InstallDownloadService();
    // MUST precede XRE_NotifyProfile(): DoStartup() runs the add-on manager,
    // whose very first act is to read Services.appinfo.version — and without a
    // real nsIXULAppInfo that is `undefined`, which is what crashed the daemon on
    // device. See the JihadAppInfo block above for the full chain.
    initStep("InstallAppInfoService (nsIXULAppInfo/nsIXULRuntime)");
    if (!InstallAppInfoService())
      fprintf(stderr, "[jihad-bs] WARNING: could not register the app-info service — "
                      "the add-on manager will fail and may take the process with it\n");
    ProbeAppInfo("late-backstop");
    ProbeWidgetComponents();   // T-067/R6 — see the comment on the function
    InstallConsoleListener();  // T-067/R6 — chrome JS errors were being discarded entirely
    InstallClipboardHelper();  // T-067/R6 — without it chrome/global/content/config.js aborts
    // ── announce the profile ────────────────────────────────────────────────
    // XRE_InitEmbedding2 takes the directory provider but does NOT arm it:
    // nsXREDirProvider::GetFile short-circuits EVERY profile key with
    //   if (!mProfileNotified) return NS_ERROR_FAILURE;
    // BEFORE it ever consults the embedder's provider, and mProfileNotified is
    // only set by XRE_NotifyProfile()/DoStartup(). So without this call
    // NS_GetSpecialDirectory("ProfD") fails process-wide, and every consumer
    // that keys off it silently degrades to memory-only — nsCookieService takes
    // its "couldn't get cookie file" branch in InitDBStates(), which is exactly
    // why cookies.sqlite was never created and cookies died with the daemon
    // (browser-services R2). nsXULAppAPI.h documents this call as mandatory for
    // scenario 1 (profile chosen before XRE_InitEmbedding): "XRE_NotifyProfile
    // should be called immediately after XRE_InitEmbedding".
    // It also fires profile-do-change / profile-after-change, the notifications
    // profile-scoped services wait on. Called BEFORE the pref block below
    // because DoStartup() runs Preferences::ResetAndReadUserPrefs(), which would
    // otherwise discard the runtime prefs we set here.
    //
    // KILL SWITCH (device bring-up): $JIHAD_NO_PROFILE_NOTIFY=1 skips this call.
    // It exists because XRE_NotifyProfile is the single heaviest thing engine
    // init does — DoStartup() re-reads user prefs, starts the JS Add-on Manager
    // and fires profile-do-change/profile-after-change, i.e. it runs a large
    // amount of GRE JavaScript that the desktop dist has in full and the trimmed
    // device bundle may not. Setting the switch turns a "does the daemon come up
    // at all" question into ONE run instead of a rebuild-and-reflash cycle. With
    // it set, cookies degrade to memory-only (that is exactly what this call was
    // added to fix), so it is a diagnostic, never a shipping configuration.
    const char* noNotify = getenv("JIHAD_NO_PROFILE_NOTIFY");
    if (sJihadDirProvider && !(noNotify && *noNotify && strcmp(noNotify, "0"))) {
      jihad::JihadMaybeTestCrash();   // $JIHAD_TEST_CRASH — proves the handler fires
    initStep("XRE_NotifyProfile (DoStartup: prefs + addons + profile-after-change)");
      XRE_NotifyProfile();
      initStep("XRE_NotifyProfile returned");
    } else if (sJihadDirProvider) {
      initStep("XRE_NotifyProfile SKIPPED ($JIHAD_NO_PROFILE_NOTIFY) — cookies will be memory-only");
    }
    // Mobile-browser defaults. <meta name=viewport> is off by default on desktop
    // Goanna; a webOS phone browser must honor it (drives msgMetaViewportSet).
    initStep("preferences-service");
    nsCOMPtr<nsIPrefBranch> pb =
      do_GetService("@mozilla.org/preferences-service;1");
    if (pb) pb->SetBoolPref("dom.meta-viewport.enabled", true);
    // Force devicePixelRatio = 1.0 so 1 CSS px == 1 buffer (device) px. The shared
    // framebuffer the adapter hands us IS in device pixels, and RenderDocument scales
    // content by AppUnitsPerDevPixel/60 == 1/DPR. Left on "auto", the device context
    // derives a DPR from the fixed 1024x768 screen (DPR 1.333 = 1024/768), so content
    // rendered at 1/1.333 = 0.75 filled only 75% of the buffer (white bars) and text
    // laid out at the wrong width. Pinning DPR=1 makes the page fill the buffer at 1:1;
    // user pinch-zoom is still handled separately via setZoomAndScroll (full zoom).
    if (pb) pb->SetCharPref("layout.css.devPixelsPerPx", "1.0");
    // Set the complete, identifiable UA HERE via general.useragent.override. NB on
    // goanna.js: the loose $greDir/goanna.js IS loaded by libpref (proven on-device —
    // the F-235 surfacecache cap appended there fixed the blank-degradation, and gfx
    // "Once" prefs snapshot it before Init returns), BUT a UA override placed only
    // there did NOT stick for navigator.userAgent under this embedding — the observed
    // fallback was the branding-stripped engine default "Mozilla/5.0 (X11; Linux
    // armv7l; rv:6.9) Goanna/ /6.9". Setting it as a runtime pref makes
    // nsHttpHandler::PrefsChanged pick it up reliably. SetUserAgentOverride ignores
    // the empty setUserAgent the adapter sends at connect, so this value sticks.
    //   Tokens: webOS/TouchPad platform; Goanna/6.9 (engine); UXP/<commit> (build);
    //   Firefox/52.9 (site-compat, ESR52 base); ECMAScript/2024 (JS level UXP b2594a4
    //   supports: Object.groupBy, Promise.withResolvers, String.isWellFormed, ...).
    // Keep JIHAD_UA in sync with build/webos-oe/make-device-bundle.sh docs and NOTICE.
    initStep("general.useragent.override");
    if (pb) pb->SetCharPref("general.useragent.override", JIHAD_USER_AGENT);
    // Per-domain User-Agent overrides so modern sites serve working content (see jihadUaTable).
    initStep("registering the per-domain UA observer");
    jihadRegisterUaOverride();
    // NOTE: the low-RAM (512 MB) memory + repaint tuning prefs are NOT set here — a SetIntPref at
    // this point is too late for the "Once"-style gfx prefs (image.mem.surfacecache.max_size_kb,
    // layout.frame_rate), which gfxPlatform snapshots during XRE_InitEmbedding2 BEFORE this returns
    // (Codex F-235). They live in goanna.js instead (loaded before gfx init) — see the append block
    // in build/webos-oe/make-device-bundle.sh. Critically, the stock surfacecache cap is 1 GB, which
    // is catastrophic on a 512 MB device (the surface memory grows until the live render is evicted
    // to near-blank); goanna.js overrides it to a small bounded value.
    // Read-back check (inspector P3): confirm the goanna.js low-RAM block actually
    // loaded by reading one of its prefs. Logs the live value — if it prints the
    // stock default (1048576 = 1 GB) the loose-pref load is broken and every
    // low-RAM pref is dead; investigate before trusting memory behavior.
    if (pb) {
      int32_t sc = -1;
      pb->GetIntPref("image.mem.surfacecache.max_size_kb", &sc);
      fprintf(stderr, "[jihad-bs] prefs check: surfacecache.max_size_kb=%d (%s)\n",
              sc, sc == 32768 ? "goanna.js low-RAM block ACTIVE"
                              : "UNEXPECTED — goanna.js block missing/stale?");
    }
    // NOTE: PSM/NSS (TLS) is force-initialized on the main thread in
    // GoannaRenderPage::LoadUrl (it is not registered yet this early at engine init).
    initStep("engine init complete");
  }
  return mInited;
}

already_AddRefed<nsIWebBrowser>
EngineHost::CreateBrowser()
{
  if (!mInited) {
    return nullptr;
  }
  nsCOMPtr<nsIWebBrowser> webBrowser =
    do_CreateInstance(JIHAD_NS_WEBBROWSER_CONTRACTID);
  return webBrowser.forget();
}

void
EngineHost::Shutdown()
{
  if (!mInited) {
    return;
  }
  // Clear the process-global service sinks before XPCOM teardown so a late
  // prompt/download callback can never call through a dangling sink pointer
  // (Codex P0). These services are driven on the embedding (main) thread, and
  // the sink must only be set/cleared from that thread; clearing here is the
  // process-lifetime backstop even if a caller forgot to clear its own sink.
  // F-9: abort anything still downloading FIRST, while the sink is still
  // installed — that is what lets each interrupted download deliver its terminal
  // msgDownloadError and delete its own "<target>.part" + placeholder instead of
  // leaving them as residue (R8). Nothing in the engine will call us back after
  // this, so the service terminates them itself rather than waiting.
  ShutdownDownloadService();
  SetDialogSink(nullptr);
  SetDownloadSink(nullptr);
  // CAUTION (Codex P1): XRE_TermEmbedding tears down the process-wide runtime.
  // The caller MUST have released every nsIWebBrowser and listener first. In the
  // daemon this is invoked once at process exit, after BrowserPageManager has
  // destroyed all pages; it must never run while any BrowserPage/nsIWebBrowser
  // is still live. A future revision should track outstanding instances and
  // assert the count is zero here.
  XRE_TermEmbedding();
  mInited = false;
}

// ── low-memory guardrail ────────────────────────────────────────────────────
// Ported patterns: Palm BrowserServer memchute watcher (doMemWatch — timed poll,
// malloc_trim + tiered purge) and Atlas WPE's memory budget lesson (the budget
// must sit near the REAL free memory or the engine grows past what the system
// can commit and dies before it ever purges — exactly the 512 MB Pre 3 risk).
// Goanna already has the reaction machinery: the "memory-pressure" observer
// notification drives JS GC/CC, image surface discard, and cache eviction
// (nsMemoryImpl/FlushMemory subscribers) — we just have to FIRE it, because in
// this bare embedding nothing else watches system memory.

// Available-memory estimate in kB: MemFree + Buffers + (Cached - Shmem). The
// webOS 3 kernel (2.6.35) predates MemAvailable, and its Cached INCLUDES
// shmem/tmpfs pages, which are NOT reclaimable — and this device leans on them
// (the YAP shared framebuffers are SysV shm). Counting them would over-estimate
// headroom and let the guardrail miss the OOM it exists to prevent (inspector
// P3). Same /proc/meminfo fields the Palm BrowserServer getMemInfo read.
static long jihadAvailableKb() {
  FILE* f = fopen("/proc/meminfo", "r");
  if (!f) return -1;
  char line[128]; long freeKb = 0, buffersKb = 0, cachedKb = 0, shmemKb = 0; int got = 0;
  while (got < 4 && fgets(line, sizeof(line), f)) {
    long v = 0;
    if (sscanf(line, "MemFree: %ld", &v) == 1)      { freeKb = v;    ++got; }
    else if (sscanf(line, "Buffers: %ld", &v) == 1) { buffersKb = v; ++got; }
    else if (sscanf(line, "Cached: %ld", &v) == 1)  { cachedKb = v;  ++got; }
    else if (sscanf(line, "Shmem: %ld", &v) == 1)   { shmemKb = v;   ++got; }
  }
  fclose(f);
  long reclaimable = cachedKb - shmemKb; if (reclaimable < 0) reclaimable = 0;
  return freeKb + buffersKb + reclaimable;
}

void
EngineHost::CheckMemoryPressure()
{
  if (!mInited) return;
  long now = 0;
  { struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    now = ts.tv_sec * 1000L + ts.tv_nsec / 1000000L; }
  if (now - mMemPollMs < 5000) return;   // poll /proc/meminfo at most every 5 s
  mMemPollMs = now;

  static long lowKb = -2;
  if (lowKb == -2) {   // resolve the threshold once
    lowKb = 49152;     // default 48 MB — headroom for one paint + GC on a 512 MB device
    const char* e = getenv("JIHAD_MEM_LOW_KB");
    if (e) { long v = atol(e); if (v >= 8192 && v <= 262144) lowKb = v; }
  }
  long avail = jihadAvailableKb();
  if (avail < 0 || avail >= lowKb) return;

  // Throttle the reaction: repeated pressure notifications thrash the GC (the
  // memchute watcher throttled for the same reason; Atlas's aggressive-purge
  // regression was over-frequent pressure handling).
  if (now - mMemNotifyMs < 30000) return;
  mMemNotifyMs = now;
  fprintf(stderr, "[jihad-bs] memory pressure: %ld kB available (< %ld kB) — flushing engine caches\n",
          avail, lowKb);
  nsCOMPtr<nsIObserverService> obs = do_GetService("@mozilla.org/observer-service;1");
  if (obs) obs->NotifyObservers(nullptr, "memory-pressure", u"low-memory");
  malloc_trim(0);   // return freed heap pages to the kernel (memchute pattern)
}

} // namespace jihad
