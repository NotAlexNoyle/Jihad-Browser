// Copyright 2026 the Jihad Browser project.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadWebView — the Enyo 2 equivalent of the Enyo 1.0 BasicWebView, bound to
// the UNCHANGED BrowserAdapter NPAPI plugin. Enyo 2 ships no BasicWebView, so
// this kind renders the plugin <object> itself and exposes the same adapter
// method surface the Enyo 1.0 app (../../app/source/Browser.js) drives.
//
// SELF-CONTAINED ENGINE ROUTING: the plugin MIME is application/x-jihad-browser
// (NOT the stock application/x-palm-browser). LunaSysMgr therefore loads OUR
// adapter (BrowserAdapterJihad.so -> BrowserServer at /tmp/yapserver.jihad-
// browser); the stock browser (application/x-palm-browser) is untouched, so the
// two coexist. This mirrors ../../app/source/JihadEngineOverride.js, which swaps
// the identical MIME onto the Enyo 1.0 BasicWebView.
//
// CONTRACT INVARIANT (cavekit-ipc-contract R1, cavekit-mochi-ui R3): the public
// callBrowserAdapter(method, args) proxy is the ONLY app-facing adapter surface,
// and the set of method names routed through it is identical to the Enyo 1.0 app
// (no additions, no renames). WebView-internal plugin calls (openURL,
// setPageIdentifier) go straight to the node via _invoke() — exactly as the
// Enyo 1.0 BasicWebView keeps those out of the app's own call set.

enyo.kind({
	name: "JihadWebView",
	kind: "enyo.Control",
	// The NPAPI plugin element. `type` selects OUR adapter (self-contained MIME).
	tag: "object",
	attributes: {
		type: "application/x-jihad-browser"
	},
	published: {
		//* Current page URL. Setting it navigates (BrowserAdapter openURL).
		url: "",
		//* Page identifier assigned by the window manager (setPageIdentifier).
		identifier: ""
	},
	events: {
		onLoadStarted: "",
		onLoadProgress: "",
		onLoadStopped: "",
		onPageTitleChanged: "",
		onUrlChanged: "",
		onServerConnected: ""
	},
	// --- lifecycle ----------------------------------------------------------
	rendered: function() {
		this.inherited(arguments);
		// The adapter dispatches page callbacks by invoking methods by name on
		// the plugin node's `eventListener` property (AdapterBase reads
		// mDOMObject.eventListener and invokes on it). Point it at this control
		// so the adapter -> app callbacks below fire, exactly as the Enyo 1.0
		// BasicWebView registers itself as its own event listener.
		var node = this.hasNode();
		if (node) {
			node.eventListener = this;
		}
	},
	// --- public adapter surface (FROZEN: identical set to the Enyo 1.0 app) --
	//* Proxy a scriptable BrowserAdapter method call to the plugin node. This is
	//* the single app-facing choke point; the set of method names passed here
	//* stays identical to ../../app/source/Browser.js (cavekit-ipc-contract R1).
	callBrowserAdapter: function(method, args) {
		var node = this.hasNode();
		if (node && typeof node[method] === "function") {
			return node[method].apply(node, args || []);
		}
	},
	// --- WebView-internal plugin calls --------------------------------------
	// These invoke frozen-contract adapter methods (openURL, setPageIdentifier
	// are both in the adapter's exported method table) directly on the node,
	// NOT through callBrowserAdapter — mirroring how the Enyo 1.0 BasicWebView
	// keeps its own setUrl/setIdentifier out of the app's callBrowserAdapter set.
	urlChanged: function() {
		this._invoke("openURL", [this.url]);
	},
	identifierChanged: function() {
		this._invoke("setPageIdentifier", [this.identifier]);
	},
	_invoke: function(method, args) {
		var node = this.hasNode();
		if (node && typeof node[method] === "function") {
			return node[method].apply(node, args || []);
		}
	},
	// --- adapter -> app callbacks (invoked by name on node.eventListener) ----
	// Only the shell-relevant subset is handled here; the full page / dialog /
	// download callback surface is the navigation-events parity port (T-053).
	// Unhandled adapter callbacks resolve to no JS method and are dropped by the
	// NPAPI bridge without error.
	loadStarted: function() {
		this.doLoadStarted();
	},
	loadProgress: function(progress) {
		this.doLoadProgress({progress: progress});
	},
	loadStopped: function() {
		this.doLoadStopped();
	},
	titleURLChange: function(title, url) {
		if (url) { this.url = url; }
		this.doPageTitleChanged({title: title, url: url || this.url});
	},
	titleChange: function(title) {
		this.doPageTitleChanged({title: title, url: this.url});
	},
	urlChange: function(url) {
		if (url) { this.url = url; }
		this.doUrlChanged({url: url || this.url});
	},
	serverConnected: function() {
		this.doServerConnected();
	},
	adapterInitialized: function() {
		this.doServerConnected();
	}
});
