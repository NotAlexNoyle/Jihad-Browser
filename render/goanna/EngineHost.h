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
  // Threshold override: JIHAD_MEM_LOW_KB (default 49152 = 48 MB).
  void CheckMemoryPressure();

private:
  bool mInited;
  long mMemPollMs = 0;      // last /proc/meminfo poll (monotonic ms)
  long mMemNotifyMs = 0;    // last memory-pressure notification (throttle)

  EngineHost(const EngineHost&) = delete;
  EngineHost& operator=(const EngineHost&) = delete;
};

// The per-domain User-Agent override for a URL's host (walking up sub-domains), or nullptr if none.
// Same table the http-on-modify-request observer uses — so navigator.userAgent (set via the
// docShell customUserAgent in LoadUrl) matches the request header for override domains (Codex F-240).
const char* JihadPerDomainUaForUrl(const char* url);

} // namespace jihad

#endif // JIHAD_ENGINEHOST_H
