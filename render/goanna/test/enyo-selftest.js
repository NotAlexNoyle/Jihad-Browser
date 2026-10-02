//   Copyright 2026 NotAlexNoyle.
//   Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// DEVICE TEST HOOK for the Enyo shell — NOT part of the app and never packaged. See
// mochi-selftest.js for what such a hook is for and what it cannot prove; this one is pushed
// by `build/webos-oe/card-selftest.sh enyo push` and runs ONE scenario, named by the launch
// parameter `jihadTest`, through the shell's own handlers.
(function() {
	function log(name, text) { enyo.log("[JIHAD-TEST] " + name + ": " + text); }
	function later(ms, fn) { window.setTimeout(fn, ms); }
	function j(o) { try { return enyo.json.stringify(o); } catch (e) { return String(o); } }
	function appRoot() {
		var c = (enyo.master && enyo.master.getComponents) ? enyo.master.getComponents() : [];
		for (var i = 0; i < c.length; i++) { if (c[i].kindName === "BrowserApp") { return c[i]; } }
		return enyo.$ && enyo.$.browserApp;
	}
	function flashQuery(cb) {
		var b = new PalmServiceBridge();
		b.onservicecallback = function(p) { cb(p); };
		b.call("palm://net.riverstonerelay.jihadBrowser/getFlash", "{}");
		window.__jihadTestBridge = b;
	}
	var scenarios = {
		// The card's settings panel: which rows it has, the Flash switch, More Settings.
		prefs: function(app, name) {
			app.preferencesClick();
			later(3500, function() {
				var p = app.$.pane.viewByName("preferences");
				log(name, "rows: blockPopups=" + Boolean(p.$.blockPopups) + " acceptCookies=" + Boolean(p.$.acceptCookies) +
					" enableJavascript=" + Boolean(p.$.enableJavascript) + " flashSwitch=" + Boolean(p.$.flashSwitch) +
					" flash state=" + (p.$.flashSwitch ? p.$.flashSwitch.getState() : "?") + " note=[" + p.$.flashNote.getContent() + "] DONE");
			});
		},
		flash: function(app, name, arg) {
			app.preferencesClick();
			later(3500, function() {
				var p = app.$.pane.viewByName("preferences"), want = (arg === "on");
				log(name, "before=" + p.$.flashSwitch.getState());
				p.$.flashSwitch.setState(want);
				p.togglePreferenceClick(p.$.flashSwitch, want);     // what a tap on the switch does
				later(3000, function() {
					flashQuery(function(payload) { log(name, "switch=" + p.$.flashSwitch.getState() + " daemon=" + payload + " DONE"); });
				});
			});
		},
		// The SSL warning must already be on screen (launch on a self-signed https target):
		// is View Certificate enabled, what does the certificate view show, then refuse.
		cert: function(app, name) {
			later(5000, function() {
				var b = app.$.browser;
				log(name, "viewCertButton disabled=" + b.$.viewCertButton.getDisabled());
				b.viewSSLCertificate();
				later(2500, function() {
					var d = b.$.sslCertDialog.$.certDetail;
					log(name, "issued to=[" + d.$.commonName.getContent() + "] issuer CN=[" + d.$.issuerCommonName.getContent() +
						"] from=[" + d.$.startDate.getContent() + "] until=[" + d.$.expirationDate.getContent() + "] serial=[" + d.$.serialNumber.getContent() + "]");
					later(6000, function() {
						b.$.sslCertDialog.close();
						b.closeSSLConfirmBox({response: "0"});
						log(name, "answered DONE");
					});
				});
			});
		},
		// More Settings in the panel: the settings page opens in the card.
		more: function(app, name) {
			app.preferencesClick();
			later(3000, function() {
				var p = app.$.pane.viewByName("preferences");
				p.openSettingsClick();
				later(5000, function() { log(name, "view=" + app.$.pane.getViewName() + " url=" + app.$.browser.getUrl() + " DONE"); });
			});
		}
	};
	function run() {
		var p = enyo.windowParams || null;
		if (!p || !p.jihadTest) { try { p = enyo.json.parse(window.PalmSystem.launchParams); } catch (e) { p = null; } }
		var spec = p && p.jihadTest, app = appRoot();
		if (!spec) { return; }
		var name = String(spec).split(":")[0], arg = String(spec).split(":")[1];
		if (!app) { log(spec, "no BrowserApp found DONE"); return; }
		if (!scenarios[name]) { log(spec, "no such scenario DONE"); return; }
		log(spec, "start");
		try { scenarios[name](app, spec, arg); } catch (e) { log(spec, "THREW " + e + " DONE"); }
	}
	window.setTimeout(run, 6000);
}());
