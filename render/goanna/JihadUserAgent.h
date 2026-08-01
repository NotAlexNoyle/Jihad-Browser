/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — the single source of truth for the browser's IDENTITY: the
 * User-Agent string AND the application identity the engine reports through
 * nsIXULAppInfo (see JihadAppInfo in EngineHost.cpp).
 *
 * Included by EngineHost.cpp (sets general.useragent.override for network request
 * headers, and registers the app-info service) and GoannaRenderPage.cpp (sets the
 * content docShell's customUserAgent, which is what navigator.userAgent actually
 * returns — see UXP dom/base/Navigator.cpp Navigator::GetUserAgent: a non-empty
 * docShell customUserAgent short-circuits ahead of nsHttpHandler, so it is the
 * reliable per-page override).
 *
 * WHY THE APP IDENTITY LIVES HERE TOO. It is the same fact stated twice — the UA's
 * product token and nsIXULAppInfo's name/version both answer "which browser is
 * this, at what version". Keeping them in two files is how they drift. The UA
 * string below is COMPOSED from the macros, so the product token cannot disagree
 * with what the engine reports to chrome JS. (The composed value is byte-identical
 * to the string this file carried before the macros existed.)
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

// ── application identity (nsIXULAppInfo) ────────────────────────────────────
//
// THIS IS A COMPATIBILITY CONTRACT, NOT DECORATION. An installed XPI declares
// <em:targetApplication> against exactly this ID plus a min/max version range, so
// an extension that claims to support Jihad Browser is naming THIS GUID at THIS
// version. Changing either silently disables every add-on a user has installed.
//
//   * JIHAD_APP_ID is FROZEN. Minted once, for this browser; it must not change
//     for anything short of a deliberate compatibility break.
//   * JIHAD_APP_VERSION is the version add-ons range against. Two components is a
//     normal Gecko app version (Firefox shipped "3.6", "52.9"). It is a DIFFERENT
//     namespace from the webOS package version in app*/appinfo.json, which tracks
//     packaging and moves on its own schedule.
//   * The branding strip (cavekit-licensing-branding.md R3) removed the Pale Moon
//     / Basilisk identity strings from the engine. Do not reintroduce them here:
//     this is Jihad's own identity, not a borrowed one.
#define JIHAD_APP_ID      "{4534aac8-d8c8-4765-95ee-7f61fd0b762d}"
#define JIHAD_APP_NAME    "JihadBrowser"
#define JIHAD_APP_VENDOR  "Jihad Browser project"
#define JIHAD_APP_VERSION "1.0"

// Build ID. Gecko's convention is a 14-digit YYYYMMDDHHMMSS stamp whose ONLY
// semantic is "did this change since the last run": the add-on manager rebuilds
// its database and re-evaluates compatibility when it moves, and the startup
// cache is keyed on it. Deliberately a FIXED constant rather than
// __DATE__/__TIME__ — a stamp that moved on every compile would force an add-on
// rescan on every build and make the daemon binary non-reproducible. BUMP IT
// when shipping a build whose chrome/components changed in a way that must
// invalidate those caches.
#define JIHAD_APP_BUILD_ID "20260801000000"

#define JIHAD_USER_AGENT \
  "Mozilla/5.0 (Linux armv7l; webOS/3.0.5; hpwOS/3.0.5; TouchPad) " \
  "Goanna/6.9 UXP/b2594a4 Firefox/52.9 ECMAScript/2024 " \
  JIHAD_APP_NAME "/" JIHAD_APP_VERSION

#endif // JIHAD_USER_AGENT_H
