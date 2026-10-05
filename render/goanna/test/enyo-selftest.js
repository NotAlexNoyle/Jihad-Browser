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
		// Print: open the dialog from the menu and report what the daemon finds. With an
		// argument (an address, or address:port) that address is typed into the dialog's own
		// field, searched for, and the page is printed on the printer found there. Point it at
		// render/goanna/test/ipp-sink.py, NOT at a real printer:  run print:192.168.1.245:8090
		print: function(app, name) {
			app.printClick();
			var d = app.$.printDialog, OPEN = d.isOpen;
			var want = String(name).indexOf(":") > 0 ? String(name).slice(String(name).indexOf(":") + 1) : "";
			function rows() {
				var out = [], l = d._printers || [];
				for (var i = 0; i < l.length; i++) { out.push(l[i].name + " @ " + l[i].address + ":" + l[i].port + (l[i].tls ? " tls" : "")); }
				return out.join(" ; ");
			}
			log(name, "dialog open=" + OPEN + " status=[" + d.$.status.getContent() + "]");
			later(16000, function() {
				log(name, "after search: status=[" + d.$.status.getContent() + "] printers=[" + rows() + "]");
				if (!want) { log(name, "DONE"); return; }
				d.$.address.setValue(want);
				d.search();
				later(12000, function() {
					log(name, "after address search: status=[" + d.$.status.getContent() + "] printers=[" + rows() + "]");
					var idx = -1, l = d._printers || [];
					for (var i = 0; i < l.length; i++) { if (l[i].address + ":" + l[i].port === want || l[i].address === want) { idx = i; } }
					if (idx < 0) { log(name, "printer " + want + " is not in the list DONE"); return; }
					d.pick({printerIndex: idx});          // what a tap on that row does
					log(name, "picked " + l[idx].name + "; status=[" + d.$.status.getContent() + "]");
					later(22000, function() { log(name, "end: status=[" + d.$.status.getContent() + "] DONE"); });
				});
			});
		},
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
