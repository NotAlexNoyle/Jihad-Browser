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
#include <time.h>

// Cadence clock. Deliberately CLOCK_MONOTONIC and deliberately not the goanna side's
// jihadNowMs(), which is gettimeofday(): a wall-clock step (NTP, the user setting the time)
// would show up as one absurd frame period and poison a min/avg/max that is being read to
// decide whether motion is even.
static int64_t jihadTickNowMs() {
  struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
  return (int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
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
void ProxySink::msgSSLConfirm2(const char* pipe, const char* host, int32_t code, const char* certFile) { mSrv->msgDialogSSLConfirm(mProxy, pipe, host, code, certFile); }
void ProxySink::msgDialogAlert(const char* p, const char* m) { mSrv->msgDialogAlert(mProxy, p, m); }
void ProxySink::msgDialogConfirm(const char* p, const char* m) { mSrv->msgDialogConfirm(mProxy, p, m); }
void ProxySink::msgDialogPrompt(const char* p, const char* m, const char* d) { mSrv->msgDialogPrompt(mProxy, p, m, d); }
void ProxySink::msgDialogUserPassword(const char* p, const char* m) { mSrv->msgDialogUserPassword(mProxy, p, m); }
void ProxySink::msgLinkClicked(const char* url) { mSrv->msgLinkClicked(mProxy, url); }
void ProxySink::msgPopupMenuShow(const char* id, const char* file) { mSrv->msgPopupMenuShow(mProxy, id, file); }
void ProxySink::msgMimeHandoffUrl(const char* mimeType, const char* url) { mSrv->msgMimeHandoffUrl(mProxy, mimeType, url); }
void ProxySink::msgDownloadStart(const char* url) { mSrv->msgDownloadStart(mProxy, url); }
void ProxySink::msgDownloadProgress(const char* url, int32_t soFar, int32_t total) { mSrv->msgDownloadProgress(mProxy, url, soFar, total); }
void ProxySink::msgDownloadFinished(const char* url, const char* mimeType, const char* tmpFilePath) { mSrv->msgDownloadFinished(mProxy, url, mimeType, tmpFilePath); }
void ProxySink::msgDownloadError(const char* url, const char* errorMsg) { mSrv->msgDownloadError(mProxy, url, errorMsg); }
void ProxySink::msgAddFlashRects(const char* j)    { mSrv->msgAddFlashRects(mProxy, j); }
void ProxySink::msgRemoveFlashRects(const char* j) { mSrv->msgRemoveFlashRects(mProxy, j); }

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
//   move X Y           -> pointer move (rollover highlight over an open menu)
//   clickid <id>       -> click an element by id at its centre (ignores zoom/scroll)
//   addon <id> on|off  -> enable/disable an installed add-on
//   cookie set H N V   -> store a persistent cookie;  cookie count -> list/count them
//   rect <id>          -> report an element's viewport rect
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
    } else if (sscanf(c.c_str(), "move %d %d", &x, &y) == 2) {
      // Pointer move without a press — how a finger rolling over an open menu is
      // delivered, and the only way to test the rollover highlight without a human.
      printf("[jihad-bs] inject move %d,%d\n", x, y);
      p->mouseEvent(2, x, y, 0);
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
    } else if (strncmp(c.c_str(), "jsurl ", 6) == 0) {
      // DEBUG: run a javascript: URL with the SYSTEM principal (executes inside privileged
      // chrome documents such as about:addons, where a plain `url javascript:` is refused).
      // Same gating as the rest of this channel: off unless $JIHAD_INJECT is set.
      std::string u2 = c.substr(6); while (!u2.empty() && (u2.back()=='\n'||u2.back()=='\r')) u2.pop_back();
      bool ok = jihad::DebugRunChromeJs(u2.c_str());
      printf("[jihad-bs] inject jsurl ok=%d (%zu chars)\n", (int)ok, u2.size());
    } else if (strncmp(c.c_str(), "title", 5) == 0) {
      // DEBUG: print the current document title (jsurl probe readback).
      printf("[jihad-bs] inject title=[%s]\n", jihad::DebugGetTitle().c_str());
    } else if (strncmp(c.c_str(), "clickid ", 8) == 0) {
      // DEBUG: click an element by id (zoom/scroll independent — see DebugClickElement).
      std::string id = c.substr(8);
      while (!id.empty() && (id.back()=='\n' || id.back()=='\r' || id.back()==' ')) id.pop_back();
      printf("[jihad-bs] inject clickid %s ok=%d\n", id.c_str(),
             (int)jihad::DebugClickElement(id.c_str()));
    } else if (strncmp(c.c_str(), "touch ", 6) == 0) {
      // DEBUG: `touch <0|1|2> x1,y1[ x2,y2 …]` — 0=start 1=move 2=end. Builds the same
      // touches JSON the adapter sends, so this exercises the REAL parse, not a shortcut.
      // Coordinates are document space, as the adapter's payload is.
      int tt = 0; char rest[1024] = {0};
      if (sscanf(c.c_str(), "touch %d %1023[^\n\r]", &tt, rest) == 2) {
        std::string json = "[";
        int x = 0, y = 0, n = 0; const char* q = rest;
        while (*q) {
          while (*q == ' ') ++q;
          if (sscanf(q, "%d,%d", &x, &y) != 2) break;
          if (n++) json += ",";
          json += "{\"x\":" + std::to_string(x) + ",\"y\":" + std::to_string(y) + ",\"state\":0}";
          while (*q && *q != ' ') ++q;
        }
        json += "]";
        printf("[jihad-bs] inject touch type=%d points=%d\n", tt, n);
        if (n > 0) p->touchEvent(tt, n, 0, json.c_str());
      }
    } else if (strncmp(c.c_str(), "spotlight ", 10) == 0) {
      // DEBUG: `spotlight <x> <y> <w> <h>` | `spotlight end` — drive the frozen
      // asyncCmdPluginSpotlightStart/End pair without the fullscreen round trip that
      // normally produces it (plugin asks -> card scrims and smart-zooms -> card confirms).
      // Same doc-space rect the adapter sends.
      int sx = 0, sy = 0, sw = 0, sh = 0;
      if (sscanf(c.c_str(), "spotlight %d %d %d %d", &sx, &sy, &sw, &sh) == 4) {
        printf("[jihad-bs] inject spotlight %d,%d %dx%d\n", sx, sy, sw, sh);
        p->pluginSpotlightStart(sx, sy, sw, sh);
      } else {
        printf("[jihad-bs] inject spotlight end\n");
        p->pluginSpotlightEnd();
      }
    } else if (strncmp(c.c_str(), "freeze", 6) == 0) {
      // DEBUG: the card-backgrounded path (device-build R5 reclaim). Real cards send this
      // over YAP; the inject form lets a test measure what backgrounding actually frees.
      printf("[jihad-bs] inject freeze\n"); p->freeze();
    } else if (strncmp(c.c_str(), "thaw", 4) == 0) {
      // thaw(0,0,0) could never un-freeze — thaw()'s guard is `ok = key1 && ...` — so this
      // re-thaws onto the buffers the page already holds, which is what a real card does.
      printf("[jihad-bs] inject thaw\n"); p->thawSelf();
    } else if (strncmp(c.c_str(), "gettext ", 8) == 0) {
      // DEBUG: read an element's text — the only way to assert on what a GENERATED chrome
      // page says (about:plugins, about:addons), where a rect proves existence but not content.
      std::string sel = c.substr(8);
      while (!sel.empty() && (sel.back()=='\n' || sel.back()=='\r' || sel.back()==' ')) sel.pop_back();
      printf("[jihad-bs] inject gettext %s = [%s]\n", sel.c_str(),
             jihad::DebugElementText(sel.c_str(), 400).c_str());
    } else if (strncmp(c.c_str(), "setpref ", 8) == 0) {
      // DEBUG: `setpref <b|i|s> <name> <value>` — write a pref, then read it straight back so
      // one line proves both. Added 2026-08-10 after a device run was wasted discovering there
      // was no way to write a pref from a test: `jsurl` runs with the system PRINCIPAL but in a
      // CONTENT scope, where `Components` is not exposed, so the probe silently did nothing and
      // the run read as a negative result about the shutdown flush rather than a broken probe.
      std::string a = c.substr(8);
      while (!a.empty() && (a.back()=='\n' || a.back()=='\r')) a.pop_back();
      char t = a.empty() ? '\0' : a[0];
      size_t s1 = a.find(' '), s2 = (s1 == std::string::npos) ? s1 : a.find(' ', s1 + 1);
      if (s1 == std::string::npos || s2 == std::string::npos) {
        printf("[jihad-bs] inject setpref: usage `setpref <b|i|s> <name> <value>`\n");
      } else {
        std::string nm = a.substr(s1 + 1, s2 - s1 - 1), val = a.substr(s2 + 1);
        bool ok = jihad::DebugSetPref(nm.c_str(), t, val.c_str());
        printf("[jihad-bs] inject setpref %c %s = %s -> ok=%d readback=[%s]\n",
               t, nm.c_str(), val.c_str(), (int)ok, jihad::DebugGetPref(nm.c_str(), t).c_str());
      }
    } else if (strncmp(c.c_str(), "getpref ", 8) == 0) {
      // DEBUG: `getpref <b|i|s> <name>` — read a pref after a restart, which is the half that
      // proves persistence rather than just that the setter ran.
      std::string a = c.substr(8);
      while (!a.empty() && (a.back()=='\n' || a.back()=='\r')) a.pop_back();
      char t = a.empty() ? '\0' : a[0];
      size_t s1 = a.find(' ');
      std::string nm = (s1 == std::string::npos) ? std::string() : a.substr(s1 + 1);
      printf("[jihad-bs] inject getpref %c %s = [%s]\n",
             t, nm.c_str(), jihad::DebugGetPref(nm.c_str(), t).c_str());
    } else if (strncmp(c.c_str(), "anon ", 5) == 0) {
      // DEBUG: `anon <selector>` — the element's native anonymous content, i.e. whether an XBL
      // binding actually ATTACHED. The only honest test for it: the `controls` attribute
      // reflects either way, and `document.getAnonymousNodes` is not exposed to page script here
      // even under the system principal (device-measured 2026-08-10 — it throws "not a
      // function"). "?" means no such element; "0" means the element exists and bound NOTHING,
      // which is the real negative.
      std::string sel = c.substr(5);
      while (!sel.empty() && (sel.back()=='\n' || sel.back()=='\r' || sel.back()==' ')) sel.pop_back();
      printf("[jihad-bs] inject anon %s = [%s]\n", sel.c_str(),
             jihad::DebugAnonNodes(sel.c_str()).c_str());
    } else if (strncmp(c.c_str(), "find ", 5) == 0) {
      // DEBUG: drive findInPage without a card's find bar. The frozen command is already
      // dispatched from the UI; this exercises the same daemon path from the inject channel.
      std::string q = c.substr(5);
      while (!q.empty() && (q.back()=='\n' || q.back()=='\r')) q.pop_back();
      printf("[jihad-bs] inject find \"%s\" -> %d\n", q.c_str(), (int)p->findString(q.c_str(), true));
    } else if (strncmp(c.c_str(), "popups", 6) == 0) {
      // DEBUG: how many XUL popups are open. Readback for tests that drive a menu — and the
      // only cheap way to observe a XUL-only KEY reaction (Escape rolls a menu up), since a
      // menu changes no DOM a selector can see.
      printf("[jihad-bs] inject popups=%d\n", (int)p->popupsOpen());
    } else if (strncmp(c.c_str(), "clickoff ", 9) == 0) {
      // DEBUG: `clickoff <dx> <dy> <count> <selector>` — click at an OFFSET from the
      // element's top-left. A XUL tree's rows are not DOM nodes, so no selector reaches
      // one; a centre click on a filtered (nearly empty) tree lands below every row.
      int dx = 0, dy = 0, cnt = 1; char sel[512] = {0};
      if (sscanf(c.c_str(), "clickoff %d %d %d %511[^\n\r]", &dx, &dy, &cnt, sel) == 4) {
        printf("[jihad-bs] inject clickoff %s +%d,%d n=%d ok=%d\n", sel, dx, dy, cnt,
               (int)jihad::DebugClickElementAt(sel, dx, dy, cnt));
      } else {
        printf("[jihad-bs] inject clickoff: need <dx> <dy> <count> <selector>\n");
      }
    } else if (strncmp(c.c_str(), "dblclickid ", 11) == 0) {
      // DEBUG: same lookup as `clickid`, delivered as a DOUBLE click — the only way to
      // change a value in a XUL tree (about:config toggles a boolean on dblclick).
      std::string id = c.substr(11);
      while (!id.empty() && (id.back()=='\n' || id.back()=='\r' || id.back()==' ')) id.pop_back();
      printf("[jihad-bs] inject dblclickid %s ok=%d\n", id.c_str(),
             (int)jihad::DebugClickElement(id.c_str(), 2));
    } else if (strncmp(c.c_str(), "addon ", 6) == 0) {
      // DEBUG: `addon <id> on|off` — flip an installed add-on's enabled state.
      std::string rest = c.substr(6);
      while (!rest.empty() && (rest.back()=='\n' || rest.back()=='\r' || rest.back()==' ')) rest.pop_back();
      size_t sp = rest.rfind(' ');
      std::string id = (sp == std::string::npos) ? rest : rest.substr(0, sp);
      std::string act = (sp == std::string::npos) ? "off" : rest.substr(sp + 1);
      printf("[jihad-bs] inject addon %s %s ok=%d\n", id.c_str(), act.c_str(),
             (int)jihad::DebugSetAddonEnabled(id.c_str(), act == "on"));
    } else if (strncmp(c.c_str(), "cookie ", 7) == 0) {
      // DEBUG: `cookie set <host> <name> <value>` | `cookie count`.
      std::string rest = c.substr(7);
      while (!rest.empty() && (rest.back()=='\n' || rest.back()=='\r')) rest.pop_back();
      char h[128] = {0}, n2[64] = {0}, v[128] = {0};
      if (sscanf(rest.c_str(), "set %127s %63s %127s", h, n2, v) == 3) {
        printf("[jihad-bs] inject cookie set ok=%d\n", (int)jihad::DebugCookieSet(h, n2, v));
      } else {
        printf("[jihad-bs] inject cookie count=%d\n", jihad::DebugCookieCount());
      }
    } else if (strncmp(c.c_str(), "rect ", 5) == 0) {
      // DEBUG: report an element's viewport rect, so a test can click a real control
      // instead of guessing coordinates (chrome XUL has no other way in from here — a
      // javascript: URL does not run in a chrome document; `jsurl` prints ok=1 and does
      // nothing there). Three forms, see DebugElementRect:
      //   rect <id>                     rect sel:<css>                rect anon:<css>|<anonid>
      std::string id = c.substr(5);
      while (!id.empty() && (id.back()=='\n' || id.back()=='\r' || id.back()==' ')) id.pop_back();
      printf("[jihad-bs] inject rect %s\n", jihad::DebugElementRect(id.c_str()).c_str());
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

  // 4 ms of engine pump per tick, not 10.
  //
  // The tick is a 16 ms timer, but a 10 ms pump plus a paint made each tick take ~33 ms, so
  // the timer never actually got to run at its own rate. That matters for MOTION, not
  // throughput: a frame can only be delivered on a tick, so a 33 ms tick quantises every
  // frame gap to 33/66/99 ms and animation arrives visibly uneven no matter how fast the
  // source is. A smaller budget keeps the tick near its 16 ms period, which halves that
  // quantisation. Total pump time per second is unchanged or better — the pump simply runs
  // more often for less time each, which is also what a plugin's own timer sources want.
  //
  // TICK CADENCE, measured at the source rather than inferred. A frame can only be delivered
  // on a tick, so the tick PERIOD is a hard floor on how evenly motion can be delivered, and
  // no amount of cheap painting can get under it. Separating PERIOD from WORK is the whole
  // point: if period >> work the loop is busy with something other than us and the fix is not
  // to make the paint cheaper. Device-measured 2026-08-10 on a 30 fps plugin: the timer asks
  // for 16 ms and the observed period was 28.6 ms while the paint itself cost only 3-7 ms.
  int64_t tickT0 = jihadTickNowMs();
  int64_t pumpMs = 0;
  for (auto* pg : snap) {
    int64_t a = jihadTickNowMs();
    pg->pump(4, /*drainOnly*/true);
    pumpMs += jihadTickNowMs() - a;
    pg->maybePaint();
    // Pull the next plugin frame. HERE and not inside maybePaint, because maybePaint is also
    // called from returnBuffer() outside the tick, and the pull paces itself by counting calls
    // — an off-tick call would shift the request off the tick grid it exists to stay on.
    pg->requestNextPluginFrame();
  }
  int64_t tickT1 = jihadTickNowMs();
  {
    static int64_t sPrevStart = 0, sReport = 0;
    static uint32_t sPerMin = 0, sPerMax = 0, sWorkMax = 0, sN = 0;
    static uint64_t sPerSum = 0, sWorkSum = 0, sPumpSum = 0;
    if (sPrevStart) {
      uint32_t per = (uint32_t)(tickT0 - sPrevStart);
      uint32_t work = (uint32_t)(tickT1 - tickT0);
      if (!sN || per < sPerMin) sPerMin = per;
      if (per > sPerMax) sPerMax = per;
      if (work > sWorkMax) sWorkMax = work;
      sPerSum += per; sWorkSum += work; sPumpSum += (uint64_t)pumpMs; sN++;
    }
    sPrevStart = tickT0;
    if (!sReport) sReport = tickT1;
    else if (tickT1 - sReport >= 2000) {
      if (sN) {
        // pump vs paint, split: the tick is WORK-bound, not timer-bound, whenever work
        // approaches the 16 ms period, and then the frame grid can only land on multiples of
        // the work time. Knowing which half to attack is the difference between shrinking the
        // pump budget and making the paint cheaper.
        fprintf(stderr, "[jihad-bs]   tick period ms min=%u avg=%u max=%u | work avg=%u max=%u "
                        "(pump avg=%u paint avg=%u) | n=%u\n",
                sPerMin, (uint32_t)(sPerSum / sN), sPerMax,
                (uint32_t)(sWorkSum / sN), sWorkMax,
                (uint32_t)(sPumpSum / sN), (uint32_t)((sWorkSum - sPumpSum) / sN), sN);
        fflush(stderr);
      }
      sPerMin = sPerMax = sWorkMax = sN = 0; sPerSum = sWorkSum = sPumpSum = 0;
      sReport = tickT1;
    }
  }

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
void JihadBrowserServer::asyncCmdSetExtraBuffer(YapProxy* proxy, int32_t sharedBufferKey3, int32_t sharedBufferSize)
{
  printf("[jihad-bs] extra buffer key=0x%x sz=%d\n", (unsigned)sharedBufferKey3, sharedBufferSize);
  fflush(stdout);
  if (auto* p = pageFor(proxy)) p->setExtraBuffer(sharedBufferKey3, sharedBufferSize);
}

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
  if (auto* p = pageFor(proxy)) p->popupMenuSelect(identifier, selectedIdx);
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
  // READ BEFORE IMPLEMENTING THIS. The stock Mojo WebView widget's hold handler
  // (widget_webview.js _handleHoldPluginSpotlight -> _pluginSpotlightCreate("partial"))
  // turns a reply naming an <object>/<embed>/<applet> into adapter.setSpotlight(), i.e.
  // YAP 0x1501. The Mojo variant uses that widget unmodified, so once this stub answers, a
  // long-press on Flash becomes a plugin-spotlight event — which kills the Flash instance.
  // This stub returning nothing is the ONLY thing breaking that chain today. It is safe to
  // implement only while GoannaRenderPage::SetPluginSpotlight suppresses delivery.
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

// The confirmation leg of the fullscreen-plugin round trip: the plugin asks for fullscreen, the
// card scrims and smart-zooms to it, and then tells the server the rect it actually settled on.
// The rect arrives in the adapter's DOCUMENT space, as origin + WIDTH/HEIGHT (unlike the
// interactive-rect JSON, which uses absolute edges).
//
// Queued rather than dispatched here, like every other input command: this callback runs on the
// YAP socket with no page-lifetime guard, and it ends in a synchronous call into the plugin.
void JihadBrowserServer::asyncCmdPluginSpotlightStart(YapProxy* proxy, int32_t cx, int32_t cy, int32_t cw, int32_t ch)
{
  if (auto* p = pageFor(proxy)) p->pluginSpotlightStart(cx, cy, cw, ch);
}

void JihadBrowserServer::asyncCmdPluginSpotlightEnd(YapProxy* proxy)
{
  if (auto* p = pageFor(proxy)) p->pluginSpotlightEnd();
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
  // The long-press is GATED on this round-trip: the adapter queues the mousehold gesture
  // (mouseHoldTimeoutCb -> asyncCmdHitTest) and only sends asyncCmdHoldAt after
  // msgHitTestResponse arrives and the card's eventFired handler declines the hit. As a
  // stub this silently killed EVERY long-press — no holdAt was ever sent (device
  // 2026-08-02: "holding on the yellow doesn't work"; the daemon log showed nothing
  // because there was nothing to log). Always reply, even with no page: a lost reply
  // strands the adapter's gesture queue.
  std::string json;
  if (auto* p = pageFor(proxy)) p->hitTest(cx, cy, &json);
  if (json.empty()) json = "{\"isNull\":true}";
  fprintf(stderr, "[jihad-bs] hitTest q=%d at %d,%d -> %s\n", queryNum, cx, cy, json.c_str());
  msgHitTestResponse(proxy, queryNum, json.c_str());
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
