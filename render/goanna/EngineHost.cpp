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

// From nsEmbedCID.h; inlined to avoid include-path churn across SDK layouts.
#define JIHAD_NS_WEBBROWSER_CONTRACTID "@mozilla.org/embedding/browser/nsWebBrowser;1"

namespace jihad {

EngineHost::~EngineHost()
{
  Shutdown();
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

  nsCOMPtr<nsIFile> dir;
  nsresult rv = NS_NewNativeLocalFile(nsDependentCString(greDir),
                                      /* followLinks */ true,
                                      getter_AddRefs(dir));
  if (NS_FAILED(rv) || !dir) {
    return false;
  }

  // greDir doubles as the application directory for this headless embedder;
  // no custom directory-service provider is needed for the smoke bring-up.
  rv = XRE_InitEmbedding2(dir, dir, nullptr);
  mInited = NS_SUCCEEDED(rv);

  // Override "@mozilla.org/prompter;1" so content dialogs (alert/confirm/prompt)
  // are captured by our sink instead of trying to open a chrome dialog window,
  // which is absent in the headless daemon and would otherwise hang the load.
  // With no sink installed the default is deny/OK — the engine never blocks.
  if (mInited) {
    InstallDialogService();
    InstallDownloadService();
    // Mobile-browser defaults. <meta name=viewport> is off by default on desktop
    // Goanna; a webOS phone browser must honor it (drives msgMetaViewportSet).
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
    // Set the complete, identifiable UA HERE via general.useragent.override. A bare
    // XRE_InitEmbedding embedder loads greprefs.js (from omni.ja) but NOT the loose
    // goanna.js in greDir, so a UA override placed only in goanna.js is silently
    // ignored — navigator.userAgent then falls back to the branding-stripped engine
    // default "Mozilla/5.0 (X11; Linux armv7l; rv:6.9) Goanna/ /6.9". Setting it as a
    // runtime pref makes nsHttpHandler::PrefsChanged pick it up. SetUserAgentOverride
    // ignores the empty setUserAgent the adapter sends at connect, so this value sticks.
    //   Tokens: webOS/TouchPad platform; Goanna/6.9 (engine); UXP/<commit> (build);
    //   Firefox/52.9 (site-compat, ESR52 base); ECMAScript/2024 (JS level UXP b2594a4
    //   supports: Object.groupBy, Promise.withResolvers, String.isWellFormed, ...).
    // Keep JIHAD_UA in sync with build/webos-oe/make-device-bundle.sh docs and NOTICE.
    if (pb) pb->SetCharPref("general.useragent.override", JIHAD_USER_AGENT);
    // NOTE: PSM/NSS (TLS) is force-initialized on the main thread in
    // GoannaRenderPage::LoadUrl (it is not registered yet this early at engine init).
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

} // namespace jihad
