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

// Registers `palm://<serviceName>/` with clearCache + clearCookies and attaches it to the
// daemon's GLib loop. Returns false (having logged why) if the library, the bus, or the
// registration is unavailable; the daemon runs on regardless — this is an added capability,
// never a startup requirement.
bool LunaServiceStart(const std::string& serviceName, GMainLoop* loop);

// Best-effort unregister. Safe if Start() failed or was never called.
void LunaServiceStop();

// The service name for a variant, derived from its YAP name so the two identities cannot
// drift: "jihad-browser" -> "net.riverstonerelay.jihad-browser". See
// context/plans/plan-variant-identity.md, which is the single source for the table.
std::string LunaServiceNameFor(const char* yapName);

}  // namespace jihad

#endif
