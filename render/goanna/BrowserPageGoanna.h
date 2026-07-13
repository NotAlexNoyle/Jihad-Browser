/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — BrowserPageGoanna: the daemon-side render backend.
 *
 * This is the Goanna replacement for the QtWebKit BrowserPage. The BrowserServer
 * YAP dispatch (BrowserServerBase) calls into a BrowserPage per card; this class
 * provides that surface by driving a GoannaRenderPage (the proven render/load
 * core) and emitting the server->adapter YAP messages (msgPainted, load
 * lifecycle, etc.). It keeps the BrowserAdapter<->BrowserServer contract
 * unchanged — see docs/IPC-CONTRACT.md and PORT-MAP.md.
 *
 * This header intentionally exposes only the Phase-1 core of the BrowserPage
 * surface (lifecycle, navigation, paint). The full ~80-command / ~60-message
 * vtable is filled in during daemon integration (T-016); each addition maps 1:1
 * to a GoannaRenderPage/nsI* action per PORT-MAP.md.
 */
#ifndef JIHAD_BROWSERPAGEGOANNA_H
#define JIHAD_BROWSERPAGEGOANNA_H

#include <cstdint>
#include <string>
#include <vector>
#include "GoannaRenderPage.h"   // GoannaRenderPage + global browser-service fns

namespace jihad {

class EngineHost;

// Sink for the server->adapter YAP messages. In the daemon this is implemented
// by the BrowserServer (which forwards over YAP to BrowserAdapter). Decoupled
// here so the backend builds/tests without the full daemon.
class IPageMessageSink
{
public:
  virtual ~IPageMessageSink() {}
  virtual void msgPainted(int32_t sharedBufferKey) = 0;
  virtual void msgLoadStarted() = 0;
  virtual void msgLoadProgress(int32_t progress) = 0;
  virtual void msgLoadStopped() = 0;
  virtual void msgLocationChanged(const char* uri, bool canBack, bool canFwd) = 0;
  virtual void msgTitleChanged(const char* title) = 0;
  virtual void msgContentsSizeChanged(int32_t width, int32_t height) = 0;
  virtual void msgScrolledTo(int32_t x, int32_t y) = 0;
  virtual void msgMetaViewportSet(double initialScale, double minimumScale,
                                  double maximumScale, int32_t width,
                                  int32_t height, bool userScalable) = 0;
  virtual void msgFailedLoad(const char* domain, int32_t code,
                             const char* url, const char* description) = 0;
  // Optional events (default no-op so existing sinks need not override).
  virtual void msgUpdateGlobalHistory(const char* url, bool reload) { (void)url; (void)reload; }
  virtual void msgUrlRedirected(const char* url, const char* userData) { (void)url; (void)userData; }
  virtual void msgSSLConfirm(const char* host, int32_t code, const char* certFile) {
    (void)host; (void)code; (void)certFile;
  }
  virtual void msgLinkClicked(const char* url) { (void)url; }
  // Title+URL together (YAP 0x200A). This — not msgLocationChanged — is what drives the
  // isis address bar: BasicWebView.titleURLChange -> urlTitleChanged -> ActionBar.setUrl.
  virtual void msgTitleAndUrlChanged(const char* title, const char* uri, bool canBack, bool canFwd) {
    (void)title; (void)uri; (void)canBack; (void)canFwd;
  }
  // An editable element gained/lost focus -> isis raises/hides the VKB
  // (BasicWebView.editorFocused -> PalmSystem.editorFocused). fieldType/fieldActions are
  // PalmIME hints (0 = default text).
  virtual void msgEditorFocused(bool focused, int fieldType, int fieldActions) {
    (void)focused; (void)fieldType; (void)fieldActions;
  }
};

class BrowserPageGoanna
{
public:
  BrowserPageGoanna(EngineHost& host, IPageMessageSink& sink);
  ~BrowserPageGoanna();

  // --- lifecycle / surface (YAP: connect/setWindowSize) ---
  // Attach the double-buffered shared memory the adapter provided (SysV keys).
  bool init(uint32_t width, uint32_t height,
            int sharedBufferKey1, int sharedBufferKey2, int sharedBufferSize);
  void setWindowSize(uint32_t width, uint32_t height);
  void returnBuffer(int sharedBufferKey);  // YAP: returnBuffer (adapter freed it)
  void freeze();                           // YAP: freeze (card backgrounded — pause paint)
  void thaw(int key1, int key2, int size); // YAP: thaw (reattach buffers, resume)
  bool findString(const char* text, bool forward);  // YAP: findString
  void setScrollPosition(int x, int y);   // YAP: setScrollPosition
  void setZoomAndScroll(double zoom, int x, int y);   // YAP: setZoomAndScroll

  // --- navigation (YAP: openUrl/back/forward/reload/stop) ---
  void openUrl(const char* url);
  void setHTML(const char* url, const char* body);
  void pageBackward();
  void pageForward();
  void pageReload();
  void pageStop();
  void clearHistory();                        // YAP: clearHistory
  void getHistoryState(bool* back, bool* fwd); // YAP: getHistoryState (query)
  // Register a URL redirect rule (YAP: addUrlRedirect). A matching URL is handed
  // to the client via msgUrlRedirected; if redirect is true it is NOT loaded.
  void addUrlRedirect(const char* urlRe, int type, bool redirect, const char* userData);

  // --- input (YAP: clickAt/keyDown/keyUp/mouseEvent) ---
  void clickAt(int contentX, int contentY, int numClicks);
  void keyDown(int key, int modifiers, int chr);
  void keyUp(int key, int modifiers, int chr);
  void mouseEvent(int type, int contentX, int contentY, int detail);
  void touchEvent(int type, int touchCount, int modifiers, const char* touchesJson);
  void holdAt(int x, int y);                       // YAP: holdAt (long-press)
  void insertStringAtCursor(const char* text);     // YAP: insertStringAtCursor
  void dragStart(int x, int y);                    // YAP: dragStart
  void dragProcess(int deltaX, int deltaY);        // YAP: dragProcess (scroll)
  void dragEnd(int x, int y);                      // YAP: dragEnd

  // --- settings (YAP: setEnableJavaScript) — per-page ---
  void settingsJavaScriptEnabled(bool enable);

  // --- paint (YAP: drives msgPainted) ---
  // Render the current content into the inactive shared buffer and emit
  // msgPainted(key). Called from the paint timer / on invalidation.
  void paintToSharedBuffer();

  // The daemon calls these each event-loop tick: pump advances engine+load
  // state (emitting load msgs); maybePaint paints only when there is new
  // content (dedup — avoids 60 Hz blank/stale frames; Codex P2).
  void pump(int msBudget);
  void maybePaint();

private:
  void emitLoadAndLocation();   // poll GoannaRenderPage -> sink messages
  bool emitGeometry();          // contents-size + meta-viewport (dedup); true if a valid size was read
  void emitScrollIfChanged();   // scrolled-to when the offset moved

  EngineHost&        mHost;
  IPageMessageSink&  mSink;
  GoannaRenderPage*  mPage;
  int                mKey1, mKey2, mBufSize;
  int                mActiveKey;        // which shm buffer we last painted into
  bool               mLoadWasDone;
  std::string        mAliasUrl;         // non-empty when the current page is an internal
                                        // about: page rendered from inline HTML (about:jihad
                                        // /about:isis): report THIS as the location instead
                                        // of the underlying data: URL the engine sees
  bool               mNeedsPaint;       // set when there is a new frame to send
  bool               mFrozen;           // card backgrounded: skip painting
  bool               mHadContent;       // a non-blank frame has been produced (suppress blanks over it)
  bool               mGeometryDirty;    // a resize happened; emit geometry from pump once reflow settles
  // Queued tap: clickAt (a YAP socket callback) only records the point; pump() does the
  // hit-test / activation / click / navigation on the tick, where page teardown is safe.
  bool               mPendingClick;
  int                mPendingClickX, mPendingClickY, mPendingClickN;
  int                mLastContentW, mLastContentH;  // last emitted content size
  int                mLastScrollX, mLastScrollY;     // last emitted scroll offset
  double             mZoom;   // current full-page zoom (for input coord mapping)
  int                mAdapterScrollX, mAdapterScrollY;  // adapter's scroll in zoomed-content px
                                                        // (== BrowserOffscreenInfo::renderedX/Y)
  // Map an adapter surface coordinate to a content/CSS coordinate (R5): input
  // events use content space, so undo zoom + scroll.
  void mapToContent(int sx, int sy, int* cx, int* cy);

  struct UrlRule;                             // {compiled regex, userData, redirect}
  std::vector<UrlRule*> mRedirectRules;       // addUrlRedirect rules (R6)
  // Returns true if url matched a redirect rule that consumed the load.
  bool applyRedirectRules(const char* url);

  BrowserPageGoanna(const BrowserPageGoanna&) = delete;
  BrowserPageGoanna& operator=(const BrowserPageGoanna&) = delete;
};

} // namespace jihad

#endif // JIHAD_BROWSERPAGEGOANNA_H
