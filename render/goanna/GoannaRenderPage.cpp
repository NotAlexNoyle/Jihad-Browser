/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — GoannaRenderPage implementation. See GoannaRenderPage.h.
 * Consolidates the proven embed_load pipeline (T-019/T-020/T-024) into a class.
 */
#include "GoannaRenderPage.h"
#include "EngineHost.h"

// JIHAD_OFFSCREEN_ONLY (device/ARM headless build): the engine's libxul is built
// with MOZ_WIDGET_TOOLKIT=headless (no GTK/X). Compile the daemon GTK-free too so
// the device bundle need not ship libgtk/gdk/pango/cairo/X. Only the JIHAD_OFFSCREEN
// PuppetWidget path is used; the legacy on-screen GTK-window path is compiled out.
#ifdef JIHAD_OFFSCREEN_ONLY
#define JIHAD_GTK_PUMP() ((void)0)
#else
#include <gtk/gtk.h>
#define JIHAD_GTK_PUMP() do { while (gtk_events_pending()) gtk_main_iteration_do(FALSE); } while (0)
#endif
#include <glib.h>
#include <ctime>

#include "nsCOMPtr.h"
#include "nsStringGlue.h"
#include "nsIWebBrowser.h"
#include "nsIWebBrowserChrome.h"
#include "nsIEmbeddingSiteWindow.h"
#include "nsIInterfaceRequestor.h"
#include "nsIBaseWindow.h"
#include "nsIWebNavigation.h"
#include "nsISHistory.h"
#include "nsIWebBrowserFind.h"
#include "nsIWebBrowserFocus.h"
#include "nsIFocusManager.h"         // GetFocusedElement — reconcile edit target after Focus (F-225)
#include "nsIWebProgress.h"
#include "nsIWebProgressListener.h"
#include "nsIURI.h"
#include "nsIChannel.h"
#include "nsIHttpChannel.h"         // content-nav request method (POST vs GET re-drive)
#include "nsISSLStatus.h"
#include "nsIBadCertListener2.h"
#include "nsIX509Cert.h"
#include "nsICertOverrideService.h"
#include "nsIDOMClientRect.h"
#include "nsWeakReference.h"
#include "nsIWeakReferenceUtils.h"
#include "nsThreadUtils.h"
#include "nsIThread.h"
#include "nsIDOMWindowUtils.h"
#include "mozIDOMWindow.h"
#include "nsIInterfaceRequestor.h"
#include "nsIDocShell.h"
#include "nsIPresShell.h"            // FlushPendingNotifications(Flush_Layout), ResizeReflow
#include "nsIContentViewer.h"
#include "nsIDOMDocument.h"          // document.readyState (load-complete fallback)
#include "nsIDOMNodeList.h"          // JihadTypingSelfTest: enumerate <input> elements
#include "nsIDOMWindow.h"            // content window -> document (GetTitle)
#include "nsIDOMElement.h"           // elementFromPoint (clickAt target hit-test)
#include "nsIDOMHTMLElement.h"       // DOMClick() — button / JS-onclick activation
#include "nsIDOMHTMLAnchorElement.h" // resolve <a href> at a tap -> direct navigation
#include "nsIDOMHTMLImageElement.h"  // hit-test: <img> src/alt for the adapter's HitTest JSON
#include "nsIDOMHTMLSelectElement.h" // <select> dropdown -> card-native popup (Atlas model)
#include "nsIDOMHTMLOptionElement.h"
#include "nsIDOMHTMLOptGroupElement.h" // a disabled <optgroup> disables its options (apply guard)
#include "nsIDOMHTMLOptionsCollection.h"
#include "nsIDOMNode.h"              // walk up to the nearest anchor ancestor
#include <cctype>                    // toupper/tolower for editable tag/type checks
#include "nsIDOMHTMLLabelElement.h"  // resolve a tapped <label> to its control (VKB, avoid focus-crash)
#include "nsIDOMHTMLInputElement.h"  // InsertText: type by DOM value mutation (no focus)
#include "nsIDOMHTMLTextAreaElement.h"
#include "nsIDOMHTMLButtonElement.h" // tapped <button type=submit> -> FireFormSubmit
#include "nsIDOMEventListener.h"     // engine-driven focus/blur -> VKB (Atlas IM-context port)
#include "nsIDOMEventTarget.h"
#include "nsIDOMEvent.h"
#include "nsIDOMHTMLFormElement.h"   // HandleEnter: submit the focused input's form
#include "nsIDOMEvent.h"             // dispatch 'input' so controlled/React fields see edits (F-238)
#include "nsIDOMEventTarget.h"
#include "nsIPrefBranch.h"
#include "nsICookieManager.h"
#include "nsICacheStorageService.h"
#include "nsServiceManagerUtils.h"
#include "nsIScriptSecurityManager.h" // DebugRunChromeJs: system principal for javascript: probes
#include "nsIPrincipal.h"
#include "JihadUserAgent.h"          // JIHAD_USER_AGENT (shared UA string)

namespace jihad {

// XPCOM chrome + progress listener for one page. Refcounted; owns the browser.
class PageChrome final : public nsIWebBrowserChrome,
                         public nsIEmbeddingSiteWindow,
                         public nsIInterfaceRequestor,
                         public nsIWebProgressListener,
                         public nsIBadCertListener2,
                         public nsIDOMEventListener,
                         public nsSupportsWeakReference
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIWEBBROWSERCHROME
  NS_DECL_NSIEMBEDDINGSITEWINDOW
  NS_DECL_NSIINTERFACEREQUESTOR
  NS_DECL_NSIWEBPROGRESSLISTENER
  NS_DECL_NSIBADCERTLISTENER2
  NS_DECL_NSIDOMEVENTLISTENER   // engine focus/blur -> VKB state (Atlas IM-context port)

  PageChrome(int w, int h) : mDone(false), mLoadFailed(false), mRedirected(false),
                             mCertError(false), mProgrammaticLoad(true),
                             mLinkClicked(false), mLinkIsPost(false), mErrorStatus(NS_OK), mCertPort(443),
                             mProgressPct(0), mEngineFocusIsText(false), mEngineFocusEvent(false),
                             mUserInteracted(false), mW(w), mH(h) {}
  bool mDone;
  bool mLoadFailed;          // last load ended in a network error
  bool mRedirected;          // the main document was redirected during this load
  bool mCertError;           // last load failed on an (overridable) cert error
  bool mProgrammaticLoad;    // current load was started by a command (not a link)
  bool mLinkClicked;         // a content-initiated (link) navigation was seen
  bool mLinkIsPost;          // ...and it was a non-GET (POST) request (not re-driven — F-262)
  nsCString mLinkUrl;        // its target URL
  nsresult mErrorStatus;     // the failing nsresult
  nsCString mFailedUrl;      // URL that failed
  nsCString mCertHost;       // host of the cert error
  int32_t mCertPort;         // port of the cert error
  int32_t mProgressPct;      // aggregate load progress 0..99 during a load (100 emitted on completion)
  nsCOMPtr<nsIX509Cert> mCertCert;   // the untrusted server cert (for override)
  nsCOMPtr<nsIDOMElement> mFocusedEditable;  // last-tapped editable, marked for InsertText DOM mutation
  nsCOMPtr<nsIDOMElement> mPendingInputEl;   // element that was edited + needs an 'input' event (F-266)
  // Engine-driven focus state (Atlas IM-context port): capture-phase focus/blur on the top
  // document land in HandleEvent, which records the ENGINE's view of whether a text control is
  // focused. GoannaRenderPage::PollEngineFocus merges this into the VKB state machine each pump
  // tick, so a page that moves/removes focus by SCRIPT (SPA login flows, blur() calls, focus
  // wedged after multi-site sessions — device T4) drives msgEditorFocused without a tap.
  nsCOMPtr<nsIDOMEventTarget> mFocusListenTarget;  // doc the focus/blur pair is registered on
  bool mEngineFocusIsText;    // engine focus currently on a text control / contentEditable
  bool mEngineFocusEvent;     // focus/blur seen since the last PollEngineFocus drain
  bool mUserInteracted;       // a tap happened since this page's load started (Atlas autofocus
                              // gate: a page that AUTOFOCUSES on load must not grab the VKB)
  int32_t mW, mH;
  nsCOMPtr<nsIWebBrowser> mBrowser;
private:
  ~PageChrome() {}
};

NS_IMPL_ISUPPORTS(PageChrome, nsIWebBrowserChrome, nsIEmbeddingSiteWindow,
                  nsIInterfaceRequestor, nsIWebProgressListener,
                  nsIBadCertListener2, nsIDOMEventListener, nsISupportsWeakReference)

// Cert error hook (R5): called during a bad TLS handshake with the SSL status
// (which carries the untrusted server cert) BEFORE the connection fails. We
// capture the cert + host/port and return false (don't silently proceed) so the
// error surfaces as an SSL-confirm; on accept, AcceptCurrentCert adds a validity
// override and a reload then handshakes clean (this hook isn't called again).
NS_IMETHODIMP PageChrome::NotifyCertProblem(nsIInterfaceRequestor*,
                                            nsISSLStatus* status,
                                            const nsACString& targetSite,
                                            bool* _retval) {
  if (_retval) *_retval = false;   // surface the error (reject by default)
  if (status) {
    nsCOMPtr<nsIX509Cert> cert;
    status->GetServerCert(getter_AddRefs(cert));
    if (cert) {
      mCertError = true;
      mCertCert = cert;
      // targetSite is "host:port".
      nsCString site(targetSite);
      int32_t colon = site.RFindChar(':');
      if (colon >= 0) {
        mCertHost = Substring(site, 0, colon);
        nsCString portStr(Substring(site, colon + 1));
        mCertPort = atoi(portStr.get());
      } else {
        mCertHost = site; mCertPort = 443;
      }
    }
  }
  return NS_OK;
}

NS_IMETHODIMP PageChrome::GetInterface(const nsIID& aIID, void** r) { return QueryInterface(aIID, r); }
NS_IMETHODIMP PageChrome::SetStatus(uint32_t, const char16_t*) { return NS_OK; }
NS_IMETHODIMP PageChrome::GetWebBrowser(nsIWebBrowser** b) { NS_IF_ADDREF(*b = mBrowser); return NS_OK; }
NS_IMETHODIMP PageChrome::SetWebBrowser(nsIWebBrowser* b) { mBrowser = b; return NS_OK; }
NS_IMETHODIMP PageChrome::GetChromeFlags(uint32_t* f) { *f = 0; return NS_OK; }
NS_IMETHODIMP PageChrome::SetChromeFlags(uint32_t) { return NS_OK; }
NS_IMETHODIMP PageChrome::DestroyBrowserWindow() { return NS_OK; }
NS_IMETHODIMP PageChrome::SizeBrowserTo(int32_t, int32_t) { return NS_OK; }
NS_IMETHODIMP PageChrome::ShowAsModal() { return NS_OK; }
NS_IMETHODIMP PageChrome::IsWindowModal(bool* r) { *r = false; return NS_OK; }
NS_IMETHODIMP PageChrome::ExitModalEventLoop(nsresult) { return NS_OK; }
NS_IMETHODIMP PageChrome::SetDimensions(uint32_t, int32_t, int32_t, int32_t, int32_t) { return NS_OK; }
NS_IMETHODIMP PageChrome::GetDimensions(uint32_t, int32_t* x, int32_t* y, int32_t* cx, int32_t* cy) {
  if (x) *x = 0; if (y) *y = 0; if (cx) *cx = mW; if (cy) *cy = mH; return NS_OK;
}
NS_IMETHODIMP PageChrome::SetFocus() { return NS_OK; }
NS_IMETHODIMP PageChrome::GetVisibility(bool* v) { *v = true; return NS_OK; }
NS_IMETHODIMP PageChrome::SetVisibility(bool) { return NS_OK; }
NS_IMETHODIMP PageChrome::GetTitle(char16_t** t) { *t = nullptr; return NS_OK; }
NS_IMETHODIMP PageChrome::SetTitle(const char16_t*) { return NS_OK; }
NS_IMETHODIMP PageChrome::GetSiteWindow(void** w) { *w = nullptr; return NS_OK; }
NS_IMETHODIMP PageChrome::Blur() { return NS_OK; }
// NS_BINDING_ABORTED — a cancelled navigation (e.g. navigating away / teardown),
// which is NOT a load error and can arrive late, bleeding into the next load.
static const nsresult kBindingAborted = (nsresult)0x804B0002;
// NS_BINDING_REDIRECTED — the document channel was replaced by a server 3xx redirect. This is
// NOT the end of the navigation (the redirect target keeps loading) and NOT a failure, so its
// STATE_STOP must not complete the load or reset mProgrammaticLoad — otherwise the redirect
// target's STATE_START is misread as a content-initiated link nav and re-driven via openUrl,
// which aborts the in-flight redirect and loops forever (breaks every http->https/www-redirecting
// site: google, iana.org, most https). It is NS_FAILED-category, so it must also be excluded from
// the failed-load check below.
static const nsresult kBindingRedirected = (nsresult)0x804B0003;

NS_IMETHODIMP PageChrome::OnStateChange(nsIWebProgress* aWebProgress, nsIRequest* aRequest, uint32_t f, nsresult aStatus) {
  // Only the TOP-LEVEL document drives page-level events (Codex P1): an iframe's
  // document load must not set the page's redirected/cert/failed/link state.
  bool top = false;
  if (aWebProgress && mBrowser) {
    nsCOMPtr<mozIDOMWindowProxy> pw; aWebProgress->GetDOMWindow(getter_AddRefs(pw));
    nsCOMPtr<mozIDOMWindowProxy> cw; mBrowser->GetContentDOMWindow(getter_AddRefs(cw));
    top = pw && cw && pw == cw;
  }
  // A redirect of the main document (R4 url-redirected). STATE_REDIRECTING fires
  // on the document request before the new location is followed.
  if (top && (f & STATE_REDIRECTING) && (f & STATE_IS_DOCUMENT)) mRedirected = true;
  // Link-clicked (R6): a top-level document load that STARTs while we did not
  // initiate it via a command (BeginLoad sets mProgrammaticLoad) is a
  // content-initiated (link/anchor) navigation. Capture its target.
  if (top && (f & STATE_START) && (f & STATE_IS_DOCUMENT) && !mProgrammaticLoad) {
    mLinkClicked = true;
    mLinkIsPost = false;
    // Reset the per-load completion/failure state HERE, synchronously with the content nav's real
    // START, so mDone tracks THIS navigation rather than the previous load. Without this, a fast POST
    // that also STOPs before the daemon adopts it (STATE_START is delivered asynchronously while
    // PumpFor runs) would be indistinguishable — mDone would still read the previous load's value, and
    // the daemon would either complete prematurely or reset an already-finished load and stall it to
    // the watchdog (Codex F-332). The daemon's AdoptContentLoad no longer touches mDone; it only marks
    // the nav programmatic while it is still in flight.
    mDone = false; mLoadFailed = false; mRedirected = false; mCertError = false;
    mErrorStatus = NS_OK; mProgressPct = 0;
    mUserInteracted = false;   // new page (content nav): autofocus must not grab the VKB
    // Release the outgoing document (content nav): the focus-listener pair pins it otherwise
    // (see BeginLoad — same inspector P3). Progress-callback context: listener bookkeeping only.
    if (mFocusListenTarget) {
      mFocusListenTarget->RemoveEventListener(NS_LITERAL_STRING("focus"), this, true);
      mFocusListenTarget->RemoveEventListener(NS_LITERAL_STRING("blur"), this, true);
      mFocusListenTarget = nullptr;
    }
    nsCOMPtr<nsIChannel> ch = do_QueryInterface(aRequest);
    if (ch) {
      nsCOMPtr<nsIURI> u; ch->GetURI(getter_AddRefs(u)); if (u) u->GetSpec(mLinkUrl);
      // Capture the method: a POST content-nav is NOT re-driven (the body may already be on the
      // wire — re-issuing would double the request, Codex F-262); only GET-class navs get the
      // re-drive. The engine completes POSTs; the load watchdog clears the overlay if one stalls.
      nsCOMPtr<nsIHttpChannel> http = do_QueryInterface(ch);
      if (http) { nsAutoCString m; http->GetRequestMethod(m); mLinkIsPost = !m.EqualsLiteral("GET"); }
    }
  }
  if (top && (f & (STATE_START | STATE_STOP))) {
    fprintf(stderr, "[jihad-bs] state top f=0x%x doc=%d win=%d net=%d %s status=0x%x\n",
            f, !!(f & STATE_IS_DOCUMENT), !!(f & STATE_IS_WINDOW), !!(f & STATE_IS_NETWORK),
            (f & STATE_START) ? "START" : "STOP", (unsigned)aStatus);
  }
  if (f & STATE_STOP) {
    // Failed-load (R3, non-cert): scope to the main document so a broken
    // subresource doesn't mark the whole page failed; ignore NS_BINDING_ABORTED
    // (a cancelled nav, can arrive late). First failure only. (Cert errors are
    // captured separately in NotifyCertProblem during the handshake, R5.)
    if (top && NS_FAILED(aStatus) && aStatus != kBindingAborted && aStatus != kBindingRedirected &&
        (f & STATE_IS_DOCUMENT) && !mLoadFailed && !mDone) {
      mLoadFailed = true;
      mErrorStatus = aStatus;
      nsCOMPtr<nsIChannel> ch = do_QueryInterface(aRequest);
      if (ch) {
        nsCOMPtr<nsIURI> u; ch->GetURI(getter_AddRefs(u));
        if (u) {
          u->GetSpec(mFailedUrl);
          // A security-module failure on the document is surfaced as an
          // SSL-confirm (R5): nsresult module == NS_ERROR_MODULE_SECURITY(21) +
          // offset(0x45). NOTE (Codex P1): this also catches non-overridable
          // security failures; that is acceptable because rejecting aborts the
          // load either way, and accept (the override) is device-gated. The code
          // is carried so the adapter can distinguish specific errors.
          if ((((uint32_t)aStatus >> 16) & 0x7fff) == (21u + 0x45u)) {
            mCertError = true;
            u->GetHost(mCertHost);
            int32_t p = -1; u->GetPort(&p); mCertPort = (p < 0) ? 443 : p;
          }
        }
      }
    }
    // Complete the load on the top-level DOCUMENT stop as well as window/network.
    // Waiting ONLY for STATE_IS_NETWORK means a single hanging subresource (an ad,
    // tracker, analytics beacon, or slow image that real sites are full of) keeps the
    // page "loading" forever: the isis UI's loading overlay never clears (it covers the
    // whole UI), the address-bar X never turns into refresh, and the card looks hung.
    // The top document STOP means the page itself is loaded and usable — signal done.
    // A redirect hop (NS_BINDING_REDIRECTED) or a cancelled load (NS_BINDING_ABORTED) is NOT the
    // end of the navigation — do NOT complete or clear mProgrammaticLoad on it, or the redirect
    // target's STATE_START is misread as a link click and re-driven into an abort/redirect loop.
    if (aStatus != kBindingRedirected && aStatus != kBindingAborted &&
        ((f & (STATE_IS_WINDOW | STATE_IS_NETWORK)) || (top && (f & STATE_IS_DOCUMENT)))) {
      mDone = true;
      // The command-initiated load has finished; any load that starts next
      // without a BeginLoad is a content-initiated (link) navigation.
      mProgrammaticLoad = false;
    }
  }
  return NS_OK;
}
// Aggregate load progress across the load group (bytes). Fed to the isis address-bar progress bar so
// a slow 512 MB-device load visibly advances instead of sitting at 0% — a frozen full-width bar reads
// as a crashed "loading screen" (the user's #1 complaint). Held to 1..99 here; 100 is emitted only on
// real completion (STATE_STOP) so the bar can't finish early. maxTotal is -1 when the total is unknown
// (no Content-Length) — keep the last value rather than snapping the bar around.
NS_IMETHODIMP PageChrome::OnProgressChange(nsIWebProgress*, nsIRequest*, int32_t, int32_t,
                                           int32_t curTotal, int32_t maxTotal) {
  if (maxTotal > 0 && curTotal >= 0) {
    int64_t pct = (int64_t)curTotal * 100 / maxTotal;
    if (pct < 1) pct = 1;
    if (pct > 99) pct = 99;
    if ((int32_t)pct > mProgressPct) mProgressPct = (int32_t)pct;   // monotonic (isis ignores decreases)
  }
  return NS_OK;
}
NS_IMETHODIMP PageChrome::OnLocationChange(nsIWebProgress*, nsIRequest*, nsIURI*, uint32_t) { return NS_OK; }
NS_IMETHODIMP PageChrome::OnStatusChange(nsIWebProgress*, nsIRequest*, nsresult, const char16_t*) { return NS_OK; }
NS_IMETHODIMP PageChrome::OnSecurityChange(nsIWebProgress*, nsIRequest*, uint32_t) { return NS_OK; }

// ── GoannaRenderPage ────────────────────────────────────────────────────────

// Offscreen render entry points exported from libxul (patches/0005). Let the
// frozen-API daemon create a memory-backed PuppetWidget (no X/GTK/native window),
// hand it to nsIBaseWindow::InitWindow, drive a paint, and read the ARGB32 pixels.
// Active when the env var JIHAD_OFFSCREEN is set (also switches PuppetWidget to a
// BasicLayerManager). This is the path the on-device daemon uses.
// (nsIWidget is forward-declared at global scope in GoannaRenderPage.h.)
extern "C" {
  nsIWidget* jihad_offscreen_create(int aWidth, int aHeight);
  void jihad_offscreen_resize(nsIWidget* aWidget, int aWidth, int aHeight);
  void jihad_offscreen_paint(nsIWidget* aWidget);
  bool jihad_init_nss();   // force PSM/NSS init on the main thread (internal libxul code)
  bool jihad_offscreen_render_document_v2(nsIWidget* aWidget, nsIDocShell* aDocShell, double aZoom,
                                          double aPanX, double aPanY);
  // WEAK: on a pre-overscan libxul the symbol is absent and RenderRegion degrades to the
  // viewport-exact paint (the caller's fallback) instead of failing the daemon at load.
  // New-symbol-weak is safe (no signature change on an existing symbol — the F1 ABI lesson
  // applies to CHANGED signatures, which still get a rename).
  bool jihad_offscreen_render_region(nsIWidget* aWidget, nsIDocShell* aDocShell, double aZoom,
                                     double aDocX, double aDocY, void* aDest, int aStride,
                                     int aWidth, int aHeight) __attribute__((weak));
  bool jihad_offscreen_readback(nsIWidget* aWidget, void* aDest, int aStride,
                                int aWidth, int aHeight);
  // WEAK, same reasoning: composite any OPEN XUL popup (menupopup / context menu) over
  // an already-painted buffer. A popup is a separate display root, so the main render
  // never contains it; absent the symbol, popups simply do not appear (today's behaviour)
  // rather than the daemon failing to load. Returns the number of popups drawn.
  int jihad_offscreen_composite_popups(nsIWidget* aWidget, void* aDest, int aStride,
                                       int aWidth, int aHeight, double aOriginX,
                                       double aOriginY, double aZoom) __attribute__((weak));
  // WEAK: dispatch a mouse event into an OPEN popup, whose separate display root the
  // content document's hit-testing cannot see. True = the point was inside a popup and
  // the event went there instead. Absent the symbol, taps behave as before (they land on
  // whatever is under the popup).
  bool jihad_offscreen_popup_mouse(int aMsg, double aX, double aY) __attribute__((weak));
  void jihad_offscreen_release(nsIWidget* aWidget);
  // Sticky content-invalidation drain (patches/0012). WEAK so the daemon still
  // loads against a pre-0012 libxul (then reports no dirty and paint falls back
  // to the input/load-driven behavior instead of failing to start).
  bool jihad_offscreen_take_dirty(nsIWidget* aWidget) __attribute__((weak));
}

// DEBUG ONLY: last-created page, target of DebugRunChromeJs (single-page embedding;
// the inject channel that calls it is compile-gated and off by default).
static GoannaRenderPage* sDebugLastPage = nullptr;

GoannaRenderPage::GoannaRenderPage(EngineHost& host)
  : mHost(host), mChrome(nullptr), mWindow(nullptr), mWidget(nullptr),
    mOffscreen(false), mWidth(0), mHeight(0),
    mEditorFocused(false), mEditorFocusDirty(false), mEditorFieldType(0) {}

GoannaRenderPage::~GoannaRenderPage() {
  SetSelectPopupEl(nullptr);   // release the held <select> before the engine tears down
  // Ordered teardown (Codex P1): stop navigation, remove the progress listener,
  // clear the container window, destroy the browser + base window, then release
  // the chrome and the native GTK window. All browser refs must be gone before
  // the engine's XRE_TermEmbedding runs.
  nsCOMPtr<nsIWebBrowser> wb = mChrome ? mChrome->mBrowser : nullptr;
  // Drop the engine focus/blur listener pair before the browser goes away — a focus event
  // delivered into a half-destroyed chrome would be a use-after-free (same ordered-teardown
  // rule as the progress listener below).
  if (mChrome && mChrome->mFocusListenTarget) {
    mChrome->mFocusListenTarget->RemoveEventListener(NS_LITERAL_STRING("focus"), mChrome, true);
    mChrome->mFocusListenTarget->RemoveEventListener(NS_LITERAL_STRING("blur"), mChrome, true);
    mChrome->mFocusListenTarget = nullptr;
  }
  if (wb) {
    nsCOMPtr<nsIWebNavigation> nav = do_QueryInterface(wb);
    if (nav) nav->Stop(nsIWebNavigation::STOP_ALL);
    nsCOMPtr<nsIWeakReference> weak =
      do_GetWeakReference(static_cast<nsIWebBrowserChrome*>(mChrome));
    if (weak) wb->RemoveWebBrowserListener(weak, NS_GET_IID(nsIWebProgressListener));
    wb->SetContainerWindow(nullptr);
    nsCOMPtr<nsIBaseWindow> bw = do_QueryInterface(wb);
    if (bw) bw->Destroy();
    mChrome->mBrowser = nullptr;
  }
  if (mChrome) { NS_RELEASE(mChrome); }   // release our ref
#ifndef JIHAD_OFFSCREEN_ONLY
  if (mWindow) { gtk_widget_destroy(mWindow); mWindow = nullptr; }
#endif
  if (mWidget) { jihad_offscreen_release(mWidget); mWidget = nullptr; }
  if (sDebugLastPage == this) sDebugLastPage = nullptr;
}

bool GoannaRenderPage::Create(int width, int height) {
  mWidth = width; mHeight = height;
  mOffscreen = (getenv("JIHAD_OFFSCREEN") != nullptr);
#ifdef JIHAD_OFFSCREEN_ONLY
  mOffscreen = true;   // device build: no GTK on-screen fallback exists
#endif
  RefPtr<PageChrome> chrome = new PageChrome(width, height);

  nsCOMPtr<nsIWebBrowser> wb = mHost.CreateBrowser();
  if (!wb) return false;
  chrome->mBrowser = wb;
  wb->SetContainerWindow(static_cast<nsIWebBrowserChrome*>(chrome));

  nsCOMPtr<nsIBaseWindow> baseWin = do_QueryInterface(wb);
  if (!baseWin) return false;

  if (mOffscreen) {
    // Headless: a PuppetWidget backed by an in-memory DrawTarget. Passed as the
    // parent nsIWidget (2nd arg) so nsWebBrowser uses it directly as the docShell
    // widget — no native/GTK child widget is created.
    mWidget = jihad_offscreen_create(width, height);
    if (!mWidget) return false;
    if (NS_FAILED(baseWin->InitWindow(nullptr, mWidget, 0, 0, width, height))) return false;
  } else {
#ifndef JIHAD_OFFSCREEN_ONLY
    mWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(mWindow), width, height);
    gtk_widget_realize(mWindow);
    if (NS_FAILED(baseWin->InitWindow(mWindow, nullptr, 0, 0, width, height))) return false;
#else
    return false;   // unreachable: mOffscreen forced true in device build
#endif
  }
  baseWin->Create();
  baseWin->SetVisibility(true);
#ifndef JIHAD_OFFSCREEN_ONLY
  if (!mOffscreen) {
    gtk_widget_show_all(mWindow);
  }
#endif
  JIHAD_GTK_PUMP();

  nsCOMPtr<nsIWeakReference> weak = do_GetWeakReference(static_cast<nsIWebBrowserChrome*>(chrome));
  wb->AddWebBrowserListener(weak, NS_GET_IID(nsIWebProgressListener));

  mChrome = chrome.forget().take();   // GoannaRenderPage keeps a strong ref
  sDebugLastPage = this;              // DEBUG: DebugRunChromeJs target (inject channel)
  return true;
}

nsIWebBrowser* GoannaRenderPage::DebugWebBrowser() const {
  return mChrome ? mChrome->mBrowser.get() : nullptr;
}

// DEBUG ONLY — see the header comment. javascript: URL with the SYSTEM principal so it
// executes inside privileged chrome documents. LoadURIWithOptions is the only frozen-API
// entry that accepts a triggering principal; a void() expression does not navigate.
bool DebugRunChromeJs(const char* jsUrl) {
  GoannaRenderPage* page = sDebugLastPage;
  if (!page || !jsUrl) return false;
  nsIWebBrowser* wb = page->DebugWebBrowser();
  if (!wb) return false;
  nsCOMPtr<nsIWebNavigation> nav = do_QueryInterface(wb);
  if (!nav) return false;
  nsCOMPtr<nsIScriptSecurityManager> ssm =
    do_GetService("@mozilla.org/scriptsecuritymanager;1");
  if (!ssm) return false;
  nsCOMPtr<nsIPrincipal> system;
  if (NS_FAILED(ssm->GetSystemPrincipal(getter_AddRefs(system))) || !system) return false;
  NS_ConvertUTF8toUTF16 u(jsUrl);
  return NS_SUCCEEDED(nav->LoadURIWithOptions(u.get(), nsIWebNavigation::LOAD_FLAGS_NONE,
                                              nullptr, 0, nullptr, nullptr, nullptr,
                                              system));
}

// DEBUG ONLY — see the header comment.
std::string DebugGetTitle() {
  GoannaRenderPage* page = sDebugLastPage;
  return page ? page->GetTitle() : std::string();
}


// Forward declarations (defined below) so Resize/SetZoom can resolve the content docShell.
static already_AddRefed<nsIDocShell> GetDocShell(nsIWebBrowser* wb);
static already_AddRefed<nsIDOMWindowUtils> GetWindowUtils(nsIWebBrowser* wb);

std::string DebugElementRect(const char* elementId) {
  GoannaRenderPage* page = sDebugLastPage;
  if (!page || !elementId) return std::string();
  nsIWebBrowser* wb = page->DebugWebBrowser();
  if (!wb) return std::string();
  nsCOMPtr<nsIDocShell> ds = GetDocShell(wb);
  if (!ds) return std::string();
  nsCOMPtr<nsIContentViewer> cv; ds->GetContentViewer(getter_AddRefs(cv));
  if (!cv) return std::string();
  nsCOMPtr<nsIDOMDocument> doc; cv->GetDOMDocument(getter_AddRefs(doc));
  if (!doc) return std::string();
  nsCOMPtr<nsIDOMElement> el;
  doc->GetElementById(NS_ConvertUTF8toUTF16(elementId), getter_AddRefs(el));
  if (!el) return std::string("(no element)");
  nsCOMPtr<nsIDOMClientRect> r;
  el->GetBoundingClientRect(getter_AddRefs(r));
  if (!r) return std::string("(no rect)");
  float x = 0, y = 0, w = 0, h = 0;
  r->GetLeft(&x); r->GetTop(&y); r->GetWidth(&w); r->GetHeight(&h);
  char buf[160];
  snprintf(buf, sizeof buf, "%s %d,%d %dx%d center=%d,%d", elementId,
           (int)(x + 0.5f), (int)(y + 0.5f), (int)(w + 0.5f), (int)(h + 0.5f),
           (int)(x + w / 2 + 0.5f), (int)(y + h / 2 + 0.5f));
  return std::string(buf);
}

bool GoannaRenderPage::Resize(int width, int height) {
  if (!mChrome) return false;
  if ((mOffscreen && !mWidget) || (!mOffscreen && !mWindow)) return false;
  if (width <= 0 || height <= 0 || width > 8192 || height > 8192) return false;
  mWidth = width; mHeight = height;
  mChrome->mW = width; mChrome->mH = height;   // keep GetDimensions() consistent
  if (mOffscreen) {
    jihad_offscreen_resize(mWidget, width, height);
  } else {
#ifndef JIHAD_OFFSCREEN_ONLY
    gtk_widget_set_size_request(mWindow, width, height);
    gtk_window_resize(GTK_WINDOW(mWindow), width, height);
#endif
  }
  // Resize the embedded surface; eRepaint invalidates so the page reflows to the
  // new viewport (100vw/100vh content follows the new size).
  nsCOMPtr<nsIBaseWindow> bw = do_QueryInterface(mChrome->mBrowser);
  if (bw) bw->SetPositionAndSize(0, 0, width, height, nsIBaseWindow::eRepaint);
  // Force the document to actually REFLOW at the new size. In this headless embedding
  // the widget/baseWindow resize does not propagate to a presShell resize-reflow (there
  // is no nsView listener wired up), so after a rotate the page stayed laid out at the
  // initial (portrait) width: GetContentSize kept returning 768 while the window was
  // 1024, and the adapter zoom-fit to 1024/768 = 1.333 and garbled the composite.
  // ResizeReflowIgnoreOverride sets the presContext visible area to the new size so the
  // content (and a width=device-width viewport) tracks the window in both orientations.
  nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
  if (ds) {
    nsCOMPtr<nsIPresShell> ps = ds->GetPresShell();
    if (ps) {
      // ResizeReflow wants app units. AppUnitsPerCSSPixel is the constant 60, and the
      // devicePixelRatio is pinned to 1 (layout.css.devPixelsPerPx), so 1 device px ==
      // 1 CSS px == 60 app units. (nsPresContext.h can't be included here -- it drags in
      // internal string headers unusable from the frozen-API daemon.)
      ps->ResizeReflowIgnoreOverride((nscoord)width * 60, (nscoord)height * 60, 0, 0);
    }
  }
  JIHAD_GTK_PUMP();
  return true;
}

static already_AddRefed<nsIDOMWindowUtils> GetWindowUtils(nsIWebBrowser* wb);
static already_AddRefed<nsIDocShell> GetDocShell(nsIWebBrowser* wb);

void GoannaRenderPage::ScrollTo(int x, int y) {
  if (!mChrome) return;
  // No scroll entry point in the frozen embedding API; a javascript: URL runs in
  // the page's context without navigating (window.scrollTo returns undefined, so
  // the load is not replaced). Requires JS enabled (the default).
  nsCOMPtr<nsIWebNavigation> nav = do_QueryInterface(mChrome->mBrowser);
  if (!nav) return;
  char js[96];
  snprintf(js, sizeof js, "javascript:void(window.scrollTo(%d,%d))", x, y);
  NS_ConvertUTF8toUTF16 u(js);
  mChrome->mProgrammaticLoad = true;   // internal nav: never a link-click (Codex P1)
  nav->LoadURI(u.get(), nsIWebNavigation::LOAD_FLAGS_NONE, nullptr, nullptr, nullptr);
}

bool GoannaRenderPage::Find(const char* text, bool forward) {
  if (!mChrome || !text) return false;
  nsCOMPtr<nsIInterfaceRequestor> ir = do_QueryInterface(mChrome->mBrowser);
  if (!ir) return false;
  nsCOMPtr<nsIWebBrowserFind> finder;
  ir->GetInterface(NS_GET_IID(nsIWebBrowserFind), getter_AddRefs(finder));
  if (!finder) return false;
  nsCOMPtr<nsIWebBrowserFocus> focus = do_QueryInterface(mChrome->mBrowser);
  if (focus) {
    focus->Activate();
    nsCOMPtr<mozIDOMWindowProxy> win;
    mChrome->mBrowser->GetContentDOMWindow(getter_AddRefs(win));
    if (win) focus->SetFocusedWindow(win);
  }
  NS_ConvertUTF8toUTF16 s(text);
  finder->SetSearchString(s.get());
  finder->SetFindBackwards(!forward);
  finder->SetSearchFrames(true);
  // KNOWN LIMITATION: nsIWebBrowserFind::FindNext faults in this offscreen
  // configuration (it dereferences a frame-selection controller the offscreen
  // browser doesn't fully set up). The BrowserServer daemon renders offscreen on
  // the device too, so we must NOT call FindNext here -- it would crash the
  // daemon. The finder is fully prepared (search string/direction set); wiring
  // an offscreen-safe selection controller so FindNext can run is future work.
  // The findString command stays dispatched (contract preserved) and safe.
  return false;
}

// --- caret-aware editing over the focused <input>/<textarea> --------------------------------
// The engine editor DOES work in the headless/offscreen build (validated on-device: Focus,
// GetValue/SetValue, GetSelectionStart/SetSelectionRange are all crash-free
// after UXP patch 0010). So we keep the ENGINE'S selection as the caret and edit the value
// around it, instead of the old append-only content-attribute mutation. Value get/set + caret
// get/set work for BOTH <input> and <textarea> (identical HTML API, different XPCOM interfaces).
static bool edGetValue(nsIDOMElement* el, nsAString& v) {
  nsCOMPtr<nsIDOMHTMLInputElement> in = do_QueryInterface(el);
  if (in) { in->GetValue(v); return true; }
  nsCOMPtr<nsIDOMHTMLTextAreaElement> ta = do_QueryInterface(el);
  if (ta) { ta->GetValue(v); return true; }
  return false;
}
static bool edSetValue(nsIDOMElement* el, const nsAString& v) {
  nsCOMPtr<nsIDOMHTMLInputElement> in = do_QueryInterface(el);
  if (in) { in->SetValue(v); return true; }
  nsCOMPtr<nsIDOMHTMLTextAreaElement> ta = do_QueryInterface(el);
  if (ta) { ta->SetValue(v); return true; }
  return false;
}
static bool edSetCaret(nsIDOMElement* el, int32_t pos) {
  nsCOMPtr<nsIDOMHTMLInputElement> in = do_QueryInterface(el);
  if (in) return NS_SUCCEEDED(in->SetSelectionRange(pos, pos, NS_LITERAL_STRING("")));
  nsCOMPtr<nsIDOMHTMLTextAreaElement> ta = do_QueryInterface(el);
  if (ta) return NS_SUCCEEDED(ta->SetSelectionRange(pos, pos, NS_LITERAL_STRING("")));
  return false;
}
// True for a TEXT-entry control: a <textarea>, or an <input> of a text-like type. Used to reject
// retargeting the edit target to a checkbox/radio/submit/button <input> (all nsIDOMHTMLInputElement)
// when a focus handler moved focus there (Codex F-270).
static bool edIsTextInput(nsIDOMElement* el) {
  if (!el) return false;
  nsCOMPtr<nsIDOMHTMLTextAreaElement> ta = do_QueryInterface(el);
  if (ta) return true;
  nsCOMPtr<nsIDOMHTMLInputElement> in = do_QueryInterface(el);
  if (!in) return false;
  nsAutoString ty; el->GetAttribute(NS_LITERAL_STRING("type"), ty);
  std::string t = NS_ConvertUTF16toUTF8(ty).get();
  for (char& c : t) c = (char)tolower((unsigned char)c);
  return t.empty() || t == "text" || t == "search" || t == "email" || t == "url" ||
         t == "tel" || t == "number" || t == "password";
}

// True for anything the VKB should serve: a text control or a contentEditable region.
static bool edIsEditable(nsIDOMElement* el) {
  if (edIsTextInput(el)) return true;
  nsCOMPtr<nsIDOMHTMLElement> h = do_QueryInterface(el);
  if (!h) return false;
  bool ce = false; h->GetIsContentEditable(&ce);
  return ce;
}

// Engine focus/blur observer (Atlas IM-context port). Runs SYNCHRONOUSLY inside engine event
// dispatch (during PumpFor), so it must only flip flags/swap COMPtrs — no engine calls, no
// navigation (the F-219 crash class). The VKB emission happens later, from PollEngineFocus in
// the guarded pump. Registered capture-phase on the TOP document only (RegisterEngineFocusListener);
// iframe fields don't propagate here — the tap heuristic still covers those.
NS_IMETHODIMP PageChrome::HandleEvent(nsIDOMEvent* aEvent) {
  if (!aEvent) return NS_OK;
  nsAutoString type; aEvent->GetType(type);
  nsCOMPtr<nsIDOMEventTarget> t; aEvent->GetTarget(getter_AddRefs(t));
  nsCOMPtr<nsIDOMElement> el = do_QueryInterface(t);
  if (!el) return NS_OK;   // window/document focus transitions — not a field
  if (type.EqualsLiteral("focus")) {
    bool text = edIsEditable(el);
    mEngineFocusIsText = text;
    mEngineFocusEvent = true;
    // Track the ENGINE's focused field as the type target so keystrokes follow script-driven
    // focus moves (login flows that swap fields), not just taps — but only once the user has
    // interacted with the page. Gating the RETARGET on the same mUserInteracted flag as the VKB
    // raise keeps key routing consistent with it (inspector P3): a load-time autofocus neither
    // raises the keyboard NOR captures the raw-key path (KeyEvent falls through to SendKeyEvent
    // exactly as before this feature — matters for desktop/synthetic-key callers).
    mFocusedEditable = (text && mUserInteracted) ? el : nullptr;
  } else if (type.EqualsLiteral("blur")) {
    // Only a blur OF the tracked field lowers the VKB — a blur elsewhere (a button losing
    // focus as the user taps the field) must not clobber the just-set focus state.
    if (mFocusedEditable && el == mFocusedEditable) {
      mEngineFocusIsText = false;
      mEngineFocusEvent = true;
      mFocusedEditable = nullptr;
    }
  }
  return NS_OK;
}

// Read the selection [start,end], normalized (start<=end) and clamped to the value length. Returns
// false when the element exposes no text-selection API — notably <input type=number>/email/etc.,
// whose selectionStart is null (Codex F-220); callers then fall back to an append-at-end edit.
static bool edGetSelection(nsIDOMElement* el, const nsAString& v, int32_t* s, int32_t* e) {
  int32_t a = 0, b = 0; bool ok = false;
  nsCOMPtr<nsIDOMHTMLInputElement> in = do_QueryInterface(el);
  if (in) ok = NS_SUCCEEDED(in->GetSelectionStart(&a)) && NS_SUCCEEDED(in->GetSelectionEnd(&b));
  else { nsCOMPtr<nsIDOMHTMLTextAreaElement> ta = do_QueryInterface(el);
         if (ta) ok = NS_SUCCEEDED(ta->GetSelectionStart(&a)) && NS_SUCCEEDED(ta->GetSelectionEnd(&b)); }
  if (!ok) return false;
  int32_t len = (int32_t)v.Length();
  if (a < 0) a = 0; if (a > len) a = len;
  if (b < 0) b = 0; if (b > len) b = len;
  if (a > b) { int32_t t = a; a = b; b = t; }
  *s = a; *e = b; return true;
}
// True when UTF-16 index i is the LOW half of a surrogate pair whose HIGH half is at i-1, so a
// caret step / delete spanning it must move by two units to stay on a code-point boundary.
static bool edIsPairBoundaryLow(const nsAString& v, int32_t i) {
  if (i <= 0 || i >= (int32_t)v.Length()) return false;
  char16_t lo = v.CharAt(i), hi = v.CharAt(i - 1);
  return lo >= 0xDC00 && lo <= 0xDFFF && hi >= 0xD800 && hi <= 0xDBFF;
}

// Dispatch a bubbling DOM 'input' event on the focused editable if a value edit is pending (see the
// header). Called only from the guarded pump loop (BrowserPageGoanna::pump), never from keyDown.
void GoannaRenderPage::FlushPendingInputEvent() {
  if (!mChrome || !mChrome->mPendingInputEl) return;
  nsCOMPtr<nsIDOMElement> el = mChrome->mPendingInputEl;   // the element that was EDITED (F-266)...
  mChrome->mPendingInputEl = nullptr;                      // ...not necessarily the current focus
  nsCOMPtr<nsIDOMNode> node = do_QueryInterface(el);
  if (!node) return;
  nsCOMPtr<nsIDOMDocument> doc; node->GetOwnerDocument(getter_AddRefs(doc));
  if (!doc) return;
  nsCOMPtr<nsIDOMEvent> ev;
  doc->CreateEvent(NS_LITERAL_STRING("Event"), getter_AddRefs(ev));
  if (!ev) return;
  ev->InitEvent(NS_LITERAL_STRING("input"), true, false);   // bubbles, non-cancelable
  nsCOMPtr<nsIDOMEventTarget> tgt = do_QueryInterface(el);
  bool dummy = false; if (tgt) tgt->DispatchEvent(ev, &dummy);
}

void GoannaRenderPage::InsertText(const char* text) {
  if (!mChrome || !mChrome->mFocusedEditable || !text || !*text) return;
  mChrome->mPendingInputEl = mChrome->mFocusedEditable;   // fire 'input' from pump (F-238) on THIS field
  nsCOMPtr<nsIDOMElement> el = mChrome->mFocusedEditable;
  NS_ConvertUTF8toUTF16 t(text);
  nsAutoString v;
  if (edGetValue(el, v)) {
    int32_t s, e;
    if (edGetSelection(el, v, &s, &e)) {
      // Replace the selected range [s,e] (a collapsed caret has s==e) with the text, then put the
      // caret just past it. Replacing the selection is what makes onfocus=this.select() fields (e.g.
      // a search box) type correctly instead of appending to the selected text (Codex F-221).
      nsAutoString nv; nv.Append(Substring(v, 0, s)); nv.Append(t); nv.Append(Substring(v, e));
      edSetValue(el, nv);
      int32_t nc = s + (int32_t)t.Length();
      edSetCaret(el, nc);
      if (nc >= (int32_t)nv.Length()) el->SetScrollLeft(1 << 24);   // caret at end -> keep visible
    } else {
      // A value control with no selection API (<input type=number> etc., Codex F-220): append at
      // the end via the live value setter (SetTextContent would not update the control's value).
      // KNOWN LIMITATION (Codex F-242): <input type=number> sanitizes .value, so a temporarily-
      // invalid prefix (a lone leading "-", or "1." mid-decimal) is dropped — typing "-1" yields
      // "1". Faithfully supporting that needs the engine's own text-control editor buffer, which we
      // bypass; acceptable for now since numeric entry is normally left-to-right and valid.
      nsAutoString nv(v); nv.Append(t); edSetValue(el, nv); el->SetScrollLeft(1 << 24);
    }
    return;
  }
  // contentEditable / other: append via textContent (no value-based caret model here).
  nsCOMPtr<nsIDOMNode> node = do_QueryInterface(el);
  if (node) { nsAutoString cv; node->GetTextContent(cv); cv.Append(t); node->SetTextContent(cv); }
}

// Drop the last USER-PERCEIVED character: one UTF-16 code unit, or two if it is a surrogate pair
// (e.g. an emoji), so Backspace never leaves a malformed lone surrogate (Jihad review F-187).
static void jihadTrimLastChar(nsAutoString& v) {
  uint32_t len = v.Length();
  if (len == 0) return;
  uint32_t drop = 1;
  if (len >= 2) {
    char16_t lo = v.CharAt(len - 1), hi = v.CharAt(len - 2);
    if (lo >= 0xDC00 && lo <= 0xDFFF && hi >= 0xD800 && hi <= 0xDBFF) drop = 2;  // low+high surrogate
  }
  v.Truncate(len - drop);
}

// Backspace: delete the code point immediately BEFORE the engine caret (surrogate-aware), then
// keep the caret where the deleted text was. The VKB delivers Backspace as a keyDown.
void GoannaRenderPage::DeleteBackward() {
  if (!mChrome || !mChrome->mFocusedEditable) return;
  // NB: mark the pending 'input' event only when a value change ACTUALLY happens (F-267) — a
  // Backspace at position 0 / on an empty field mutates nothing and must not fire input.
  nsCOMPtr<nsIDOMElement> el = mChrome->mFocusedEditable;
  nsAutoString v;
  if (edGetValue(el, v)) {
    int32_t s, e;
    if (edGetSelection(el, v, &s, &e)) {
      if (s != e) {   // a range is selected: Backspace deletes the whole selection (Codex F-221)
        nsAutoString nv; nv.Append(Substring(v, 0, s)); nv.Append(Substring(v, e));
        edSetValue(el, nv); edSetCaret(el, s); mChrome->mPendingInputEl = el; return;
      }
      if (s == 0) return;   // nothing before the caret: no mutation, no input event
      int32_t drop = edIsPairBoundaryLow(v, s - 1) ? 2 : 1; if (drop > s) drop = s;
      nsAutoString nv; nv.Append(Substring(v, 0, s - drop)); nv.Append(Substring(v, s));
      edSetValue(el, nv); edSetCaret(el, s - drop); mChrome->mPendingInputEl = el; return;
    }
    if (!v.IsEmpty()) { jihadTrimLastChar(v); edSetValue(el, v); mChrome->mPendingInputEl = el; }
    return;
  }
  nsCOMPtr<nsIDOMNode> node = do_QueryInterface(el);   // contentEditable
  if (node) {
    nsAutoString cv; node->GetTextContent(cv);
    if (!cv.IsEmpty()) { jihadTrimLastChar(cv); node->SetTextContent(cv); mChrome->mPendingInputEl = el; }
  }
}

static bool edIsSpace(char16_t c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; }

// Accelerated Backspace: delete a whole word before the caret (the run of whitespace immediately
// before the caret, then the run of non-whitespace) — used when Backspace is held long enough to
// auto-repeat, so clearing a long field is quick. If a range is selected it deletes that instead.
void GoannaRenderPage::DeleteBackwardWord() {
  if (!mChrome || !mChrome->mFocusedEditable) return;
  nsCOMPtr<nsIDOMElement> el = mChrome->mFocusedEditable;
  nsAutoString v; int32_t s, e;
  if (edGetValue(el, v) && edGetSelection(el, v, &s, &e)) {
    if (s != e) {   // selection present: delete it
      nsAutoString nv; nv.Append(Substring(v, 0, s)); nv.Append(Substring(v, e));
      edSetValue(el, nv); edSetCaret(el, s); mChrome->mPendingInputEl = el; return;
    }
    if (s == 0) return;   // nothing before the caret: no mutation (F-267)
    int32_t i = s;
    while (i > 0 && edIsSpace(v.CharAt(i - 1))) i--;          // trailing whitespace
    while (i > 0 && !edIsSpace(v.CharAt(i - 1))) i--;         // the word itself
    if (i >= s) i = s - 1;                                    // guarantee progress
    if (edIsPairBoundaryLow(v, i)) i--;                       // never split a surrogate pair (F-187)
    nsAutoString nv; nv.Append(Substring(v, 0, i)); nv.Append(Substring(v, s));
    edSetValue(el, nv); edSetCaret(el, i); mChrome->mPendingInputEl = el;
    return;
  }
  DeleteBackward();   // number/contentEditable: no word model — remove one char
}

// Non-character editing keys, applied at the engine selection of the focused <input>/<textarea>.
// Arrows/Home/End move/collapse the caret; Delete removes the selection or the code point after the
// caret; Tab moves focus to the next/prev field. (Enter is handled by HandleEnter, deferred to the
// pump loop, because it may submit a form and navigate — Codex F-219/F-223.)
void GoannaRenderPage::EditKey(int action) {
  if (!mChrome || !mChrome->mFocusedEditable) return;
  nsCOMPtr<nsIDOMElement> el = mChrome->mFocusedEditable;
  nsAutoString v; int32_t s, e;
  if (!edGetValue(el, v) || !edGetSelection(el, v, &s, &e)) return;   // no caret model (number/CE)
  int32_t len = (int32_t)v.Length();
  const bool hasSel = (s != e);
  const int32_t c = e;   // caret reference = the focus (right) end of the selection
  switch (action) {
    case EK_LEFT:  if (hasSel) edSetCaret(el, s);
                   else if (c > 0)   edSetCaret(el, c - (edIsPairBoundaryLow(v, c - 1) ? 2 : 1)); break;
    case EK_RIGHT: if (hasSel) edSetCaret(el, e);
                   else if (c < len) edSetCaret(el, c + (edIsPairBoundaryLow(v, c + 1) ? 2 : 1)); break;
    case EK_HOME:  edSetCaret(el, 0);   break;
    case EK_END:   edSetCaret(el, len); break;
    case EK_UP: case EK_DOWN: {
      // Move to the same column on the adjacent line. For a single-line <input> (no '\n') this
      // degrades to Home (up) / End (down), which is the expected behaviour.
      int32_t lineStart = c; while (lineStart > 0 && v.CharAt(lineStart - 1) != '\n') lineStart--;
      int32_t col = c - lineStart, target;
      if (action == EK_UP) {
        if (lineStart == 0) { edSetCaret(el, 0); break; }
        int32_t prevEnd = lineStart - 1, prevStart = prevEnd;
        while (prevStart > 0 && v.CharAt(prevStart - 1) != '\n') prevStart--;
        int32_t prevLen = prevEnd - prevStart;
        target = prevStart + (col < prevLen ? col : prevLen);
      } else {
        int32_t nextStart = c; while (nextStart < len && v.CharAt(nextStart) != '\n') nextStart++;
        if (nextStart >= len) { edSetCaret(el, len); break; }
        nextStart++;   // skip the '\n'
        int32_t nextEnd = nextStart; while (nextEnd < len && v.CharAt(nextEnd) != '\n') nextEnd++;
        int32_t nextLen = nextEnd - nextStart;
        target = nextStart + (col < nextLen ? col : nextLen);
      }
      if (edIsPairBoundaryLow(v, target)) target--;   // never land between a surrogate pair (F-224)
      edSetCaret(el, target);
      break;
    }
    case EK_DELETE: {
      if (hasSel) {   // forward-Delete with a selection removes the selection
        nsAutoString nv; nv.Append(Substring(v, 0, s)); nv.Append(Substring(v, e));
        edSetValue(el, nv); edSetCaret(el, s); mChrome->mPendingInputEl = el; break;
      }
      if (c >= len) break;   // nothing after the caret: no mutation, no input event (F-267)
      int32_t drop = edIsPairBoundaryLow(v, c + 1) ? 2 : 1; if (c + drop > len) drop = len - c;
      nsAutoString nv; nv.Append(Substring(v, 0, c)); nv.Append(Substring(v, c + drop));
      edSetValue(el, nv); edSetCaret(el, c); mChrome->mPendingInputEl = el;
      break;
    }
    default: break;
  }
}

// Enter in the focused editable. A <textarea> inserts a newline; a single-line <input> performs
// implicit form submission — click the form's submit control (so onclick/onsubmit + validation run)
// or, if none, submit the form directly. This NAVIGATES, so it must only run from the guarded pump()
// loop (BrowserPageGoanna defers Enter there), never synchronously from the YAP key callback (F-219).
bool GoannaRenderPage::HandleEnter() {
  if (!mChrome || !mChrome->mFocusedEditable) return false;
  nsCOMPtr<nsIDOMElement> el = mChrome->mFocusedEditable;
  nsCOMPtr<nsIDOMHTMLTextAreaElement> ta = do_QueryInterface(el);
  if (ta) { InsertText("\n"); return false; }   // newline, not a submit
  nsCOMPtr<nsIDOMHTMLInputElement> in = do_QueryInterface(el);
  if (!in) return false;
  nsCOMPtr<nsIDOMHTMLFormElement> form; in->GetForm(getter_AddRefs(form));
  if (!form) return false;
  // Implicit form submission: validate (honoring novalidate), fire the cancelable 'submit' event so
  // onsubmit runs + can preventDefault, then submit — all crash-safe (FireFormSubmit). Clear the edit
  // target only if a submission was actually ISSUED (a nav is starting): a genuine submit must not be
  // re-run by a second queued Enter (F-264), while a blocked/invalid/cancelled submit keeps focus +
  // the VKB so the user can correct it (F-290). Returns true regardless — a submit was ATTEMPTED, so
  // the pump's Enter drain stops here.
  if (FireFormSubmit(form)) mChrome->mFocusedEditable = nullptr;
  return true;
}

// Tab in the focused editable. A <textarea> inserts a literal tab; a single-line <input> moves
// focus to the next/prev text field (the standard browser behaviour — inserting a tab into a
// search/login value is wrong). Runs from the guarded pump() loop because FocusNextField's Focus()
// fires page focus/blur JS that can navigate (F-219).
void GoannaRenderPage::HandleTab(bool backward) {
  if (!mChrome || !mChrome->mFocusedEditable) return;
  nsCOMPtr<nsIDOMHTMLTextAreaElement> ta = do_QueryInterface(mChrome->mFocusedEditable);
  if (ta) { InsertText("\t"); return; }
  FocusNextField(backward);
}

// Move focus to the next (or previous) text field and make it the type target. Enumerate the
// document's <input>/<textarea> in document order, skipping disabled/readonly (F-222).
void GoannaRenderPage::FocusNextField(bool backward) {
  if (!mChrome || !mChrome->mFocusedEditable) return;
  nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
  if (!ds) return;
  nsCOMPtr<nsIContentViewer> cv; ds->GetContentViewer(getter_AddRefs(cv));
  if (!cv) return;
  nsCOMPtr<nsIDOMDocument> doc; cv->GetDOMDocument(getter_AddRefs(doc));
  if (!doc) return;
  nsCOMPtr<nsIDOMNodeList> list;
  doc->QuerySelectorAll(NS_LITERAL_STRING(
      "input[type=text],input[type=search],input[type=email],input[type=url],input[type=tel],"
      "input[type=number],input[type=password],input:not([type]),textarea"),
      getter_AddRefs(list));
  uint32_t n = 0; if (list) list->GetLength(&n);
  if (!n) return;
  int32_t cur = -1;
  for (uint32_t i = 0; i < n; ++i) {
    nsCOMPtr<nsIDOMNode> node; list->Item(i, getter_AddRefs(node));
    nsCOMPtr<nsIDOMElement> e = do_QueryInterface(node);
    if (e == mChrome->mFocusedEditable) { cur = (int32_t)i; break; }
  }
  // Step to the next candidate, skipping disabled/readonly fields (F-222). If none of the other
  // fields are focusable, stay put rather than making a readonly field the type target.
  int32_t next = cur; bool found = false;
  for (uint32_t step = 0; step < n; ++step) {
    next = (next < 0) ? 0 : (backward ? (next - 1 + (int32_t)n) % n : (next + 1) % n);
    nsCOMPtr<nsIDOMNode> cand; list->Item((uint32_t)next, getter_AddRefs(cand));
    nsCOMPtr<nsIDOMElement> ce = do_QueryInterface(cand);
    if (!ce) continue;
    bool dis = false, ro = false;
    ce->HasAttribute(NS_LITERAL_STRING("disabled"), &dis);
    ce->HasAttribute(NS_LITERAL_STRING("readonly"), &ro);
    if (!dis && !ro) { found = true; break; }
  }
  if (!found) return;
  nsCOMPtr<nsIDOMNode> nnode; list->Item((uint32_t)next, getter_AddRefs(nnode));
  nsCOMPtr<nsIDOMElement> nel = do_QueryInterface(nnode);
  if (!nel) return;
  mChrome->mFocusedEditable = nel;
  nsCOMPtr<nsIDOMHTMLElement> he = do_QueryInterface(nel);
  if (he) he->Focus();
  ActivateEditorCaret();   // keep the caret painting on the newly-focused field
  // F-245 (same class as F-225 for taps): Focus() may fail (CSS-hidden field) or a focus handler
  // may redirect focus elsewhere. Reconcile the edit target with the element ACTUALLY focused: keep it
  // if it is another text control, but if focus landed on a NON-text control (a button/checkbox a
  // handler moved to), clear the target and stop — typing must not silently mutate the old, unfocused
  // field (Codex F-270/F-292).
  {
    nsCOMPtr<nsIFocusManager> fm = do_GetService("@mozilla.org/focus-manager;1");
    if (fm) {
      nsCOMPtr<nsIDOMElement> foc; fm->GetFocusedElement(getter_AddRefs(foc));
      if (foc && foc != nel) {
        if (edIsTextInput(foc)) { mChrome->mFocusedEditable = foc; nel = foc; }
        else {
          // Tab landed focus on a non-text control: clear the target AND lower the VKB (no editable to
          // type into) instead of leaving the keyboard raised over a dead target (Codex F-326).
          mChrome->mFocusedEditable = nullptr;
          mEditorFocused = false; mEditorFieldType = 0; mEditorFocusDirty = true;
          return;
        }
      }
    }
  }
  int32_t vlen = 0; { nsAutoString vv; if (edGetValue(nel, vv)) vlen = (int32_t)vv.Length(); }
  edSetCaret(nel, vlen);   // caret at end of the newly-focused field
}

// Make the editor caret actually PAINT. The offscreen embedding starts "deactivated", so even a
// focused editable draws no caret (the caret positions correctly — GetSelectionStart is right —
// but nsCaret is only painted for an active, focused window). Activate the browser + focus its
// content window, and disable blink so the caret is a SOLID bar (the offscreen refresh driver
// does not tick a blink reliably, which would otherwise leave the caret invisible most of the time).
void GoannaRenderPage::ActivateEditorCaret() {
  if (!mChrome) return;
  nsCOMPtr<nsIWebBrowserFocus> focus = do_QueryInterface(mChrome->mBrowser);
  if (focus) {
    focus->Activate();
    nsCOMPtr<mozIDOMWindowProxy> win;
    mChrome->mBrowser->GetContentDOMWindow(getter_AddRefs(win));
    if (win) focus->SetFocusedWindow(win);
  }
  nsCOMPtr<nsIPrefBranch> pb = do_GetService("@mozilla.org/preferences-service;1");
  if (pb) { pb->SetIntPref("ui.caretBlinkTime", 0); pb->SetIntPref("ui.caretWidth", 2); }
}

bool GoannaRenderPage::HasFocusedEditable() const {
  return mChrome && mChrome->mFocusedEditable;
}

bool GoannaRenderPage::GetScrollXY(int* x, int* y, bool flushLayout) {
  if (!mChrome) return false;
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return false;
  int32_t sx = 0, sy = 0;
  if (NS_FAILED(u->GetScrollXY(flushLayout, &sx, &sy))) return false;
  if (x) *x = sx; if (y) *y = sy;
  return true;
}

void GoannaRenderPage::SetZoom(double zoom) {
  if (!mChrome || zoom <= 0.0) return;
  nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
  if (!ds) return;
  // Pinch/fit zoom is a pure RENDER magnification applied in the offscreen capture:
  // JihadRenderDocument scales the gfxContext by mRenderZoom and renders the 1/zoom-reduced
  // visible region so it fills the device buffer. The engine layout viewport is LEFT at the
  // window width (device-width) — NO reflow, NO SetFullZoom, NO SetResolution — so content
  // size is stable and the old landscape 1.333 fit-zoom feedback loop cannot arm (keeps the
  // rotation fix intact). SetResolution/SetFullZoom were both wrong here: RenderDocument's
  // internal scale (AppUnitsPerDevPixel/AppUnitsPerCSSPixel) counteracts an engine zoom, so
  // they shrank the capture into a 1/zoom quadrant with the rest white ("things get cut off",
  // device test 2026-07-27). renderedX/Y + contentZoom keep the adapter composite at inv==1.
  (void)ds;
  mRenderZoom = (zoom > 0.0) ? zoom : 1.0;
  // Zoom doesn't resize the native window, so nudge a repaint to refresh readback.
  nsCOMPtr<nsIBaseWindow> bw = do_QueryInterface(mChrome->mBrowser);
  if (bw) bw->Repaint(true);
}

bool GoannaRenderPage::GetContentSize(int* w, int* h) {
  if (!mChrome) return false;
  // Flush layout FIRST so the size reflects the current viewport. emitGeometry() calls
  // this right after a Resize (rotation), before the async reflow runs; without the
  // flush GetRootBounds returns the stale pre-reflow width (e.g. 768 portrait). The
  // adapter divides that into the window width for zoom-to-fit, so a stale 768 under a
  // 1024 landscape window yields zoom 1024/768 = 1.333 -> SetFullZoom upscales and the
  // page renders larger than the buffer (tiling/garbled). Flushing makes the width
  // accurate (1024), so the adapter computes zoom 1.0.
  {
    nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
    if (ds) {
      nsCOMPtr<nsIPresShell> ps = ds->GetPresShell();
      if (ps) ps->FlushPendingNotifications(Flush_Layout);
    }
  }
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return false;
  // GetRootBounds returns the root scroll frame's bounds, i.e. the full scrollable
  // document rect (Codex P2): geo_test confirms a 2500px-tall page reports 2500,
  // not the 768px viewport -- so this is the content size, not just the viewport.
  nsCOMPtr<nsIDOMClientRect> r;
  if (NS_FAILED(u->GetRootBounds(getter_AddRefs(r))) || !r) return false;
  float fw = 0.0f, fh = 0.0f;
  r->GetWidth(&fw); r->GetHeight(&fh);
  if (w) *w = (int)(fw + 0.5f);
  if (h) *h = (int)(fh + 0.5f);
  return true;
}

bool GoannaRenderPage::GetViewport(double* initialScale, double* minScale,
                                   double* maxScale, int* w, int* h,
                                   bool* userScalable) {
  if (!mChrome) return false;
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return false;
  double dz = 1.0, mn = 1.0, mx = 1.0;
  bool allow = true, autoSize = true;
  uint32_t vw = 0, vh = 0;
  if (NS_FAILED(u->GetViewportInfo((uint32_t)mWidth, (uint32_t)mHeight,
                                   &dz, &allow, &mn, &mx, &vw, &vh, &autoSize)))
    return false;
  if (initialScale) *initialScale = dz;
  if (minScale) *minScale = mn;
  if (maxScale) *maxScale = mx;
  if (w) *w = (int)vw;
  if (h) *h = (int)vh;
  if (userScalable) *userScalable = allow;
  return true;
}

// Reset per-load state before any navigation so a prior load's completion/failure
// never bleeds into the next one (Codex P1). Called from every nav entry point.
void GoannaRenderPage::BeginLoad() {
  if (!mChrome) return;
  mChrome->mDone = false;
  mChrome->mLoadFailed = false;
  mChrome->mRedirected = false;
  mChrome->mCertError = false;
  mChrome->mProgrammaticLoad = true;   // this load is command-initiated, not a link
  mChrome->mProgressPct = 0;           // fresh progress for the new load (isis resets its bar too)
  mChrome->mUserInteracted = false;    // Atlas autofocus gate: new page, no tap yet
  // A navigation drops any pending <select> popup: its element belongs to the outgoing page.
  mSelectPopupPending = false; SetSelectPopupEl(nullptr);
  // Drop the focus listener NOW, not at the next completion — the listener pair holds a strong
  // ref to the OLD document (chrome -> doc -> listener -> chrome cycle), which would pin the
  // outgoing page's whole content tree for the duration of a slow load, exactly when a 512 MB
  // device is tightest (inspector P3). VKB lowering on nav is handled by ClearEditorFocus, not
  // by the dying page's blur.
  if (mChrome->mFocusListenTarget) {
    mChrome->mFocusListenTarget->RemoveEventListener(NS_LITERAL_STRING("focus"), mChrome, true);
    mChrome->mFocusListenTarget->RemoveEventListener(NS_LITERAL_STRING("blur"), mChrome, true);
    mChrome->mFocusListenTarget = nullptr;
  }
  mChrome->mErrorStatus = NS_OK;
  mChrome->mFailedUrl.Truncate();
  mChrome->mCertHost.Truncate();
  mChrome->mCertCert = nullptr;
  mChrome->mFocusedEditable = nullptr;   // the marked field belongs to the old document
}

bool GoannaRenderPage::LoadUrl(const char* url) {
  if (!mChrome) return false;
  // Force PSM/NSS (TLS) to construct on THIS main thread before the load. Necko
  // otherwise lazily constructs nsNSSComponent from the socket-transport thread on the
  // first https:// request, and its ctor MOZ_RELEASE_ASSERT(NS_IsMainThread()) crashes
  // the daemon ("loads forever"). PSM isn't registered yet at engine-init time, but it
  // is by first navigation, so do it here (idempotent — returns the singleton after).
  jihad_init_nss();
  nsCOMPtr<nsIWebNavigation> nav = do_QueryInterface(mChrome->mBrowser);
  if (!nav) return false;
  // Pin the browser identity: a non-empty docShell customUserAgent is returned
  // directly by Navigator::GetUserAgent (ahead of nsHttpHandler), so navigator.userAgent
  // reflects JIHAD_USER_AGENT reliably. general.useragent.override alone did not stick
  // for navigator.userAgent under XRE_InitEmbedding (the loose goanna.js prefs file
  // isn't loaded by a bare embedder). Set per-load: the attribute lives on the docShell
  // and survives same-docShell navigations, but re-applying is cheap and covers a fresh
  // content window. Also drives the network User-Agent header for this docShell's loads.
  BeginLoad();
  // Scheme fixup: a bare host like "whatismybrowser.com" has no scheme, and nav->LoadURI
  // needs one or the load silently fails (no load-done, blank page). Prepend http:// unless
  // it already has a scheme or is an internal pseudo-scheme (about:/data:/etc). The app's
  // URL bar should do this too, but the daemon must be robust to a schemeless URL.
  std::string fixed(url ? url : "");
  if (!fixed.empty() && fixed.find("://") == std::string::npos &&
      fixed.compare(0, 6, "about:") != 0 && fixed.compare(0, 5, "data:") != 0 &&
      fixed.compare(0, 11, "javascript:") != 0 && fixed.compare(0, 7, "mailto:") != 0 &&
      fixed.compare(0, 5, "file:") != 0 && fixed.compare(0, 4, "tel:") != 0) {
    fixed = "http://" + fixed;
  }
  // Pin the browser identity via the docShell customUserAgent (returned by Navigator::GetUserAgent
  // ahead of nsHttpHandler, so navigator.userAgent is reliable). For a domain with a per-domain UA
  // override, use THAT UA so navigator.userAgent matches the request header the observer sends —
  // otherwise client-side UA gating (e.g. google's JS) sees a stale FF52 while the server saw FF71
  // (Codex F-240). Otherwise the standard JIHAD_USER_AGENT.
  {
    const char* domUa = JihadPerDomainUaForUrl(fixed.c_str());
    nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
    if (ds) ds->SetCustomUserAgent(NS_ConvertUTF8toUTF16(domUa ? domUa : JIHAD_USER_AGENT));
  }
  NS_ConvertUTF8toUTF16 u(fixed.c_str());
  return NS_SUCCEEDED(nav->LoadURI(u.get(), nsIWebNavigation::LOAD_FLAGS_NONE, nullptr, nullptr, nullptr));
}

void GoannaRenderPage::PumpFor(int msBudget) {
  nsCOMPtr<nsIThread> thread;
  NS_GetCurrentThread(getter_AddRefs(thread));
  struct timespec ts0; clock_gettime(CLOCK_MONOTONIC, &ts0);
  for (;;) {
    NS_ProcessNextEvent(thread, false);
    JIHAD_GTK_PUMP();
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    long ms = (ts.tv_sec - ts0.tv_sec) * 1000 + (ts.tv_nsec - ts0.tv_nsec) / 1000000;
    if (ms >= msBudget) break;
    g_usleep(1000);
  }
}

bool GoannaRenderPage::LoadUrlAndWait(const char* url, int timeoutSec) {
  if (!LoadUrl(url)) return false;
  nsCOMPtr<nsIThread> thread;
  NS_GetCurrentThread(getter_AddRefs(thread));
  time_t start = time(nullptr);
  while (!mChrome->mDone && (time(nullptr) - start) < timeoutSec) {
    NS_ProcessNextEvent(thread, false);
    JIHAD_GTK_PUMP();
    g_usleep(1000);
  }
  return mChrome->mDone;
}

bool GoannaRenderPage::SetHtml(const char* body) {
  if (!body) return false;
  std::string url = "data:text/html;charset=utf-8,";
  url += body;
  return LoadUrl(url.c_str());
}
bool GoannaRenderPage::CanGoBack() {
  if (!mChrome) return false;
  nsCOMPtr<nsIWebNavigation> n = do_QueryInterface(mChrome->mBrowser);
  bool v = false; if (n) n->GetCanGoBack(&v); return v;
}
bool GoannaRenderPage::CanGoForward() {
  if (!mChrome) return false;
  nsCOMPtr<nsIWebNavigation> n = do_QueryInterface(mChrome->mBrowser);
  bool v = false; if (n) n->GetCanGoForward(&v); return v;
}

void GoannaRenderPage::GoBack()    { if (mChrome) { BeginLoad(); nsCOMPtr<nsIWebNavigation> n = do_QueryInterface(mChrome->mBrowser); if (n) n->GoBack(); } }
void GoannaRenderPage::GoForward() { if (mChrome) { BeginLoad(); nsCOMPtr<nsIWebNavigation> n = do_QueryInterface(mChrome->mBrowser); if (n) n->GoForward(); } }
void GoannaRenderPage::Reload()    { if (mChrome) { BeginLoad(); nsCOMPtr<nsIWebNavigation> n = do_QueryInterface(mChrome->mBrowser); if (n) n->Reload(nsIWebNavigation::LOAD_FLAGS_NONE); } }
void GoannaRenderPage::Stop()      { if (mChrome) { nsCOMPtr<nsIWebNavigation> n = do_QueryInterface(mChrome->mBrowser); if (n) n->Stop(nsIWebNavigation::STOP_ALL); } }

void GoannaRenderPage::ClearHistory() {
  if (!mChrome) return;
  nsCOMPtr<nsIWebNavigation> nav = do_QueryInterface(mChrome->mBrowser);
  if (!nav) return;
  nsCOMPtr<nsISHistory> sh;
  nav->GetSessionHistory(getter_AddRefs(sh));
  if (!sh) return;
  int32_t count = 0;
  sh->GetCount(&count);
  if (count > 0) sh->PurgeHistory(count);
}

bool GoannaRenderPage::LoadDone() const { return mChrome && mChrome->mDone; }

int GoannaRenderPage::GetLoadProgress() const { return mChrome ? mChrome->mProgressPct : 0; }

bool GoannaRenderPage::TakeDirty() {
  if (!mOffscreen || !mWidget || !jihad_offscreen_take_dirty) return false;
  return jihad_offscreen_take_dirty(mWidget);
}

void GoannaRenderPage::AdoptContentLoad() {
  if (!mChrome) return;
  // OnStateChange already reset the per-load state (mDone/failure/progress) at this nav's STATE_START,
  // so mDone accurately reflects whether the POST is still loading or already finished — do NOT touch
  // it here (resetting a finished load to not-done would stall it to the watchdog — Codex F-332).
  mChrome->mFocusedEditable = nullptr;   // the submitted form's field belongs to the old document
  // If the nav is STILL in flight, mark it programmatic so its own redirect/subframe STARTs aren't
  // re-detected as fresh link clicks. If it already COMPLETED (a fast POST whose STATE_STOP already
  // reset mProgrammaticLoad to false), leave it false so the NEXT content nav is still detected.
  if (!mChrome->mDone) mChrome->mProgrammaticLoad = true;
}

// Reset mProgrammaticLoad so a content nav on a watchdog-dismissed partial page is still detected
// (Codex F-333). Used by the stall watchdog, which — unlike a command load — must not leave the
// engine looking "mid command load" once the overlay is gone.
void GoannaRenderPage::ClearProgrammaticLoad() {
  if (mChrome) mChrome->mProgrammaticLoad = false;
}

bool GoannaRenderPage::DidRedirect() const { return mChrome && mChrome->mRedirected; }

bool GoannaRenderPage::TakeLinkClicked(std::string* url, bool* isPost) {
  if (!mChrome || !mChrome->mLinkClicked) return false;
  if (url) *url = std::string(mChrome->mLinkUrl.get());
  if (isPost) *isPost = mChrome->mLinkIsPost;
  mChrome->mLinkClicked = false;   // consume
  mChrome->mLinkUrl.Truncate();
  return true;
}

bool GoannaRenderPage::GetCertError(std::string* host, int* code) {
  if (!mChrome || !mChrome->mCertError) return false;
  if (host) *host = std::string(mChrome->mCertHost.get());
  if (code) *code = (int)(uint32_t)mChrome->mErrorStatus;
  return true;
}

bool GoannaRenderPage::AcceptCurrentCert() {
  if (!mChrome || !mChrome->mCertError || !mChrome->mCertCert) return false;
  nsCOMPtr<nsICertOverrideService> ovr =
    do_GetService(NS_CERTOVERRIDE_CONTRACTID);
  if (!ovr) return false;
  uint32_t bits = nsICertOverrideService::ERROR_UNTRUSTED |
                  nsICertOverrideService::ERROR_MISMATCH |
                  nsICertOverrideService::ERROR_TIME;
  nsresult rv = ovr->RememberValidityOverride(mChrome->mCertHost, mChrome->mCertPort,
                                              mChrome->mCertCert, bits, /*temporary*/true);
  return NS_SUCCEEDED(rv);
}

bool GoannaRenderPage::GetLoadError(bool* failed, int* code, std::string* url) {
  if (!mChrome) return false;
  if (failed) *failed = mChrome->mLoadFailed;
  if (code) *code = (int)(uint32_t)mChrome->mErrorStatus;
  if (url) *url = mChrome->mFailedUrl.IsEmpty()
             ? CurrentUri() : std::string(mChrome->mFailedUrl.get());
  return true;
}

std::string GoannaRenderPage::CurrentUri() {
  if (!mChrome) return std::string();
  nsCOMPtr<nsIWebNavigation> nav = do_QueryInterface(mChrome->mBrowser);
  if (!nav) return std::string();
  nsCOMPtr<nsIURI> uri; nav->GetCurrentURI(getter_AddRefs(uri));
  if (!uri) return std::string();
  nsCString spec; uri->GetSpec(spec);
  return std::string(spec.get());
}

std::string GoannaRenderPage::GetTitle() {
  if (!mChrome) return std::string();
  nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
  if (!ds) return std::string();
  nsCOMPtr<nsIContentViewer> cv;
  ds->GetContentViewer(getter_AddRefs(cv));
  if (!cv) return std::string();
  nsCOMPtr<nsIDOMDocument> doc;
  cv->GetDOMDocument(getter_AddRefs(doc));
  if (!doc) return std::string();
  nsAutoString title; doc->GetTitle(title);
  return std::string(NS_ConvertUTF16toUTF8(title).get());
}

// DIAGNOSTIC (gated on a URL marker): programmatically focus the first <input> and dispatch real
// key events into the engine, to verify — WITHOUT a physical VKB tap — that (a) focusing an editable
// no longer crashes headless and (b) SendKeyEvent no longer null-derefs mTabChild (UXP patch 0010).
// Logs each step so the last line before any daemon respawn pinpoints a remaining crash site.


// --- process-global browser services ---------------------------------------
void SetUserAgentOverride(const char* ua) {
  // Only apply a NON-empty UA. The isis adapter/UI sends an empty setUserAgent at
  // connect; without this guard it would wipe the complete default UA set at engine
  // init (EngineHost) and the engine would fall back to the branding-stripped
  // "Goanna/ /x.y" string (empty app-name/version fields).
  if (!ua || !*ua) return;
  nsCOMPtr<nsIPrefBranch> pb = do_GetService("@mozilla.org/preferences-service;1");
  if (pb) pb->SetCharPref("general.useragent.override", ua);
}
void ClearCache() {
  nsCOMPtr<nsICacheStorageService> c =
    do_GetService("@mozilla.org/netwerk/cache-storage-service;1");
  if (c) c->Clear();
}
void ClearCookies() {
  nsCOMPtr<nsICookieManager> cm = do_GetService("@mozilla.org/cookiemanager;1");
  if (cm) cm->RemoveAll();
}
void SetMinFontSize(int px) {
  nsCOMPtr<nsIPrefBranch> pb = do_GetService("@mozilla.org/preferences-service;1");
  if (pb) pb->SetIntPref("font.minimum-size.x-western", px);
}
void SetBlockPopups(bool block) {
  // Blocks script-initiated window.open that isn't driven by a user event.
  nsCOMPtr<nsIPrefBranch> pb = do_GetService("@mozilla.org/preferences-service;1");
  if (pb) pb->SetBoolPref("dom.disable_open_during_load", block);
}
void SetAcceptCookies(bool accept) {
  // network.cookie.cookieBehavior: 0 = accept all, 2 = reject all.
  nsCOMPtr<nsIPrefBranch> pb = do_GetService("@mozilla.org/preferences-service;1");
  if (pb) pb->SetIntPref("network.cookie.cookieBehavior", accept ? 0 : 2);
}

// Get the real content nsIDocShell. nsWebBrowser forwards nsIWebNavigation to
// its docshell but is not itself the docshell, so QI'ing the webBrowser fails;
// reach it through the content window's interface requestor instead.
static already_AddRefed<nsIDocShell> GetDocShell(nsIWebBrowser* wb) {
  nsCOMPtr<nsIDocShell> ds;
  if (!wb) return ds.forget();
  nsCOMPtr<mozIDOMWindowProxy> win;
  wb->GetContentDOMWindow(getter_AddRefs(win));
  nsCOMPtr<nsIInterfaceRequestor> ir = do_QueryInterface(win);
  if (!ir) return ds.forget();
  nsCOMPtr<nsIWebNavigation> wn;
  ir->GetInterface(NS_GET_IID(nsIWebNavigation), getter_AddRefs(wn));
  ds = do_QueryInterface(wn);   // the window's nsIWebNavigation IS the docshell
  return ds.forget();
}

// F-1: the identity token the download service reports as a download's origin.
// The nsWebBrowser owns this docShell for its whole life, so the raw pointer is a
// stable key while the page exists — and it is only ever COMPARED (see
// GoannaRenderPage.h / jihad::DownloadOrigin), never dereferenced.
const void* GoannaRenderPage::DocShellKey() const {
  if (!mChrome) return nullptr;
  nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
  return ds.get();
}

// ── T-067 / cavekit-input-bridging R6 — WHY SYNTHESIZED INPUT DID NOTHING ──────────────────
//
// Every synthesized event was DROPPED before it reached the DOM, on the offscreen path only.
// Nothing about this was XUL-specific: XUL was simply the only content with no hand-written
// fallback propping it up. The chain, all in UXP:
//
//   1. `nsIWidget::UsePuppetWidgets()` is `XRE_IsContentProcess()`, and this daemon is a single
//      process, so it is FALSE.
//   2. `nsDocumentViewer::ShouldAttachToTopLevel()` therefore falls through to a platform branch
//      guarded by `#if defined(XP_WIN) || defined(MOZ_WIDGET_GTK)` which ALSO demands a
//      `typeChrome` docshell. The device build is `--enable-default-toolkit=cairo-headless`, so
//      MOZ_WIDGET_GTK is not even defined — and our docshell is typeContent regardless. False.
//   3. So `nsDocumentViewer::MakeWindow` takes the `CreateWidgetForParent` branch: the root view
//      gets its own CHILD PuppetWidget registered through `nsView::InitializeWindow` ->
//      `SetWidgetListener()`, the PLAIN listener — not the ATTACHED one.
//   4. `PuppetWidget::DispatchEvent` consults `GetCurrentWidgetListener()`, which returns
//      `mAttachedWidgetListener` AND NOTHING ELSE. It never falls back to `mWidgetListener` the
//      way every real widget does (`nsWindow::DispatchEvent` -> `GetListener()`, i.e.
//      `mAttachedWidgetListener ? mAttachedWidgetListener : mWidgetListener`). PuppetWidget only
//      ever runs in a content process, where step 1 is true and the view is always attached, so
//      upstream never needed the fallback.
//   => the listener is null, `DispatchEvent` returns eIgnore, the event is discarded in silence.
//      No mousedown, no mouseup, no click, no default action, no XUL command, nothing.
//
// MEASURED, not argued: on the GTK on-screen widget path a synthesized click fires the page's
// onclick; under JIHAD_OFFSCREEN — the only path the device ever runs — the same click on the
// same engine fires nothing at all, including on plain HTML. See render/goanna/test/xul_test.cpp
// phases G and H. This also CORRECTS the reading recorded in cavekit-input-bridging R1 ("a
// synthesized click fires onclick but not the default action"): offscreen, the click was never
// generated at all, which is the real reason every native control had to be hand-activated.
//
// THE FIX used here: dispatch through the `…ToWindow` entry points. `nsContentUtils::SendMouseEvent`
// with `aToWindow == true` skips `widget->DispatchEvent` entirely and calls
// `presShell->HandleEvent(rootView->GetFrame(), …)` directly, so the un-attached PuppetWidget is
// no longer in the path. This is the documented purpose of those methods ("ensure that the event
// is dispatched to this DOM window or one of its children") and it needs NO libxul rebuild — it
// works against the engine already on the device.
//
// NOT FIXED BY THIS: `sendKeyEvent` has no `…ToWindow` variant (`nsDOMWindowUtils::SendKeyEvent`
// goes straight to `widget->DispatchEvent`), so raw key synthesis still dies in PuppetWidget.
// Text entry does not depend on it — R2/R2a drive the engine editor directly — but true keyboard
// delivery into XUL needs the engine-side fallback in PuppetWidget::DispatchEvent.
//
// WHAT THIS DOES **NOT** EXPLAIN (F-4, honesty about the evidence). The dropped-event chain above
// is verified and it fixes the DROPPED-EVENT defect. It does NOT account for the 2026-07-20 device
// report "SendMouseEvent on XUL SIGSEGVs (core dumped)": a discarded event cannot dump core. So one
// of these is true and we do not yet know which — (a) that crash was misattributed and its real
// cause is still unidentified, or (b) the device path diverges from desktop somewhere this analysis
// does not reach (the device widget probe shows theme components ABSENT, and the offscreen path now
// runs strictly MORE XUL frame code than has ever run there: holdAt -> contextmenu -> the XUL
// <menupopup> open path, tree frames, column pickers — none of which the desktop taps exercise).
// Claim only what is established: the dropped-event defect is fixed and XUL input works on the
// desktop headless build; the recorded SIGSEGV remains unattributed until a device run says
// otherwise.

// Get the content window's nsIDOMWindowUtils (for input synthesis).
static already_AddRefed<nsIDOMWindowUtils> GetWindowUtils(nsIWebBrowser* wb) {
  nsCOMPtr<nsIDOMWindowUtils> utils;
  if (!wb) return utils.forget();
  nsCOMPtr<mozIDOMWindowProxy> win;
  wb->GetContentDOMWindow(getter_AddRefs(win));
  nsCOMPtr<nsIInterfaceRequestor> ir = do_QueryInterface(win);
  if (ir) ir->GetInterface(NS_GET_IID(nsIDOMWindowUtils), getter_AddRefs(utils));
  return utils.forget();
}

// Give the content window activation + focus. The offscreen PuppetWidget
// (JIHAD_OFFSCREEN_ONLY) gets NO window-manager activation, so the document is
// treated as inactive and a synthesized mousedown+mouseup does not run the click's
// default action -> links/buttons never navigate. (On desktop the GTK window
// provides this for free, which is why link_test passes there but taps did nothing
// on device.) Activate the browser, focus the content window, and mark the docShell
// active so click default-actions fire.
static void ActivateContent(nsIWebBrowser* wb) {
  if (!wb) return;
  nsCOMPtr<nsIWebBrowserFocus> focus = do_QueryInterface(wb);
  if (focus) {
    focus->Activate();
    nsCOMPtr<mozIDOMWindowProxy> win;
    wb->GetContentDOMWindow(getter_AddRefs(win));
    if (win) focus->SetFocusedWindow(win);
  }
  nsCOMPtr<nsIDocShell> ds = GetDocShell(wb);
  if (ds) ds->SetIsActive(true);
}

// Crash-safe implicit form submission. We CANNOT use nsIDOMHTMLElement::DOMClick() — the engine
// forbids Click() from native code (nsGenericHTMLElement::Click() → IsCallerChrome() →
// SubjectPrincipal() MOZ_CRASHes with no JSContext on the stack), which was a latent daemon SIGSEGV
// on every submit-button activation. And a synthesized mousedown+mouseup fires the `click` event
// (so onclick handlers run) but NOT the submit DEFAULT ACTION in this offscreen embedding — the
// reason the crashing DOMClick was there at all. So replicate requestSubmit(): run constraint
// validation (honoring novalidate), dispatch a cancelable `submit` event so onsubmit runs and can
// preventDefault, then form->Submit() (which itself skips both validation and the submit event) only
// if it wasn't cancelled. Returns true iff a submission was actually issued (a nav is starting).
bool GoannaRenderPage::FireFormSubmit(nsIDOMHTMLFormElement* form) {
  if (!form) return false;
  bool noValidate = false; form->GetNoValidate(&noValidate);
  if (!noValidate) { bool valid = true; form->CheckValidity(&valid); if (!valid) return false; }
  bool proceed = true;   // becomes false iff an onsubmit handler calls preventDefault
  nsCOMPtr<nsIDOMDocument> doc;
  { nsCOMPtr<nsIDOMNode> fn = do_QueryInterface(form);
    if (fn) fn->GetOwnerDocument(getter_AddRefs(doc)); }
  nsCOMPtr<nsIDOMEventTarget> ftgt = do_QueryInterface(form);
  if (doc && ftgt) {
    nsCOMPtr<nsIDOMEvent> ev;
    if (NS_SUCCEEDED(doc->CreateEvent(NS_LITERAL_STRING("Events"), getter_AddRefs(ev))) && ev) {
      ev->InitEvent(NS_LITERAL_STRING("submit"), /*bubbles*/true, /*cancelable*/true);
      ftgt->DispatchEvent(ev, &proceed);
    }
  }
  if (!proceed) return false;
  form->Submit();
  return true;
}

// Monotonic-ish ms clock for the F-1 pen-pair/clickAt correlation window. CLOCK_MONOTONIC, so a
// wall-clock step (ntp, the device's clock coming up late) cannot make a fresh pair look ancient.
static long jihadInputNowMs() {
  struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Is `inner` the element `outer`, or inside it? Walks the DOM parent chain, which for XBL
// ANONYMOUS content (the html:input inside a XUL <textbox>) leads back to the bound element — so
// this answers "did the tap land on the thing that now has focus?" for exactly the case the tap
// classifier cannot see. Depth-capped as a cycle guard, like the anchor walks above.
static bool jihadIsSelfOrInside(nsIDOMElement* inner, nsIDOMElement* outer) {
  if (!inner || !outer) return false;
  nsCOMPtr<nsIDOMNode> n = do_QueryInterface(inner);
  nsCOMPtr<nsIDOMNode> o = do_QueryInterface(outer);
  for (int d = 0; n && d < 64; ++d) {
    if (n == o) return true;
    nsCOMPtr<nsIDOMNode> p; n->GetParentNode(getter_AddRefs(p)); n = p;
  }
  return false;
}

// The element the focus manager currently reports, as a bare pointer for IDENTITY COMPARISON
// ONLY (F-7 / F-1). Never dereferenced — by the time it is compared the element may be gone.
static const void* jihadFocusedPtr() {
  nsCOMPtr<nsIFocusManager> fm = do_GetService("@mozilla.org/focus-manager;1");
  if (!fm) return nullptr;
  nsCOMPtr<nsIDOMElement> foc; fm->GetFocusedElement(getter_AddRefs(foc));
  return foc.get();
}

// ── F-2/F-3 — DELIVERY DETECTION, because STATE cannot tell the two cases apart ───────────────
// The checkbox fallback used to infer "the engine did not act" from `checked` being unchanged.
// It is not inferable: UXP PRE-toggles the control and REVERTS the toggle when the click comes
// back eConsumeNoDefault (HTMLInputElement.cpp:4483-4505), so a preventDefault()ed click and a
// click that never arrived leave byte-identical state — and the fallback then force-flipped a
// control the page had deliberately refused, firing phantom input/change. The submit fallback had
// the same shape: "nothing started loading" includes every legitimate refusal (onsubmit returning
// false, a preventDefault()ed click), so it re-ran exactly what the engine declined.
// The …ToWindow dispatch passes nullptr for aPreventDefault, so the status is not available that
// way. Instead OBSERVE the click: a one-shot capture-phase listener on the control answers the
// only question the fallback may act on — did a click reach this control AT ALL. `defaultPrevented`
// is read off the retained event AFTER dispatch returns (at capture time it is not final yet); it
// is logged for diagnosis, never used to hand-activate.
class ClickProbe final : public nsIDOMEventListener {
public:
  NS_DECL_ISUPPORTS
  NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent) override {
    mSaw = true;
    mEvent = aEvent;   // retain: the flags on it stay readable after the dispatch unwinds
    return NS_OK;
  }
  bool Saw() const { return mSaw; }
  bool DefaultPrevented() const {
    bool p = false; if (mEvent) mEvent->GetDefaultPrevented(&p); return p;
  }
private:
  ~ClickProbe() {}
  bool mSaw = false;
  nsCOMPtr<nsIDOMEvent> mEvent;
};
NS_IMPL_ISUPPORTS(ClickProbe, nsIDOMEventListener)

void GoannaRenderPage::MouseEvent(const char* type, int x, int y, int button) {
  if (!mChrome) return;
  ActivateContent(mChrome->mBrowser);
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return;
  bool ret = false;
  NS_ConvertUTF8toUTF16 t(type);
  // F-1 bookkeeping, BEFORE the dispatch (the focus snapshot has to predate it). A down+up pair
  // that lands within kRawTapSlop of each other is a TAP, and since T-067 the engine synthesises a
  // real click from it — including the default action. Record that so the clickAt the adapter
  // sends for the same tap does not activate the element a second time. A pair that ends far from
  // its down is a drag: the adapter sends no tap gesture for it and the engine synthesises no
  // click, so nothing is recorded.
  const int kRawTapSlop = 16;                     // content px; adapter drag hysteresis is 4
  if (strcmp(type, "mousedown") == 0) {
    mRawDownX = x; mRawDownY = y; mRawDownMs = jihadInputNowMs(); mRawClickMs = 0;
    mRawFocusBefore = jihadFocusedPtr();
  } else if (strcmp(type, "mouseup") == 0) {
    long now = jihadInputNowMs();
    int dx = x - mRawDownX, dy = y - mRawDownY;
    if (mRawDownMs && (now - mRawDownMs) < 4000 && (dx * dx + dy * dy) <= kRawTapSlop * kRawTapSlop) {
      mRawClickX = x; mRawClickY = y; mRawClickMs = now;
    }
    mRawDownMs = 0;
  }
  // Pass explicit buttons (_argc=6): the left button is held for down/move, released
  // (0) for up. With _argc=0 the impl derives buttons from aButton for BOTH, so a
  // mouseup reports the button still held and the press/release may not register.
  int32_t buttons = (strcmp(type, "mouseup") == 0) ? 0 : 1;
  // ToWindow: routed via the presShell, not the widget (T-067). No preventDefault out-param.
  (void)ret;
  u->SendMouseEventToWindow(t, (float)x, (float)y, button, /*clickCount*/1, /*mods*/0,
                            false, 0.0f, 0, false, false, buttons, 6);
}

// ── F-1 — ONE TAP MUST DELIVER ONE CLICK SEQUENCE ─────────────────────────────────────────────
//
// A device tap reaches this daemon TWICE. The adapter's pen path sends asyncCmdMouseEvent(down)
// then (up) whenever `shouldPassInputEvents()` is true — which is "the content fits the viewport
// AND the meta-viewport is non-scalable", i.e. most mobile-optimised pages, NOT only when the tap
// hits a known interactive rect — and Jihad's single-tap gesture separately sends asyncCmdClickAt.
// Before T-067 the raw pair was discarded by the un-attached PuppetWidget, so only clickAt acted
// and nobody noticed. Now both act: a checkbox toggles twice (net zero, two `change` events), a
// form submits twice, and a link tap starts two loads that abort each other (NS_BINDING_ABORTED).
// Reproduced on the desktop harness — xul_test phase K2 came back UNCHECKED before this fix.
//
// WHERE THE DEDUP LIVES, and why here rather than in the adapter:
//   * The adapter could suppress its tap->clickAt when it forwarded the pen pair, and that is
//     exact (it knows what it sent). But it throws away everything clickAt does that the raw pair
//     does not — hit-test, the near-miss link touch target, the VKB editable classification, the
//     crash-safe deferral to the guarded pump — on precisely the pages where the pen path is live.
//     It is also UNTESTABLE without the device: no desktop harness runs the adapter.
//   * Here, the daemon keeps all of that and drops only the DUPLICATE DOM DISPATCH. It covers any
//     client that sends both (all three UI variants share one adapter, plus the inject channel),
//     it survives the adapter's condition changing, and — decisive — xul_test phase K exercises it
//     on the desktop, so the fix is proven before it ever reaches hardware.
//   * Cost: four ints and a timestamp. The correlation is not a guess: the record is CONSUMED by
//     the first clickAt after it, so a stale pair can never suppress a later tap.
//
// WHICH PATH WINS: the raw pair, necessarily — it is delivered first and cannot be recalled. That
// is also the correct browser sequence (mousedown, mouseup, click). Measured (xul_test K1/K3) that
// the pair alone activates a checkbox AND navigates an <a href> via the engine's own default
// action, so nothing is lost by standing down here.
void GoannaRenderPage::ClickAt(int x, int y, int numClicks) {
  if (!mChrome) return;
  mChrome->mUserInteracted = true;   // Atlas autofocus gate: a real tap unlocks VKB raises
  ActivateContent(mChrome->mBrowser);   // offscreen widget needs explicit activation (see above)
  // An OPEN popup owns the pixels under it, so it must own the taps on them too. Its
  // separate display root is invisible to the content document's elementFromPoint —
  // measured: with the about:addons tools menu open, a tap on its first row resolved to
  // the <vbox> UNDERNEATH. Offer the tap to the popup first; if it takes it, this tap is
  // finished (dispatching to the content document as well would act on both).
  if (jihad_offscreen_popup_mouse) {
    double cssX = x, cssY = y;
    double Z = (mRenderZoom >= 0.05 && mRenderZoom <= 20.0) ? mRenderZoom : 1.0;
    if (Z != 1.0) { cssX = x / Z + mPanX; cssY = y / Z + mPanY; }
    if (jihad_offscreen_popup_mouse(1, cssX, cssY)) {   // down
      jihad_offscreen_popup_mouse(2, cssX, cssY);       // up -> the menuitem's command
      // No explicit repaint request needed: the popup's own invalidation sets the
      // sticky dirty flag the daemon drains each tick (jihad_offscreen_take_dirty).
      fprintf(stderr, "[jihad-bs] clickAt (%d,%d) went to an open popup\n", x, y);
      return;
    }
  }
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return;
  // Did the pen path already deliver a complete click for THIS tap? Window: the adapter sends the
  // tap gesture immediately after pen-up and the daemon defers clickAt to the next ~10ms tick, so
  // a real pairing is tens of ms; 1s is slack for a loaded device, still far below any plausible
  // second deliberate tap on the same pixel. The record is consumed either way — an unmatched
  // clickAt means the correspondence is broken, and a kept record could then suppress a real tap.
  bool rawClickDelivered = false;
  if (mRawClickMs) {
    long age = jihadInputNowMs() - mRawClickMs;
    int dx = x - mRawClickX, dy = y - mRawClickY;
    const int kSlop = 24;   // content px between the pen-up point and the gesture's tap point
    rawClickDelivered = (age >= 0 && age <= 1000) && (dx * dx + dy * dy) <= kSlop * kSlop;
    mRawClickMs = 0;
  }
  if (rawClickDelivered)
    fprintf(stderr, "[jihad-bs] clickAt (%d,%d): pen path already delivered this tap's click — "
                    "no second activation (F-1)\n", x, y);
  // Resolve the tap target FIRST. This offscreen embedding does NOT run the click
  // default-action (confirmed on device: taps land on <A> but the anchor never
  // navigates on its own). Dispatch by target:
  //   - a real link (<a href> to http/https/ftp/file) -> navigate directly via the
  //     daemon's own load path (LoadUrl). Do ONLY this for a link: dispatching the
  //     mouse events / DOMClick as well made the anchor ALSO start a load, and the two
  //     competing loads cancelled each other (NS_BINDING_ABORTED) so nothing navigated.
  //   - anything else -> mouse events + DOM click() so buttons / form controls / JS
  //     onclick handlers activate.
  nsCOMPtr<nsIDOMElement> el;
  u->ElementFromPoint((float)x, (float)y, false, true, getter_AddRefs(el));
  // Walk up to the nearest <a href> ancestor (the tap often lands on inline content
  // inside the anchor). The parent chain terminates at the document (null parent); the
  // depth cap is just a cycle guard for a malformed tree.
  nsAutoString href;
  nsCOMPtr<nsIDOMNode> node = do_QueryInterface(el);
  for (int depth = 0; node && depth < 256 && href.IsEmpty(); ++depth) {
    nsCOMPtr<nsIDOMHTMLAnchorElement> a = do_QueryInterface(node);
    if (a) a->GetHref(href);
    if (href.IsEmpty()) { nsCOMPtr<nsIDOMNode> p; node->GetParentNode(getter_AddRefs(p)); node = p; }
  }
  // Touch-target tolerance: a small link is easy to miss by a few px (the tap lands on the
  // surrounding text/heading/<html>). If the exact point found no link AND it did not land on an
  // interactive control (never hijack a tap meant for a form field/button), search a small radius
  // for the nearest <a href> and follow it. NodesFromRect returns nodes topmost-first.
  if (href.IsEmpty() && el) {
    nsAutoString dtag; el->GetTagName(dtag);
    std::string dt = NS_ConvertUTF16toUTF8(dtag).get();
    for (char& c : dt) c = (char)toupper((unsigned char)c);
    // Only expand the touch target when the tap clearly hit nothing — the root <html> or <body>
    // background. Do NOT expand over other elements (a <div role=button>, a custom control, a
    // heading with its own handler, etc.) or a slightly-off tap could hijack the intended element
    // and navigate to a nearby link instead (Codex F-246). Small radius keeps it to true near-misses.
    if (dt == "HTML" || dt == "BODY") {
      nsCOMPtr<nsIDOMNodeList> near;
      u->NodesFromRect((float)x, (float)y, 10, 10, 10, 10, false, true, getter_AddRefs(near));
      uint32_t nn = 0; if (near) near->GetLength(&nn);
      for (uint32_t i = 0; i < nn && href.IsEmpty(); ++i) {
        nsCOMPtr<nsIDOMNode> nd; near->Item(i, getter_AddRefs(nd));
        for (int d = 0; nd && d < 64 && href.IsEmpty(); ++d) {
          nsCOMPtr<nsIDOMHTMLAnchorElement> a = do_QueryInterface(nd);
          if (a) a->GetHref(href);
          if (href.IsEmpty()) { nsCOMPtr<nsIDOMNode> p; nd->GetParentNode(getter_AddRefs(p)); nd = p; }
        }
      }
    }
  }
  NS_ConvertUTF16toUTF8 hrefUtf8(href);
  const char* h = hrefUtf8.get();
  // A single tap on a real external link navigates directly (the click default-action is
  // inert offscreen). numClicks!=1 (double-tap etc.) falls through to the mouse/DOMClick
  // path rather than direct-navigating (L-2).
  bool navigable = numClicks == 1 && h &&
      (strncmp(h, "http://", 7) == 0 || strncmp(h, "https://", 8) == 0 ||
       strncmp(h, "ftp://", 6) == 0 || strncmp(h, "file://", 7) == 0);
  // Same-document fragment link (#frag): GetHref resolves to absolute, so "#frag" becomes
  // "<current-sans-frag>#frag". A full openUrl would restart the load lifecycle (stuck
  // overlay, wrong history); route it to the in-page click path instead (M-1).
  if (navigable) {
    const char* hash = strchr(h, '#');
    if (hash) {
      std::string cur = CurrentUri();
      size_t cf = cur.find('#');
      std::string curBase = (cf == std::string::npos) ? cur : cur.substr(0, cf);
      if (curBase == std::string(h, hash - h)) navigable = false;
    }
  }
  nsAutoString tag; if (el) el->GetTagName(tag);
  fprintf(stderr, "[jihad-bs] clickAt (%d,%d) <%s> href=%s nav=%d\n",
          x, y, el ? NS_ConvertUTF16toUTF8(tag).get() : "null", navigable ? h : "-", navigable);
  if (navigable) {
    // F-1: if the pen pair already delivered this tap's click, the ENGINE has already started the
    // anchor's load (measured — xul_test K3 ends on the link target with no clickAt involved).
    // Recording it here as well would openUrl the same href a second time: two loads racing, which
    // is the NS_BINDING_ABORTED failure the comment below was written about. Just do the VKB
    // bookkeeping the navigation implies.
    if (rawClickDelivered) {
      fprintf(stderr, "[jihad-bs] clickAt: link already navigated by the pen path — not re-driving\n");
      if (mEditorFocused) { mEditorFocused = false; mEditorFocusDirty = true; }
      return;
    }
    // Record the target; BrowserPageGoanna::pump drains it (TakeClickNav) and navigates
    // via openUrl on the tick. Navigating here (inside the click flow) either stalls the
    // load or, done synchronously in the socket callback, re-enters + crashes. Skip the
    // mouse events for a link too — they made the anchor start its own (aborting) load.
    mClickNavUrl = h;
    if (mEditorFocused) { mEditorFocused = false; mEditorFocusDirty = true; }  // page change -> hide VKB
    return;
  }
  // --- <select> dropdown -> CARD-NATIVE popup (Atlas model). A tap on a dropdown <select>
  // must NOT dispatch a normal click: the engine would open its own XUL/native combobox
  // popup, which in this offscreen embedding is created 0x0 and never painted (a separate
  // display root — see impl-menupopup-2026-08-02.md). Instead read the options straight
  // from the DOM, serialize them, and hand them to the card, which shows a native list and
  // sends the choice back via popupMenuSelect (asyncCmdPopupMenuSelect). This is exactly the
  // BrowserAdapter msgPopupMenuShow/selectPopupMenuItem contract inherited from isis, and is
  // the good-webOS-citizen path Atlas uses. Multi-selects and list-boxes (size>1) render
  // inline and are left to the normal click path. ---
  {
    nsCOMPtr<nsIDOMHTMLSelectElement> sel;
    nsCOMPtr<nsIDOMNode> n = do_QueryInterface(el);
    for (int d = 0; n && d < 32 && !sel; ++d) {
      sel = do_QueryInterface(n);
      if (!sel) { nsCOMPtr<nsIDOMNode> p; n->GetParentNode(getter_AddRefs(p)); n = p; }
    }
    if (sel) {
      bool multiple = false; uint32_t size = 0; bool disabled = false;
      sel->GetMultiple(&multiple); sel->GetSize(&size); sel->GetDisabled(&disabled);
      if (!disabled && !multiple && size <= 1) {
        // A dropdown <select>: hand it to the card popup (BuildSelectPopup queues it, or a
        // dedup skips a duplicate). EITHER WAY return — never fall through to the normal
        // click/VKB path, which would dispatch a mouse click that just focus-rings the
        // <select> (device 2026-08-03: "box around Apple") and can open the engine's own
        // 0x0 combobox.
        BuildSelectPopup(sel);
        return;
      }
    }
  }
  // --- VKB editable detection. Compute editability from the tapped element BEFORE the
  // click, using the tag name + "type" ATTRIBUTE via nsIDOMElement (known-good QI —
  // ElementFromPoint returned it). Do NOT depend on nsIDOMHTMLInputElement/GetType
  // (review #6 F-002). Skip readonly/disabled fields (F-010). ---
  // If the tap is on a <label> (or inside one), the control it labels is what a real click
  // would activate — and a label for a TEXT field focuses that field, which crashes headless
  // (editable focus). Resolve the label's control and detect editability on IT, so the tap
  // raises the VKB instead of DOMClick-ing into the crash (review #7 P1).
  nsCOMPtr<nsIDOMElement> effEl = el;
  {
    nsCOMPtr<nsIDOMNode> n = do_QueryInterface(el);
    for (int d = 0; n && d < 32; ++d) {
      nsCOMPtr<nsIDOMHTMLLabelElement> lab = do_QueryInterface(n);
      if (lab) {
        nsCOMPtr<nsIDOMHTMLElement> ctrl; lab->GetControl(getter_AddRefs(ctrl));
        nsCOMPtr<nsIDOMElement> ctrlEl = do_QueryInterface(ctrl);
        if (ctrlEl) effEl = ctrlEl;
        break;
      }
      nsCOMPtr<nsIDOMNode> p; n->GetParentNode(getter_AddRefs(p)); n = p;
    }
  }
  bool editable = false;
  {
    nsAutoString effTag; if (effEl) effEl->GetTagName(effTag);
    std::string tg = effEl ? std::string(NS_ConvertUTF16toUTF8(effTag).get()) : std::string();
    for (char& c : tg) c = (char)toupper((unsigned char)c);
    bool ro = false, dis = false;
    if (effEl) { effEl->HasAttribute(NS_LITERAL_STRING("readonly"), &ro);
                 effEl->HasAttribute(NS_LITERAL_STRING("disabled"), &dis); }
    if (effEl && tg == "TEXTAREA") {
      editable = !ro && !dis;
    } else if (effEl && tg == "INPUT") {
      nsAutoString typeAttr; effEl->GetAttribute(NS_LITERAL_STRING("type"), typeAttr);
      std::string ty = std::string(NS_ConvertUTF16toUTF8(typeAttr).get());
      for (char& c : ty) c = (char)tolower((unsigned char)c);
      const char* ts = ty.c_str();
      bool textlike = !*ts || !strcmp(ts, "text") || !strcmp(ts, "search") || !strcmp(ts, "email") ||
                      !strcmp(ts, "url") || !strcmp(ts, "tel") || !strcmp(ts, "number") ||
                      !strcmp(ts, "password");
      editable = textlike && !ro && !dis;
    }
    if (!editable && effEl) {   // a contentEditable region (rich text editors)
      nsCOMPtr<nsIDOMHTMLElement> h2 = do_QueryInterface(effEl);
      if (h2) { bool ce = false; h2->GetIsContentEditable(&ce); editable = ce; }
    }
  }
  // Log the detection + update the editor-focus state BEFORE any click, so the VKB
  // decision is recorded even if a later engine call faults (and so the crash-prone click
  // path below is never taken for an editable).
  fprintf(stderr, "[jihad-bs] vkb tag=[%s] editable=%d was=%d\n",
          el ? NS_ConvertUTF16toUTF8(tag).get() : "null", editable, mEditorFocused);
  // (Re)assert the VKB state on every tap. For an editable tap ALWAYS emit msgEditorFocused(true),
  // even if we already thought it was focused (was=1): the user may have dismissed the keyboard
  // (webOS swipe-down / the VKB's own hide) without the daemon knowing, so a re-tap must re-raise
  // it. For a non-editable tap, hide the VKB only if it was up.
  if (editable) { mEditorFocused = true; mEditorFieldType = 0; mEditorFocusDirty = true; }
  else if (mEditorFocused) { mEditorFocused = false; mEditorFocusDirty = true; }

  if (editable) {
    // Editable field: pump() emits msgEditorFocused(true) to raise the VKB. Remember the field as
    // the type target, then FOCUS it so the engine shows a real, visible caret. Focusing an editable
    // is crash-free in the headless/offscreen build after UXP patch 0010 (validated on-device) — the
    // earlier "focus crashes the daemon" behaviour was the pre-0010 mTabChild null-deref. The caret
    // lands at a sane default; the user repositions it with the arrow keys (exact tap-to-offset would
    // need a non-navigating layout hit-test — a javascript: URL flashes the isis loading overlay).
    // Remember the scroll position before focusing: Focus() scrolls the field into view, but the
    // user just tapped it so it is already on-screen — that scroll only cuts the top of the page off
    // (e.g. google's logo) when the VKB shrinks the viewport. Restore it afterwards. window.scrollTo
    // is overlay-free (unlike a javascript: URL that does editor work), so no white flash.
    int svScrollX = 0, svScrollY = 0; GetScrollXY(&svScrollX, &svScrollY);
    mChrome->mFocusedEditable = effEl;
    nsCOMPtr<nsIDOMHTMLElement> hedit = do_QueryInterface(effEl);
    if (hedit) hedit->Focus();
    ActivateEditorCaret();   // make the caret actually paint (activate window + solid caret)
    // F-225: Focus() can run a page focus handler that moves focus to a DIFFERENT field (e.g. a
    // wrapper redirects to a hidden proxy input). Retarget the edit target to whatever is ACTUALLY
    // focused if that is itself an <input>/<textarea>, so the visible caret and the keystroke target
    // never diverge. If the handler instead moved focus to a NON-text control, clear the target so a
    // keystroke can't silently mutate the tapped-but-now-unfocused field (Codex F-270/F-292). Focus
    // going nowhere (null) leaves the tapped field as the target.
    {
      nsCOMPtr<nsIFocusManager> fm = do_GetService("@mozilla.org/focus-manager;1");
      if (fm) {
        nsCOMPtr<nsIDOMElement> foc; fm->GetFocusedElement(getter_AddRefs(foc));
        if (foc && foc != effEl) {
          if (edIsTextInput(foc)) {
            mChrome->mFocusedEditable = foc;
          } else {
            // Focus went to a non-text control: no editable target, so also LOWER the VKB we were about
            // to raise — otherwise the keyboard stays up over a field it can't edit (Codex F-326).
            mChrome->mFocusedEditable = nullptr;
            mEditorFocused = false; mEditorFieldType = 0; mEditorFocusDirty = true;
          }
        }
      }
    }
    // NB: do NOT position the caret at the tap via a javascript: URL here — a javascript: LoadURI
    // runs through the docShell load machinery and flashes the isis loading overlay (the page
    // "whites out" on every tap). Focus() leaves the caret at a sane default; the user positions
    // it with the arrow keys. Exact tap-to-offset needs a non-navigating hit-test (future work).
    // Undo the focus-induced scroll so the page top stays visible under the keyboard (see above) —
    // but ONLY when the tapped field is in the upper part of the viewport, where it stays visible
    // above the VKB. For a field tapped low (which the VKB would cover), keep Focus()'s scroll so
    // the field is lifted above the keyboard instead of being restored back under it.
    int nx = 0, ny = 0; GetScrollXY(&nx, &ny);
    // Compare the tap's VIEWPORT y (content y minus the scroll offset), not the raw content y —
    // otherwise on a scrolled page a field near the top of the visible viewport has a large content
    // y and the restore is wrongly skipped (Codex F-247).
    if ((nx != svScrollX || ny != svScrollY) && mHeight > 0 && (y - svScrollY) < (mHeight * 55) / 100)
      ScrollTo(svScrollX, svScrollY);
    return;
  }
  // Non-editable tap: stop treating keystrokes as edits to a previously-tapped field. Without
  // this, mFocusedEditable stayed set after tapping away, so keyDown kept swallowing keys and
  // InsertText kept mutating the old field for the rest of the page's life (Jihad review F-164).
  mChrome->mFocusedEditable = nullptr;
  // Non-editable: dispatch mousedown+mouseup at the tap. On the same element these synthesize a
  // trusted `click` through the event-state manager — JS onclick handlers, :active, and focus all
  // fire. Do NOT call nsIDOMHTMLElement::DOMClick(): the engine forbids Click() from native code
  // (SubjectPrincipal() MOZ_CRASH with no JSContext on the stack) — it was a latent daemon SIGSEGV on
  // every non-anchor tap (see FireFormSubmit).
  bool navBefore = mChrome->mLinkClicked;
  bool ret = false;
  NS_ConvertUTF8toUTF16 down("mousedown"), up("mouseup");
  // (T-067/R6) The XUL-namespace test that used to live here — and the crash-avoidance SKIP of
  // both SendMouseEvent calls that it gated — are GONE. XUL was never a special case for INPUT
  // DELIVERY: the events were being discarded for every document type (see the dropped-event
  // analysis above), and XUL was simply the only content with no hand-written activation propping
  // it up. Nothing downstream branches on document namespace any more.
  // F-4, stated plainly: removing the skip is justified by the dropped-event chain and by the
  // desktop headless runs (xul_test — repeated taps on about:config, its tree, and about:addons
  // all survive). It is NOT backed by a device run, and the 2026-07-20 SIGSEGV that the skip was
  // added for has no confirmed cause — a dropped event cannot dump core, so either that
  // attribution was wrong or something device-specific is still unaccounted for. Taps on XUL are
  // therefore live again on a desktop-proven basis, not a device-proven one.
  // F-001 + T-067: a tapped checkbox/radio.
  //
  // This used to hand-flip `checked` BEFORE the synthetic click, because in this embedding the
  // click's default action never ran. Now that events reach the document (see the
  // ScopedWidget/ToWindow analysis above) the ENGINE toggles it, so flipping it ourselves as well
  // would toggle it TWICE and land back on the original state — turning the R1 fix into the exact
  // bug it was written to cure. So the hand-flip became a FALLBACK: remember the state, and only
  // apply it AFTER the click if the engine did not.
  //
  // Kept rather than deleted deliberately. The engine-side activation is proven on the desktop
  // headless build; if any device-specific difference stops it, this silently restores the
  // previously device-verified behaviour instead of regressing to a dead control. It costs one
  // bool comparison when the engine has already done the job.
  //
  // F-2: the trigger is NOT "checked is unchanged" any more. UXP pre-toggles the control and
  // REVERTS that toggle when the click returns eConsumeNoDefault (HTMLInputElement.cpp:4483-4505),
  // so a page that preventDefault()s the click leaves exactly the state a dropped click leaves,
  // and the old test force-flipped it and fired phantom input/change on a control the page had
  // deliberately refused. The fallback now runs only when NO CLICK WAS DELIVERED AT ALL (ClickProbe
  // above), which is the one condition it was written for and the one that state cannot express.
  nsCOMPtr<nsIDOMHTMLInputElement> pendingToggle; bool toggleWasChecked = false; bool toggleIsRadio = false;
  if (effEl) {
    nsAutoString effTag2; effEl->GetTagName(effTag2);
    std::string tg2 = NS_ConvertUTF16toUTF8(effTag2).get();
    for (char& c : tg2) c = (char)toupper((unsigned char)c);
    if (tg2 == "INPUT") {
      nsAutoString it2; effEl->GetAttribute(NS_LITERAL_STRING("type"), it2);
      std::string its2 = NS_ConvertUTF16toUTF8(it2).get();
      for (char& c : its2) c = (char)tolower((unsigned char)c);
      if (its2 == "checkbox" || its2 == "radio") {
        nsCOMPtr<nsIDOMHTMLInputElement> cb = do_QueryInterface(effEl);
        bool dis = false; if (cb) cb->GetDisabled(&dis);          // F-007: catches <fieldset disabled>
        if (cb && !dis) {
          cb->GetChecked(&toggleWasChecked);
          pendingToggle = cb; toggleIsRadio = (its2 == "radio");
        }
      }
    }
  }
  // F-2/F-3: watch for the click on the element the fallbacks would act on. Capture phase, so a
  // content handler calling stopPropagation() cannot hide the click from us and trick a fallback
  // into re-activating. Registered only when we are about to dispatch — if the pen path already
  // delivered (F-1) there is nothing of ours to observe, and `clickDelivered` is true by fiat.
  const void* focBefore = jihadFocusedPtr();   // F-7: focus BEFORE the dispatch (compared only)
  RefPtr<ClickProbe> probe;
  nsCOMPtr<nsIDOMEventTarget> probeTarget = do_QueryInterface(effEl);
  if (probeTarget && !rawClickDelivered) {
    probe = new ClickProbe();
    probeTarget->AddEventListener(NS_LITERAL_STRING("click"), probe, true);
  }
  if (rawClickDelivered) {
    // F-1: the adapter's pen pair already produced a real click here (down, up, click — the
    // browser sequence). Dispatching our own pair would be the SECOND one.
    fprintf(stderr, "[jihad-bs] mouseSend <%s> at %d,%d : SKIPPED, pen path already clicked (F-1)\n",
            el ? NS_ConvertUTF16toUTF8(tag).get() : "null", x, y);
    focBefore = mRawFocusBefore;   // the focus move, if any, happened during THAT dispatch
  } else {
    // Breadcrumb BEFORE the synthetic click: page JS run synchronously inside
    // SendMouseEvent dispatch can fault (a native-code MOZ_CRASH like the old
    // DOMClick SubjectPrincipal, or a headless-unimplemented API). If the daemon
    // dies here, this line + the matching core (wherever /proc/sys/kernel/core_pattern
    // points — NOT /media/internal any more, T-057/R8) name the element that
    // triggered it (device U1). Flushed (stderr unbuffered).
    // T-067: the down and the up are logged SEPARATELY. The 2026-07-20 device report could only
    // say "somewhere inside SendMouseEvent" because a single breadcrumb spanned both calls, and
    // down/up run very different code (down: focus/activation/drag-gesture tracking; up: click
    // synthesis + XUL command dispatch out of nsButtonBoxFrame). Which one dies is half the answer.
    fprintf(stderr, "[jihad-bs] mouseSend <%s> at %d,%d n=%d : down\n",
            el ? NS_ConvertUTF16toUTF8(tag).get() : "null", x, y, numClicks);
    // …ToWindow, NOT SendMouseEvent: the plain form dispatches through the widget, and the
    // offscreen PuppetWidget discards it (T-067 — see the block comment above). These go through
    // the presShell instead, which is what makes a tap actually reach the document.
    (void)ret;
    u->SendMouseEventToWindow(down, (float)x, (float)y, 0, numClicks, 0, false, 0.0f, 0, false, false, 1, 6);
    fprintf(stderr, "[jihad-bs] mouseSend : down ok, up\n");
    u->SendMouseEventToWindow(up,   (float)x, (float)y, 0, numClicks, 0, false, 0.0f, 0, false, false, 0, 6);
    fprintf(stderr, "[jihad-bs] mouseSend done\n");
  }
  // One-shot: off again the moment the dispatch is over, so nothing of ours stays on the page.
  if (probeTarget && probe)
    probeTarget->RemoveEventListener(NS_LITERAL_STRING("click"), probe, true);
  // The single question the fallbacks below are allowed to ask. `defaultPrevented` is reported for
  // the log only — a page that cancels its click has REFUSED the activation, and re-running it by
  // hand is precisely the bug F-2/F-3 record.
  bool clickDelivered = rawClickDelivered || (probe && probe->Saw());
  if (probe && probe->Saw())
    fprintf(stderr, "[jihad-bs] click delivered to the tapped control (defaultPrevented=%d)\n",
            (int)probe->DefaultPrevented());
  // T-067 / R6 keyboard-into-XUL: adopt whatever the ENGINE focused as the keystroke target.
  //
  // A XUL <textbox> (about:config's filter box) is an XBL binding wrapping an ANONYMOUS
  // html:input. `elementFromPoint` never returns anonymous content, so the tap classifier above
  // sees only the XUL <textbox>, decides "not editable", and typing has nowhere to go. Now that
  // the click actually reaches the document it also moves focus for real — and the focus manager
  // DOES resolve through anonymous content — so asking it after the click is what turns a tap on
  // a XUL search field into a working edit target, using the same SetValue/SetSelectionRange
  // machinery HTML fields already use (R2a). Same reason this is not restricted to XUL: any
  // shadow/anonymous text control benefits.
  //
  // Only ADOPTS: it never clears an existing target (the non-editable-tap path above already did
  // that deliberately), and it only follows focus to a real text control.
  //
  // F-7: and only when this tap can be shown to be responsible for that focus — either focus
  // CHANGED across the dispatch, or the focused control is the tapped element itself / anonymous
  // content inside it (a re-tap on the already-focused XUL <textbox>, which must keep working).
  // Adopting whatever happens to be focused fights the tap-away policy twelve lines up: if the
  // page cancels `mousedown` (drag and custom-UI libraries do this routinely) focus never moves,
  // and the old code re-adopted the very field the user just tapped AWAY from — re-raising the VKB
  // over a password field they had dismissed. Neither condition holds for that tap, so it now
  // earns nothing. (focBefore is a bare pointer compared for identity only — never dereferenced.
  // A recycled address can only make this MISS an adoption, which is the safe direction.)
  if (!mChrome->mFocusedEditable) {
    nsCOMPtr<nsIFocusManager> fm = do_GetService("@mozilla.org/focus-manager;1");
    if (fm) {
      nsCOMPtr<nsIDOMElement> foc; fm->GetFocusedElement(getter_AddRefs(foc));
      bool causedByThisTap = (foc.get() != focBefore) || jihadIsSelfOrInside(foc, effEl);
      if (causedByThisTap && edIsTextInput(foc)) {
        mChrome->mFocusedEditable = foc;
        mEditorFocused = true; mEditorFieldType = 0; mEditorFocusDirty = true;
        fprintf(stderr, "[jihad-bs] adopted engine-focused text control as the edit target\n");
      }
    }
  }
  // F-001/F-004 + T-067 + F-2: only now do we know whether a click reached the control. If one
  // did, we do nothing at all — the engine ran the default action (or the page cancelled it, which
  // is equally its decision), fired `input`/`change` itself, in the right order, with the right
  // trust flags. Only a click that NEVER ARRIVED licenses the hand-flip.
  nsCOMPtr<nsIDOMHTMLInputElement> toggledCtrl; bool toggledState = false;
  if (pendingToggle && clickDelivered) {
    bool nowChecked = false; pendingToggle->GetChecked(&nowChecked);
    fprintf(stderr, "[jihad-bs] control click delivered — engine owns the toggle (checked=%d)\n",
            (int)nowChecked);
  } else if (pendingToggle) {
    bool wanted = toggleIsRadio ? true : !toggleWasChecked;
    if (wanted != toggleWasChecked) {
      fprintf(stderr, "[jihad-bs] NO click reached the control — hand-flipping (F-2 fallback)\n");
      pendingToggle->SetChecked(wanted);
      bool ind = false; pendingToggle->GetIndeterminate(&ind);
      if (ind) pendingToggle->SetIndeterminate(false);            // F-007
      toggledCtrl = pendingToggle; toggledState = wanted;
    }
  }
  if (toggledCtrl) {
    nsCOMPtr<nsIDOMNode> cn = do_QueryInterface(toggledCtrl);
    nsCOMPtr<nsIDOMDocument> cdoc; if (cn) cn->GetOwnerDocument(getter_AddRefs(cdoc));
    nsCOMPtr<nsIDOMEventTarget> ct = do_QueryInterface(toggledCtrl);
    if (cdoc && ct) {
      nsCOMPtr<nsIDOMEvent> iev; cdoc->CreateEvent(NS_LITERAL_STRING("Event"), getter_AddRefs(iev));
      if (iev) { iev->InitEvent(NS_LITERAL_STRING("input"),  true, false); bool d = false; ct->DispatchEvent(iev, &d); }
      nsCOMPtr<nsIDOMEvent> cev; cdoc->CreateEvent(NS_LITERAL_STRING("Event"), getter_AddRefs(cev));
      if (cev) { cev->InitEvent(NS_LITERAL_STRING("change"), true, false); bool d = false; ct->DispatchEvent(cev, &d); }
    }
    fprintf(stderr, "[jihad-bs] toggled control -> %d\n", toggledState);
  }
  // A tapped SUBMIT control (search "Go" button, login submit). This used to be UNCONDITIONAL,
  // because the click's submit default action never ran offscreen. Now it does (T-067), so an
  // unconditional FireFormSubmit here would submit the form TWICE — two loads racing, and on a
  // POST form a genuinely duplicated request. So it is now a FALLBACK, taken only if nothing
  // actually started loading.
  //
  // The docshell's own busy flags are the check, not `mLinkClicked`: mLinkClicked is set from the
  // progress listener, and a submit begun inside the click dispatch has not necessarily reported
  // STATE_START by the time this line runs, whereas the docshell is marked busy synchronously
  // when the load is initiated. Both are consulted, so either signal suppresses the fallback.
  // (That interlock's TIMING was re-verified in review — OnLinkClickSync sets mBusyFlags
  // synchronously inside the mouseup dispatch — and is deliberately left alone.)
  //
  // F-3: busy flags alone were never enough, because "nothing started loading" also describes
  // every LEGITIMATE refusal — an `onsubmit` that returns false (the handler then ran twice per
  // tap), a preventDefault()ed submit click (which still navigated), and a DISABLED button (which
  // submitted, because the classifier read the `type` attribute and never asked GetDisabled the
  // way the checkbox branch does). So the fallback now requires, in addition: no click was
  // delivered to the control at all, and the control is not disabled.
  uint32_t busy = nsIDocShell::BUSY_FLAGS_NONE;
  { nsCOMPtr<nsIDocShell> bds = GetDocShell(mChrome->mBrowser);
    if (bds) bds->GetBusyFlags(&busy); }
  bool navStarted = (busy != nsIDocShell::BUSY_FLAGS_NONE) ||
                    (mChrome->mLinkClicked && mChrome->mLinkClicked != navBefore);
  if (navStarted) {
    fprintf(stderr, "[jihad-bs] click started a load (busy=0x%x) — no fallback submit\n",
            (unsigned)busy);
  }
  {
    // Classify first, gate after, so the "declined" diagnostic below is only printed for a tap
    // that really was on a submit control — the whole point of F-3 is telling a refusal apart
    // from a dropped event, and a log line on every ordinary tap would bury it.
    nsCOMPtr<nsIDOMHTMLFormElement> sform;
    { nsAutoString tg2; if (effEl) effEl->GetTagName(tg2);
      std::string t2 = NS_ConvertUTF16toUTF8(tg2).get();
      for (char& c : t2) c = (char)toupper((unsigned char)c);
      if (t2 == "BUTTON") {
        nsCOMPtr<nsIDOMHTMLButtonElement> btn = do_QueryInterface(effEl);
        nsAutoString bt; if (effEl) effEl->GetAttribute(NS_LITERAL_STRING("type"), bt);
        std::string bts = NS_ConvertUTF16toUTF8(bt).get();
        for (char& c : bts) c = (char)tolower((unsigned char)c);
        // GetDisabled, not the attribute: it also catches a button inside a <fieldset disabled>
        // (F-3, mirroring the checkbox branch's F-007 check). A disabled control receives no
        // click, so without this the "no click arrived" test would read as a dropped event.
        bool bdis = false; if (btn) btn->GetDisabled(&bdis);
        if (btn && !bdis && (bts.empty() || bts == "submit")) btn->GetForm(getter_AddRefs(sform));   // default type is submit
      } else if (t2 == "INPUT") {
        nsAutoString it; if (effEl) effEl->GetAttribute(NS_LITERAL_STRING("type"), it);
        std::string its = NS_ConvertUTF16toUTF8(it).get();
        for (char& c : its) c = (char)tolower((unsigned char)c);
        if (its == "submit" || its == "image") {
          nsCOMPtr<nsIDOMHTMLInputElement> ib = do_QueryInterface(effEl);
          bool idis = false; if (ib) ib->GetDisabled(&idis);          // F-3, as above
          if (ib && !idis) ib->GetForm(getter_AddRefs(sform));
        }
        // (checkbox/radio are flipped BEFORE the click above — F-001 — not here.)
      }
    }
    if (sform && clickDelivered) {
      fprintf(stderr, "[jihad-bs] submit control got its click and the form did not submit — the "
                      "page's decision (onsubmit/preventDefault), not a dropped event (F-3)\n");
    } else if (sform && !navStarted) {
      fprintf(stderr, "[jihad-bs] NO click reached the submit control — falling back (F-3)\n");
      FireFormSubmit(sform);
    }
    // XUL needs NOTHING here (T-067/R6). The 2026-07-20 note that used to sit at this line said
    // XUL `oncommand` activation had to be hand-fired and that doing so crashed the daemon. Both
    // halves were consequences of the dropped-event defect: with the events actually reaching the
    // document, `nsButtonBoxFrame` synthesises the `command` itself off the real click, exactly as
    // it does in a normal browser. Measured on the headless build: a tap on about:config's
    // "I promise to be careful!" reaches `oncommand@about:config` -> `ShowPrefs@config.js:346`
    // and the prefs tree replaces the warning deck (xul_test phase D, default click coordinate).
    // No synthetic command event, and no crash ON DESKTOP — the device has not run this path since
    // the change, and the XUL frame code it now reaches there (holdAt -> contextmenu -> menupopup,
    // tree frames) is more than has ever run on a widget probe that reports theme components
    // ABSENT. Treat "no crash" as a desktop result until a device session says otherwise (F-4).
  }
}

bool GoannaRenderPage::TakeEditorFocus(bool* focused, int* fieldType, int* fieldActions) {
  if (!mEditorFocusDirty) return false;
  mEditorFocusDirty = false;
  if (focused) *focused = mEditorFocused;
  if (fieldType) *fieldType = mEditorFieldType;
  if (fieldActions) *fieldActions = 0;
  return true;
}

bool GoannaRenderPage::ClearEditorFocus() {
  bool was = mEditorFocused;
  mEditorFocused = false;
  mEditorFocusDirty = false;   // any pending change is superseded by the navigation
  if (mChrome) {
    mChrome->mFocusedEditable = nullptr;   // the field is no longer the type target
    mChrome->mEngineFocusEvent = false;    // pending engine focus/blur belongs to the old page
    mChrome->mEngineFocusIsText = false;
  }
  return was;
}

// (Re)register the capture-phase focus/blur pair on the CURRENT top document (Atlas IM-context
// port: the engine, not the tap heuristic, is the authority on editor focus). Called after each
// completed load; a same-document (SPA) completion re-resolves to the same target and no-ops.
// Listening on the document — not the window — survives everything but a doc swap, which is
// exactly when we re-register. Safe to call from the guarded pump only (resolves layout).
void GoannaRenderPage::RegisterEngineFocusListener() {
  if (!mChrome) return;
  nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
  if (!ds) return;
  nsCOMPtr<nsIContentViewer> cv; ds->GetContentViewer(getter_AddRefs(cv));
  if (!cv) return;
  nsCOMPtr<nsIDOMDocument> doc; cv->GetDOMDocument(getter_AddRefs(doc));
  if (!doc) return;
  nsCOMPtr<nsIDOMEventTarget> tgt = do_QueryInterface(doc);
  if (!tgt || tgt == mChrome->mFocusListenTarget) return;   // same doc: already registered
  if (mChrome->mFocusListenTarget) {
    mChrome->mFocusListenTarget->RemoveEventListener(NS_LITERAL_STRING("focus"), mChrome, true);
    mChrome->mFocusListenTarget->RemoveEventListener(NS_LITERAL_STRING("blur"), mChrome, true);
  }
  tgt->AddEventListener(NS_LITERAL_STRING("focus"), mChrome, true);
  tgt->AddEventListener(NS_LITERAL_STRING("blur"), mChrome, true);
  mChrome->mFocusListenTarget = tgt;
}

// Merge engine-observed focus/blur into the VKB state machine (drained by TakeEditorFocus).
// Emits only on CHANGE — the tap path keeps its always-re-raise semantics (the user may have
// manually dismissed the VKB), engine events don't need them. The Atlas autofocus gate drops a
// RAISE that arrives before any tap on this page (a load-time autofocus grabbing the keyboard
// would block the app's own address bar); lowers always pass — they can only unwedge (T4).
void GoannaRenderPage::PollEngineFocus() {
  if (!mChrome || !mChrome->mEngineFocusEvent) return;
  mChrome->mEngineFocusEvent = false;
  bool text = mChrome->mEngineFocusIsText;
  if (text && !mChrome->mUserInteracted) return;   // autofocus before first tap: don't raise
  if (text != mEditorFocused) {
    mEditorFocused = text;
    mEditorFieldType = 0;
    mEditorFocusDirty = true;
  }
}

bool GoannaRenderPage::TakeClickNav(std::string* url) {
  if (mClickNavUrl.empty()) return false;
  if (url) *url = mClickNavUrl;
  mClickNavUrl.clear();
  return true;
}

void GoannaRenderPage::KeyEvent(const char* type, int keyCode, int charCode, int modifiers) {
  if (!mChrome) return;
  // When the user is typing into a field we tracked via ClickAt (mFocusedEditable), DO NOT
  // dispatch a synthesized key event through the engine. SendKeyEvent routes to the focused
  // editor, and a headless PuppetWidget has no backing widget for the editor's key/caret
  // handling -> SIGSEGV (the same class of crash as Focus()). Text entry for these fields is
  // handled by InsertText (a plain DOM value mutation) via insertStringAtCursor instead.
  if (mChrome->mFocusedEditable)
    return;
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return;
  bool ret = false;
  NS_ConvertUTF8toUTF16 t(type);
  // NB: there is no sendKeyEventToWindow — this still goes through the widget and is therefore
  // still swallowed on the offscreen path (T-067). Left in place: it is harmless, and R2/R2a
  // drive the engine editor directly rather than relying on it.
  u->SendKeyEvent(t, keyCode, charCode, modifiers, 0, &ret);
}

void GoannaRenderPage::TouchEvent(const char* type, int x, int y) {
  if (!mChrome) return;
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return;
  uint32_t ids[1]   = { 0 };
  int32_t  xs[1]    = { x }, ys[1] = { y };
  uint32_t rxs[1]   = { 1 }, rys[1] = { 1 };
  float    angs[1]  = { 0.0f }, forces[1] = { 1.0f };
  bool ret = false;
  NS_ConvertUTF8toUTF16 t(type);
  u->SendTouchEventToWindow(t, ids, xs, ys, rxs, rys, angs, forces, 1, 0, false, &ret);   // T-067
}

void GoannaRenderPage::SetJavaScriptEnabled(bool enabled) {
  // Global pref (read by the JS engine when a page's context is created) — must
  // be set before the page loads. Also set the per-docShell flag when present.
  nsCOMPtr<nsIPrefBranch> pb = do_GetService("@mozilla.org/preferences-service;1");
  if (pb) pb->SetBoolPref("javascript.enabled", enabled);
  if (!mChrome) return;
  nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
  if (ds) ds->SetAllowJavascript(enabled);
}

// JSON string escape for the HitTest reply (quotes, backslash, control chars). The values
// are page-controlled (href/alt/text), so this is load-bearing, not cosmetic.
static void jihadJsonEscape(const nsAString& in, std::string* out, size_t maxLen) {
  NS_ConvertUTF16toUTF8 u8(in);
  const char* s = u8.get();
  size_t n = strlen(s);
  if (n > maxLen) {
    n = maxLen;
    // never cut a UTF-8 sequence in half — pbnjson would reject the whole document
    while (n > 0 && ((unsigned char)s[n] & 0xC0) == 0x80) --n;
  }
  for (size_t i = 0; i < n; ++i) {
    unsigned char c = (unsigned char)s[i];
    switch (c) {
      case '"':  out->append("\\\""); break;
      case '\\': out->append("\\\\"); break;
      default:
        if (c < 0x20) { char b[8]; snprintf(b, sizeof b, "\\u%04x", c); out->append(b); }
        else out->push_back((char)c);
    }
  }
}

void GoannaRenderPage::HitTestAt(int x, int y, std::string* json) {
  char head[96];
  snprintf(head, sizeof head, "{\"isNull\":true,\"x\":%d,\"y\":%d}", x, y);
  *json = head;                                  // the safe default: nothing at the point
  if (!mChrome || !mChrome->mBrowser) return;
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return;
  nsCOMPtr<nsIDOMElement> el;
  u->ElementFromPoint((float)x, (float)y, false, true, getter_AddRefs(el));
  if (!el) return;
  nsAutoString tag; el->GetTagName(tag);
  // Nearest <a href> / <img> ancestors — the press usually lands on inline content inside
  // them (same walk as clickAt; depth cap is a cycle guard).
  nsAutoString href, linkText, imgSrc, imgAlt;
  nsCOMPtr<nsIDOMNode> node = do_QueryInterface(el);
  for (int depth = 0; node && depth < 256; ++depth) {
    if (href.IsEmpty()) {
      nsCOMPtr<nsIDOMHTMLAnchorElement> a = do_QueryInterface(node);
      if (a) { a->GetHref(href); node->GetTextContent(linkText); }
    }
    if (imgSrc.IsEmpty()) {
      nsCOMPtr<nsIDOMHTMLImageElement> im = do_QueryInterface(node);
      if (im) { im->GetSrc(imgSrc); im->GetAlt(imgAlt); }
    }
    if (!href.IsEmpty() && !imgSrc.IsEmpty()) break;
    nsCOMPtr<nsIDOMNode> p; node->GetParentNode(getter_AddRefs(p)); node = p;
  }
  // Editable: tag-based, same spirit as the clickAt VKB classifier (INPUT/TEXTAREA).
  bool editable = tag.LowerCaseEqualsLiteral("input") || tag.LowerCaseEqualsLiteral("textarea");
  std::string t;
  jihadJsonEscape(tag, &t, 32);
  char buf[160];
  snprintf(buf, sizeof buf, "{\"isNull\":false,\"x\":%d,\"y\":%d,\"isLink\":%s,\"isImage\":%s,\"editable\":%s,\"element\":\"",
           x, y, href.IsEmpty() ? "false" : "true", imgSrc.IsEmpty() ? "false" : "true",
           editable ? "true" : "false");
  std::string out(buf);
  out += t; out += "\"";
  if (!href.IsEmpty())    { out += ",\"linkUrl\":\"";  jihadJsonEscape(href, &out, 512);    out += "\""; }
  if (!linkText.IsEmpty()){ out += ",\"linkText\":\""; jihadJsonEscape(linkText, &out, 256); out += "\""; }
  if (!imgSrc.IsEmpty())  { out += ",\"imageUrl\":\""; jihadJsonEscape(imgSrc, &out, 512);  out += "\""; }
  if (!imgAlt.IsEmpty())  { out += ",\"altText\":\"";  jihadJsonEscape(imgAlt, &out, 256);  out += "\""; }
  out += "}";
  *json = out;
}

// Serialize a dropdown <select>'s options to the JSON the card popup consumes, and hold a
// reference to the element so a later popupMenuSelect can apply the choice. Returns false
// (fall back to a normal click) if the select is empty or the DOM read fails.
bool GoannaRenderPage::BuildSelectPopup(nsIDOMHTMLSelectElement* aSelect) {
  if (!aSelect) return false;
  // Dedup: a single physical tap reaches ClickAt TWICE (the raw pen down/up pair delivers a
  // click, then the gesture single-tap delivers another), so without this each tap opened
  // two card popups and they dismissed each other (device 2026-08-03: every tap returned -1).
  // Suppress a second popup for the SAME element within a short window.
  long now = jihadInputNowMs();
  if (aSelect == mSelectPopupEl && (now - mSelectPopupMs) < 700) return false;
  mSelectPopupMs = now;
  nsCOMPtr<nsIDOMHTMLOptionsCollection> opts;
  aSelect->GetOptions(getter_AddRefs(opts));
  if (!opts) return false;
  uint32_t len = 0; opts->GetLength(&len);
  if (len == 0) return false;
  int32_t selIdx = -1; aSelect->GetSelectedIndex(&selIdx);
  // The JSON shape is CONTRACT, not ours to invent: the card side is the stock
  // enyo.WebView.createSelectPopup, which reads items[].text / items[].isEnabled
  // (isis BrowserComboBox.cpp createJSONItem — text/isEnabled/isSeparator/isLabel
  // + top-level selectedIdx). Our first cut wrote label/enabled/selected and the
  // framework built a list of undefined captions — the "empty popup" of
  // impl-select-popup-2026-08-03.md.
  std::string json = "{\"items\":[";
  for (uint32_t i = 0; i < len; ++i) {
    nsCOMPtr<nsIDOMNode> node; opts->Item(i, getter_AddRefs(node));
    nsCOMPtr<nsIDOMHTMLOptionElement> opt = do_QueryInterface(node);
    nsAutoString label; bool odis = false;
    if (opt) {
      opt->GetLabel(label);                       // label attr, or the text if unset...
      if (label.IsEmpty()) opt->GetText(label);   // ...which is the common case
      opt->GetDisabled(&odis);
    }
    if (i) json += ",";
    json += "{\"text\":\"";
    jihadJsonEscape(label, &json, 200);
    json += "\",\"isEnabled\":";
    json += odis ? "false" : "true";
    // nsIDOMHTMLOptionsCollection yields <option>s only, so neither shows up here;
    // emitted anyway because the isis file always carries them.
    json += ",\"isSeparator\":false,\"isLabel\":false}";
  }
  json += "],\"selectedIdx\":" + std::to_string((int)selIdx);
  // ADDITIVE to the isis shape (the framework's enyo.json.parse ignores unknown keys):
  // the <select>'s rect in CARD/plugin px, so the card can anchor the popup under the
  // box instead of screen-centre. The stock anchor path (enyo.WebView._selectRect via
  // click info) is dead in this embedding — BasicWebView publishes no onClick — so the
  // daemon, which owns the element, ships the geometry. Mapping is the exact inverse of
  // BrowserPageGoanna::docToViewport: at z~1 card px == client-rect px (scroll already
  // excluded by GetBoundingClientRect); zoomed, engine scroll parks at 0 and the visual-
  // viewport pan carries the offset, so card = (client + pan) * z.
  {
    nsCOMPtr<nsIDOMElement> el = do_QueryInterface(aSelect);
    nsCOMPtr<nsIDOMClientRect> r;
    if (el) el->GetBoundingClientRect(getter_AddRefs(r));
    if (r) {
      float fL = 0, fT = 0, fR = 0, fB = 0;
      r->GetLeft(&fL); r->GetTop(&fT); r->GetRight(&fR); r->GetBottom(&fB);
      double L = fL, T = fT, R = fR, B = fB;
      double z = (mRenderZoom >= 0.99 && mRenderZoom <= 1.01) ? 1.0
               : ((mRenderZoom >= 0.05 && mRenderZoom <= 20.0) ? mRenderZoom : 1.0);
      if (z != 1.0) { L = (L + mPanX) * z; R = (R + mPanX) * z;
                      T = (T + mPanY) * z; B = (B + mPanY) * z; }
      json += ",\"rect\":{\"left\":"   + std::to_string((int)(L + 0.5)) +
              ",\"top\":"    + std::to_string((int)(T + 0.5)) +
              ",\"right\":"  + std::to_string((int)(R + 0.5)) +
              ",\"bottom\":" + std::to_string((int)(B + 0.5)) + "}";
    }
  }
  json += "}";
  mSelectPopupJson = json;
  mSelectPopupPending = true;
  SetSelectPopupEl(aSelect);                       // held (AddRef'd) until popupMenuSelect / nav
  // Process-global like isis's BrowserComboBox idSeq (static int): a per-page counter
  // restarts at 0 when the daemon rebuilds the page (e.g. the card's auto-reconnect),
  // and the framework CACHES popups by id — a reused id reopens the stale cached list
  // and replies with an index into it (Opus review #7).
  static unsigned sSelectPopupSeq = 0;
  mSelectPopupId = std::string("sel") + std::to_string(++sSelectPopupSeq);
  return true;
}

// Strong-ref swap for the held <select> (the header keeps XPCOM types opaque, so no nsCOMPtr).
void GoannaRenderPage::SetSelectPopupEl(nsIDOMHTMLSelectElement* el) {
  if (el == mSelectPopupEl) return;
  if (el) el->AddRef();
  if (mSelectPopupEl) mSelectPopupEl->Release();
  mSelectPopupEl = el;
}

bool GoannaRenderPage::TakeSelectPopup(std::string* json, std::string* id) {
  if (!mSelectPopupPending) return false;
  mSelectPopupPending = false;
  if (json) *json = mSelectPopupJson;
  if (id)   *id   = mSelectPopupId;
  return true;
}

void GoannaRenderPage::ApplySelectPopup(const char* id, int idx) {
  // Ignore a stale reply (a newer popup, or the page navigated and dropped the element).
  if (!mSelectPopupEl || !id || mSelectPopupId != id) return;
  // The card's PopupList is NOT disabled-aware (its listSetupRow never reads the
  // `disabled` flag), and page JS can rewrite the option list while the popup is
  // up — so the daemon is the ONLY enforcement point (Opus review #1/#2/#3/#4):
  // refuse out-of-range indexes, options disabled directly or via an enclosing
  // disabled <optgroup>, and no-op picks (re-selecting the current option must
  // not fire change — real browsers don't).
  bool apply = false;
  if (idx >= 0) {
    nsCOMPtr<nsIDOMHTMLOptionsCollection> opts;
    mSelectPopupEl->GetOptions(getter_AddRefs(opts));
    uint32_t len = 0; if (opts) opts->GetLength(&len);
    int32_t cur = -1; mSelectPopupEl->GetSelectedIndex(&cur);
    if (opts && (uint32_t)idx < len && idx != cur) {
      nsCOMPtr<nsIDOMNode> node; opts->Item((uint32_t)idx, getter_AddRefs(node));
      nsCOMPtr<nsIDOMHTMLOptionElement> opt = do_QueryInterface(node);
      bool dis = false; if (opt) opt->GetDisabled(&dis);
      if (opt && !dis && node) {          // walk to the <select>: a disabled <optgroup> disables its options
        nsCOMPtr<nsIDOMNode> up; node->GetParentNode(getter_AddRefs(up));
        for (int hop = 0; !dis && up && hop < 4; ++hop) {
          if (SameCOMIdentity(up, mSelectPopupEl)) break;
          nsCOMPtr<nsIDOMHTMLOptGroupElement> og = do_QueryInterface(up);
          if (og) og->GetDisabled(&dis);
          nsCOMPtr<nsIDOMNode> next; up->GetParentNode(getter_AddRefs(next)); up = next;
        }
      }
      apply = opt && !dis;
    }
    if (!apply) fprintf(stderr, "[jihad-bs] popupMenuSelect idx=%d refused (len=%u cur=%d)\n",
                        idx, len, (int)cur);
  }
  if (apply) {
    mSelectPopupEl->SetSelectedIndex(idx);
    // Fire input+change so page JS (framework onChange) reacts, exactly as a real pick would.
    // Same dispatch pattern as FlushPendingInputEvent (bubbling, non-trusted DOM Event).
    nsCOMPtr<nsIDOMNode> node = do_QueryInterface(mSelectPopupEl);
    nsCOMPtr<nsIDOMEventTarget> tgt = do_QueryInterface(mSelectPopupEl);
    nsCOMPtr<nsIDOMDocument> doc; if (node) node->GetOwnerDocument(getter_AddRefs(doc));
    if (doc && tgt) {
      const char* kEvents[] = { "input", "change" };
      for (const char* name : kEvents) {
        nsCOMPtr<nsIDOMEvent> ev;
        doc->CreateEvent(NS_LITERAL_STRING("Event"), getter_AddRefs(ev));
        if (!ev) continue;
        ev->InitEvent(NS_ConvertUTF8toUTF16(name), true, false);
        bool dummy = false; tgt->DispatchEvent(ev, &dummy);
      }
    }
  }
  SetSelectPopupEl(nullptr);
}

// Draw any open XUL popup over a buffer the caller has already painted. docX/docY are
// the document CSS coords of its top-left and zoom the scale it was rendered at, so the
// popup lands with the same transform as the content under it. 0 = nothing was open,
// which is the normal case and costs one popup-manager query.
int GoannaRenderPage::CompositePopups(unsigned char* dst, int stride, int w, int h,
                                      double docX, double docY, double zoom) {
  if (!mOffscreen || !dst || w <= 0 || h <= 0) return 0;
  if (!jihad_offscreen_composite_popups) return 0;   // old libxul: weak symbol absent
  return jihad_offscreen_composite_popups(mWidget, dst, stride, w, h, docX, docY, zoom);
}

bool GoannaRenderPage::RenderRegion(unsigned char* dst, int stride, int w, int h,
                                    double docX, double docY, double zoom) {
  if (!mOffscreen || !dst || w <= 0 || h <= 0) return false;
  if (!jihad_offscreen_render_region) return false;   // old libxul: weak symbol absent
  if (!mChrome || !mChrome->mBrowser) return false;
  nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
  if (!ds) return false;
  return jihad_offscreen_render_region(mWidget, ds, zoom, docX, docY, dst, stride, w, h);
}

long GoannaRenderPage::ReadPixels(unsigned char* dst, size_t dstBytes) {
  if (mOffscreen) {
    if (!mWidget) return -1;
    int w = mWidth, h = mHeight;
    if (w <= 0 || h <= 0) return -1;
    if (dstBytes < (size_t)w * h * 4) return -1;
    // Render the document into the widget's DrawTarget via the presShell (the widget
    // Paint() path does not paint embedded content into our offscreen widget -> black).
    if (mChrome && mChrome->mBrowser) {
      nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
      if (ds) {
        // Flush layout BEFORE rendering. A paint triggered right after load-done (or any
        // time reflow is still pending) otherwise captures un-laid-out content -> a blank
        // frame, and the card stays black until a later resize/refocus forces a repaint
        // ("black until you back out of the card and return"). Flushing makes every paint
        // capture the current laid-out state.
        nsCOMPtr<nsIPresShell> ps = ds->GetPresShell();
        if (ps) ps->FlushPendingNotifications(Flush_Layout);
        // A failed render leaves the target as the white FillRect -> readback is all-white
        // and would be blitted as a blank frame. Treat it as "no frame" so the caller keeps
        // the last good frame instead (review #7 P2).
        if (!jihad_offscreen_render_document_v2(mWidget, ds, mRenderZoom, mPanX, mPanY)) return -1;
      }
    }
    // PuppetWidget's DrawTarget is B8G8R8A8 == the ARGB32 LE (B,G,R,A) shmem layout.
    if (!jihad_offscreen_readback(mWidget, dst, w * 4, w, h)) return -1;
    long nonblank = 0;
    for (int i = 0; i < w * h; ++i) {
      unsigned char* o = dst + (size_t)i * 4;   // B,G,R,A
      if (!(o[0] > 240 && o[1] > 240 && o[2] > 240)) ++nonblank;
      o[3] = 0xff;
    }
    return nonblank;
  }
#ifdef JIHAD_OFFSCREEN_ONLY
  return -1;   // device build: only the offscreen path exists
}
#else
  if (!mWindow) return -1;
  GdkWindow* gw = gtk_widget_get_window(mWindow);
  if (!gw) return -1;
  gint w = 0, h = 0;
  gdk_drawable_get_size(GDK_DRAWABLE(gw), &w, &h);
  if (dstBytes < (size_t)w * h * 4) return -1;
  GdkPixbuf* pb = gdk_pixbuf_get_from_drawable(nullptr, GDK_DRAWABLE(gw),
                    gdk_colormap_get_system(), 0, 0, 0, 0, w, h);
  if (!pb) return -1;
  int nch = gdk_pixbuf_get_n_channels(pb);
  int rs = gdk_pixbuf_get_rowstride(pb);
  guchar* px = gdk_pixbuf_get_pixels(pb);
  long nonblank = 0;
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      guchar* p = px + y * rs + x * nch;
      unsigned char* o = dst + ((size_t)y * w + x) * 4;
      o[0] = p[2]; o[1] = p[1]; o[2] = p[0]; o[3] = 0xff;   // B,G,R,A (ARGB32 LE)
      if (!(p[0] > 240 && p[1] > 240 && p[2] > 240)) ++nonblank;
    }
  }
  g_object_unref(pb);
  return nonblank;
}
#endif // !JIHAD_OFFSCREEN_ONLY

} // namespace jihad
