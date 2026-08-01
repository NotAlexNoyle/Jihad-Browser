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
#include "nsIObserver.h"             // per-domain UA override (http-on-modify-request)
#include "nsIObserverService.h"
#include "nsIDirectoryService.h"     // nsIDirectoryServiceProvider (profile dir — cookie persistence)
#include "nsIHttpChannel.h"
#include "nsIURI.h"
#include "nsISupportsImpl.h"         // NS_IMPL_ISUPPORTS
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

  // Override "@mozilla.org/prompter;1" so content dialogs (alert/confirm/prompt)
  // are captured by our sink instead of trying to open a chrome dialog window,
  // which is absent in the headless daemon and would otherwise hang the load.
  // With no sink installed the default is deny/OK — the engine never blocks.
  if (mInited) {
    initStep("InstallDialogService");
    InstallDialogService();
    initStep("InstallDownloadService");
    InstallDownloadService();
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
