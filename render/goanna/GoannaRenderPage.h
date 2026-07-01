/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — GoannaRenderPage: the reusable render/load core.
 *
 * Encapsulates the proven pipeline (embed_load, T-019/T-020/T-024) as a class:
 * one offscreen page = one nsIWebBrowser in an offscreen GTK widget, driven by
 * the frozen embedding API, painting via the in-process BasicLayerManager and
 * reading back into an ARGB32 buffer. This is what BrowserPageGoanna (the daemon
 * BrowserPage backend) drives per card; the daemon adds the YAP command/message
 * plumbing on top.
 *
 * Requires the engine patches (build/desktop/patches/0003 OMTC-off env, 0004
 * gfx init) and a display (Xvfb on desktop). See render/goanna/PORT-MAP.md.
 */
#ifndef JIHAD_GOANNARENDERPAGE_H
#define JIHAD_GOANNARENDERPAGE_H

#include <cstdint>
#include <string>

typedef struct _GtkWidget GtkWidget;

namespace jihad {

class EngineHost;
class PageChrome;   // internal XPCOM chrome + progress listener (impl detail)

// Process-global browser services (YAP: setUserAgent/clearCache/clearCookies).
void SetUserAgentOverride(const char* ua);
void ClearCache();
void ClearCookies();
void SetMinFontSize(int px);        // YAP: setMinFontSize
void SetBlockPopups(bool block);    // YAP: setBlockPopups
void SetAcceptCookies(bool accept); // YAP: setAcceptCookies

class GoannaRenderPage
{
public:
  // The engine host must already be initialized (EngineHost::Init).
  explicit GoannaRenderPage(EngineHost& host);
  ~GoannaRenderPage();

  // Create the offscreen page at the given size. Returns false on failure.
  bool Create(int width, int height);

  // Resize the offscreen surface + content viewport (YAP: setWindowSize). The
  // page reflows to the new size; the next ReadPixels returns width*height*4.
  bool Resize(int width, int height);

  // Scroll the content to an absolute CSS-pixel offset (YAP: setScrollPosition).
  void ScrollTo(int x, int y);
  // Read the current scroll offset (for scrolled-to events / tests).
  bool GetScrollXY(int* x, int* y);

  // Full-page zoom factor (YAP: setZoomAndScroll). 1.0 = 100%.
  void SetZoom(double zoom);

  // Rendered content size in CSS px (for contents-size-changed events).
  bool GetContentSize(int* w, int* h);
  // Parsed viewport meta info (for meta-viewport events). Any out-ptr may be null.
  bool GetViewport(double* initialScale, double* minScale, double* maxScale,
                   int* w, int* h, bool* userScalable);

  // Navigate. LoadUrlAndWait pumps the event loop until the load reaches
  // STATE_STOP or timeoutSec elapses; returns true if it completed.
  bool LoadUrl(const char* url);
  bool LoadUrlAndWait(const char* url, int timeoutSec);
  bool SetHtml(const char* body);       // load inline HTML (YAP: setHtml)
  bool CanGoBack();
  bool CanGoForward();

  // Navigation controls (map to the YAP back/forward/reload/stop commands).
  void GoBack();
  void GoForward();
  void Reload();
  void Stop();
  void ClearHistory();   // YAP: clearHistory — purge session history

  // Pump the event loop for up to msBudget milliseconds (paint/idle work).
  void PumpFor(int msBudget);

  // --- input synthesis (YAP: clickAt/keyDown/keyUp/mouseEvent) ---
  // Synthesize DOM events at content coordinates via nsIDOMWindowUtils.
  void ClickAt(int x, int y, int numClicks);                 // mousedown+mouseup
  void MouseEvent(const char* type, int x, int y, int button); // type = "mousedown"/"mouseup"/"mousemove"
  void KeyEvent(const char* type, int keyCode, int charCode, int modifiers); // "keydown"/"keyup"/"keypress"
  void TouchEvent(const char* type, int x, int y); // single-touch "touchstart"/"touchmove"/"touchend"

  // --- settings (YAP: setEnableJavaScript) — per-page via the docShell ---
  void SetJavaScriptEnabled(bool enabled);

  // Read the current painted content into dst as ARGB32 (native LE B,G,R,A),
  // width*height*4 bytes. dst must be at least that large. Returns the number
  // of non-near-white pixels (a cheap "did it render" signal), or -1 on error.
  long ReadPixels(unsigned char* dst, size_t dstBytes);

  int Width() const { return mWidth; }
  int Height() const { return mHeight; }
  bool LoadDone() const;
  // Whether the last load ended in a network error (R3 failed-load). Fills the
  // failing nsresult code + URL when *failed is true.
  bool GetLoadError(bool* failed, int* code, std::string* url);
  // Whether the main document was redirected during the last load (R4).
  bool DidRedirect() const;
  std::string CurrentUri();

private:
  void BeginLoad();   // reset per-load state (done + failure) before a navigation

  EngineHost& mHost;
  PageChrome* mChrome;   // holds the nsIWebBrowser + listener (opaque here)
  GtkWidget* mWindow;
  int mWidth;
  int mHeight;

  GoannaRenderPage(const GoannaRenderPage&) = delete;
  GoannaRenderPage& operator=(const GoannaRenderPage&) = delete;
};

} // namespace jihad

#endif // JIHAD_GOANNARENDERPAGE_H
