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
#include <map>
#include <string>
#include <vector>
#include "GoannaRenderPage.h"
#include "DialogService.h"      // DialogSink: engine dialogs -> this page's card   // GoannaRenderPage + global browser-service fns

namespace jihad {

class EngineHost;

// The daemon's tick period in ms ($JIHAD_TICK_MS, clamped 4-64, default 8). One definition,
// shared by the tick that consumes it and the plugin pull that has to be a multiple of it.
int jihadTickPeriodMs();

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
  // Same message, but carrying the reply FIFO so the card's answer can come back and be
  // acted on (accept -> remember the validity override and reload). The no-pipe overload
  // above stays for sinks that only observe.
  virtual void msgSSLConfirm2(const char* syncPipePath, const char* host, int32_t code,
                              const char* certFile) {
    (void)syncPipePath; msgSSLConfirm(host, code, certFile);
  }
  virtual void msgLinkClicked(const char* url) { (void)url; }
  // Content opened a JS dialog (alert/confirm/prompt/auth). syncPipePath is a FIFO the
  // daemon is about to block on: the adapter stashes it, shows the card's dialog, and the
  // card's sendDialogResponse writes the answer back through it. Frozen contract — the
  // adapter and all three cards already implement their ends.
  virtual void msgDialogAlert(const char* syncPipePath, const char* msg) {
    (void)syncPipePath; (void)msg;
  }
  virtual void msgDialogConfirm(const char* syncPipePath, const char* msg) {
    (void)syncPipePath; (void)msg;
  }
  virtual void msgDialogPrompt(const char* syncPipePath, const char* msg, const char* defaultValue) {
    (void)syncPipePath; (void)msg; (void)defaultValue;
  }
  virtual void msgDialogUserPassword(const char* syncPipePath, const char* msg) {
    (void)syncPipePath; (void)msg;
  }
  // A dropdown <select> was tapped -> the card shows a native option list (Atlas model).
  // menuDataFileName is a temp JSON file the adapter reads + unlinks. The choice returns via
  // asyncCmdPopupMenuSelect -> BrowserPageGoanna::popupMenuSelect.
  virtual void msgPopupMenuShow(const char* identifier, const char* menuDataFileName) {
    (void)identifier; (void)menuDataFileName;
  }
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
  // A navigation resolved to a non-displayable resource (download / MIME handoff).
  // Drives BrowserServerBase::msgMimeHandoffUrl (YAP 0x2015) -> adapter
  // mimeHandoffUrl -> BasicWebView.doFileLoad -> app handleResource, which routes
  // it to com.palm.downloadmanager. (mimeType may be empty if unknown.)
  virtual void msgMimeHandoffUrl(const char* mimeType, const char* url) {
    (void)mimeType; (void)url;
  }
  // Lifecycle of a download the ENGINE performs to a temp file (YAP 0x2010..0x2013).
  // Exactly one start, then >=0 progress, terminated by one finished OR one error.
  virtual void msgDownloadStart(const char* url) { (void)url; }
  virtual void msgDownloadProgress(const char* url, int32_t soFar, int32_t total) {
    (void)url; (void)soFar; (void)total;
  }
  virtual void msgDownloadFinished(const char* url, const char* mimeType,
                                   const char* tmpFilePath) {
    (void)url; (void)mimeType; (void)tmpFilePath;
  }
  virtual void msgDownloadError(const char* url, const char* errorMsg) {
    (void)url; (void)errorMsg;
  }
  // Interactive (plugin) rects, YAP 0x2037/0x2038. NOT hole-punching and NOT a compositing
  // hint — the payload carries no surface handle at all. They are INPUT ARBITRATION: the
  // adapter files them into mFlashRects and consults them through flashRectContainsPoint()
  // to decide whether a drag pans the card or belongs to the plugin. Emitting them is what
  // lets a finger reach a plugin on an ordinary scrollable page, where the adapter otherwise
  // claims every gesture for scrolling.
  virtual void msgAddFlashRects(const char* rectsArrayJson) { (void)rectsArrayJson; }
  virtual void msgRemoveFlashRects(const char* rectIdJson) { (void)rectIdJson; }
};

// Also the DialogSink for its own page: an engine-raised JS dialog (alert/confirm/prompt/
// auth, and the XPI install confirm) is emitted to THIS page's card over the frozen contract
// and answered through the sync FIFO the adapter writes back.
class BrowserPageGoanna : public DialogSink
{
public:
  BrowserPageGoanna(EngineHost& host, IPageMessageSink& sink);
  ~BrowserPageGoanna();

  // jihad::DialogSink — called synchronously on the engine thread.
  void OnDialog(DialogKind kind, const char* text, DialogReply* reply) override;

 private:
  // Shared plumbing for every card dialog: create the reply FIFO, and block on it with a
  // deadline. Used by OnDialog and by the SSL-confirm path.
  std::string makeDialogPipe();
  // `answer` receives the RAW three-way choice the card sent (0 reject, 1 trust
  // always, 2 trust once) — `accept` collapses 1 and 2, which is right for every
  // dialog except the SSL confirm, where the two arms mean different things.
  bool awaitDialogReply(const std::string& path, const char* what,
                        bool* accept, std::string* value, int* answer = nullptr);
 public:

  // --- lifecycle / surface (YAP: connect/setWindowSize) ---
  // Attach the double-buffered shared memory the adapter provided (SysV keys).
  bool init(uint32_t width, uint32_t height,
            int sharedBufferKey1, int sharedBufferKey2, int sharedBufferSize);
  void setWindowSize(uint32_t width, uint32_t height);
  // Emit a download/MIME handoff for this page (from the process-wide download
  // interceptor, routed to the active page). See IPageMessageSink::msgMimeHandoffUrl.
  void emitMimeHandoff(const char* mimeType, const char* url) { mSink.msgMimeHandoffUrl(mimeType, url); }
  // Same routing for the engine-performed download lifecycle (YAP msgDownload*).
  void emitDownloadStart(const char* url) { mSink.msgDownloadStart(url); }
  void emitDownloadProgress(const char* url, int32_t soFar, int32_t total) {
    mSink.msgDownloadProgress(url, soFar, total);
  }
  void emitDownloadFinished(const char* url, const char* mime, const char* path) {
    mSink.msgDownloadFinished(url, mime, path);
  }
  void emitDownloadError(const char* url, const char* msg) { mSink.msgDownloadError(url, msg); }
  // Identity token for this card's engine docShell (F-1). The daemon compares it
  // against the origin the download service reports so a download's messages go
  // to the card that STARTED it, not to whichever card connected last.
  const void* docShellKey() const { return mPage ? mPage->DocShellKey() : nullptr; }
  void returnBuffer(int sharedBufferKey);  // YAP: returnBuffer (adapter freed it)
  void freeze();                           // YAP: freeze (card backgrounded — pause paint)
  void thaw(int key1, int key2, int size); // YAP: thaw (reattach buffers, resume)
  // Re-thaw onto the buffers we already hold. The inject channel called thaw(0,0,0), and
  // thaw's own guard is `ok = key1 && ...`, so that could never un-freeze — injected `freeze`
  // was a one-way trap and the thaw path was untestable without a real card background cycle.
  void thawSelf() { thaw(mKey1, mKey2, mBufSize); }
  // YAP 0x1600. Announced after Connect/Thaw rather than inside them.
  void setExtraBuffer(int key3, int size);
  // How many buffers are actually in rotation right now: 3 once the adapter has offered a
  // third, 2 otherwise. One place to ask, so the rotation and the reset loops cannot disagree.
  int bufferCount() const { return mKey3 ? 3 : 2; }
  bool findString(const char* text, bool forward);  // YAP: findString
  void setScrollPosition(int x, int y);   // YAP: setScrollPosition
  // YAP: pluginSpotlightStart/End (0x1501/0x1502). The card's confirmation that it has
  // scrimmed and zoomed to a plugin; forwarded to the plugin as Palm's spotlight system
  // event. Rect arrives in DOCUMENT space as origin + width/height.
  void pluginSpotlightStart(int x, int y, int w, int h);
  void pluginSpotlightEnd();
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
  // Shared enqueue for clickAt/mouseEvent/holdAt/touchEvent — records only; pump() dispatches.
  void queueInput(int type, int x, int y, int detail);
  void touchEvent(int type, int touchCount, int modifiers, const char* touchesJson);
  void holdAt(int x, int y);                       // YAP: holdAt (long-press)
  // YAP: hitTest — resolve what sits at CONTENT (x,y) into the isis HitTest.schema JSON.
  // The adapter GATES the long-press on this round-trip; see GoannaRenderPage::HitTestAt.
  void hitTest(int x, int y, std::string* json);
  // YAP: popupMenuSelect — the card's native <select> list returned a choice. selectedIdx<0
  // is a dismissal. (The show side is emitSelectPopupIfPending, driven from the click drain.)
  // DEBUG readback: is any XUL popup open (see the inject `popups` command).
  bool popupsOpen() const;
  void popupMenuSelect(const char* identifier, int selectedIdx);

 private:
  // Map the adapter's DOCUMENT coords (m_penDownDoc, zoomed px) to the VIEWPORT-relative
  // CSS px the engine dispatch expects (SendMouseEvent / ElementFromPoint). Measured on
  // device 2026-08-02: a long-press at doc(388,1488) with the engine scrolled to y=909
  // reached the page at client y=1488 (should be 579) — every below-the-fold press
  // resolved a screenful low. The earlier "no mapping" pass fixed the old double-ADD of
  // scroll but overshot to zero mapping.
  void docToViewport(int* x, int* y);

 public:
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
  // drainOnly: return as soon as the engine has nothing ready instead of spending the whole
  // budget. The 16 ms tick passes true (time spent waiting is time the timer cannot use); the
  // tests pass false, because they use pump() as "let the engine run for N ms" while a load or
  // a timer completes. See GoannaRenderPage::PumpReady.
  void pump(int msBudget, bool drainOnly = false);
  void maybePaint();
  // Pull the next plugin frame, so the plugin draws on the daemon's clock instead of a second
  // free-running one of its own. CALL ONCE PER TICK, from the tick and nowhere else: it paces
  // itself by COUNTING calls, and maybePaint() is also reached from returnBuffer(), which would
  // put requests off the tick grid — the exact phase noise this exists to remove.
  void requestNextPluginFrame();

private:
  void emitLoadAndLocation();   // poll GoannaRenderPage -> sink messages
  std::string emitLocationAndTitle();           // failure/cert + location + title/url; returns uri ("" if failed)
  std::string emitLocationAndTitleCore();       // location + title/url only (no failure/cert re-check)
  void emitCompletion(bool emitProgress100);    // full boundary: progress/location/title/load-stopped/history
  bool emitGeometry();          // contents-size + meta-viewport (dedup); true if a valid size was read
  void emitScrollIfChanged();   // scrolled-to when the offset moved
  void syncPluginRects();       // interactive-rect add/remove for live plugins (R7)
  void clearPluginRects();      // withdraw every emitted rect (navigation, teardown)
  void drainPluginSpotlight();  // deliver a queued spotlight state change to the plugin
  void emitSelectPopupIfPending(); // drain a queued <select> popup -> write file + msgPopupMenuShow

  EngineHost&        mHost;
  IPageMessageSink&  mSink;
  GoannaRenderPage*  mPage;
  int                mKey1, mKey2, mBufSize;
  // OPTIONAL third shared buffer (YAP 0x1600, sent separately because Connect/Thaw carry two
  // keys and their shapes are frozen). 0 when the adapter did not offer one, in which case
  // everything below behaves exactly as the two-buffer version did.
  int                mKey3 = 0;
  int                mLastWinH = 0;     // last surface height (VKB-shrink scroll-restore, T1)
  // When the surface height last CHANGED, and by how much. Diagnostic only (T-133): the white
  // band on a VKB toggle is rows the adapter composites from OUTSIDE the painted region, and the
  // only thing separating that from a fling which outran the overscan is whether a resize just
  // happened. Read by the uncovered-pan log in setScrollPosition.
  int64_t            mLastWinResizeMs = 0;
  int                mLastWinResizeDh = 0;   // new height - old height (>0 = grow, i.e. VKB down)
  int                mActiveKey;        // which shm buffer we last painted into
  // Cache of attached shared buffers (up to the 2 alternating segments). shmat/shmdt of the ~12MB
  // segment on EVERY paint cost ~400ms/keystroke on the device; attach once and reuse instead.
  unsigned char*     mShmBuf[3];
  int                mShmId[3];
  unsigned char*     attachShm(int keyOrId);   // resolve+attach (cached); nullptr on failure
  void               detachShm();               // shmdt all cached (on thaw/teardown)
  // Double-buffer flow control (F-211): the adapter holds exactly one buffer and returns the
  // previous one (asyncCmdReturnBuffer) each time it receives a new msgPainted. A buffer that has
  // been msgPainted but not yet returned is "in flight" — the adapter may still be blitting it, so
  // overwriting it corrupts/crashes the adapter (seen as an app crash on fast typing). We refuse to
  // repaint an in-flight buffer until returnBuffer clears it, with a timeout valve so a lost return
  // can't deadlock painting. Slot i corresponds to mKey1 (0) / mKey2 (1).
  bool               mInFlight[3];
  int64_t            mPaintMs[3];               // ms timestamp of the msgPainted that put it in flight
  int                slotForKey(int key) const;
  int                nextBufferKey() const;   // 2- or 3-buffer paint rotation
  // Held-Backspace acceleration: consecutive Backspace keyDowns arriving within the auto-repeat
  // window build a run; once it is long enough, Backspace deletes a word at a time (not a char).
  int64_t            mLastBackspaceMs;
  int                mBackspaceRun;
  bool               mLoadWasDone;
  int                mLastProgress;     // last msgLoadProgress value emitted this load (0..99); reset at
                                        // each load start so a stale high value can't freeze the bar
  bool               mWatchdogDismissed; // the stall watchdog cleared the overlay while the engine load
                                         // is still running; a later real completion emits location/
                                         // title/repaint without a fresh load-start/stop boundary (F-324)
  bool               mDirtyPending;      // engine invalidation seen, repaint owed once the rate-limit
                                         // window opens (inspector P2: cap dirty-driven full repaints)
  int64_t            mLastDirtyPaintMs;  // last dirty-driven repaint (ms), for the 150 ms cap
  // Next scheduled publish instant for plugin-driven damage. A GRID, deliberately not
  // "last publish + period": see jihadPluginFramePeriodMs in the .cpp for why a floor drifts
  // and beats where a deadline does not. 0 = no plugin / pacer idle.
  int64_t            mPaceDeadlineMs;
  // Value of GoannaRenderPage::PluginFrameSeq() at the last published frame. The comparison
  // against it is what makes the card 1:1 with the plugin instead of guessing from a boolean.
  uint32_t           mLastPluginFrameSeq;
  // --- plugin PULL state (requestNextPluginFrame) ---
  uint32_t           mLastRequestedSeq;    // seq when we last checked; a change clears outstanding
  int64_t            mPluginReqMs;         // when the outstanding request went out (ms)
  int                mPluginReqTicks;      // ticks since the last request, for the whole-tick grid
  bool               mPluginReqOutstanding; // asked, no frame back yet — do not stack another
  std::string        mAliasUrl;         // non-empty when the current page is an internal
                                        // about: page rendered from inline HTML (about:jihad
                                        // /about:isis): report THIS as the location instead
                                        // of the underlying data: URL the engine sees
  bool               mNeedsPaint;       // set when there is a new frame to send
  int64_t            mLoadStartMs;      // ms when the current load's msgLoadStarted was emitted (0=idle)
  bool               mFrozen;           // card backgrounded: skip painting
  bool               mHadContent;       // a non-blank frame has been produced (suppress blanks over it)
  bool               mGeometryDirty;    // a resize happened; emit geometry from pump once reflow settles
  int                mGeometryTries;    // bounded retries while the size is still degenerate
  // Queued tap: clickAt (a YAP socket callback) only records the point; pump() does the
  // hit-test / activation / click / navigation on the tick, where page teardown is safe.
  // Queued editing keys that run page JS which may focus/navigate (Tab, Enter). Like clickAt these
  // must run in pump(), not synchronously in the keyDown YAP callback, or a focus/submit handler
  // could tear the page down under us (Codex F-219). A QUEUE, not a single slot, so several presses
  // between pump ticks aren't dropped (Codex F-241); see the PEA_* constants in the .cpp.
  std::vector<int>   mPendingEditActions;
  // Queued raw mouse events (F-9). mouseEvent()/holdAt() are YAP socket callbacks with no
  // page-lifetime protection, exactly like clickAt: a mousedown/mouseup/contextmenu runs page JS
  // (onmousedown -> location/document.open) that can tear the document down mid-callback, after
  // which the engine objects we keep using are freed -> the SIGSEGV whose core dump stalled I/O
  // hard enough to REBOOT the device (R1 AC4, review #5 H-1). They only RECORD here; pump()
  // dispatches inside the tick's mInTick/mReap guard where teardown is safe.
  // detail carries numClicks for PM_CLICK and the contextmenu detail for PM_CONTEXTMENU.
  // x/y is point 0 (every non-touch input has exactly one). `pts` carries the FULL point list
  // for a multi-touch event — empty for everything else, so single-point paths are unchanged
  // and the move-coalescing/dedup logic below still keys off x/y.
  struct PendingMouse { int type; int x; int y; int detail;
                        std::vector<std::pair<int,int>> pts; };
  std::vector<PendingMouse> mPendingMouse;
  // PM_* are the queue's own type codes, deliberately NOT the adapter's wire codes: the wire has
  // no contextmenu, and holdAt/clickAt/touchEvent all ride the SAME queue so their relative order
  // is total. Taps are queued rather than kept in a separate mPendingClick slot because the two
  // could invert: YAP sockets live on the default GMainContext, and page JS run during a drain can
  // spin a nested GLib loop (a sync XHR in onmousedown) that delivers the finger-up mouseEvent AND
  // the tap's clickAt while the outer drain is still inside its swapped batch. With a separate
  // slot the click then ran BEFORE the queued mouseup, so the F-1 dedup record (written at
  // mouseup DISPATCH time) did not exist yet, ClickAt fully activated, and the engine later
  // synthesized a second click from the down/up pair — a double activation, i.e. the double form
  // POST this project rates critical. One ordered queue makes that interleaving unrepresentable.
  enum { PM_DOWN = 0, PM_UP = 1, PM_MOVE = 2, PM_CONTEXTMENU = 3, PM_CLICK = 4,
         PM_TOUCHSTART = 5, PM_TOUCHMOVE = 6, PM_TOUCHEND = 7 };

  // ---- double-activation suppressor (T-120) ------------------------------------------------
  // One finger on this device produces TWO independent YAP command streams: the touch commands
  // (only when the adapter's JIHAD_TOUCH_EVENTS switch is on) and the pen/click commands, which
  // are always sent. They share no identifier and no timestamp, so nothing correlates them but
  // order, position and time — see touchSuppressesMouse() for the rule and its failure modes.
  // Costs nothing when touch is off: nothing ever arms it.
  bool               mTouchSeqActive;      // a touchstart has arrived and no touchend yet
  bool               mTouchSeqConsumed;    // ...and the page called preventDefault on it
  int                mTouchAnchorX;        // last touch point of the consumed sequence, DOC coords
  int                mTouchAnchorY;
  int64_t            mTouchSeqStartMs;     // arms the runaway-sequence cap
  int64_t            mTouchSuppressUntilMs;// after touchend: how long the tail-end pen stream is dropped
  // True if this queued mouse/click event is the pen echo of a touch gesture the page consumed.
  bool touchSuppressesMouse(int type, int docX, int docY);
  // Bumped by every explicit navigation. The drain re-reads it each iteration: a navigation
  // delivered by a nested loop mid-drain clears the (already swapped, hence empty) member queue,
  // so without this the rest of the local batch would still dispatch into the superseded document.
  unsigned           mNavGen;
  // ms of the last adapter scroll/zoom update. Engine-driven repaints are deferred while a pan is
  // in flight: renderedX/Y are stamped from mAdapterScrollX/Y, which only move when the adapter
  // sends setScrollPosition, so a repaint mid-fling ships a buffer whose origin disagrees with
  // where the adapter has already panned -> content appears to jump and regions blank out.
  int64_t            mLastScrollMs;
  long               mPrevPaintScrollY = 0;  // pan direction for the overscan bias (paint path)
  // Last painted region (zoomed px + its zoom) — coverage-aware repaint: a scroll whose
  // viewport is still well inside this region needs NO repaint (the adapter pans locally).
  int                mPaintedX = 0;
  long               mPaintedLo = 0, mPaintedHi = 0;
  double             mPaintedZoom = 1.0;
  int64_t            mLastPaintDoneMs = 0;    // pan-cadence refresh (fixed-content drift bound)
  int                mLastContentW, mLastContentH;  // last emitted content size
  int                mLastScrollX, mLastScrollY;     // last emitted scroll offset
  double             mZoom;   // current full-page zoom (for input coord mapping)
  // Absolute finger position during a drag. The adapter sends dragProcess as DELTAS, which
  // is all a scroll needs — but a drag over an open menu is a menu drag-select and has to
  // hit-test an absolute point, so the deltas are accumulated here from dragStart.
  int                mDragX = 0, mDragY = 0;
  double             mLastLoggedZoom = -1.0;   // trace only: last zoom the card asked for
  int                mAdapterScrollX, mAdapterScrollY;  // adapter's scroll in zoomed-content px
  // Interactive (plugin) rects already announced to the adapter, keyed by the id we minted.
  // The adapter NEVER clears its own copy — not on navigation, not on load, not on resize —
  // so every id in here is a promise to send a matching remove. mPluginRectIds maps element
  // identity to that id so a plugin keeps one id across ticks; ids are a small counter and
  // NOT a pointer, because the wire field is a C `int` and a heap address serializes negative.
  struct FlashRect { int l, t, r, b; };
  std::map<void*, int>     mPluginRectIds;
  std::map<int, FlashRect> mEmittedFlashRects;
  int64_t            mLastRectSyncMs = 0;
  int64_t            mLastGeomDirtyMs = 0;   // cadence for the post-load size re-measure
  // Damage-only repaint state (the 30 fps path in paintToSharedBuffer).
  // What each buffer was last FULLY painted with. A partial repaint is only sound into a
  // buffer that already holds a complete frame of the identical geometry, and buffers
  // alternate, so this has to be per-buffer rather than one global record.
  struct BufFrame {
    bool   valid = false;
    int    w = 0, h = 0;
    double zeff = 1.0;
    int    scrollX = 0, scrollY = 0;
    int    paintedX = 0, paintedTop = 0, paintedH = 0;
    long   bandRow = 0;      // first visible row of the band inside the buffer
  };
  BufFrame           mBufFull[3];
  // Drop the ACTIVE slot's record: this buffer no longer provably holds a complete frame, so
  // it must not become the base for a damage-only repaint. Called on every paint path that
  // returns without publishing, and wherever the segments themselves change underneath us.
  void invalidateBufRecord() {
    int fs = slotForKey(mActiveKey);
    if (fs >= 0) mBufFull[fs].valid = false;
  }
  // Accumulated engine damage since the last paint, in viewport device px.
  bool               mDamageValid = false;
  bool               mDamageForceFull = false;   // scroll/zoom/resize/nav happened: full frame
  // Last engine scroll SEEN BY THE DAMAGE CHECK. Separate from mLastScrollX/Y, which belong
  // to the adapter echo and are only maintained at zoom ~1 (see emitScrollIfChanged).
  int                mDamageScrollX = 0, mDamageScrollY = 0;
  int                mDamageX = 0, mDamageY = 0, mDamageW = 0, mDamageH = 0;
  bool               mRectSyncStarted = false;   // see the 32-bit clock note in syncPluginRects
  // Queued plugin-spotlight state (YAP 0x1501/0x1502). Queued, not dispatched in the YAP
  // callback, for the same reason as every other input command: delivery is a synchronous
  // re-entrant call into the plugin, and the callback has no page-lifetime guard.
  bool               mSpotlightPending = false;
  bool               mSpotlightOn = false;
  int                mSpotL = 0, mSpotT = 0, mSpotR = 0, mSpotB = 0;
                                                        // (== BrowserOffscreenInfo::renderedX/Y)
  // Map an adapter surface coordinate to a content/CSS coordinate (R5): input
  // events use content space, so undo zoom + scroll.
  void mapToContent(int sx, int sy, int* cx, int* cy);
  // Land every navigation at the top on BOTH sides of the YAP boundary (see definition).
  void resetScrollForNavigation();

  struct UrlRule;                             // {compiled regex, userData, redirect}
  std::vector<UrlRule*> mRedirectRules;       // addUrlRedirect rules (R6)
  // Returns true if url matched a redirect rule that consumed the load.
  bool applyRedirectRules(const char* url);

  BrowserPageGoanna(const BrowserPageGoanna&) = delete;
  BrowserPageGoanna& operator=(const BrowserPageGoanna&) = delete;
};

} // namespace jihad

#endif // JIHAD_BROWSERPAGEGOANNA_H
