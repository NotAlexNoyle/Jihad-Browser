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

namespace {

// ── the slice of luna-service we use, declared rather than included ─────────
// Layouts follow the webOS 3 sources in this workspace (luna-sysmgr, ref-BrowserServer).
struct LSHandle;
struct LSMessage;

typedef bool (*LSMethodFunction)(LSHandle* sh, LSMessage* msg, void* ctx);

// webOS 3's LSMethod is {name, function} — NO flags member. Getting this wrong would
// misregister every method after the first, so it is copied from the device's own
// generation of the API rather than from a later one.
struct LSMethod {
  const char*      name;
  LSMethodFunction function;
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

// Terminated with {0,0}, exactly as the stock BrowserServer's table is.
LSMethod gMethods[] = {
  { "clearCache",   onClearCache   },
  { "clearCookies", onClearCookies },
  { 0, 0 },
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
  const std::string y = (yapName && *yapName) ? yapName : "jihad-browser";
  return std::string("net.riverstonerelay.") + y;
}

bool LunaServiceStart(const std::string& serviceName, GMainLoop* loop) {
  if (!loop) return false;
  if (!loadApi()) return false;

  LSErrorBuf err; memset(&err, 0, sizeof err);
  if (gApi.ErrorInit) gApi.ErrorInit(&err);

  if (!gApi.Register(serviceName.c_str(), &gHandle, &err)) {
    // The usual cause is a missing role file: the hub refuses a name the process is not
    // permitted to own. packaging/gen-variant-scripts.sh installs prv+pub roles keyed on
    // the loader path (/proc/pid/exe is ld-2.23.so, NOT jihad-browserserver — the daemon
    // is exec'd through the bundled loader).
    printf("[jihad-bs] luna: LSRegister('%s') FAILED (code=%d) — is the role file installed?\n",
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
