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
#include <sys/time.h>   // gettimeofday (paint render timing)
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
    mAdapterScrollX(0), mAdapterScrollY(0), mFrozen(false), mHadContent(false), mGeometryDirty(false),
    mPendingClick(false), mPendingClickX(0), mPendingClickY(0), mPendingClickN(1) {
  mShmBuf[0] = mShmBuf[1] = nullptr; mShmId[0] = mShmId[1] = -1;
  mInFlight[0] = mInFlight[1] = false; mPaintMs[0] = mPaintMs[1] = 0;
  mLastBackspaceMs = 0; mBackspaceRun = 0;
  mLoadStartMs = 0;
  mLastProgress = 0;
  mWatchdogDismissed = false;
  mDirtyPending = false; mLastDirtyPaintMs = 0;
}

// Deferred editing keys (run in pump(), not the keyDown YAP callback — Codex F-219). Enter may
// submit a form and navigate; Tab in an <input> focuses another field (fires focus/blur JS). Both
// must run in the page-lifetime-protected pump loop. (Tab in a <textarea> just inserts a tab, but
// HandleTab decides that, so all Tabs defer for uniformity.)
enum { PEA_NONE = 0, PEA_TAB = 1, PEA_TAB_BACK = 2, PEA_ENTER = 3 };

// Monotonic-enough wall clock in ms for the buffer flow-control timeout valve.
static long jihadNowMs() {
  struct timeval tv; gettimeofday(&tv, NULL);
  return (long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int BrowserPageGoanna::slotForKey(int key) const {
  if (key && key == mKey1) return 0;
  if (key && key == mKey2) return 1;
  return -1;
}

static int jihadShmResolve(int keyOrId);   // defined below

// Resolve a key/shmid and return a cached attachment (attaching once, then reusing). shmat/shmdt
// per paint cost ~400ms/keystroke on device; the two alternating segments are attached at most once.
unsigned char* BrowserPageGoanna::attachShm(int keyOrId) {
  int id = jihadShmResolve(keyOrId);
  if (id < 0) return nullptr;
  for (int i = 0; i < 2; ++i) if (mShmId[i] == id && mShmBuf[i]) return mShmBuf[i];
  void* p = shmat(id, nullptr, 0);
  if (p == (void*)-1) { perror("[BrowserPageGoanna] shmat"); return nullptr; }
  int slot = (mShmBuf[0] == nullptr) ? 0 : 1;   // 2 buffers total; fill an empty slot
  if (mShmBuf[slot]) { shmdt(mShmBuf[slot]); }
  mShmBuf[slot] = (unsigned char*)p; mShmId[slot] = id;
  return mShmBuf[slot];
}

void BrowserPageGoanna::detachShm() {
  for (int i = 0; i < 2; ++i) { if (mShmBuf[i]) shmdt(mShmBuf[i]); mShmBuf[i] = nullptr; mShmId[i] = -1; }
}

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
  detachShm();
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
  mInFlight[0] = mInFlight[1] = false;   // fresh segments: nothing in flight (F-211)

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
  if (width > 8192 || height > 8192) return;                 // bound BEFORE the multiply (32-bit ARM, review #7 P3)
  if ((uint64_t)width * height * 4 > (uint64_t)mBufSize) return;
  // Defer the geometry emit to pump() (after PumpFor lets the reflow settle) rather than
  // emitting synchronously here, mid-resize, when GetContentSize can still read 0x0
  // (review #7 P2). mGeometryDirty is drained on the next tick.
  // T1 instrumentation (device 2026-07-17: "page pushed up off screen when the search bar is
  // focused"): log the engine scroll across the resize — the VKB shrink arrives as a
  // setWindowSize, so these lines show whether the reflow moved the scroll (engine-side push)
  // or the scroll was already moved before the resize (adapter/app-side push). GATED behind
  // JIHAD_T1_LOG (inspector P3): each GetScrollXY forces a layout flush, and two extra full
  // reflows per VKB toggle in the socket-callback context is a real cost on the ARMv7.
  static const bool t1log = getenv("JIHAD_T1_LOG") != nullptr;
  // T1 fix ("page pushed up off screen when the search box is focused"): a VKB-raise
  // arrives as a height SHRINK. The engine's Resize reflow re-scrolls the focused
  // editable into view, shoving the page up — even though the user just tapped that
  // field so it was already visible. Capture the scroll before the shrink and, if a
  // field is focused and the reflow pushed the scroll DOWN (content up), restore the
  // pre-resize offset so the page stays put under the keyboard. Only on a shrink
  // (VKB up); a grow (VKB down) keeps the engine's natural scroll. The field the user
  // tapped stays visible because ClickAt already ensured it was on-screen pre-resize.
  const bool shrink = (mLastWinH != 0 && (int)height < mLastWinH);
  const bool hadEditable = mPage->HasFocusedEditable();
  int sx0 = 0, sy0 = 0;
  if (t1log || (shrink && hadEditable)) mPage->GetScrollXY(&sx0, &sy0);
  if (mPage->Resize((int)width, (int)height)) { mNeedsPaint = true; mGeometryDirty = true; }
  if (shrink && hadEditable) {
    int sx1 = 0, sy1 = 0; mPage->GetScrollXY(&sx1, &sy1);
    // Only counter an engine-induced downward push (field-into-view), and only when
    // the tapped field's top stays within the shrunken viewport if we restore — i.e.
    // its content-y is above the pre-resize scroll + the new (smaller) height. That
    // keeps a field the VKB would otherwise cover lifted, but stops the common case
    // (top-of-page search box) from being shoved off the top.
    if (sy1 > sy0) mPage->ScrollTo(sx0, sy0);
  }
  mLastWinH = (int)height;
  if (t1log) {
    int sx2 = 0, sy2 = 0; mPage->GetScrollXY(&sx2, &sy2);
    fprintf(stderr, "[jihad-bs] setWindowSize %ux%u scroll %d,%d -> %d,%d editable=%d shrink=%d\n",
            width, height, sx0, sy0, sx2, sy2, (int)hadEditable, (int)shrink);
  }
}

void BrowserPageGoanna::freeze() {
  // Card backgrounded: stop painting (the adapter may free/reuse the buffers).
  // Drop the cached shm attachments too: the production segments are IPC_RMID-marked,
  // so holding an attach keeps them alive and can exhaust the device's scarce shared
  // memory while backgrounded. thaw() re-validates and maybePaint() re-attaches on demand.
  mFrozen = true;
  detachShm();
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
  detachShm();       // the buffers may be new segments — drop stale cached attachments
  mKey1 = key1; mKey2 = key2; mBufSize = size; mActiveKey = mKey1;
  mInFlight[0] = mInFlight[1] = false;   // reattached (possibly new) segments: clear in flight (F-211)
  mFrozen = false;
  mNeedsPaint = true;
}

bool BrowserPageGoanna::findString(const char* text, bool forward) {
  return mPage && text && mPage->Find(text, forward);
}

void BrowserPageGoanna::returnBuffer(int sharedBufferKey) {
  // The adapter is done blitting this buffer (F-211): clear its in-flight mark so the next
  // paint may reuse it. This is what makes per-keystroke immediate painting safe under fast
  // typing — without it the daemon overwrote a buffer the adapter was still reading and the
  // adapter (in LunaSysMgr) crashed. If a queued paint was deferred waiting on this return,
  // paint it now so the latest frame lands promptly instead of on the next tick.
  int s = slotForKey(sharedBufferKey);
  if (s >= 0) mInFlight[s] = false;
  if (mNeedsPaint && !mFrozen) maybePaint();
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
  mPage->SetZoom(z);   // clamped — SetZoom(raw) diverged from mZoom/contentZoom (review #6 F-008)
  // The adapter's scroll (x,y) is in ZOOMED-content px (its mScrollPos, buffer space);
  // the engine scrolls in CSS px, so divide by the zoom. Keep the zoomed values for
  // BrowserOffscreenInfo::renderedX/Y so the adapter's pan math lines up (== mScrollPos).
  mAdapterScrollX = x; mAdapterScrollY = y;
  mPage->ScrollTo((int)(x / z), (int)(y / z));
  mNeedsPaint = true;
  // NB: do NOT emitGeometry() here. A zoom-only command does not change the page's
  // intrinsic CSS content size, but re-reporting it fed the adapter's fit-zoom, which sent
  // a new zoom, which re-reported size... a cross-process oscillation (mZoom flapped
  // 1.0<->0.75<->0.7837, size 768x942<->1024x686 on rotate). Content size is emitted only
  // from setWindowSize + load-done now; the adapter derives its own contentWidth from the
  // zoom it applied. (review #6 F-001)
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
  mPendingClick = false;   // a newer explicit navigation supersedes any queued tap
  mPendingEditActions.clear();   // queued Tab/Enter for the old page are stale after a navigation
  // Any navigation lowers the VKB (covers link tap, JS location.href re-drive, form submit,
  // typed URL, back/forward) — otherwise the keyboard stays up over the new page (F-005).
  if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false, 0, 0);
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
  mLastProgress = 0;
  mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted();
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
  mLoadWasDone = false; mNeedsPaint = false; mLastProgress = 0;
  mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted();
  if (!mPage->SetHtml(body)) { mSink.msgLoadStopped(); mLoadWasDone = true; }
}

// Nav commands restart the load lifecycle so completion re-emits load+location.
// back/forward/reload also clear editor focus so the VKB lowers over the new page (review #7 P2).
void BrowserPageGoanna::pageBackward() { if (mPage) { mPendingClick=false; if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false,0,0); mLoadWasDone=false; mNeedsPaint=false; mLastProgress=0; mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted(); mPage->GoBack(); } }
void BrowserPageGoanna::pageForward() { if (mPage) { mPendingClick=false; if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false,0,0); mLoadWasDone=false; mNeedsPaint=false; mLastProgress=0; mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted(); mPage->GoForward(); } }
void BrowserPageGoanna::pageReload()  { if (mPage) { mPendingClick=false; if (mPage->ClearEditorFocus()) mSink.msgEditorFocused(false,0,0); mLoadWasDone=false; mNeedsPaint=false; mLastProgress=0; mLoadStartMs = jihadNowMs(); mSink.msgLoadStarted(); mPage->Reload(); } }
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
  // Record ONLY. All engine interaction for the tap — activation, hit-test
  // (ElementFromPoint flushes layout), mouse/DOMClick synthesis, and navigation — runs in
  // pump() on the tick. clickAt is a YAP socket callback (JihadBrowserServer mInTick==false)
  // with no page-lifetime protection; any of those calls can run page script (onfocus,
  // onmousedown, onclick -> location/history/document.open) that tears the document down
  // mid-callback, then we keep using freed engine objects -> SIGSEGV whose core dump
  // stalled I/O hard enough to REBOOT the device. pump() runs inside the tick's
  // mInTick/mReap guard, so teardown there is safe.
  mPendingClickX = x; mPendingClickY = y; mPendingClickN = numClicks;
  mPendingClick = true;
  mNeedsPaint = true;
}
void BrowserPageGoanna::keyDown(int key, int modifiers, int chr) {
  if (!mPage) return;
  // Never log key/chr — those are user keystrokes (F-163). When a tapped editable is the type
  // target, the webOS VKB delivers each character (and the editing keys) as a keyDown; route it
  // into the field via the engine editor (caret-aware value edits + SetSelectionRange), and render
  // IMMEDIATELY (don't wait for the ~16ms tick) so typing feels responsive. Non-editable focus
  // falls through to normal engine key dispatch.
  if (mPage->HasFocusedEditable()) {
    // webOS delivers editing keys to the plugin as: ASCII control codes where they exist
    // (ESC=27, Backspace=8, Tab=9, Enter=13) and the Apple/NSEvent function-key Unicode range
    // (0xF700+) for arrows / Home / End / forward-Delete — the WebKit webOS derives from puts
    // those code points in the key field. (Qt keycodes 0x0100001x are matched too, as a fallback.)
    // The code arrives in `key` with `chr`==0, but we accept it from either field. A printable
    // character inserts at the caret; the editing keys drive the engine caret.
    typedef jihad::GoannaRenderPage GRP;
    const int kBackspace = 8, kTab = 9, kEnter = 13, kDelete = 127;
    const int kQtTab = 0x01000001, kQtBacktab = 0x01000002, kQtReturn = 0x01000004,
              kQtEnter = 0x01000005, kQtDelete = 0x01000007, kQtHome = 0x01000010,
              kQtEnd = 0x01000011, kQtLeft = 0x01000012, kQtUp = 0x01000013,
              kQtRight = 0x01000014, kQtDown = 0x01000015;
    const int kMacUp = 0xF700, kMacDown = 0xF701, kMacLeft = 0xF702, kMacRight = 0xF703,
              kMacDelete = 0xF728, kMacHome = 0xF729, kMacEnd = 0xF72B;
    // webOS's own special-key block (measured on-device via the keyprobe): the VKB keypad's arrow
    // keys arrive here, NOT in the Apple/Qt ranges. Down/Up/Left/Right are 0xE0A0..0xE0A3.
    const int kWebDown = 0xE0A0, kWebUp = 0xE0A1, kWebLeft = 0xE0A2, kWebRight = 0xE0A3;
    const bool shift = (modifiers & (1 << 2)) != 0;   // npPalmShiftKeyModifier
    auto km = [key, chr](int v) { return key == v || chr == v; };
    if (!km(kBackspace)) mBackspaceRun = 0;   // any non-Backspace key breaks the accelerate run
    if (km(kBackspace)) {
      // Accelerate a held Backspace: consecutive presses within the auto-repeat window build a run;
      // once sustained (~0.5s of holding), delete a whole word per repeat so long text clears fast.
      long now = jihadNowMs();
      mBackspaceRun = (now - mLastBackspaceMs <= 250) ? (mBackspaceRun + 1) : 1;
      mLastBackspaceMs = now;
      if (mBackspaceRun >= 12) mPage->DeleteBackwardWord();
      else                     mPage->DeleteBackward();
      mNeedsPaint = true;
    } else if (km(kDelete) || km(kQtDelete) || km(kMacDelete)) {
      mPage->EditKey(GRP::EK_DELETE); mNeedsPaint = true;
    } else if (km(kTab) || km(kQtTab) || km(kQtBacktab)) {
      // Tab: <textarea> inserts a tab, single-line <input> moves to the next field (fires focus JS)
      // — defer to pump() (F-219). Shift+Tab / the Qt backtab code go backward.
      mPendingEditActions.push_back((shift || km(kQtBacktab)) ? PEA_TAB_BACK : PEA_TAB); mNeedsPaint = true;
    } else if (km(kEnter) || km(kQtReturn) || km(kQtEnter)) {
      // Enter may submit a form and navigate — defer to pump() (never navigate in the key callback).
      mPendingEditActions.push_back(PEA_ENTER); mNeedsPaint = true;
    } else if (km(kQtLeft)  || km(kMacLeft)  || km(kWebLeft))  { mPage->EditKey(GRP::EK_LEFT);  mNeedsPaint = true; }
    else if (km(kQtRight) || km(kMacRight) || km(kWebRight)) { mPage->EditKey(GRP::EK_RIGHT); mNeedsPaint = true; }
    else if (km(kQtUp)    || km(kMacUp)    || km(kWebUp))    { mPage->EditKey(GRP::EK_UP);    mNeedsPaint = true; }
    else if (km(kQtDown)  || km(kMacDown)  || km(kWebDown))  { mPage->EditKey(GRP::EK_DOWN);  mNeedsPaint = true; }
    else if (km(kQtHome)  || km(kMacHome))  { mPage->EditKey(GRP::EK_HOME);  mNeedsPaint = true; }
    else if (km(kQtEnd)   || km(kMacEnd))   { mPage->EditKey(GRP::EK_END);   mNeedsPaint = true; }
    else {
      // The webOS VKB puts the printable character (letters, digits, symbols, and non-ASCII) in
      // `key` (rawkeyCode) with `chr`==0 on device; accept it from whichever field carries it, for
      // the FULL Unicode range (not just ASCII), then UTF-8 encode -> InsertText. Anything >= 0x20
      // and not DEL is treated as a character; Backspace/Enter/Tab (< 0x20) are handled elsewhere.
      unsigned int c = (chr >= 0x20 && chr != 0x7f) ? (unsigned int)chr
                     : (key >= 0x20 && key != 0x7f) ? (unsigned int)key : 0u;
      // Only encode a valid Unicode scalar: reject the UTF-16 surrogate range and anything
      // above U+10FFFF so we never emit malformed UTF-8 into the DOM value (Codex F-210). We do
      // NOT reject keycodes like 0x25-0x28 as "arrow keys": on this device the VKB delivers the
      // actual character in `key`, so those values are the '%','&',apostrophe,'(' symbols — the
      // VKB has no arrow keys. Enter/Tab/Backspace (< 0x20) are handled/swallowed above.
      if (c >= 0xD800 && c <= 0xDFFF) c = 0u;
      else if (c > 0x10FFFF) c = 0u;
      // Function/nav keys arrive as Private-Use code points, NOT text: webOS's own special-key
      // block 0xE0A0-0xE0BF (arrows measured at 0xE0A0-0xE0A3; Home/End/PageUp/Down likely adjacent)
      // and the Apple/NSEvent function-key range 0xF700-0xF8FF. The ones we map are consumed above;
      // swallow the rest here so an unmapped nav key never inserts an invisible PUA glyph (which
      // read as "arrows type characters" + repaint lag + Backspace eating the invisible chars).
      else if (c >= 0xE0A0 && c <= 0xE0BF) c = 0u;
      else if (c >= 0xF700 && c <= 0xF8FF) c = 0u;
      if (c) {
        char buf[5] = {0};
        if (c < 0x80) { buf[0] = (char)c; }
        else if (c < 0x800) { buf[0] = (char)(0xC0 | (c >> 6)); buf[1] = (char)(0x80 | (c & 0x3F)); }
        else if (c < 0x10000) { buf[0] = (char)(0xE0 | (c >> 12)); buf[1] = (char)(0x80 | ((c >> 6) & 0x3F)); buf[2] = (char)(0x80 | (c & 0x3F)); }
        else { buf[0] = (char)(0xF0 | (c >> 18)); buf[1] = (char)(0x80 | ((c >> 12) & 0x3F)); buf[2] = (char)(0x80 | ((c >> 6) & 0x3F)); buf[3] = (char)(0x80 | (c & 0x3F)); }
        mPage->InsertText(buf);
        mNeedsPaint = true;
      }
      // else: an unmapped control/nav key — swallow it (do not insert).
    }
    if (mNeedsPaint) maybePaint();   // paint now, in this keyDown, instead of waiting for the tick
    return;
  }
  mPage->KeyEvent("keydown", key, chr, modifiers);
  mNeedsPaint = true;
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
  // NB: never log `text` — it is user keystrokes (incl. passwords) and this stream is redirected
  // to a persistent, user-readable file on device (Jihad review F-163).
  if (mPage && text) { mPage->InsertText(text); mNeedsPaint = true; if (mNeedsPaint) maybePaint(); }
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

// Emit failure/cert + location + title/url for a settled load, and return the reported uri ("" if the
// load failed, so the caller skips writing history). Does NOT emit the load-stopped boundary — isis's
// pageLoadStopped writes the app history entry from the title/url this sets, so load-stopped must run
// AFTER this or history records stale data (Codex F-004/F-334).
std::string BrowserPageGoanna::emitLocationAndTitle() {
  // R5: an overridable certificate error surfaces as an SSL-confirm dialog rather than a generic
  // failed load. R3: other network failures -> failed.
  bool failed = false; int code = 0; std::string furl;
  mPage->GetLoadError(&failed, &code, &furl);
  std::string chost; int ccode = 0;
  bool certErr = mPage->GetCertError(&chost, &ccode);
  fprintf(stderr, "[jihad-bs] loaderr failed=%d code=0x%x certErr=%d chost=%s ccode=0x%x\n",
          (int)failed, (unsigned)code, (int)certErr, chost.c_str(), (unsigned)ccode);
  if (certErr) mSink.msgSSLConfirm(chost.c_str(), ccode, "");
  else if (failed) mSink.msgFailedLoad("Goanna", code, furl.c_str(), "Load failed");
  // For internal about: pages, report the typed about: URL, not the data: URL the engine loaded
  // (keeps the bar showing about:jihad and avoids a huge data: history entry). NB: never emit
  // msgUrlRedirected for an ordinary HTTP 3xx redirect — in isis the app binds onUrlRedirected ->
  // openResource -> the DEFAULT (stock) browser, so a normal redirect (google.com -> www.google.com)
  // would leave Jihad. A redirect is just a location change, reported here.
  std::string uri = mAliasUrl.empty() ? mPage->CurrentUri() : mAliasUrl;
  mSink.msgLocationChanged(uri.c_str(), mPage->CanGoBack(), mPage->CanGoForward());
  // The isis address bar updates on the title+url message (BasicWebView.titleURLChange ->
  // urlTitleChanged -> ActionBar.setUrl), NOT on msgLocationChanged. If the page has no <title>, fall
  // back to the URL so the bar still shows where we are (review #6 F-003).
  { std::string title = mPage->GetTitle();
    if (title.empty()) title = uri;
    bool cb = mPage->CanGoBack(), cf = mPage->CanGoForward();
    fprintf(stderr, "[jihad-bs] titleAndUrl title=[%s] uri=%s back=%d fwd=%d\n", title.c_str(), uri.c_str(), (int)cb, (int)cf);
    mSink.msgTitleAndUrlChanged(title.c_str(), uri.c_str(), cb, cf); }
  return failed ? std::string() : uri;
}

// Full completion boundary: progress, location/title, load-stopped, global history, geometry, repaint.
// emitProgress100 drives the bar to 100 (true for a normal completion and the watchdog's forced one;
// omit only if it is already at 100). Emits EXACTLY ONE load-stopped per load — the late-completion path
// deliberately does not call this (it only re-syncs the address bar), so pageLoadStopped/history run
// once (Codex F-361).
void BrowserPageGoanna::emitCompletion(bool emitProgress100) {
  if (emitProgress100) mSink.msgLoadProgress(100);
  std::string uri = emitLocationAndTitle();
  mSink.msgLoadStopped();   // records app history from the title/url just set (F-334)
  if (!uri.empty() && mAliasUrl.empty()) mSink.msgUpdateGlobalHistory(uri.c_str(), false);  // R6
  emitGeometry();      // R4: contents-size + meta-viewport once the page settled
  mNeedsPaint = true;  // paint the final frame once (dedup — Codex P2)
}

void BrowserPageGoanna::emitLoadAndLocation() {
  if (!mPage) return;
  // Incremental load progress: feed the isis address-bar progress bar while the load is in flight so a
  // slow 512 MB-device page visibly advances instead of sitting at 0% — a frozen full-width bar reads
  // as a crashed "loading screen" (the user's #1 complaint). Only emit increases (isis ignores
  // decreases and clears its bar at 100, which the completion boundary sends).
  if (!mLoadWasDone) {
    int p = mPage->GetLoadProgress();
    // Progress also repaints (belt-and-braces under the 0012 dirty flag): bytes arrived, so the
    // incremental render likely changed — show the page building up instead of a stale frame.
    if (p > mLastProgress) { mLastProgress = p; mSink.msgLoadProgress(p); mNeedsPaint = true; }
  }
  bool stalled = (!mLoadWasDone && mLoadStartMs != 0 && (jihadNowMs() - mLoadStartMs) > 12000);
  if (mPage->LoadDone() && !mLoadWasDone) {
    // Normal completion — the one and only load-stopped boundary for this load.
    mLoadWasDone = true; mLoadStartMs = 0; mWatchdogDismissed = false;
    fprintf(stderr, "[jihad-bs] load done uri=%s\n", mPage->CurrentUri().c_str());
    emitCompletion(true);
    // New document settled: hang the engine focus/blur listener on it so script-driven
    // focus changes drive the VKB (Atlas IM-context port, device T4). No-op if same doc.
    mPage->RegisterEngineFocusListener();
  } else if (mWatchdogDismissed && mPage->LoadDone()) {
    // LATE completion: the engine finished a load the stall watchdog already gave a full completion
    // boundary. The lifecycle (load-started..load-stopped) is already balanced, so DON'T emit another
    // load-stopped/progress (that would double pageLoadStopped + unbalance the stream — Codex F-361).
    // Just re-sync the address bar to the FINAL url/title in case it changed since the watchdog fired
    // (e.g. a redirect that resolved late) and repaint the final frame.
    mWatchdogDismissed = false;
    fprintf(stderr, "[jihad-bs] late load done uri=%s\n", mPage->CurrentUri().c_str());
    emitLocationAndTitle();
    emitGeometry();
    mNeedsPaint = true;
    mPage->RegisterEngineFocusListener();   // the late-settled doc still needs the focus listener
  } else if (stalled) {
    // Stall watchdog: a load active too long without completing must NEVER pin the isis loading overlay
    // open — it covers the whole card and looks crashed. Force ONE full completion boundary (R3) so the
    // lifecycle is balanced even if the request never finishes (a permanently-stalled load isn't left
    // with load-started unmatched — Codex F-353). By 12s the doc has almost always COMMITTED (headers
    // received) so CurrentUri is the new page, not stale (F-360). This does NOT Stop() the engine: the
    // request keeps loading and, if it finishes, the late branch above re-syncs the final url + repaints
    // (F-288) — without a second load-stopped. ClearProgrammaticLoad lets a form/location.href/
    // meta-refresh nav the user triggers on the partial page still be detected and adopted/re-driven
    // instead of being misread as a command load and dropped (Codex F-333).
    fprintf(stderr, "[jihad-bs] load watchdog: forcing load-stopped after %ldms (engine still loading)\n",
            jihadNowMs() - mLoadStartMs);
    mLoadWasDone = true; mLoadStartMs = 0; mWatchdogDismissed = true;
    mPage->ClearProgrammaticLoad();
    emitCompletion(true);
    // The watchdog-completed page still needs the engine focus listener — by 12 s the document
    // has almost always committed, and this slow-load path is exactly the wedged-VKB scenario
    // the feature targets (inspector P2). Re-resolves when the load later finishes (late branch).
    mPage->RegisterEngineFocusListener();
  }
}

bool BrowserPageGoanna::emitGeometry() {
  if (!mPage) return false;
  int cw = 0, ch = 0;
  bool got = mPage->GetContentSize(&cw, &ch);
  fprintf(stderr, "[jihad-bs] emitGeometry contentSize=%dx%d win=%dx%d mZoom=%.4f (reporting=%d)\n",
          cw, ch, mPage->Width(), mPage->Height(), mZoom,
          (int)(got && cw > 0 && ch > 0 && (cw != mLastContentW || ch != mLastContentH)));
  // NEVER emit a degenerate (0,0) content size. During a resize, GetContentSize (GetRootBounds)
  // momentarily returns (0,0) while the viewport reflow rebuilds the root frame — the Flush_Layout
  // can complete mid-reconstruction. The adapter treats contentsSizeChanged(0,0) as "new page":
  // it DROPS the live offscreen buffer (-> white card) and resets zoom-fit to 1.0 (-> the 1.0<->0.75
  // flicker). Guarding cw>0 && ch>0 kills the dominant path of BOTH the resize white-out and the
  // zoom oscillation (review #7 P1). Keeps the last good size until a real one arrives.
  if (got && cw > 0 && ch > 0 && (cw != mLastContentW || ch != mLastContentH)) {
    mLastContentW = cw; mLastContentH = ch;
    mSink.msgContentsSizeChanged(cw, ch);        // R4: contents-size-changed
  }
  double is = 1.0, mn = 1.0, mx = 1.0; int vw = 0, vh = 0; bool us = true;
  if (mPage->GetViewport(&is, &mn, &mx, &vw, &vh, &us))
    mSink.msgMetaViewportSet(is, mn, mx, vw, vh, us);   // R4: meta-viewport
  return got && cw > 0 && ch > 0;   // a valid content size was available (deferred-emit can stop retrying)
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
  // Fire the pending 'input' event for a keystroke edit FIRST — before the queued tap AND the Tab/Enter
  // queue below. It targets the element that was actually edited (mPendingInputEl). Flushing before the
  // tap matters when the user types then taps a submit/button or a link: a controlled/React handler must
  // observe the edited value before the click submits or navigates, and the event must dispatch on the
  // still-current document before openUrl swaps it out (Codex F-266/F-291). Runs here (guarded pump),
  // not in keyDown, since onChange/oninput run page JS.
  mPage->FlushPendingInputEvent();
  // Process a queued tap next — inside the tick's page-lifetime guard, and BEFORE
  // spending the pump budget so a link's load gets pumped this call (matters for
  // single-pump callers like link_test). ClickAt does the hit-test + activation +
  // mouse/DOMClick; for a link it records the href (TakeClickNav) instead of navigating.
  if (mPendingClick) {
    mPendingClick = false;
    fprintf(stderr, "[jihad-bs] clickAt %d,%d n=%d\n", mPendingClickX, mPendingClickY, mPendingClickN);
    mPage->ClickAt(mPendingClickX, mPendingClickY, mPendingClickN);
    // VKB: the tap may have focused/blurred an editable field -> tell isis to raise/hide
    // the on-screen keyboard.
    bool efoc = false; int eft = 0, efa = 0;
    if (mPage->TakeEditorFocus(&efoc, &eft, &efa)) {
      fprintf(stderr, "[jihad-bs] editorFocused=%d fieldType=%d\n", (int)efoc, eft);
      mSink.msgEditorFocused(efoc, eft, efa);
    }
    std::string clickNav;
    if (mPage->TakeClickNav(&clickNav)) {
      // R6 (navigation-events): report the intercepted link activation, THEN navigate via
      // the load path. openUrl marks the load programmatic, so OnStateChange won't ALSO
      // flag it link-clicked — this msg is the single R6 notification for the tap.
      fprintf(stderr, "[jihad-bs] clickAt -> navigate %s\n", clickNav.c_str());
      mSink.msgLinkClicked(clickNav.c_str());
      openUrl(clickNav.c_str());
    }
  }
  // Process a queued editing key (Tab/Enter) in the SAME page-lifetime guard — it runs page JS
  // that may move focus or submit a form (navigate), which is unsafe in the keyDown YAP callback
  // (Codex F-219). A form submit navigates via the engine and is completed by the TakeLinkClicked
  // re-drive below, exactly like any content-initiated navigation.
  if (!mPendingEditActions.empty()) {
    // Process every queued Tab/Enter in press order (a swap-and-clear so a handler that queues more
    // doesn't reenter this loop). Stop early once an editable is gone (a submit navigated away).
    std::vector<int> acts;
    acts.swap(mPendingEditActions);
    for (int act : acts) {
      if (!mPage->HasFocusedEditable()) break;   // a prior Enter submitted + navigated
      // A form-submit attempt ends the drain so a second queued Enter can't double-submit — regardless
      // of whether the nav commits synchronously (which also clears the edit target) or asynchronously
      // (cleared next tick by the adopt/re-drive's BeginLoad) — Codex F-323/F-325.
      if (act == PEA_ENTER)         { if (mPage->HandleEnter()) break; }
      else if (act == PEA_TAB)      mPage->HandleTab(false);
      else if (act == PEA_TAB_BACK) mPage->HandleTab(true);
    }
    mNeedsPaint = true;
    // A form submit navigates away / Tab moves fields — keep the VKB state in sync if it changed.
    bool efoc = false; int eft = 0, efa = 0;
    if (mPage->TakeEditorFocus(&efoc, &eft, &efa)) mSink.msgEditorFocused(efoc, eft, efa);
  }
  // R6 link-clicked: detect + re-drive/adopt a content-initiated navigation BEFORE PumpFor, so the
  // re-driven (GET) or adopted (POST) load runs to completion within THIS tick's PumpFor and is
  // reported by the emitLoadAndLocation below. Doing this AFTER PumpFor raced: a fast POST that fired
  // STATE_START+STATE_STOP inside PumpFor was already done, and adopting it then reset mDone=false
  // after its only STOP — no completion until the 12 s watchdog (Codex F-323).
  std::string linkUrl; bool linkIsPost = false;
  if (mPage->TakeLinkClicked(&linkUrl, &linkIsPost)) {
    mSink.msgLinkClicked(linkUrl.c_str());
    // Content-initiated navigation (JS `location.href`/`location.assign`, a form GET/POST,
    // meta-refresh, or a button onclick that sets location) STARTS but does NOT COMPLETE on its own
    // in this offscreen embedding (verified: it fires STATE_START for the target but never load-done).
    if (!linkIsPost) {
      // Re-drive a GET-class nav through the programmatic load path, which completes and supersedes
      // the stalled content load. openUrl marks the load programmatic so it won't re-trigger
      // TakeLinkClicked -> no loop.
      fprintf(stderr, "[jihad-bs] content-nav re-drive GET -> %s\n", linkUrl.c_str());
      openUrl(linkUrl.c_str());
    } else {
      // A POST content-nav (form submit) is NOT re-driven: the original request has already reached
      // STATE_START — the body may already be on the wire — so re-issuing it would DOUBLE the POST
      // (double login/charge, Codex F-262). Leave the request to the ENGINE, but ADOPT it as the
      // tracked load so the daemon still: shows the overlay + progress, arms the stall watchdog, and
      // (crucially) emits the completion + new location and schedules a repaint when the POST response
      // arrives — otherwise the response silently stays on the old frame (Codex F-289).
      fprintf(stderr, "[jihad-bs] content-nav POST (engine-driven) %s\n", linkUrl.c_str());
      mPage->AdoptContentLoad();
      mLoadWasDone = false;
      mNeedsPaint = false;
      mLastProgress = 0;
      mLoadStartMs = jihadNowMs();
      mSink.msgLoadStarted();
    }
  }
  mPage->PumpFor(msBudget);
  // Engine-driven repaint (UXP patch 0012): PumpFor just ran layout/JS/imagelib work; if any of it
  // invalidated content (incremental page render, SPA/JS DOM update, async image decode, animation),
  // repaint. This is the frame-delivery loop the stock QtWebKit server got from Qt paint events —
  // without it the shared buffer goes stale until user input forces a paint (device T3: "the old page
  // sticks around until you tap or drag"). RATE-LIMITED (inspector P2): a page with a persistent
  // animation (CSS spinner, video, SPA churn) otherwise re-dirties EVERY tick, forcing a full-document
  // software render + 3 MB buffer copy at ~30 Hz on the single-core ARMv7 — input latency + battery
  // drain. Dirty-driven paints are capped at one per 150 ms (~6 fps for pure animation; latency the
  // user can't perceive on a spinner); input/load-driven paints stay immediate via their own
  // mNeedsPaint sets. The pending flag is sticky so a dirty burst still paints when the window opens.
  if (mPage->TakeDirty()) mDirtyPending = true;
  if (mDirtyPending) {
    long dnow = jihadNowMs();
    if (dnow - mLastDirtyPaintMs >= 150) { mNeedsPaint = true; mDirtyPending = false; mLastDirtyPaintMs = dnow; }
  }
  // Engine-driven VKB sync (Atlas IM-context port, device T4): merge any focus/blur the engine
  // dispatched during PumpFor into the VKB state machine and emit the change. Script-driven focus
  // moves and blurs now raise/lower the keyboard without a tap; a wedged app-side state gets clean
  // false transitions to recover on.
  mPage->PollEngineFocus();
  { bool efoc = false; int eft = 0, efa = 0;
    if (mPage->TakeEditorFocus(&efoc, &eft, &efa)) {
      fprintf(stderr, "[jihad-bs] engine editorFocused=%d\n", (int)efoc);
      mSink.msgEditorFocused(efoc, eft, efa);
    } }
  // Emit deferred resize geometry now that PumpFor has let the reflow settle (review #7 P2).
  // emitGeometry itself guards against a still-degenerate 0x0 (P1), so a not-yet-settled
  // reflow just re-defers via the guard until a real size is available.
  if (mGeometryDirty && emitGeometry()) mGeometryDirty = false;   // retry until reflow yields a valid size
  emitLoadAndLocation();
  emitScrollIfChanged();
  // Low-memory guardrail (512 MB Pre 3 floor): internally rate-limited /proc/meminfo poll; fires the
  // engine "memory-pressure" flush + malloc_trim when RAM runs short. Lives here (not the engine-
  // agnostic browserserver tick) so the daemon layer keeps zero engine includes.
  mHost.CheckMemoryPressure();
}

void BrowserPageGoanna::maybePaint() {
  if (mFrozen) return;                       // card backgrounded: don't paint
  if (mNeedsPaint) paintToSharedBuffer();    // only when there is a new frame
}

void BrowserPageGoanna::paintToSharedBuffer() {
  if (!mPage || !mActiveKey) return;
  // Flow control (F-211): never overwrite a buffer the adapter still holds. If the target buffer
  // was msgPainted but not yet returned, defer this paint (mNeedsPaint stays set, so the next
  // pump tick / returnBuffer retries) — the typed text is already in the DOM, only the frame is
  // delayed. A timeout valve reclaims the buffer if a return was lost so painting can't stall.
  {
    int gs = slotForKey(mActiveKey);
    if (gs >= 0 && mInFlight[gs]) {
      if (jihadNowMs() - mPaintMs[gs] < 250) return;   // adapter may still be blitting it — wait
      mInFlight[gs] = false;                            // stale: assume the return was lost, reclaim
    }
  }
  int id = jihadShmResolve(mActiveKey);       // shmid (isis) or key->shmid (test adapter)
  if (id < 0) { perror("[BrowserPageGoanna] shm resolve"); return; }
  struct shmid_ds ds; size_t segSize = (size_t)mBufSize;
  if (shmctl(id, IPC_STAT, &ds) == 0 && ds.shm_segsz > 0) segSize = ds.shm_segsz;
  unsigned char* buf = attachShm(mActiveKey);   // cached attach — no per-paint shmat/shmdt (~400ms)
  if (!buf) return;

  // isis BrowserOffscreen shmem layout: [BrowserOffscreenInfo header][ARGB32 pixels].
  // The real BrowserAdapter reads header() at the base and rasterBuffer() at
  // base+sizeof(BrowserOffscreenInfo); it bails if renderedWidth<=0. So populate the
  // header and write pixels AFTER it (previously we wrote raw pixels at offset 0 with
  // no header -> the adapter read a garbage geometry and blitted nothing = white).
  int w = mPage->Width(), h = mPage->Height();
  const size_t hdr = sizeof(BrowserOffscreenInfo);
  if (segSize < hdr + (size_t)w * h * 4) { return; }
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
  if (nb < 0) return;
  // Blank-over-good suppression (review #7 P1): a mid-reflow render fills the target white
  // (nb==0). Do NOT push that over a previously-good frame — keep the last valid frame on
  // screen and retry next tick (leave mNeedsPaint set, do NOT flip the active buffer). This
  // removes the resize/rotation/navigation white-flash. Only a genuinely-never-rendered page
  // (mHadContent==false) blits a blank frame.
  if (nb == 0 && mHadContent) return;   // mNeedsPaint stays true -> retried until content lands
  if (nb > 0) mHadContent = true;

  mNeedsPaint = false;
  mSink.msgPainted(mActiveKey);
  // This buffer is now the adapter's until it returns it (F-211). Mark it in flight so a
  // subsequent paint won't overwrite it mid-blit, and record when for the timeout valve.
  {
    int ps = slotForKey(mActiveKey);
    if (ps >= 0) { mInFlight[ps] = true; mPaintMs[ps] = jihadNowMs(); }
  }
  // Double buffer: next paint targets the other segment. The adapter holds one buffer at a
  // time and returns the previous one on the next msgPainted, so alternating keeps a free
  // buffer available; the in-flight gate above blocks reuse until that return arrives.
  mActiveKey = (mActiveKey == mKey1 && mKey2) ? mKey2 : mKey1;
}

} // namespace jihad
