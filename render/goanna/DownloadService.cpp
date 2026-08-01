/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — download / MIME-handoff interception. See DownloadService.h.
 */
#include "DownloadService.h"
#include "JihadRuntimePaths.h"            // the ONE runtime-path derivation (T-057 / R8)

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
#include "nsIInterfaceRequestorUtils.h"   // do_GetInterface (window context -> docShell)
#include "nsIDocShell.h"                  // download origin identity (F-1)
#include "nsIDocShellTreeItem.h"          // ... normalised to the ROOT docShell
#include "nsIWebNavigation.h"
#include "nsXPCOM.h"
#include "mozilla/RefPtr.h"
#include "mozilla/RefCountType.h"
#include <vector>
#include <string>
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

  JihadTransfer() : mOrigin(nullptr), mDone(false), mPseudo(false) {}

  const nsCString& Url() const { return mUrl; }
  bool Done() const { return mDone; }
  // Abort this download. Cancelling the helper-app handler finishes its file
  // saver with the abort status, which comes back as OnStateChange(STATE_STOP,
  // <failure>) -> exactly one msgDownloadError and never a Finished. Safe to
  // call re-entrantly (Finish is idempotent).
  //
  // aForceTerminal terminates the lifecycle HERE instead of waiting for that
  // callback. Two callers need it: a transfer with nothing to cancel through,
  // and engine shutdown (F-9), where no further engine callback will ever run —
  // waiting there would mean no terminal message and no file cleanup at all.
  void Abort(bool aForceTerminal = false) {
    if (mDone) return;
    nsCOMPtr<nsICancelable> c = mCancelable;
    if (c) c->Cancel(NS_BINDING_ABORTED);
    if (!c || aForceTerminal) Finish(NS_BINDING_ABORTED);
  }

 private:
  ~JihadTransfer() {}
  void Finish(nsresult aStatus);
  // Rename the "<target>.part" work file the helper-app service writes into over
  // the final target. Firefox does this move in its own nsITransfer (Download.jsm);
  // there is no other owner for it, so an embedder that supplies the contract must.
  // Returns the rename's result: F-5 — it used to be discarded.
  nsresult FinalizePartFile();

  nsCString         mUrl;
  nsCString         mMime;
  nsCOMPtr<nsIFile> mTarget;      // where the finished file must end up
  nsCOMPtr<nsIFile> mTempFile;    // "<target>.part" while the download runs
  nsCOMPtr<nsICancelable> mCancelable;
  DownloadOrigin    mOrigin;      // page that started it; null = unknown (F-1)
  bool              mDone;
  // True for nsExternalAppHandler::CreateFailedTransfer's pseudo-transfer (F-2):
  // the download never started, and neither the ".part" nor the target on disk is
  // ours, so the error path must not delete anything.
  bool              mPseudo;
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

// Recently TERMINATED download URLs, newest last, bounded (F-4). Only so the
// daemon can tell "you cancelled something that already ended" from "you
// cancelled something that never existed" in its log; nothing on the wire keys
// off it, and it is dropped at shutdown.
static const size_t kRecentMax = 8;
static std::vector<nsCString>* gRecent = nullptr;

static void RecentAdd(const nsCString& url) {
  if (url.IsEmpty()) return;
  if (!gRecent) gRecent = new std::vector<nsCString>();
  gRecent->push_back(url);
  if (gRecent->size() > kRecentMax) gRecent->erase(gRecent->begin());
}
static bool RecentHas(const char* url) {
  if (!gRecent || !url || !*url) return false;
  for (size_t i = 0; i < gRecent->size(); ++i)
    if ((*gRecent)[i].Equals(nsDependentCString(url))) return true;
  return false;
}

// ── which card started this download (F-1) ──────────────────────────────────
// The engine hands the helper-app dialog the load's window context, and that is
// the LAST point in the download's life at which the originating page is still
// identifiable — everything after it (nsExternalAppHandler, the file saver, the
// nsITransfer) is page-agnostic. Resolve it to the ROOT docShell of that page's
// tree, so a download started from an iframe attributes to the card that hosts
// it, and use the docShell pointer purely as an identity token (see
// DownloadOrigin in the header — never dereferenced by the daemon).
static DownloadOrigin jihadOriginKey(nsISupports* aWindowContext) {
  if (!aWindowContext) return nullptr;
  // The context is normally the loading nsDocShell itself; accept the two other
  // shapes the uriloader can hand over rather than silently losing the origin.
  nsCOMPtr<nsIDocShell> ds = do_QueryInterface(aWindowContext);
  if (!ds) {
    nsCOMPtr<nsIWebNavigation> nav = do_GetInterface(aWindowContext);
    ds = do_QueryInterface(nav);
  }
  if (!ds) {
    nsCOMPtr<nsIDocShellTreeItem> ti = do_GetInterface(aWindowContext);
    ds = do_QueryInterface(ti);
  }
  if (!ds) return nullptr;
  nsCOMPtr<nsIDocShellTreeItem> item = do_QueryInterface(ds);
  if (item) {
    nsCOMPtr<nsIDocShellTreeItem> root;
    item->GetRootTreeItem(getter_AddRefs(root));
    nsCOMPtr<nsIDocShell> rootDs = do_QueryInterface(root);
    if (rootDs) return rootDs.get();
  }
  return ds.get();
}

// The origin of the download whose transfer the engine is about to create.
// nsIHelperAppLauncherDialog::show -> BeginSave -> nsIHelperAppLauncher::
// saveToDisk -> ContinueSave -> CreateTransfer -> nsITransfer::init is ONE
// synchronous call stack (verified in nsExternalHelperAppService.cpp), so a
// single slot scoped to that stack is enough and — unlike a launcher->origin map
// — cannot leak an entry when a launcher is abandoned.
static DownloadOrigin gPendingOrigin = nullptr;

namespace {
struct ScopedPendingOrigin {
  explicit ScopedPendingOrigin(DownloadOrigin o) { gPendingOrigin = o; }
  ~ScopedPendingOrigin() { gPendingOrigin = nullptr; }
};
}  // namespace

NS_IMETHODIMP
JihadTransfer::Init(nsIURI* aSource, nsIURI* aTarget, const nsAString& /*aDisplayName*/,
                    nsIMIMEInfo* aMIMEInfo, PRTime /*aStartTime*/, nsIFile* aTempFile,
                    nsICancelable* aCancelable, bool /*aIsPrivate*/) {
  if (aSource) aSource->GetSpec(mUrl);
  if (aMIMEInfo) aMIMEInfo->GetMIMEType(mMime);
  mTempFile = aTempFile;
  mCancelable = aCancelable;
  mOrigin = gPendingOrigin;           // F-1: whose card this belongs to
  // aTarget is a file:// URI for the destination we picked in the dialog.
  nsCOMPtr<nsIFileURL> furl = do_QueryInterface(aTarget);
  if (furl) furl->GetFile(getter_AddRefs(mTarget));

  // F-2: a transfer initialised with NO temp file is not a download at all — it
  // is nsExternalAppHandler::CreateFailedTransfer's pseudo-transfer, built when
  // SetUpTempFile() failed (typically: the download dir is full or unwritable,
  // which F-3's /tmp default made a live risk on the device). That path sets
  // mCanceled=true WITHOUT ever calling NotifyTransfer, so the engine will never
  // send us OnStateChange. Left as-is we emitted msgDownloadStart and then
  // nothing, forever: the client's download list kept a phantom entry with no
  // terminal message, we stayed pinned in gActive, and every later
  // cancelDownload matched us and reported "aborted" while doing nothing at all
  // (nsExternalAppHandler::Cancel early-returns on mCanceled). Terminate it here
  // instead — Start immediately followed by Error, which is a valid, complete
  // lifecycle under the contract in DownloadService.h.
  mPseudo = !aTempFile;

  if (!mPseudo) ActiveAdd(this);
  printf("[jihad-dl] start %s (mime=%s)%s\n", mUrl.get(), mMime.get(),
         mPseudo ? " [engine could not stage a temp file — failing immediately]" : "");
  if (gSink) gSink->OnDownloadStart(mOrigin, mUrl.get());
  // The engine does not pass CreateFailedTransfer's real nsresult to init(), so
  // report the generic failure rather than invent a more specific one.
  if (mPseudo) Finish(NS_ERROR_FAILURE);
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
  if (gSink) gSink->OnDownloadProgress(mOrigin, mUrl.get(), aCurTotalProgress,
                                       aMaxTotalProgress);
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

nsresult JihadTransfer::FinalizePartFile() {
  // No destination at all: we have nothing to report a path for, so this is a
  // failure even though the bytes may have arrived.
  if (!mTarget) return NS_ERROR_UNEXPECTED;
  if (!mTempFile) return NS_OK;          // engine wrote straight to the target
  bool same = false;
  if (NS_SUCCEEDED(mTempFile->Equals(mTarget, &same)) && same) return NS_OK;
  bool exists = false;
  if (NS_FAILED(mTempFile->Exists(&exists)) || !exists) {
    // Nothing left to move. Fine only if the destination is actually there;
    // otherwise the download produced no file and must not report success.
    bool haveTarget = false;
    if (NS_SUCCEEDED(mTarget->Exists(&haveTarget)) && haveTarget) return NS_OK;
    return NS_ERROR_FILE_NOT_FOUND;
  }
  nsAutoString leaf;
  nsresult rv = mTarget->GetLeafName(leaf);
  if (NS_FAILED(rv)) return rv;
  // Same directory (the service derived the .part name from our target), so a
  // leaf-name move is an atomic rename over the placeholder we created.
  return mTempFile->MoveTo(nullptr, leaf);
}

void JihadTransfer::Finish(nsresult aStatus) {
  if (mDone) return;
  mDone = true;
  // Keep ourselves alive across the sink callback + registry drop.
  RefPtr<JihadTransfer> kungFuDeathGrip(this);
  ActiveRemove(this);
  mCancelable = nullptr;

  // F-5: the ".part" -> final rename is ours to perform and it can fail (target
  // directory gone, cross-device after a JIHAD_DOWNLOAD_DIR change, ENOSPC on
  // the metadata write, a stale read-only mount). Its result used to be thrown
  // away and msgDownloadFinished emitted regardless — pointing the client at the
  // 0-byte placeholder BeginSave reserved with CreateUnique, so the user was
  // handed an empty file and told the download succeeded. Demote it to the error
  // path, where the leftovers are cleaned up like any other failure.
  if (NS_SUCCEEDED(aStatus)) {
    nsresult mv = FinalizePartFile();
    if (NS_FAILED(mv)) {
      printf("[jihad-dl] finalize FAILED for %s (0x%08x) — reporting as an error\n",
             mUrl.get(), (unsigned)static_cast<uint32_t>(mv));
      aStatus = mv;
    }
  }

  RecentAdd(mUrl);   // F-4: remember that this URL reached a terminal state

  if (NS_SUCCEEDED(aStatus)) {
    nsAutoCString path;
    if (mTarget) mTarget->GetNativePath(path);
    printf("[jihad-dl] finished %s -> %s\n", mUrl.get(), path.get());
    if (gSink) gSink->OnDownloadFinished(mOrigin, mUrl.get(), mMime.get(), path.get());
  } else {
    char msg[64];
    snprintf(msg, sizeof msg, "0x%08x", (unsigned)static_cast<uint32_t>(aStatus));
    printf("[jihad-dl] error %s (%s)\n", mUrl.get(), msg);
    if (gSink) gSink->OnDownloadError(mOrigin, mUrl.get(), msg);
    // A cancelled/failed download leaves two files behind: the "<target>.part"
    // the saver was writing, and the empty target we reserved with CreateUnique.
    // Both are ours; drop them so a cancel doesn't litter the download dir.
    // NOT for the F-2 pseudo-transfer: nothing on disk there is ours (the engine
    // never staged a temp file, and its "target" is a speculative name in the
    // download dir that may well be somebody else's existing file).
    if (!mPseudo) {
      if (mTempFile) mTempFile->Remove(false);
      if (mTarget) {
        int64_t sz = -1;
        if (NS_SUCCEEDED(mTarget->GetFileSize(&sz)) && sz == 0) mTarget->Remove(false);
      }
    }
  }
}

CancelOutcome CancelDownload(const char* url) {
  const bool all = !url || !*url;
  if (gActive && !gActive->empty()) {
    // Snapshot: Abort() runs the whole termination path synchronously and mutates
    // gActive (Codex-style re-entrancy guard).
    std::vector<RefPtr<JihadTransfer>> snap(*gActive);
    bool any = false;
    for (size_t i = 0; i < snap.size(); ++i) {
      if (snap[i]->Done()) continue;
      if (!all && !snap[i]->Url().Equals(nsDependentCString(url))) continue;
      snap[i]->Abort();
      any = true;
      // F-4: one cancelDownload(url) cancels ONE download. Without this break a
      // URL being fetched twice (a re-tapped link, a page that triggers the same
      // attachment from two frames) had every one of its transfers aborted, so
      // the client received several msgDownloadError for a single url — which,
      // since the URL is the only key the frozen contract carries, reads as one
      // download failing repeatedly and breaks the "exactly one terminal message
      // per download" invariant the download list depends on.
      if (!all) return CancelOutcome::Aborted;
      // The empty-url form deliberately means "stop everything" (see the header),
      // so it keeps going.
    }
    if (any) return CancelOutcome::Aborted;
  }
  // Nothing in flight matched. Distinguish the two ways that happens, for the
  // log line — the frozen YAP surface has no cancelDownload reply and no
  // "cancel-failed" message, and emitting a msgDownloadError for a download that
  // already reported its terminal message would corrupt the client's list, so
  // this deliberately stays off the wire.
  if (!all && RecentHas(url)) return CancelOutcome::AlreadyTerminated;
  return CancelOutcome::Unknown;
}

void ShutdownDownloadService() {
  // F-9: in-flight downloads used to simply die with the process. That leaked
  // gActive, denied every interrupted download its terminal message, and — worse
  // on the device — left a "<target>.part" plus the empty CreateUnique
  // placeholder behind for each one, i.e. permanent residue in a directory
  // cavekit-device-build.md R8 requires to be clean. Abort with the terminal
  // forced (no engine callback will arrive after this point), which runs the
  // normal error path: one msgDownloadError, both files removed.
  if (gActive) {
    std::vector<RefPtr<JihadTransfer>> snap(*gActive);
    for (size_t i = 0; i < snap.size(); ++i) {
      if (!snap[i]->Done()) snap[i]->Abort(/* aForceTerminal */ true);
    }
    delete gActive;
    gActive = nullptr;
  }
  delete gRecent;
  gRecent = nullptr;
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
  static void Report(nsIHelperAppLauncher* aLauncher, DownloadOrigin aOrigin);
  // Pick a temp destination and tell the launcher to save there. viaPrompt
  // selects the callback the service is waiting on (saveDestinationAvailable
  // for promptForSaveToFileAsync, saveToDisk for show).
  static void BeginSave(nsIHelperAppLauncher* aLauncher, bool viaPrompt);
};

void JihadHelperDialog::Report(nsIHelperAppLauncher* aLauncher, DownloadOrigin aOrigin) {
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
    gSink->OnDownload(aOrigin, url.get(), mime.get(),
                      NS_ConvertUTF16toUTF8(name).get(), len);
}

// ── destination directory for engine downloads ──────────────────────────────
// F-3: this used to spell its own path — $JIHAD_DOWNLOAD_DIR taken on trust,
// with NS_OS_TEMP_DIR as the fallback — and both halves were wrong for the
// device. Nothing checked the value, so an inherited/stale variable could point
// finished downloads straight at /media/internal, the user's vfat USB volume
// that cavekit-device-build.md R8 forbids the package to write to at all; and
// since no upstart job sets that variable, the REAL device path was the
// fallback, /tmp — which no prerm cleans, so every download was permanent
// residue (R8's "no residue" criterion) and a filled /tmp then fed F-2's
// temp-file-setup failure.
//
// Everything now goes through JihadRuntimePaths.h, which is the ONE place in the
// tree allowed to derive a runtime path. That gives us, for free: the
// unconditional R8 guard, RuntimeDirUsable()'s lstat/ownership/mode validation
// (so a symlink planted at the path, or a group-writable dir, is refused instead
// of silently used for the user's downloads), and the variant-scoped default
// /var/palm/jihad/<variant>/downloads — inside the tree that variant's own prerm
// deletes.
//
// $JIHAD_DOWNLOAD_DIR keeps RuntimeResolvePath's vocabulary: an absolute path is
// honoured (unless it is on the user's volume, which is redirected with a loud
// line), a bare name is a leaf under the state dir, and "0"/"off"/"no"/"false"
// disables the engine-side save entirely — the client still gets
// msgMimeHandoffUrl and can fetch the URL itself. UNSET means the DEFAULT, not
// "disabled", because a download has to land somewhere; hence the "1" below.
static already_AddRefed<nsIFile> jihadDownloadDir() {
  nsCOMPtr<nsIFile> dir;
  const char* env = getenv("JIHAD_DOWNLOAD_DIR");
  const std::string want = RuntimeResolvePath((env && *env) ? env : "1", "downloads");
  if (want.empty()) return dir.forget();          // disabled, or no writable state
  std::string usable;
  // Creates it 0700 if missing and re-checks the R8 + ownership/mode rules on
  // what is actually there.
  if (!RuntimeTryDir(want, 0700, usable)) {
    fprintf(stderr, "[jihad-dl] download dir %s unusable — engine save disabled\n",
            want.c_str());
    return dir.forget();
  }
  if (NS_FAILED(NS_NewNativeLocalFile(nsDependentCString(usable.c_str()), true,
                                      getter_AddRefs(dir))))
    dir = nullptr;
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
// aWindowContext is the ONLY page identity the engine still carries at this
// point (F-1). Resolve it once and keep it in scope for the whole synchronous
// save stack, so the nsITransfer the engine creates below picks it up.
NS_IMETHODIMP
JihadHelperDialog::Show(nsIHelperAppLauncher* aLauncher, nsISupports* aWindowContext,
                        uint32_t) {
  DownloadOrigin origin = jihadOriginKey(aWindowContext);
  ScopedPendingOrigin pending(origin);
  Report(aLauncher, origin);
  BeginSave(aLauncher, /* viaPrompt */ false);
  return NS_OK;
}
NS_IMETHODIMP
JihadHelperDialog::PromptForSaveToFileAsync(nsIHelperAppLauncher* aLauncher,
                                            nsISupports* aWindowContext,
                                            const char16_t*, const char16_t*, bool) {
  DownloadOrigin origin = jihadOriginKey(aWindowContext);
  ScopedPendingOrigin pending(origin);
  Report(aLauncher, origin);
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
