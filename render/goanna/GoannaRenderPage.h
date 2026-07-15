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
class nsIWidget;   // opaque; the offscreen PuppetWidget handle (see GoannaRenderPage.cpp)

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
  // If the last ClickAt landed on a link, its href is returned here so the caller can
  // navigate via the normal load path — the click default-action does not fire in the
  // offscreen embedding, and calling LoadUrl inside the click handler stalls the load.
  bool TakeClickNav(std::string* url);
  // If a tap changed editable-element focus, returns the new state so the caller can emit
  // msgEditorFocused (isis raises/hides the VKB). Drains the change; returns false if none.
  bool TakeEditorFocus(bool* focused, int* fieldType, int* fieldActions);
  // Clear editor focus (a navigation happened); returns true if it WAS focused so the
  // caller can emit msgEditorFocused(false) to lower the VKB over the new page.
  bool ClearEditorFocus();
  void MouseEvent(const char* type, int x, int y, int button); // type = "mousedown"/"mouseup"/"mousemove"
  void KeyEvent(const char* type, int keyCode, int charCode, int modifiers); // "keydown"/"keyup"/"keypress"
  void TouchEvent(const char* type, int x, int y); // single-touch "touchstart"/"touchmove"/"touchend"
  void InsertText(const char* text);               // insert at caret (YAP: insertStringAtCursor)
  void DeleteBackward();                            // Backspace: delete the char before the caret
  void DeleteBackwardWord();                        // accelerated Backspace: delete a word before caret
  // Non-character editing keys, applied to the focused <input>/<textarea> using the ENGINE'S
  // selection as the caret (validated crash-free headless after UXP patch 0010). Enter inserts a
  // newline only in a <textarea>; Tab moves focus to the next field.
  enum EditKeyAction { EK_LEFT, EK_RIGHT, EK_UP, EK_DOWN, EK_HOME, EK_END, EK_DELETE };
  void EditKey(int action);
  void HandleEnter();                              // Enter: newline (textarea) or submit form (input)
  bool HasFocusedEditable() const;                 // true when a tapped editable is the type target
  void JihadTypingSelfTest();                      // diag: programmatic focus+type (no VKB tap)
  bool Find(const char* text, bool forward);       // find in page (YAP: findString)

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
  // If a content-initiated (link) navigation was seen, return its URL and clear
  // the flag (R6 link-clicked). Returns false if none pending.
  bool TakeLinkClicked(std::string* url, bool* isPost = nullptr);
  // Whether the last load hit an overridable certificate error (R5). On accept,
  // AcceptCurrentCert adds a validity override so a reload of the host proceeds.
  bool GetCertError(std::string* host, int* code);
  bool AcceptCurrentCert();
  std::string CurrentUri();
  std::string GetTitle();   // current document title (for the address-bar title+url msg)

private:
  void BeginLoad();   // reset per-load state (done + failure) before a navigation
  void ActivateEditorCaret();              // activate the offscreen window so nsCaret paints (solid)

  EngineHost& mHost;
  PageChrome* mChrome;   // holds the nsIWebBrowser + listener (opaque here)
  std::string mClickNavUrl;   // href from the last link tap, drained by TakeClickNav
  bool mEditorFocused;        // is an editable element currently focused (VKB up)?
  bool mEditorFocusDirty;     // the focus state changed and needs emitting
  int  mEditorFieldType;      // PalmIME field-type hint for the focused editable
  GtkWidget* mWindow;      // legacy desktop path (GTK offscreen window)
  nsIWidget* mWidget;      // JIHAD_OFFSCREEN path (memory-backed PuppetWidget)
  bool mOffscreen;         // true when rendering via the offscreen widget
  int mWidth;
  int mHeight;

  GoannaRenderPage(const GoannaRenderPage&) = delete;
  GoannaRenderPage& operator=(const GoannaRenderPage&) = delete;
};

} // namespace jihad

#endif // JIHAD_GOANNARENDERPAGE_H
