/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — see JihadLunaService.h for why this service exists and why it is not
 * named com.palm.browserServer.
 */
#include "JihadLunaService.h"
#include "../goanna/GoannaRenderPage.h"   // jihad::ClearCache / ClearCookies

#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <cctype>

namespace {

// ── the slice of luna-service we use, declared rather than included ─────────
// Layouts follow the webOS 3 sources in this workspace (luna-sysmgr, ref-BrowserServer).
struct LSHandle;
struct LSMessage;

typedef bool (*LSMethodFunction)(LSHandle* sh, LSMessage* msg, void* ctx);

// LSMethod has THREE fields on this device: {name, function, flags}.
//
// I got this wrong first time by copying the shape from luna-sysmgr's sources in this
// workspace, which are LS1-era usage — but the TouchPad ships luna-service**2** (the device's
// own error text names it: luna-service2-2.0.0-136). With a two-field struct the array STRIDE
// is 8 bytes where the library reads 12, so it walks the table at the wrong offsets: LSRegister
// and LSRegisterCategory both report success, the service appears on the bus, and every call
// comes back
//     {"returnValue":false,"errorCode":-1,"errorText":"Unknown method \"clearCookies\" for category \"/\""}
// which is exactly what happened (2026-08-05). A registration that succeeds while registering
// nothing is the worst shape of wrong, so: three fields, terminator {0,0,0}.
struct LSMethod {
  const char*      name;
  LSMethodFunction function;
  unsigned int     flags;   // LSMethodFlags; 0 is the plain, non-validated method
};

// LSError is written by the library. Its exact size varies across versions and we never
// read past the first member, so it is carried as an over-sized zeroed buffer: `error_code`
// is the first field in every version, which is the only one worth logging.
struct LSErrorBuf { char raw[256]; };

typedef bool (*fn_LSErrorInit)(void*);
typedef void (*fn_LSErrorFree)(void*);
typedef bool (*fn_LSRegister)(const char*, LSHandle**, void*);
typedef bool (*fn_LSRegisterCategory)(LSHandle*, const char*, LSMethod*, void*, void*, void*);
typedef bool (*fn_LSGmainAttach)(LSHandle*, GMainLoop*, void*);
typedef bool (*fn_LSUnregister)(LSHandle*, void*);
typedef bool (*fn_LSMessageReply)(LSHandle*, LSMessage*, const char*, void*);

struct LunaApi {
  void* lib = nullptr;
  fn_LSErrorInit        ErrorInit        = nullptr;
  fn_LSErrorFree        ErrorFree        = nullptr;
  fn_LSRegister         Register         = nullptr;
  fn_LSRegisterCategory RegisterCategory = nullptr;
  fn_LSGmainAttach      GmainAttach      = nullptr;
  fn_LSUnregister       Unregister       = nullptr;
  fn_LSMessageReply     MessageReply     = nullptr;
};

LunaApi   gApi;
LSHandle* gHandle = nullptr;

const char* kOk   = "{\"returnValue\":true}";
const char* kFail = "{\"returnValue\":false}";

int errCode(const LSErrorBuf& e) { int c = 0; memcpy(&c, e.raw, sizeof c); return c; }

void reply(LSHandle* sh, LSMessage* msg, bool ok) {
  if (!gApi.MessageReply) return;
  LSErrorBuf e; memset(&e, 0, sizeof e);
  if (gApi.ErrorInit) gApi.ErrorInit(&e);
  gApi.MessageReply(sh, msg, ok ? kOk : kFail, &e);
  if (gApi.ErrorFree) gApi.ErrorFree(&e);
}

// The two methods. Both run on the daemon's main loop (LSGmainAttach), which is the same
// loop the engine is pumped on — so calling straight into the engine here is safe in the
// way a YAP callback is, and needs no extra thread hop.
bool onClearCache(LSHandle* sh, LSMessage* msg, void*) {
  printf("[jihad-bs] luna: clearCache\n");
  jihad::ClearCache();
  reply(sh, msg, true);
  return true;
}

bool onClearCookies(LSHandle* sh, LSMessage* msg, void*) {
  printf("[jihad-bs] luna: clearCookies\n");
  jihad::ClearCookies();
  reply(sh, msg, true);
  return true;
}

// Terminated with a fully-zeroed entry — all THREE fields, or the walk reads past the end.
LSMethod gMethods[] = {
  { "clearCache",   onClearCache,   0 },
  { "clearCookies", onClearCookies, 0 },
  { 0, 0, 0 },
};

bool loadApi() {
  if (gApi.lib) return true;
  // No SONAME guessing games: this is the name the device ships, and the stock browser and
  // LunaSysMgr both link it.
  gApi.lib = dlopen("liblunaservice.so", RTLD_NOW | RTLD_GLOBAL);
  if (!gApi.lib) {
    printf("[jihad-bs] luna: liblunaservice.so unavailable (%s) — no Luna service\n",
           dlerror() ? dlerror() : "?");
    return false;
  }
  gApi.ErrorInit        = (fn_LSErrorInit)       dlsym(gApi.lib, "LSErrorInit");
  gApi.ErrorFree        = (fn_LSErrorFree)       dlsym(gApi.lib, "LSErrorFree");
  gApi.Register         = (fn_LSRegister)        dlsym(gApi.lib, "LSRegister");
  gApi.RegisterCategory = (fn_LSRegisterCategory)dlsym(gApi.lib, "LSRegisterCategory");
  gApi.GmainAttach      = (fn_LSGmainAttach)     dlsym(gApi.lib, "LSGmainAttach");
  gApi.Unregister       = (fn_LSUnregister)      dlsym(gApi.lib, "LSUnregister");
  gApi.MessageReply     = (fn_LSMessageReply)    dlsym(gApi.lib, "LSMessageReply");
  if (!gApi.Register || !gApi.RegisterCategory || !gApi.GmainAttach || !gApi.MessageReply) {
    printf("[jihad-bs] luna: liblunaservice.so is missing entry points — no Luna service\n");
    return false;
  }
  return true;
}

}  // namespace

namespace jihad {

std::string LunaServiceNameFor(const char* yapName) {
  // The YAP name IS the variant identity everywhere else in this daemon, so the Luna name is
  // derived from it rather than being a second hand-maintained list that can disagree.
  //
  // BUT A LUNA SERVICE NAME CANNOT CONTAIN A HYPHEN, and our YAP names all do
  // ("jihad-browser", "jihad-browser-mochi"). LS2 rejects it at the CALLER, before anything
  // reaches us: `luna-send palm://net.riverstonerelay.jihad-browser/clearCookies` fails with
  //     _UriParse: Not a valid service name in uri … (service name: net.riverstonerelay.jihad-browser)
  // and the daemon's own LSRegister fails with -1027 for the same reason. Measured 2026-08-05.
  // Every name the platform ships is dot-separated camelCase — com.palm.browserServer,
  // com.palm.applicationManager, com.palm.appInstallService — and none has a hyphen.
  //
  // So: strip the hyphens and camel-case what follows, which turns "jihad-browser-mochi" into
  // "jihadBrowserMochi" and keeps the one-to-one mapping with the YAP name intact.
  const std::string y = (yapName && *yapName) ? yapName : "jihad-browser";
  std::string camel;
  bool up = false;
  for (char c : y) {
    if (c == '-' || c == '_') { up = true; continue; }
    camel += up ? (char)toupper((unsigned char)c) : c;
    up = false;
  }
  return std::string("net.riverstonerelay.") + camel;
}

bool LunaServiceStart(const std::string& serviceName, GMainLoop* loop) {
  if (!loop) return false;
  if (!loadApi()) return false;

  LSErrorBuf err; memset(&err, 0, sizeof err);
  if (gApi.ErrorInit) gApi.ErrorInit(&err);

  if (!gApi.Register(serviceName.c_str(), &gHandle, &err)) {
    // Two causes, and the message names both because guessing one wasted a device cycle:
    //  - an INVALID NAME (a hyphen anywhere is fatal — see LunaServiceNameFor), or
    //  - a missing/incorrect role file: the hub refuses a name the process may not own, and
    //    the role is keyed on the loader path (/proc/pid/exe is ld-2.23.so, NOT
    //    jihad-browserserver, because the job execs through the bundled loader).
    printf("[jihad-bs] luna: LSRegister('%s') FAILED (code=%d) — invalid name (no hyphens "
           "allowed) or no matching role in /usr/share/ls2/roles\n",
           serviceName.c_str(), errCode(err));
    if (gApi.ErrorFree) gApi.ErrorFree(&err);
    gHandle = nullptr;
    return false;
  }
  if (!gApi.RegisterCategory(gHandle, "/", gMethods, nullptr, nullptr, &err)) {
    printf("[jihad-bs] luna: LSRegisterCategory FAILED (code=%d)\n", errCode(err));
    if (gApi.ErrorFree) gApi.ErrorFree(&err);
    LunaServiceStop();
    return false;
  }
  if (!gApi.GmainAttach(gHandle, loop, &err)) {
    printf("[jihad-bs] luna: LSGmainAttach FAILED (code=%d)\n", errCode(err));
    if (gApi.ErrorFree) gApi.ErrorFree(&err);
    LunaServiceStop();
    return false;
  }
  if (gApi.ErrorFree) gApi.ErrorFree(&err);
  printf("[jihad-bs] luna: serving palm://%s/{clearCache,clearCookies}\n", serviceName.c_str());
  return true;
}

void LunaServiceStop() {
  if (!gHandle || !gApi.Unregister) { gHandle = nullptr; return; }
  LSErrorBuf err; memset(&err, 0, sizeof err);
  if (gApi.ErrorInit) gApi.ErrorInit(&err);
  gApi.Unregister(gHandle, &err);
  if (gApi.ErrorFree) gApi.ErrorFree(&err);
  gHandle = nullptr;
}

}  // namespace jihad
