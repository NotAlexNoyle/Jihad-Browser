/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
/* Jihad Browser — JihadBrowserServer implementation. See header. */
#include "JihadBrowserServer.h"
#include <cstdio>
// EngineHost is only forwarded by reference (forward-declared), so this
// dispatch layer stays free of XPCOM/engine headers.

// ---- ProxySink: route one page's msg* to its client proxy ------------------
void ProxySink::msgPainted(int32_t key)      { mSrv->msgPainted(mProxy, key); }
void ProxySink::msgLoadStarted()             { mSrv->msgLoadStarted(mProxy); }
void ProxySink::msgLoadProgress(int32_t p)   { mSrv->msgLoadProgress(mProxy, p); }
void ProxySink::msgLoadStopped()             { mSrv->msgLoadStopped(mProxy); }
void ProxySink::msgLocationChanged(const char* uri, bool b, bool f) { mSrv->msgLocationChanged(mProxy, uri, b, f); }
void ProxySink::msgTitleChanged(const char* t) { mSrv->msgTitleChanged(mProxy, t); }
void ProxySink::msgContentsSizeChanged(int32_t w, int32_t h) { mSrv->msgContentsSizeChanged(mProxy, w, h); }
void ProxySink::msgScrolledTo(int32_t x, int32_t y) { mSrv->msgScrolledTo(mProxy, x, y); }
void ProxySink::msgMetaViewportSet(double is, double mn, double mx, int32_t w, int32_t h, bool us) { mSrv->msgMetaViewportSet(mProxy, is, mn, mx, w, h, us); }
void ProxySink::msgFailedLoad(const char* d, int32_t c, const char* u, const char* desc) { mSrv->msgFailedLoad(mProxy, d, c, u, desc); }
void ProxySink::msgUpdateGlobalHistory(const char* u, bool reload) { mSrv->msgUpdateGlobalHistory(mProxy, u, reload); }

// ---- server ----------------------------------------------------------------
JihadBrowserServer::JihadBrowserServer(const char* name, jihad::EngineHost& host)
  : BrowserServerBase(name), mHost(host), mInTick(false) {}

JihadBrowserServer::~JihadBrowserServer() {
  for (auto& kv : mPages) { delete kv.second.page; delete kv.second.sink; }
  for (auto& e : mReap)   { delete e.page; delete e.sink; }
}

jihad::BrowserPageGoanna* JihadBrowserServer::pageFor(YapProxy* proxy) {
  auto it = mPages.find(proxy);
  return it == mPages.end() ? nullptr : it->second.page;
}

void JihadBrowserServer::reap(YapProxy* proxy) {
  auto it = mPages.find(proxy);
  if (it == mPages.end()) return;
  Page pg = it->second;
  mPages.erase(it);
  if (mInTick) mReap.push_back(pg);          // defer delete out of the tick loop
  else { delete pg.page; delete pg.sink; }
}

void JihadBrowserServer::clientConnected(YapProxy* proxy) {
  (void)proxy; printf("[jihad-bs] client connected\n");
}
void JihadBrowserServer::clientDisconnected(YapProxy* proxy) {
  printf("[jihad-bs] client disconnected\n");
  reap(proxy);
}

void JihadBrowserServer::tick() {
  // Snapshot page pointers so a connect/disconnect during nested GLib pumping
  // can't invalidate our iteration or delete a page under us (Codex P0).
  std::vector<jihad::BrowserPageGoanna*> snap;
  snap.reserve(mPages.size());
  for (auto& kv : mPages) snap.push_back(kv.second.page);

  mInTick = true;
  for (auto* pg : snap) { pg->pump(10); pg->maybePaint(); }
  mInTick = false;

  for (auto& e : mReap) { delete e.page; delete e.sink; }
  mReap.clear();
}

// ---- core commands wired to the Goanna backend -----------------------------
void JihadBrowserServer::asyncCmdConnect(YapProxy* proxy, int32_t pageWidth, int32_t pageHeight, int32_t sharedBufferKey1, int32_t sharedBufferKey2, int32_t sharedBufferSize, int32_t identifier)
{
  (void)identifier;
  printf("[jihad-bs] connect %dx%d keys=0x%x,0x%x sz=%d\n", pageWidth, pageHeight, (unsigned)sharedBufferKey1, (unsigned)sharedBufferKey2, sharedBufferSize);
  reap(proxy);                                     // replace any existing page
  ProxySink* sink = new ProxySink(this, proxy);
  jihad::BrowserPageGoanna* page = new jihad::BrowserPageGoanna(mHost, *sink);
  if (!page->init(pageWidth, pageHeight, sharedBufferKey1, sharedBufferKey2, sharedBufferSize)) {
    printf("[jihad-bs] page init FAILED\n"); delete page; delete sink; return;
  }
  mPages[proxy] = Page{ page, sink };
}
void JihadBrowserServer::asyncCmdOpenUrl(YapProxy* proxy, const char* url)
{ printf("[jihad-bs] openUrl %s\n", url ? url : "(null)"); if (auto* p = pageFor(proxy)) p->openUrl(url); }
void JihadBrowserServer::asyncCmdSetWindowSize(YapProxy* proxy, int32_t width, int32_t height)
{ if (auto* p = pageFor(proxy)) p->setWindowSize(width, height); }
void JihadBrowserServer::asyncCmdForward(YapProxy* proxy) { if (auto* p = pageFor(proxy)) p->pageForward(); }
void JihadBrowserServer::asyncCmdBack(YapProxy* proxy)    { if (auto* p = pageFor(proxy)) p->pageBackward(); }
void JihadBrowserServer::asyncCmdReload(YapProxy* proxy)  { if (auto* p = pageFor(proxy)) p->pageReload(); }
void JihadBrowserServer::asyncCmdStop(YapProxy* proxy)    { if (auto* p = pageFor(proxy)) p->pageStop(); }
void JihadBrowserServer::asyncCmdDisconnect(YapProxy* proxy) { reap(proxy); }

// ---- remaining commands: stubs (T-016) -------------------------------------
void JihadBrowserServer::syncCmdRenderToFile(YapProxy* proxy, const char* filename, int32_t viewX, int32_t viewY, int32_t viewW, int32_t viewH, int32_t& result)
{
  (void)proxy; result = -1; // TODO(T-016)
}

void JihadBrowserServer::asyncCmdSetUserAgent(YapProxy* proxy, const char* userAgent)
{
  (void)proxy; jihad::SetUserAgentOverride(userAgent);   // process-global pref
}

void JihadBrowserServer::asyncCmdSetHtml(YapProxy* proxy, const char* url, const char* body)
{
  if (auto* p = pageFor(proxy)) p->setHTML(url, body);
}

void JihadBrowserServer::asyncCmdClickAt(YapProxy* proxy, int32_t contentX, int32_t contentY, int32_t numClicks, int32_t counter)
{
  (void)counter;
  if (auto* p = pageFor(proxy)) p->clickAt(contentX, contentY, numClicks);
}

void JihadBrowserServer::asyncCmdKeyDown(YapProxy* proxy, int32_t key, int32_t modifiers, int32_t chr)
{
  if (auto* p = pageFor(proxy)) p->keyDown(key, modifiers, chr);
}

void JihadBrowserServer::asyncCmdKeyUp(YapProxy* proxy, int32_t key, int32_t modifiers, int32_t chr)
{
  if (auto* p = pageFor(proxy)) p->keyUp(key, modifiers, chr);
}

void JihadBrowserServer::asyncCmdPageFocused(YapProxy* proxy, bool focused)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdExit(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdCancelDownload(YapProxy* proxy, const char* url)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdInterrogateClicks(YapProxy* proxy, bool enable)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdZoomSmartCalculateRequest(YapProxy* proxy, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdDragStart(YapProxy* proxy, int32_t contentX, int32_t contentY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdDragProcess(YapProxy* proxy, int32_t deltaX, int32_t deltaY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdDragEnd(YapProxy* proxy, int32_t contentX, int32_t contentY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetMinFontSize(YapProxy* proxy, int32_t minFontSizePt)
{
  (void)proxy; jihad::SetMinFontSize(minFontSizePt);
}

void JihadBrowserServer::asyncCmdFindString(YapProxy* proxy, const char* str, bool fwd)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdClearSelection(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdClearCache(YapProxy* proxy)
{
  (void)proxy; jihad::ClearCache();
}

void JihadBrowserServer::asyncCmdClearCookies(YapProxy* proxy)
{
  (void)proxy; jihad::ClearCookies();
}

void JihadBrowserServer::asyncCmdPopupMenuSelect(YapProxy* proxy, const char* identifier, int32_t selectedIdx)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetEnableJavaScript(YapProxy* proxy, bool enable)
{
  if (auto* p = pageFor(proxy)) p->settingsJavaScriptEnabled(enable);
}

void JihadBrowserServer::asyncCmdSetBlockPopups(YapProxy* proxy, bool enable)
{
  (void)proxy; jihad::SetBlockPopups(enable);
}

void JihadBrowserServer::asyncCmdSetAcceptCookies(YapProxy* proxy, bool enable)
{
  (void)proxy; jihad::SetAcceptCookies(enable);
}

void JihadBrowserServer::asyncCmdMouseEvent(YapProxy* proxy, int32_t type, int32_t contentX, int32_t contentY, int32_t detail)
{
  if (auto* p = pageFor(proxy)) p->mouseEvent(type, contentX, contentY, detail);
}

void JihadBrowserServer::asyncCmdGestureEvent(YapProxy* proxy, int32_t type, int32_t contentX, int32_t contentY, double scale, double rotate, int32_t centerX, int32_t centerY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdInspectUrlAtPoint(YapProxy* proxy, int32_t queryNum, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGetHistoryState(YapProxy* proxy, int32_t queryNum)
{
  bool back = false, fwd = false;
  if (auto* p = pageFor(proxy)) p->getHistoryState(&back, &fwd);
  msgGetHistoryStateResponse(proxy, queryNum, back, fwd);   // carries the queryNum
}

void JihadBrowserServer::asyncCmdClearHistory(YapProxy* proxy)
{
  if (auto* p = pageFor(proxy)) p->clearHistory();
}

void JihadBrowserServer::asyncCmdSetAppIdentifier(YapProxy* proxy, const char* identifier)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdAddUrlRedirect(YapProxy* proxy, const char* urlRe, int32_t type, bool redirect, const char* userData)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetShowClickedLink(YapProxy* proxy, bool enable)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGetInteractiveNodeRects(YapProxy* proxy, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdIsEditing(YapProxy* proxy, int32_t queryNum)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdInsertStringAtCursor(YapProxy* proxy, const char* text)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdEnableSelection(YapProxy* proxy, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdDisableSelection(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSaveImageAtPoint(YapProxy* proxy, int32_t queryNum, int32_t pointX, int32_t pointY, const char* dstDir)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGetImageInfoAtPoint(YapProxy* proxy, int32_t queryNum, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdIsInteractiveAtPoint(YapProxy* proxy, int32_t queryNum, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGetElementInfoAtPoint(YapProxy* proxy, int32_t queryNum, int32_t pointX, int32_t pointY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSelectAll(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdCopy(YapProxy* proxy, int32_t queryNum)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdPaste(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdCut(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetMouseMode(YapProxy* proxy, int32_t mode)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdDisableEnhancedViewport(YapProxy* proxy, bool disable)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdIgnoreMetaTags(YapProxy* proxy, bool ignore)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetScrollPosition(YapProxy* proxy, int32_t cx, int32_t cy, int32_t cw, int32_t ch)
{
  (void)cw; (void)ch; // cw/ch describe the content window; scroll uses cx/cy.
  if (auto* p = pageFor(proxy)) p->setScrollPosition(cx, cy);
}

void JihadBrowserServer::asyncCmdPluginSpotlightStart(YapProxy* proxy, int32_t cx, int32_t cy, int32_t cw, int32_t ch)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdPluginSpotlightEnd(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdHideSpellingWidget(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetNetworkInterface(YapProxy* proxy, const char* interfaceName)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdHitTest(YapProxy* proxy, int32_t queryNum, int32_t cx, int32_t cy)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetVirtualWindowSize(YapProxy* proxy, int32_t width, int32_t height)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdPrintFrame(YapProxy* proxy, const char* frameName, int32_t lpsJobId, int32_t width, int32_t height, int32_t dpi, bool landscape, bool reverseOrder)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdTouchEvent(YapProxy* proxy, int32_t type, int32_t touchCount, int32_t modifiers, const char* touchesJson)
{
  if (auto* p = pageFor(proxy)) p->touchEvent(type, touchCount, modifiers, touchesJson);
}

void JihadBrowserServer::asyncCmdHoldAt(YapProxy* proxy, int32_t contentX, int32_t contentY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdGetTextCaretBounds(YapProxy* proxy, int32_t queryNum)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdFreeze(YapProxy* proxy)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdThaw(YapProxy* proxy, int32_t sharedBufferKey1, int32_t sharedBufferKey2, int32_t sharedBufferSize)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdReturnBuffer(YapProxy* proxy, int32_t sharedBufferKey)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetZoomAndScroll(YapProxy* proxy, double zoom, int32_t cx, int32_t cy)
{
  if (auto* p = pageFor(proxy)) p->setZoomAndScroll(zoom, cx, cy);
}

void JihadBrowserServer::asyncCmdScrollLayer(YapProxy* proxy, int32_t id, int32_t deltaX, int32_t deltaY)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdSetDNSServers(YapProxy* proxy, const char* servers)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}
