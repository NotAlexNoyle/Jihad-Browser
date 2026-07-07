/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — the single source of truth for the browser User-Agent string.
 *
 * Included by EngineHost.cpp (sets general.useragent.override for network request
 * headers) and GoannaRenderPage.cpp (sets the content docShell's customUserAgent,
 * which is what navigator.userAgent actually returns — see UXP dom/base/Navigator.cpp
 * Navigator::GetUserAgent: a non-empty docShell customUserAgent short-circuits ahead
 * of nsHttpHandler, so it is the reliable per-page override).
 *
 * Token rationale (per project UA requirements):
 *   webOS/3.0.5; hpwOS/3.0.5; TouchPad  — the real HP webOS 3 platform tokens
 *   Goanna/6.9                          — the rendering engine (UXP/Goanna)
 *   UXP/b2594a4                         — the exact UXP commit this build embeds
 *   Firefox/52.9                        — site-compat token (UXP is ESR52-derived)
 *   ECMAScript/2024                     — the JS language level Goanna b2594a4 ships
 *                                         (Object.groupBy, Promise.withResolvers,
 *                                          String.isWellFormed, Array toSorted/…)
 *   JihadBrowser/1.0                    — the product (never bare "Jihad")
 */
#ifndef JIHAD_USER_AGENT_H
#define JIHAD_USER_AGENT_H

#define JIHAD_USER_AGENT \
  "Mozilla/5.0 (Linux armv7l; webOS/3.0.5; hpwOS/3.0.5; TouchPad) " \
  "Goanna/6.9 UXP/b2594a4 Firefox/52.9 ECMAScript/2024 JihadBrowser/1.0"

#endif // JIHAD_USER_AGENT_H
