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
class nsIWidget;              // opaque; the offscreen PuppetWidget handle (see GoannaRenderPage.cpp)
class nsIDOMHTMLFormElement;  // opaque; crash-safe implicit submission target (FireFormSubmit)
class nsIDOMHTMLSelectElement;// opaque; the dropdown <select> whose popup is on the card
class nsIWebBrowser;          // opaque; DebugWebBrowser() handle for the debug inject channel

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

// DEBUG ONLY (self-drive inject channel, off by default — see JihadBrowserServer.cpp).
// Runs a javascript: URL against the LAST-CREATED page with the SYSTEM principal, so a
// probe can execute inside privileged chrome documents (about:addons/about:config) where
// a plain LoadURI's null triggering principal is rightly refused. Never call this from
// production paths: it executes arbitrary script with full chrome privileges.
bool DebugRunChromeJs(const char* jsUrl);
// DEBUG ONLY: current document title of the last-created page (probe result readback —
// jsurl probes report via document.title, which is otherwise only logged at load-done).
std::string DebugGetTitle();

// DEBUG ONLY: report an element's viewport rect by id, as "id x,y WxH" (empty if there
// is no such element). Exists because driving XUL chrome (about:addons) from a test
// needs the real coordinates of a control, and the alternatives are both dead ends here:
// a `javascript:` URL does not execute in a chrome document (measured — it returns
// success and does nothing), and guessing coordinates resolves to `<null>` and tells you
// nothing about why.
std::string DebugElementRect(const char* elementId);

// DEBUG ONLY: click an element by id at its own centre, in the viewport CSS space, so a
// test does not have to undo the daemon's zoom/scroll mapping to hit a known control.
bool DebugClickElement(const char* elementId);

// DEBUG ONLY: enable/disable an installed add-on by id (R4's "disabling stops the effect",
// which otherwise needs a chrome UI this embedding does not have).
bool DebugSetAddonEnabled(const char* addonId, bool enable);

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
  // Read the current scroll offset (for scrolled-to events / tests). flushLayout=false
  // skips the synchronous reflow — the paint path uses it because the render that follows
  // flushes anyway (review 2026-08-02 F4: an extra forced reflow per paint).
  bool GetScrollXY(int* x, int* y, bool flushLayout = true);

  // Full-page zoom factor (YAP: setZoomAndScroll). 1.0 = 100%.
  void SetZoom(double zoom);
  // Visual-viewport pan (CSS px) applied in the offscreen render when zoomed (>1), so the
  // whole page is reachable in both axes without the engine's device-width scroll limits.
  void SetRenderPan(double x, double y) { mPanX = x; mPanY = y; }
  // The pan actually in effect (post-clamp) — the paint path stamps renderedX/Y from THIS,
  // not from the raw adapter scroll, so the header always describes the rendered pixels.
  void GetRenderPan(double* x, double* y) const { if (x) *x = mPanX; if (y) *y = mPanY; }

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
  // Resolve what sits at CONTENT (x,y) into the isis HitTest.schema JSON (isNull/isLink/
  // isImage/linkUrl/linkText/imageUrl/altText/editable). The adapter GATES the long-press
  // gesture on this round-trip (asyncCmdHitTest -> msgHitTestResponse -> card eventFired ->
  // asyncCmdHoldAt), so this must always produce a valid document — on any miss it emits
  // {"isNull":true,...}.
  void HitTestAt(int x, int y, std::string* json);

  // <select> dropdown -> card-native popup (Atlas msgPopupMenuShow/selectPopupMenuItem model,
  // impl-menupopup-2026-08-02.md). BuildSelectPopup (called from ClickAt when a dropdown
  // <select> is tapped) serializes the options + holds the element; the daemon drains
  // TakeSelectPopup and emits msgPopupMenuShow; the card's choice returns via ApplySelectPopup.
  bool BuildSelectPopup(nsIDOMHTMLSelectElement* aSelect);
  bool TakeSelectPopup(std::string* json, std::string* id);
  void ApplySelectPopup(const char* id, int idx);

  // --- settings (YAP: setEnableJavaScript) — per-page via the docShell ---
  void SetJavaScriptEnabled(bool enabled);

  // Read the current painted content into dst as ARGB32 (native LE B,G,R,A),
  // width*height*4 bytes. dst must be at least that large. Returns the number
  // of non-near-white pixels (a cheap "did it render" signal), or -1 on error.
  long ReadPixels(unsigned char* dst, size_t dstBytes);

  // Render an ABSOLUTE document region (document-relative, engine scroll ignored)
  // directly into dst: w x h DEVICE px at zoom Z, covering the CSS rect
  // (docX, docY, w/Z, h/Z). Used by the overscan paint path to fill a region
  // TALLER than the viewport (scroll pan headroom); position:fixed content is not
  // placed at the scroll here — the caller overlays a viewport-relative band via
  // ReadPixels for the visible rows at z~1. Offscreen path only.
  bool RenderRegion(unsigned char* dst, int stride, int w, int h,
                    double docX, double docY, double zoom);

  // Composite any OPEN XUL popup (menupopup: the about:addons tools menu, context
  // menus) over a buffer already painted by RenderRegion/ReadPixels. A popup is a
  // SEPARATE display root, so no amount of document rendering will contain it —
  // it has to be drawn as an overlay. docX/docY/zoom describe the buffer so the
  // popup is placed with the same transform as the content beneath it. Returns the
  // number of popups drawn; 0 (nothing open) is the normal case and costs one query.
  int CompositePopups(unsigned char* dst, int stride, int w, int h,
                      double docX, double docY, double zoom);

  // --- pointer interaction with an open popup -------------------------------------
  // A popup is a separate display root: the content document cannot hit-test it, so the
  // daemon has to steer the pointer into it explicitly.
  // PopupHover: highlight the row under the finger (XUL sets _moz-menuactive on a
  // mousemove) — the rollover feedback AND the "what am I about to pick" indicator.
  // PopupActivate: commit the row under the finger (a lift after a drag picks it, the way
  // a desktop menu drag-select does). Both return false if the point is outside every
  // popup, which is the caller's signal to treat the event normally.
  bool PopupHover(int x, int y);
  bool PopupActivate(int x, int y);
  bool PopupsOpen() const;

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
  // Identity of this page's root content docShell, as an opaque token (F-1). The
  // process-wide download service reports the same token as a download's origin
  // (jihad::DownloadOrigin), so the daemon can route msgDownload* back to the
  // card that started the download instead of guessing at the newest one. Pure
  // identity — compare it, never dereference it; it is null before Create().
  const void* DocShellKey() const;

  // DEBUG ONLY: raw browser handle for DebugRunChromeJs (below). Not for production use.
  nsIWebBrowser* DebugWebBrowser() const;
  std::string CurrentUri();
  std::string GetTitle();   // current document title (for the address-bar title+url msg)

private:
  void BeginLoad();   // reset per-load state (done + failure) before a navigation
  void ActivateEditorCaret();              // activate the offscreen window so nsCaret paints (solid)
  void FocusNextField(bool backward);      // Tab in an <input>: focus the next/prev text field
  // Crash-safe implicit form submission: validate (honoring novalidate), fire a cancelable 'submit'
  // event (onsubmit can preventDefault), then form->Submit() if not cancelled. Replaces DOMClick(),
  // which MOZ_CRASHes from native code (no JSContext -> no subject principal), and a synthesized click,
  // which fires onclick but not the submit default action in this embedding. Returns true iff submitted.
  bool FireFormSubmit(nsIDOMHTMLFormElement* form);

  EngineHost& mHost;
  PageChrome* mChrome;   // holds the nsIWebBrowser + listener (opaque here)
  std::string mClickNavUrl;   // href from the last link tap, drained by TakeClickNav
  // <select> card-native popup state (see BuildSelectPopup):
  std::string mSelectPopupJson;               // options serialized for the card
  std::string mSelectPopupId;                 // id echoed back by popupMenuSelect
  bool mSelectPopupPending = false;           // a popup is queued for the daemon to emit
  long mSelectPopupMs = 0;                     // last-build time (dedup the raw+gesture double tap)
  // Raw ptr, not nsCOMPtr: this header keeps XPCOM types opaque (forward-decl only). AddRef'd
  // in BuildSelectPopup, Release'd in SetSelectPopupEl/ApplySelectPopup/BeginLoad/dtor.
  nsIDOMHTMLSelectElement* mSelectPopupEl = nullptr;  // held until the choice returns / nav
  void SetSelectPopupEl(nsIDOMHTMLSelectElement* el);  // ref-swap helper (defined in .cpp)
  bool mEditorFocused;        // is an editable element currently focused (VKB up)?
  bool mEditorFocusDirty;     // the focus state changed and needs emitting
  int  mEditorFieldType;      // PalmIME field-type hint for the focused editable
  GtkWidget* mWindow;      // legacy desktop path (GTK offscreen window)
  nsIWidget* mWidget;      // JIHAD_OFFSCREEN path (memory-backed PuppetWidget)
  bool mOffscreen;         // true when rendering via the offscreen widget
  int mWidth;
  int mHeight;
  double mRenderZoom = 1.0;  // pinch/fit zoom applied as a gfxContext scale in the offscreen capture
  double mPanX = 0.0;        // visual-viewport pan (CSS px) used by the render when zoomed (>1)
  double mPanY = 0.0;

  // F-1: raw pen-path click dedup. The adapter forwards mousedown/mouseup for a tap whenever
  // shouldPassInputEvents() is true AND sends clickAt from the single-tap gesture, so one tap
  // arrives as BOTH. Since T-067 both actually reach the DOM, so both activate. MouseEvent()
  // records the down/up pair; ClickAt() suppresses its own duplicate activation when the pair
  // it matches already delivered a real click. See the block comment in ClickAt().
  int  mRawDownX = 0, mRawDownY = 0;
  long mRawDownMs = 0;              // >0 while a raw mousedown is unmatched by an up
  int  mRawClickX = 0, mRawClickY = 0;
  // A XUL popup opened at this time/point (jihad_offscreen_composite_popups draws it).
  // One physical tap is delivered TWICE, so without this the duplicate re-hits the anchor
  // and shuts the menu the first delivery just opened — see ClickAt.
  long mPopupOpenMs = 0;
  int mPopupOpenX = 0, mPopupOpenY = 0;
  long mRawClickMs = 0;             // >0 when a raw down+up pair completed and no clickAt consumed it
  // Focused element at the raw mousedown, for the F-7 "focus actually CHANGED" test on the raw
  // path. COMPARED ONLY, never dereferenced (same rule as DocShellKey) — it may be dead by then.
  const void* mRawFocusBefore = nullptr;

  GoannaRenderPage(const GoannaRenderPage&) = delete;
  GoannaRenderPage& operator=(const GoannaRenderPage&) = delete;
};

} // namespace jihad

#endif // JIHAD_GOANNARENDERPAGE_H
