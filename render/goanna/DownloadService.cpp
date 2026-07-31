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
#include "nsIFileURL.h"
#include "nsIFile.h"
#include "nsICancelable.h"
#include "nsITransfer.h"                  // the embedding download-report hook
#include "nsIWebProgressListener.h"       // STATE_STOP
#include "nsIComponentRegistrar.h"
#include "nsISupportsImpl.h"              // NS_IMPL_ISUPPORTS
#include "nsDirectoryServiceDefs.h"       // NS_OS_TEMP_DIR
#include "nsDirectoryServiceUtils.h"      // NS_GetSpecialDirectory
#include "nsXPCOM.h"
#include "mozilla/RefPtr.h"
#include "mozilla/RefCountType.h"
#include <vector>
#include <cstdlib>
#include <cstdio>

namespace jihad {

static DownloadSink* gSink = nullptr;

// {d0e1f2a3-4b56-4c78-9d0e-1f2a3b4c5d6e}
#define JIHAD_HELPERDIALOG_CID \
  { 0xd0e1f2a3, 0x4b56, 0x4c78, \
    { 0x9d, 0x0e, 0x1f, 0x2a, 0x3b, 0x4c, 0x5d, 0x6e } }
static const nsCID kJihadHelperDialogCID = JIHAD_HELPERDIALOG_CID;
static const char* kHelperDialogContract = "@mozilla.org/helperapplauncherdialog;1";

// {b7c8d9ea-1f20-4a31-8b42-5c6d7e8f9a0b}
#define JIHAD_TRANSFER_CID \
  { 0xb7c8d9ea, 0x1f20, 0x4a31, \
    { 0x8b, 0x42, 0x5c, 0x6d, 0x7e, 0x8f, 0x9a, 0x0b } }
static const nsCID kJihadTransferCID = JIHAD_TRANSFER_CID;
// From nsITransfer.idl (NS_TRANSFER_CONTRACTID); spelled out so this TU does not
// depend on the %{C++ block's macro being exported.
static const char* kTransferContract = "@mozilla.org/transfer;1";

// ── the download the engine is performing ───────────────────────────────────
// nsExternalAppHandler creates ONE of these per download (via the
// "@mozilla.org/transfer;1" contract) and drives it with the standard
// nsIWebProgressListener2 notifications. That is the whole lifecycle:
//   Init                       -> msgDownloadStart
//   OnProgressChange64         -> msgDownloadProgress
//   OnStateChange(STATE_STOP)  -> msgDownloadFinished | msgDownloadError
// Init also hands us the nsICancelable, which is what makes the YAP
// cancelDownload command implementable (CancelDownload below).
class JihadTransfer final : public nsITransfer {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIWEBPROGRESSLISTENER
  NS_DECL_NSIWEBPROGRESSLISTENER2
  NS_DECL_NSITRANSFER

  JihadTransfer() : mDone(false) {}

  const nsCString& Url() const { return mUrl; }
  bool Done() const { return mDone; }
  // Abort this download. Cancelling the helper-app handler finishes its file
  // saver with the abort status, which comes back as OnStateChange(STATE_STOP,
  // <failure>) -> exactly one msgDownloadError and never a Finished. Safe to
  // call re-entrantly (Finish is idempotent).
  void Abort() {
    if (mDone) return;
    nsCOMPtr<nsICancelable> c = mCancelable;
    if (c) c->Cancel(NS_BINDING_ABORTED);
    // Nothing to cancel through: terminate the lifecycle ourselves so a client
    // that asked for a cancel always gets a terminal message.
    if (!c) Finish(NS_BINDING_ABORTED);
  }

 private:
  ~JihadTransfer() {}
  void Finish(nsresult aStatus);
  // Rename the "<target>.part" work file the helper-app service writes into over
  // the final target. Firefox does this move in its own nsITransfer (Download.jsm);
  // there is no other owner for it, so an embedder that supplies the contract must.
  void FinalizePartFile();

  nsCString         mUrl;
  nsCString         mMime;
  nsCOMPtr<nsIFile> mTarget;      // where the finished file must end up
  nsCOMPtr<nsIFile> mTempFile;    // "<target>.part" while the download runs
  nsCOMPtr<nsICancelable> mCancelable;
  bool              mDone;
};

NS_IMPL_ISUPPORTS(JihadTransfer, nsITransfer, nsIWebProgressListener,
                  nsIWebProgressListener2)

// Active (not yet finished) downloads, in start order. Strong refs: the entry is
// dropped in Finish(), so an in-flight download always has a live cancelable.
static std::vector<RefPtr<JihadTransfer>>* gActive = nullptr;

static void ActiveAdd(JihadTransfer* t) {
  if (!gActive) gActive = new std::vector<RefPtr<JihadTransfer>>();
  gActive->push_back(t);
}
static void ActiveRemove(JihadTransfer* t) {
  if (!gActive) return;
  for (size_t i = 0; i < gActive->size(); ++i) {
    if ((*gActive)[i].get() == t) { gActive->erase(gActive->begin() + i); return; }
  }
}

NS_IMETHODIMP
JihadTransfer::Init(nsIURI* aSource, nsIURI* aTarget, const nsAString& /*aDisplayName*/,
                    nsIMIMEInfo* aMIMEInfo, PRTime /*aStartTime*/, nsIFile* aTempFile,
                    nsICancelable* aCancelable, bool /*aIsPrivate*/) {
  if (aSource) aSource->GetSpec(mUrl);
  if (aMIMEInfo) aMIMEInfo->GetMIMEType(mMime);
  mTempFile = aTempFile;
  mCancelable = aCancelable;
  // aTarget is a file:// URI for the destination we picked in the dialog.
  nsCOMPtr<nsIFileURL> furl = do_QueryInterface(aTarget);
  if (furl) furl->GetFile(getter_AddRefs(mTarget));

  ActiveAdd(this);
  printf("[jihad-dl] start %s (mime=%s)\n", mUrl.get(), mMime.get());
  if (gSink) gSink->OnDownloadStart(mUrl.get());
  return NS_OK;
}

NS_IMETHODIMP JihadTransfer::SetSha256Hash(const nsACString&)   { return NS_OK; }
NS_IMETHODIMP JihadTransfer::SetSignatureInfo(nsIArray*)        { return NS_OK; }
NS_IMETHODIMP JihadTransfer::SetRedirects(nsIArray*)            { return NS_OK; }

NS_IMETHODIMP
JihadTransfer::OnProgressChange64(nsIWebProgress*, nsIRequest*,
                                  int64_t, int64_t,
                                  int64_t aCurTotalProgress,
                                  int64_t aMaxTotalProgress) {
  if (mDone) return NS_OK;
  if (gSink) gSink->OnDownloadProgress(mUrl.get(), aCurTotalProgress, aMaxTotalProgress);
  return NS_OK;
}

NS_IMETHODIMP
JihadTransfer::OnStateChange(nsIWebProgress*, nsIRequest*, uint32_t aStateFlags,
                             nsresult aStatus) {
  if (aStateFlags & nsIWebProgressListener::STATE_STOP) Finish(aStatus);
  return NS_OK;
}

// Unused halves of the listener surface.
NS_IMETHODIMP JihadTransfer::OnProgressChange(nsIWebProgress*, nsIRequest*, int32_t,
                                              int32_t, int32_t, int32_t) { return NS_OK; }
NS_IMETHODIMP JihadTransfer::OnLocationChange(nsIWebProgress*, nsIRequest*, nsIURI*,
                                              uint32_t) { return NS_OK; }
NS_IMETHODIMP JihadTransfer::OnStatusChange(nsIWebProgress*, nsIRequest*, nsresult,
                                            const char16_t*) { return NS_OK; }
NS_IMETHODIMP JihadTransfer::OnSecurityChange(nsIWebProgress*, nsIRequest*,
                                              uint32_t) { return NS_OK; }
NS_IMETHODIMP JihadTransfer::OnRefreshAttempted(nsIWebProgress*, nsIURI*, int32_t,
                                                bool, bool* _retval) {
  if (_retval) *_retval = true;
  return NS_OK;
}

void JihadTransfer::FinalizePartFile() {
  if (!mTempFile || !mTarget) return;
  bool same = false;
  if (NS_SUCCEEDED(mTempFile->Equals(mTarget, &same)) && same) return;
  bool exists = false;
  if (NS_FAILED(mTempFile->Exists(&exists)) || !exists) return;
  nsAutoString leaf;
  if (NS_FAILED(mTarget->GetLeafName(leaf))) return;
  // Same directory (the service derived the .part name from our target), so a
  // leaf-name move is an atomic rename over the placeholder we created.
  mTempFile->MoveTo(nullptr, leaf);
}

void JihadTransfer::Finish(nsresult aStatus) {
  if (mDone) return;
  mDone = true;
  // Keep ourselves alive across the sink callback + registry drop.
  RefPtr<JihadTransfer> kungFuDeathGrip(this);
  ActiveRemove(this);
  mCancelable = nullptr;

  if (NS_SUCCEEDED(aStatus)) {
    FinalizePartFile();
    nsAutoCString path;
    if (mTarget) mTarget->GetNativePath(path);
    printf("[jihad-dl] finished %s -> %s\n", mUrl.get(), path.get());
    if (gSink) gSink->OnDownloadFinished(mUrl.get(), mMime.get(), path.get());
  } else {
    char msg[64];
    snprintf(msg, sizeof msg, "0x%08x", (unsigned)static_cast<uint32_t>(aStatus));
    printf("[jihad-dl] error %s (%s)\n", mUrl.get(), msg);
    if (gSink) gSink->OnDownloadError(mUrl.get(), msg);
    // A cancelled/failed download leaves two files behind: the "<target>.part"
    // the saver was writing, and the empty target we reserved with CreateUnique.
    // Both are ours; drop them so a cancel doesn't litter the download dir.
    if (mTempFile) mTempFile->Remove(false);
    if (mTarget) {
      int64_t sz = -1;
      if (NS_SUCCEEDED(mTarget->GetFileSize(&sz)) && sz == 0) mTarget->Remove(false);
    }
  }
}

bool CancelDownload(const char* url) {
  if (!gActive || gActive->empty()) return false;
  // Snapshot: Abort() runs the whole termination path synchronously and mutates
  // gActive (Codex-style re-entrancy guard).
  std::vector<RefPtr<JihadTransfer>> snap(*gActive);
  bool any = false;
  for (size_t i = 0; i < snap.size(); ++i) {
    if (snap[i]->Done()) continue;
    if (url && *url && !snap[i]->Url().Equals(nsDependentCString(url))) continue;
    snap[i]->Abort();
    any = true;
  }
  return any;
}

// ── the helper-app dialog override ──────────────────────────────────────────
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
  // Pick a temp destination and tell the launcher to save there. viaPrompt
  // selects the callback the service is waiting on (saveDestinationAvailable
  // for promptForSaveToFileAsync, saveToDisk for show).
  static void BeginSave(nsIHelperAppLauncher* aLauncher, bool viaPrompt);
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

// Destination directory for engine downloads: $JIHAD_DOWNLOAD_DIR, else the
// engine temp dir. On the device the launcher points this at the media
// partition so a finished file survives for the app to hand to the user.
static already_AddRefed<nsIFile> jihadDownloadDir() {
  nsCOMPtr<nsIFile> dir;
  const char* env = getenv("JIHAD_DOWNLOAD_DIR");
  if (env && *env) {
    if (NS_SUCCEEDED(NS_NewNativeLocalFile(nsDependentCString(env), true,
                                           getter_AddRefs(dir))) && dir) {
      nsresult crv = dir->Create(nsIFile::DIRECTORY_TYPE, 0700);
      if (NS_FAILED(crv) && crv != NS_ERROR_FILE_ALREADY_EXISTS) dir = nullptr;
    }
  }
  if (!dir) NS_GetSpecialDirectory(NS_OS_TEMP_DIR, getter_AddRefs(dir));
  return dir.forget();
}

void JihadHelperDialog::BeginSave(nsIHelperAppLauncher* aLauncher, bool viaPrompt) {
  if (!aLauncher) return;
  nsCOMPtr<nsIFile> dir = jihadDownloadDir();
  nsCOMPtr<nsIFile> file;
  if (dir) dir->Clone(getter_AddRefs(file));
  if (file) {
    nsString name;
    aLauncher->GetSuggestedFileName(name);
    // The service already stripped path separators + illegal characters from
    // the suggested name; guard against an empty one only.
    if (name.IsEmpty()) name.AssignLiteral("jihad-download");
    if (NS_FAILED(file->Append(name)) ||
        NS_FAILED(file->CreateUnique(nsIFile::NORMAL_FILE_TYPE, 0600))) {
      file = nullptr;
    }
  }
  if (!file) {
    // No writable destination: terminate the launcher rather than leave it (and
    // its temp file) dangling. msgMimeHandoffUrl already went out, so the client
    // can still fetch the URL itself.
    fprintf(stderr, "[jihad-dl] no writable download dir — aborting engine save\n");
    if (viaPrompt) aLauncher->SaveDestinationAvailable(nullptr);
    else aLauncher->Cancel(NS_BINDING_ABORTED);
    return;
  }
  if (viaPrompt) aLauncher->SaveDestinationAvailable(file);
  else aLauncher->SaveToDisk(file, false);
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

// nsIHelperAppLauncherDialog — capture the handoff instead of opening a window,
// then drive the save so the download actually completes and is reported.
NS_IMETHODIMP
JihadHelperDialog::Show(nsIHelperAppLauncher* aLauncher, nsISupports*, uint32_t) {
  Report(aLauncher);
  BeginSave(aLauncher, /* viaPrompt */ false);
  return NS_OK;
}
NS_IMETHODIMP
JihadHelperDialog::PromptForSaveToFileAsync(nsIHelperAppLauncher* aLauncher,
                                            nsISupports*, const char16_t*,
                                            const char16_t*, bool) {
  Report(aLauncher);
  BeginSave(aLauncher, /* viaPrompt */ true);
  return NS_OK;
}

static JihadHelperDialog& Singleton() { static JihadHelperDialog s; return s; }

// Factory for the per-download transfer objects (unlike the dialog, each
// CreateInstance MUST hand back a fresh, really-refcounted object).
class JihadTransferFactory final : public nsIFactory {
 public:
  NS_IMETHOD QueryInterface(const nsIID& aIID, void** aResult) override {
    if (!aResult) return NS_ERROR_NULL_POINTER;
    if (aIID.Equals(NS_GET_IID(nsISupports)) || aIID.Equals(NS_GET_IID(nsIFactory))) {
      *aResult = static_cast<nsIFactory*>(this);
      AddRef();
      return NS_OK;
    }
    *aResult = nullptr;
    return NS_NOINTERFACE;
  }
  NS_IMETHOD_(MozExternalRefCountType) AddRef(void) override { return 2; }
  NS_IMETHOD_(MozExternalRefCountType) Release(void) override { return 1; }

  NS_IMETHOD CreateInstance(nsISupports* aOuter, const nsIID& aIID, void** aResult) override {
    if (aOuter) return NS_ERROR_NO_AGGREGATION;
    RefPtr<JihadTransfer> t = new JihadTransfer();
    return t->QueryInterface(aIID, aResult);
  }
  NS_IMETHOD LockFactory(bool) override { return NS_OK; }
};
static JihadTransferFactory& TransferFactory() { static JihadTransferFactory f; return f; }

bool InstallDownloadService() {
  static bool installed = false;
  if (installed) return true;
  nsCOMPtr<nsIComponentRegistrar> reg;
  if (NS_FAILED(NS_GetComponentRegistrar(getter_AddRefs(reg))) || !reg) return false;
  nsresult rv = reg->RegisterFactory(kJihadHelperDialogCID, "Jihad Helper Dialog",
                                     kHelperDialogContract, &Singleton());
  // Without this second registration the helper-app service cannot create an
  // nsITransfer and CANCELS every download (CreateTransfer failure => Cancel).
  nsresult rv2 = reg->RegisterFactory(kJihadTransferCID, "Jihad Transfer",
                                      kTransferContract, &TransferFactory());
  installed = NS_SUCCEEDED(rv) && NS_SUCCEEDED(rv2);
  return installed;
}

void SetDownloadSink(DownloadSink* sink) { gSink = sink; }

} // namespace jihad
