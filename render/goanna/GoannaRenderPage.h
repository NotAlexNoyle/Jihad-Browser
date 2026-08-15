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
#include <vector>

#include "JihadCertStore.h"   // jihad::certstore::Problem (R5 cert-error description)

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

// Chrome-owned settings, read by the CARD over this variant's Luna service.
//
// The settings page (about:preferences) lives in the ENGINE and writes these through
// Services.prefs; the home button and the start page live in the CARD and need to read
// them. The card cannot reach engine prefs and the page cannot reach the card's storage,
// so this is the bridge — deliberately over Luna (a channel the cards already use) and
// NOT over YAP, which is byte-frozen (cavekit-ipc-contract.md R1).
//
// One string pref carrying JSON, not a pref per setting: the set is owned end-to-end by
// this project, it is read and written whole, and one pref keeps the Luna surface to a
// single method. Returns false if the pref is unset, leaving *outJson untouched.
bool GetChromeSettings(std::string* outJson);

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

// DEBUG ONLY: write a pref from the inject channel. `type` is 'b', 'i' or 's'.
//
// This exists because there was NO WAY to write a pref from a test, and the absence was only
// noticed after a whole device run was wasted on it (2026-08-10, T-132). The obvious route —
// a `jsurl` javascript: URL calling Components.classes["@mozilla.org/preferences-service;1"] —
// runs with the system PRINCIPAL but in a CONTENT scope, where `Components` is not exposed at
// all, so the probe silently did nothing and the run looked like a negative result about the
// shutdown flush. The daemon is in-process with libxul and already holds nsIPrefBranch, so
// doing it here is both correct and trivial. Returns false if the pref service is unavailable
// or the type letter is unknown.
bool DebugSetPref(const char* name, char type, const char* value);
// DEBUG ONLY: read a pref back as text, so a test can assert on what it just wrote without
// depending on the page. Empty string when the pref does not exist.
std::string DebugGetPref(const char* name, char type);

// DEBUG ONLY: report an element's NATIVE ANONYMOUS CONTENT — the only honest way to ask
// "did this XBL binding actually attach". Returns "NAC:<n> <name>[<k>],…", where n is the
// element's native-anonymous child count and k is each child's own flattened-children count;
// k > 0 IS "an XBL binding attached to that child". "?" when there is no such element.
//
// READ THE .cpp BEFORE CHANGING THIS. Two earlier shapes of this probe reported a null as a
// negative RESULT when it was a limitation of the accessor, and the second of those was
// published as the finding "videocontrols does not attach" — later overturned by a screenshot.
// Nothing that bottoms out in nsIDOMDocumentXBL can see native anonymous content.
//
// Page JS cannot answer this either: `document.getAnonymousNodes` is not exposed to script here
// even with the system principal (measured on device 2026-08-10, T-125 — it throws
// "not a function"), and the `controls` DOM attribute reflects whether or not a binding bound,
// so it proves nothing.
std::string DebugAnonNodes(const char* selector);

// DEBUG ONLY: click an element by id at its own centre, in the viewport CSS space, so a
// test does not have to undo the daemon's zoom/scroll mapping to hit a known control.
bool DebugClickElement(const char* elementId, int clickCount = 1);
// Click at an OFFSET from an element's top-left, rather than its centre. Needed for XUL
// trees: their rows are not DOM nodes (a tree is view-backed), so no selector reaches a
// row and a centre click on a mostly-empty tree lands below every row.
bool DebugClickElementAt(const char* elementId, int dx, int dy, int clickCount);
// Read an element's text. The only way to assert on what a generated chrome page actually
// SAYS (about:plugins lists name/version/MIME types), since a rect proves existence only.
std::string DebugElementText(const char* selector, int maxChars);

// DEBUG ONLY: enable/disable an installed add-on by id (R4's "disabling stops the effect",
// which otherwise needs a chrome UI this embedding does not have).
bool DebugSetAddonEnabled(const char* addonId, bool enable);

// DEBUG ONLY: cookie persistence probe (browser-services R2) — write a year-long cookie,
// and count/list what the store currently holds, so a restart test needs no website.
bool DebugCookieSet(const char* host, const char* name, const char* value);
int DebugCookieCount();

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

  // The bounding box of everything the engine invalidated since the last call, in viewport
  // device px, drained. False when nothing changed. Lets the caller repaint only those rows
  // instead of the whole viewport — the difference between ~20 fps and 30 fps for animated
  // content on this hardware.
  bool TakeDirtyRect(int* x, int* y, int* w, int* h);

  // Tell every running NPAPI plugin that the card has entered/left the plugin spotlight
  // (webOS fullscreen). Rect is in VIEWPORT CSS px, absolute edges. Returns how many
  // instances were told — 0 means no plugin is running, which the caller should log rather
  // than treat as success.
  int SetPluginSpotlight(bool on, int left, int top, int right, int bottom);

  // One running NPAPI plugin's on-screen box, in VIEWPORT CSS px (getBoundingClientRect
  // space). Used to emit the frozen interactive-rect messages (YAP 0x2037/0x2038).
  struct PluginRect {
    void* key;                 // element identity across ticks; never dereferenced
    float left, top, width, height;
  };
  // Every <embed>/<object>/<applet> in the content document whose plugin is actually
  // RUNNING. Flushes layout (getBoundingClientRect does), so the caller rate-limits.
  // Returns false if there is no document to ask.
  bool CollectPluginRects(std::vector<PluginRect>* out);

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
  // SPENDS the budget: it keeps spinning even with an empty queue, which is what a caller
  // waiting on a load or a timer wants. Do NOT use it from the tick — see PumpReady.
  void PumpFor(int msBudget);

  // Drain what the engine already has ready, then return. Same work as PumpFor, opposite
  // contract about time: it never sleeps and stops as soon as the queue is empty.
  void PumpReady(int msBudget);

  // Monotonic count of plugin frames delivered into this process. The embedder compares it
  // against the value it saw at its last publish to tell a NEW plugin frame from a repeat,
  // which the sticky dirty flag cannot express. 0 if the engine does not provide it.
  uint32_t PluginFrameSeq();

  // Pull: ask every live plugin for one frame. Called right after a card frame is published, so
  // the plugin's draw clock IS the daemon's publish clock instead of a second free-running one.
  // Returns the number of instances asked; 0 if the engine does not support the pull.
  int RequestPluginFrame();

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
  // True once per same-document location change (fragment edit, pushState/replaceState), which
  // produces no load lifecycle and so no completion boundary. Drains the flag.
  bool TakeSameDocumentLocation();
  void MouseEvent(const char* type, int x, int y, int button); // type = "mousedown"/"mouseup"/"mousemove"
  void KeyEvent(const char* type, int keyCode, int charCode, int modifiers); // "keydown"/"keyup"/"keypress"
  // Both forms RETURN the preventDefault result of sendTouchEventToWindow, i.e. "the page
  // consumed this gesture". That is the only signal the double-activation suppressor in
  // BrowserPageGoanna has: on this platform the same finger also produces pen events, which
  // arrive as a SEPARATE YAP command that the engine cannot know is related.
  bool TouchEvent(const char* type, int x, int y);
  // Multi-point form. A pinch is two points that move relative to each other, so a
  // single-point touch API cannot express one at all.
  bool TouchEvent(const char* type, const int* xs, const int* ys, int count); // "touchstart"/"touchmove"/"touchend"
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
  // aPermanent mirrors the card's answer: "1" (Trust Always) writes a PERMANENT
  // override, which is the only form that survives a daemon restart in Goanna's
  // own store; "2" (Trust Once) keeps the temporary one. Defaulted so callers
  // that only ever meant "once" (the TLS harness) read the same as before.
  bool AcceptCurrentCert(bool aPermanent = false);
  // Raw DER of the certificate the failing handshake presented, for armouring
  // into the PEM that both the card's View Certificate button and the platform
  // store want. False when no certificate was captured.
  bool GetCertDer(std::string* der);
  // What is actually wrong with that certificate, for choosing the card's
  // message. False when the last load had no cert error.
  bool GetCertProblem(jihad::certstore::Problem* out);
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
  int64_t mSelectPopupMs = 0;                     // last-build time (dedup the raw+gesture double tap)
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
  int64_t mRawDownMs = 0;              // >0 while a raw mousedown is unmatched by an up
  int  mRawClickX = 0, mRawClickY = 0;
  // A XUL popup opened at this time/point (jihad_offscreen_composite_popups draws it).
  // One physical tap is delivered TWICE, so without this the duplicate re-hits the anchor
  // and shuts the menu the first delivery just opened — see ClickAt.
  int64_t mPopupOpenMs = 0;
  int mPopupOpenX = 0, mPopupOpenY = 0;
  int64_t mRawClickMs = 0;             // >0 when a raw down+up pair completed and no clickAt consumed it
  // Focused element at the raw mousedown, for the F-7 "focus actually CHANGED" test on the raw
  // path. COMPARED ONLY, never dereferenced (same rule as DocShellKey) — it may be dead by then.
  const void* mRawFocusBefore = nullptr;

  GoannaRenderPage(const GoannaRenderPage&) = delete;
  GoannaRenderPage& operator=(const GoannaRenderPage&) = delete;
};

} // namespace jihad

#endif // JIHAD_GOANNARENDERPAGE_H
