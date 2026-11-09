/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — Goanna embedding runtime host (T-013).
 *
 * EngineHost owns the process-wide UXP/Goanna embedding lifecycle: it brings
 * the engine up once via XRE_InitEmbedding2 (pointing the GRE at the built
 * libxul directory), hands out per-page nsIWebBrowser instances, and tears the
 * runtime down via XRE_TermEmbedding. One EngineHost per BrowserServer process;
 * one nsIWebBrowser per BrowserPage. Rendering/wiring is layered on top later
 * (offscreen widget, listeners) — this is just the runtime + instance.
 */
#ifndef JIHAD_ENGINEHOST_H
#define JIHAD_ENGINEHOST_H

#include "nsCOMPtr.h"

class nsIWebBrowser;

namespace jihad {

class EngineHost
{
public:
  EngineHost() : mInited(false) {}
  ~EngineHost();

  // Initialize the embedding runtime. greDir is the directory containing
  // libxul.so (the GRE / "dist/bin"). Returns true on success. Idempotent-ish:
  // XRE_InitEmbedding2 refcounts internally.
  bool Init(const char* greDir);

  // Create a fresh browser instance (one per page/card). Caller owns the ref.
  already_AddRefed<nsIWebBrowser> CreateBrowser();

  // Tear down the embedding runtime (mirrors Init).
  void Shutdown();

  bool IsInited() const { return mInited; }

  // Low-memory guardrail for the 512 MB Pre 3 / 1 GB TouchPad targets. Called from the
  // daemon tick; internally rate-limited (polls /proc/meminfo every few seconds). When
  // available memory (MemFree+Buffers+Cached) drops below the threshold it fires the
  // engine's "memory-pressure" observer notification — JS GC/CC, image discard, and the
  // caches all subscribe — and malloc_trim(0)s the heap back to the kernel, throttled so
  // repeated pressure can't thrash the GC. Pattern ported from the Palm BrowserServer
  // memchute watcher (doMemWatch: malloc_trim + purge tiers) and Atlas's WPE memory
  // budget ("set the budget near the real free memory so the engine purges early
  // enough" — a 480 MB default budget let the WebProcess die before ever purging).
  // Threshold: the device pref jihad.memory.flushKb (default 49152 = 48 MB; JIHAD_MEM_LOW_KB for a test run).
  //
  // Past the flush (cavekit-device-headroom.md R1): when available memory stays under a floor, or the
  // system swaps in without pause, a flush alone has not helped, and on 2026-11-07 the TouchPad stopped
  // answering over USB that way (a YouTube page and a 17 MB movie, swap in use). If this daemon and its
  // players hold a real share of the memory, it sheds load one step every 10 s while it lasts: first its
  // Flash players, then its most recently navigated page, stopped and replaced by about:stopped. The thresholds are
  // device prefs (packaging/prefs/jihad-platform-prefs.js; an environment word wins for a test run):
  // jihad.memory.floorKb / JIHAD_MEM_FLOOR_KB (24 MB), jihad.memory.swapinPages / JIHAD_MEM_SWAPIN_PAGES (1024 pages
  // swapped in or out per 5 s poll, twice running), jihad.memory.shareKb / JIHAD_MEM_SHARE_KB (the larger of 128 MB and 35 %
  // of the RAM, this daemon and its players, before it sheds). And without waiting for the swap: this daemon and its
  // players past jihad.memory.budgetKb / JIHAD_MEM_BUDGET_KB (half the RAM) with under twice the flush threshold
  // available.
  void CheckMemoryPressure();

  // The daemon layer's half of the shedding: stop one page, the most recently navigated; false if none.
  static void SetShedPageHook(bool (*hook)());

private:
  bool mInited;
  int64_t mMemPollMs = 0;      // last /proc/meminfo poll (monotonic ms)
  int64_t mMemNotifyMs = 0;    // last memory-pressure notification (throttle)
  int64_t mShedMs = 0;         // last shedding step
  int mShedStep = 0;           // 0: the players next, 1: a page next
  bool mShedSpent = false;     // nothing left to stop until the pressure ends
  long mSwapIn0 = -1;          // /proc/vmstat pswpin at the last poll
  int mSwapHot = 0;            // polls in a row with swap-in over the threshold

  EngineHost(const EngineHost&) = delete;
  EngineHost& operator=(const EngineHost&) = delete;
};

// The per-domain User-Agent override for a URL's host (walking up sub-domains), or nullptr if none.
// Same table the http-on-modify-request observer uses — so navigator.userAgent (set via the
// docShell customUserAgent in LoadUrl) matches the request header for override domains (Codex F-240).
const char* JihadPerDomainUaForUrl(const char* url);
// A card connected or left: the components learn it through the observer service (the content
// blocker holds its lists only while a card is connected — cavekit-content-blocking.md R6).
// `clients` is the number still connected after this event.
void JihadNotifyClients(bool connected, int clients);
// The heap purged now (jemalloc's dirty pages returned, glibc's arena trimmed), one log line naming why — the
// memory-pressure watcher's and the last card's purge, and T-556/T-557's purge after a load goes idle.
void JihadPurgeHeap(const char* why);

} // namespace jihad

#endif // JIHAD_ENGINEHOST_H
