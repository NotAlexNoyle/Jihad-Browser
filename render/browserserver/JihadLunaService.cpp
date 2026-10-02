/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — see JihadLunaService.h for why this service exists and why it is not
 * named com.palm.browserServer.
 */
#include "JihadLunaService.h"
#include "../goanna/GoannaRenderPage.h"   // jihad::ClearCache / ClearCookies
#include "../goanna/DialogService.h"      // jihad::NotificationSink / SetNotificationSink

#include "../goanna/JihadRuntimePaths.h"   // RuntimeProfileDir (the app directory, for thumbnails)

#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <string>

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

// THE PALM-SERVICE FAMILY: one registration that serves BOTH buses.
//
// LSRegister gives a PRIVATE-bus handle only, and an app CARD is on the PUBLIC bus — so with
// LSRegister alone the card's calls never arrive at all. Measured 2026-08-06 on device, both
// sides: `luna-send -n 1 …/clearCache` (private) replies {"returnValue":true} and the daemon
// logs it; `luna-send -P -n 1 …/clearCache` (public) returns nothing and the daemon logs
// NOTHING. That is why app/source/Browser.js's clearCookies/clearCache buttons had never once
// worked — cavekit-browser-services.md R2 was verified with `luna-send -a com.palm.configurator`,
// a privileged private-bus caller, which does not exercise the shipped path.
//
// The public role files this needs are ALREADY installed by packaging/gen-variant-scripts.sh
// (/usr/share/ls2/roles/pub/<name>.json with inbound:["*"]), verified present on device — so
// this is a daemon change, not a packaging or policy change.
struct LSPalmService;
typedef bool (*fn_LSRegisterPalmService)(const char*, LSPalmService**, void*);
// (psh, category, methods_PUBLIC, methods_PRIVATE, signals, category_user_data, lserror)
typedef bool (*fn_LSPalmServiceRegisterCategory)(LSPalmService*, const char*, LSMethod*,
                                                 LSMethod*, void*, void*, void*);
typedef bool (*fn_LSGmainAttachPalmService)(LSPalmService*, GMainLoop*, void*);
typedef bool (*fn_LSUnregisterPalmService)(LSPalmService*, void*);

// THE SUBSCRIPTION SLICE — the non-blocking message channel (cavekit-gre-widgets.md R5).
//
// Signatures read out of the webOS 3 sources in this workspace rather than guessed:
//   LSSubscriptionAdd(LSHandle*, const char* key, LSMessage*, LSError*)
//       luna-sysmgr/Src/base/DisplayManager.cpp:1836, HapticsController.cpp:161,
//       application/ApplicationInstaller.cpp:1938
//   LSSubscriptionReply(LSHandle*, const char* key, const char* payload, LSError*)
//       luna-sysmgr/Src/base/AmbientLightSensor.cpp:403, DisplayManager.cpp:2470,
//       application/ApplicationInstaller.cpp:3657
// Those are LS1-era CALLERS, and this device runs luna-service2 — the same mistake that cost a
// cycle on LSMethod's third field. What protects us here is that neither takes a struct BY
// VALUE and no struct layout is involved: they are pointer-and-string calls whose shape is
// identical in both generations. THE SYMBOLS THEMSELVES ARE NOT CONFIRMED PRESENT ON THIS
// DEVICE — no device was available when this landed — so both are dlsym'd, NULL-checked, and
// the whole channel degrades to "no channel" if either is missing. That is the established
// rule for this file (see loadApi below).
typedef bool (*fn_LSSubscriptionAdd)(LSHandle*, const char*, LSMessage*, void*);
typedef bool (*fn_LSSubscriptionReply)(LSHandle*, const char*, const char*, void*);
typedef const char* (*fn_LSMessageGetApplicationID)(LSMessage*);
typedef const char* (*fn_LSMessageGetPayload)(LSMessage*);

struct LunaApi {
  void* lib = nullptr;
  fn_LSErrorInit        ErrorInit        = nullptr;
  fn_LSErrorFree        ErrorFree        = nullptr;
  fn_LSRegister         Register         = nullptr;
  fn_LSRegisterCategory RegisterCategory = nullptr;
  fn_LSGmainAttach      GmainAttach      = nullptr;
  fn_LSUnregister       Unregister       = nullptr;
  fn_LSMessageReply     MessageReply     = nullptr;
  fn_LSRegisterPalmService         RegisterPalmService         = nullptr;
  fn_LSPalmServiceRegisterCategory PalmServiceRegisterCategory = nullptr;
  fn_LSGmainAttachPalmService      GmainAttachPalmService      = nullptr;
  fn_LSUnregisterPalmService       UnregisterPalmService       = nullptr;
  fn_LSSubscriptionAdd             SubscriptionAdd             = nullptr;
  fn_LSSubscriptionReply           SubscriptionReply           = nullptr;
  fn_LSMessageGetApplicationID     MessageGetApplicationID     = nullptr;
  fn_LSMessageGetPayload           MessageGetPayload           = nullptr;
};

LunaApi        gApi;
LSHandle*      gHandle = nullptr;      // set only on the LSRegister fallback path
LSPalmService* gPalm   = nullptr;      // set on the normal (both-buses) path

// ── subscription bookkeeping ─────────────────────────────────────────────────────────────
// The subscription key is per-category, not per-caller: every subscriber gets every message.
const char* kNotifyKey = "/notifications";

// The bus handles that have at least one subscriber, learned from the callbacks themselves.
//
// A palm-service registration serves TWO buses and LSSubscriptionReply takes an LSHandle*, so
// replying needs the right handle(s). The alternative — LSPalmServiceGetPublicConnection /
// …GetPrivateConnection — is two MORE unconfirmed symbols for information the subscribe
// callback already hands us in its `sh` argument. Recording what actually arrived is both
// smaller and impossible to get wrong: a handle only lands here because a real subscriber
// called through it. Two slots because there are exactly two buses.
LSHandle* gSubHandles[2] = { nullptr, nullptr };

void rememberSubHandle(LSHandle* sh) {
  if (!sh) return;
  for (LSHandle* h : gSubHandles) if (h == sh) return;
  for (LSHandle*& h : gSubHandles) { if (!h) { h = sh; return; } }
}

const char* kOk   = "{\"returnValue\":true}";
const char* kFail = "{\"returnValue\":false}";

int errCode(const LSErrorBuf& e) { int c = 0; memcpy(&c, e.raw, sizeof c); return c; }

// Append `s` to `out` as a QUOTED, escaped JSON string. Every value that reaches a reply from
// here is either a stored pref or a message built from content-controlled text (an add-on name
// out of install.rdf), so none of it may be spliced in raw: a stray quote or control character
// would not just corrupt the value, it would break the reply's own JSON and the card would see
// a parse error instead of a message.
void appendJsonString(std::string& out, const std::string& s) {
  out += '"';
  for (size_t i = 0; i < s.size(); ++i) {
    unsigned char c = (unsigned char)s[i];
    switch (c) {
    case '"':  out += "\\\""; break;
    case '\\': out += "\\\\"; break;
    case '\n': out += "\\n"; break;
    case '\r': out += "\\r"; break;
    case '\t': out += "\\t"; break;
    default:
      if (c < 0x20) { char b[8]; snprintf(b, sizeof b, "\\u%04x", c); out += b; }
      else          { out += (char)c; }
    }
  }
  out += '"';
}

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

// getChromePrefs — the CARD's read side of the settings the SETTINGS PAGE owns.
//
// about:preferences runs in this process with the system principal and writes
// `jihad.chrome.settings` (one JSON string) through Services.prefs. The home button and the
// start page live in the card, which cannot reach engine prefs — so it asks here. Read-only
// on purpose: there is exactly one writer, which is what makes the merged settings page a
// single source of truth (cavekit-preferences-ui.md R5).
//
// Nothing is added to the YAP contract by this; Luna is a separate, already-used channel.
bool onGetChromePrefs(LSHandle* sh, LSMessage* msg, void*) {
  std::string json;
  bool have = jihad::GetChromeSettings(&json);
  printf("[jihad-bs] luna: getChromePrefs (%s, %u bytes)\n",
         have ? "set" : "unset", (unsigned)json.size());
  if (!gApi.MessageReply) return true;
  // The stored value is a JSON *string*; it is embedded as a quoted, escaped string rather
  // than spliced in raw, so a malformed or hostile stored value cannot break the reply's
  // own JSON. The card parses the inner string itself.
  std::string out = "{\"returnValue\":true,\"settings\":";
  appendJsonString(out, have ? json : std::string());
  out += "}";
  LSErrorBuf e; memset(&e, 0, sizeof e);
  if (gApi.ErrorInit) gApi.ErrorInit(&e);
  gApi.MessageReply(sh, msg, out.c_str(), &e);
  if (gApi.ErrorFree) gApi.ErrorFree(&e);
  return true;
}

// getFlash / enableFlash / disableFlash — the card's Flash switch
// (cavekit-addons-extensions.md R7, cavekit-mochi-ui.md R2).
//
// The value is the user pref `plugin.state.flash`, the SAME one the settings page's row writes,
// so there is one stored value and two places to change it. These are on the PUBLIC bus because
// that is where a card is — which means any app on the device can call them, so the two that
// change something check WHO is calling: the hub stamps each message with the caller's app id,
// and only this variant's own card is accepted. Reading is open (it reveals whether Flash is
// installed and on, nothing about the user).
std::string gOwnAppId;   // "net.riverstonerelay.jihad-browser[-variant]", set by LunaServiceStart

bool callerIsOwnCard(LSMessage* msg) {
  if (!gApi.MessageGetApplicationID || gOwnAppId.empty()) return false;
  const char* id = gApi.MessageGetApplicationID(msg);     // "<appid> <pid>" for a card
  if (!id) return false;
  const size_t n = gOwnAppId.size();
  return strncmp(id, gOwnAppId.c_str(), n) == 0 && (id[n] == '\0' || id[n] == ' ');
}

void replyFlash(LSHandle* sh, LSMessage* msg, bool ok) {
  bool available = false, enabled = true;
  jihad::GetFlashState(&available, &enabled);
  if (!gApi.MessageReply) return;
  std::string out = ok ? "{\"returnValue\":true" : "{\"returnValue\":false,\"errorText\":\"not permitted\"";
  out += std::string(",\"available\":") + (available ? "true" : "false");
  out += std::string(",\"enabled\":") + (enabled ? "true" : "false") + "}";
  LSErrorBuf e; memset(&e, 0, sizeof e);
  if (gApi.ErrorInit) gApi.ErrorInit(&e);
  gApi.MessageReply(sh, msg, out.c_str(), &e);
  if (gApi.ErrorFree) gApi.ErrorFree(&e);
}

bool onGetFlash(LSHandle* sh, LSMessage* msg, void*) {
  printf("[jihad-bs] luna: getFlash\n");
  replyFlash(sh, msg, true);
  return true;
}

bool setFlashFrom(LSHandle* sh, LSMessage* msg, bool enabled) {
  const bool allowed = callerIsOwnCard(msg);
  printf("[jihad-bs] luna: %sFlash (%s)\n", enabled ? "enable" : "disable",
         allowed ? "own card" : "REFUSED: caller is not this variant's card");
  if (allowed) jihad::SetFlashEnabled(enabled);
  replyFlash(sh, msg, allowed);
  return true;
}
bool onEnableFlash(LSHandle* sh, LSMessage* msg, void*)  { return setFlashFrom(sh, msg, true); }
bool onDisableFlash(LSHandle* sh, LSMessage* msg, void*) { return setFlashFrom(sh, msg, false); }

// saveThumbnail / deleteThumbnail / clearThumbnails — bookmark thumbnails
// (cavekit-mochi-ui.md R2).
//
// A card can only show an image from its own application directory, and only the daemon has
// the page's pixels — so the daemon renders the active page to <appdir>/thumbnails/t-….png and
// hands back the path. Own card only, like the Flash setters: a thumbnail shows what the user
// is looking at. The card sends pageFocused(true) down the adapter just before saveThumbnail,
// which is how the daemon knows which of several cards is asking (Luna carries no page).
jihad::LunaThumbnailFn gThumbnailFn = nullptr;

std::string thumbnailDir() {
  std::string d = jihad::RuntimeProfileDir();              // <appdir>/profile
  const size_t cut = d.find_last_of('/');
  if (cut == std::string::npos || cut == 0) return std::string();
  return d.substr(0, cut) + "/thumbnails";
}

// "t-<digits>-<digits>.png" and nothing else: the only names saveThumbnail ever hands out, so
// the only names deleteThumbnail will touch.
bool isThumbnailName(const std::string& n) {
  if (n.size() < 9 || n.size() > 48 || n.compare(0, 2, "t-") != 0) return false;
  if (n.compare(n.size() - 4, 4, ".png") != 0) return false;
  for (size_t i = 2; i + 4 < n.size(); ++i) {
    const char c = n[i];
    if (!((c >= '0' && c <= '9') || c == '-')) return false;
  }
  return true;
}

void replyJson(LSHandle* sh, LSMessage* msg, const std::string& out) {
  if (!gApi.MessageReply) return;
  LSErrorBuf e; memset(&e, 0, sizeof e);
  if (gApi.ErrorInit) gApi.ErrorInit(&e);
  gApi.MessageReply(sh, msg, out.c_str(), &e);
  if (gApi.ErrorFree) gApi.ErrorFree(&e);
}

bool onSaveThumbnail(LSHandle* sh, LSMessage* msg, void*) {
  static unsigned seq = 0;
  const std::string dir = thumbnailDir();
  bool ok = callerIsOwnCard(msg) && gThumbnailFn && !dir.empty();
  std::string path;
  if (ok) {
    mkdir(dir.c_str(), 0755);
    char name[64];
    snprintf(name, sizeof name, "t-%ld-%u.png", (long)time(nullptr), ++seq);
    path = dir + "/" + name;
    ok = gThumbnailFn(path);
  }
  printf("[jihad-bs] luna: saveThumbnail (%s)\n", ok ? path.c_str() : "refused or no page");
  std::string out = ok ? "{\"returnValue\":true,\"path\":" : "{\"returnValue\":false";
  if (ok) appendJsonString(out, path);
  out += "}";
  replyJson(sh, msg, out);
  return true;
}

bool onDeleteThumbnail(LSHandle* sh, LSMessage* msg, void*) {
  bool ok = false;
  std::string name;
  if (callerIsOwnCard(msg) && gApi.MessageGetPayload) {
    // The request is {"name":"t-….png"} (a full path is accepted and its last component
    // used). Not a JSON parser: the value is pulled out and then held to isThumbnailName,
    // which is what actually decides.
    const char* pl = gApi.MessageGetPayload(msg);
    const char* k = pl ? strstr(pl, "\"name\"") : nullptr;
    const char* q = k ? strchr(k + 6, '"') : nullptr;
    const char* e = q ? strchr(q + 1, '"') : nullptr;
    if (q && e) {
      name.assign(q + 1, e - q - 1);
      const size_t slash = name.find_last_of('/');
      if (slash != std::string::npos) name = name.substr(slash + 1);
      const std::string dir = thumbnailDir();
      if (isThumbnailName(name) && !dir.empty()) ok = (unlink((dir + "/" + name).c_str()) == 0);
    }
  }
  printf("[jihad-bs] luna: deleteThumbnail (%s)\n", ok ? name.c_str() : "refused or not found");
  reply(sh, msg, ok);
  return true;
}

bool onClearThumbnails(LSHandle* sh, LSMessage* msg, void*) {
  int n = 0;
  const std::string dir = thumbnailDir();
  const bool allowed = callerIsOwnCard(msg) && !dir.empty();
  if (allowed) {
    if (DIR* d = opendir(dir.c_str())) {
      while (struct dirent* de = readdir(d)) {
        if (isThumbnailName(de->d_name) && unlink((dir + "/" + de->d_name).c_str()) == 0) ++n;
      }
      closedir(d);
    }
  }
  printf("[jihad-bs] luna: clearThumbnails (%s, %d removed)\n", allowed ? "own card" : "REFUSED", n);
  reply(sh, msg, allowed);
  return true;
}

// notifications — the CARD's non-blocking message channel (cavekit-gre-widgets.md R5).
//
// The card calls this ONCE at startup with {"subscribe":true} and leaves the call open; the
// daemon then pushes one payload per jihad::PostNotification. There is no polling method and
// no request/response form on purpose — a message the daemon raises has no request to answer,
// and giving it one would recreate the round-trip this whole channel exists to avoid.
//
// Wire shape:
//   ->  {"subscribe":true}
//   <-  {"returnValue":true,"subscribed":true}            (once, immediately)
//   <-  {"category":"privacy","text":"Cookies cleared."}   (zero or more, later)
// The pushes carry no "returnValue" — they are not answers to anything. A card distinguishes
// them by the presence of `text`.
//
// PUBLIC BUS, deliberately: an app CARD is a public-bus caller, measured 2026-08-06, and a
// private-only registration means the card's call never arrives at all. The exposure that buys
// is bounded and was weighed rather than assumed: any app on the device may subscribe and will
// then see the same one-line statements the user is already being shown on screen — "Cookies
// cleared.", "<name> installed." Nothing here carries a URL, a page title, cookie contents or
// the user's home page, which is exactly why getChromePrefs stays private-only two tables down.
// KEEP IT THAT WAY: this method must never become a general log tap.
bool onNotifications(LSHandle* sh, LSMessage* msg, void*) {
  LSErrorBuf e; memset(&e, 0, sizeof e);
  if (gApi.ErrorInit) gApi.ErrorInit(&e);
  bool added = false;
  if (gApi.SubscriptionAdd) {
    // NOT gated on a subscribe:true check: a caller that omitted the flag is asking
    // for something this method cannot give it any other way, and a subscription it never
    // reads costs one dead entry the hub reaps when the caller goes away. Degrading to a
    // silent no-op instead would look identical to a broken bus from the card's side.
    added = gApi.SubscriptionAdd(sh, kNotifyKey, msg, &e);
    if (!added)
      printf("[jihad-bs] luna: LSSubscriptionAdd FAILED (code=%d) — no toasts for this card\n",
             errCode(e));
  }
  if (added) rememberSubHandle(sh);
  std::string out = "{\"returnValue\":";
  out += added ? "true,\"subscribed\":true}"
               : "false,\"subscribed\":false,\"errorText\":\"notifications unavailable\"}";
  if (gApi.MessageReply) gApi.MessageReply(sh, msg, out.c_str(), &e);
  if (gApi.ErrorFree) gApi.ErrorFree(&e);
  return true;
}

// The NotificationSink the engine posts through. Fire-and-forget by construction: it formats
// one JSON line and hands it to the hub. LSSubscriptionReply does not wait for any subscriber
// to read it, and with no subscribers at all it is a cheap no-op — so a message raised deep
// inside an engine operation (ClearCookies, an add-on install) cannot stall that operation,
// which is the whole difference from the msgDialog* path.
class LunaNotifier final : public jihad::NotificationSink {
 public:
  void OnNotification(const char* category, const char* text) override {
    if (!gApi.SubscriptionReply) return;
    std::string payload = "{\"category\":";
    appendJsonString(payload, category ? category : "info");
    payload += ",\"text\":";
    appendJsonString(payload, text ? text : "");
    payload += "}";
    LSErrorBuf e; memset(&e, 0, sizeof e);
    if (gApi.ErrorInit) gApi.ErrorInit(&e);
    int sent = 0;
    for (LSHandle* h : gSubHandles) {
      if (!h) continue;
      if (gApi.SubscriptionReply(h, kNotifyKey, payload.c_str(), &e)) ++sent;
    }
    if (gApi.ErrorFree) gApi.ErrorFree(&e);
    // One line, and only one, whichever way it went: this is how "the toast never appeared"
    // is split into "the daemon never raised it" and "the card never drew it", which is
    // otherwise unattributable across two processes.
    printf("[jihad-bs] notify: %s: %s (%s)\n", category ? category : "info", text ? text : "",
           sent ? "pushed" : "no subscribers");
  }
};
LunaNotifier gNotifier;

// Terminated with a fully-zeroed entry — all THREE fields, or the walk reads past the end.
//
// TWO TABLES, DELIBERATELY DIFFERENT (least privilege). The public bus is reachable by ANY
// app on the device, not just our card, so only what the card actually calls goes there:
//   - clearCache / clearCookies: app/source/Browser.js calls both from the card. They are
//     destructive only to OUR OWN profile (worst case another app logs the user out of sites
//     in Jihad), and without them on the public bus the shipped buttons do nothing at all.
//   - getChromePrefs is PRIVATE-ONLY: it returns the user's home page and shortcut list, and
//     no card needs it any more — the settings page reaches the shells through the url
//     fragment (cavekit-preferences-ui.md R5, device-verified 2026-08-06). Keeping it off the
//     public bus means an arbitrary app cannot read those back. It stays registered privately
//     so `luna-send` can still read it when debugging.
//   - notifications is PUBLIC because the card is the consumer and cards are on the public
//     bus; see the long note on onNotifications for what that does and does not expose. Also
//     private, so `luna-send -i` can watch the stream while debugging.
//   - getFlash / enableFlash / disableFlash are PUBLIC for the card's Flash switch; the two
//     that change the value accept only this variant's own card (see callerIsOwnCard). They
//     are not on the private table: a debugging `luna-send` has no app id to be checked.
LSMethod gMethodsPublic[] = {
  { "clearCache",     onClearCache,     0 },
  { "clearCookies",   onClearCookies,   0 },
  { "clearThumbnails", onClearThumbnails, 0 },
  { "deleteThumbnail", onDeleteThumbnail, 0 },
  { "disableFlash",   onDisableFlash,   0 },
  { "enableFlash",    onEnableFlash,    0 },
  { "getFlash",       onGetFlash,       0 },
  { "notifications",  onNotifications,  0 },
  { "saveThumbnail",  onSaveThumbnail,  0 },
  { 0, 0, 0 },
};
LSMethod gMethodsPrivate[] = {
  { "clearCache",     onClearCache,     0 },
  { "clearCookies",   onClearCookies,   0 },
  { "getChromePrefs", onGetChromePrefs, 0 },
  { "getFlash",       onGetFlash,       0 },
  { "notifications",  onNotifications,  0 },
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
  gApi.RegisterPalmService         = (fn_LSRegisterPalmService)
                                       dlsym(gApi.lib, "LSRegisterPalmService");
  gApi.PalmServiceRegisterCategory = (fn_LSPalmServiceRegisterCategory)
                                       dlsym(gApi.lib, "LSPalmServiceRegisterCategory");
  gApi.GmainAttachPalmService      = (fn_LSGmainAttachPalmService)
                                       dlsym(gApi.lib, "LSGmainAttachPalmService");
  gApi.UnregisterPalmService       = (fn_LSUnregisterPalmService)
                                       dlsym(gApi.lib, "LSUnregisterPalmService");
  // The subscription slice (gre-widgets R5). NOT in the hard requirement below: a device whose
  // liblunaservice.so lacks these keeps clearCache/clearCookies/getChromePrefs and simply has
  // no toast channel, which is a smaller loss than refusing to serve anything at all.
  gApi.SubscriptionAdd       = (fn_LSSubscriptionAdd)      dlsym(gApi.lib, "LSSubscriptionAdd");
  gApi.SubscriptionReply     = (fn_LSSubscriptionReply)    dlsym(gApi.lib, "LSSubscriptionReply");
  // Optional: without it the Flash switch can be read but not changed from a card.
  gApi.MessageGetApplicationID = (fn_LSMessageGetApplicationID)
                                   dlsym(gApi.lib, "LSMessageGetApplicationID");
  // Optional: only deleteThumbnail reads its request.
  gApi.MessageGetPayload = (fn_LSMessageGetPayload) dlsym(gApi.lib, "LSMessageGetPayload");
  if (!gApi.SubscriptionAdd || !gApi.SubscriptionReply) {
    // Named individually because "the toasts do not work" is otherwise a hunt across two
    // processes, and this is the one line that settles it at the daemon end.
    printf("[jihad-bs] luna: no %s%s%s — the non-blocking message channel is OFF (dialogs and "
           "the other methods are unaffected)\n",
           gApi.SubscriptionAdd ? "" : "LSSubscriptionAdd",
           (!gApi.SubscriptionAdd && !gApi.SubscriptionReply) ? "/" : "",
           gApi.SubscriptionReply ? "" : "LSSubscriptionReply");
  }
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

// One registration attempt. `verbose` is false on retries, so a role that never appears costs one
// failure line, not one per attempt.
static bool startOnce(const std::string& serviceName, GMainLoop* loop, bool verbose) {
  if (!loop) return false;
  if (!loadApi()) return false;

  LSErrorBuf err; memset(&err, 0, sizeof err);
  if (gApi.ErrorInit) gApi.ErrorInit(&err);

  // Preferred path: register on BOTH buses, because the consumer is an app card and cards are
  // on the PUBLIC bus. Falls through to the private-only LSRegister below if this device's
  // liblunaservice.so lacks the palm-service family (it does not — all four symbols verified
  // present on the TouchPad — but a missing symbol must degrade, not crash).
  if (gApi.RegisterPalmService && gApi.PalmServiceRegisterCategory &&
      gApi.GmainAttachPalmService) {
    if (!gApi.RegisterPalmService(serviceName.c_str(), &gPalm, &err)) {
      if (verbose)
        printf("[jihad-bs] luna: LSRegisterPalmService('%s') FAILED (code=%d) — invalid name (no "
               "hyphens allowed) or no matching role in /usr/share/ls2/roles/{pub,prv}\n",
               serviceName.c_str(), errCode(err));
      if (gApi.ErrorFree) gApi.ErrorFree(&err);
      gPalm = nullptr;
      return false;
    }
    if (!gApi.PalmServiceRegisterCategory(gPalm, "/", gMethodsPublic, gMethodsPrivate,
                                          nullptr, nullptr, &err)) {
      printf("[jihad-bs] luna: LSPalmServiceRegisterCategory FAILED (code=%d)\n", errCode(err));
      if (gApi.ErrorFree) gApi.ErrorFree(&err);
      LunaServiceStop();
      return false;
    }
    if (!gApi.GmainAttachPalmService(gPalm, loop, &err)) {
      printf("[jihad-bs] luna: LSGmainAttachPalmService FAILED (code=%d)\n", errCode(err));
      if (gApi.ErrorFree) gApi.ErrorFree(&err);
      LunaServiceStop();
      return false;
    }
    if (gApi.ErrorFree) gApi.ErrorFree(&err);
    // The channel is armed only once the category is registered and attached: an engine
    // message posted before that has nowhere to go, and a sink installed early would report
    // "no subscribers" for a window in which there could not have been any.
    if (gApi.SubscriptionAdd && gApi.SubscriptionReply) SetNotificationSink(&gNotifier);
    printf("[jihad-bs] luna: serving palm://%s/ — public{clearCache,clearCookies,notifications,"
           "getFlash,enableFlash,disableFlash,saveThumbnail,deleteThumbnail,clearThumbnails} "
           "private{clearCache,clearCookies,getChromePrefs,getFlash,notifications} (toast channel %s)\n",
           serviceName.c_str(),
           (gApi.SubscriptionAdd && gApi.SubscriptionReply) ? "ON" : "OFF");
    return true;
  }
  printf("[jihad-bs] luna: no LSRegisterPalmService — falling back to the PRIVATE bus only; "
         "the app card will not be able to call this service\n");

  if (!gApi.Register(serviceName.c_str(), &gHandle, &err)) {
    // Two causes, and the message names both because guessing one wasted a device cycle:
    //  - an INVALID NAME (a hyphen anywhere is fatal — see LunaServiceNameFor), or
    //  - a missing/incorrect role file: the hub refuses a name the process may not own, and
    //    the role is keyed on the loader path (/proc/pid/exe is ld-2.23.so, NOT
    //    jihad-browserserver, because the job execs through the bundled loader).
    if (verbose)
      printf("[jihad-bs] luna: LSRegister('%s') FAILED (code=%d) — invalid name (no hyphens "
             "allowed) or no matching role in /usr/share/ls2/roles\n",
             serviceName.c_str(), errCode(err));
    if (gApi.ErrorFree) gApi.ErrorFree(&err);
    gHandle = nullptr;
    return false;
  }
  if (!gApi.RegisterCategory(gHandle, "/", gMethodsPrivate, nullptr, nullptr, &err)) {
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
  if (gApi.SubscriptionAdd && gApi.SubscriptionReply) SetNotificationSink(&gNotifier);
  // Private-bus only: a card cannot reach this, so the toast channel is armed but will never
  // have a card subscriber. Left armed anyway so `luna-send -i` can still watch the stream,
  // which is the only diagnostic available on a device that lands in this fallback.
  printf("[jihad-bs] luna: serving palm://%s/{clearCache,clearCookies,getChromePrefs,"
         "notifications} on the PRIVATE bus only\n", serviceName.c_str());
  return true;
}

// ── REGISTRATION IS RETRIED (cavekit-device-build.md R4, 2026-10-01) ─────────────────────────
// It used to be attempted exactly once, at daemon start. The hub only grants a name whose role
// file it has READ, and it reads role files at its own startup or on a rescan — so a daemon
// started before the hub knows the role (first install: postinst writes the role and starts the
// job; a hub restart racing a respawn; a role file pushed after the daemon) got -1027 and then
// ran for its whole life with no service: clearCache/clearCookies from the card, getChromePrefs
// and every toast silently dead until someone restarted it by hand. Observed on webOS CE 3.1.0
// on 2026-09-27.
//
// The retry makes the daemon converge by itself once the hub learns the role. Backoff 5 s
// doubling to 60 s, for about ten minutes: long enough to cover an install and the hub rescan
// that follows it, bounded so a daemon that genuinely has no role (a hand-launched test) does
// not poll the bus for ever. One line when it starts retrying, one when it succeeds or gives up.
namespace {
GSource*    gRetrySrc = nullptr;
GMainLoop*  gRetryLoop = nullptr;
std::string gRetryName;
int         gRetryAttempt = 0;
const int   kRetryMaxAttempts = 14;          // 5+10+20+40+60*10 s ≈ 11 min

gboolean retryRegister(gpointer);
void scheduleRetry() {
  int delay = 5 << (gRetryAttempt < 4 ? gRetryAttempt : 4);
  if (delay > 60) delay = 60;
  gRetrySrc = g_timeout_source_new_seconds(delay);
  g_source_set_callback(gRetrySrc, retryRegister, nullptr, nullptr);
  g_source_attach(gRetrySrc, g_main_loop_get_context(gRetryLoop));
}
gboolean retryRegister(gpointer) {
  g_source_unref(gRetrySrc); gRetrySrc = nullptr;     // one-shot: returning FALSE destroys it
  ++gRetryAttempt;
  if (jihad::startOnce(gRetryName, gRetryLoop, /*verbose*/false)) {
    printf("[jihad-bs] luna: registered on retry %d — the hub has the role now\n", gRetryAttempt);
  } else if (gRetryAttempt >= kRetryMaxAttempts) {
    printf("[jihad-bs] luna: still not registered after %d retries — giving up; no Luna service "
           "(clear cache/cookies and toasts are OFF) until this daemon is restarted\n",
           gRetryAttempt);
  } else {
    scheduleRetry();
  }
  return FALSE;
}
}  // namespace

void LunaServiceSetOwnAppId(const std::string& appId) { gOwnAppId = appId; }
void LunaServiceSetThumbnailFn(LunaThumbnailFn fn) { gThumbnailFn = fn; }

bool LunaServiceStart(const std::string& serviceName, GMainLoop* loop) {
  if (startOnce(serviceName, loop, /*verbose*/true)) return true;
  // Retry only when there is a bus library to retry with; "no liblunaservice.so" (the desktop
  // build) is permanent.
  if (loop && gApi.lib && (gApi.RegisterPalmService || gApi.Register)) {
    gRetryName = serviceName; gRetryLoop = loop; gRetryAttempt = 0;
    printf("[jihad-bs] luna: will retry the registration (5 s, backing off to 60 s)\n");
    scheduleRetry();
  }
  return false;
}

void LunaServiceStop() {
  if (gRetrySrc) { g_source_destroy(gRetrySrc); g_source_unref(gRetrySrc); gRetrySrc = nullptr; }
  // Clear the sink FIRST: after this point PostNotification takes its no-channel path, so an
  // engine message raised during shutdown cannot reach a handle that is about to be freed.
  SetNotificationSink(nullptr);
  gSubHandles[0] = gSubHandles[1] = nullptr;
  LSErrorBuf err; memset(&err, 0, sizeof err);
  if (gPalm) {
    if (gApi.UnregisterPalmService) {
      if (gApi.ErrorInit) gApi.ErrorInit(&err);
      gApi.UnregisterPalmService(gPalm, &err);
      if (gApi.ErrorFree) gApi.ErrorFree(&err);
    }
    gPalm = nullptr;
  }
  if (gHandle) {
    if (gApi.Unregister) {
      if (gApi.ErrorInit) gApi.ErrorInit(&err);
      gApi.Unregister(gHandle, &err);
      if (gApi.ErrorFree) gApi.ErrorFree(&err);
    }
    gHandle = nullptr;
  }
}

}  // namespace jihad
