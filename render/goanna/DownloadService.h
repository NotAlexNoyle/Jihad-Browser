/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — download / MIME-handoff interception (domain G / R4).
 *
 * When content cannot be rendered (Content-Disposition: attachment, or an
 * unsupported MIME type), Goanna routes it to the external helper-app service,
 * which asks the "@mozilla.org/helperapplauncherdialog;1" what to do. The stock
 * dialog opens a chrome save/open window -- meaningless in the headless daemon.
 * We override that contract so every download/handoff is reported to a sink,
 * which maps to the YAP msgDownload* / msgMimeNotSupported surface.
 *
 * Two things happen for one captured download:
 *   1. OnDownload  -> msgMimeHandoffUrl (YAP 0x2015): the isis app's existing
 *      path, which hands the URL to com.palm.downloadmanager.
 *   2. the engine SAVES the response to a temp file and reports the lifecycle
 *      (OnDownloadStart / OnDownloadProgress / OnDownloadFinished |
 *      OnDownloadError) -> msgDownloadStart/Progress/Finished/Error
 *      (YAP 0x2010..0x2013), the same messages the stock isis BrowserServer
 *      emitted from its WebKit download callbacks.
 * (2) costs no extra network traffic: nsExternalAppHandler has already opened
 * the channel and is streaming the body into its own temp file by the time the
 * dialog is consulted. Driving saveToDisk is what makes that download RESOLVE
 * (and stops the temp file being orphaned) instead of hanging unreferenced.
 */
#ifndef JIHAD_DOWNLOAD_SERVICE_H
#define JIHAD_DOWNLOAD_SERVICE_H

#include <string>
#include <cstdint>

namespace jihad {

// ── whose download is this? (F-1) ───────────────────────────────────────────
// The engine's download machinery is PROCESS-wide: nsExternalHelperAppService
// owns the transfer, not a page, so by the time the lifecycle callbacks fire
// there is no page handle left in them. The daemon used to paper over that by
// routing every download message to the last-connected card, which is wrong the
// moment two cards are open (the other card's progress/finished land on the card
// the user just opened) and catastrophic once that card closes (reap() nulled
// the target without re-electing, so every remaining message — including the one
// terminal message the client's download list is waiting on — was discarded).
//
// `originKey` closes that: it is an OPAQUE identity token for the page whose
// docShell started the download, captured at the one place the engine still
// knows the origin (nsIHelperAppLauncherDialog::show's window context) and
// carried through every callback of that download. It is an identity value only
// — compared, never dereferenced — so this header stays free of XPCOM types and
// can keep being included by the daemon dispatch layer, which is compiled
// without engine headers. nullptr means "origin unknown" (the engine built the
// transfer outside our dialog stack); a sink must then FALL BACK to some live
// page, never drop the message.
typedef const void* DownloadOrigin;

// Process-wide receiver of download/handoff requests. Not owned by the service.
// Every call runs on the embedding (main) thread.
class DownloadSink {
 public:
  virtual ~DownloadSink() {}
  // Called when content is handed off as a download. All strings are UTF-8;
  // contentLength is -1 when unknown. Drives msgMimeHandoffUrl.
  virtual void OnDownload(DownloadOrigin origin, const char* url,
                          const char* mimeType, const char* suggestedName,
                          int64_t contentLength) = 0;

  // --- engine-performed download lifecycle (YAP msgDownload*) ---------------
  // Defaulted to no-ops so existing sinks (tests) need not override them.
  // Exactly one Start is emitted per download, followed by zero or more
  // Progress, terminated by exactly one Finished OR one Error. `origin` is the
  // same token for every callback of one download.
  virtual void OnDownloadStart(DownloadOrigin origin, const char* url) {
    (void)origin; (void)url;
  }
  // bytesSoFar/totalBytes are the raw 64-bit counts; totalBytes is -1 when the
  // response had no Content-Length.
  virtual void OnDownloadProgress(DownloadOrigin origin, const char* url,
                                  int64_t bytesSoFar, int64_t totalBytes) {
    (void)origin; (void)url; (void)bytesSoFar; (void)totalBytes;
  }
  // tmpFilePath is the on-disk path of the completed file (see JIHAD_DOWNLOAD_DIR).
  virtual void OnDownloadFinished(DownloadOrigin origin, const char* url,
                                  const char* mimeType, const char* tmpFilePath) {
    (void)origin; (void)url; (void)mimeType; (void)tmpFilePath;
  }
  virtual void OnDownloadError(DownloadOrigin origin, const char* url,
                               const char* errorMsg) {
    (void)origin; (void)url; (void)errorMsg;
  }
};

// Install the override of "@mozilla.org/helperapplauncherdialog;1" and of
// "@mozilla.org/transfer;1" (the embedding hook the helper-app service uses to
// report download progress). Call once after the engine is initialised.
// Idempotent; returns false on failure.
bool InstallDownloadService();

// Set (or clear, with nullptr) the process-wide sink. Not owned. Threading/
// lifetime contract (Codex P0): call only from the embedding (main) thread, and
// clear the sink before the sink object is destroyed. EngineHost::Shutdown clears
// it as a process-lifetime backstop.
void SetDownloadSink(DownloadSink* sink);

// What a cancelDownload request actually did (F-4). The frozen YAP surface has
// no reply for cancelDownload — it is a fire-and-forget async command — so this
// is for the DAEMON's log line, which on the device is the only diagnostic there
// is. Reporting a flat "aborted"/"no match" made "the download already ended"
// indistinguishable from "there was never such a download", and the first of
// those is the normal race (the user taps Cancel as the last chunk lands).
enum class CancelOutcome {
  Aborted,            // an in-flight download was matched and aborted
  AlreadyTerminated,  // it ran, but it had already finished/failed
  Unknown             // no download with that URL was ever started here
};

// YAP: cancelDownload(url) — abort an in-flight engine download. Matches the
// source URL emitted by OnDownloadStart; an empty/null url cancels every active
// download, a non-empty one cancels exactly ONE (see F-4 in the .cpp). The
// aborted download reports OnDownloadError (never OnDownloadFinished).
CancelOutcome CancelDownload(const char* url);

// Engine teardown: abort everything still in flight and drop the service's
// process-global state (F-9). Call from the embedding (main) thread while the
// sink is still installed, so each interrupted download still delivers its
// terminal OnDownloadError and still cleans up its own files. Idempotent.
void ShutdownDownloadService();

} // namespace jihad

#endif // JIHAD_DOWNLOAD_SERVICE_H
