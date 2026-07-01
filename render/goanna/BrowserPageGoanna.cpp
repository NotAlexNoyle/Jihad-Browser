/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — BrowserPageGoanna implementation. See BrowserPageGoanna.h.
 * Bridges the YAP BrowserPage command surface to GoannaRenderPage and emits the
 * server->adapter messages via IPageMessageSink.
 */
#include "BrowserPageGoanna.h"
#include "EngineHost.h"
#include "GoannaRenderPage.h"

#include <cstdio>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace jihad {

BrowserPageGoanna::BrowserPageGoanna(EngineHost& host, IPageMessageSink& sink)
  : mHost(host), mSink(sink), mPage(nullptr),
    mKey1(0), mKey2(0), mBufSize(0), mActiveKey(0),
    mLoadWasDone(false), mNeedsPaint(false),
    mLastContentW(-1), mLastContentH(-1),
    mLastScrollX(-1), mLastScrollY(-1) {}

BrowserPageGoanna::~BrowserPageGoanna() {
  // The BrowserAdapter owns the shared segments (it allocated them and passed
  // the keys via connect()); the daemon must NOT IPC_RMID them (Codex P1).
  delete mPage;
}

bool BrowserPageGoanna::init(uint32_t width, uint32_t height,
                             int sharedBufferKey1, int sharedBufferKey2, int sharedBufferSize) {
  // Validate the adapter-provided geometry/buffers (Codex P2): no overflow, and
  // the segment must be large enough for width*height*4.
  if (width == 0 || height == 0 || width > 8192 || height > 8192) return false;
  const size_t need = (size_t)width * height * 4;
  if (sharedBufferSize <= 0 || (size_t)sharedBufferSize < need) return false;
  if (!sharedBufferKey1) return false;

  mKey1 = sharedBufferKey1;
  mKey2 = sharedBufferKey2;
  mBufSize = sharedBufferSize;
  mActiveKey = mKey1;

  // Attach-only: the adapter created the segments (Codex P2). If they are not
  // present, connect() setup was wrong — fail rather than mint daemon-owned ones.
  for (int k : { mKey1, mKey2 }) {
    if (!k) continue;
    if (shmget(k, mBufSize, 0) < 0) return false;
  }

  mPage = new GoannaRenderPage(mHost);
  if (!mPage->Create((int)width, (int)height)) {
    delete mPage; mPage = nullptr;
    return false;
  }
  return true;
}

void BrowserPageGoanna::setWindowSize(uint32_t width, uint32_t height) {
  if (!mPage) return;
  // The BrowserAdapter owns the shared framebuffer and its size (Codex P1/P2);
  // the daemon must not paint beyond it. Only accept a surface that still fits
  // the segments handed to us at connect(). Growing past that needs the adapter
  // to re-allocate + re-key first (a returnBuffer/reconnect round-trip).
  if (width == 0 || height == 0) return;
  if ((size_t)width * height * 4 > (size_t)mBufSize) return;
  if (mPage->Resize((int)width, (int)height)) { mNeedsPaint = true; emitGeometry(); }
}

void BrowserPageGoanna::returnBuffer(int /*sharedBufferKey*/) {
  // The adapter is done with a painted buffer. The current model paints once per
  // load into alternating segments, so there is no in-flight bookkeeping to undo;
  // accepting the return (without error) satisfies the contract and lets a
  // subsequent paint proceed. Full double-buffer flow-control is future work.
}

void BrowserPageGoanna::setScrollPosition(int x, int y) {
  if (!mPage) return;
  // The javascript: scroll applies asynchronously; the scrolled-to message is
  // emitted from pump() once the offset actually moves (emitScrollIfChanged).
  mPage->ScrollTo(x, y);
  mNeedsPaint = true;
}

void BrowserPageGoanna::setZoomAndScroll(double zoom, int x, int y) {
  if (!mPage) return;
  mPage->SetZoom(zoom);
  mPage->ScrollTo(x, y);
  mNeedsPaint = true;
  emitGeometry();   // zoom changes the rendered content size (scroll via pump)
}

void BrowserPageGoanna::openUrl(const char* url) {
  if (!mPage || !url) return;
  mLoadWasDone = false;
  mNeedsPaint = false;
  mSink.msgLoadStarted();
  if (!mPage->LoadUrl(url)) {
    // Synchronous rejection (bad/unknown-scheme URL): report it as a failed load
    // and don't leave the adapter permanently "loading" (Codex P2 + R3).
    mSink.msgFailedLoad("Goanna", 0, url, "Load failed");
    mSink.msgLoadProgress(100);
    mSink.msgLoadStopped();
    mLoadWasDone = true;
  }
}

void BrowserPageGoanna::setHTML(const char* /*url*/, const char* body) {
  if (!mPage || !body) return;
  mLoadWasDone = false; mNeedsPaint = false;
  mSink.msgLoadStarted();
  if (!mPage->SetHtml(body)) { mSink.msgLoadStopped(); mLoadWasDone = true; }
}

// Nav commands restart the load lifecycle so completion re-emits load+location.
void BrowserPageGoanna::pageBackward() { if (mPage) { mLoadWasDone=false; mNeedsPaint=false; mSink.msgLoadStarted(); mPage->GoBack(); } }
void BrowserPageGoanna::pageForward() { if (mPage) { mLoadWasDone=false; mNeedsPaint=false; mSink.msgLoadStarted(); mPage->GoForward(); } }
void BrowserPageGoanna::pageReload()  { if (mPage) { mLoadWasDone=false; mNeedsPaint=false; mSink.msgLoadStarted(); mPage->Reload(); } }
void BrowserPageGoanna::pageStop()    { if (mPage) mPage->Stop(); }
void BrowserPageGoanna::clearHistory() { if (mPage) mPage->ClearHistory(); }
void BrowserPageGoanna::getHistoryState(bool* back, bool* fwd) {
  if (back) *back = mPage && mPage->CanGoBack();
  if (fwd)  *fwd  = mPage && mPage->CanGoForward();
}

void BrowserPageGoanna::clickAt(int x, int y, int numClicks) {
  if (mPage) { mPage->ClickAt(x, y, numClicks); mNeedsPaint = true; }
}
void BrowserPageGoanna::keyDown(int key, int modifiers, int chr) {
  if (mPage) { mPage->KeyEvent("keydown", key, chr, modifiers); mNeedsPaint = true; }
}
void BrowserPageGoanna::keyUp(int key, int modifiers, int chr) {
  if (mPage) { mPage->KeyEvent("keyup", key, chr, modifiers); mNeedsPaint = true; }
}
void BrowserPageGoanna::mouseEvent(int type, int x, int y, int /*detail*/) {
  if (!mPage) return;
  const char* t = (type == 1) ? "mousedown" : (type == 2) ? "mouseup" : "mousemove";
  mPage->MouseEvent(t, x, y, 0);
  mNeedsPaint = true;
}
void BrowserPageGoanna::settingsJavaScriptEnabled(bool enable) {
  if (mPage) mPage->SetJavaScriptEnabled(enable);
}
void BrowserPageGoanna::touchEvent(int type, int /*count*/, int /*mods*/, const char* touchesJson) {
  if (!mPage) return;
  // Minimal parse of the first touch point's x/y from the touches JSON.
  int x = 0, y = 0;
  if (touchesJson) {
    const char* px = strstr(touchesJson, "\"x\"");
    const char* py = strstr(touchesJson, "\"y\"");
    if (px) sscanf(px, "\"x\"%*[: ]%d", &x);
    if (py) sscanf(py, "\"y\"%*[: ]%d", &y);
  }
  const char* t = (type == 0) ? "touchstart" : (type == 2) ? "touchend" : "touchmove";
  mPage->TouchEvent(t, x, y);
  mNeedsPaint = true;
}

void BrowserPageGoanna::emitLoadAndLocation() {
  if (!mPage) return;
  if (mPage->LoadDone() && !mLoadWasDone) {
    mLoadWasDone = true;
    mNeedsPaint = true;   // paint the final frame once (dedup — Codex P2)
    mSink.msgLoadProgress(100);
    // R5: an overridable certificate error surfaces as an SSL-confirm dialog
    // rather than a generic failed load. R3: other network failures -> failed.
    bool failed = false; int code = 0; std::string furl;
    mPage->GetLoadError(&failed, &code, &furl);
    std::string chost; int ccode = 0;
    bool certErr = mPage->GetCertError(&chost, &ccode);
    if (certErr) {
      mSink.msgSSLConfirm(chost.c_str(), ccode, "");
    } else if (failed) {
      mSink.msgFailedLoad("Goanna", code, furl.c_str(), "Load failed");
    }
    mSink.msgLoadStopped();
    std::string uri = mPage->CurrentUri();
    if (mPage->DidRedirect()) mSink.msgUrlRedirected(uri.c_str(), "");  // R4
    mSink.msgLocationChanged(uri.c_str(), mPage->CanGoBack(), mPage->CanGoForward());
    if (!failed) mSink.msgUpdateGlobalHistory(uri.c_str(), false);  // R6
    emitGeometry();   // R4: contents-size + meta-viewport once the page settled
  }
}

void BrowserPageGoanna::emitGeometry() {
  if (!mPage) return;
  int cw = 0, ch = 0;
  if (mPage->GetContentSize(&cw, &ch) &&
      (cw != mLastContentW || ch != mLastContentH)) {
    mLastContentW = cw; mLastContentH = ch;
    mSink.msgContentsSizeChanged(cw, ch);        // R4: contents-size-changed
  }
  double is = 1.0, mn = 1.0, mx = 1.0; int vw = 0, vh = 0; bool us = true;
  if (mPage->GetViewport(&is, &mn, &mx, &vw, &vh, &us))
    mSink.msgMetaViewportSet(is, mn, mx, vw, vh, us);   // R4: meta-viewport
}

void BrowserPageGoanna::emitScrollIfChanged() {
  if (!mPage) return;
  int sx = 0, sy = 0;
  if (mPage->GetScrollXY(&sx, &sy) &&
      (sx != mLastScrollX || sy != mLastScrollY)) {
    mLastScrollX = sx; mLastScrollY = sy;
    mSink.msgScrolledTo(sx, sy);          // R4: scrolled-to
  }
}

void BrowserPageGoanna::pump(int msBudget) {
  if (!mPage) return;
  mPage->PumpFor(msBudget);
  emitLoadAndLocation();
  emitScrollIfChanged();
}

void BrowserPageGoanna::maybePaint() {
  if (mNeedsPaint) paintToSharedBuffer();   // only when there is a new frame
}

void BrowserPageGoanna::paintToSharedBuffer() {
  if (!mPage || !mActiveKey) return;
  int id = shmget(mActiveKey, mBufSize, 0);
  if (id < 0) { perror("[BrowserPageGoanna] shmget"); return; }
  unsigned char* buf = (unsigned char*)shmat(id, nullptr, 0);
  if (buf == (unsigned char*)-1) { perror("[BrowserPageGoanna] shmat"); return; }

  long nb = mPage->ReadPixels(buf, (size_t)mBufSize);
  shmdt(buf);
  if (nb < 0) return;

  mNeedsPaint = false;
  mSink.msgPainted(mActiveKey);
  // Double buffer: next paint targets the other segment. NOTE (Codex P1): a
  // correct daemon must wait for the adapter's returnBuffer before reusing a
  // buffer; asyncCmdReturnBuffer is still a stub. With maybePaint() we paint
  // once per load so we don't race, but full double-buffering is T-016 work.
  mActiveKey = (mActiveKey == mKey1 && mKey2) ? mKey2 : mKey1;
}

} // namespace jihad
