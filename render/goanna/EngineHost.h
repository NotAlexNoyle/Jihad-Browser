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

private:
  bool mInited;

  EngineHost(const EngineHost&) = delete;
  EngineHost& operator=(const EngineHost&) = delete;
};

} // namespace jihad

#endif // JIHAD_ENGINEHOST_H
