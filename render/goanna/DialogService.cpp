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
#include "nsIComponentRegistrar.h"
#include "nsXPCOM.h"              // NS_GetComponentRegistrar
#include "mozilla/RefCountType.h" // MozExternalRefCountType
#include <cstring>                // strcmp (observer topic)
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
                            public nsIPrompt {
 public:
  // nsISupports (hand-rolled: static singleton, no real refcount).
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aResult) override;
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override { return 2; }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override { return 1; }

  NS_DECL_NSIFACTORY
  NS_DECL_NSIPROMPTFACTORY
  NS_DECL_NSIPROMPT

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
                         const char16_t*, const char16_t*, const char16_t*,
                         const char16_t*, bool* checkValue, int32_t* _retval) {
  if (checkValue) *checkValue = false;
  DialogReply r; Dispatch(DialogKind::Confirm, text, &r);
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
    nsAutoString host, names;
    bag->GetPropertyAsAString(NS_LITERAL_STRING("host"), host);
    bag->GetPropertyAsAString(NS_LITERAL_STRING("names"), names);
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
    if (!host.IsEmpty()) {
      text.AssignLiteral("Allow ");
      text.Append(host);
      text.AppendLiteral(" to install add-on: ");
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
  }
  return installed;
}

void SetDialogSink(DialogSink* sink) {
  gSink = sink;
  // Shutdown symmetry (review 2026-08-02 F13): the XPI observer holds no state, but an
  // observer left registered across XPCOM teardown is a needless dangling registration.
  // Clearing the sink at shutdown is the existing lifecycle hook — drop the observer with it.
  if (!sink) {
    nsCOMPtr<nsIObserverService> obs =
        do_GetService("@mozilla.org/observer-service;1");
    if (obs) obs->RemoveObserver(&XpiObserver(), "jihad-xpi-confirm");
  }
}

} // namespace jihad
