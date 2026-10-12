// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// DEVICE TEST HOOK for the Mochi shell — NOT part of the app and never packaged.
//
// A card's own chrome (its panels, sheets and dialogs) lives in LunaSysMgr; nothing on the
// daemon's inject channel reaches it, and a finger is not always at hand. This file is pushed
// next to the shell's sources FOR A TEST RUN ONLY (build/webos-oe/card-selftest.sh adds it to
// the device copy of source/package.js and removes it again). It runs ONE named scenario,
// chosen by the launch parameter `jihadTest`, drives the shell's own handlers — the functions
// a tap would call — and writes what it sees to the device log as "[JIHAD-TEST] <name>: ...".
// What it proves: the card code does what it says once its handler runs. What it does not
// prove: that a finger reaches that handler — those steps stay on the human checklist.
(function() {
	function log(name, text) { enyo.log("[JIHAD-TEST] " + name + ": " + text); }
	function later(ms, fn) { window.setTimeout(fn, ms); }
	function j(o) { try { return enyo.json.stringify(o); } catch (e) { return String(o); } }
	var HOST = "http://192.168.1.245:8090/";

	var scenarios = {
		// The settings page (about:preferences), which since 2026-10-05 carries what the card's
		// own panel used to: the search engine, Flash, the clear actions. Opened from the menu's
		// own handler, then driven with REAL CLICKS through the adapter — clickAt is what a tap
		// on the page becomes.
		//   settings                 open it and report the card's cache of its settings
		//   settings:x,y;x,y;...     ...then click each point (view pixels), 3 s apart. A step
		//                            written sN picks entry N of the drop-down list the previous
		//                            click opened; wN waits N seconds; b adds a test bookmark; ? reports.
		// Reports the cache, the toast and the sizes of the bookmark and history lists at the end.
		settings: function(app, name) {
			// When the engine's answer to a hold reaches this card, and in what state: the one
			// place a "the menu did not open" can be told apart (late, not asked, or not on screen).
			if (!app.__holdProbe) {
				app.__holdProbe = app.pageHold;
				app.pageHold = function(t) {
					log(name, "hold answer arrives: asked " + (this._pageHold ? (Date.now() - this._pageHold) + " ms ago" : "never") +
						", card activated=" + (window.PalmSystem ? window.PalmSystem.isActivated : "n/a") + ", answer=" + t);
					return app.__holdProbe.apply(this, arguments);
				};
			}
			var spec = String(name).indexOf(":") > 0 ? String(name).slice(String(name).indexOf(":") + 1) : "";
			var steps = spec ? spec.split(";") : [];
			function count(kind, cb) { enyo.jihad.dbFind(kind, null, "_rev", true, function(r) { cb((r || []).length); }); }
			function state(tag, then) {
				count(enyo.jihad.kinds.bookmarks, function(nb) {
					count(enyo.jihad.kinds.history, function(nh) {
						var t = document.getElementById("jihad-toast");
						log(name, tag + ": url=" + String(app.url).split("#")[0] + " engine=" + enyo.jihadChrome.loadSearchEngine() +
							" searchUrl=" + enyo.jihad.searchUrlFor("a b") + " home=" + enyo.jihadChrome.loadHome() +
							" bookmarks=" + nb + " history=" + nh + " toast=[" + (t ? t.textContent : "") + "]");
						then();
					});
				});
			}
			function step(i) {
				if (i >= steps.length) { later(3000, function() { state("end", function() { log(name, "DONE"); }); }); return; }
				var st = steps[i], wait = 3000;
				if (st.charAt(0) === "w") {
					wait = (parseInt(st.slice(1), 10) || 1) * 1000;
				} else if (st === "T") {              // the toolbar's theme as painted (settings scenario)
					var tbs = app.$.actionBar.hasNode();
					log(name, "toolbar style: background=[" + (tbs ? tbs.style.background : "?").slice(0, 80) + "] color=[" + (tbs ? tbs.style.color : "?") + "]");
				} else if (st === "b") {              // a bookmark of the test's own, to have one to clear
					enyo.jihad.dbPut([{_kind: enyo.jihad.kinds.bookmarks, title: "jihad test bookmark", url: "http://example.invalid/jihad-test", date: (new Date()).getTime()}]);
					log(name, "test bookmark added");
				} else if (st.charAt(0) === "f") {    // fVX,VY: a flick, as the adapter's scroller gets it when a
					// finger lets go — the real kinetic scroll, at the adapter's own frame rate. (The
					// adapter's pageScaleAndScroll, which an earlier "z" step called, is an empty stub:
					// that step never moved anything, and what it was thought to prove it did not.)
					var fv = st.slice(1).split(",");
					app.$.view.callBrowserAdapter("handleFlick", [parseInt(fv[0], 10), parseInt(fv[1], 10)]);
					log(name, "flick " + st.slice(1));
					wait = 2500;
				} else if (st.charAt(0) === "d") {    // dX,Y,DX,DY[,VX,VY]: a drag from view X,Y by DX,DY as DOM
					// mouse events on the plugin node (what one finger becomes), in eight moves; with
					// VX,VY the release is a flick of that velocity. Whether the platform passes
					// script-made mouse events on to a plugin is itself something this step finds out.
					var dp = st.slice(1).split(","), dnode = app.$.view.hasNode(), drect = dnode.getBoundingClientRect();
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
						if (dp.length >= 6) { app.$.view.callBrowserAdapter("handleFlick", [parseInt(dp[4], 10), parseInt(dp[5], 10)]); }
						dfire("mouseup", dx0 + ddx, dy0 + ddy);
					})();
					log(name, "drag " + st.slice(1));
					wait = 2500;
				} else if (st.charAt(0) === "g") {    // gSCALE,X,Y: a pinch to SCALE about view X,Y, in eight
					// steps through the adapter's own gesture calls. The scale and rotation must be
					// non-integers: the adapter refuses an integer where it expects a double.
					var gp = st.slice(1).split(","), gTarget = parseFloat(gp[0]), gx = parseInt(gp[1], 10), gy = parseInt(gp[2], 10), gn = 0;
					app.$.view.callBrowserAdapter("gestureStart", [gx, gy, 1.0001, 0.0001, gx, gy]);
					(function gtick() {
						gn++;
						var sc = 1 + (gTarget - 1) * gn / 8 + 0.00001;
						if (gn < 8) { app.$.view.callBrowserAdapter("gestureChange", [gx, gy, sc, 0.0001, gx, gy]); setTimeout(gtick, 60); }
						else { app.$.view.callBrowserAdapter("gestureEnd", [gx, gy, sc, 0.0001, gx, gy]); }
					})();
					log(name, "pinch " + st.slice(1));
					wait = 3500;
				} else if (st === "a") {              // answer OK to the engine alert or confirm that is open (its own handler)
					var alertUp = !!(app.$.dialogs.$.alert && app.$.dialogs.$.alert.getShowing());
					var confirmUp = !!(app.$.dialogs.$.confirm && app.$.dialogs.$.confirm.getShowing());
					log(name, "engine alert showing=" + alertUp + " confirm showing=" + confirmUp + (confirmUp ? " [" + String(app.$.dialogs.$.confirmMsg ? app.$.dialogs.$.confirmMsg.getContent() : "").slice(0, 120) + "]" : "") + ((alertUp || confirmUp) ? "; answering OK" : "; nothing to answer"));
					if (alertUp) { app.$.dialogs.alertOk(); } else if (confirmUp) { app.$.dialogs.confirmOk(); }
				} else if (st === "x") {              // close this card (a second card that only needed to connect)
					log(name, "closing this card");
					window.close();
				} else if (st === "?") {              // report the state now
					state("now", function() {});
				} else if (st.charAt(0) === "h") {    // hX,Y: a long-press at view X,Y, from the point the
					// card has passed it on: the card is marked as waiting, the engine is sent the hold.
					// NOT a finger: it skips the adapter's own gesture timing and hit test.
					var hs = st.slice(1).split(",");
					app.pageHoldPending();
					app.$.view.callBrowserAdapter("mouseHoldAt", [parseInt(hs[0], 10), parseInt(hs[1], 10)]);
					log(name, "hold " + st.slice(1));
				} else if (st.charAt(0) === "m") {    // m: list the open long-press sheet; mN: choose entry N
					var its = app.$.sheetItems.getClientControls(), caps = [];
					for (var q = 0; q < its.length; q++) { caps.push(its[q].getContent()); }
					log(name, "sheet showing=" + app.$.sheet.getShowing() + " title=[" + app.$.sheetTitle.getContent() + "] items=[" + caps.join(" | ") + "]");
					if (st.length > 1 && its[parseInt(st.slice(1), 10)]) {
						log(name, "sheet pick [" + its[parseInt(st.slice(1), 10)].getContent() + "]");
						app.sheetPick(its[parseInt(st.slice(1), 10)]);                 // what a tap on the row does
					}
				} else if (st === "K") {              // focus the address field as a tap does (the keyboard comes up)
					if (app.$.address.hasNode()) { app.$.address.node.focus(); }
					log(name, "address field focus asked; window " + window.innerWidth + "x" + window.innerHeight);
				} else if (st === "E") {              // the keyboard goes down with a page field still focused, as its hide key leaves it
					if (window.PalmSystem) { window.PalmSystem.editorFocused(false, 0, 0); }
					log(name, "keyboard lowered; window " + window.innerWidth + "x" + window.innerHeight);
				} else if (st === "N") {              // what has the focus now, and the window's size
					var ae = document.activeElement;
					log(name, "focus: " + (ae ? ae.tagName + (ae.className ? "." + String(ae.className).split(" ")[0] : "") : "none") + " window " + window.innerWidth + "x" + window.innerHeight);
				} else if (st.charAt(0) === "U") {    // navigate as a typed address does (openUrl); "L" right after reads the padlock at once
					app.openUrl(st.slice(1));
					log(name, "navigate (typed) to " + st.slice(1));
					wait = 50;
				} else if (st === "L") {              // the padlock, read now — no tap, no wait
					log(name, "padlock showing=" + app.$.lock.getShowing() + " address=[" + String(app.url).slice(0, 60) + "]");
					wait = 1000;
				} else if (st === "Z") {              // take the painted theme off the toolbar (card side only), so the next notification's painting shows
					app.applyTheme(null);
					log(name, "toolbar cleared (card side)");
				} else if (st === "Y") {              // probe: what getTheme answers, and what a direct applyTheme does to the node
					enyo.jihad.own("getTheme", {}, function(r) { log(name, "getTheme answered: " + JSON.stringify(r).slice(0, 300)); });
					try {
						app.applyTheme({accentcolor: "#1e2f5c", headerURL: "http://192.168.1.245:8091/persona-header.png", textcolor: "#ffffff", name: "probe"});
						var tbp = app.$.actionBar.hasNode();
						log(name, "after direct applyTheme: background=[" + (tbp ? tbp.style.background : "?").slice(0, 160) + "] color=[" + (tbp ? tbp.style.color : "?") + "] cssText=[" + (tbp ? tbp.style.cssText : "?").slice(0, 200) + "]");
					} catch (e) { log(name, "direct applyTheme THREW " + e); }
				} else if (st.charAt(0) === "J") {   // J or JMS: hold MS (default 1500) before the lift              // a FINGER-LIKE long-press with the lift (see the Enyo hook's J)
					var jv = app.$.view, jnode = jv && jv.hasNode(), jn = jnode && (jnode.tagName === "OBJECT" ? jnode : jnode.querySelector("object"));
					var jx = Math.round(window.innerWidth / 2), jy = Math.round(window.innerHeight / 2);
					var jm = function (type) { var me = document.createEvent("MouseEvents"); me.initMouseEvent(type, true, true, window, 1, jx, jy, jx, jy, false, false, false, false, 0, null); if (jn) { jn.dispatchEvent(me); } };
					log(name, "finger hold at " + jx + "," + jy + " (plugin " + (jn ? "present" : "MISSING") + ")");
					jm("mousedown");
					setTimeout(function () {
						var openBefore = app.$.sheet.getShowing();
						jm("mouseup"); jm("click");
						setTimeout(function () { log(name, "finger hold: sheet open before lift=" + openBefore + " after lift=" + app.$.sheet.getShowing() + (app.$.sheet.getShowing() ? " [" + app.$.sheetTitle.getContent() + "]" : "")); }, 600);
					}, parseInt(st.slice(1), 10) || 1500);
					wait = (parseInt(st.slice(1), 10) || 1500) + 2000;
				} else if (st === "P") {              // show the start page (an overlay a loaded page hides) and read its colours
					app.$.startPage.show();
					var spn = app.$.startPage.hasNode(), spc = spn ? window.getComputedStyle(spn, null) : null;
					log(name, "start page shown: background=[" + (spc ? spc.backgroundColor : "?") + "]");
				} else if (st.charAt(0) === "M") {    // open a toolbar menu as its button does: Mshare, Mlibrary, Mapp
					var which = st.slice(1);
					if (which === "share") { app.doShare(); } else if (which === "library") { app.doBookmarks(); } else { app.toggleAppMenu(); }
					var rows = [], mc = app.$.menuItems.getControls(), mi;
					for (mi = 0; mi < mc.length; mi++) { rows.push(mc[mi].getContent()); }
					log(name, "menu " + which + " open=" + app.$.menuPopup.getShowing() + " left=" + app.$.menuBox.hasClass("jihad-menu-left") + " rows=[" + rows.join(" | ") + "]");
				} else if (st.charAt(0) === "K") {    // tap row N of the open menu (its own handler)
					var kc = app.$.menuItems.getControls(), kn = parseInt(st.slice(1), 10);
					if (kc[kn]) { log(name, "menu tap [" + kc[kn].getContent() + "]"); app.menuPick(kc[kn]); } else { log(name, "menu: no row " + kn); }
				} else if (st === "R") {              // what is showing: the menu, the sheet, the panels
					var sh = app.$.sheet.getShowing(), srows = [], sc2 = app.$.sheetItems.getControls(), si;
					for (si = 0; si < sc2.length; si++) { srows.push(sc2[si].getContent()); }
					log(name, "menu open=" + app.$.menuPopup.getShowing() + " sheet=" + sh + (sh ? " [" + app.$.sheetTitle.getContent() + ": " + srows.join(" | ") + "]" : "") +
						" bookmarks=" + app.$.bookmarkList.getShowing() + " history=" + app.$.historyList.getShowing() + " downloads=" + app.$.downloadList.getShowing() +
						" find=" + app.$.findBar.getShowing() + " print=" + (app.$.printDialog.isOpen ? app.$.printDialog.isOpen() : "?") + " url=" + String(app.url).slice(0, 50));
				} else if (st.charAt(0) === "E") {    // TEST ONLY: evaluate an expression in the card and log the result (no ';' — steps split on it)
					var ev; try { ev = String(eval(st.slice(1))); } catch (e) { ev = "THREW " + e; }
					log(name, "eval: " + ev.slice(0, 400));
				} else if (st === "T") {              // the toolbar's theme as painted (inline style set by applyTheme)
					var tb = app.$.actionBar.hasNode();
					var ic0 = tb ? tb.querySelector("img.jihad-nav-img") : null;
					log(name, "toolbar style: background=[" + (tb ? tb.style.background : "?").slice(0, 160) + "] color=[" + (tb ? tb.style.color : "?") + "]" +
						" glyphs: " + (function() { var n = 0, d = 0, all = tb ? tb.querySelectorAll("img.jihad-nav-img") : [], j, k;
							for (j = 0; j < all.length; j++) { n++; for (k in enyo.JihadIconsDark) { if (all[j].getAttribute("src") === enyo.JihadIconsDark[k]) { d++; break; } } }
							return d + " of " + n + " ink"; })());
				} else if (st === "l") {              // the padlock: is it showing, and what does its tap bring up?
					log(name, "padlock showing=" + app.$.lock.getShowing() + " field has room for it=" + app.$.addressBox.hasClass("jihad-secure"));
					app.showPageCertificate();                   // the padlock's own ontap handler
					later(2500, function() {
						log(name, "dialog showing=" + app.$.dialog.getShowing() + " title=[" + app.$.dialogTitle.getContent() + "] text=[" + String(app.$.dialogMessage.getContent()).replace(/\n/g, " | ").slice(0, 420) + "]");
					});
				} else if (st === "p") {              // the menu's Paste, without the menu
					log(name, "paste (direct)");
					app.pagePaste();
				} else if (st.charAt(0) === "c") {    // c=TEXT: is TEXT what is on the clipboard now? Only the
					// answer and the length are logged — the clipboard is the user's, the log is kept.
					(function(want) {
						enyo.jihad.getClipboard(function(t) { log(name, "clipboard is the expected text: " + (String(t) === want) + " (length " + String(t).length + ", expected " + want.length + ")"); });
					})(st.slice(2));
				} else if (st.charAt(0) === "k") {    // kTEXT: put TEXT on the clipboard (to have something to paste)
					log(name, "clipboard set=" + enyo.jihad.setClipboard(st.slice(1)));
				} else if (st.charAt(0) === "s") {
					var want = parseInt(st.slice(1), 10), rows = app.$.selectBox.getClientControls(), row = null;
					for (var k = 0; k < rows.length; k++) { if (rows[k]._optIndex === want) { row = rows[k]; } }
					log(name, "list showing=" + app.$.selectPopup.getShowing() + " rows=" + rows.length + " pick " + want + (row ? " [" + row.getContent() + "]" : " (no such row)"));
					if (row) { app.selectPopupPick(row); }
				} else {
					var xy = st.split(",");
					app.$.view.callBrowserAdapter("clickAt", [parseInt(xy[0], 10), parseInt(xy[1], 10), 1]);
					log(name, "click " + st);
				}
				later(wait, function() { step(i + 1); });
			}
			// `page:<steps>` runs the same steps on the page the card was launched on, without
			// opening the settings page first.
			if (String(name).indexOf("page") !== 0) { app.menuSettings(); }
			later(8000, function() { state("opened", function() { step(0); }); });
		},

		bookmarks: function(app, name) {
			enyo.jihad.dbDelByQuery(enyo.jihad.kinds.bookmarks, null, function(delResp) {
				log(name, "del=" + j(delResp));
				var t = (new Date()).getTime();
				enyo.jihad.dbPut([
					{_kind: enyo.jihad.kinds.bookmarks, title: "First bookmark", url: "http://example.com/one", date: t, lastVisited: t, defaultEntry: false, visitCount: 0, idx: null},
					{_kind: enyo.jihad.kinds.bookmarks, title: "Second bookmark", url: "http://example.com/two", date: t - 1000, lastVisited: t, defaultEntry: false, visitCount: 0, idx: null}
				], function(putResp) {
					log(name, "put=" + j(putResp));
					app.menuBookmarks();
					later(2000, function() {
						var l = app.$.bookmarkList;
						log(name, "rows=" + l.items.length);
						l.editTapped(null, {index: 0});
						later(5000, function() {
							l.$.editTitle.setValue("Edited & saved <b>");
							l.$.editUrl.setValue("http://example.com/?a=1&b=2");
							l.editSave();
							later(1500, function() {
								enyo.jihad.dbFind(enyo.jihad.kinds.bookmarks, null, "_rev", true, function(res) {
									log(name, "stored=" + j((res || []).map(function(r) { return [r.title, r.url]; })) + " DONE");
								});
							});
						});
					});
				});
			});
		},
		history: function(app, name) {
			var now = (new Date()).getTime(), D = 86400000, recs = [], ages = [0, 1, 3, 9, 20, 60, 400];
			for (var i = 0; i < ages.length; i++) {
				recs.push({_kind: enyo.jihad.kinds.history, url: "http://example.com/h" + ages[i], title: "Visited " + ages[i] + " days ago", date: now - ages[i] * D - 3600000});
			}
			enyo.jihad.dbDelByQuery(enyo.jihad.kinds.history, null, function() {
				enyo.jihad.dbPut(recs, function() {
					app.menuHistory();
					later(2500, function() {
						var l = app.$.historyList, labels = [];
						for (var k = 0; k < l.items.length; k++) { labels.push(l.groupLabel(l.items[k].date)); }
						log(name, "rows=" + l.items.length + " groups=" + j(labels) + " DONE");
					});
				});
			});
		},
		// Clear History from the history view, through its confirmation, then what is left.
		historyclear: function(app, name) {
			app.menuHistory();
			later(2000, function() {
				var l = app.$.historyList;
				log(name, "before rows=" + l.items.length + " back.disabled=" + app.$.back.hasClass("disabled"));
				l.clearTapped();
				log(name, "confirm showing=" + l.$.ask.getShowing());
				later(5000, function() {
					l.askOk();
					later(1500, function() {
						enyo.jihad.dbFind(enyo.jihad.kinds.history, null, null, false, function(res) {
							log(name, "after rows=" + (res || []).length + " back.disabled=" + app.$.back.hasClass("disabled") +
								" forward.disabled=" + app.$.forward.hasClass("disabled") + " DONE");
						});
					});
				});
			});
		},
		share: function(app, name) {
			app.doShare();
			later(1000, function() {
				var rows = app.$.sheetItems.getClientControls(), labels = [];
				for (var i = 0; i < rows.length; i++) { labels.push(rows[i].getContent()); }
				log(name, "sheet showing=" + app.$.sheet.getShowing() + " title=[" + app.$.sheetTitle.getContent() + "] sub=[" +
					app.$.sheetSub.getContent() + "] rows=" + j(labels) + " DONE");
			});
		},
		sharecopy: function(app, name) {
			var ok = enyo.jihad.setClipboard(app.url);
			log(name, "setClipboard=" + ok + " url=" + app.url + " DONE");
		},
		// Email / Text Message from the share sheet: the launch request and its reply.
		sharemail: function(app, name, arg) {
			var page = {title: app._title || app.url, url: app.url};
			var id = (arg === "sms") ? "com.palm.app.messaging" : "com.palm.app.email";
			var params = (arg === "sms") ? {compose: {messageText: "Check out this web page: " + page.url}}
				: {summary: "Check out this web page...", text: "Here's a website I think you'll like: " + page.url};
			enyo.jihad.launch({id: id, params: params}, function(r) {
				log(name, "open " + id + " -> " + j(r) + " DONE");
			});
		},
		launcher: function(app, name) {
			var base = String(window.location.href || "").replace(/^file:\/\//, "").replace(/[^\/]*$/, "");
			enyo.jihad.addLaunchPoint({id: enyo.jihad.appId(), title: "Jihad selftest page", icon: base + "icon.png", params: {url: app.url}}, function(r) {
				log(name, "addLaunchPoint=" + j(r));
				if (r && r.launchPointId) {
					later(6000, function() {
						enyo.jihad.luna("palm://com.palm.applicationManager/removeLaunchPoint", {id: enyo.jihad.appId(), launchPointId: r.launchPointId}, function(r2) {
							log(name, "removeLaunchPoint=" + j(r2) + " DONE");
						});
					});
				} else { log(name, "DONE"); }
			});
		},
		hold: function(app, name, arg) {
			var e = {linkUrl: HOST + "index.html", linkText: "A link on the page", imageUrl: HOST + "jihad-test.png", altText: "A picture", pageX: 100, pageY: 100};
			if (arg === "link") { e.imageUrl = ""; }
			if (arg === "image") { e.linkUrl = ""; }
			// Through the same door the adapter uses: the view's eventFired with a hit-test result.
			var claimed = app.$.view.eventFired({type: "mousehold", pageX: 100, pageY: 100},
				{isNull: false, isLink: Boolean(e.linkUrl), linkUrl: e.linkUrl, linkText: e.linkText, isImage: Boolean(e.imageUrl), imageUrl: e.imageUrl, altText: e.altText});
			var tap = app.$.view.eventFired({type: "click", pageX: 100, pageY: 100}, {isNull: false, isLink: true, linkUrl: e.linkUrl});
			var plain = app.$.view.eventFired({type: "mousehold", pageX: 5, pageY: 5}, {isNull: false, isLink: false, isImage: false});
			later(800, function() {
				var rows = app.$.sheetItems.getClientControls(), labels = [];
				for (var i = 0; i < rows.length; i++) { labels.push(rows[i].getContent()); }
				log(name, "hold claimed=" + claimed + " tap claimed=" + tap + " plain hold claimed=" + plain +
					" sheet=" + app.$.sheet.getShowing() + " rows=" + j(labels) + " DONE");
			});
		},
		// A long press on an image, then the row of the sheet a finger would tap — both through
		// the doors a finger uses (the view's eventFired, the sheet's own row handler).
		//   holdpick:share       "Share Image": the picture is fetched and handed to Email
		//   holdpick:wallpaper   "Set as Wallpaper": fetched, imported, made the wallpaper
		// The wallpaper one CHANGES THE DEVICE'S WALLPAPER: save the current one first and put
		// it back afterwards (card-selftest.sh does not do that for you).
		holdpick: function(app, name, arg) {
			var want = (arg === "wallpaper") ? "holdSetWallpaper" : "holdShareImage";
			var claimed = app.$.view.eventFired({type: "mousehold", pageX: 100, pageY: 100},
				{isNull: false, isLink: false, linkUrl: "", isImage: true, imageUrl: HOST + "jihad-test.png", altText: "A picture"});
			later(900, function() {
				var rows = app.$.sheetItems.getClientControls(), row = null, labels = [];
				for (var i = 0; i < rows.length; i++) { labels.push(rows[i].getContent()); if (rows[i].sheetAction === want) { row = rows[i]; } }
				log(name, "hold claimed=" + claimed + " sheet=" + app.$.sheet.getShowing() + " rows=" + j(labels));
				if (!row) { log(name, "no such row DONE"); return; }
				app.sheetPick(row);                  // what a tap on that row does
				log(name, "picked [" + row.getContent() + "]; sheet=" + app.$.sheet.getShowing());
				later(14000, function() {
					var t = document.getElementById("jihad-toast");
					enyo.jihad.luna("palm://com.palm.systemservice/getPreferences", {keys: ["wallpaper"]}, function(r) {
						log(name, "toast=[" + (t ? t.textContent : "") + "] wallpaper now=" + j(r && r.wallpaper && r.wallpaper.wallpaperName) + " DONE");
					});
				});
			});
		},
		saveimage: function(app, name) {
			app.holdSaveImage({imageUrl: HOST + "jihad-test.png"});
			later(9000, function() {
				enyo.jihad.downloadHistory(enyo.jihad.appId(), function(r) {
					var items = (r && r.items) || [], last = items.length ? items[items.length - 1] : null;
					log(name, "manager history items=" + items.length + " toast=[" + (document.getElementById("jihad-toast") ? document.getElementById("jihad-toast").textContent : "?") + "] DONE");
				});
			});
		},
		// Clear History with real pages behind the card: Back is live, then it is not, and the
		// engine no longer has anywhere to go back to.
		navclear: function(app, name) {
			app.openUrl("file:///tmp/jihad-shell.html");
			later(5000, function() {
				log(name, "two pages loaded: url=" + app.url + " back.disabled=" + app.$.back.hasClass("disabled"));
				app.clearHistory();
				later(1000, function() {
					log(name, "cleared: back.disabled=" + app.$.back.hasClass("disabled") + " forward.disabled=" + app.$.forward.hasClass("disabled"));
					app.$.view.callBrowserAdapter("goBack");      // straight at the engine, past the disabled control
					later(3500, function() { log(name, "after an engine goBack: url=" + app.url + " DONE"); });
				});
			});
		},
		downloads: function(app, name) {
			later(5000, function() {
				app.menuDownloads();
				later(2000, function() {
					var l = app.$.downloadList, rows = [];
					for (var i = 0; i < l.downloads.length; i++) { rows.push([l.fileNameOf(l.downloads[i]), l.statusOf(l.downloads[i]), Boolean(l.downloads[i].engine)]); }
					log(name, "rows=" + j(rows) + " DONE");
				});
			});
		},
		// A failed engine download and its Retry: the failure is injected at the handler the
		// adapter calls (no server here fails on demand), the retry is the real thing.
		retry: function(app, name) {
			var url = HOST + "jihad-testfile.bin";
			app.engineDownloadStart(null, {url: url});
			app.engineDownloadError(null, {url: url, message: "connection lost"});
			app.menuDownloads();
			later(2000, function() {
				var l = app.$.downloadList, d = l.downloads[0];
				log(name, "row0=" + j([l.fileNameOf(d), l.statusOf(d), l.isFailed(d)]));
				app.retryDownload(null, {item: d});
				later(9000, function() {
					var rows = [];
					for (var i = 0; i < l.downloads.length; i++) { rows.push([l.fileNameOf(l.downloads[i]), l.statusOf(l.downloads[i])]); }
					log(name, "after retry rows=" + j(rows) + " DONE");
				});
			});
		},
		// Which local directories may this card load an image from? (LunaSysMgr decides; the
		// answer chooses where bookmark thumbnails can live.) The probe files are put there by
		// the test runner beforehand.
		imgprobe: function(app, name) {
			var paths = ["/var/palm/jihad/mochi/probe.png", "/tmp/probe.png", "/media/internal/downloads/jihad-test.png",
				"/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser-mochi/thumbnails/probe.png"];
			var out = [], left = paths.length;
			function one(p) {
				var im = new Image();
				im.onload = function() { out.push(p + "=OK " + im.width + "x" + im.height); if (--left === 0) { log(name, out.join(" ; ") + " DONE"); } };
				im.onerror = function() { out.push(p + "=REFUSED"); if (--left === 0) { log(name, out.join(" ; ") + " DONE"); } };
				im.src = "file://" + p;
			}
			for (var i = 0; i < paths.length; i++) { one(paths[i]); }
			// A refused local load may fire neither event: report what is known after a while.
			later(4000, function() { if (left > 0) { log(name, out.join(" ; ") + " ; no event for " + left + " path(s) DONE"); } });
		},
		// Add Bookmark on a real page: the record, its thumbnail file, the row showing it;
		// then delete the bookmark and see the file go.
		thumb: function(app, name) {
			enyo.jihad.dbDelByQuery(enyo.jihad.kinds.bookmarks, null, function() {
				app.addCurrentBookmark();
				later(4000, function() {
					enyo.jihad.dbFind(enyo.jihad.kinds.bookmarks, null, "_rev", true, function(res) {
						var r = (res || [])[0] || {};
						log(name, "bookmark=" + j([r.title, r.url, r.thumbnailFile]));
						var im = new Image();
						im.onload = function() { log(name, "thumbnail loads in the card: " + im.width + "x" + im.height); };
						im.onerror = function() { log(name, "thumbnail REFUSED or missing"); };
						if (r.thumbnailFile) { im.src = "file://" + r.thumbnailFile; }
						app.menuBookmarks();
						later(9000, function() {
							app.$.bookmarkList.deleteTapped(null, {index: 0});
							later(1500, function() { log(name, "deleted; rows=" + app.$.bookmarkList.items.length + " DONE"); });
						});
					});
				});
			});
		},
		// The long-press menu's "Open in New Card": a second card, opened on the link.
		newcard: function(app, name) {
			app.holdOpenInNewCard({linkUrl: "file:///tmp/jihad-shell.html"});
			log(name, "requested DONE");
		},
		// Print: open the dialog from the menu and report what the daemon finds. With an
		// argument (an address, or address:port) that address is typed into the dialog's own
		// field, searched for, and the page is printed on the printer found there. Point it at
		// render/goanna/test/ipp-sink.py, NOT at a real printer:  run print:192.168.1.245:8090
		print: function(app, name) {
			app.menuPrint();
			var d = app.$.printDialog, OPEN = d.getShowing();
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
		// The SSL warning must already be on screen (launch the card on a self-signed https
		// target): show the certificate, report it, then refuse the certificate.
		cert: function(app, name) {
			later(6000, function() {
				var d = app.$.dialogs;
				log(name, "ssl showing=" + d.$.ssl.getShowing() + " view offered=" + d.$.sslView.getShowing());
				d.sslViewTapped();
				log(name, "cert=[" + String(d.$.sslCert.getContent()).replace(/\n/g, " | ") + "]");
				later(7000, function() { d.sslDontTrust(); log(name, "answered DONE"); });
			});
		}
	};

	scenarios.page = scenarios.settings;
	function run() {
		var p = null;
		try { p = enyo.json.parse(window.PalmSystem.launchParams); } catch (e) { p = null; }
		var spec = p && p.jihadTest;
		if (!spec || !window.jihadApp) { return; }
		var name = String(spec).split(":")[0], arg = String(spec).split(":")[1];
		if (!scenarios[name]) { log(name, "no such scenario DONE"); return; }
		log(name, "start");
		try { scenarios[name](window.jihadApp, spec, arg); } catch (e) { log(spec, "THREW " + e + " DONE"); }
	}
	// After the shell has rendered and connected.
	window.setTimeout(run, 5000);
}());
