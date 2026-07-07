/* Route THIS app's WebView to the Jihad/Goanna engine:
 * swap the NPAPI plugin mime so Enyo loads OUR adapter (application/x-jihad-browser
 * -> BrowserAdapterJihad.so), which connects to our BrowserServer at
 * /tmp/yapserver.jihad-browser. Stock browser (application/x-palm-browser ->
 * /tmp/yapserver.browser) is untouched, so Jihad is a self-contained, redistributable
 * Goanna alternative to Atlas that coexists with the system browser. */
(function () {
	function patch() {
		if (window.enyo && enyo.BasicWebView && enyo.BasicWebView.prototype) {
			var orig = enyo.BasicWebView.prototype.create;
			enyo.BasicWebView.prototype.create = function () {
				orig.apply(this, arguments);
				this.domAttributes.type = "application/x-jihad-browser";
			};
			if (enyo.log) { enyo.log("[Jihad] WebView engine -> application/x-jihad-browser"); }
			return true;
		}
		return false;
	}
	// Enyo 1 loads framework kinds before app source, so BasicWebView is usually ready.
	// Retry briefly in case app source is parsed before the palm controls register.
	if (!patch()) {
		var t = setInterval(function () { if (patch()) { clearInterval(t); } }, 50);
	}
})();
