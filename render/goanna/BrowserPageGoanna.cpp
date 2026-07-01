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
    mLoadWasDone(false), mNeedsPaint(false) {}

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

void BrowserPageGoanna::setWindowSize(uint32_t, uint32_t) {
  // TODO(T-016): resize GoannaRenderPage + reallocate/re-key the shared buffers.
}

void BrowserPageGoanna::openUrl(const char* url) {
  if (!mPage || !url) return;
  mLoadWasDone = false;
  mNeedsPaint = false;
  mSink.msgLoadStarted();
  if (!mPage->LoadUrl(url)) {
    // Don't leave the adapter permanently "loading" on a bad URL (Codex P2).
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
    mSink.msgLoadStopped();
    mSink.msgLocationChanged(mPage->CurrentUri().c_str(),
                             mPage->CanGoBack(), mPage->CanGoForward());
  }
}

void BrowserPageGoanna::pump(int msBudget) {
  if (!mPage) return;
  mPage->PumpFor(msBudget);
  emitLoadAndLocation();
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
