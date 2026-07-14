/* Route THIS app's WebView to the Jihad/Goanna engine:
 * swap the NPAPI plugin mime so Enyo loads OUR adapter (application/x-jihad-browser
 * -> BrowserAdapterJihad.so), which connects to our BrowserServer at
 * /tmp/yapserver.jihad-browser. Stock browser (application/x-palm-browser ->
 * /tmp/yapserver.browser) is untouched, so Jihad is a self-contained, redistributable
 * Goanna alternative to Atlas that coexists with the system browser.
 *
 * Also bridges the on-screen keyboard to the plugin: webOS shows the VKB on editorFocused
 * (BasicWebView calls PalmSystem.editorFocused) but, for our swapped-type plugin, does not
 * route the committed keystrokes into it — so typed text was lost even though the daemon's
 * insert path works. We track the focused WebView and forward key input to the plugin's
 * insertStringAtCursor. enyo.log lines (visible via palm-log) also confirm whether the keys
 * reach the app document at all. */
(function () {
	var jihadView = null;   // the BasicWebView whose editor is currently focused

	function log(m) { if (window.enyo && enyo.log) { enyo.log("[Jihad] " + m); } }

	function patch() {
		if (!(window.enyo && enyo.BasicWebView && enyo.BasicWebView.prototype)) { return false; }
		var proto = enyo.BasicWebView.prototype;
		if (!proto._jihadPatched) {
			// 1) engine selection: load OUR adapter for this WebView.
			var origCreate = proto.create;
			proto.create = function () {
				origCreate.apply(this, arguments);
				this.domAttributes.type = "application/x-jihad-browser";
			};
			// 2) track editor focus so the key bridge knows the target plugin. (NB: do NOT put the
			// VKB in manual mode here — manual mode makes the app responsible for showing the
			// keyboard, so it stopped auto-appearing on tap.)
			var origEF = proto.editorFocused;
			proto.editorFocused = function (foc, ft, fa) {
				jihadView = foc ? this : (jihadView === this ? null : jihadView);
				log("editorFocused=" + (foc ? 1 : 0) + " type=" + ft);
				return origEF ? origEF.apply(this, arguments) : undefined;
			};
			proto._jihadPatched = true;
			log("WebView engine -> application/x-jihad-browser");
		}
		return true;
	}

	// Forward a character / backspace to the focused plugin's scriptable methods.
	function forward(ch, code) {
		if (!jihadView) { return; }
		var node = jihadView.hasNode && jihadView.hasNode();
		if (!node) { return; }
		try {
			if (code === 8) {                       // backspace -> engine key (daemon DeleteBackward)
				if (node.keyDown) { node.keyDown(8, 0, 8); }
			} else if (ch && node.insertStringAtCursor) {
				node.insertStringAtCursor(ch);
			}
			log("fwd code=" + code + " len=" + (ch ? ch.length : 0));
		} catch (e) { log("fwd err " + e); }
	}

	function installKeyBridge() {
		if (!window.document || window._jihadKeyBridge) { return; }
		window._jihadKeyBridge = true;
		// keypress carries the printable charCode; keydown carries editing keys (backspace).
		document.addEventListener("keypress", function (e) {
			log("keypress cc=" + e.charCode + " kc=" + e.keyCode);
			forward(e.charCode ? String.fromCharCode(e.charCode) : "", e.keyCode);
		}, true);
		document.addEventListener("keydown", function (e) {
			if (e.keyCode === 8) { log("keydown backspace"); forward("", 8); }
		}, true);
		log("key bridge installed");
	}

	// Enyo 1 loads framework kinds before app source, so BasicWebView is usually ready.
	// Retry briefly in case app source is parsed before the palm controls register.
	installKeyBridge();
	if (!patch()) {
		var t = setInterval(function () { if (patch()) { clearInterval(t); } }, 50);
	}
})();
