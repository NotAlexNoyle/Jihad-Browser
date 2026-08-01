/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — download-lifecycle adapter stand-in (domain G / R4).
 *
 * A YapClient that drives the real jihad-browserserver over the frozen YAP
 * contract and asserts the daemon-side download messages:
 *
 *   openUrl($JIHAD_DL_URL)   -> msgDownloadStart    (0x2010)
 *                            -> msgDownloadProgress (0x2011)  [>= 2 INTERMEDIATE,
 *                               monotonic, last one reaching the total — see F-6]
 *                            -> msgDownloadFinished (0x2013)  with mime + temp path
 *   openUrl($JIHAD_DL_SLOW)  -> msgDownloadStart, then
 *   cancelDownload (0x1015)  -> msgDownloadError    (0x2012), and NO finished.
 *
 * The finished temp file is stat()ed to prove the daemon really wrote the bytes
 * (both processes run in the same container / filesystem).
 */
#include <YapClient.h>
#include <YapPacket.h>
#include <glib.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

static const char* envOr(const char* k, const char* dflt) {
  const char* v = getenv(k);
  return (v && *v) ? v : dflt;
}

class DownloadClient : public YapClient {
public:
  explicit DownloadClient(const char* name)
    : YapClient(name), W(640), H(480), sz(640*480*4),
      key1(0x4a494451), key2(0x4a494452) {}

  int W, H, sz, key1, key2;
  std::string bigUrl, slowUrl;
  GMainContext* ctx = nullptr;   // the client's own loop context (timers attach here)

  // Phase 1 (bigUrl) tallies.
  int  startBig = 0, progressBig = 0, finishedBig = 0, errorBig = 0;
  // F-6: counting progress messages is NOT enough to prove per-chunk progress.
  // nsExternalAppHandler::NotifyTransfer ALWAYS fires one final
  // onProgressChange64(mProgress, mContentLength) immediately before the
  // STATE_STOP that terminates the download, so `progressBig >= 1` is satisfied
  // even if the engine reported nothing at all while the bytes were arriving —
  // deleting per-chunk progress entirely still PASSED. What the terminal
  // notification can never produce is an INTERMEDIATE tick (soFar strictly less
  // than a known total), so that is what is counted and asserted, together with
  // monotonicity and the final value actually reaching the total.
  int  progressBigMid = 0;      // soFar < total, total > 0 — only a real mid-transfer tick
  int  progressBigBad = 0;      // out-of-order/regressing soFar (must stay 0)
  long lastBigSoFar = -1;       // last reported byte count for bigUrl
  long lastBigTotal = -1;
  long finishedSize = -1;
  std::string finishedMime, finishedPath;
  // Phase 2 (slowUrl) tallies.
  int  startSlow = 0, progressSlow = 0, finishedSlow = 0, errorSlow = 0;
  bool cancelSent = false;
  bool phase2 = false;

  void serverConnected() override {}

  void openUrl(const std::string& url) {
    YapPacket* o = packetCommand();
    (*o) << (int16_t)0x1004; (*o) << url.c_str();
    sendAsyncCommand();
    printf("[dlc] -> openUrl %s\n", url.c_str());
  }

  void cancelDownload(const std::string& url) {
    YapPacket* c = packetCommand();
    (*c) << (int16_t)0x1015; (*c) << url.c_str();
    sendAsyncCommand();
    cancelSent = true;
    printf("[dlc] -> cancelDownload %s\n", url.c_str());
  }

  void start() {
    shmget(key1, sz, IPC_CREAT | 0600);
    shmget(key2, sz, IPC_CREAT | 0600);
    YapPacket* c = packetCommand();      // Connect (0x1000)
    (*c) << (int16_t)0x1000;
    (*c) << (int32_t)W; (*c) << (int32_t)H;
    (*c) << (int32_t)key1; (*c) << (int32_t)key2;
    (*c) << (int32_t)sz; (*c) << (int32_t)1;
    sendAsyncCommand();
    openUrl(bigUrl);
  }

  void beginPhase2() {
    if (phase2) return;
    phase2 = true;
    openUrl(slowUrl);
  }

  // Cancel the slow download only after it has really been running for a while,
  // so the assertion is "aborts an IN-PROGRESS download", not "refuses to start".
  void scheduleCancel(int ms) {
    if (cancelArmed || !ctx) return;
    cancelArmed = true;
    GSource* s = g_timeout_source_new(ms);
    g_source_set_callback(s, &DownloadClient::cancelCb, this, nullptr);
    g_source_attach(s, ctx);
    g_source_unref(s);
  }
  static gboolean cancelCb(gpointer d) {
    DownloadClient* c = (DownloadClient*)d;
    if (!c->cancelSent) c->cancelDownload(c->slowUrl);
    return FALSE;
  }
  bool cancelArmed = false;

  // End the run a beat after the cancel is reported — long enough that a stray
  // msgDownloadFinished for the cancelled download would still be caught.
  void scheduleQuit(int ms) {
    if (quitArmed || !ctx) return;
    quitArmed = true;
    GSource* s = g_timeout_source_new(ms);
    g_source_set_callback(s, &DownloadClient::quitCb2, this, nullptr);
    g_source_attach(s, ctx);
    g_source_unref(s);
  }
  static gboolean quitCb2(gpointer d) {
    g_main_loop_quit(((DownloadClient*)d)->mainLoop());
    return FALSE;
  }
  bool quitArmed = false;

  void serverDisconnected() override { printf("[dlc] serverDisconnected\n"); }

  void handleAsyncMessage(YapPacket* msg) override {
    int16_t id = 0;
    (*msg) >> id;
    switch ((uint16_t)id) {
      case 0x2000: {   // msgPainted — hand the buffer straight back
        int32_t k = 0; (*msg) >> k;
        YapPacket* r = packetCommand();
        (*r) << (int16_t)0x150d; (*r) << (int32_t)k;
        sendAsyncCommand();
        break; }
      case 0x2010: {   // msgDownloadStart(url)
        char* u = 0; (*msg) >> u;
        std::string url = u ? u : ""; if (u) free(u);
        printf("[dlc] <- msgDownloadStart(%s)\n", url.c_str());
        if (url == slowUrl) { ++startSlow; scheduleCancel(2000); }
        else { ++startBig; }
        break; }
      case 0x2011: {   // msgDownloadProgress(url, soFar, total)
        char* u = 0; int32_t so = 0, tot = 0;
        (*msg) >> u; (*msg) >> so; (*msg) >> tot;
        std::string url = u ? u : ""; if (u) free(u);
        if (url == slowUrl) {
          if (++progressSlow <= 3)
            printf("[dlc] <- msgDownloadProgress(slow, %d/%d)\n", so, tot);
        } else {
          if (++progressBig <= 3)
            printf("[dlc] <- msgDownloadProgress(big, %d/%d)\n", so, tot);
          if (tot > 0 && so < tot) ++progressBigMid;   // real mid-transfer tick
          if (so < lastBigSoFar) ++progressBigBad;     // progress must not go backwards
          lastBigSoFar = so; lastBigTotal = tot;
        }
        break; }
      case 0x2012: {   // msgDownloadError(url, msg)
        char* u = 0; char* e = 0; (*msg) >> u; (*msg) >> e;
        std::string url = u ? u : "", err = e ? e : "";
        if (u) free(u); if (e) free(e);
        printf("[dlc] <- msgDownloadError(%s, %s)\n", url.c_str(), err.c_str());
        if (url == slowUrl) { ++errorSlow; scheduleQuit(3000); } else ++errorBig;
        break; }
      case 0x2013: {   // msgDownloadFinished(url, mime, tmpPath)
        char* u = 0; char* m = 0; char* p = 0;
        (*msg) >> u; (*msg) >> m; (*msg) >> p;
        std::string url = u ? u : "", mime = m ? m : "", path = p ? p : "";
        if (u) free(u); if (m) free(m); if (p) free(p);
        printf("[dlc] <- msgDownloadFinished(%s, mime=%s, path=%s)\n",
               url.c_str(), mime.c_str(), path.c_str());
        if (url == slowUrl) { ++finishedSlow; break; }
        ++finishedBig;
        finishedMime = mime; finishedPath = path;
        struct stat st;
        finishedSize = (!path.empty() && stat(path.c_str(), &st) == 0) ? (long)st.st_size : -1;
        beginPhase2();
        break; }
      case 0x2015: {   // msgMimeHandoffUrl(mime, url) — the app's existing path
        char* m = 0; char* u = 0; (*msg) >> m; (*msg) >> u;
        printf("[dlc] <- msgMimeHandoffUrl(%s, %s)\n", m ? m : "", u ? u : "");
        if (m) free(m); if (u) free(u);
        break; }
      default: break;
    }
  }
};

static gboolean quitCb(gpointer data) {
  g_main_loop_quit((GMainLoop*)data);
  return FALSE;
}
// Phase 2 never completes on its own (the download is cancelled), so a second
// timer closes it out once the cancel has had time to be reported.
static gboolean phase2GuardCb(gpointer data) {
  DownloadClient* c = (DownloadClient*)data;
  if (!c->phase2) c->beginPhase2();   // phase 1 stalled; still exercise cancel
  return FALSE;
}

int main(int argc, char** argv) {
  (void)argc; (void)argv;
  setvbuf(stdout, nullptr, _IONBF, 0);
  const char* name = envOr("JIHAD_BS_NAME", "jihad-browserserver");
  DownloadClient client(name);
  client.bigUrl  = envOr("JIHAD_DL_URL",  "http://127.0.0.1:8137/big");
  client.slowUrl = envOr("JIHAD_DL_SLOW", "http://127.0.0.1:8137/slow");
  long expectSize = atol(envOr("JIHAD_DL_SIZE", "524288"));

  if (!client.connect()) { fprintf(stderr, "[dlc] connect('%s') FAILED\n", name); return 1; }
  printf("[dlc] connected to '%s'\n", name);
  GMainContext* ctx = g_main_loop_get_context(client.mainLoop());
  client.ctx = ctx;
  client.start();

  GSource* guard = g_timeout_source_new(25000);
  g_source_set_callback(guard, phase2GuardCb, &client, nullptr);
  g_source_attach(guard, ctx);
  GSource* quit = g_timeout_source_new(45000);
  g_source_set_callback(quit, quitCb, client.mainLoop(), nullptr);
  g_source_attach(quit, ctx);

  client.run();

  { int i1 = shmget(client.key1, client.sz, 0); if (i1 >= 0) shmctl(i1, IPC_RMID, nullptr);
    int i2 = shmget(client.key2, client.sz, 0); if (i2 >= 0) shmctl(i2, IPC_RMID, nullptr); }

  bool startOK    = client.startBig >= 1;
  // F-6: the download is 16 chunks of 32 KiB with a 30 ms pause between them, so
  // a working engine reports many intermediate ticks. Require at least two of
  // them (one could conceivably be an artefact of a single early flush), require
  // the counts to be monotonic, and require the last one to have actually reached
  // the total. Every one of these clauses is FALSE for a build that reports only
  // NotifyTransfer's terminal notification: that one arrives exactly once with
  // soFar == total, so progressBigMid stays 0.
  bool progressOK = client.progressBig >= 2 && client.progressBigMid >= 2 &&
                    client.progressBigBad == 0 &&
                    client.lastBigTotal == expectSize &&
                    client.lastBigSoFar == expectSize;
  bool finishOK   = client.finishedBig == 1 && !client.finishedPath.empty() &&
                    client.finishedMime.find("octet-stream") != std::string::npos &&
                    client.finishedSize == expectSize;
  bool cancelOK   = client.startSlow >= 1 && client.progressSlow >= 1 &&
                    client.cancelSent && client.finishedSlow == 0 &&
                    client.errorSlow >= 1;

  printf("[dlc] big: start=%d progress=%d mid=%d backwards=%d last=%ld/%ld "
         "finished=%d mime='%s' size=%ld (expect %ld)\n",
         client.startBig, client.progressBig, client.progressBigMid,
         client.progressBigBad, client.lastBigSoFar, client.lastBigTotal,
         client.finishedBig, client.finishedMime.c_str(), client.finishedSize,
         expectSize);
  printf("[dlc] slow: start=%d progress=%d cancelSent=%d finished=%d error=%d\n",
         client.startSlow, client.progressSlow, (int)client.cancelSent,
         client.finishedSlow, client.errorSlow);
  printf("[dlc] startOK=%d progressOK=%d finishOK=%d cancelOK=%d\n",
         startOK, progressOK, finishOK, cancelOK);

  bool ok = startOK && progressOK && finishOK && cancelOK;
  printf("[dlc] %s\n", ok ? "DOWNLOAD-LIFECYCLE PASS" : "DOWNLOAD-LIFECYCLE FAIL");
  return ok ? 0 : 2;
}
