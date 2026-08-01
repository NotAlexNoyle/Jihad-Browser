// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Route THIS app's Mojo WebView to the Jihad render daemon.
//
// PER-VARIANT MIME (cavekit-device-build.md R7, cavekit-mojo-ui.md R3,
// ../../../context/plans/plan-variant-identity.md): application/x-jihad-browser-mojo
// belongs to the MOJO variant alone. LunaSysMgr therefore loads this variant's own
// adapter shim (BrowserAdapterJihadMojo.so -> BrowserServer at
// /tmp/yapserver.jihad-browser-mojo). The Enyo variant (application/x-jihad-browser),
// the Mochi variant (application/x-jihad-browser-mochi), the stock webOS browser
// (application/x-palm-browser) and the stock Mojo browser
// (application/x-palm-mojo-browser -> /tmp/yapserver.browsermojo) are all untouched,
// so every one of them coexists with this package. The literal below is declared in
// this package and only here: the three Jihad front-ends are independent packages and
// none imports a shared constant from another.
//
// WHY A FRAMEWORK OVERRIDE IS NEEDED AT ALL: Mojo.Widget.WebView hard-codes the
// plugin MIME in its own setup() —
//
//     this.adapter = this.controller.document.createElement("object");
//     ...
//     this.adapter.setAttribute("type", "application/x-palm-mojo-browser");
//     ...
//     this.controller.element.appendChild(this.adapter);
//
// — and exposes no attribute for it, so there is no supported hook. Rewriting the
// type AFTER setup() returns is too late: appendChild is what makes the <object>
// live, and by then WebKit has already instantiated the stock plugin. This is the
// Mojo analogue of ../../../app/source/JihadEngineOverride.js, which swaps the same
// attribute on Enyo 1.0's BasicWebView.
//
// SCOPE OF THE OVERRIDE: it wraps Mojo.Widget.WebView.prototype.setup and, for the
// duration of that ONE synchronous call, shadows appendChild on that widget's OWN
// container element. The <object> is still detached at that point, so rewriting its
// type there is in time; the shadow is removed in a finally, so nothing outside the
// call — and no other element, widget or document — is affected. Stock
// Mojo.Widget.WebView behaviour is otherwise untouched, which matters because the
// framework special-cases the literal x-mojo-element="WebView" elsewhere (the
// character-picker and long-press context-menu paths), so this variant keeps that
// element name rather than registering a differently named widget class.
var JihadEngine = {

	//* THIS variant's NPAPI MIME. Single declaration for the whole package.
	MIME: "application/x-jihad-browser-mojo",

	//* The stock plugin types Mojo.Widget.WebView may set, by framework submission.
	//* Submission 506 (the one webOS 3.0.5 ships) uses the -mojo- form; older
	//* submissions used the plain one. Only these are rewritten, so an <object> the
	//* app or a page creates for anything else is left alone.
	STOCK_MIMES: {
		"application/x-palm-mojo-browser": true,
		"application/x-palm-browser": true
	},

	//* True once Mojo.Widget.WebView.prototype.setup has been wrapped.
	patched: false,

	isStockBrowserObject: function (node) {
		return !!(node && node.tagName && String(node.tagName).toLowerCase() === "object" &&
			node.getAttribute && this.STOCK_MIMES[node.getAttribute("type")]);
	},

	//* Is this live plugin element bound to our adapter? Used by the scene assistant
	//* to fail loudly rather than render silently on somebody else's engine.
	isOurs: function (node) {
		return !!(node && node.getAttribute && node.getAttribute("type") === this.MIME);
	},

	install: function () {
		if (!(window.Mojo && Mojo.Widget && Mojo.Widget.WebView &&
				Mojo.Widget.WebView.prototype)) {
			return false;
		}
		if (this.patched) {
			return true;
		}
		var engine = this;
		var proto = Mojo.Widget.WebView.prototype;
		var origSetup = proto.setup;
		proto.setup = function () {
			var host = this.controller && this.controller.element;
			var origAppendChild = host && host.appendChild;
			if (origAppendChild) {
				host.appendChild = function (child) {
					if (engine.isStockBrowserObject(child)) {
						// Still detached — setting the type now is what selects OUR
						// adapter when appendChild instantiates the plugin.
						child.setAttribute("type", engine.MIME);
					}
					return origAppendChild.call(this, child);
				};
			}
			try {
				return origSetup.apply(this, arguments);
			} finally {
				if (origAppendChild) {
					host.appendChild = origAppendChild;
				}
				if (engine.isStockBrowserObject(this.adapter)) {
					// A future framework submission inserted the plugin by some other
					// route. Report it; re-inserting the node here would only mean the
					// stock plugin had already been instantiated once.
					Mojo.Log.error("[Jihad] WebView plugin MIME override did NOT take: " +
						"adapter is still " + this.adapter.getAttribute("type"));
				}
			}
		};
		this.patched = true;
		Mojo.Log.info("[Jihad] Mojo WebView engine -> " + this.MIME);
		return true;
	}
};

// sources.json loads this file first and with no "scenes" key, so it runs at app
// startup — after the Mojo framework has been fully evaluated (Mojo.setupFramework
// -> loadApplicationSources) and before the stage assistant pushes any scene, so the
// widget class exists and no WebView has been instantiated yet. The retry only
// covers a framework build that defines the widget lazily.
(function () {
	if (JihadEngine.install()) {
		return;
	}
	var tries = 0;
	var timer = setInterval(function () {
		if (JihadEngine.install() || ++tries > 100) {
			clearInterval(timer);
		}
	}, 50);
})();
