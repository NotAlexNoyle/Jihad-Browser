/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — download / MIME-handoff interception. See DownloadService.h.
 */
#include "DownloadService.h"

#include "nsCOMPtr.h"
#include "nsStringGlue.h"
#include "nsIFactory.h"
#include "nsIHelperAppLauncherDialog.h"
#include "nsIExternalHelperAppService.h"  // nsIHelperAppLauncher
#include "nsIMIMEInfo.h"
#include "nsIURI.h"
#include "nsIComponentRegistrar.h"
#include "nsXPCOM.h"
#include "mozilla/RefCountType.h"

namespace jihad {

static DownloadSink* gSink = nullptr;

// {d0e1f2a3-4b56-4c78-9d0e-1f2a3b4c5d6e}
#define JIHAD_HELPERDIALOG_CID \
  { 0xd0e1f2a3, 0x4b56, 0x4c78, \
    { 0x9d, 0x0e, 0x1f, 0x2a, 0x3b, 0x4c, 0x5d, 0x6e } }
static const nsCID kJihadHelperDialogCID = JIHAD_HELPERDIALOG_CID;
static const char* kHelperDialogContract = "@mozilla.org/helperapplauncherdialog;1";

class JihadHelperDialog final : public nsIFactory,
                                public nsIHelperAppLauncherDialog {
 public:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aResult) override;
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override { return 2; }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override { return 1; }

  NS_DECL_NSIFACTORY
  NS_DECL_NSIHELPERAPPLAUNCHERDIALOG

 private:
  static void Report(nsIHelperAppLauncher* aLauncher);
};

void JihadHelperDialog::Report(nsIHelperAppLauncher* aLauncher) {
  if (!aLauncher) return;
  nsCString url, mime;
  nsString name;
  int64_t len = -1;
  nsCOMPtr<nsIURI> src; aLauncher->GetSource(getter_AddRefs(src));
  if (src) src->GetSpec(url);
  nsCOMPtr<nsIMIMEInfo> mi; aLauncher->GetMIMEInfo(getter_AddRefs(mi));
  if (mi) mi->GetMIMEType(mime);
  aLauncher->GetSuggestedFileName(name);
  aLauncher->GetContentLength(&len);
  if (gSink)
    gSink->OnDownload(url.get(), mime.get(),
                      NS_ConvertUTF16toUTF8(name).get(), len);
}

NS_IMETHODIMP
JihadHelperDialog::QueryInterface(const nsIID& aIID, void** aResult) {
  if (!aResult) return NS_ERROR_NULL_POINTER;
  if (aIID.Equals(NS_GET_IID(nsISupports)) ||
      aIID.Equals(NS_GET_IID(nsIFactory))) {
    *aResult = static_cast<nsIFactory*>(this);
  } else if (aIID.Equals(NS_GET_IID(nsIHelperAppLauncherDialog))) {
    *aResult = static_cast<nsIHelperAppLauncherDialog*>(this);
  } else {
    *aResult = nullptr;
    return NS_NOINTERFACE;
  }
  AddRef();
  return NS_OK;
}

// nsIFactory
NS_IMETHODIMP
JihadHelperDialog::CreateInstance(nsISupports* aOuter, const nsIID& aIID, void** aResult) {
  if (aOuter) return NS_ERROR_NO_AGGREGATION;
  return QueryInterface(aIID, aResult);
}
NS_IMETHODIMP JihadHelperDialog::LockFactory(bool) { return NS_OK; }

// nsIHelperAppLauncherDialog — capture the handoff instead of opening a window.
NS_IMETHODIMP
JihadHelperDialog::Show(nsIHelperAppLauncher* aLauncher, nsISupports*, uint32_t) {
  Report(aLauncher);
  return NS_OK;
}
NS_IMETHODIMP
JihadHelperDialog::PromptForSaveToFileAsync(nsIHelperAppLauncher* aLauncher,
                                            nsISupports*, const char16_t*,
                                            const char16_t*, bool) {
  Report(aLauncher);
  return NS_OK;
}

static JihadHelperDialog& Singleton() { static JihadHelperDialog s; return s; }

bool InstallDownloadService() {
  static bool installed = false;
  if (installed) return true;
  nsCOMPtr<nsIComponentRegistrar> reg;
  if (NS_FAILED(NS_GetComponentRegistrar(getter_AddRefs(reg))) || !reg) return false;
  nsresult rv = reg->RegisterFactory(kJihadHelperDialogCID, "Jihad Helper Dialog",
                                     kHelperDialogContract, &Singleton());
  installed = NS_SUCCEEDED(rv);
  return installed;
}

void SetDownloadSink(DownloadSink* sink) { gSink = sink; }

} // namespace jihad
