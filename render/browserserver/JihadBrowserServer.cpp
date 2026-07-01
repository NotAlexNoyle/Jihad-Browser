/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
/* Jihad Browser — JihadBrowserServer implementation. See header. */
#include "JihadBrowserServer.h"
#include <cstdio>
// Note: EngineHost is only held/forwarded by reference (forward-declared in the
// header), so this daemon-dispatch layer stays free of XPCOM/engine headers.

JihadBrowserServer::JihadBrowserServer(const char* name, jihad::EngineHost& host)
  : BrowserServerBase(name), mHost(host), mProxy(nullptr), mPage(nullptr) {}

JihadBrowserServer::~JihadBrowserServer() { delete mPage; }

void JihadBrowserServer::clientConnected(YapProxy* proxy) {
  printf("[jihad-bs] client connected\n"); mProxy = proxy;
}
void JihadBrowserServer::clientDisconnected(YapProxy* proxy) {
  printf("[jihad-bs] client disconnected\n");
  if (mProxy == proxy) mProxy = nullptr;
  delete mPage; mPage = nullptr;
}

void JihadBrowserServer::msgPainted(int32_t key) { if (mProxy) BrowserServerBase::msgPainted(mProxy, key); }
void JihadBrowserServer::msgLoadStarted() { if (mProxy) BrowserServerBase::msgLoadStarted(mProxy); }
void JihadBrowserServer::msgLoadProgress(int32_t p) { if (mProxy) BrowserServerBase::msgLoadProgress(mProxy, p); }
void JihadBrowserServer::msgLoadStopped() { if (mProxy) BrowserServerBase::msgLoadStopped(mProxy); }
void JihadBrowserServer::msgLocationChanged(const char* uri, bool b, bool f) { if (mProxy) BrowserServerBase::msgLocationChanged(mProxy, uri, b, f); }
void JihadBrowserServer::msgTitleChanged(const char* t) { if (mProxy) BrowserServerBase::msgTitleChanged(mProxy, t); }

void JihadBrowserServer::tick() {
  if (mPage) { mPage->pump(10); mPage->paintToSharedBuffer(); }
}

// ---- core commands wired to the Goanna backend -----------------------------
void JihadBrowserServer::asyncCmdConnect(YapProxy* proxy, int32_t pageWidth, int32_t pageHeight, int32_t sharedBufferKey1, int32_t sharedBufferKey2, int32_t sharedBufferSize, int32_t identifier)
{
  (void)identifier;
  printf("[jihad-bs] connect %dx%d keys=0x%x,0x%x sz=%d\n", pageWidth, pageHeight, (unsigned)sharedBufferKey1, (unsigned)sharedBufferKey2, sharedBufferSize);
  mProxy = proxy;
  delete mPage;
  mPage = new jihad::BrowserPageGoanna(mHost, *this);
  if (!mPage->init(pageWidth, pageHeight, sharedBufferKey1, sharedBufferKey2, sharedBufferSize)) {
    printf("[jihad-bs] page init FAILED\n"); delete mPage; mPage = nullptr;
  }
}
void JihadBrowserServer::asyncCmdOpenUrl(YapProxy* proxy, const char* url)
{ mProxy = proxy; printf("[jihad-bs] openUrl %s\n", url); if (mPage) mPage->openUrl(url); }
void JihadBrowserServer::asyncCmdSetWindowSize(YapProxy* proxy, int32_t width, int32_t height)
{ (void)proxy; if (mPage) mPage->setWindowSize(width, height); }
void JihadBrowserServer::asyncCmdForward(YapProxy* proxy) { (void)proxy; if (mPage) mPage->pageForward(); }
void JihadBrowserServer::asyncCmdBack(YapProxy* proxy) { (void)proxy; if (mPage) mPage->pageBackward(); }
void JihadBrowserServer::asyncCmdReload(YapProxy* proxy) { (void)proxy; if (mPage) mPage->pageReload(); }
void JihadBrowserServer::asyncCmdStop(YapProxy* proxy) { (void)proxy; if (mPage) mPage->pageStop(); }
void JihadBrowserServer::asyncCmdDisconnect(YapProxy* proxy) { (void)proxy; delete mPage; mPage = nullptr; }

// ---- remaining commands: stubs (T-016) -------------------------------------
void JihadBrowserServer::syncCmdRenderToFile(YapProxy* proxy, const char* filename, int32_t viewX, int32_t viewY, int32_t viewW, int32_t viewH, int32_t& result)
{
  (void)proxy; result = -1; // TODO(T-016)
}

void JihadBrowserServer::asyncCmdSetUserAgent(YapProxy* proxy, const char* userAgent)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetHtml(YapProxy* proxy, const char* url, const char* body)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdClickAt(YapProxy* proxy, int32_t contentX, int32_t contentY, int32_t numClicks, int32_t counter)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdKeyDown(YapProxy* proxy, int32_t key, int32_t modifiers, int32_t chr)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdKeyUp(YapProxy* proxy, int32_t key, int32_t modifiers, int32_t chr)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdPageFocused(YapProxy* proxy, bool focused)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdExit(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdCancelDownload(YapProxy* proxy, const char* url)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdInterrogateClicks(YapProxy* proxy, bool enable)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdZoomSmartCalculateRequest(YapProxy* proxy, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdDragStart(YapProxy* proxy, int32_t contentX, int32_t contentY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdDragProcess(YapProxy* proxy, int32_t deltaX, int32_t deltaY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdDragEnd(YapProxy* proxy, int32_t contentX, int32_t contentY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetMinFontSize(YapProxy* proxy, int32_t minFontSizePt)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdFindString(YapProxy* proxy, const char* str, bool fwd)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdClearSelection(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdClearCache(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdClearCookies(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdPopupMenuSelect(YapProxy* proxy, const char* identifier, int32_t selectedIdx)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetEnableJavaScript(YapProxy* proxy, bool enable)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetBlockPopups(YapProxy* proxy, bool enable)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetAcceptCookies(YapProxy* proxy, bool enable)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdMouseEvent(YapProxy* proxy, int32_t type, int32_t contentX, int32_t contentY, int32_t detail)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGestureEvent(YapProxy* proxy, int32_t type, int32_t contentX, int32_t contentY, double scale, double rotate, int32_t centerX, int32_t centerY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdInspectUrlAtPoint(YapProxy* proxy, int32_t queryNum, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGetHistoryState(YapProxy* proxy, int32_t queryNum)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdClearHistory(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetAppIdentifier(YapProxy* proxy, const char* identifier)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdAddUrlRedirect(YapProxy* proxy, const char* urlRe, int32_t type, bool redirect, const char* userData)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetShowClickedLink(YapProxy* proxy, bool enable)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGetInteractiveNodeRects(YapProxy* proxy, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdIsEditing(YapProxy* proxy, int32_t queryNum)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdInsertStringAtCursor(YapProxy* proxy, const char* text)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdEnableSelection(YapProxy* proxy, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdDisableSelection(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSaveImageAtPoint(YapProxy* proxy, int32_t queryNum, int32_t pointX, int32_t pointY, const char* dstDir)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGetImageInfoAtPoint(YapProxy* proxy, int32_t queryNum, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdIsInteractiveAtPoint(YapProxy* proxy, int32_t queryNum, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGetElementInfoAtPoint(YapProxy* proxy, int32_t queryNum, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSelectAll(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdCopy(YapProxy* proxy, int32_t queryNum)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdPaste(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdCut(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetMouseMode(YapProxy* proxy, int32_t mode)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdDisableEnhancedViewport(YapProxy* proxy, bool disable)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdIgnoreMetaTags(YapProxy* proxy, bool ignore)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetScrollPosition(YapProxy* proxy, int32_t cx, int32_t cy, int32_t cw, int32_t ch)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdPluginSpotlightStart(YapProxy* proxy, int32_t cx, int32_t cy, int32_t cw, int32_t ch)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdPluginSpotlightEnd(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdHideSpellingWidget(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetNetworkInterface(YapProxy* proxy, const char* interfaceName)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdHitTest(YapProxy* proxy, int32_t queryNum, int32_t cx, int32_t cy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetVirtualWindowSize(YapProxy* proxy, int32_t width, int32_t height)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdPrintFrame(YapProxy* proxy, const char* frameName, int32_t lpsJobId, int32_t width, int32_t height, int32_t dpi, bool landscape, bool reverseOrder)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdTouchEvent(YapProxy* proxy, int32_t type, int32_t touchCount, int32_t modifiers, const char* touchesJson)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdHoldAt(YapProxy* proxy, int32_t contentX, int32_t contentY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGetTextCaretBounds(YapProxy* proxy, int32_t queryNum)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdFreeze(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdThaw(YapProxy* proxy, int32_t sharedBufferKey1, int32_t sharedBufferKey2, int32_t sharedBufferSize)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdReturnBuffer(YapProxy* proxy, int32_t sharedBufferKey)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetZoomAndScroll(YapProxy* proxy, double zoom, int32_t cx, int32_t cy)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdScrollLayer(YapProxy* proxy, int32_t id, int32_t deltaX, int32_t deltaY)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetDNSServers(YapProxy* proxy, const char* servers)
{
  (void)proxy; // TODO(T-016): map to BrowserPageGoanna/GoannaRenderPage per PORT-MAP.md
}
