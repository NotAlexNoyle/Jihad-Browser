/* Jihad Browser (Mojo) — main scene assistant. SKELETON for a future Mojo port.
 *
 * HOW THE ENYO VARIANTS RENDER (what a Mojo port must replicate):
 *   The Enyo shells put a WebView whose plugin MIME is swapped to `application/x-jihad-browser`
 *   (see app/source/JihadEngineOverride.js). That routes the card through the Jihad NPAPI adapter
 *   -> the Jihad daemon -> the Goanna engine, instead of the stock QtWebKit browser engine. All of
 *   that (adapter shim, daemon, libxul + bundled glibc-2.23) is ALREADY installed on-device by this
 *   package's deviceroot bundle — a Mojo port does NOT touch the engine; it only builds the UI.
 *
 * TODO for a real Mojo port:
 *   1. Instantiate a Mojo web widget (Mojo.Widget.WebView / the PalmSysMgr web control) in the main
 *      scene and set its plugin type to `application/x-jihad-browser` BEFORE first load.
 *   2. Build the chrome: address bar, back / forward / reload / stop, progress, the on-screen
 *      keyboard focus handling.
 *   3. Wire the WebView callbacks + the address bar to the SAME contract the Enyo shell speaks —
 *      `callBrowserAdapter(...)` + `palm://com.palm.browserServer/*` (byte-identical YAP). No new
 *      engine/daemon work: the daemon already serves /tmp/yapserver.jihad-browser.
 */
function MainAssistant() {}

MainAssistant.prototype.setup = function () {
	// TODO: build the browser chrome + the application/x-jihad-browser WebView here.
};

MainAssistant.prototype.activate = function () {};
MainAssistant.prototype.deactivate = function () {};
MainAssistant.prototype.cleanup = function () {};
