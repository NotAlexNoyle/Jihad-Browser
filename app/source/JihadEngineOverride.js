/* Copyright 2026 NotAlexNoyle.
 * Licensed under the Apache License, Version 2.0; see ../../LICENSE.
 *
 * Route THIS app's WebView to the Jihad/Goanna engine:
 * swap the NPAPI plugin mime so Enyo loads OUR adapter (application/x-jihad-browser
 * -> BrowserAdapterJihad.so), which connects to our BrowserServer at
 * /tmp/yapserver.jihad-browser. Stock browser (application/x-palm-browser ->
 * /tmp/yapserver.browser) is untouched, so Jihad is a self-contained, redistributable
 * Goanna alternative to Atlas that coexists with the system browser.
 *
 * PER-VARIANT MIME (cavekit-device-build.md R7, ../../context/plans/plan-variant-identity.md):
 * application/x-jihad-browser belongs to the ENYO-1.0 variant alone. The Mochi and Mojo
 * front-ends ship their own packages with their own MIME (…-mochi / …-mojo), their own
 * adapter shim, YAP name, socket and upstart job, and never load this one. The literal is
 * declared here and only here — the three variants are independent packages and none
 * imports a shared constant from another.
 *
 * Typing is handled entirely in the daemon: the webOS VKB delivers each committed
 * character to the focused plugin as an npPalmKeyDownEvent, and BrowserPageGoanna::keyDown
 * inserts it into the focused editable. We do NOT bridge keys at the document level —
 * the VKB does not raise document key events for our swapped-type plugin, and forwarding
 * them here would double-insert with the daemon path and could leak keystrokes to the log. */
(function () {
	function log(m) { if (window.enyo && enyo.log) { enyo.log("[Jihad] " + m); } }

	function patch() {
		if (!(window.enyo && enyo.BasicWebView && enyo.BasicWebView.prototype)) { return false; }
		var proto = enyo.BasicWebView.prototype;
		if (!proto._jihadPatched) {
			// engine selection: load OUR adapter for this WebView. (Do NOT touch editorFocused
			// or the VKB mode — the base BasicWebView already raises the keyboard on focus.)
			var origCreate = proto.create;
			proto.create = function () {
				origCreate.apply(this, arguments);
				this.domAttributes.type = "application/x-jihad-browser";
			};
			proto._jihadPatched = true;
			log("WebView engine -> application/x-jihad-browser");
		}
		return true;
	}

	// Enyo 1 loads framework kinds before app source, so BasicWebView is usually ready.
	// Retry briefly in case app source is parsed before the palm controls register.
	if (!patch()) {
		var t = setInterval(function () { if (patch()) { clearInterval(t); } }, 50);
	}
})();
