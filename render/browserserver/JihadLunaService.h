/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — the daemon's own LunaService (cavekit-ipc-contract.md R4).
 *
 * WHY THIS EXISTS, and why it is NOT `com.palm.browserServer`.
 * The isis app clears cache/cookies by calling `palm://com.palm.browserServer/clearCache`
 * over the Luna bus. That name belongs to the STOCK browser daemon, which we deliberately
 * coexist with rather than replace — so on a device with Jihad installed, the call SUCCEEDS
 * and clears the wrong browser: measured 2026-08-04, `{"returnValue":true}` came back from
 * the stock daemon while Jihad's own cookies were untouched. Silent, and user-visible the
 * moment cookie persistence started working.
 *
 * So each variant registers its OWN service name, the fifth instance of a pattern this port
 * already uses four times for coexistence (own NPAPI MIME, own YAP service name, own upstart
 * job, own state directory). The alternative — exposing clearCache/clearCookies as scriptable
 * adapter methods — would widen the frozen `callBrowserAdapter` set that cavekit-ui-shell.md
 * R2 holds byte-identical to upstream, i.e. break a met criterion in another kit to fix this
 * one. The service name is not part of that contract; the adapter surface is.
 *
 * NO BUILD-TIME DEPENDENCY. There is no `lunaservice.h` in the ARM sysroot (the device has
 * `/usr/lib/liblunaservice.so` but no headers), so the handful of entry points are declared
 * here and resolved with dlopen/dlsym at startup. That also means a device or a desktop build
 * WITHOUT the library degrades to "no Luna service" instead of failing to start — which is
 * exactly what the desktop harness needs, since there is no Luna bus there at all.
 *
 * The struct layouts below are taken from the webOS 3 sources in this workspace, not guessed:
 * `LSMethod` is {name, function} with NO flags field on this generation (luna-sysmgr
 * BackupManager.cpp, ref-BrowserServer BrowserServer.cpp both terminate with {0,0}).
 */
#ifndef JIHAD_LUNA_SERVICE_H
#define JIHAD_LUNA_SERVICE_H

#include <glib.h>
#include <string>

namespace jihad {

// Registers `palm://<serviceName>/` with clearCache + clearCookies + notifications and
// attaches it to the daemon's GLib loop. Returns false (having logged why) if the library, the
// bus, or the registration is unavailable; the daemon runs on regardless — this is an added
// capability, never a startup requirement.
//
// `notifications` is the SUBSCRIBED method behind the non-blocking message channel
// (cavekit-gre-widgets.md R5). A card calls it once with {"subscribe":true} at startup and is
// then pushed one payload per jihad::PostNotification. On success this installs a
// jihad::NotificationSink; LunaServiceStop clears it again. Same argument as every other
// method here: this is a LUNA surface, so the byte-frozen YAP command/message interface
// (cavekit-ipc-contract.md R1) is untouched and no card gains an adapter call.
bool LunaServiceStart(const std::string& serviceName, GMainLoop* loop);

// The app id of THIS variant's card ("net.riverstonerelay.jihad-browser[-variant]"). The methods
// that change something on the public bus (enableFlash / disableFlash) accept a call only from
// it. Call before LunaServiceStart; unset means those methods refuse everyone.
void LunaServiceSetOwnAppId(const std::string& appId);

// Who renders a bookmark thumbnail: given a path, arrange for a PNG of the active page to be
// written there and say whether a page was found. Set by the server before LunaServiceStart.
typedef bool (*LunaThumbnailFn)(const std::string& path);
void LunaServiceSetThumbnailFn(LunaThumbnailFn fn);

// Who prints: given the card's request (JSON naming the printer), hand the active page to the
// printing component. Set by the server before LunaServiceStart.
typedef bool (*LunaPrintFn)(const std::string& requestJson);
void LunaServiceSetPrintFn(LunaPrintFn fn);

// Who knows the active page's certificate: fills `json` (one object, returnValue included)
// and says whether the page came over TLS. Set by the server before LunaServiceStart.
typedef bool (*LunaCertificateFn)(std::string* json);
void LunaServiceSetCertificateFn(LunaCertificateFn fn);

// Read webOS's own proxy setting (com.palm.connectionmanager/getNwProxiesConfig) once; the
// answer, or a returnValue:false saying why there is none, is raised to the components as the
// observer topic "jihad-webos-proxy" (cavekit-proxies-certs.md R1). Main loop only.
void LunaServiceReadWebosProxy();

// Best-effort unregister. Safe if Start() failed or was never called.
void LunaServiceStop();

// The service name for a variant, derived from its YAP name so the two identities cannot
// drift: "jihad-browser" -> "net.riverstonerelay.jihad-browser". See
// context/plans/plan-variant-identity.md, which is the single source for the table.
std::string LunaServiceNameFor(const char* yapName);

}  // namespace jihad

#endif
