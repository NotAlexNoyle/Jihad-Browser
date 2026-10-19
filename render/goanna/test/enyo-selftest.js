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
		//                            click opened; wN waits N seconds; b adds a test bookmark; ? reports;
		//                            G reports the address fields' reload/stop glyphs (works on the start page).
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
						log(name, tag + ": view=" + app.$.pane.getViewName() + " url=" + (app.$.browser ? String(app.$.browser.getUrl()).split("#")[0] : "(no browser view yet)") +
							" engine=" + enyo.jihadChrome.loadSearchEngine() + " defaultSearch=" + app.getDefaultSearch() +
							" home=" + enyo.jihadChrome.loadHome() + " bookmarks=" + nb + " history=" + nh +
							" toast=[" + (t ? t.textContent : "") + "]");
						then();
					});
				});
			}
			function step(i) {
				if (i >= steps.length) { later(3000, function() { state("end", function() { log(name, "DONE"); }); }); return; }
				// (the browser view is lazy: on a card launched to the start page it does not exist yet)
				var st = steps[i], wait = 3000, view = app.$.browser && app.$.browser.$.view;
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
				} else if (st.charAt(0) === "t") {    // tX,Y: a double tap at view X,Y, as the adapter's smart zoom
					// gets it (handlePenDoubleClick ends in the same prvSmartZoom): the daemon finds the
					// block under the point and the adapter zooms to it, or back out when zoomed already
					var tp = st.slice(1).split(",");
					view.callBrowserAdapter("smartZoom", [parseInt(tp[0], 10), parseInt(tp[1], 10)]);
					log(name, "double tap " + st.slice(1));
					wait = 3500;
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
				} else if (st === "a") {              // answer OK to the engine dialog that is open: the alert ($.dialog) or the confirm ($.confirmDialog)
					var br = app.$.browser, al = br.$.dialog, cf = br.$.confirmDialog;
					var alOpen = !!(al && al.isOpen), cfOpen = !!(cf && cf.isOpen);
					log(name, "engine alert open=" + alOpen + " confirm open=" + cfOpen + (cfOpen ? " [" + String(cf.$.confirmMessage ? cf.$.confirmMessage.getContent() : "").slice(0, 120) + "]" : "") + ((alOpen || cfOpen) ? "; answering OK" : "; nothing to answer"));
					try {
						if (cfOpen) { cf.acceptClick(); }                                    // the popup's own Accept button handler
						else if (alOpen) { br.viewCall("acceptDialog", []); }
					} catch (e) { log(name, "answering THREW " + e); }
				} else if (st === "D") {              // report the engine dialogs that are open, answering none
					var db0 = app.$.browser, dal = db0.$.dialog, dcf = db0.$.confirmDialog, dss = db0.$.sslDialog;
					// (the messages belong to the Browser, which declares them, not to their popups)
					log(name, "dialogs: alert open=" + !!(dal && dal.isOpen) + (dal && dal.isOpen && db0.$.dialogMessage ? " [" + String(db0.$.dialogMessage.getContent()).slice(0, 140) + "]" : "") +
						" confirm open=" + !!(dcf && dcf.isOpen) + (dcf && dcf.isOpen && db0.$.confirmMessage ? " [" + String(db0.$.confirmMessage.getContent()).slice(0, 160) + "]" : "") +
						" ssl open=" + !!(dss && dss.isOpen));
				} else if (st.charAt(0) === "V") {    // VN: answer the SSL dialog as its button N does (1 Trust Always, 2 Trust Once, 0 Don't Trust)
					var vs = app.$.browser.$.sslDialog;
					log(name, "ssl dialog open=" + !!(vs && vs.isOpen) + "; answering " + st.slice(1));
					if (vs && vs.isOpen) { vs.response = st.slice(1); vs.close(); }
				} else if (st.charAt(0) === "W") {    // WUSER:PASS: fill the open login dialog (a 401/407) and press OK,
					// through the card's own fields and handler — what typing them and tapping OK does
					var wu = st.slice(1).split(":"), wb = app.$.browser;
					wb.$.userInput.setValue(wu[0] || "");
					wb.$.passwordInput.setValue(wu.slice(1).join(":"));
					log(name, "login dialog: [" + wb.$.loginMessage.getContent() + "] answered as " + wu[0]);
					wb.loginResponse(wb, true);
					wb.$.loginDialog.close();
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
				} else if (st === "K") {              // focus the address field as a tap does (the keyboard comes up), and report
					var kb = app.$.browser || app.$.startPage;
					var ka = kb && kb.$.actionbar && kb.$.actionbar.$.search && kb.$.actionbar.$.search.$.address;
					if (ka) { ka.forceFocus(); }
					log(name, "address field focused: " + !!(ka && document.activeElement && document.activeElement.tagName === "INPUT") + " window " + window.innerWidth + "x" + window.innerHeight);
				} else if (st === "RF") {             // refocus with a caret at 3: a tap's caret survives unless the field selects all on focus;
					// a bare input made here is the control (WebKit restores its caret)
					var yd = document, ya = yd.activeElement, yc = yd.createElement("input");
					yc.value = "control value";
					yc.style.cssText = "position:absolute;left:-2000px;top:0";
					yd.body.appendChild(yc);
					if (ya && ya.setSelectionRange) { ya.blur(); ya.setSelectionRange(3, 3); ya.focus(); }
					setTimeout(function () {
						var a = ya ? ya.selectionStart + "-" + ya.selectionEnd + " of " + String(ya.value).length : "none";
						yc.setSelectionRange(3, 3); yc.focus();
						setTimeout(function () {
							log(name, "refocused address: " + a + "; control input: " + yc.selectionStart + "-" + yc.selectionEnd + " of " + yc.value.length);
							yc.blur(); yc.parentNode.removeChild(yc);
						}, 800);
					}, 800);
				} else if (st === "S") {              // the focused field's selection: is the whole value selected?
					var se = document.activeElement;
					log(name, "selection: " + (se && typeof se.selectionStart === "number" ? se.selectionStart + "-" + se.selectionEnd + " of " + String(se.value).length + " [" + String(se.value).slice(0, 40) + "]" : "no text field focused"));
				} else if (st.charAt(0) === "I") {    // ITEXT: type TEXT into the page's focused field, as the card's paste does
					view.callBrowserAdapter("insertStringAtCursor", [st.slice(1)]);
					log(name, "typed into the page: " + st.slice(1).length + " characters");
				} else if (st === "H") {              // the keyboard put away by the system (the card's window grows back, as
					// when the user closes it), then the keyboard back in automatic mode
					var hb = window.innerHeight;
					enyo.keyboard.forceHide();
					later(2500, function () {
						log(name, "keyboard hidden: window " + hb + " -> " + window.innerHeight);
						enyo.keyboard.setManualMode(false);
					});
				} else if (st === "Y") {              // the page's title as the card has it
					log(name, "page title [" + (app.$.browser ? app.$.browser.title : "(no browser view)") + "] address [" +
						(app.$.browser ? String(app.$.browser.getUrl()).replace(/chrome=[^&]*/, "chrome=…") : "") + "]");
				} else if (st === "N") {              // what has the focus now, and the window's size (it shrinks under the keyboard)
					var ae = document.activeElement;
					log(name, "focus: " + (ae ? ae.tagName + (ae.className ? "." + String(ae.className).split(" ")[0] : "") : "none") + " window " + window.innerWidth + "x" + window.innerHeight);
				} else if (st.charAt(0) === "U") {    // navigate as a typed address does (setUrl); "L" right after reads the padlock at once
					(app.$.browser || app).setUrl(st.slice(1));   // from the start page, the app's own setUrl opens the browser view
					log(name, "navigate (typed) to " + st.slice(1));
					wait = 50;
				} else if (st === "Q") {              // bookmark the open page through the app's own Add Bookmark (BrowserApp.addBookmark)
					app.addBookmark();
					log(name, "bookmarked [" + app.$.browser.title + "] " + app.$.browser.url);
				} else if (st.charAt(0) === "J") {   // J or JMS: hold MS (default 1500) before the lift              // a FINGER-LIKE long-press: mousedown on the plugin node, held 1.2 s, then
					// mouseup + click (the lift a scripted hold never had) — is the menu still open after it?
					var jv = app.$.browser.$.view, jn = jv && (jv.hasNode() && (jv.hasNode().tagName === "OBJECT" ? jv.hasNode() : jv.hasNode().querySelector("object")));
					var jx = Math.round(window.innerWidth / 2), jy = Math.round(window.innerHeight / 2);
					var jat = /@(\d+),(\d+)$/.exec(st);   // J[MS]@X,Y: hold at window X,Y instead of the centre
					if (jat) { jx = parseInt(jat[1], 10); jy = parseInt(jat[2], 10); st = st.replace(/@.*$/, ""); }
					var jm = function (type) { var me = document.createEvent("MouseEvents"); me.initMouseEvent(type, true, true, window, 1, jx, jy, jx, jy, false, false, false, false, 0, null); if (jn) { jn.dispatchEvent(me); } };
					log(name, "finger hold at " + jx + "," + jy + " (plugin " + (jn ? "present" : "MISSING") + ")");
					jm("mousedown");
					setTimeout(function () {
						var ctx = app.$.browser.$.context, openBefore = !!(ctx && ctx.isOpen);
						jm("mouseup"); jm("click");
						setTimeout(function () { log(name, "finger hold: menu open before lift=" + openBefore + " after lift=" + !!(ctx && ctx.isOpen)); }, 600);
					}, parseInt(st.slice(1), 10) || 1500);
					wait = (parseInt(st.slice(1), 10) || 1500) + 2000;
				} else if (st === "P") {              // show the start page (a LAZY view: created now) and read its action bar's theme
					var hadSp = !!app.$.startPage;
					app.$.pane.selectViewByName("startPage");
					var spb = app.$.startPage && app.$.startPage.$.actionbar.hasNode();
					log(name, "start page (" + (hadSp ? "existed" : "created now") + ") bar: background=[" + (spb ? spb.style.background : "?").slice(0, 100) + "] color=[" + (spb ? spb.style.color : "?") + "]");
				} else if (st === "T") {              // the action bar's theme as painted (inline style set by applyTheme)
					var tb = app.$.browser.$.actionbar.hasNode();
					var ic0 = tb ? tb.querySelector(".enyo-button-icon") : null;
					var cs = tb ? window.getComputedStyle(tb, null) : null, th = app._theme;
					log(name, "theme given: layers=" + (th && th.layers ? Object.prototype.toString.call(th.layers) + "/" + th.layers.length : "none") +
						" colors=" + (th && th.colors ? "yes" : "no") + " headerPosition=" + (th ? th.headerPosition : "?") + " inline image=[" + (tb ? tb.style.backgroundImage : "?").replace(/file:\/\/\/[^)]*\//g, "") + "]");
					log(name, "toolbar layers: image=[" + (cs ? cs.backgroundImage : "?").replace(/file:\/\/\/[^)]*\//g, "") + "] position=[" + (cs ? cs.backgroundPosition : "?") +
						"] repeat=[" + (cs ? cs.backgroundRepeat : "?") + "] size=[" + (cs ? cs.webkitBackgroundSize || cs.backgroundSize : "?") + "] color=[" + (cs ? cs.backgroundColor : "?") + "]");
					log(name, "toolbar style: background=[" + (tb ? tb.style.background : "?").slice(0, 160) + "] color=[" + (tb ? tb.style.color : "?") + "]" +
						" glyphs: " + (function() { var n = 0, d = 0, all = tb ? tb.querySelectorAll(".enyo-button-icon") : [], j;
							for (j = 0; j < all.length; j++) { if (/menu-icon-/.test(all[j].style.backgroundImage)) { n++; if (/\/dark\//.test(all[j].style.backgroundImage)) { d++; } } }
							return d + " of " + n + " ink"; })());
				} else if (st === "G") {              // the reload/stop glyphs in the address fields: the theme's rule, and as drawn
					var gsh = document.getElementById("jihad-theme-colors"), gtx = gsh ? gsh.textContent : "";
					var gcount = function(sel) {
						var all = document.querySelectorAll(sel), ink = 0, k;
						for (k = 0; k < all.length; k++) { if (/\/dark\/menu-icon-/.test(window.getComputedStyle(all[k], null).backgroundImage)) { ink++; } }
						return ink + " of " + all.length + " ink";
					};
					log(name, "glyphs: browser view " + (app.$.browser ? "exists" : "not created") +
						"; theme sheet's ink rule " + (gtx.indexOf("dark/menu-icon-refresh.png") >= 0 ? "present" : "absent") +
						"; drawn: refresh " + gcount(".addressbar .refresh-button") + ", stop " + gcount(".addressbar .stop-button"));
					wait = 1000;
				} else if (st === "L") {              // the padlock, read now — no click, no wait
					var lk0 = app.$.browser.$.actionbar.$.search.$.address.$.secureLock;
					log(name, "padlock showing=" + Boolean(lk0 && lk0.showing) + " address=[" + String(app.$.browser.$.actionbar.url).slice(0, 60) + "]");
					wait = 1000;
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
					if (pop && pop.items && isNaN(want)) {   // "s?" lists the rows instead of picking one
						var rowsText = [];
						for (var q = 0; q < pop.items.length; q++) { var it = pop.items[q]; rowsText.push(q + ":" + (it && (it.caption || it.label || it.value || (it.content)) || String(it))); }
						log(name, "rows: " + rowsText.join(" | "));
						pop.close();
						pop = null;
					}
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
