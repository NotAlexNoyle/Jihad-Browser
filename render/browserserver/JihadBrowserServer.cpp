/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
/* Jihad Browser — JihadBrowserServer implementation. See header. */
#include "JihadBrowserServer.h"
#include "../goanna/JihadRuntimePaths.h"   // the ONE runtime-state dir (T-057 / R8)
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
// EngineHost is only forwarded by reference (forward-declared), so this
// dispatch layer stays free of XPCOM/engine headers.

// ---- ProxySink: route one page's msg* to its client proxy ------------------
void ProxySink::msgPainted(int32_t key)      { mSrv->msgPainted(mProxy, key); }
void ProxySink::msgLoadStarted()             { mSrv->msgLoadStarted(mProxy); }
void ProxySink::msgLoadProgress(int32_t p)   { mSrv->msgLoadProgress(mProxy, p); }
void ProxySink::msgLoadStopped()             { mSrv->msgLoadStopped(mProxy); }
void ProxySink::msgLocationChanged(const char* uri, bool b, bool f) { mSrv->msgLocationChanged(mProxy, uri, b, f); }
void ProxySink::msgTitleChanged(const char* t) { mSrv->msgTitleChanged(mProxy, t); }
void ProxySink::msgTitleAndUrlChanged(const char* title, const char* uri, bool b, bool f) { mSrv->msgTitleAndUrlChanged(mProxy, title, uri, b, f); }
void ProxySink::msgEditorFocused(bool focused, int fieldType, int fieldActions) { mSrv->msgEditorFocused(mProxy, focused, fieldType, fieldActions); }
void ProxySink::msgContentsSizeChanged(int32_t w, int32_t h) { mSrv->msgContentsSizeChanged(mProxy, w, h); }
void ProxySink::msgScrolledTo(int32_t x, int32_t y) { mSrv->msgScrolledTo(mProxy, x, y); }
void ProxySink::msgMetaViewportSet(double is, double mn, double mx, int32_t w, int32_t h, bool us) { mSrv->msgMetaViewportSet(mProxy, is, mn, mx, w, h, us); }
void ProxySink::msgFailedLoad(const char* d, int32_t c, const char* u, const char* desc) { mSrv->msgFailedLoad(mProxy, d, c, u, desc); }
void ProxySink::msgUpdateGlobalHistory(const char* u, bool reload) { mSrv->msgUpdateGlobalHistory(mProxy, u, reload); }
void ProxySink::msgUrlRedirected(const char* u, const char* ud) { mSrv->msgUrlRedirected(mProxy, u, ud); }
// syncPipePath empty: the blocking accept/reject reply pipe is adapter/device work.
void ProxySink::msgSSLConfirm(const char* host, int32_t code, const char* certFile) { mSrv->msgDialogSSLConfirm(mProxy, "", host, code, certFile); }
void ProxySink::msgLinkClicked(const char* url) { mSrv->msgLinkClicked(mProxy, url); }
void ProxySink::msgMimeHandoffUrl(const char* mimeType, const char* url) { mSrv->msgMimeHandoffUrl(mProxy, mimeType, url); }
void ProxySink::msgDownloadStart(const char* url) { mSrv->msgDownloadStart(mProxy, url); }
void ProxySink::msgDownloadProgress(const char* url, int32_t soFar, int32_t total) { mSrv->msgDownloadProgress(mProxy, url, soFar, total); }
void ProxySink::msgDownloadFinished(const char* url, const char* mimeType, const char* tmpFilePath) { mSrv->msgDownloadFinished(mProxy, url, mimeType, tmpFilePath); }
void ProxySink::msgDownloadError(const char* url, const char* errorMsg) { mSrv->msgDownloadError(mProxy, url, errorMsg); }

// ---- server ----------------------------------------------------------------
JihadBrowserServer::JihadBrowserServer(const char* name, jihad::EngineHost& host)
  : BrowserServerBase(name), mHost(host), mInTick(false) {
  jihad::SetDownloadSink(this);   // receive engine download/handoff callbacks
#ifndef JIHAD_NO_INJECT
  // Resolve the self-drive channel ONCE (see the block above processInjectFile
  // for what it is and how to turn it on). Disabled resolves to "", which makes
  // the tick-loop poll a single bool test — no getenv, no stat, no open per tick.
  mInjectPath = jihad::RuntimeResolvePath(getenv("JIHAD_INJECT"), "inject.cmd");
  if (!mInjectPath.empty())
    printf("[jihad-bs] self-drive inject channel ENABLED: %s\n", mInjectPath.c_str());
#endif
}

JihadBrowserServer::~JihadBrowserServer() {
  jihad::SetDownloadSink(nullptr);   // clear before we (the sink) are destroyed
  for (auto& kv : mPages) { delete kv.second.page; delete kv.second.sink; }
  for (auto& e : mReap)   { delete e.page; delete e.sink; }
}

// ── routing a process-wide download back to ONE card (F-1) ──────────────────
// The engine's download machinery has no page handle by the time it calls us, so
// these used to be routed to mLastProxy — the card that connected LAST. That is
// the wrong client as soon as two cards are open (card A's download progress
// lands on card B), and once that card closed reap() nulled mLastProxy without
// re-electing, so from then on EVERY download message was silently dropped —
// including the single terminal message the client's download list waits on,
// leaving a permanently-stuck entry even though other cards were still live.
//
// jihad::DownloadOrigin fixes the attribution at the source: the download service
// captures the originating page's docShell identity while the engine still knows
// it, and hands the same token back on every callback. Here we turn it into a
// page. The fallbacks matter as much as the match: an unknown origin must land
// somewhere live, never nowhere, because dropping a terminal message strands the
// client's UI. Order: the originating card -> the active card -> the newest card
// still connected.
jihad::BrowserPageGoanna* JihadBrowserServer::pageForDownload(jihad::DownloadOrigin origin) {
  if (origin) {
    for (auto& kv : mPages)
      if (kv.second.page && kv.second.page->docShellKey() == origin) return kv.second.page;
  }
  if (auto* p = pageFor(mLastProxy)) return p;
  for (auto it = mConnectOrder.rbegin(); it != mConnectOrder.rend(); ++it)
    if (auto* p = pageFor(*it)) return p;
  return nullptr;
}

// A download/handoff fired in the engine (helperapplauncherdialog). Runs on the
// embedding thread. The app then drives com.palm.downloadmanager.
void JihadBrowserServer::OnDownload(jihad::DownloadOrigin origin, const char* url,
                                    const char* mimeType,
                                    const char* suggestedName, int64_t contentLength) {
  (void)suggestedName; (void)contentLength;
  auto* p = pageForDownload(origin);
  // Log whether the ORIGIN matched, not just that something was routed: on the
  // device the log is the only way to tell a correct attribution from a fallback.
  printf("[jihad-bs] download handoff mime=%s url=%s origin=%p (%s)\n",
         mimeType ? mimeType : "", url ? url : "", origin,
         (origin && p && p->docShellKey() == origin) ? "originating card" : "fallback");
  if (p)
    p->emitMimeHandoff(mimeType ? mimeType : "", url ? url : "");
  else
    printf("[jihad-bs] download: no connected page to hand off to\n");
}

// The frozen msgDownloadProgress carries int32 byte counts; the engine counts in
// int64. Saturate rather than wrap so a >2 GB transfer still reports monotonic
// progress (and keep -1 = "unknown total" intact).
static int32_t clampToI32(int64_t v) {
  if (v < 0) return -1;
  return v > 0x7fffffffLL ? 0x7fffffff : (int32_t)v;
}

// Lifecycle of the download the ENGINE performs (DownloadService drives the
// helper-app save). Every one of these carries the same origin token for a given
// download, so start/progress/terminal all land on the same client (F-1).
void JihadBrowserServer::OnDownloadStart(jihad::DownloadOrigin origin, const char* url) {
  if (auto* p = pageForDownload(origin)) p->emitDownloadStart(url ? url : "");
}
void JihadBrowserServer::OnDownloadProgress(jihad::DownloadOrigin origin, const char* url,
                                            int64_t bytesSoFar, int64_t totalBytes) {
  if (auto* p = pageForDownload(origin))
    p->emitDownloadProgress(url ? url : "", clampToI32(bytesSoFar), clampToI32(totalBytes));
}
void JihadBrowserServer::OnDownloadFinished(jihad::DownloadOrigin origin, const char* url,
                                            const char* mimeType, const char* tmpFilePath) {
  printf("[jihad-bs] download finished mime=%s path=%s\n",
         mimeType ? mimeType : "", tmpFilePath ? tmpFilePath : "");
  if (auto* p = pageForDownload(origin))
    p->emitDownloadFinished(url ? url : "", mimeType ? mimeType : "", tmpFilePath ? tmpFilePath : "");
  else
    printf("[jihad-bs] download finished with no connected page — terminal message dropped\n");
}
void JihadBrowserServer::OnDownloadError(jihad::DownloadOrigin origin, const char* url,
                                         const char* errorMsg) {
  printf("[jihad-bs] download error %s: %s\n", url ? url : "", errorMsg ? errorMsg : "");
  if (auto* p = pageForDownload(origin))
    p->emitDownloadError(url ? url : "", errorMsg ? errorMsg : "");
  else
    printf("[jihad-bs] download error with no connected page — terminal message dropped\n");
}

jihad::BrowserPageGoanna* JihadBrowserServer::pageFor(YapProxy* proxy) {
  auto it = mPages.find(proxy);
  return it == mPages.end() ? nullptr : it->second.page;
}

void JihadBrowserServer::reap(YapProxy* proxy) {
  // Drop it from the connect order even if it has no page (a client that
  // connected and disconnected before asyncCmdConnect succeeded), so a dead
  // proxy can never be re-elected below.
  mConnectOrder.erase(std::remove(mConnectOrder.begin(), mConnectOrder.end(), proxy),
                      mConnectOrder.end());
  auto it = mPages.find(proxy);
  if (it == mPages.end()) return;
  Page pg = it->second;
  mPages.erase(it);
  // F-1: RE-ELECT rather than null. mLastProxy is both the self-drive inject
  // target and the download fallback; clearing it when the newest card closed
  // meant every later download message — terminal included — was discarded even
  // with other cards still connected. The newest surviving card is the closest
  // thing to "the card the user is on".
  if (proxy == mLastProxy)
    mLastProxy = mConnectOrder.empty() ? nullptr : mConnectOrder.back();
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

#ifndef JIHAD_NO_INJECT
// ── DEBUG self-drive channel — OFF BY DEFAULT ───────────────────────────────
// Device testing without a human on the glass: a text file of one command per
// line, consumed atomically (read then unlink) so each batch applies once.
// Content coordinates, same as the adapter sends.
//   click X Y [N]      -> clickAt
//   hold X Y           -> holdAt (long-press)
//   key CODE [CHR]     -> keyDown+keyUp (CHR defaults to CODE)
//   text STRING...     -> insertStringAtCursor (rest of line)
//   url URL            -> openUrl
//   back|forward|reload|stop
//   scroll X Y         -> setScrollPosition
//   drag X Y DX DY     -> dragStart/dragProcess/dragEnd (flick scroll path)
//   size W H           -> setWindowSize (VKB-resize / rotation simulation)
//   zoom Z X Y         -> setZoomAndScroll
//
// HOW TO TURN IT ON (T-057): set $JIHAD_INJECT in the daemon's environment —
// in the variant's upstart job, or on an ad-hoc novacom run:
//     env JIHAD_INJECT=1 ./jihad-browserserver <greDir>
// "1"/"on"/"yes"/"true" -> <state>/inject.cmd, i.e. /var/palm/jihad/$V/inject.cmd
// on the device; a bare filename or an absolute path picks another file. Then:
//     novacom run file://bin/sh -- -c 'echo url http://example.com > /var/palm/jihad/enyo/inject.cmd'
// Unset (the default) disables it entirely: no polling, no syscalls, nothing to
// write to. `-DJIHAD_NO_INJECT` compiles the whole facility out for a release
// build.
//
// WHY IT IS GATED AT ALL. This is a remote control: anything that can create the
// file drives the browser (navigate, type, click). It used to live on
// /media/internal — the user's vfat volume, where every file is effectively
// world-writable and any app or a PC in USB-drive mode could plant it. Moving it
// to the root-owned 0755 /var/palm/jihad/$V/ (R8) means only root can write it,
// which is most of the risk gone; keeping it OFF by default means an installed
// package ships with no live control channel at all, which is the rest.
//
// Defence in depth on top of the gate: the file is opened once and validated
// through that same fd (no path re-lookup between check and read — no TOCTOU),
// and is refused unless it is a regular file owned by our own euid and not
// group/world-writable. A rejected file is still unlinked so a hostile one
// cannot wedge the channel by sitting there.

void JihadBrowserServer::processInjectFile() {
  const char* path = mInjectPath.c_str();
  // O_NOFOLLOW: a symlink at the path is not the file we agreed on. O_NONBLOCK: a
  // FIFO left there would otherwise block open() until a writer showed up — i.e.
  // hang the whole daemon tick loop. Neither affects a plain regular file.
  int fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
  if (fd < 0) {
    if (errno != ENOENT) {                 // ENOENT is the normal "no commands" case
      printf("[jihad-bs] inject: %s unusable (%s) — clearing\n", path, strerror(errno));
      unlink(path);                        // e.g. ELOOP: don't let it wedge the channel
    }
    return;
  }
  struct stat st;
  if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) ||
      st.st_uid != geteuid() || (st.st_mode & (S_IWGRP | S_IWOTH))) {
    printf("[jihad-bs] inject: refusing %s (not a private regular file of uid %lu)\n",
           path, (unsigned long)geteuid());
    close(fd);
    unlink(path);
    return;
  }
  FILE* f = fdopen(fd, "r");
  if (!f) { close(fd); return; }
  char line[2048];
  std::vector<std::string> cmds;
  while (fgets(line, sizeof line, f)) cmds.push_back(line);
  fclose(f);
  unlink(path);
  jihad::BrowserPageGoanna* p = pageFor(mLastProxy);
  if (!p) { printf("[jihad-bs] inject: no page\n"); return; }
  for (auto& c : cmds) {
    int x=0, y=0, n=1, a=0, b=0;
    char buf[1600] = {0};
    if (sscanf(c.c_str(), "click %d %d %d", &x, &y, &n) >= 2) {
      printf("[jihad-bs] inject click %d,%d n=%d\n", x, y, n); p->clickAt(x, y, n);
    } else if (sscanf(c.c_str(), "hold %d %d", &x, &y) == 2) {
      printf("[jihad-bs] inject hold %d,%d\n", x, y); p->holdAt(x, y);
    } else if (sscanf(c.c_str(), "key %d %d", &x, &y) >= 1) {
      int chr = (sscanf(c.c_str(), "key %d %d", &x, &y) == 2) ? y : x;
      printf("[jihad-bs] inject key %d chr=%d\n", x, chr);
      p->keyDown(x, 0, chr); p->keyUp(x, 0, chr);
    } else if (strncmp(c.c_str(), "text ", 5) == 0) {
      std::string t = c.substr(5); while (!t.empty() && (t.back()=='\n'||t.back()=='\r')) t.pop_back();
      printf("[jihad-bs] inject text (%zu chars)\n", t.size()); p->insertStringAtCursor(t.c_str());
    } else if (strncmp(c.c_str(), "url ", 4) == 0) {
      // rest-of-line (not %s) so data: URLs and query strings with spaces survive
      std::string u2 = c.substr(4); while (!u2.empty() && (u2.back()=='\n'||u2.back()=='\r')) u2.pop_back();
      printf("[jihad-bs] inject url %s\n", u2.c_str()); p->openUrl(u2.c_str());
    } else if (strncmp(c.c_str(), "back", 4) == 0)    { p->pageBackward(); }
    else if (strncmp(c.c_str(), "forward", 7) == 0)   { p->pageForward(); }
    else if (strncmp(c.c_str(), "reload", 6) == 0)    { p->pageReload(); }
    else if (strncmp(c.c_str(), "stop", 4) == 0)      { p->pageStop(); }
    else if (sscanf(c.c_str(), "scroll %d %d", &x, &y) == 2) { p->setScrollPosition(x, y); }
    else if (sscanf(c.c_str(), "drag %d %d %d %d", &x, &y, &a, &b) == 4) {
      p->dragStart(x, y); p->dragProcess(a, b); p->dragEnd(x + a, y + b);
    } else if (sscanf(c.c_str(), "size %d %d", &x, &y) == 2) {
      printf("[jihad-bs] inject size %dx%d\n", x, y); p->setWindowSize(x, y);
    } else if (c.find_first_not_of(" \t\r\n") != std::string::npos) {
      double z = 1.0;
      if (sscanf(c.c_str(), "zoom %lf %d %d", &z, &x, &y) == 3) p->setZoomAndScroll(z, x, y);
      else printf("[jihad-bs] inject: bad cmd: %s", c.c_str());
    }
  }
}
#endif  // !JIHAD_NO_INJECT

void JihadBrowserServer::tick() {
  // Re-entrancy guard (device crash: BeginLoad this=0xa, SIGBUS). A synchronous
  // navigation started from INSIDE pump() — Enter form-submit (FireFormSubmit),
  // a tapped link/button (SendMouseEvent), or a content-nav re-drive (openUrl) —
  // dispatches DOM events that run page JS and spins the engine's nested event
  // loop. That nested loop can fire our g_timeout again, re-entering tick() while
  // the outer pump is mid-navigation; the nested pump then re-drives the same
  // page and corrupts the outer frame's mPage (observed as mPage==0xa). Bail on
  // any re-entry — the outer tick still owns this cycle. (mInTick also defers
  // reap, so a connect/disconnect during nested pumping stays safe.)
  if (mInTick) return;
  mInTick = true;

  // Snapshot page pointers so a connect/disconnect during nested GLib pumping
  // can't invalidate our iteration or delete a page under us (Codex P0).
  std::vector<jihad::BrowserPageGoanna*> snap;
  snap.reserve(mPages.size());
  for (auto& kv : mPages) snap.push_back(kv.second.page);

  // Self-drive: poll the inject file ~5x/s (tick is ~10 ms). Commands only queue
  // work (clickAt defers to pump, keys queue edits), mirroring the adapter path.
  // Disabled is the default (T-057): mInjectPath is empty, so this whole thing
  // is one predictable branch per tick — the counter never even increments.
#ifndef JIHAD_NO_INJECT
  if (!mInjectPath.empty() && ++mInjectThrottle >= 20) {
    mInjectThrottle = 0; processInjectFile();
  }
#endif

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
  mConnectOrder.push_back(proxy);
  mLastProxy = proxy;   // newest connect = active card = self-drive inject target
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

// YAP cancelDownload(url): abort the matching in-flight engine download. The
// download is process-wide (the engine's helper-app service owns it, not a
// page), so this is not routed per-proxy. An empty url cancels all of them,
// matching the adapter's "stop whatever is downloading" intent; a specific url
// cancels exactly one (F-4).
//
// Nothing goes back on the wire for a cancel that matched nothing, and that is
// deliberate: cancelDownload (0x1015) is a fire-and-forget async command with no
// reply in the FROZEN contract, and synthesising a msgDownloadError for a
// download that already sent its terminal message would corrupt the client's
// download list. So the three outcomes are separated in the log instead — the
// only diagnostic that exists on the device.
void JihadBrowserServer::asyncCmdCancelDownload(YapProxy* proxy, const char* url)
{
  (void)proxy;
  const char* what = "no such download";
  switch (jihad::CancelDownload(url)) {
    case jihad::CancelOutcome::Aborted:           what = "aborted"; break;
    case jihad::CancelOutcome::AlreadyTerminated: what = "already finished/failed"; break;
    case jihad::CancelOutcome::Unknown:           break;
  }
  printf("[jihad-bs] cancelDownload %s -> %s\n", url ? url : "(all)", what);
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
  if (auto* p = pageFor(proxy)) p->dragStart(contentX, contentY);
}

void JihadBrowserServer::asyncCmdDragProcess(YapProxy* proxy, int32_t deltaX, int32_t deltaY)
{
  if (auto* p = pageFor(proxy)) p->dragProcess(deltaX, deltaY);
}

void JihadBrowserServer::asyncCmdDragEnd(YapProxy* proxy, int32_t contentX, int32_t contentY)
{
  if (auto* p = pageFor(proxy)) p->dragEnd(contentX, contentY);
}

void JihadBrowserServer::asyncCmdSetMinFontSize(YapProxy* proxy, int32_t minFontSizePt)
{
  (void)proxy; jihad::SetMinFontSize(minFontSizePt);
}

void JihadBrowserServer::asyncCmdFindString(YapProxy* proxy, const char* str, bool fwd)
{
  if (auto* p = pageFor(proxy)) p->findString(str, fwd);
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
  if (auto* p = pageFor(proxy)) p->addUrlRedirect(urlRe, type, redirect, userData);
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
  if (auto* p = pageFor(proxy)) p->insertStringAtCursor(text);
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
  if (auto* p = pageFor(proxy)) p->holdAt(contentX, contentY);
}

void JihadBrowserServer::asyncCmdGetTextCaretBounds(YapProxy* proxy, int32_t queryNum)
{
  (void)proxy; // TODO(T-016): route to pageFor(proxy) / GoannaRenderPage per PORT-MAP.md
}

void JihadBrowserServer::asyncCmdFreeze(YapProxy* proxy)
{
  if (auto* p = pageFor(proxy)) p->freeze();
}

void JihadBrowserServer::asyncCmdThaw(YapProxy* proxy, int32_t sharedBufferKey1, int32_t sharedBufferKey2, int32_t sharedBufferSize)
{
  if (auto* p = pageFor(proxy)) p->thaw(sharedBufferKey1, sharedBufferKey2, sharedBufferSize);
}

void JihadBrowserServer::asyncCmdReturnBuffer(YapProxy* proxy, int32_t sharedBufferKey)
{
  if (auto* p = pageFor(proxy)) p->returnBuffer(sharedBufferKey);
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
