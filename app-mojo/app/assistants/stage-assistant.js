/* Jihad Browser (Mojo) — stage assistant. SKELETON for a future Mojo port.
 * Pushes the main browser scene. The engine/daemon/adapter are already on-device (this package's
 * deviceroot bundle installs them) — a Mojo port only has to build the UI + the x-jihad-browser
 * WebView. See README.md. */
function StageAssistant() {}

StageAssistant.prototype.setup = function () {
	this.controller.pushScene("main");
};
