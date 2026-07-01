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
    mKey1(0), mKey2(0), mBufSize(0), mActiveKey(0), mLoadWasDone(false) {}

BrowserPageGoanna::~BrowserPageGoanna() {
  delete mPage;
  // In the daemon the adapter owns the shared segments; standalone we created
  // them in init(), so remove them here to avoid leaking kernel objects.
  for (int k : { mKey1, mKey2 }) {
    if (!k) continue;
    int id = shmget(k, mBufSize, 0);
    if (id >= 0) shmctl(id, IPC_RMID, nullptr);
  }
}

bool BrowserPageGoanna::init(uint32_t width, uint32_t height,
                             int sharedBufferKey1, int sharedBufferKey2, int sharedBufferSize) {
  mKey1 = sharedBufferKey1;
  mKey2 = sharedBufferKey2;
  mBufSize = sharedBufferSize ? sharedBufferSize : (int)(width * height * 4);
  mActiveKey = mKey1;

  mPage = new GoannaRenderPage(mHost);
  if (!mPage->Create((int)width, (int)height)) {
    delete mPage; mPage = nullptr;
    return false;
  }
  // Ensure the shared segments exist (the adapter provides them in the daemon;
  // create-if-absent keeps the standalone path working).
  for (int k : { mKey1, mKey2 }) {
    if (k) (void)shmget(k, mBufSize, IPC_CREAT | 0600);
  }
  return true;
}

void BrowserPageGoanna::setWindowSize(uint32_t, uint32_t) {
  // TODO(T-016): resize GoannaRenderPage + reallocate/re-key the shared buffers.
}

void BrowserPageGoanna::openUrl(const char* url) {
  if (!mPage) return;
  mLoadWasDone = false;
  mSink.msgLoadStarted();
  mPage->LoadUrl(url);
}

void BrowserPageGoanna::pageBackward() { if (mPage) mPage->GoBack(); }
void BrowserPageGoanna::pageForward() { if (mPage) mPage->GoForward(); }
void BrowserPageGoanna::pageReload()  { if (mPage) mPage->Reload(); }
void BrowserPageGoanna::pageStop()    { if (mPage) mPage->Stop(); }

void BrowserPageGoanna::emitLoadAndLocation() {
  if (!mPage) return;
  if (mPage->LoadDone() && !mLoadWasDone) {
    mLoadWasDone = true;
    mSink.msgLoadProgress(100);
    mSink.msgLoadStopped();
    // TODO(T-016): real canGoBack/canGoForward from nsIWebNavigation.
    mSink.msgLocationChanged(mPage->CurrentUri().c_str(), false, false);
  }
}

void BrowserPageGoanna::pump(int msBudget) {
  if (!mPage) return;
  mPage->PumpFor(msBudget);
  emitLoadAndLocation();
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

  mSink.msgPainted(mActiveKey);
  // Double buffer: next paint targets the other segment (the adapter returns
  // the one it finished displaying via returnBuffer in the daemon).
  mActiveKey = (mActiveKey == mKey1 && mKey2) ? mKey2 : mKey1;
}

} // namespace jihad
