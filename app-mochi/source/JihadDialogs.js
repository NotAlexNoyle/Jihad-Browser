// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../LICENSE.
//
// JihadDialogs — the Enyo 2 / Mochi port of the engine-driven dialog set in
// ../../app/source/Browser.js (alert / confirm / prompt / user-password / SSL
// confirm). The BrowserAdapter fires these as callbacks on the WebView node's
// eventListener; JihadWebView surfaces them as events, JihadBrowser routes them
// here for presentation, and every answer bubbles back as onDialogAnswer with the
// exact argument list the adapter's scriptable sendDialogResponse expects
// (verified against render/adapter/BrowserAdapter.cpp js_sendDialogResponse):
//
//     arg0 = "1" accept / "0" cancel  (SSL: "1" trust-always, "2" trust-once,
//                                      "0" don't-trust)
//     arg1 = prompt text   OR  username   (optional)
//     arg2 = password                      (optional)
//
// All args are strings (the adapter rejects non-string variants). The shell hands
// the array to JihadWebView.sendDialogResponse, which writes it back down the
// same YAP response pipe the Enyo 1.0 app uses — the IPC contract is unchanged.
//
// PRESENTATION: each dialog is a plain in-app overlay (a Control toggled by
// `showing`, over a scrim), NOT a mochi.Popup — a floating/modal mochi.Popup
// crashes the app card on this engine (Goanna/ESR52 host). The overlay pattern is
// the same one the parity list-view panels use, and renders correctly.

enyo.kind({
	name: "JihadDialogs",
	kind: "enyo.Control",
	events: {
		//* {args: [String, ...]} — passed verbatim to node.sendDialogResponse.
		onDialogAnswer: ""
	},
	components: [
		// Alert: message + OK only (Enyo 1.0 alertDialog has no cancel).
		{name: "alert", classes: "jihad-modal-overlay", showing: false, components: [
			{classes: "jihad-dialog-box", components: [
				{name: "alertMsg", classes: "jihad-dialog-message"},
				{classes: "jihad-dialog-buttons", components: [
					{classes: "jihad-btn", content: "OK", ontap: "alertOk"}
				]}
			]}
		]},
		// Confirm: message + OK / Cancel.
		{name: "confirm", classes: "jihad-modal-overlay", showing: false, components: [
			{classes: "jihad-dialog-box", components: [
				{name: "confirmMsg", classes: "jihad-dialog-message"},
				{classes: "jihad-dialog-buttons", components: [
					{classes: "jihad-btn jihad-btn-alt", content: "Cancel", ontap: "confirmCancel"},
					{classes: "jihad-btn", content: "OK", ontap: "confirmOk"}
				]}
			]}
		]},
		// Prompt: message + input + OK / Cancel.
		{name: "prompt", classes: "jihad-modal-overlay", showing: false, components: [
			{classes: "jihad-dialog-box", components: [
				{name: "promptMsg", classes: "jihad-dialog-message"},
				{kind: "mochi.InputDecorator", classes: "jihad-dialog-inputbox", components: [
					{kind: "mochi.Input", name: "promptInput", classes: "jihad-dialog-input",
						attributes: {autocapitalize: "off", autocorrect: "off", spellcheck: "false"}}
				]},
				{classes: "jihad-dialog-buttons", components: [
					{classes: "jihad-btn jihad-btn-alt", content: "Cancel", ontap: "promptCancel"},
					{classes: "jihad-btn", content: "OK", ontap: "promptOk"}
				]}
			]}
		]},
		// User/password auth: message + username + password + OK / Cancel.
		{name: "login", classes: "jihad-modal-overlay", showing: false, components: [
			{classes: "jihad-dialog-box", components: [
				{name: "loginMsg", classes: "jihad-dialog-message"},
				{kind: "mochi.InputDecorator", classes: "jihad-dialog-inputbox", components: [
					{kind: "mochi.Input", name: "userInput", placeholder: "Username", classes: "jihad-dialog-input",
						attributes: {autocapitalize: "off", autocorrect: "off", spellcheck: "false"}}
				]},
				{kind: "mochi.InputDecorator", classes: "jihad-dialog-inputbox", components: [
					{kind: "mochi.Input", name: "passInput", placeholder: "Password", classes: "jihad-dialog-input",
						type: "password", attributes: {autocapitalize: "off", autocorrect: "off", spellcheck: "false"}}
				]},
				{classes: "jihad-dialog-buttons", components: [
					{classes: "jihad-btn jihad-btn-alt", content: "Cancel", ontap: "loginCancel"},
					{classes: "jihad-btn", content: "OK", ontap: "loginOk"}
				]}
			]}
		]},
		// SSL confirm: message + View Certificate + Trust Always / Trust Once / Don't Trust.
		// The certificate is read while this dialog is OPEN: the daemon writes the server's
		// certificate to a file for the duration of the question and deletes it when the
		// question is answered, so the details are fetched as soon as the dialog shows and
		// the button only reveals what was fetched.
		{name: "ssl", classes: "jihad-modal-overlay", showing: false, components: [
			{classes: "jihad-dialog-box jihad-dialog-ssl", components: [
				{name: "sslMsg", classes: "jihad-dialog-message"},
				{name: "sslCert", classes: "jihad-cert", showing: false, allowHtml: false},
				{name: "sslView", classes: "jihad-cert-link", content: "View Certificate", ontap: "sslViewTapped", showing: false},
				{classes: "jihad-dialog-buttons", components: [
					{classes: "jihad-btn jihad-btn-alt", content: "Don't Trust", ontap: "sslDontTrust"},
					{classes: "jihad-btn", content: "Trust Once", ontap: "sslTrustOnce"},
					{classes: "jihad-btn", content: "Trust Always", ontap: "sslTrustAlways"}
				]}
			]}
		]}
	],

	// --- presentation (called by the shell from the WebView dialog events) ---
	showAlert: function(msg) {
		this.$.alertMsg.setContent(msg || "");
		this.$.alert.show();
	},
	showConfirm: function(msg) {
		this.$.confirmMsg.setContent(msg || "");
		this.$.confirm.show();
	},
	showPrompt: function(msg, defaultValue) {
		this.$.promptMsg.setContent(msg || "");
		this.$.promptInput.setValue(defaultValue || "");
		this.$.prompt.show();
	},
	showLogin: function(msg) {
		// A proxy's login (a 407) comes as "[proxy] <host:port> - <realm>" (cavekit-proxies-certs.md R5).
		var proxy = /^\[proxy\] /.test(String(msg || ""));
		this.$.loginMsg.setContent(msg ? ((proxy ? "The proxy " + String(msg).slice(8) : "The server " + msg) +
			" requires a username and password") : "Authentication required");
		this.$.userInput.setValue("");
		this.$.passInput.setValue("");
		this.$.login.show();
	},
	showSSL: function(host, code, certFile) {
		this._sslCertFile = certFile;
		this._sslCertText = "";
		this.$.sslMsg.setContent(this.sslMessage(host, code));
		this.$.sslCert.hide();
		this.$.sslView.hide();
		this.$.ssl.show();
		if (certFile) {
			var self = this, asked = certFile;
			enyo.jihad.certDetails(certFile, function(resp) {
				// A reply for a certificate that is no longer the one on screen is dropped.
				if (self._sslCertFile !== asked) { return; }
				var text = self.certText(resp);
				self._sslCertText = text;
				self.$.sslView.setShowing(Boolean(text));
			});
		}
	},
	sslViewTapped: function() {
		var open = !this.$.sslCert.getShowing();
		this.$.sslCert.setContent(this._sslCertText || "");
		this.$.sslCert.setShowing(open);
		this.$.sslView.setContent(open ? "Hide Certificate" : "View Certificate");
		return true;
	},
	//* The certificate manager's reply as readable lines — the fields the Enyo certificate
	//* view shows (CertificateDetail.js): who it was issued to and by, validity, serial,
	//* version and algorithms. Anything the reply lacks is left out, and a reply with nothing
	//* usable gives "" (the button then never appears).
	certText: function(resp) {
		if (!resp || resp.returnValue === false) { return ""; }
		var lines = [];
		function party(label, p) {
			if (!p) { return; }
			var bits = [], keys = ["commonname", "organization", "unit", "location", "state", "country", "altname"];
			for (var i = 0; i < keys.length; i++) {
				if (p[keys[i]]) { bits.push(String(p[keys[i]])); }
			}
			if (bits.length) { lines.push(label + ": " + bits.join(", ")); }
		}
		party("Issued to", resp.subject);
		party("Issued by", resp.issuer);
		if (resp.startdate)    { lines.push("Valid from: " + resp.startdate); }
		if (resp.expiredate)   { lines.push("Valid until: " + resp.expiredate); }
		if (resp.serialNumber !== undefined && resp.serialNumber !== "") { lines.push("Serial number: " + resp.serialNumber); }
		if (resp.version !== undefined && resp.version !== "")           { lines.push("Version: " + resp.version); }
		if (resp.sha256) { lines.push("SHA-256 fingerprint: " + resp.sha256); }
		if (resp.signature && resp.signature.algorithm) { lines.push("Signature: " + resp.signature.algorithm); }
		if (resp.publicKey && resp.publicKey.algorithm) { lines.push("Public key: " + resp.publicKey.algorithm); }
		return lines.join("\n");
	},
	//* SSL warning text keyed on the adapter's error code bands (Browser.js).
	sslMessage: function(host, code) {
		var name = host || "this site";
		var why;
		if (code === 0) {
			why = "is expired";
		} else if (code >= 2 && code < 5) {
			why = "was not sent";
		} else if (code >= 5 && code < 10) {
			why = "could not be read completely";
		} else if (code >= 10 && code < 18) {
			why = "has some invalid information";
		} else if (code >= 18 && code < 24) {
			why = "has questionable signatures";
		} else if (code >= 24 && code < 30) {
			why = "is invalid";
		} else {
			why = "has inconsistent information in it";
		}
		return "The security certificate " + name + " sent " + why +
			". Connecting to this site might put your confidential information at risk.";
	},

	// --- answers -> onDialogAnswer({args}) -> node.sendDialogResponse ---------
	answer: function(popup, args) {
		popup.hide();
		this.doDialogAnswer({args: args});
	},
	alertOk:       function() { this.answer(this.$.alert, ["1"]); },
	confirmOk:     function() { this.answer(this.$.confirm, ["1"]); },
	confirmCancel: function() { this.answer(this.$.confirm, ["0"]); },
	promptOk:      function() { this.answer(this.$.prompt, ["1", this.$.promptInput.getValue() || ""]); },
	promptCancel:  function() { this.answer(this.$.prompt, ["0"]); },
	loginOk:       function() { this.answer(this.$.login, ["1", this.$.userInput.getValue() || "", this.$.passInput.getValue() || ""]); },
	loginCancel:   function() { this.answer(this.$.login, ["0"]); },
	sslTrustAlways: function() { this._sslCertFile = null; this.answer(this.$.ssl, ["1"]); },
	sslTrustOnce:   function() { this._sslCertFile = null; this.answer(this.$.ssl, ["2"]); },
	sslDontTrust:   function() { this._sslCertFile = null; this.answer(this.$.ssl, ["0"]); }
});
