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
#include "nsIWebProgress.h"
#include "nsIWebProgressListener.h"
#include "nsIURI.h"
#include "nsWeakReference.h"
#include "nsIWeakReferenceUtils.h"
#include "nsThreadUtils.h"
#include "nsIThread.h"
#include "nsIDOMWindowUtils.h"
#include "mozIDOMWindow.h"
#include "nsIInterfaceRequestor.h"

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

  PageChrome(int w, int h) : mDone(false), mW(w), mH(h) {}
  bool mDone;
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
NS_IMETHODIMP PageChrome::OnStateChange(nsIWebProgress*, nsIRequest*, uint32_t f, nsresult) {
  if ((f & STATE_STOP) && (f & (STATE_IS_WINDOW | STATE_IS_NETWORK))) mDone = true;
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

bool GoannaRenderPage::LoadUrl(const char* url) {
  if (!mChrome) return false;
  nsCOMPtr<nsIWebNavigation> nav = do_QueryInterface(mChrome->mBrowser);
  if (!nav) return false;
  mChrome->mDone = false;
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

void GoannaRenderPage::GoBack()    { if (mChrome) { nsCOMPtr<nsIWebNavigation> n = do_QueryInterface(mChrome->mBrowser); if (n) n->GoBack(); } }
void GoannaRenderPage::GoForward() { if (mChrome) { nsCOMPtr<nsIWebNavigation> n = do_QueryInterface(mChrome->mBrowser); if (n) n->GoForward(); } }
void GoannaRenderPage::Reload()    { if (mChrome) { nsCOMPtr<nsIWebNavigation> n = do_QueryInterface(mChrome->mBrowser); if (n) n->Reload(nsIWebNavigation::LOAD_FLAGS_NONE); } }
void GoannaRenderPage::Stop()      { if (mChrome) { nsCOMPtr<nsIWebNavigation> n = do_QueryInterface(mChrome->mBrowser); if (n) n->Stop(nsIWebNavigation::STOP_ALL); } }

bool GoannaRenderPage::LoadDone() const { return mChrome && mChrome->mDone; }

std::string GoannaRenderPage::CurrentUri() {
  if (!mChrome) return std::string();
  nsCOMPtr<nsIWebNavigation> nav = do_QueryInterface(mChrome->mBrowser);
  if (!nav) return std::string();
  nsCOMPtr<nsIURI> uri; nav->GetCurrentURI(getter_AddRefs(uri));
  if (!uri) return std::string();
  nsCString spec; uri->GetSpec(spec);
  return std::string(spec.get());
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
