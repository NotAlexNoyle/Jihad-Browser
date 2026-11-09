// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// LoadErrorView — the card's own page for a load that failed (T-600, from the Atlas app's 0.9.6 `e148635`; the
// owner's "yes, all three shells", 2026-11-09). isis raised a modal "Unable to Load Page" dialog over whatever was on
// screen; this panel takes the page view's place instead: the address, what went wrong in plain words, Try Again, and
// the Wi-Fi settings where the cause is the network. It lies OVER the web view (the view is a plugin node: hiding it
// would tear the BrowserAdapter instance down), sized to the view's bounds when shown, and leaves when a load commits.
//
// The daemon reports a failed load with the engine's own status (msgFailedLoad, an NS_ERROR_* code, negative as the
// adapter hands it over); describe() turns the codes a network failure produces into the words.
enyo.kind({
	name: "LoadErrorView",
	kind: enyo.VFlexBox,
	className: "jihad-load-error",
	showing: false,
	style: "position:absolute; left:0; right:0; z-index:40; background:#f2f2f2;",
	events: {
		onRetry: "",
		onOpenSettings: ""
	},
	components: [
		{kind: "Scroller", flex: 1, components: [
			{className: "jihad-load-error-box", components: [
				{name: "title", className: "jihad-load-error-title", content: $L("This page could not be loaded")},
				{name: "message", className: "jihad-load-error-message"},
				{name: "hint", className: "jihad-load-error-hint"},
				{name: "address", className: "jihad-load-error-address"},
				{kind: "HFlexBox", className: "jihad-load-error-buttons", components: [
					{name: "retryButton", kind: "Button", className: "enyo-button-affirmative", caption: $L("Try Again"), onclick: "retryClick"},
					{name: "settingsButton", kind: "Button", caption: $L("Wi-Fi Settings"), showing: false, onclick: "settingsClick"}
				]}
			]}
		]}
	],
	//* The words for a code: {message, hint, settings} — settings names the system app that fixes the cause, or null.
	describe: function(inCode, inUrl) {
		var c = inCode >>> 0;   // the engine's nsresult as an unsigned word
		var net = function(m, h) { return {message: m, hint: h, settings: "com.palm.app.wifi"}; };
		switch (c) {
			case 0x804B001E: return net($L("The server's name could not be found."), $L("Check the address, or the network connection."));
			case 0x804B000D: return net($L("The server refused the connection."), $L("The site may be down, or a port in the address is wrong."));
			case 0x804B000E: return net($L("The server took too long to answer."), $L("Try again in a moment, or check the network connection."));
			case 0x804B0014: case 0x804B0047: return net($L("The connection was interrupted."), $L("Try again; if it keeps happening, check the network connection."));
			case 0x804B0010: return net($L("This device is offline."), $L("Turn Wi-Fi on, then try again."));
			case 0x804B0048: return net($L("The proxy refused the connection."), $L("Check the proxy in the network settings."));
			case 0x804B0012: return {message: $L("This kind of address is not understood."), hint: $L("Only web addresses (http, https) open here."), settings: null};
			case 0x80520012: return {message: $L("The file does not exist."), hint: "", settings: null};
			case 0x804B0013: return {message: $L("The address is not valid."), hint: $L("Check it for a typing mistake."), settings: null};
			case 0x804B0002: return {message: $L("The load was stopped."), hint: "", settings: null};
		}
		return {message: $L("The page could not be loaded."), hint: $L("Try again; if it keeps happening, check the network connection."), settings: "com.palm.app.wifi"};
	},
	//* Show the panel for a failed load over the view (inBounds: the view's {top, left, width, height}).
	setError: function(inCode, inUrl, inBounds) {
		var d = this.describe(inCode, inUrl);
		this.$.message.setContent(d.message);
		this.$.hint.setContent(d.hint);
		this.$.address.setContent(enyo.string.escapeHtml(String(inUrl || "")));
		this.settingsApp = d.settings;
		this.$.settingsButton.setShowing(!!d.settings);
		if (inBounds) { this.applyStyle("top", inBounds.top + "px"); this.applyStyle("height", inBounds.height + "px"); }
		this.setRetrying(false);
		this.show();
	},
	setRetrying: function(inOn) {
		this.$.retryButton.setCaption(inOn ? $L("Trying…") : $L("Try Again"));
		this.$.retryButton.setDisabled(!!inOn);
	},
	retryClick: function() {
		this.setRetrying(true);
		this.doRetry();
	},
	settingsClick: function() {
		this.doOpenSettings(this.settingsApp);
	}
});
