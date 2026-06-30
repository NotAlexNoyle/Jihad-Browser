// Copyright 2026 the Jihad Browser project.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// Mochi (Enyo 2) UI variant of Jihad Browser — main application kind.
//
// SCAFFOLD ONLY. This sketches the structure of the Mochi front-end so the
// build/plan has a concrete home; it is NOT the finished UI. The full port to
// feature parity with the Enyo-1.0 app (../../app) is specified in
// context/kits/cavekit-mochi-ui.md and tracked in the build site.
//
// Contract invariant: this UI MUST drive the engine only through the existing
// BrowserAdapter / BrowserServer YAP contract — identical callBrowserAdapter
// method set and palm://com.palm.browserServer/* URIs as the Enyo-1.0 app.
// See ../../docs/IPC-CONTRACT.md and cavekit-ipc-contract.md.

enyo.kind({
	name: "JihadBrowser",
	kind: "FittableRows",
	classes: "jihad enyo-fit",
	components: [
		// Top action bar built from Mochi controls (Header + IconButtons + Input).
		{kind: "mochi.Header", name: "actionBar", components: [
			{kind: "mochi.IconButton", name: "back",    src: "images/menu-icon-back.png",    ontap: "goBack"},
			{kind: "mochi.IconButton", name: "forward", src: "images/menu-icon-forward.png", ontap: "goForward"},
			{kind: "mochi.IconButton", name: "reload",  src: "images/menu-icon-refresh.png", ontap: "reloadPage"},
			{kind: "mochi.InputDecorator", fit: true, components: [
				{kind: "mochi.Input", name: "address", placeholder: "Search or type a URL", onchange: "addressEntered"}
			]},
			{kind: "mochi.IconButton", name: "menu", src: "images/button-menu.png", ontap: "openMenu"}
		]},
		// Load progress (Mochi ProgressBar), shown during navigation.
		{kind: "mochi.ProgressBar", name: "progress", showing: false},
		// The rendered web content. TODO: an Enyo-2 WebView control bound to the
		// BrowserAdapter NPAPI plugin (the Enyo-1.0 app uses a "WebView" kind;
		// the Enyo-2 equivalent + adapter bridge is a port task — see kit R3).
		{name: "view", fit: true /*, kind: "JihadWebView" */}
	],

	// --- navigation: forwarded verbatim to the BrowserAdapter contract ---
	goBack:      function() { this.callAdapter("goBack"); },
	goForward:   function() { this.callAdapter("goForward"); },
	reloadPage:  function() { this.callAdapter("reloadPage"); },
	stopLoad:    function() { this.callAdapter("stopLoad"); },
	addressEntered: function(inSender) { /* TODO: normalize + openUrl via view */ },
	openMenu:    function() { /* TODO: Mochi ContextualPopup menu */ },

	// Single choke point so the adapter contract stays identical to the Enyo app.
	callAdapter: function(method, args) {
		// TODO: delegate to the WebView control's callBrowserAdapter(method, args).
	}

	// TODO (parity with ../../app, per cavekit-mochi-ui.md R2):
	//   bookmarks, history, downloads, find-in-page, preferences, start page,
	//   certificate detail, share, and the alert/confirm/prompt/SSL dialogs.
});
