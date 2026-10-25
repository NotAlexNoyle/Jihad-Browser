/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — JS dialog interception. See DialogService.h.
 */
#include "DialogService.h"

#include "nsCOMPtr.h"
#include "nsStringGlue.h"
#include "nsIFactory.h"
#include "nsIPromptFactory.h"
#include "nsIPrompt.h"
#include "nsIAuthPrompt2.h"       // HTTP authentication challenges (necko asks for this, not nsIPrompt)
#include "nsIAuthInformation.h"
#include "nsIChannel.h"
#include "nsIProxiedChannel.h"    // a 407 names the proxy (proxies-certs R5)
#include "nsIProxyInfo.h"
#include "nsIURI.h"
#include "nsIComponentRegistrar.h"
#include "nsXPCOM.h"              // NS_GetComponentRegistrar
#include "mozilla/RefCountType.h" // MozExternalRefCountType
#include <cstring>                // strcmp (observer topic)
#include <dlfcn.h>                // dlsym (SSL_ClearSessionCache, the TLS sessions observer)
#include "nsIObserver.h"          // XPI web-install confirm round-trip (R3)
#include "nsIObserverService.h"
#include "nsIPropertyBag2.h"
#include "nsIWritablePropertyBag2.h"
#include "nsServiceManagerUtils.h" // do_GetService

namespace jihad {

// Process-wide sink (not owned) + a fresh CID for our prompter component.
static DialogSink* gSink = nullptr;

// {c1a7e6d2-3b44-4f8a-9e21-0abb11774203}
#define JIHAD_PROMPTER_CID \
  { 0xc1a7e6d2, 0x3b44, 0x4f8a, \
    { 0x9e, 0x21, 0x0a, 0xbb, 0x11, 0x77, 0x42, 0x03 } }
static const nsCID kJihadPrompterCID = JIHAD_PROMPTER_CID;
static const char* kPrompterContract = "@mozilla.org/prompter;1";

// One static singleton is the component factory, the prompt factory, and the
// prompt itself. It never really refcounts (static storage, process lifetime).
class JihadPrompter final : public nsIFactory,
                            public nsIPromptFactory,
                            public nsIPrompt,
                            public nsIAuthPrompt2 {
 public:
  // nsISupports (hand-rolled: static singleton, no real refcount).
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aResult) override;
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override { return 2; }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override { return 1; }

  NS_DECL_NSIFACTORY
  NS_DECL_NSIPROMPTFACTORY
  NS_DECL_NSIPROMPT
  NS_DECL_NSIAUTHPROMPT2

 private:
  // Route one dialog to the sink and read back the reply.
  static void Dispatch(DialogKind kind, const char16_t* text, DialogReply* reply);
};

void JihadPrompter::Dispatch(DialogKind kind, const char16_t* text,
                             DialogReply* reply) {
  DialogReply local;
  if (!reply) reply = &local;
  reply->accept = false;                 // default deny for confirm/prompt
  NS_ConvertUTF16toUTF8 utf8(text ? nsDependentString(text) : EmptyString());
  if (gSink) gSink->OnDialog(kind, utf8.get(), reply);
}

NS_IMETHODIMP
JihadPrompter::QueryInterface(const nsIID& aIID, void** aResult) {
  if (!aResult) return NS_ERROR_NULL_POINTER;
  if (aIID.Equals(NS_GET_IID(nsISupports)) ||
      aIID.Equals(NS_GET_IID(nsIFactory))) {
    *aResult = static_cast<nsIFactory*>(this);
  } else if (aIID.Equals(NS_GET_IID(nsIPromptFactory))) {
    *aResult = static_cast<nsIPromptFactory*>(this);
  } else if (aIID.Equals(NS_GET_IID(nsIPrompt))) {
    *aResult = static_cast<nsIPrompt*>(this);
  } else if (aIID.Equals(NS_GET_IID(nsIAuthPrompt2))) {
    *aResult = static_cast<nsIAuthPrompt2*>(this);
  } else {
    *aResult = nullptr;
    return NS_NOINTERFACE;
  }
  AddRef();
  return NS_OK;
}

// --- nsIFactory: the component manager instantiates the service through us ---
NS_IMETHODIMP
JihadPrompter::CreateInstance(nsISupports* aOuter, const nsIID& aIID, void** aResult) {
  if (aOuter) return NS_ERROR_NO_AGGREGATION;
  return QueryInterface(aIID, aResult);
}
NS_IMETHODIMP JihadPrompter::LockFactory(bool) { return NS_OK; }

// --- nsIPromptFactory: hand back ourselves as the nsIPrompt for any window ---
NS_IMETHODIMP
JihadPrompter::GetPrompt(mozIDOMWindowProxy*, const nsIID& aIID, void** aResult) {
  return QueryInterface(aIID, aResult);
}

// --- nsIPrompt: capture content dialogs, answer from the sink ---
NS_IMETHODIMP
JihadPrompter::Alert(const char16_t*, const char16_t* text) {
  Dispatch(DialogKind::Alert, text, nullptr);
  return NS_OK;
}
NS_IMETHODIMP
JihadPrompter::AlertCheck(const char16_t*, const char16_t* text,
                          const char16_t*, bool* checkValue) {
  if (checkValue) *checkValue = false;
  Dispatch(DialogKind::Alert, text, nullptr);
  return NS_OK;
}
NS_IMETHODIMP
JihadPrompter::Confirm(const char16_t*, const char16_t* text, bool* _retval) {
  DialogReply r; Dispatch(DialogKind::Confirm, text, &r);
  if (_retval) *_retval = r.accept;
  return NS_OK;
}
NS_IMETHODIMP
JihadPrompter::ConfirmCheck(const char16_t*, const char16_t* text,
                            const char16_t*, bool* checkValue, bool* _retval) {
  if (checkValue) *checkValue = false;
  DialogReply r; Dispatch(DialogKind::Confirm, text, &r);
  if (_retval) *_retval = r.accept;
  return NS_OK;
}
NS_IMETHODIMP
JihadPrompter::ConfirmEx(const char16_t*, const char16_t* text, uint32_t,
                         const char16_t* button0Title, const char16_t* button1Title, const char16_t*,
                         const char16_t*, bool* checkValue, int32_t* _retval) {
  if (checkValue) *checkValue = false;
  // The card's confirm has OK and Cancel and nothing else. A prompt that names its buttons — the
  // slow-script dialog's "Stop script" / "Continue" (by finger 2026-10-25: a user reads "a script on
  // this page may be busy … OK / Cancel" and cannot tell which stops it) — says in its text which
  // card button is which.
  nsString t(text ? text : u"");
  if (button0Title && *button0Title && button1Title && *button1Title) {
    // The card shows the text with its whitespace collapsed: one sentence, not lines.
    t.AppendLiteral(" (OK = "); t.Append(button0Title);
    t.AppendLiteral(", Cancel = "); t.Append(button1Title); t.AppendLiteral(")");
  }
  DialogReply r; Dispatch(DialogKind::Confirm, t.get(), &r);
  if (_retval) *_retval = r.accept ? 0 : 1;   // 0 = button0 (OK), 1 = Cancel
  return NS_OK;
}
NS_IMETHODIMP
JihadPrompter::Prompt(const char16_t*, const char16_t* text, char16_t** value,
                      const char16_t*, bool* checkValue, bool* _retval) {
  if (checkValue) *checkValue = false;
  DialogReply r; Dispatch(DialogKind::Prompt, text, &r);
  if (_retval) *_retval = r.accept;
  if (value && r.accept) {
    *value = ToNewUnicode(NS_ConvertUTF8toUTF16(r.promptValue.c_str()));
  }
  return NS_OK;
}
NS_IMETHODIMP
JihadPrompter::PromptPassword(const char16_t*, const char16_t* text,
                              char16_t**, const char16_t*, bool*, bool* _retval) {
  DialogReply r; Dispatch(DialogKind::Prompt, text, &r);
  if (_retval) *_retval = false;   // never surrender a password headless
  return NS_OK;
}
NS_IMETHODIMP
JihadPrompter::PromptUsernameAndPassword(const char16_t*, const char16_t* text,
                                         char16_t**, char16_t**,
                                         const char16_t*, bool*, bool* _retval) {
  DialogReply r; Dispatch(DialogKind::Prompt, text, &r);
  if (_retval) *_retval = false;
  return NS_OK;
}
NS_IMETHODIMP
JihadPrompter::Select(const char16_t*, const char16_t* text, uint32_t,
                      const char16_t**, int32_t* outSelection, bool* _retval) {
  if (outSelection) *outSelection = -1;
  DialogReply r; Dispatch(DialogKind::Confirm, text, &r);
  if (_retval) *_retval = false;
  return NS_OK;
}

// --- nsIAuthPrompt2: an HTTP / proxy authentication challenge ---
// Necko does not go through nsIPrompt for a 401/407: it asks the docshell for nsIAuthPrompt2,
// which the window watcher fetches from this same factory. Until this existed the factory
// answered NS_NOINTERFACE, so a challenge raised no dialog at all and the user was shown the
// server's 401 page — measured on the device 2026-10-01 (cavekit-browser-services.md R3 had the
// box checked on the strength of PromptUsernameAndPassword, which nothing in the HTTP path
// calls). The card's user-name/password dialog and its three-field reply were already there.
// The async form is declined so necko falls back to the synchronous one, which is the shape
// every other dialog here has: block on the card, bounded by the dialog deadline.
NS_IMETHODIMP
JihadPrompter::PromptAuth(nsIChannel* aChannel, uint32_t, nsIAuthInformation* aAuthInfo,
                          bool* _retval) {
  if (_retval) *_retval = false;
  if (!aAuthInfo) return NS_ERROR_INVALID_ARG;
  nsAutoString text;
  nsAutoCString hostPort;
  uint32_t flags = 0;
  aAuthInfo->GetFlags(&flags);
  if (flags & nsIAuthInformation::AUTH_PROXY) {
    // A 407: the login is the proxy's, not the site's, and the channel's URI is the site
    // (cavekit-proxies-certs.md R5). Name the proxy, so a site never seems to ask for it. The
    // text starts "[proxy] ", which no site's host:port can (a site named "proxy" could pass for
    // "proxy "; the inspector, round 24): the Enyo and Mochi shells then say "The proxy …
    // requires a username and password" instead of "The server …"; Mojo's dialog is the system
    // framework's own and shows the text as it is.
    nsCOMPtr<nsIProxiedChannel> proxied = do_QueryInterface(aChannel);
    nsCOMPtr<nsIProxyInfo> pi;
    if (proxied) proxied->GetProxyInfo(getter_AddRefs(pi));
    nsAutoCString phost; int32_t pport = -1;
    if (pi) { pi->GetHost(phost); pi->GetPort(&pport); }
    text.AppendLiteral("[proxy] ");
    if (!phost.IsEmpty()) {
      hostPort = phost;
      if (pport > 0) { hostPort.Append(':'); hostPort.AppendInt(pport); }
    } else {
      hostPort.AssignLiteral("in use");
    }
  } else if (aChannel) {
    nsCOMPtr<nsIURI> uri; aChannel->GetURI(getter_AddRefs(uri));
    if (uri) uri->GetHostPort(hostPort);
  }
  nsAutoString realm; aAuthInfo->GetRealm(realm);
  text.Append(NS_ConvertUTF8toUTF16(hostPort));
  if (!realm.IsEmpty()) { text.AppendLiteral(" - "); text.Append(realm); }
  DialogReply r; Dispatch(DialogKind::Auth, text.get(), &r);
  if (!r.accept) return NS_OK;
  aAuthInfo->SetUsername(NS_ConvertUTF8toUTF16(r.promptValue.c_str()));
  aAuthInfo->SetPassword(NS_ConvertUTF8toUTF16(r.password.c_str()));
  if (_retval) *_retval = true;
  return NS_OK;
}
NS_IMETHODIMP
JihadPrompter::AsyncPromptAuth(nsIChannel*, nsIAuthPromptCallback*, nsISupports*, uint32_t,
                               nsIAuthInformation*, nsICancelable** _retval) {
  if (_retval) *_retval = nullptr;
  return NS_ERROR_NOT_IMPLEMENTED;
}

// --- installation ---
static JihadPrompter& Singleton() { static JihadPrompter sPrompter; return sPrompter; }

// --- XPI web-install confirm (browser-services R3) -------------------------------------------
// The bundled JS component components/jihadInstallPrompt.js takes the toolkit's
// "@mozilla.org/addons/web-install-prompt;1" hook and raises a SYNCHRONOUS
// "jihad-xpi-confirm" observer notification carrying a property bag
// (host, names, accept=false). This observer routes it through the same DialogSink
// as every other content dialog — so each card answers in its own framework's
// idiom — and writes the decision back into the bag before returning. Default
// stays DENY on any missing piece: no sink, no card, wrong bag type.
class JihadXpiConfirmObserver final : public nsIObserver {
 public:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aResult) override {
    if (!aResult) return NS_ERROR_NULL_POINTER;
    if (aIID.Equals(NS_GET_IID(nsISupports)) ||
        aIID.Equals(NS_GET_IID(nsIObserver))) {
      *aResult = static_cast<nsIObserver*>(this);
      AddRef();
      return NS_OK;
    }
    *aResult = nullptr;
    return NS_NOINTERFACE;
  }
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override { return 2; }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override { return 1; }

  NS_IMETHOD Observe(nsISupports* aSubject, const char* aTopic,
                     const char16_t* /*aData*/) override {
    if (!aTopic || strcmp(aTopic, "jihad-xpi-confirm") != 0) return NS_OK;
    nsCOMPtr<nsIPropertyBag2> bag = do_QueryInterface(aSubject);
    nsCOMPtr<nsIWritablePropertyBag2> wbag = do_QueryInterface(aSubject);
    if (!bag || !wbag) return NS_OK;               // wrong subject: leave accept=false
    nsAutoString host, names, kind;
    bag->GetPropertyAsAString(NS_LITERAL_STRING("host"), host);
    bag->GetPropertyAsAString(NS_LITERAL_STRING("names"), names);
    bag->GetPropertyAsAString(NS_LITERAL_STRING("kind"), kind);   // "" = an add-on; "theme" = a persona
    // Sanitize CONTENT-CONTROLLED text (review 2026-08-02 F12): an install.rdf name can
    // embed "\nfrom addons.example.org" to forge the origin line, or be multi-KB to push
    // the real origin off the card dialog. Control chars become spaces, both strings get
    // a hard budget, and the HOST comes FIRST so content can never displace it.
    struct Clean {
      static void Run(nsAutoString& s, uint32_t maxLen) {
        for (uint32_t i = 0; i < s.Length(); ++i) {
          char16_t c = s.CharAt(i);
          if (c < 0x20 || c == 0x7f) s.SetCharAt(' ', i);
        }
        if (s.Length() > maxLen) {
          s.Truncate(maxLen - 3);
          s.AppendLiteral("...");
        }
      }
    };
    Clean::Run(host, 80);
    Clean::Run(names, 160);
    nsAutoString text;
    // One literal per call: AssignLiteral/AppendLiteral take array references, and a ?: of two
    // literals of different lengths is a plain pointer (this tree has a char* overload, an
    // engine update may not — Codex 2026-10-06).
    const bool theme = kind.EqualsLiteral("theme");
    if (!host.IsEmpty()) {
      text.AssignLiteral("Allow ");
      text.Append(host);
      if (theme) text.AppendLiteral(" to change the browser's look to the Persona: ");
      else text.AppendLiteral(" to install add-on: ");
    } else if (theme) {
      text.AssignLiteral("Use the Persona: ");
    } else {
      text.AssignLiteral("Install add-on: ");
    }
    text.Append(names);
    DialogReply reply;
    reply.accept = false;                          // default deny (unattended install)
    NS_ConvertUTF16toUTF8 utf8(text);
    // Log the prompt: on device this is the only record that a web install was offered at
    // all, and while bringing the path up it is what distinguishes "the prompt was reached
    // and denied" from "something upstream cancelled the install before it" — both of which
    // surface to the page as the same USER_CANCELLED status.
    fprintf(stderr, "[jihad-bs] xpi confirm: %s (sink=%s)\n",
            utf8.get(), gSink ? "yes" : "NONE — denying");
    if (gSink) gSink->OnDialog(DialogKind::Confirm, utf8.get(), &reply);
    fprintf(stderr, "[jihad-bs] xpi confirm -> %s\n", reply.accept ? "ACCEPT" : "deny");
    wbag->SetPropertyAsBool(NS_LITERAL_STRING("accept"), reply.accept);
    return NS_OK;
  }
};

static JihadXpiConfirmObserver& XpiObserver() {
  static JihadXpiConfirmObserver o;
  return o;
}

// ── the non-blocking channel (gre-widgets R5) ────────────────────────────────────────────
// See DialogService.h for why this exists next to the blocking one.
static NotificationSink* gNotify = nullptr;

void SetNotificationSink(NotificationSink* sink) { gNotify = sink; }

static unsigned gPageCopied = 0;
unsigned PageCopiedCount() { return gPageCopied; }

void PostNotification(const char* category, const char* text) {
  const char* cat = (category && *category) ? category : "info";
  const char* msg = text ? text : "";
  if (!gNotify) {
    // The degrade path, and it is deliberately ONE line rather than silence: on desktop this
    // is the only record that the message was raised at all, and on device an unexpected
    // "no channel" here is how a broken Luna registration announces itself. It is a log
    // write, not a dialog — nothing blocks and the caller carries on either way.
    fprintf(stderr, "[jihad-bs] notify (no channel): %s: %s\n", cat, msg);
    return;                       // nothing was handed to any card: PageCopiedCount stays
  }
  // "page-copied" counts as a hand-over only if some card was actually told the name.
  const int delivered = gNotify->OnNotification(cat, msg);
  if (strcmp(cat, "page-copied") == 0 && delivered != 0) ++gPageCopied;
}

// The JS-facing door to PostNotification. Bundled components (components/*.js) raise
// "jihad-notify" with a property bag carrying `category` and `text`.
//
// UNLIKE "jihad-xpi-confirm" this observer writes NOTHING back and answers nothing: the
// notifyObservers call returns as soon as the payload has been handed to the sink, so a JS
// component can report something without its own execution being parked on a card.
class JihadNotifyObserver final : public nsIObserver {
 public:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aResult) override {
    if (!aResult) return NS_ERROR_NULL_POINTER;
    if (aIID.Equals(NS_GET_IID(nsISupports)) ||
        aIID.Equals(NS_GET_IID(nsIObserver))) {
      *aResult = static_cast<nsIObserver*>(this);
      AddRef();
      return NS_OK;
    }
    *aResult = nullptr;
    return NS_NOINTERFACE;
  }
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override { return 2; }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override { return 1; }

  NS_IMETHOD Observe(nsISupports* aSubject, const char* aTopic,
                     const char16_t* /*aData*/) override {
    if (!aTopic || strcmp(aTopic, "jihad-notify") != 0) return NS_OK;
    nsCOMPtr<nsIPropertyBag2> bag = do_QueryInterface(aSubject);
    if (!bag) return NS_OK;
    nsAutoString category, text;
    bag->GetPropertyAsAString(NS_LITERAL_STRING("category"), category);
    bag->GetPropertyAsAString(NS_LITERAL_STRING("text"), text);
    // Same sanitising, and the same reason, as the XPI confirm above: an add-on name comes
    // out of install.rdf, i.e. from content, so it can carry control characters (which would
    // break the JSON line this becomes on the wire) or be long enough to fill the card.
    for (uint32_t i = 0; i < text.Length(); ++i) {
      char16_t c = text.CharAt(i);
      if (c < 0x20 || c == 0x7f) text.SetCharAt(' ', i);
    }
    // A toast is one short line. The printing component's results ride the same door under
    // "print-…" categories and are JSON (a list of printers), which a card parses rather than
    // shows — so those get room, and are NOT cut in the middle with an ellipsis (a truncated
    // JSON string is not JSON); anything over the limit is dropped whole.
    // The same for "theme": the lightweight theme's JSON (components/jihadPersonas.js), which a
    // card parses and paints — cut at 200 characters it parsed as nothing and the card stayed
    // unthemed (device 2026-10-06). Over its room it becomes "null": no theme, not a broken one.
    const bool isPrint = StringBeginsWith(category, NS_LITERAL_STRING("print-"));
    const bool isTheme = category.EqualsLiteral("theme");
    if (isPrint) {
      if (text.Length() > 6000) text.AssignLiteral("[]");
    } else if (isTheme) {
      if (text.Length() > 6000) text.AssignLiteral("null");
    } else if (text.Length() > 200) { text.Truncate(197); text.AppendLiteral("..."); }
    for (uint32_t i = 0; i < category.Length(); ++i) {
      char16_t c = category.CharAt(i);
      if (c < 0x20 || c == 0x7f) category.SetCharAt(' ', i);
    }
    if (category.Length() > 32) category.Truncate(32);
    NS_ConvertUTF16toUTF8 cat8(category);
    NS_ConvertUTF16toUTF8 txt8(text);
    PostNotification(cat8.get(), txt8.get());
    return NS_OK;
  }
};

static JihadNotifyObserver& NotifyObserver() {
  static JihadNotifyObserver o;
  return o;
}

// webOS's proxy setting, read again on a component's request (cavekit-proxies-certs.md R1).
// The reader lives on the Luna side of the daemon; this is only the door from JS to it.
static WebosProxyReaderFn gWebosProxyReader = nullptr;

void SetWebosProxyReader(WebosProxyReaderFn fn) { gWebosProxyReader = fn; }

class JihadWebosProxyObserver final : public nsIObserver {
 public:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aResult) override {
    if (!aResult) return NS_ERROR_NULL_POINTER;
    if (aIID.Equals(NS_GET_IID(nsISupports)) ||
        aIID.Equals(NS_GET_IID(nsIObserver))) {
      *aResult = static_cast<nsIObserver*>(this);
      AddRef();
      return NS_OK;
    }
    *aResult = nullptr;
    return NS_NOINTERFACE;
  }
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override { return 2; }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override { return 1; }

  NS_IMETHOD Observe(nsISupports*, const char* aTopic, const char16_t*) override {
    if (!aTopic || strcmp(aTopic, "jihad-webos-proxy-refresh") != 0) return NS_OK;
    if (gWebosProxyReader) gWebosProxyReader();
    else fprintf(stderr, "[jihad-bs] proxy: no Luna connection to read webOS's proxy setting\n");
    return NS_OK;
  }
};

static JihadWebosProxyObserver& WebosProxyObserver() {
  static JihadWebosProxyObserver o;
  return o;
}

// TLS sessions checked under a trust that was just taken away (cavekit-proxies-certs.md R2, R4,
// R6): components/jihadCerts.js raises "jihad-clear-tls-sessions" and this clears NSS's session
// cache, so a resumed session cannot skip the new check. The script-reachable alternative,
// nsISecretDecoderRing.logoutAndTeardown, also logs out of every token, cancels every open TLS
// socket and forgets every temporary certificate override (each Trust Once) — the inspector's
// finding, 2026-10-13. SSL_ClearSessionCache is libssl3's, which libxul already loaded; it is
// looked up rather than linked so the daemon gains no build-time NSS dependency.
class JihadTlsSessionsObserver final : public nsIObserver {
 public:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aResult) override {
    if (!aResult) return NS_ERROR_NULL_POINTER;
    if (aIID.Equals(NS_GET_IID(nsISupports)) ||
        aIID.Equals(NS_GET_IID(nsIObserver))) {
      *aResult = static_cast<nsIObserver*>(this);
      AddRef();
      return NS_OK;
    }
    *aResult = nullptr;
    return NS_NOINTERFACE;
  }
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override { return 2; }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override { return 1; }

  NS_IMETHOD Observe(nsISupports*, const char* aTopic, const char16_t*) override {
    if (!aTopic || strcmp(aTopic, "jihad-clear-tls-sessions") != 0) return NS_OK;
    typedef void (*ClearFn)(void);
    static ClearFn sClear = (ClearFn)dlsym(RTLD_DEFAULT, "SSL_ClearSessionCache");
    if (sClear) sClear();
    fprintf(stderr, "[jihad-bs] tls: session cache %s\n", sClear ? "cleared" : "NOT cleared (no SSL_ClearSessionCache)");
    return NS_OK;
  }
};

static JihadTlsSessionsObserver& TlsSessionsObserver() {
  static JihadTlsSessionsObserver o;
  return o;
}

bool InstallDialogService() {
  static bool installed = false;
  if (installed) return true;
  nsCOMPtr<nsIComponentRegistrar> reg;
  if (NS_FAILED(NS_GetComponentRegistrar(getter_AddRefs(reg))) || !reg) return false;
  nsresult rv = reg->RegisterFactory(kJihadPrompterCID, "Jihad Prompter",
                                     kPrompterContract, &Singleton());
  installed = NS_SUCCEEDED(rv);
  // The XPI confirm round-trip (R3): non-fatal if the observer service is absent —
  // the JS component then leaves accept=false and every web install is denied,
  // which is the safe default, not a broken one.
  if (installed) {
    nsCOMPtr<nsIObserverService> obs =
        do_GetService("@mozilla.org/observer-service;1");
    if (obs) obs->AddObserver(&XpiObserver(), "jihad-xpi-confirm", false);
    // The non-blocking channel's JS door (gre-widgets R5). Registered here rather than in
    // its own installer because it shares this one's lifecycle exactly and because the very
    // first consumer — components/jihadInstallPrompt.js — is force-instantiated below.
    if (obs) obs->AddObserver(&NotifyObserver(), "jihad-notify", false);
    // components/jihadProxy.js asks for webOS's proxy setting again (a card's Settings).
    if (obs) obs->AddObserver(&WebosProxyObserver(), "jihad-webos-proxy-refresh", false);
    // components/jihadCerts.js, after a trust is taken away.
    if (obs) obs->AddObserver(&TlsSessionsObserver(), "jihad-clear-tls-sessions", false);
    // R3 "clear reason": components/jihadInstallPrompt.js ALSO observes "addon-install-failed",
    // which is the ONLY notification an incompatible (appDisabled) XPI produces. That refusal
    // happens inside amWebInstallListener BEFORE any confirm prompt is reached, so nothing would
    // otherwise construct the component in time and the observer would never be registered.
    // Force it into existence here, where XPCOM is up and no install can have run yet. Safe this
    // early even though this precedes InstallAppInfoService(): the component imports only
    // XPCOMUtils.jsm and Services.jsm (never AddonManager.jsm), and its one Services.appinfo read
    // is inside observe(), which cannot run until a web install does. Non-fatal: without it the
    // add-on is still refused, the user is just not told why.
    // The printing component (components/jihadPrint.js) waits on "jihad-print-find" and
    // "jihad-print-job"; like the install prompt it is created here so its observers exist
    // before the first request. Absent means printing answers "not available", nothing worse.
    nsCOMPtr<nsISupports> printer = do_GetService("@riverstonerelay.net/jihad/print;1");
    if (!printer) fprintf(stderr, "[jihad-bs] print: component absent, printing is unavailable\n");
    // The page's half of the long-press menu (components/jihadPageCommands.js) waits on
    // "jihad-page-command". Absent means a long-press raises no menu on plain content.
    nsCOMPtr<nsISupports> pageCmds = do_GetService("@riverstonerelay.net/jihad/page-commands;1");
    if (!pageCmds) fprintf(stderr, "[jihad-bs] long-press: component absent, no page menu\n");
    // The add-ons that come with the browser (components/jihadBundledAddons.js): offered to
    // the profile once, a few seconds after start. Absent means they are simply not offered.
    nsCOMPtr<nsISupports> bundledAddons = do_GetService("@riverstonerelay.net/jihad/bundled-addons;1");
    if (!bundledAddons) fprintf(stderr, "[jihad-bs] bundled add-ons: component absent, none offered\n");
    // Lightweight themes (components/jihadPersonas.js): catches the theme sites' events the
    // pages hand up, applies through the engine's manager, tells the cards. Absent means a
    // theme site's button does nothing.
    nsCOMPtr<nsISupports> personas = do_GetService("@riverstonerelay.net/jihad/personas;1");
    if (!personas) fprintf(stderr, "[jihad-bs] themes: component absent, personas cannot be installed\n");
    // Certificates and proxies (cavekit-proxies-certs.md): components/jihadCerts.js trusts what
    // webOS added, and the Relay's CA when asked, once NSS is up ("jihad-nss-ready");
    // components/jihadProxy.js applies webOS's proxy setting when the user chose it. Absent means
    // Jihad trusts only its own roots, or does not follow webOS's proxy.
    nsCOMPtr<nsISupports> certs = do_GetService("@riverstonerelay.net/jihad/certs;1");
    if (!certs) fprintf(stderr, "[jihad-bs] certs: component absent, webOS's certificates are not trusted\n");
    // Content blocking (cavekit-content-blocking.md): components/jihadContentBlocker.js refuses
    // loads from EasyList/EasyPrivacy hosts (it is also in the content-policy category, so the
    // first load creates it even without this; created here so the lists compile off the
    // start-up path, before the first page needs them). Absent means nothing is blocked.
    // The browser's identity (cavekit-modern-web.md R3): components/jihadUserAgent.js — 52.9 by
    // default, a learned current identity for the sites whose bot check turns the page away.
    nsCOMPtr<nsISupports> identity = do_GetService("@riverstonerelay.net/jihad/user-agent;1");
    if (!identity) fprintf(stderr, "[jihad-bs] identity: component absent, no learned identities\n");
    nsCOMPtr<nsISupports> blocker = do_GetService("@riverstonerelay.net/jihad/content-blocker;1");
    if (!blocker) fprintf(stderr, "[jihad-bs] blocker: component absent, ads and trackers are not blocked\n");
    // Web compatibility shims (cavekit-modern-web.md R1/R6): components/jihadWebCompat.js — the
    // platform pieces the engine lacks, supplied to each page before its scripts run.
    nsCOMPtr<nsISupports> compat = do_GetService("@riverstonerelay.net/jihad/web-compat;1");
    if (!compat) fprintf(stderr, "[jihad-bs] compat: component absent, no web compatibility shims\n");
    nsCOMPtr<nsISupports> proxy = do_GetService("@riverstonerelay.net/jihad/proxy;1");
    if (!proxy) fprintf(stderr, "[jihad-bs] proxy: component absent, webOS's proxy setting is not followed\n");
    nsCOMPtr<nsISupports> xpiPrompt =
        do_GetService("@mozilla.org/addons/web-install-prompt;1");
    if (!xpiPrompt) {
      fprintf(stderr, "[jihad-bs] xpi: web-install-prompt component absent, an incompatible "
                      "add-on will be refused with no reason shown\n");
    }
  }
  return installed;
}

void SetDialogSink(DialogSink* sink) { gSink = sink; }

DialogSink* GetDialogSink() { return gSink; }

void ShutdownDialogService() {
  gSink = nullptr;
  // Shutdown symmetry (review 2026-08-02 F13): the observers hold no state, but one left
  // registered across XPCOM teardown is a needless dangling registration. They are dropped
  // HERE and nowhere else — see the header for what dropping them on a sink change cost.
  nsCOMPtr<nsIObserverService> obs = do_GetService("@mozilla.org/observer-service;1");
  if (obs) obs->RemoveObserver(&XpiObserver(), "jihad-xpi-confirm");
  if (obs) obs->RemoveObserver(&NotifyObserver(), "jihad-notify");
  if (obs) obs->RemoveObserver(&WebosProxyObserver(), "jihad-webos-proxy-refresh");
  if (obs) obs->RemoveObserver(&TlsSessionsObserver(), "jihad-clear-tls-sessions");
  gWebosProxyReader = nullptr;
}

} // namespace jihad
