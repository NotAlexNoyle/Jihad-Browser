/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — page-load test (T-019 event loop + navigation).
 *
 * Stands up a minimal embedder: an (invisible) GTK window, an nsIWebBrowser
 * with a tiny chrome, loads a data: URL, pumps the event loop until the load
 * reaches STATE_STOP, and reports the final URI. Proves Goanna fetches/parses/
 * lays out a real page inside our embedder. Run under Xvfb (no real display).
 *
 * Usage: xvfb-run embed_load <greDir>
 */
#include <gtk/gtk.h>
#include <cstdio>
#include <ctime>

#include "../EngineHost.h"
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
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"
#include "nsServiceManagerUtils.h"

// Minimal chrome: just enough for nsWebBrowser to operate, plus a progress
// listener to detect load completion.
class Chrome final : public nsIWebBrowserChrome,
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

  Chrome() : mDone(false), mW(1024), mH(768) {}
  bool mDone;
  int32_t mW, mH;
  nsCOMPtr<nsIWebBrowser> mBrowser;
private:
  ~Chrome() {}
};

NS_IMPL_ISUPPORTS(Chrome,
                  nsIWebBrowserChrome,
                  nsIEmbeddingSiteWindow,
                  nsIInterfaceRequestor,
                  nsIWebProgressListener,
                  nsISupportsWeakReference)

// nsIInterfaceRequestor
NS_IMETHODIMP Chrome::GetInterface(const nsIID& aIID, void** aResult) {
  return QueryInterface(aIID, aResult);
}

// nsIWebBrowserChrome
NS_IMETHODIMP Chrome::SetStatus(uint32_t, const char16_t*) { return NS_OK; }
NS_IMETHODIMP Chrome::GetWebBrowser(nsIWebBrowser** aB) { NS_IF_ADDREF(*aB = mBrowser); return NS_OK; }
NS_IMETHODIMP Chrome::SetWebBrowser(nsIWebBrowser* aB) { mBrowser = aB; return NS_OK; }
NS_IMETHODIMP Chrome::GetChromeFlags(uint32_t* aF) { *aF = 0; return NS_OK; }
NS_IMETHODIMP Chrome::SetChromeFlags(uint32_t) { return NS_OK; }
NS_IMETHODIMP Chrome::DestroyBrowserWindow() { return NS_OK; }
NS_IMETHODIMP Chrome::SizeBrowserTo(int32_t, int32_t) { return NS_OK; }
NS_IMETHODIMP Chrome::ShowAsModal() { return NS_OK; }
NS_IMETHODIMP Chrome::IsWindowModal(bool* aR) { *aR = false; return NS_OK; }
NS_IMETHODIMP Chrome::ExitModalEventLoop(nsresult) { return NS_OK; }

// nsIEmbeddingSiteWindow
NS_IMETHODIMP Chrome::SetDimensions(uint32_t, int32_t, int32_t, int32_t, int32_t) { return NS_OK; }
NS_IMETHODIMP Chrome::GetDimensions(uint32_t, int32_t* x, int32_t* y, int32_t* cx, int32_t* cy) {
  if (x) *x = 0; if (y) *y = 0; if (cx) *cx = mW; if (cy) *cy = mH; return NS_OK;
}
NS_IMETHODIMP Chrome::SetFocus() { return NS_OK; }
NS_IMETHODIMP Chrome::GetVisibility(bool* aV) { *aV = true; return NS_OK; }
NS_IMETHODIMP Chrome::SetVisibility(bool) { return NS_OK; }
NS_IMETHODIMP Chrome::GetTitle(char16_t** aT) { *aT = nullptr; return NS_OK; }
NS_IMETHODIMP Chrome::SetTitle(const char16_t*) { return NS_OK; }
NS_IMETHODIMP Chrome::GetSiteWindow(void** aW) { *aW = nullptr; return NS_OK; }
NS_IMETHODIMP Chrome::Blur() { return NS_OK; }

// nsIWebProgressListener
NS_IMETHODIMP Chrome::OnStateChange(nsIWebProgress*, nsIRequest*, uint32_t aFlags, nsresult) {
  // data: loads complete as STATE_IS_WINDOW; http as STATE_IS_NETWORK.
  if ((aFlags & STATE_STOP) && (aFlags & (STATE_IS_WINDOW | STATE_IS_NETWORK))) {
    mDone = true;
    printf("[embed_load] load reached STATE_STOP (flags=0x%x)\n", aFlags);
  }
  return NS_OK;
}
NS_IMETHODIMP Chrome::OnProgressChange(nsIWebProgress*, nsIRequest*, int32_t, int32_t, int32_t, int32_t) { return NS_OK; }
NS_IMETHODIMP Chrome::OnLocationChange(nsIWebProgress*, nsIRequest*, nsIURI*, uint32_t) { return NS_OK; }
NS_IMETHODIMP Chrome::OnStatusChange(nsIWebProgress*, nsIRequest*, nsresult, const char16_t*) { return NS_OK; }
NS_IMETHODIMP Chrome::OnSecurityChange(nsIWebProgress*, nsIRequest*, uint32_t) { return NS_OK; }

// Grab the rendered window pixels (what Goanna painted) and write a binary PPM.
// Returns the count of non-near-white pixels (i.e. actual rendered content).
static long CaptureWindowPPM(GtkWidget* win, const char* path) {
  GdkWindow* gw = gtk_widget_get_window(win);
  if (!gw) { printf("[embed_load] capture: no GdkWindow\n"); return -1; }
  gint w = 0, h = 0;
  gdk_drawable_get_size(GDK_DRAWABLE(gw), &w, &h);
  GdkPixbuf* pb = gdk_pixbuf_get_from_drawable(nullptr, GDK_DRAWABLE(gw),
                    gdk_colormap_get_system(), 0, 0, 0, 0, w, h);
  if (!pb) { printf("[embed_load] capture: get_from_drawable failed\n"); return -1; }

  int nch = gdk_pixbuf_get_n_channels(pb);
  int rs = gdk_pixbuf_get_rowstride(pb);
  guchar* px = gdk_pixbuf_get_pixels(pb);

  FILE* f = fopen(path, "wb");
  long nonblank = 0;
  if (f) fprintf(f, "P6\n%d %d\n255\n", w, h);
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      guchar* p = px + y * rs + x * nch;
      guchar r = p[0], g = p[1], b = p[2];
      if (!(r > 240 && g > 240 && b > 240)) ++nonblank;  // not near-white
      if (f) { fputc(r, f); fputc(g, f); fputc(b, f); }
    }
  }
  if (f) fclose(f);
  printf("[embed_load] capture: %dx%d, %ld non-white px -> %s\n", w, h, nonblank, path);
  g_object_unref(pb);
  return nonblank;
}

int main(int argc, char** argv) {
  setvbuf(stdout, nullptr, _IONBF, 0);   // unbuffered: keep trace across a crash
  setvbuf(stderr, nullptr, _IONBF, 0);
  const char* greDir = (argc > 1) ? argv[1] : ".";
  const bool tryRender = getenv("JIHAD_TRY_RENDER") != nullptr;
  printf("[embed_load] start; greDir=%s tryRender=%d\n", greDir, (int)tryRender);
  gtk_init(&argc, &argv);
  printf("[embed_load] gtk_init done\n");

  jihad::EngineHost host;
  if (!host.Init(greDir)) { fprintf(stderr, "[embed_load] FAIL: engine init\n"); return 1; }
  printf("[embed_load] engine up\n");
  // NOTE: OMTC is disabled via greprefs (goanna.js) read before init
  // (see build-embed-load.sh); gfxPlatform caches that decision at init.
  // Without it the headless embedder crashes in
  // ClientLayerManager::ForwardTransaction (no compositor process).
  {
    nsCOMPtr<nsIPrefBranch> pb = do_GetService("@mozilla.org/preferences-service;1");
    bool omtc = true;
    if (pb) pb->GetBoolPref("layers.offmainthreadcomposition.enabled", &omtc);
    printf("[embed_load] OMTC enabled pref = %d (want 0)\n", (int)omtc);
  }

  RefPtr<Chrome> chrome = new Chrome();

  nsCOMPtr<nsIWebBrowser> wb = host.CreateBrowser();
  if (!wb) { fprintf(stderr, "[embed_load] FAIL: create browser\n"); return 2; }
  chrome->mBrowser = wb;
  wb->SetContainerWindow(static_cast<nsIWebBrowserChrome*>(chrome));

  // Offscreen GTK top-level to host the browser.
  GtkWidget* win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(win), chrome->mW, chrome->mH);
  gtk_widget_realize(win);

  nsCOMPtr<nsIBaseWindow> baseWin = do_QueryInterface(wb);
  if (!baseWin) { fprintf(stderr, "[embed_load] FAIL: QI nsIBaseWindow\n"); return 3; }
  nsresult rv = baseWin->InitWindow(win, nullptr, 0, 0, chrome->mW, chrome->mH);
  if (NS_FAILED(rv)) { fprintf(stderr, "[embed_load] FAIL: InitWindow 0x%x\n", (unsigned)rv); return 4; }
  baseWin->Create();
  baseWin->SetVisibility(true);
  if (tryRender) {
    // Mapping triggers paint. KNOWN ISSUE: on this UXP/Linux the GTK widget
    // forces a ClientLayerManager and XRE_InitEmbedding2 brings up no compositor
    // for it to forward to, so the first expose crashes in
    // ClientLayerManager::ForwardTransaction. Gated until the render backend
    // initializes a compositor (see PORT-MAP.md + context/impl/dead-ends.md).
    // The load path below works without mapping.
    gtk_widget_show_all(win);
    while (gtk_events_pending()) gtk_main_iteration_do(FALSE);
    printf("[embed_load] window mapped + embedded\n");
  }

  // Listen for load completion via the embedding listener API (the webBrowser
  // does not QI to nsIWebProgress directly). Uses a weak ref to the chrome.
  nsCOMPtr<nsIWeakReference> weak = do_GetWeakReference(static_cast<nsIWebBrowserChrome*>(chrome));
  nsresult prv = wb->AddWebBrowserListener(weak, NS_GET_IID(nsIWebProgressListener));
  printf("[embed_load] AddWebBrowserListener rv=0x%x\n", (unsigned)prv);

  nsCOMPtr<nsIWebNavigation> nav = do_QueryInterface(wb);
  if (!nav) { fprintf(stderr, "[embed_load] FAIL: QI nsIWebNavigation\n"); return 5; }
  const char16_t* url = u"data:text/html,<title>Jihad</title>"
    u"<body style='background:%23224488;color:white;font-family:sans-serif;font-size:48px'>"
    u"<h1>Goanna inside, webOS alive, inshallah.</h1></body>";
  rv = nav->LoadURI(url, nsIWebNavigation::LOAD_FLAGS_NONE, nullptr, nullptr, nullptr);
  printf("[embed_load] LoadURI rv=0x%x\n", (unsigned)rv);

  // Pump the XPCOM thread event queue (canonical embedding loop) plus any
  // pending GTK events, until the load completes or we time out.
  nsCOMPtr<nsIThread> thread;
  NS_GetCurrentThread(getter_AddRefs(thread));
  time_t start = time(nullptr);
  while (!chrome->mDone && (time(nullptr) - start) < 15) {
    NS_ProcessNextEvent(thread, false);
    while (gtk_events_pending()) gtk_main_iteration_do(FALSE);
    g_usleep(1000);
  }

  nsCOMPtr<nsIURI> cur;
  nav->GetCurrentURI(getter_AddRefs(cur));
  nsCString spec;
  if (cur) cur->GetSpec(spec);
  printf("[embed_load] currentURI=%s\n", spec.get());

  bool ok = chrome->mDone;
  if (ok) {
    printf("[embed_load] PASS: page loaded to completion\n");
  } else {
    printf("[embed_load] TIMEOUT: load did not reach STATE_STOP in 15s\n");
  }

  bool rendered = false;
  if (tryRender) {
    // Let the page paint, then capture the rendered window pixels to a PPM.
    for (int i = 0; i < 1500; ++i) {
      NS_ProcessNextEvent(thread, false);
      while (gtk_events_pending()) gtk_main_iteration_do(FALSE);
      g_usleep(1000);
    }
    long content = CaptureWindowPPM(win, "/out/jihad_render.ppm");
    rendered = content > 100;
    printf("[embed_load] RENDER %s (non-white px=%ld)\n", rendered ? "PASS" : "FAIL", content);
  } else {
    printf("[embed_load] render skipped (set JIHAD_TRY_RENDER to attempt; needs compositor)\n");
  }
  (void)rendered;

  // Tear down in order before terminating the engine, to avoid shutdown crashes.
  wb->RemoveWebBrowserListener(weak, NS_GET_IID(nsIWebProgressListener));
  baseWin->Destroy();
  nav = nullptr;
  baseWin = nullptr;
  wb = nullptr;
  chrome->mBrowser = nullptr;

  host.Shutdown();
  return ok ? 0 : 6;   // load is the pass criterion; render is gated/experimental
}
