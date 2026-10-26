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

#include "JihadFrameWindow.h"   // T-517: the frame window trailer (render/browserserver/Src)
#include <atomic>
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
// ... and the period while a plugin is on a page (16 unless $JIHAD_TICK_MS is set).
int jihadPluginTickPeriodMs();

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
  virtual void msgSmartZoomCalculateResponseSimple(int32_t pointX, int32_t pointY, int32_t left, int32_t top,
                                                   int32_t right, int32_t bottom, int32_t spotlight) {
    (void)pointX; (void)pointY; (void)left; (void)top; (void)right; (void)bottom; (void)spotlight; }
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
  // A script asked for a new window on `url`. The server turns this into the frozen
  // msgCreatePage(identifier) and opens the address in the card that connects with it.
  virtual void msgNewPageRequested(const char* url) { (void)url; }
  // Content opened a JS dialog (alert/confirm/prompt/auth). syncPipePath is a FIFO the
  // daemon is about to block on: the adapter stashes it, shows the card's dialog, and the
  // card's sendDialogResponse writes the answer back through it. Frozen contract — the
  // adapter and all three cards already implement their ends.
  // Whether the card this sink speaks to can still answer: false once a write to it failed or it
  // disconnected. A dialog's reply wait ends on it (T-540) instead of running its full deadline.
  virtual bool clientAlive() const { return true; }
  // The card's message socket, for a wait to watch for its hang-up; -1 when there is none.
  virtual int clientFd() const { return -1; }
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
  // The answer to the adapter's copy(queryNum) (YAP 0x2032): whether selected text was
  // handed to the card. Sent once the queued command has actually run.
  virtual void msgCopySuccessResponse(int queryNum, bool success) {
    (void)queryNum; (void)success;
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
  // `value2` receives the third argument when there is one (the password of a user-name/password
  // reply).
  bool awaitDialogReply(const std::string& path, const char* what,
                        bool* accept, std::string* value, int* answer = nullptr,
                        std::string* value2 = nullptr);
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
  // T-515, the express lane (JihadFrameLock.h, BrowserPageGoanna.cpp): the card's scroll and zoom
  // (the want lock held by the caller) and its returned buffers (the frame lock held) as the YAP
  // reader thread sees them, ahead of the main loop, and the express thread's step (the frame lock
  // held; true: step again soon).
  // tag: the command's number in its proxy's arrival order (YapProxy); the main loop reports the
  // tag of each scroll or zoom it handled (the frame lock): the main lane's frame serves the card
  // once it has taken up the newest one
  void expressWant(int x, int y, double zoom, bool hasZoom, uint32_t tag);
  void expressCommandDone(uint32_t tag) { if (tag > mXCmdDone) mXCmdDone = tag; }
  void expressReturn(int sharedBufferKey);
  bool expressStep(int64_t now);
  // out of the express lane's reach at once, its frames waited out: the card left (reap), its proxy
  // is deleted now and this page perhaps only at the tick's end
  void expressDetach();
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
  // T-527 device rounds (the inject channel): the card told to scroll to (x, y) CSS px (a
  // scrolled-to: its own view moves, as for a page's own scroll), and where the card is (CSS px)
  void debugCardScrollTo(int x, int y);
  void debugCardFollow(int zoomedY);   // the inject pan: the card's own view to the scroll it was sent (quiet)
  void debugCardScroll(int* x, int* y) const;
  void gpuScrollTo(int x, int y, double z);   // T-502: the engine scroll plus the residual view offset
  // YAP: pluginSpotlightStart/End (0x1501/0x1502). The card's confirmation that it has
  // scrimmed and zoomed to a plugin; forwarded to the plugin as Palm's spotlight system
  // event. Rect arrives in DOCUMENT space as origin + width/height.
  void pluginSpotlightStart(int x, int y, int w, int h);
  void pluginSpotlightEnd();
  void setZoomAndScroll(double zoom, int x, int y);   // YAP: setZoomAndScroll

  // --- navigation (YAP: openUrl/back/forward/reload/stop) ---
  void openUrl(const char* url, const char* referrer = nullptr, uint32_t referrerPolicy = 0, bool fromPage = false);   // see GoannaRenderPage::LoadUrl
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
  // A scaled PNG of what this page is showing, written to `path` at its next real paint.
  void requestThumbnail(const std::string& path);
  // Hand this page to a bundled JS component (see GoannaRenderPage::NotifyChrome).
  bool notifyChrome(const char* topic, const std::string& data) { return mPage && mPage->NotifyChrome(topic, data.c_str()); }
  // The loaded page's certificate and connection, as JSON for the card (the padlock).
  bool pageCertificate(std::string* json) { return mPage && mPage->GetPageCertInfoJson(json); }
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
  void docToViewport(int* x, int* y, bool discrete = true);
  void docOffset(double* ox, double* oy, bool discrete);
  bool gpuAlignForHitTest(bool discrete);
 public:
  // YAP ZoomSmartCalculateRequest: (x,y) in CSS page px; answers with the block to fit.
  void smartZoomRequest(int x, int y);
 private:

 public:
  void insertStringAtCursor(const char* text);     // YAP: insertStringAtCursor
  // YAP: selectAll / copy / cut — the long-press menu's page actions. Done by the bundled
  // component components/jihadPageCommands.js (selection and editing are a few lines of
  // script there and pages of C++ here). QUEUED, like every other input command: two of
  // them can run page script (an `input` event, a selection listener), which must not
  // happen inside the socket callback. queryNum >= 0 is a copy whose result the adapter is
  // waiting for.
  void pageCommand(const char* cmd, int queryNum = -1);
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
  // Whether this page needs the frame-rate tick right now: something to paint, a load running,
  // queued input, a plugin, media playing, or a paint/scroll in the last moment. The daemon
  // backs its tick off while every page says no (Main.cpp, the quiet tick).
  bool isBusy() const;
  // A plugin instance is on this page (its rect has been reported to the card).
  bool hasPlugin() const { return !mEmittedFlashRects.empty(); }
  bool gpuPage() const { return mPage && mPage->GpuPath() && mHadContent; }   // T-502 device rounds

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
  int                mLastWinW = 0;     // last surface width (a rotation is not a keyboard)
  bool               mBlurOwed = false; // keyboard down over a field: blur it in the next pump
  // When the surface height last CHANGED, and by how much. Diagnostic only (T-133): the white
  // band on a VKB toggle is rows the adapter composites from OUTSIDE the painted region, and the
  // only thing separating that from a fling which outran the overscan is whether a resize just
  // happened. Read by the uncovered-pan log in setScrollPosition.
  int64_t            mLastWinResizeMs = 0;
  int                mLastWinResizeDh = 0;   // new height - old height (>0 = grow, i.e. VKB down)
  int                mActiveKey;        // which shm buffer we last painted into
  int                mShownKey = 0;     // T-513: the buffer of the last msgPainted (the card shows it)
  void               noteShown(int key);   // mShownKey moves on; the previous one's in-flight mark restarts
  uint32_t           mLateWrites = 0;   // T-513: jihad_offscreen_late_writes() as last seen
  // Cache of attached shared buffers (up to the 2 alternating segments). shmat/shmdt of the ~12MB
  // segment on EVERY paint cost ~400ms/keystroke on the device; attach once and reuse instead.
  unsigned char*     mShmBuf[3];
  int                mShmId[3];
  size_t             mShmSize[3] = {0, 0, 0};   // T-517: each attached segment's size (its window trailer at the end)
  unsigned char*     attachShm(int keyOrId);   // resolve+attach (cached); nullptr on failure
  // T-517, the frame windows (render/browserserver/Src/JihadFrameWindow.h): a segment's trailer
  // (null when it cannot be had), and the window a frame is published with (written before each
  // msgPainted, 0 = the pixels' start, as every frame was before)
  JihadFrameTrailer* shmTrailer(int key);
  // T-527: what changed on the card since the last published frame, in frame px, for the adapter's
  // partial invalidation (the trailer's dirty words, JihadFrameWindow.h): the union of every
  // composite's change since then, at that frame's geometry. Anything not known (a job given up,
  // another geometry, popups, a software frame, an earlier job not yet polled) makes it the whole.
  struct PubDirty { bool all = true; long lo = 0, hi = 0, x0 = 0, x1 = 0; };
  PubDirty mPubDirty;
  int mPubX = 0, mPubY = 0, mPubW = 0, mPubH = 0; double mPubZoom = -1.0; bool mPubPopups = false;
  long mPubContentH = -1;   // the page's height the published frame's white fill past its end was made for
  uint32_t mPubSeq = 0;   // the publishes so far (the trailer's dirtySeq)
  void pubDirtyAll() { mPubDirty.all = true; }
  void pubDirtyAdd(long lo, long hi, long x0, long x1, bool xKnown, int x, int y, int w, int h, double zoom);
  // at a publish: the rect for the trailer (false: the whole), and the published frame's geometry kept
  bool pubDirtyTake(int x, int y, int w, int h, double zoom, long contentH, bool popups, bool earlierUnknown, long out[4]);
  void               publishWindow(int key, int windowRows, const long* dirty = nullptr);   // T-527: dirty = l, t, r, b (frame px), else the whole
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
  std::atomic<bool>  mLoadWasDone;   // (review 2026-10-26: the express lane reads it, the pump writes it)
  int                mLastProgress;     // last msgLoadProgress value emitted this load (0..99); reset at
                                        // each load start so a stale high value can't freeze the bar
  bool               mWatchdogDismissed; // the stall watchdog cleared the overlay while the engine load
                                         // is still running; a later real completion emits location/
                                         // title/repaint without a fresh load-start/stop boundary (F-324)
  bool               mDirtyPending;      // engine invalidation seen, repaint owed once the rate-limit
  uint32_t           mGpuSeq = 0;        // T-502: the compositor's request count last pulled
  double             mGpuZoom = 0.0;     // T-502: the zoom the compositor viewport was set for
  // T-502: the compositor-side scroll. The engine is scrolled (a repaint, an upload) only when
  // the card's scroll leaves the displayport painted around the anchor; inside it the compositor
  // shifts the content (jihad_offscreen_set_scroll), fixed layers stay. JIHAD_GPU_ENGINE_SCROLL
  // restores the per-step engine scroll.
  bool               mGpuAnchorValid = false;
  double mGpuDpShiftX = 0.0, mGpuDpShiftY = 0.0;   // T-524: the shift the displayport was set with
  double mGpuDpZoom = 0.0;                          // T-524: and the zoom
  int64_t mVelMs = 0; int mVelY = 0; double mScrollVelY = 0.0;   // T-541: the card's scroll speed (card px/ms)
  float mGpuLeadUp = 0.0f, mGpuLeadDown = 0.0f;                  // T-541: the lead last handed to the page
  bool               mGpuCompositorScroll = false;   // the compositor applies the card's scroll (frames at the card's y)
  bool               mGpuRootless = false;           // T-527: the document has no root scroll frame: the world offset is the scroll (the frame lock)
  // T-502 round 112: the compositor-side scroll is engaged by a card-driven pan only. A
  // setScrollPosition that follows a position this daemon just reported (msgScrolledTo) is the
  // card catching up with the page's own scroll; the pan settles 300 ms after the card's last
  // position (the engine scrolled to it, the async scroll off). Otherwise frames follow the engine.
  int64_t            mGpuEchoMs = 0;
  int                mGpuEchoX = 0, mGpuEchoY = 0;
  void               gpuDisengage(bool moveEngine);
  // T-504 (cavekit-gpu-rendering R6): a zoom shows the tiles already painted first (the world
  // transform and the compositor-side scroll change); the displayport for the new shown area and
  // the anchor move wait until a frame at the new zoom is published, or 800 ms (moving them first
  // repainted the page on this thread before any frame: 1.2-1.8 s on desktop Wikipedia, the card
  // grey meanwhile). JIHAD_GPU_NO_ZOOM_HOLD=1 restores the old order.
  bool               mGpuSettledShort = false;   // T-504: the pan settled where the engine cannot scroll; the compositor keeps the rest
  int64_t            mGpuZoomHoldMs = 0;
  int64_t            mGpuZoomCmdMs = 0;      // T-504 (JIHAD_GPU_ZOOM_DUMP): when the zoom last changed
  int                mGpuZoomDumpN = 0;      // the zoom changes so far: the dump file's number
  double             mGpuZoomHoldZ = 0.0;
  void               gpuZoomRelease(const char* why);
  // T-527: a zoom that changes the resolution paints what the card shows first (the critical
  // displayport, GoannaRenderPage::SetGpuDpCritical), the margins once a frame of that paint is out
  int64_t            mGpuCriticalMs = 0;       // the critical displayport set (0: none)
  int64_t            mGpuCriticalPaintMs = 0;  // its first layer paint done
  bool               mGpuCriticalDone = false; // a frame posted after that paint published (either lane)
  void               gpuCriticalEnd(const char* why);
  int                mGpuAnchorX = 0, mGpuAnchorY = 0;
  // T-502: the compositor frame in flight (GoannaRenderPage::ComposeStart / ComposePoll), the
  // geometry it was started for, and per buffer slot the geometry of the frame it holds and
  // the rows that changed since it was read (the next read of that slot is that band only).
  struct GpuJob {
    uint32_t token = 0; int slot = -1, key = 0, x = 0, y = 0, w = 0, h = 0; double zoom = 1.0;
    int64_t t0 = 0; uint32_t seq = 0; long askLo = 0, askHi = 0, ask2Lo = 0, ask2Hi = 0, shift = 0;
    int viewRow = 0;   // T-505: the view's top in the frame (the overscan above it)
    uint32_t xFrames = 0;   // T-515: the express lane's frames when it was posted (newer ones since: dropped)
    uint32_t cmdDone = 0;   // T-515: the card's commands the main loop had handled when it was posted
    int win = 0;            // T-517: the frame's window in its slot (rows into the pixels)
    uint32_t vToken = 0; int vBuf = -1;   // T-513: JIHAD_GPU_VERIFY's reference, composed right behind the job
    uint32_t lostN = 0;     // T-527: mGpuLostN when it was posted (a composite lost since: not adopted)
  };
  void gpuForget(const GpuJob& j);   // the job and its verify reference given up
  // T-505: the view's row in the GPU frame and the residual offset last asked of the compositor
  int mGpuViewRow = 0;
  int mGpuJobViewRow = 0;   // T-507: the published GPU frame's view row (thumbnails)
  // T-509: the GPU path given up (refused jobs, a frame not composed within the bound)
  int mGpuRefusals = 0;
  uint32_t mGpuLostN = 0;   // T-527: gpuSlotsLost calls (a main-lane job posted before one is not adopted)
  void gpuFallBack(const char* why);
  double mGpuViewRX = 0.0, mGpuViewRY = 0.0;
  void gpuSetView(double z, double rx, double ry);
  std::vector<GpuJob> mGpuJobs;              // oldest first, at most two in flight
  bool gpuJobStart(int slot, int key, unsigned char* pixels, size_t bytes, int x, int y, double zoom, int w, int h);
  void gpuJobStartNext(int x, int y, double zoom, int w, int h, size_t segSize, size_t hdr);
  int keyAfter(int key) const;
  int                mGpuJobX = 0, mGpuJobY = 0;   // the geometry of the frame being published
  double             mGpuJobZoom = 1.0;
  int                mGpuJobPolls = 0;        // ticks that found the oldest pending
  uint32_t           mGpuTrace[8] = {0,0,0,0,0,0,0,0};   // device rounds: where ticks went while a job was pending
  int64_t            mGpuDoneMs = 0;          // when the frame came in (the publish timing)
  uint32_t           mGpuPubToken = 0;        // T-527: the job of the main lane's frame being published
  bool               mGpuRestarting = false;  // re-entered to start the next frame right after a publish
  bool               mGpuSlotValid[3] = {false, false, false};
  int                mGpuSlotX[3] = {0, 0, 0}, mGpuSlotY[3] = {0, 0, 0};
  int                mGpuSlotW[3] = {0, 0, 0}, mGpuSlotH[3] = {0, 0, 0};
  double             mGpuSlotZoom[3] = {1.0, 1.0, 1.0};
  long               mGpuStaleLo[3] = {0, 0, 0}, mGpuStaleHi[3] = {0, 0, 0};
  long               mGpuStale2Lo[3] = {0, 0, 0}, mGpuStale2Hi[3] = {0, 0, 0};   // a second stale band
  // T-511: the columns the stale rows are stale in (the hull of the changes' columns; 0..w when a
  // change's columns were not known), meaningful while the slot has stale rows
  long               mGpuStaleX0[3] = {0, 0, 0}, mGpuStaleX1[3] = {0, 0, 0};
  // T-517: the row of each slot's pixels its GPU frame starts at (the frame window,
  // JihadFrameWindow.h); 0 when windows are off, and for the software path's frames
  int                mGpuSlotWin[3] = {0, 0, 0};
  // T-517 (review): the slot's frame had fixed or stuck layers (the engine judges kept rects and
  // fixed columns against the previous composite, so an older base is only taken without them)
  bool               mGpuSlotFixed[3] = {false, false, false};
  bool               windowsFor(int key);   // the adapter that made the segment reads windows, and they are on
  // a composite's change into the other valid slots' stale rows (both lanes, T-517), and the
  // records dropped when a composite's change is in none of them
  void               gpuStaleAdd(int except, long il, long ih, const GoannaRenderPage::ComposeResult& cr, int frameY,
                                 int x, double zoom, int w, int h);
  void               gpuSlotsLost();
  long               mGpuFixedLo = 0, mGpuFixedHi = 0;   // the rows fixed layers cover, as last reported
  int                mGpuLatestSlot = -1, mGpuLatestKey = 0;   // round 113: the newest finished GPU frame
  // round 116: a job started while another is composed (the two-deep queue) also reads the rows
  // that one changed, and builds without the diff's scroll shift (its base is not the previous composite)
  long               mGpuNextExtraLo = 0, mGpuNextExtraHi = 0;
  long               mGpuNextExtraX0 = 0, mGpuNextExtraX1 = 0;   // T-511: their columns
  long               mGpuInvRows[3] = {0, 0, 0};   // T-511: the rows the last three published frames changed
  int                mGpuInvRowsAt = 0;
  bool               mGpuNextSecond = false;
  // round 130: the second job builds on the job in flight (composed: its frame is complete when
  // the second job runs, the compositor thread runs them in order), exact while scrolling
  int                mGpuNextBaseKey = 0, mGpuNextBaseY = 0;
  int                mGpuNextBaseWin = 0;   // T-517: that frame's window
  void gpuJobDrain();   // wait out a frame in flight (its buffer is about to go away)
  void gpuSlotsReset() {
    gpuJobDrain();
    for (int i = 0; i < 3; ++i) mGpuSlotValid[i] = false;
    mGpuLatestSlot = -1;   // round 113
    xInvalidate();
  }
  // T-515, the express lane's state. All of it under the frame lock (JihadFrameLock.h).
  // T-527: velY, the card's scroll speed (zoomed px a ms, smoothed over its last positions) for a frame
  // composed where the card will be when the frame is shown (xStart's prediction); measured over 30 ms
  // at least from the position at velMs (velAtY): two commands read a ms apart made it 8-16 times too fast
  struct XWant { int x = 0, y = 0; double zoom = 1.0; int64_t ms = 0; uint32_t seq = 0, cmdSeq = 0; double velY = 0.0;
                 int64_t velMs = 0; int velAtY = 0; };
  uint32_t mXCmdDone = 0;             // the newest of the card's commands the main loop handled (its tag; the frame lock)
  uint32_t mXCmdPub = 0;              // ...the main lane's newest frame took up
  XWant mXWant;                       // the card's newest position (the reader thread's view; the want lock)
  int64_t mXWaitSinceMs = 0;          // the card has waited for a frame since (the main lane's publish clears it; the want lock)
  double mXZoomNow = 1.0;             // the zoom the main lane last applied (for a scroll before any zoom; the want lock)
  int64_t mXZoomWantMs = 0;           // the card's last new zoom arrived (the want lock; JIHAD_GPU_ZOOM_DUMP)
  int mXZoomDumpN = 0;                // ...its number (the want lock)
  double mXLastZoom = 0.0;            // the zoom of the express lane's last frame (the zoom hold's release)
  // T-527: the resolution the layer tree on the compositor was last painted at (the main thread sets it
  // after a layer paint, under the frame lock): the express lane composes no frame that would draw those
  // tiles at under half their size (a double tap out from 2.33 to 0.69 composed 210 minified tile draws
  // in 471 ms on the Adreno, holding the compositor thread ahead of the paint at the new resolution)
  double mXTreeRes = 1.0;
  // ...and only while a paint at that zoom's resolution would draw them larger: the resolution stops at
  // 1, so under a zoom of 0.5 the tiles at 1 are what any paint makes (a page wider than 1536 px fit-zooms
  // there, and every pan frame was refused: review 2026-10-25)
  bool xUnderHalf(double z) const { return z * 2.0 < mXTreeRes && mXTreeRes > GoannaRenderPage::GpuResolutionFor(z) + 0.0005; }
  // T-527 (R6): the frame the card showed when a zoom in raised the resolution, kept, and published again
  // at once when the card comes back to exactly that zoom (a double tap out to the fit zoom: the old tiles
  // drawn at under half their size make no frame, and the paint at the new resolution came 573-669 ms
  // after the tap). Only while it covers the card's view and the page's height is the same; once.
  // Kept by whichever lane sees the zoom first (the express lane publishes at the new zoom before a busy
  // main thread handles the command), and only a frame at a resolution of 1 that is not at the new zoom;
  // dropped at an input (it would show the page from before it), once the tree is at 1 again, and on a new
  // document, size or freeze (review 2026-10-26).
  struct ZoomBack { bool ok = false; double zoom = 0.0; int x = 0, y = 0, w = 0, h = 0; long contentH = 0; uint32_t pubSeq = 0; };
  ZoomBack mZoomBack;
  std::vector<unsigned char> mZoomBackPx;
  void zoomBackKeep(double newZoom);                      // either lane (the frame lock)
  bool zoomBackPublish(int64_t now, double Z, const XWant& want);   // the express lane (the frame lock)
  void zoomBackDrop() { mZoomBack.ok = false; std::vector<unsigned char>().swap(mZoomBackPx); }
  double mXAdoptedZoom = 0.0;   // the zoom of the last main-lane frame the express lane published for it (the hold's release)
  uint32_t mXAdopted = 0;             // T-527: main-lane frames the express lane published for it
  int64_t mXLastXPubMs = 0;           // the express lane's last publish, while it is the newest frame (0: the main lane's is)
  int mXConsumed[3] = {0, 0, 0};      // returns the express hook applied; returnBuffer skips as many
  // the main lane's GPU frame geometry and buffers, as of its last frame
  struct XSnap {
    bool ok = false; int w = 0, h = 0, frameH = 0, os = 0; size_t hdr = 0;
    int keys[3] = {0, 0, 0}; size_t seg[3] = {0, 0, 0}; unsigned char* base[3] = {nullptr, nullptr, nullptr};
    long contentH = 0; bool popups = false;
  };
  XSnap mXSnap;
  uint32_t mXGeomGen = 0;             // the buffers, the frame's geometry or the document changed
  uint32_t mXLastPostToken = 0;       // the newest composite asked for, by either lane
  // the newest frame composed intact into a slot (either lane): the express lane's base
  struct XPub {
    uint32_t token = 0; int slot = -1, key = 0, x = 0, y = 0, w = 0, h = 0; double zoom = 0.0;
    long fixedLo = 0, fixedHi = -1; int cardX = 0, cardY = 0; int64_t ms = 0;
  };
  XPub mXPub;
  int64_t mXMainPubMs = 0;            // the main lane's last publish
  struct XJob {
    bool on = false; uint32_t token = 0, gen = 0; int slot = -1, key = 0, x = 0, y = 0, w = 0, h = 0, os = 0;
    double zoom = 1.0; long shift = 0; int64_t t0 = 0, wantMs = 0, waitMs = 0; bool whole = true;
    unsigned char* pixels = nullptr; int cardY = 0;
    int win = 0;   // T-517: the frame's window in its slot (pixels is the window's first row)
    int kind = 0;  // T-517: 1 built on its slot's own frame (the window moved), 2 copied from the base, 0 neither
    bool composed = false; long invLo = 0, invHi = 0; int composedY = 0;   // known once composed (being read)
    int64_t composedMs = 0;   // device rounds: when it was seen composed
    bool adopted = false; uint32_t cmdDone = 0;   // T-527: a main-lane job the express lane publishes (its main-lane bookkeeping)
    bool viewOnly = false;   // T-516: the first frame at a new zoom, the card's view read alone (the overscan white)
  };
  XJob mXJob;                         // the express lane's frame in flight (the older one)
  // T-517: the express lane's frame time on its slot's own frame and copied from the base (running
  // averages, ms; 0 = not measured yet): the faster is taken, the other tried every 16th frame. A
  // page whose sticky columns fill the band surface (Wikipedia) pays the own frame's taller bands
  // with whole composites; a page without them saves the copy
  double mXOwnMs = 0.0, mXCopyMs = 0.0;
  uint32_t mXChoiceN = 0;
  XJob mXJob2;                        // the one after it, built on it while it is read (round 130's rule)
  bool mXMainWindow = false;          // the main lane paints its layers with the lock let go of
  int mXMainSlot = -1;                // ...for this slot
  uint32_t mXFrames = 0, mXFailures = 0;
  bool mGpuPainted = false;           // gpuJobStart: the layers are painted already (post only)
  // (and the main lane's first frame of the new geometry or document comes first: its blank
  // suppression and first-paint hold are the main lane's)
  void xInvalidate() { ++mXGeomGen; mXSnap.ok = false; mXPub = XPub(); mXMainPubMs = 0; }
  bool xStart(int64_t now, double zoom, const XWant& want, int64_t waitSince, const XJob* aBase = nullptr);
  // T-517: a forgotten job may have been composed: its change is in no slot's record
  void xForget(XJob& j) { if (j.on && mPage) { mPage->ComposeForget(j.token); gpuSlotsLost(); pubDirtyAll(); } j = XJob(); }
  bool xLeads(int64_t now);           // the card is moving: the express lane makes the frames (the frame lock held)
  int64_t mXMainPaintMs = 0;          // the main lane's last layer paint while the express lane leads
  bool xPoll(int64_t now);
  void xSnapshot(int w, int h, int frameH, int os, size_t hdr);
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
  int64_t            mPluginLastFrameMs = 0; // when the plugin's frame seq last moved (any producer)
  int                mPluginReqTicks;      // ticks since the last request, for the whole-tick grid
  bool               mPluginReqOutstanding; // asked, no frame back yet — do not stack another
  std::string        mAliasUrl;         // non-empty when the current page is an internal
                                        // about: page rendered from inline HTML (about:jihad
                                        // /about:isis): report THIS as the location instead
                                        // of the underlying data: URL the engine sees
  bool               mNeedsPaint;       // set when there is a new frame to send
  int64_t            mLoadStartMs;      // ms when the current load's msgLoadStarted was emitted (0=idle)
  std::string        mThemePreviewDoc;         // the page showing a theme preview (no #fragment), "" = none
  bool               mTitleDirty = false;      // the page changed its title since the last send
  int64_t            mLastTitleEmitMs = 0;     // when a title last went to the card
  std::string        mLastTitleSent;           // and which (a repeat is not sent)
  bool               mFrozen;           // card backgrounded: skip painting
  bool               mHadContent;       // a non-blank frame has been produced (suppress blanks over it)
  bool               mWasSuppressed = false;   // the engine held its first paint at the last look
  int64_t            mSuppressedSinceMs = 0;   // since when (frames are held at most 1 s of it)
  int64_t            mBlankSinceMs;     // first blank-over-good refusal since the load finished (0=none)
  int64_t            mBlankRetryAtMs;   // do not re-render a refused blank frame before this (0=no gate)
  bool               mGeometryDirty;    // a resize happened; emit geometry from pump once reflow settles
  int                mGeometryTries;    // bounded retries while the size is still degenerate
  // Queued tap: clickAt (a YAP socket callback) only records the point; pump() does the
  // hit-test / activation / click / navigation on the tick, where page teardown is safe.
  // Queued editing keys that run page JS which may focus/navigate (Tab, Enter). Like clickAt these
  // must run in pump(), not synchronously in the keyDown YAP callback, or a focus/submit handler
  // could tear the page down under us (Codex F-219). A QUEUE, not a single slot, so several presses
  // between pump ticks aren't dropped (Codex F-241); see the PEA_* constants in the .cpp.
  std::vector<int>   mPendingEditActions;
  struct PendingPageCmd { std::string cmd; int queryNum; unsigned navGen; };
  std::vector<PendingPageCmd> mPendingPageCmds;        // drained in the guarded pump
  // Hand one command to the component NOW (pump only). True if it handed text to the card.
  bool runPageCommand(const char* cmd, const std::string& extraJson = std::string());
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
  int                mPaintedW = 0;          // width of the painted region (>= the viewport's)
  // Composed frames (paintToSharedBuffer): the slot of the newest published frame, the
  // newest generation of off-screen rows, and what maintenance is owed — the region still
  // has rows to grow by, or mSweepLeft of its off-screen rows may predate a change.
  int                mLatestSlot = -1;
  uint32_t           mStripGen = 0;
  bool               mGrowOwed = false;
  long               mRegionColsLogged = -1;   // region width last logged ("region columns:")
  long               mSweepLeft = 0, mSweepRow = 0;
  long               mSweepStep = 512;       // rows per sweep frame (128..1024, by measured cost)
  int64_t            mSweepNotBeforeMs = 0;  // pause after a completed sweep
  int                mMaintenance = 0;       // 1 grow, 2 sweep: the paint being asked for is one
  bool               panCovered(int x, int y);   // is the adapter's viewport inside the painted region?
  long               mPaintedLo = 0, mPaintedHi = 0;
  double             mPaintedZoom = 1.0;
  int64_t            mLastPaintDoneMs = 0;    // pan-cadence refresh (fixed-content drift bound)
  int                mLastContentW, mLastContentH;  // last emitted content size
  // last meta-viewport sent to the card (emitGeometry sends it only on change)
  bool               mPopupWasOpen = false;   // a XUL popup was open at the last pump (repaint on change)
  bool               mViewportSent = false;
  bool               mLayoutFollowed = false;   // T-499: this page already got its layout viewport
  uint64_t           mLayoutDocKey = 0;         // T-499: the document that is about (DocumentKey)
  double             mVpIs = 0, mVpMn = 0, mVpMx = 0; int mVpW = 0, mVpH = 0; bool mVpUs = true;
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
    int    paintedW = 0;     // the region's width = the buffer's row stride in px (>= w)
    long   bandRow = 0;      // first visible row of the band inside the buffer
    long   bandCol = 0;      // first visible column (0 unless the region is wider than w)
    // For composing the next frame from this one (paintToSharedBuffer, "build on the frame
    // already painted"): which document the rows are of, which generation of off-screen
    // rows they are, and whether a popup was drawn over them.
    uint64_t docId = 0;
    uint32_t stripGen = 0;
    bool   overlay = false;
    // Damage accumulated since THIS buffer was last painted, in viewport device px. A
    // damage-only repaint must cover everything the buffer missed, not just the newest frame:
    // buffers alternate, so a buffer painted N frames ago skipped every intermediate frame's
    // damage. Using the single global since-last-paint rect instead left a trail of stale
    // copies of a small moving element (the media scrubber thumb) that flickered as the
    // buffers cycled (2026-08-16). Reset when this buffer is painted (full or partial).
    bool   dmgValid = false;
    int    dmgX = 0, dmgY = 0, dmgW = 0, dmgH = 0;
  };
  BufFrame           mBufFull[3];
  // Union a new viewport-px damage rect into EVERY buffer's pending-damage box: each buffer
  // needs to repaint that region the next time it is the paint target, whenever that is.
  void accumulateBufDamage(int x, int y, int w, int h) {
    for (int i = 0; i < 3; ++i) {
      BufFrame& b = mBufFull[i];
      if (!b.dmgValid) { b.dmgX = x; b.dmgY = y; b.dmgW = w; b.dmgH = h; b.dmgValid = true; }
      else {
        int x0 = b.dmgX < x ? b.dmgX : x;
        int y0 = b.dmgY < y ? b.dmgY : y;
        int x1 = (b.dmgX + b.dmgW) > (x + w) ? (b.dmgX + b.dmgW) : (x + w);
        int y1 = (b.dmgY + b.dmgH) > (y + h) ? (b.dmgY + b.dmgH) : (y + h);
        b.dmgX = x0; b.dmgY = y0; b.dmgW = x1 - x0; b.dmgH = y1 - y0;
      }
    }
  }
  void clearBufDamage(int slot) {
    if (slot >= 0 && slot < 3) {
      BufFrame& b = mBufFull[slot];
      b.dmgValid = false; b.dmgX = b.dmgY = b.dmgW = b.dmgH = 0;
    }
  }
  // Drop the ACTIVE slot's record: this buffer no longer provably holds a complete frame, so
  // it must not become the base for a damage-only repaint. Called on every paint path that
  // returns without publishing, and wherever the segments themselves change underneath us.
  // Bookmark thumbnails: the path the next real paint should write a scaled PNG of itself to.
  std::string mThumbPath;
  bool writeThumbnail(const unsigned char* src, int w, int h, const std::string& path);
  void invalidateBufRecord() {
    int fs = slotForKey(mActiveKey);
    if (fs >= 0) { mBufFull[fs].valid = false; mGpuSlotValid[fs] = false; }
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
