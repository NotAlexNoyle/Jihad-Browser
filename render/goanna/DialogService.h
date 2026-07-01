/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — JS dialog interception (domain G / R3).
 *
 * Web content dialogs (window.alert / confirm / prompt) are routed by Goanna
 * through the "@mozilla.org/prompter;1" nsIPromptFactory -> nsIPrompt. The stock
 * factory tries to open a chrome dialog window, which is meaningless in the
 * headless render daemon. We override that contract with our own factory so
 * every content dialog is captured and answered by a process-wide sink instead.
 *
 * On the device the sink forwards the dialog to the BrowserAdapter (msgDialog*)
 * and blocks for the user's reply; on desktop the test installs a sink that
 * records the dialog and returns a canned answer.
 */
#ifndef JIHAD_DIALOG_SERVICE_H
#define JIHAD_DIALOG_SERVICE_H

#include <string>

namespace jihad {

// Kind of dialog raised by content, matching the YAP msgDialog* surface.
enum class DialogKind { Alert, Confirm, Prompt };

// A canned/desired outcome the sink supplies back to the engine.
struct DialogReply {
  bool accept = true;            // confirm/prompt OK vs Cancel; ignored for alert
  std::string promptValue;       // prompt() input text (UTF-8), when accepted
};

// Process-wide receiver of content dialogs. Not owned by the service.
class DialogSink {
 public:
  virtual ~DialogSink() {}
  // Called synchronously on the engine thread when content opens a dialog.
  // `text` is UTF-8. Implementations fill `reply`.
  virtual void OnDialog(DialogKind kind, const char* text, DialogReply* reply) = 0;
};

// Install the override of "@mozilla.org/prompter;1". Call once after the engine
// is initialised (XRE_InitEmbedding2). Idempotent; returns false on failure.
bool InstallDialogService();

// Set (or clear, with nullptr) the process-wide sink. Not owned.
void SetDialogSink(DialogSink* sink);

} // namespace jihad

#endif // JIHAD_DIALOG_SERVICE_H
