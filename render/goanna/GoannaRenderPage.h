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
  std::string CurrentUri();

private:
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
