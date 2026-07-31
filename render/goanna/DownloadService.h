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

// Process-wide receiver of download/handoff requests. Not owned by the service.
// Every call runs on the embedding (main) thread.
class DownloadSink {
 public:
  virtual ~DownloadSink() {}
  // Called when content is handed off as a download. All strings are UTF-8;
  // contentLength is -1 when unknown. Drives msgMimeHandoffUrl.
  virtual void OnDownload(const char* url, const char* mimeType,
                          const char* suggestedName, int64_t contentLength) = 0;

  // --- engine-performed download lifecycle (YAP msgDownload*) ---------------
  // Defaulted to no-ops so existing sinks (tests) need not override them.
  // Exactly one Start is emitted per download, followed by zero or more
  // Progress, terminated by exactly one Finished OR one Error.
  virtual void OnDownloadStart(const char* url) { (void)url; }
  // bytesSoFar/totalBytes are the raw 64-bit counts; totalBytes is -1 when the
  // response had no Content-Length.
  virtual void OnDownloadProgress(const char* url, int64_t bytesSoFar,
                                  int64_t totalBytes) {
    (void)url; (void)bytesSoFar; (void)totalBytes;
  }
  // tmpFilePath is the on-disk path of the completed file (see JIHAD_DOWNLOAD_DIR).
  virtual void OnDownloadFinished(const char* url, const char* mimeType,
                                  const char* tmpFilePath) {
    (void)url; (void)mimeType; (void)tmpFilePath;
  }
  virtual void OnDownloadError(const char* url, const char* errorMsg) {
    (void)url; (void)errorMsg;
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

// YAP: cancelDownload(url) — abort an in-flight engine download. Matches the
// source URL emitted by OnDownloadStart; an empty/null url cancels every active
// download. Returns true if at least one download was aborted. The aborted
// download reports OnDownloadError (never OnDownloadFinished).
bool CancelDownload(const char* url);

} // namespace jihad

#endif // JIHAD_DOWNLOAD_SERVICE_H
