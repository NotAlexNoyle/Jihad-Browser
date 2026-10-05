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
				} else if (st.charAt(0) === "f") {    // fVX,VY: a flick, as the adapter's scroller gets it when a
					// finger lets go — the real kinetic scroll, at the adapter's own frame rate. (The
					// adapter's pageScaleAndScroll, which an earlier "z" step called, is an empty stub:
					// that step never moved anything, and what it was thought to prove it did not.)
					var fv = st.slice(1).split(",");
					view.callBrowserAdapter("handleFlick", [parseInt(fv[0], 10), parseInt(fv[1], 10)]);
					log(name, "flick " + st.slice(1));
					wait = 2500;
				} else if (st.charAt(0) === "d") {    // dX,Y,DX,DY[,VX,VY]: a drag from view X,Y by DX,DY as DOM
					// mouse events on the plugin node (what one finger becomes), in eight moves; with
					// VX,VY the release is a flick of that velocity. Whether the platform passes
					// script-made mouse events on to a plugin is itself something this step finds out.
					var dp = st.slice(1).split(","), dnode = view.hasNode(), drect = dnode.getBoundingClientRect();
					var dx0 = drect.left + parseInt(dp[0], 10), dy0 = drect.top + parseInt(dp[1], 10);
					var ddx = parseInt(dp[2], 10), ddy = parseInt(dp[3], 10), dn = 0;
					var dfire = function(type, x, y) {
						var e = document.createEvent("MouseEvents");
						e.initMouseEvent(type, true, true, window, 1, x, y, x, y, false, false, false, false, 0, null);
						dnode.dispatchEvent(e);
					};
					dfire("mousedown", dx0, dy0);
					(function dtick() {
						dn++;
						dfire("mousemove", dx0 + ddx * dn / 8, dy0 + ddy * dn / 8);
						if (dn < 8) { setTimeout(dtick, 30); return; }
						if (dp.length >= 6) { view.callBrowserAdapter("handleFlick", [parseInt(dp[4], 10), parseInt(dp[5], 10)]); }
						dfire("mouseup", dx0 + ddx, dy0 + ddy);
					})();
					log(name, "drag " + st.slice(1));
					wait = 2500;
				} else if (st.charAt(0) === "g") {    // gSCALE,X,Y: a pinch to SCALE about view X,Y, in eight
					// steps through the adapter's own gesture calls. The scale and rotation must be
					// non-integers: the adapter refuses an integer where it expects a double.
					var gp = st.slice(1).split(","), gTarget = parseFloat(gp[0]), gx = parseInt(gp[1], 10), gy = parseInt(gp[2], 10), gn = 0;
					view.callBrowserAdapter("gestureStart", [gx, gy, 1.0001, 0.0001, gx, gy]);
					(function gtick() {
						gn++;
						var sc = 1 + (gTarget - 1) * gn / 8 + 0.00001;
						if (gn < 8) { view.callBrowserAdapter("gestureChange", [gx, gy, sc, 0.0001, gx, gy]); setTimeout(gtick, 60); }
						else { view.callBrowserAdapter("gestureEnd", [gx, gy, sc, 0.0001, gx, gy]); }
					})();
					log(name, "pinch " + st.slice(1));
					wait = 3500;
				} else if (st === "A") {              // tap the padlock of the OTHER card of this app, from here.
					// Two cards are open; this one connected last, so the daemon takes it for the
					// focused one. The other card's own handler is called in the other card's
					// window (one process, so its objects are reachable; its own timers do not run
					// while it is in the background, which is why it cannot do this itself). The
					// certificate it gets must be ITS page's, not this card's.
					var wins = enyo.windows.getWindows(), other = null;
					for (var wk in wins) {
						try { if (wins[wk] !== window && wins[wk].enyo.$.browserApp) { other = wins[wk]; } } catch (e) {}
					}
					log(name, "other card: " + (other ? String(other.enyo.$.browserApp.$.browser.getUrl()).slice(0, 60) : "NOT FOUND") + " ; this card: " + String(app.$.browser.getUrl()).slice(0, 60));
					if (other) {
						other.enyo.$.browserApp.$.browser.showPageCertificate();
						later(6000, function() {
							var dlg = other.enyo.$.browserApp.$.browser.$.sslCertDialog, cd = dlg && dlg.$.certDetail;
							log(name, "the other card's certificate dialog: open=" + Boolean(dlg && dlg.isOpen) + " name=[" + (cd && cd.$.commonName ? cd.$.commonName.getContent() : "?") + "]");
						});
					}
				} else if (st === "F") {              // this card tells the daemon it does NOT have the focus
					// (the adapter's pageFocused(false)), so that the next thing to claim it — the
					// padlock — can be seen claiming it in the daemon's log.
					view.callBrowserAdapter("pageFocused", [false]);
					log(name, "said pageFocused(false)");
				} else if (st === "a") {              // answer OK to the engine dialog that is open (an alert)
					app.$.browser.viewCall("acceptDialog", []);
					log(name, "answered the engine's dialog");
				} else if (st === "x") {              // close this card (a second card that only needed to connect)
					log(name, "closing this card");
					window.close();
				} else if (st === "?") {              // report the state now
					state("now", function() {});
				} else if (st.charAt(0) === "h") {    // hX,Y: a long-press at view X,Y, from the point the
					// card has passed it on: the card is marked as waiting, the engine is sent the hold.
					// NOT a finger: it skips the adapter's own gesture timing and hit test.
					var hs = st.slice(1).split(",");
					app.$.browser._pageHold = {left: parseInt(hs[0], 10), top: parseInt(hs[1], 10) + 82, at: Date.now()};
					view.callBrowserAdapter("mouseHoldAt", [parseInt(hs[0], 10), parseInt(hs[1], 10)]);
					log(name, "hold " + st.slice(1));
				} else if (st.charAt(0) === "m") {    // m: list the open long-press menu; mN: choose entry N
					var cm = app.$.browser.$.context, its = cm.items || [], caps = [];
					for (var q = 0; q < its.length; q++) { caps.push(its[q].caption); }
					log(name, "menu open=" + Boolean(cm.isOpen) + " items=[" + caps.join(" | ") + "]");
					if (st.length > 1 && its[parseInt(st.slice(1), 10)]) {
						var it = its[parseInt(st.slice(1), 10)];
						log(name, "menu pick [" + it.caption + "]");
						app.$.browser.contextItemClick(cm, it.value, cm.tapInfo, cm.tapPosition);   // what a tap on the row does
						cm.close();
					}
				} else if (st.charAt(0) === "c") {    // c=TEXT: is TEXT what is on the clipboard now? Only the
					// answer and the length are logged — the clipboard is the user's, the log is kept.
					(function(want) {
						enyo.jihadGetClipboard(function(t) { log(name, "clipboard is the expected text: " + (String(t) === want) + " (length " + String(t).length + ", expected " + want.length + ")"); });
					})(st.slice(2));
				} else if (st.charAt(0) === "k") {    // kTEXT: put TEXT on the clipboard (to have something to paste)
					enyo.dom.setClipboard(st.slice(1));
					log(name, "clipboard set");
				} else if (st === "l") {              // the padlock: is it showing, and what does a click on it bring up?
					var lk = app.$.browser.$.actionbar.$.search.$.address.$.secureLock, ln = lk && lk.hasNode();
					log(name, "padlock showing=" + Boolean(lk && lk.showing));
					if (ln) {
						var le = document.createEvent("MouseEvents");
						le.initMouseEvent("click", true, true, window, 1, 0, 0, 0, 0, false, false, false, false, 0, null);
						ln.dispatchEvent(le);                    // through the framework's own click dispatch
					}
					later(6000, function() {
						var dlg = app.$.browser.$.sslCertDialog, cd = dlg && dlg.$.certDetail, f = function(n) { return (cd && cd.$[n]) ? cd.$[n].getContent() : "?"; };
						log(name, "certificate dialog open=" + Boolean(dlg && dlg.isOpen) + " name=[" + f("commonName") + "] issuer=[" + f("displayIssuerCommonName") +
							"] from=[" + f("startDate") + "] to=[" + f("expirationDate") + "] connection=[" + f("connection") + "] fingerprint chars=" + String(f("fingerprint")).length);
					});
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
			// `page:<steps>` runs the same steps on the page the card was launched on, without
			// opening the settings page first.
			if (String(name).indexOf("page") !== 0) { app.openSettings(); }
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
	scenarios.page = scenarios.settings;
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
