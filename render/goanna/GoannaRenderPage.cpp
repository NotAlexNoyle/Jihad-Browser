/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — GoannaRenderPage implementation. See GoannaRenderPage.h.
 * Consolidates the proven embed_load pipeline (T-019/T-020/T-024) into a class.
 */
#include "GoannaRenderPage.h"
#include "EngineHost.h"

#include <gtk/gtk.h>
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
#include "nsIWebProgress.h"
#include "nsIWebProgressListener.h"
#include "nsIURI.h"
#include "nsIChannel.h"
#include "nsIDOMClientRect.h"
#include "nsWeakReference.h"
#include "nsIWeakReferenceUtils.h"
#include "nsThreadUtils.h"
#include "nsIThread.h"
#include "nsIDOMWindowUtils.h"
#include "mozIDOMWindow.h"
#include "nsIInterfaceRequestor.h"
#include "nsIDocShell.h"
#include "nsIContentViewer.h"
#include "nsIPrefBranch.h"
#include "nsICookieManager.h"
#include "nsICacheStorageService.h"
#include "nsServiceManagerUtils.h"

namespace jihad {

// XPCOM chrome + progress listener for one page. Refcounted; owns the browser.
class PageChrome final : public nsIWebBrowserChrome,
                         public nsIEmbeddingSiteWindow,
                         public nsIInterfaceRequestor,
                         public nsIWebProgressListener,
                         public nsSupportsWeakReference
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIWEBBROWSERCHROME
  NS_DECL_NSIEMBEDDINGSITEWINDOW
  NS_DECL_NSIINTERFACEREQUESTOR
  NS_DECL_NSIWEBPROGRESSLISTENER

  PageChrome(int w, int h) : mDone(false), mLoadFailed(false),
                             mErrorStatus(NS_OK), mW(w), mH(h) {}
  bool mDone;
  bool mLoadFailed;          // last load ended in a network error
  nsresult mErrorStatus;     // the failing nsresult
  nsCString mFailedUrl;      // URL that failed
  int32_t mW, mH;
  nsCOMPtr<nsIWebBrowser> mBrowser;
private:
  ~PageChrome() {}
};

NS_IMPL_ISUPPORTS(PageChrome, nsIWebBrowserChrome, nsIEmbeddingSiteWindow,
                  nsIInterfaceRequestor, nsIWebProgressListener,
                  nsISupportsWeakReference)

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

NS_IMETHODIMP PageChrome::OnStateChange(nsIWebProgress*, nsIRequest* aRequest, uint32_t f, nsresult aStatus) {
  if (f & STATE_STOP) {
    // A failing stop status is a failed load (R3). Scope it to the main document
    // (STATE_IS_DOCUMENT) so a broken subresource doesn't mark the page failed;
    // ignore NS_BINDING_ABORTED (a cancelled nav, can arrive late) and only take
    // the first failure of the load in progress (BeginLoad resets between loads).
    if (NS_FAILED(aStatus) && aStatus != kBindingAborted &&
        (f & STATE_IS_DOCUMENT) && !mLoadFailed && !mDone) {
      mLoadFailed = true;
      mErrorStatus = aStatus;
      nsCOMPtr<nsIChannel> ch = do_QueryInterface(aRequest);
      if (ch) {
        nsCOMPtr<nsIURI> u; ch->GetURI(getter_AddRefs(u));
        if (u) u->GetSpec(mFailedUrl);
      }
    }
    if (f & (STATE_IS_WINDOW | STATE_IS_NETWORK)) mDone = true;
  }
  return NS_OK;
}
NS_IMETHODIMP PageChrome::OnProgressChange(nsIWebProgress*, nsIRequest*, int32_t, int32_t, int32_t, int32_t) { return NS_OK; }
NS_IMETHODIMP PageChrome::OnLocationChange(nsIWebProgress*, nsIRequest*, nsIURI*, uint32_t) { return NS_OK; }
NS_IMETHODIMP PageChrome::OnStatusChange(nsIWebProgress*, nsIRequest*, nsresult, const char16_t*) { return NS_OK; }
NS_IMETHODIMP PageChrome::OnSecurityChange(nsIWebProgress*, nsIRequest*, uint32_t) { return NS_OK; }

// ── GoannaRenderPage ────────────────────────────────────────────────────────

GoannaRenderPage::GoannaRenderPage(EngineHost& host)
  : mHost(host), mChrome(nullptr), mWindow(nullptr), mWidth(0), mHeight(0) {}

GoannaRenderPage::~GoannaRenderPage() {
  // Ordered teardown (Codex P1): stop navigation, remove the progress listener,
  // clear the container window, destroy the browser + base window, then release
  // the chrome and the native GTK window. All browser refs must be gone before
  // the engine's XRE_TermEmbedding runs.
  nsCOMPtr<nsIWebBrowser> wb = mChrome ? mChrome->mBrowser : nullptr;
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
  if (mWindow) { gtk_widget_destroy(mWindow); mWindow = nullptr; }
}

bool GoannaRenderPage::Create(int width, int height) {
  mWidth = width; mHeight = height;
  RefPtr<PageChrome> chrome = new PageChrome(width, height);

  nsCOMPtr<nsIWebBrowser> wb = mHost.CreateBrowser();
  if (!wb) return false;
  chrome->mBrowser = wb;
  wb->SetContainerWindow(static_cast<nsIWebBrowserChrome*>(chrome));

  mWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(mWindow), width, height);
  gtk_widget_realize(mWindow);

  nsCOMPtr<nsIBaseWindow> baseWin = do_QueryInterface(wb);
  if (!baseWin) return false;
  if (NS_FAILED(baseWin->InitWindow(mWindow, nullptr, 0, 0, width, height))) return false;
  baseWin->Create();
  baseWin->SetVisibility(true);
  gtk_widget_show_all(mWindow);
  while (gtk_events_pending()) gtk_main_iteration_do(FALSE);

  nsCOMPtr<nsIWeakReference> weak = do_GetWeakReference(static_cast<nsIWebBrowserChrome*>(chrome));
  wb->AddWebBrowserListener(weak, NS_GET_IID(nsIWebProgressListener));

  mChrome = chrome.forget().take();   // GoannaRenderPage keeps a strong ref
  return true;
}

bool GoannaRenderPage::Resize(int width, int height) {
  if (!mChrome || !mWindow) return false;
  if (width <= 0 || height <= 0 || width > 8192 || height > 8192) return false;
  mWidth = width; mHeight = height;
  mChrome->mW = width; mChrome->mH = height;   // keep GetDimensions() consistent
  gtk_widget_set_size_request(mWindow, width, height);
  gtk_window_resize(GTK_WINDOW(mWindow), width, height);
  // Resize the embedded surface; eRepaint invalidates so the page reflows to the
  // new viewport (100vw/100vh content follows the new size).
  nsCOMPtr<nsIBaseWindow> bw = do_QueryInterface(mChrome->mBrowser);
  if (bw) bw->SetPositionAndSize(0, 0, width, height, nsIBaseWindow::eRepaint);
  while (gtk_events_pending()) gtk_main_iteration_do(FALSE);
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
  nav->LoadURI(u.get(), nsIWebNavigation::LOAD_FLAGS_NONE, nullptr, nullptr, nullptr);
}

bool GoannaRenderPage::GetScrollXY(int* x, int* y) {
  if (!mChrome) return false;
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return false;
  int32_t sx = 0, sy = 0;
  if (NS_FAILED(u->GetScrollXY(/*flushLayout*/true, &sx, &sy))) return false;
  if (x) *x = sx; if (y) *y = sy;
  return true;
}

void GoannaRenderPage::SetZoom(double zoom) {
  if (!mChrome || zoom <= 0.0) return;
  nsCOMPtr<nsIDocShell> ds = GetDocShell(mChrome->mBrowser);
  if (!ds) return;
  nsCOMPtr<nsIContentViewer> cv;
  ds->GetContentViewer(getter_AddRefs(cv));
  if (!cv) return;
  cv->SetFullZoom((float)zoom);                        // full-page zoom -> reflow
  // Zoom doesn't resize the native window, so nudge a repaint to refresh readback.
  nsCOMPtr<nsIBaseWindow> bw = do_QueryInterface(mChrome->mBrowser);
  if (bw) bw->Repaint(true);
}

bool GoannaRenderPage::GetContentSize(int* w, int* h) {
  if (!mChrome) return false;
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
  mChrome->mErrorStatus = NS_OK;
  mChrome->mFailedUrl.Truncate();
}

bool GoannaRenderPage::LoadUrl(const char* url) {
  if (!mChrome) return false;
  nsCOMPtr<nsIWebNavigation> nav = do_QueryInterface(mChrome->mBrowser);
  if (!nav) return false;
  BeginLoad();
  NS_ConvertUTF8toUTF16 u(url);
  return NS_SUCCEEDED(nav->LoadURI(u.get(), nsIWebNavigation::LOAD_FLAGS_NONE, nullptr, nullptr, nullptr));
}

void GoannaRenderPage::PumpFor(int msBudget) {
  nsCOMPtr<nsIThread> thread;
  NS_GetCurrentThread(getter_AddRefs(thread));
  struct timespec ts0; clock_gettime(CLOCK_MONOTONIC, &ts0);
  for (;;) {
    NS_ProcessNextEvent(thread, false);
    while (gtk_events_pending()) gtk_main_iteration_do(FALSE);
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
    while (gtk_events_pending()) gtk_main_iteration_do(FALSE);
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

// --- process-global browser services ---------------------------------------
void SetUserAgentOverride(const char* ua) {
  nsCOMPtr<nsIPrefBranch> pb = do_GetService("@mozilla.org/preferences-service;1");
  if (pb) pb->SetCharPref("general.useragent.override", ua ? ua : "");
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

void GoannaRenderPage::MouseEvent(const char* type, int x, int y, int button) {
  if (!mChrome) return;
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return;
  bool ret = false;
  NS_ConvertUTF8toUTF16 t(type);
  // Optional args left at IDL defaults (_argc = 0).
  u->SendMouseEvent(t, (float)x, (float)y, button, /*clickCount*/1, /*mods*/0,
                    false, 0.0f, 0, false, false, 0, 0, &ret);
}

void GoannaRenderPage::ClickAt(int x, int y, int numClicks) {
  if (!mChrome) return;
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return;
  bool ret = false;
  NS_ConvertUTF8toUTF16 down("mousedown"), up("mouseup");
  u->SendMouseEvent(down, (float)x, (float)y, 0, numClicks, 0, false, 0.0f, 0, false, false, 0, 0, &ret);
  u->SendMouseEvent(up,   (float)x, (float)y, 0, numClicks, 0, false, 0.0f, 0, false, false, 0, 0, &ret);
}

void GoannaRenderPage::KeyEvent(const char* type, int keyCode, int charCode, int modifiers) {
  if (!mChrome) return;
  nsCOMPtr<nsIDOMWindowUtils> u = GetWindowUtils(mChrome->mBrowser);
  if (!u) return;
  bool ret = false;
  NS_ConvertUTF8toUTF16 t(type);
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
  u->SendTouchEvent(t, ids, xs, ys, rxs, rys, angs, forces, 1, 0, false, &ret);
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

long GoannaRenderPage::ReadPixels(unsigned char* dst, size_t dstBytes) {
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

} // namespace jihad
