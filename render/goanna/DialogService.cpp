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

bool InstallDialogService() {
  static bool installed = false;
  if (installed) return true;
  nsCOMPtr<nsIComponentRegistrar> reg;
  if (NS_FAILED(NS_GetComponentRegistrar(getter_AddRefs(reg))) || !reg) return false;
  nsresult rv = reg->RegisterFactory(kJihadPrompterCID, "Jihad Prompter",
                                     kPrompterContract, &Singleton());
  installed = NS_SUCCEEDED(rv);
  return installed;
}

void SetDialogSink(DialogSink* sink) { gSink = sink; }

} // namespace jihad
