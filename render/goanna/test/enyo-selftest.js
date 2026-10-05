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
		// The settings page (about:preferences), which since 2026-10-05 carries what the card's
		// own panel used to. Opened from the app menu's own handler, then driven with REAL CLICKS
		// through the adapter — clickAt is what a tap on the page becomes.
		//   settings                 open it and report the card's cache of its settings
		//   settings:x,y;x,y;...     ...then click each point (view pixels), 3 s apart. A step
		//                            written sN picks entry N of the drop-down list the previous
		//                            click opened; wN waits N seconds; b adds a test bookmark; ? reports.
		settings: function(app, name) {
			var spec = String(name).indexOf(":") > 0 ? String(name).slice(String(name).indexOf(":") + 1) : "";
			var steps = spec ? spec.split(";") : [];
			function count(kind, cb) {
				var b = new PalmServiceBridge();
				b.onservicecallback = function(p) { var r = null; try { r = enyo.json.parse(p); } catch (e) {} cb(r && r.results ? r.results.length : -1); };
				b.call("palm://com.palm.db/find", enyo.json.stringify({query: {from: kind}}));
				(window.__jihadTestBridges = window.__jihadTestBridges || []).push(b);
			}
			function state(tag, then) {
				count("net.riverstonerelay.jihad-browser.bookmarks:1", function(nb) {
					count("net.riverstonerelay.jihad-browser.history:1", function(nh) {
						var t = document.getElementById("jihad-toast");
						log(name, tag + ": view=" + app.$.pane.getViewName() + " url=" + String(app.$.browser.getUrl()).split("#")[0] +
							" engine=" + enyo.jihadChrome.loadSearchEngine() + " defaultSearch=" + app.getDefaultSearch() +
							" home=" + enyo.jihadChrome.loadHome() + " bookmarks=" + nb + " history=" + nh +
							" toast=[" + (t ? t.textContent : "") + "]");
						then();
					});
				});
			}
			function step(i) {
				if (i >= steps.length) { later(3000, function() { state("end", function() { log(name, "DONE"); }); }); return; }
				var st = steps[i], wait = 3000, view = app.$.browser.$.view;
				if (st.charAt(0) === "w") {
					wait = (parseInt(st.slice(1), 10) || 1) * 1000;
				} else if (st === "b") {              // a bookmark of the test's own, to have one to clear
					var bb = new PalmServiceBridge();
					bb.call("palm://com.palm.db/put", enyo.json.stringify({objects: [{_kind: "net.riverstonerelay.jihad-browser.bookmarks:1", title: "jihad test bookmark", url: "http://example.invalid/jihad-test", date: (new Date()).getTime()}]}));
					window.__jihadTestBridges.push(bb);
					log(name, "test bookmark added");
				} else if (st === "?") {              // report the state now
					state("now", function() {});
				} else if (st.charAt(0) === "s") {
					// The framework's own drop-down (enyo.WebView's PopupList): choose and close,
					// which is what a tap on one of its rows does.
					var want = parseInt(st.slice(1), 10), pop = null, c = view.getComponents();
					for (var k = 0; k < c.length; k++) { if (c[k].name && c[k].name.indexOf("select-") === 0 && c[k].isOpen) { pop = c[k]; } }
					log(name, "list open=" + Boolean(pop) + " items=" + (pop && pop.items ? pop.items.length : "?") + " pick " + want);
					if (pop) { view.selectPopupSelect(pop, want); pop.close(); }
				} else {
					var xy = st.split(",");
					view.callBrowserAdapter("clickAt", [parseInt(xy[0], 10), parseInt(xy[1], 10), 1]);
					log(name, "click " + st);
				}
				later(wait, function() { step(i + 1); });
			}
			app.openSettings();
			later(8000, function() { state("opened", function() { step(0); }); });
		},
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
		// The app menu's Print entry as a finger finds it: is it enabled once a page has loaded?
		// Reports what the menu's own enable rule sees, 4 s and 12 s after the hook starts.
		menu: function(app, name) {
			function look(tag) {
				try {
					var b = app.$.browser, bar = b && b.$.actionbar;
					log(name, tag + ": view=" + app.$.pane.getViewName() + " progress=" + (bar ? bar.getProgress() : "?") +
						" lastProgress=" + (b ? b._lastProgress : "?") + " url=" + (b ? b.getUrl() : "?"));
					app.$.appMenu.open();                        // the real menu: builds its items, runs its enable rule
					log(name, tag + ": menu open; printDisabled=" + app.$.printMenuItem.getDisabled());
				} catch (e) { log(name, tag + ": THREW " + e); }
			}
			later(4000, function() { look("4s"); });
			later(12000, function() { try { app.$.appMenu.close(); } catch (e) {} look("12s"); log(name, "DONE"); });
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
