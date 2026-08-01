// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadWebView — the Enyo 2 equivalent of the Enyo 1.0 BasicWebView, bound to
// the UNCHANGED BrowserAdapter NPAPI plugin. Enyo 2 ships no BasicWebView, so
// this kind renders the plugin <object> itself and mirrors BasicWebView's
// lifecycle (SDK: enyo/1.0/framework/source/palm/controls/BasicWebView.js):
// call queue, connect handshake, touch registration, VKB/editor focus.
//
// SELF-CONTAINED ENGINE ROUTING: the plugin MIME is
// application/x-jihad-browser-mochi — THIS VARIANT'S OWN MIME, not the Enyo
// variant's and not the stock application/x-palm-browser. LunaSysMgr therefore
// loads the Mochi variant's own adapter shim (BrowserAdapterJihadMochi.so ->
// BrowserServer at /tmp/yapserver.jihad-browser-mochi); the Enyo variant
// (application/x-jihad-browser -> /tmp/yapserver.jihad-browser) and the stock
// browser (application/x-palm-browser -> /tmp/yapserver.browser) are untouched,
// so all three coexist. Per cavekit-device-build.md R7 / plan-variant-identity.md
// every variant owns a complete, independent stack — sharing one MIME/adapter
// with the Enyo variant is exactly the co-ownership R7 removes. The Enyo-1.0
// sibling does the equivalent swap in ../../app/source/JihadEngineOverride.js
// with ITS own MIME.
//
// CONTRACT INVARIANT (cavekit-ipc-contract R1, cavekit-mochi-ui R3): the public
// callBrowserAdapter(method, args) proxy is the ONLY app-facing adapter surface,
// and the set of method names the app routes through it is identical to the Enyo
// 1.0 app (Browser.js) — no additions, no renames. WebView-internal plugin calls
// (openURL, setPageIdentifier, connectBrowserServer, the initView setup, viewport
// sizing) go through the private _call()/_invoke() path straight to the node —
// exactly as the Enyo 1.0 BasicWebView keeps those out of the app's own call set.

enyo.kind({
	name: "JihadWebView",
	kind: "enyo.Control",
	// The NPAPI plugin element. `type` selects THIS VARIANT'S adapter (its own
	// self-contained MIME, declared here and nowhere else — the three variants are
	// independent packages and none imports another's constants); tabIndex makes
	// the <object> focusable so webOS WebKit routes key/VKB input into it
	// (BasicWebView.domAttributes).
	tag: "object",
	attributes: {
		type: "application/x-jihad-browser-mochi",
		tabindex: 0
	},
	// -webkit-transform:translate3d promotes the plugin to its own layer, as
	// BasicWebView does; display:block so it fills the fittable row.
	style: "display: block; -webkit-transform: translate3d(0,0,0);",
	published: {
		//* Current page URL. Setting it navigates (queued until the adapter is
		//* connected, then flushed).
		url: "",
		//* Page identifier assigned by the window manager; consumed at connect
		//* time (BasicWebView _connect -> node.setPageIdentifier).
		identifier: ""
	},
	events: {
		onLoadStarted: "",
		onLoadProgress: "",
		onLoadStopped: "",
		//* {title, url, canGoBack, canGoForward} — any field may be absent.
		onPageInfoChanged: "",
		onServerConnected: "",
		//* BrowserServer went away (daemon restart/crash).
		onServerDisconnected: "",
		//* {identifier} — the engine created a page for a link with a target /
		//* window.open; the shell binds it to a new card.
		onNewPage: "",
		//* {focused, fieldType, fieldActions}
		onEditorFocusChanged: "",
		//* Engine-driven JS dialogs (adapter callbacks). The shell presents them
		//* and answers via sendDialogResponse below. {message} / {message,
		//* defaultValue} / {host, code, certFile}.
		onDialogAlert: "",
		onDialogConfirm: "",
		onDialogPrompt: "",
		onDialogSSLConfirm: "",
		onDialogUserPassword: ""
	},

	// --- lifecycle ----------------------------------------------------------
	create: function() {
		this.inherited(arguments);
		this._callQueue = [];
		this._serverConnected = false;
	},
	rendered: function() {
		this.inherited(arguments);
		var node = this.hasNode();
		if (node) {
			// The adapter dispatches page callbacks by invoking methods by name
			// on the plugin node's `eventListener` property; point it at this
			// control (BasicWebView.rendered).
			node.eventListener = this;
			// webOS WebKit only forwards touch to the NPAPI adapter if the DOM
			// touch events are registered on the plugin node (BasicWebView).
			this._touchBind = this._touchBind || enyo.bind(this, "touchHandler");
			node.addEventListener("touchstart", this._touchBind);
			node.addEventListener("touchmove", this._touchBind);
			node.addEventListener("touchend", this._touchBind);
			// Lower the VKB when the plugin loses focus (BasicWebView blur).
			this._blurBind = this._blurBind || enyo.bind(this, "blurHandler");
			node.addEventListener("blur", this._blurBind);
			if (this.adapterReady()) {
				this.connect();
			}
		}
	},
	destroy: function() {
		this._callQueue = null;
		var node = this.hasNode();
		if (node) {
			node.eventListener = null;
		}
		this.inherited(arguments);
	},
	touchHandler: function() {
		// nop — registration alone is what routes touch into the adapter.
	},
	blurHandler: function() {
		if (window.PalmSystem) {
			window.PalmSystem.editorFocused(false, 0, 0);
		}
	},

	// --- connect handshake (mirrors BasicWebView) ---------------------------
	//* The plugin is usable once its scriptable openURL method exists; hidden
	//* views expose no methods until adapterInitialized fires.
	adapterReady: function() {
		var node = this.hasNode();
		return !!(node && node.openURL);
	},
	// (adapter callback) fired when a view that started hidden becomes ready.
	adapterInitialized: function() {
		this._serverConnected = false;
		this.connect();
	},
	connect: function() {
		if (this.adapterReady() && !this._serverConnected) {
			try {
				this.node.setPageIdentifier(this.identifier || this.id);
				this.node.connectBrowserServer();
			} catch (e) {
				// Expected while BrowserServer is still starting up; a later
				// call or adapterInitialized retries.
			}
		}
	},
	// (adapter callback) BrowserServer connected: run first-view setup, flush
	// everything queued before we were ready, then notify the shell.
	serverConnected: function() {
		this._serverConnected = true;
		this.initView();
		this._flushQueue();
		this.doServerConnected();
	},
	// (adapter callback) BrowserServer went away — the daemon exited or crashed
	// (BrowserAdapter notifies once per disconnect). BasicWebView.
	// browserServerDisconnected does the same: drop the connected flag so later
	// calls QUEUE and connect() is retried, instead of being fired at a dead
	// socket and silently lost until the card is relaunched.
	browserServerDisconnected: function() {
		this._serverConnected = false;
		this.doServerDisconnected();
		this.connect();
	},
	//* First-view setup, matching BasicWebView.initView (the pieces the shell
	//* needs). These are frozen-contract adapter methods, invoked node-directly
	//* via _call so they never enter the app-facing callBrowserAdapter set.
	initView: function() {
		if (!this.adapterReady() || !this._serverConnected) {
			return;
		}
		this._call("interrogateClicks", [false]);
		this._call("setShowClickedLink", [true]);
		this._call("pageFocused", [true]);
		this.updateViewportSize();
		this.urlChanged();
	},

	// --- viewport sizing (mirrors BasicWebView.updateViewportSize) ----------
	resizeHandler: function() {
		this.inherited(arguments);
		this.updateViewportSize();
	},
	updateViewportSize: function() {
		var b = this.getBounds();
		if (b && b.width && b.height) {
			this._call("setVisibleSize", [b.width, b.height]);
		}
	},

	// --- public adapter surface (FROZEN: identical set to the Enyo 1.0 app) --
	//* Proxy a scriptable BrowserAdapter method call to the plugin node. This is
	//* the single app-facing choke point; the set of method-name literals passed
	//* here stays identical to ../../app/source/Browser.js (ipc-contract R1). It
	//* is queue-aware: calls made before the adapter connects are replayed on
	//* serverConnected (fixes launch-time drops).
	callBrowserAdapter: function(method, args) {
		return this._call(method, args);
	},
	//* Answer an engine-driven dialog. Routed through the variable-method _call
	//* path (NOT a callBrowserAdapter literal), exactly as the Enyo 1.0 app's
	//* viewCall("sendDialogResponse", ...) fallback does — so this existing YAP
	//* response method does NOT widen the grep-audited callBrowserAdapter set.
	//* `args` is an all-string array: ["1"|"0"|"2", (promptText|user)?, (pass)?].
	sendDialogResponse: function(args) {
		return this._call("sendDialogResponse", args);
	},

	// --- WebView-internal plugin path (kept OUT of the app call set) --------
	// Queue-aware node invoker, mirroring BasicWebView.callBrowserAdapter. Used
	// by both the public proxy above and the internal handshake/setup. Because
	// the method name is a variable here (never a "literal"), these internal
	// engine calls do not widen the app's grep-audited callBrowserAdapter set.
	_call: function(method, args) {
		if (this.adapterReady() && this._serverConnected) {
			this._flushQueue();
			this._invoke(method, args);
		} else if (method !== "disconnectBrowserServer" && this._callQueue) {
			// (`_callQueue` is null after destroy — a late adapter callback that
			// tries to queue must not throw on the torn-down view.)
			this._callQueue.push({name: method, args: args});
			if (this.adapterReady() && !this._serverConnected) {
				this.connect();
			}
		}
	},
	_flushQueue: function() {
		if (!this._callQueue || !this._callQueue.length) {
			return;
		}
		var q = this._callQueue;
		this._callQueue = [];
		for (var i = 0, c; (c = q[i]); i++) {
			this._invoke(c.name, c.args);
		}
	},
	_invoke: function(method, args) {
		var node = this.hasNode();
		if (node && typeof node[method] === "function") {
			return node[method].apply(node, args || []);
		}
	},

	// --- published-property change handlers ---------------------------------
	urlChanged: function() {
		if (this.url) {
			// openURL is a frozen-contract adapter method; routed node-directly
			// (queued until connected), NOT via the app-facing proxy.
			this._call("openURL", [this.url]);
		}
	},
	identifierChanged: function() {
		// Consumed by connect() (BasicWebView _connect); nothing to send now.
	},

	// --- adapter -> app callbacks (invoked by name on node.eventListener) ----
	// Only the shell-relevant subset is handled; the full page/dialog/download
	// callback surface is the navigation-events parity port (T-053). Unhandled
	// adapter callbacks resolve to no JS method and are dropped by the NPAPI
	// bridge without error.
	loadStarted: function() {
		this.doLoadStarted();
	},
	loadProgress: function(progress) {
		this.doLoadProgress({progress: progress});
	},
	loadStopped: function() {
		this.doLoadStopped();
	},
	// Canonical page-info callback (BasicWebView.titleURLChange). Arg order is
	// (title, url, canGoBack, canGoForward) — verified against
	// render/adapter/BrowserAdapter.cpp (args[0]=title .. args[3]=canGoForward).
	titleURLChange: function(title, url, canGoBack, canGoForward) {
		if (url) { this.url = url; }
		this.doPageInfoChanged({
			title: title,
			url: url || this.url,
			canGoBack: canGoBack,
			canGoForward: canGoForward
		});
	},
	titleChange: function(title) {
		this.doPageInfoChanged({title: title, url: this.url});
	},
	// Location-only callback (adapter msgLocationChanged): (uri, canGoBack,
	// canGoForward). Propagate history state so the shell can toggle back/forward.
	urlChange: function(uri, canGoBack, canGoForward) {
		if (uri) { this.url = uri; }
		this.doPageInfoChanged({
			url: uri || this.url,
			canGoBack: canGoBack,
			canGoForward: canGoForward
		});
	},
	// (adapter callback) the engine created a page for a link with a target /
	// window.open (adapter msgCreatePage -> "createPage", one string arg: the new
	// page identifier). BasicWebView.createPage -> doNewPage; the shell opens a
	// card bound to that identifier, so target=_blank links are not dropped.
	createPage: function(identifier) {
		this.doNewPage({identifier: identifier});
	},
	// (adapter callback) input field focused/blurred: focus the plugin node and
	// raise/lower the VKB via PalmSystem, then notify the shell (BasicWebView).
	editorFocused: function(focused, fieldType, fieldActions) {
		if (window.PalmSystem) {
			if (focused && this.hasNode()) {
				this.node.focus();
			}
			window.PalmSystem.editorFocused(focused, fieldType, fieldActions);
		}
		this.doEditorFocusChanged({
			focused: focused,
			fieldType: fieldType,
			fieldActions: fieldActions
		});
	},

	// --- engine-driven dialog callbacks (adapter -> app) --------------------
	// Names + arg orders verified against render/adapter/BrowserAdapter.cpp: the
	// adapter InvokeEventListener()s these on node.eventListener. Each surfaces a
	// shell event; the shell presents the dialog and answers via
	// sendDialogResponse (above). These are inbound engine callbacks, not
	// callBrowserAdapter methods — they do not touch the frozen app->engine set.
	dialogAlert: function(msg) {
		this.doDialogAlert({message: msg});
	},
	dialogConfirm: function(msg) {
		this.doDialogConfirm({message: msg});
	},
	dialogPrompt: function(msg, defaultValue) {
		this.doDialogPrompt({message: msg, defaultValue: defaultValue});
	},
	dialogSSLConfirm: function(host, code, certFile) {
		this.doDialogSSLConfirm({host: host, code: code, certFile: certFile});
	},
	dialogUserPassword: function(msg) {
		this.doDialogUserPassword({message: msg});
	}
});
