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
 */
#ifndef JIHAD_DOWNLOAD_SERVICE_H
#define JIHAD_DOWNLOAD_SERVICE_H

#include <string>
#include <cstdint>

namespace jihad {

// Process-wide receiver of download/handoff requests. Not owned by the service.
class DownloadSink {
 public:
  virtual ~DownloadSink() {}
  // Called when content is handed off as a download. All strings are UTF-8;
  // contentLength is -1 when unknown.
  virtual void OnDownload(const char* url, const char* mimeType,
                          const char* suggestedName, int64_t contentLength) = 0;
};

// Install the override of "@mozilla.org/helperapplauncherdialog;1". Call once
// after the engine is initialised. Idempotent; returns false on failure.
bool InstallDownloadService();

// Set (or clear, with nullptr) the process-wide sink. Not owned.
void SetDownloadSink(DownloadSink* sink);

} // namespace jihad

#endif // JIHAD_DOWNLOAD_SERVICE_H
