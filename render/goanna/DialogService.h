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

// ── The NON-BLOCKING sibling of DialogSink (cavekit-gre-widgets.md R5) ───────────────────
//
// A msgDialog* round-trip STOPS the daemon: OnDialog writes a FIFO path to the card and then
// polls that pipe until the card answers or the deadline expires (BrowserPageGoanna.cpp,
// awaitDialogReply). That is the right price for a question — "may this site install an
// add-on?" — and much too high for a statement: "cookies cleared", "the add-on could not be
// installed". A statement nobody can answer must never be able to stall a render.
//
// So informational messages take this channel instead. It is fire-and-forget: no reply pipe,
// no deadline, no return value the caller can wait on. On the device the sink is the Luna
// service (render/browserserver/JihadLunaService.cpp), which pushes one JSON payload to every
// subscribed card; the card draws a transient toast. With no sink installed — desktop
// harnesses, or a device whose liblunaservice.so lacks the subscription entry points — the
// post degrades to a single log line and the caller proceeds unchanged.
//
// NOTHING IS ADDED TO THE YAP CONTRACT BY THIS. Luna is a separate channel the cards already
// use (cavekit-ipc-contract.md R1 freezes YAP, not Luna; R4 is the precedent).
class NotificationSink {
 public:
  virtual ~NotificationSink() {}
  // Called on the engine (main) thread. Must NOT block: it runs inside whatever engine
  // operation produced the message.
  virtual void OnNotification(const char* category, const char* text) = 0;
};

// Set (or clear, with nullptr) the process-wide notification sink. Not owned. Same threading
// and lifetime contract as SetDialogSink: embedding thread only, cleared before the sink dies.
void SetNotificationSink(NotificationSink* sink);

// Post one informational message. Never blocks, never fails, never reports back. `category`
// is a short stable tag the card may use for styling/filtering ("privacy", "addon", "info");
// null or empty becomes "info". `text` is one short UTF-8 line meant to be read at a glance.
void PostNotification(const char* category, const char* text);

// Install the override of "@mozilla.org/prompter;1". Call once after the engine
// is initialised (XRE_InitEmbedding2). Idempotent; returns false on failure.
//
// Also registers the "jihad-notify" observer, which is how BUNDLED JS COMPONENTS reach
// PostNotification (they cannot call C++ directly, and adding an XPCOM component just to
// carry two strings would be a new binary interface for no gain). The subject is a property
// bag with `category` and `text` AStrings — the same idiom as "jihad-xpi-confirm" above it.
bool InstallDialogService();

// Set (or clear, with nullptr) the process-wide sink. Not owned. Threading/
// lifetime contract (Codex P0): call only from the embedding (main) thread, and
// clear the sink (SetDialogSink(nullptr)) before the sink object is destroyed.
// EngineHost::Shutdown clears it as a process-lifetime backstop.
void SetDialogSink(DialogSink* sink);

} // namespace jihad

#endif // JIHAD_DIALOG_SERVICE_H
