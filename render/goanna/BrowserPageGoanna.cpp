/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser &mdash; BrowserPageGoanna implementation. See BrowserPageGoanna.h.
 * Bridges the YAP BrowserPage command surface to GoannaRenderPage and emits the
 * server->adapter messages via IPageMessageSink.
 */
#include "BrowserPageGoanna.h"
#include "EngineHost.h"
#include "GoannaRenderPage.h"
#include "BrowserOffscreenInfo.h"   // isis shmem header (from render/browserserver/Src)
#include "JihadLogo.h"              // JIHAD_LOGO_B64 (app icon for about: pages)

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>
#include <regex.h>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace { }  // (UrlRule defined below in the jihad namespace)

namespace jihad {

// A compiled URL redirect rule (POSIX regex &mdash; exception-free, unlike std::regex,
// which matters under -fno-exceptions).
struct BrowserPageGoanna::UrlRule {
  regex_t re;
  std::string userData;
  bool redirect;
};

BrowserPageGoanna::BrowserPageGoanna(EngineHost& host, IPageMessageSink& sink)
  : mHost(host), mSink(sink), mPage(nullptr),
    mKey1(0), mKey2(0), mBufSize(0), mActiveKey(0),
    mLoadWasDone(false), mNeedsPaint(false),
    mLastContentW(-1), mLastContentH(-1),
    mLastScrollX(-1), mLastScrollY(-1), mZoom(1.0),
    mAdapterScrollX(0), mAdapterScrollY(0), mFrozen(false) {}

void BrowserPageGoanna::mapToContent(int sx, int sy, int* cx, int* cy) {
  // mZoom is clamped to a sane range on set (see setZoomAndScroll), so sx/z can't
  // blow up; still clamp the result to a safe int range (Codex P0).
  double z = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
  int scrollX = 0, scrollY = 0;
  if (mPage) mPage->GetScrollXY(&scrollX, &scrollY);   // content-space scroll
  auto clamp = [](double v) -> int {
    if (v < -1000000.0) return -1000000;
    if (v >  1000000.0) return  1000000;
    return (int)v;
  };
  if (cx) *cx = clamp((double)sx / z + scrollX);
  if (cy) *cy = clamp((double)sy / z + scrollY);
}

BrowserPageGoanna::~BrowserPageGoanna() {
  // The BrowserAdapter owns the shared segments (it allocated them and passed
  // the keys via connect()); the daemon must NOT IPC_RMID them (Codex P1).
  delete mPage;
  for (UrlRule* r : mRedirectRules) { regfree(&r->re); delete r; }
}

void BrowserPageGoanna::addUrlRedirect(const char* urlRe, int /*type*/,
                                       bool redirect, const char* userData) {
  if (!urlRe || !*urlRe) return;
  UrlRule* r = new UrlRule();
  if (regcomp(&r->re, urlRe, REG_EXTENDED | REG_NOSUB) != 0) {   // invalid regex
    delete r; return;
  }
  r->userData = userData ? userData : "";
  r->redirect = redirect;
  mRedirectRules.push_back(r);
}

bool BrowserPageGoanna::applyRedirectRules(const char* url) {
  for (UrlRule* r : mRedirectRules) {
    if (regexec(&r->re, url, 0, nullptr, 0) == 0) {   // matched
      mSink.msgUrlRedirected(url, r->userData.c_str());   // R6: notify the client
      if (r->redirect) return true;   // handled externally; do not load it here
      break;
    }
  }
  return false;
}

// The passed shared-buffer handle can be one of two forms: the production isis adapter
// (IpcBuffer::create) IPC_RMID-marks each segment and sends its SysV SHMID (attach by id
// directly &mdash; shmget can't find an IPC_RMID'd segment); our standalone test adapter sends
// an ftok KEY (needs shmget). Resolve either to an attachable shmid: try it as a shmid
// (shmctl succeeds only for a real id), else look it up as a key.
static int jihadShmResolve(int keyOrId) {
  struct shmid_ds ds;
  if (shmctl(keyOrId, IPC_STAT, &ds) == 0) return keyOrId;   // already a valid shmid
  return shmget(keyOrId, 0, 0);                              // else treat as a key (-1 if neither)
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

  // Attach-only: the adapter (isis IpcBuffer::create) makes each segment, IPC_RMID-marks
  // it for auto-delete on last detach, and sends the SysV SHMID (not an ftok key). An
  // IPC_RMID'd segment is gone from the shmget key namespace but still reachable by shmid
  // via shmctl/shmat &mdash; so we validate with shmctl(IPC_STAT) by shmid, like IpcBuffer::attach.
  for (int k : { mKey1, mKey2 }) {
    if (!k) continue;
    if (jihadShmResolve(k) < 0) {
      fprintf(stderr, "[jihad-bs] init: shm resolve of 0x%x failed: %s\n", (unsigned)k, strerror(errno));
      return false;
    }
  }

  mPage = new GoannaRenderPage(mHost);
  if (!mPage->Create((int)width, (int)height)) {
    fprintf(stderr, "[jihad-bs] init: GoannaRenderPage::Create(%u,%u) FAILED\n", width, height);
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

void BrowserPageGoanna::freeze() {
  // Card backgrounded: stop painting (the adapter may free/reuse the buffers).
  mFrozen = true;
}

void BrowserPageGoanna::thaw(int key1, int key2, int size) {
  // Reattach the (possibly new) shared buffers the adapter provides. Only resume
  // painting if BOTH required segments validate and fit the surface; otherwise
  // STAY FROZEN so maybePaint can't write a stale/reused segment (Codex P0). The
  // adapter must re-thaw with valid buffers.
  if (!mPage) return;
  const size_t need = (size_t)mPage->Width() * mPage->Height() * 4;
  bool ok = key1 && size > 0 && (size_t)size >= need;
  // keys may be SHMIDs (isis) or ftok keys (test adapter) &mdash; validate via the resolver.
  if (ok) for (int k : { key1, key2 }) { if (k && jihadShmResolve(k) < 0) ok = false; }
  if (!ok) return;   // remain frozen; keep the old (already-detached) keys inactive
  mKey1 = key1; mKey2 = key2; mBufSize = size; mActiveKey = mKey1;
  mFrozen = false;
  mNeedsPaint = true;
}

bool BrowserPageGoanna::findString(const char* text, bool forward) {
  return mPage && text && mPage->Find(text, forward);
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
  if (zoom >= 0.05 && zoom <= 20.0) mZoom = zoom;   // sane range for coord mapping (R5, Codex P0)
  double z = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
  mPage->SetZoom(zoom);
  // The adapter's scroll (x,y) is in ZOOMED-content px (its mScrollPos, buffer space);
  // the engine scrolls in CSS px, so divide by the zoom. Keep the zoomed values for
  // BrowserOffscreenInfo::renderedX/Y so the adapter's pan math lines up (== mScrollPos).
  mAdapterScrollX = x; mAdapterScrollY = y;
  mPage->ScrollTo((int)(x / z), (int)(y / z));
  mNeedsPaint = true;
  emitGeometry();   // zoom changes the rendered content size (scroll via pump)
}

// Internal about: pages served from inline HTML by the daemon (no engine/omni.ja
// change, so no libxul rebuild). Rendered via SetHtml -> data:text/html, so the markup
// MUST NOT contain '#' (data-URL fragment delimiter), '%' (escape) or newlines.
// navigator.userAgent is filled by script so it always matches the live UA.
static bool jihadAboutPage(const char* url, std::string* outHtml, std::string* outAlias) {
  if (!url) return false;
  // case-insensitive match, tolerate a trailing slash
  std::string u(url);
  for (char& c : u) c = (char)tolower((unsigned char)c);
  while (!u.empty() && (u.back() == '/' || u.back() == ' ')) u.pop_back();
  const char* kHead =
    "<html><head><title>%TITLE%</title>"
    "<meta name=viewport content='width=device-width,initial-scale=1'>"
    "<style>body{font-family:sans-serif;margin:0;padding:24px;"
    "background:rgb(17,17,17);color:rgb(235,235,235);line-height:1.5}"
    ".hdr{display:flex;align-items:center;gap:14px;margin-bottom:20px}"
    ".hdr img{width:64px;height:64px;border-radius:14px}"
    "h1{margin:0;font-size:28px}.sub{color:rgb(140,175,190);margin-top:2px}"
    ".k{color:rgb(140,175,190)}code{background:rgb(40,44,52);padding:16px 18px;"
    "border-radius:8px;font-size:15px;line-height:1.9;overflow-wrap:break-word;"
    "word-break:normal;display:block;margin-top:8px;color:rgb(210,225,240);"
    // The device has no quality monospace font (only Prelude sans + CJK), so the default
    // monospace fallback renders pixelated. Use the body sans-serif for the UA box.
    "font-family:inherit}"
    ".row{margin:10px 0}a{color:rgb(102,204,255)}</style></head><body>";
  // User-agent block (shown before the open-source section).
  const char* kUA =
    "<div class=row><span class=k>User agent</span><br><code id=ua></code></div>"
    "<script>document.getElementById('ua').textContent=navigator.userAgent</script>";
  const char* kClose = "</body></html>";
  // The Jihad Browser logo (app icon) shown on both pages. data-URL safe (no hash/percent).
  const std::string logo =
    std::string("<img src=data:image/png;base64,") + JIHAD_LOGO_B64 + " alt=logo>";
  // Shared blocks so about:jihad and about:isis stay in sync. The open-source section
  // (with each component's license) appears LAST on BOTH pages, ending with the source link.
  const char* kFork =
    "<div class=row>Jihad Browser is a fork of isis-browser with the UXP/Goanna engine "
    "replacing QtWebKit for rendering.</div>";
  const char* kCredits =
    "<div class=row><a href=https://github.com/NotAlexNoyle/Jihad-Browser>Open source</a> "
    "Jihad Browser builds on the work of:</div>"
    "<div class=row>&bull; <a href=https://github.com/isis-project/isis-browser>isis-browser</a> "
    "&mdash; the webOS browser interface (Apache License 2.0)</div>"
    "<div class=row>&bull; <a href=https://repo.palemoon.org/MoonchildProductions/UXP>UXP / Goanna</a> "
    "&mdash; the web rendering engine by Moonchild Productions (Mozilla Public License 2.0)</div>"
    "<div class=row>&bull; <a href=https://github.com/Herrie82/atlas-browser-app>Atlas by Herrie82</a> "
    "&mdash; webOS browser integration (Apache License 2.0)</div>";
  if (u == "about:jihad") {
    std::string body =
      "<div class=hdr>" + logo +
      "<div><h1>Jihad Browser</h1><div class=sub>webOS 3 - UXP/Goanna engine</div></div></div>"
      + kFork + kUA + kCredits + kClose;
    std::string html(kHead); html.replace(html.find("%TITLE%"), 7, "Jihad Browser");
    *outHtml = html + body; *outAlias = "about:jihad"; return true;
  }
  if (u == "about:isis") {
    std::string body =
      "<div class=hdr>" + logo +
      "<div><h1>isis Browser</h1><div class=sub>the project Jihad Browser is forked from</div></div></div>"
      + kFork + kUA + kCredits + kClose;
    std::string html(kHead); html.replace(html.find("%TITLE%"), 7, "isis Browser");
    *outHtml = html + body; *outAlias = "about:isis"; return true;
  }
  return false;
}

void BrowserPageGoanna::openUrl(const char* url) {
  if (!mPage || !url) return;
  // Internal about:jihad / about:isis pages: render inline HTML and report the typed
  // about: URL as the location (not the underlying data: URL). Any other load clears
  // the alias so a real page never inherits it.
  std::string aboutHtml, alias;
  if (jihadAboutPage(url, &aboutHtml, &alias)) {
    mAliasUrl = alias;
    setHTML(url, aboutHtml.c_str());
    return;
  }
  mAliasUrl.clear();
  // R6: a matching redirect rule hands the URL to the client (msgUrlRedirected)
  // and, if it is a redirect, is not loaded in the browser at all.
  if (applyRedirectRules(url)) return;
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
  if (!mPage) return;
  fprintf(stderr, "[jihad-bs] clickAt x=%d y=%d n=%d\n", x, y, numClicks);
  // The adapter already sends CONTENT coords (asyncCmdClickAt contentX/contentY =
  // (scroll+event)/zoom). The old mapToContent re-divided by zoom + re-added scroll, so
  // taps were double-transformed and missed the target at any zoom/scroll. Use directly.
  mPage->ClickAt(x, y, numClicks);
  mNeedsPaint = true;
}
void BrowserPageGoanna::keyDown(int key, int modifiers, int chr) {
  if (mPage) { mPage->KeyEvent("keydown", key, chr, modifiers); mNeedsPaint = true; }
}
void BrowserPageGoanna::keyUp(int key, int modifiers, int chr) {
  if (mPage) { mPage->KeyEvent("keyup", key, chr, modifiers); mNeedsPaint = true; }
}
void BrowserPageGoanna::mouseEvent(int type, int x, int y, int /*detail*/) {
  if (!mPage) return;
  // The BrowserAdapter's wire convention is 0=mousedown, 1=mouseup, 2=mousemove
  // (asyncCmdMouseEvent in BrowserClientBase). A tap sends down(0) then up(1); the old
  // mapping (1->down, 2->up, 0->move) was shifted, so a tap became move+down with no
  // mouseup -> links/buttons never activated. Match the adapter exactly.
  const char* t = (type == 0) ? "mousedown" : (type == 1) ? "mouseup" : "mousemove";
  // x,y are already CONTENT coords (see clickAt) -- do not re-map.
  mPage->MouseEvent(t, x, y, 0);
  mNeedsPaint = true;
}
void BrowserPageGoanna::holdAt(int x, int y) {
  if (!mPage) return;
  int cx, cy; mapToContent(x, y, &cx, &cy);   // R5 mapping
  mPage->MouseEvent("contextmenu", cx, cy, 2);   // long-press -> context menu
  mNeedsPaint = true;
}

void BrowserPageGoanna::insertStringAtCursor(const char* text) {
  if (mPage && text) { mPage->InsertText(text); mNeedsPaint = true; }
}

void BrowserPageGoanna::dragStart(int, int) { /* nothing to latch; deltas drive scroll */ }

void BrowserPageGoanna::dragProcess(int deltaX, int deltaY) {
  if (!mPage) return;
  // Drag scrolls the content opposite the finger; surface deltas -> content px.
  double z = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
  int sx = 0, sy = 0; mPage->GetScrollXY(&sx, &sy);
  auto clamp = [](double v) -> int {
    if (v < -1000000.0) return -1000000; if (v > 1000000.0) return 1000000; return (int)v; };
  mPage->ScrollTo(clamp(sx - deltaX / z), clamp(sy - deltaY / z));
  mNeedsPaint = true;
}

void BrowserPageGoanna::dragEnd(int, int) { /* scroll already applied by dragProcess */ }

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
  int cx, cy; mapToContent(x, y, &cx, &cy);   // R5: surface -> content
  mPage->TouchEvent(t, cx, cy);
  mNeedsPaint = true;
}

void BrowserPageGoanna::emitLoadAndLocation() {
  if (!mPage) return;
  if (mPage->LoadDone() && !mLoadWasDone) {
    mLoadWasDone = true;
    mNeedsPaint = true;   // paint the final frame once (dedup &mdash; Codex P2)
    fprintf(stderr, "[jihad-bs] load done uri=%s\n", mPage->CurrentUri().c_str());
    mSink.msgLoadProgress(100);
    // R5: an overridable certificate error surfaces as an SSL-confirm dialog
    // rather than a generic failed load. R3: other network failures -> failed.
    bool failed = false; int code = 0; std::string furl;
    mPage->GetLoadError(&failed, &code, &furl);
    std::string chost; int ccode = 0;
    bool certErr = mPage->GetCertError(&chost, &ccode);
    fprintf(stderr, "[jihad-bs] loaderr failed=%d code=0x%x certErr=%d chost=%s ccode=0x%x\n",
            (int)failed, (unsigned)code, (int)certErr, chost.c_str(), (unsigned)ccode);
    if (certErr) {
      mSink.msgSSLConfirm(chost.c_str(), ccode, "");
    } else if (failed) {
      mSink.msgFailedLoad("Goanna", code, furl.c_str(), "Load failed");
    }
    mSink.msgLoadStopped();
    // For internal about: pages, report the typed about: URL, not the data: URL the
    // engine actually loaded (keeps the address bar showing about:jihad/about:isis and
    // avoids polluting global history with a huge data: entry).
    std::string uri = mAliasUrl.empty() ? mPage->CurrentUri() : mAliasUrl;
    if (mPage->DidRedirect()) mSink.msgUrlRedirected(uri.c_str(), "");  // R4
    mSink.msgLocationChanged(uri.c_str(), mPage->CanGoBack(), mPage->CanGoForward());
    if (!failed && mAliasUrl.empty()) mSink.msgUpdateGlobalHistory(uri.c_str(), false);  // R6
    emitGeometry();   // R4: contents-size + meta-viewport once the page settled
  }
}

void BrowserPageGoanna::emitGeometry() {
  if (!mPage) return;
  int cw = 0, ch = 0;
  bool got = mPage->GetContentSize(&cw, &ch);
  fprintf(stderr, "[jihad-bs] emitGeometry contentSize=%dx%d win=%dx%d mZoom=%.4f (reporting=%d)\n",
          cw, ch, mPage->Width(), mPage->Height(), mZoom,
          (int)(got && (cw != mLastContentW || ch != mLastContentH)));
  if (got && (cw != mLastContentW || ch != mLastContentH)) {
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
  // R6 link-clicked: a content-initiated navigation is reported as it happens,
  // independent of the command-driven load lifecycle.
  std::string linkUrl;
  if (mPage->TakeLinkClicked(&linkUrl)) mSink.msgLinkClicked(linkUrl.c_str());
}

void BrowserPageGoanna::maybePaint() {
  if (mFrozen) return;                       // card backgrounded: don't paint
  if (mNeedsPaint) paintToSharedBuffer();    // only when there is a new frame
}

void BrowserPageGoanna::paintToSharedBuffer() {
  if (!mPage || !mActiveKey) return;
  int id = jihadShmResolve(mActiveKey);       // shmid (isis) or key->shmid (test adapter)
  if (id < 0) { perror("[BrowserPageGoanna] shm resolve"); return; }
  struct shmid_ds ds; size_t segSize = (size_t)mBufSize;
  if (shmctl(id, IPC_STAT, &ds) == 0 && ds.shm_segsz > 0) segSize = ds.shm_segsz;
  unsigned char* buf = (unsigned char*)shmat(id, nullptr, 0);
  if (buf == (unsigned char*)-1) { perror("[BrowserPageGoanna] shmat"); return; }

  // isis BrowserOffscreen shmem layout: [BrowserOffscreenInfo header][ARGB32 pixels].
  // The real BrowserAdapter reads header() at the base and rasterBuffer() at
  // base+sizeof(BrowserOffscreenInfo); it bails if renderedWidth<=0. So populate the
  // header and write pixels AFTER it (previously we wrote raw pixels at offset 0 with
  // no header -> the adapter read a garbage geometry and blitted nothing = white).
  int w = mPage->Width(), h = mPage->Height();
  const size_t hdr = sizeof(BrowserOffscreenInfo);
  if (segSize < hdr + (size_t)w * h * 4) { shmdt(buf); return; }
  BrowserOffscreenInfo* oi = (BrowserOffscreenInfo*)buf;
  // Report the zoom we actually rendered at (SetFullZoom in setZoomAndScroll). The
  // adapter blits at invScale = contentZoom/mZoomLevel; reporting 1.0 while the page
  // is rendered at mZoom made it upscale (blur). contentZoom==mZoom => 1:1 crisp blit.
  oi->bufferWidth = w; oi->bufferHeight = h;
  oi->contentZoom = (mZoom >= 0.05 && mZoom <= 20.0) ? mZoom : 1.0;
  // The buffer holds the viewport starting at the adapter's scroll position (zoomed
  // px), so renderedX/Y = that scroll &mdash; the adapter pans within the buffer using it.
  oi->renderedX = mAdapterScrollX; oi->renderedY = mAdapterScrollY;
  oi->renderedWidth = w; oi->renderedHeight = h;
  unsigned char* pixels = buf + hdr;

  long nb = mPage->ReadPixels(pixels, segSize - hdr);
  if (nb >= 0) {
    fprintf(stderr, "[jihad-bs] painted shmid=0x%x bytes=%ld (%dx%d) mZoom=%.4f contentZoom=%.4f\n",
            (unsigned)mActiveKey, nb, w, h, mZoom, oi->contentZoom);
    // Debug: dump each NON-EMPTY painted frame to a PPM so we can see exactly what
    // the engine rendered (text vs blank) independent of the adapter's blit. The
    // first paint after connect is empty (nb==0, blank buffer); guarding on nb>0
    // (not a once-only flag) makes frame.ppm hold the latest real content frame &mdash;
    // including repaints after in-page JS runs (e.g. navigator.userAgent). Env-gated.
    const char* dp = getenv("JIHAD_DUMP");
    if (dp && nb > 0) {
      FILE* f = fopen(dp, "wb");
      if (f) {
        fprintf(f, "P6\n%d %d\n255\n", w, h);            // buffer is BGRA -> write RGB
        for (long i = 0, px = (long)w * h; i < px; i++) {
          unsigned char* p = pixels + i * 4; fputc(p[2], f); fputc(p[1], f); fputc(p[0], f);
        }
        fclose(f);
        fprintf(stderr, "[jihad-bs] dumped frame -> %s\n", dp);
      }
    }
  }
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
