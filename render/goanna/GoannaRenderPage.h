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
  // (Re)register the engine focus/blur listener on the current top document (Atlas IM-context
  // port). Call after each completed load, from the guarded pump; same-document calls no-op.
  void RegisterEngineFocusListener();
  // Merge engine-observed focus/blur into the VKB state (drained by TakeEditorFocus): script-
  // driven focus moves and blurs now drive msgEditorFocused without a tap. Change-only emission
  // + the Atlas autofocus gate (no VKB raise before the first tap on a page). Device T4.
  void PollEngineFocus();
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
  // Enter: newline (textarea) or implicit form submission (single-line input). Returns true if it
  // ATTEMPTED a form submission (whether or not it actually navigated) so the caller can stop draining
  // queued Enters after one submit — preventing a double submission independent of when the nav commits.
  bool HandleEnter();
  void HandleTab(bool backward);                   // Tab: tab char (textarea) or focus next field (input)
  // Dispatch a bubbling DOM 'input' event on the focused editable if a value edit is pending. Runs
  // page JS (framework onChange) so it MUST be called only from the guarded pump loop, never from
  // the keyDown YAP callback (F-219) — controlled/React inputs otherwise discard the programmatic
  // SetValue edit and restore the old value (Codex F-238).
  void FlushPendingInputEvent();
  bool HasFocusedEditable() const;                 // true when a tapped editable is the type target
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
  // Aggregate load progress 0..99 during a load (100 is signalled separately by LoadDone). Drives the
  // isis address-bar progress bar so a slow load looks alive instead of frozen.
  int GetLoadProgress() const;
  // True (draining) if engine content invalidated since the last call — incremental render, JS/SPA DOM
  // updates, async image decode, CSS animation (UXP patch 0012 sticky PuppetWidget flag). The daemon
  // repaints on dirty: the engine-driven frame delivery the stock QtWebKit server got from Qt paint
  // events. Always false on the non-offscreen (desktop GTK) path and on a pre-0012 libxul (weak symbol).
  bool TakeDirty();
  // Adopt an in-flight ENGINE-initiated content navigation (a POST form submit) as the tracked load:
  // reset the per-load done/failure state and mark it programmatic so its own subframe/redirect
  // STATE_STARTs aren't misread as fresh link clicks, and so its eventual STATE_STOP is reported as a
  // real completion (msgLocationChanged + repaint). Used for POSTs, which must NOT be re-driven via
  // openUrl (that would replay the body and double-submit — Codex F-262/F-289).
  void AdoptContentLoad();
  // Reset the command-load flag so a content nav on a watchdog-dismissed partial page is still detected
  // (Codex F-333). Called by the daemon's stall watchdog.
  void ClearProgrammaticLoad();
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
  void FocusNextField(bool backward);      // Tab in an <input>: focus the next/prev text field

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
