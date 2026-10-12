// Copyright 2026 NotAlexNoyle.
// Licensed under the Apache License, Version 2.0; see ../../../LICENSE.
//
// Jihad Browser (Mojo) — TEST HOOK, never packaged. Loaded only by build/webos-oe/card-selftest.sh
// (`mojo push` adds it to sources.json after the shell's own sources; `mojo restore` removes it).
// Launch the card with {"target": url, "jihadTest": "page:<steps>"} and the steps run in the card,
// 3 s apart unless a step says otherwise, each logged as "[JIHAD-TEST] <scenario>: …" through
// Mojo.Log.error (warn does not reach the device log in this framework build).
//
// What a scenario proves: the card's own handlers, driven from inside the card — not a finger.
// A hold goes to the adapter's mouseHoldAt (no gesture timing, no hit test); menus and
// dialogs are read through the scene controller's own popupSubmenu / showAlertDialog, which are
// wrapped here so their models can be read and their entries chosen as a tap would.
//
// Steps:
//   wN           wait N seconds
//   hX,Y         long-press at view X,Y (adapter mouseHoldAt; the engine answers with page-hold)
//   m / mN       list the open submenu's entries / tap entry N (a Mojo tap event on its node, so the
//                widget's own handler chooses and closes)
//   l            the padlock: is it drawn, and what does a tap on it (handleAddressTap) bring up
//   d / a        read the open alert dialog (title, message) and close it with its first choice,
//                through the dialog's own close(value) (engine dialogs come as showAlertDialog too)
//   fTEXT        Find on Page: put TEXT in the find bar and run the search as Next does
//   n            the page's selection as the engine's find left it (text, via the page commands)
//   kTEXT        put TEXT on the clipboard
//   c=TEXT       is TEXT what is on the clipboard? (only the answer and the lengths are logged)
//   p            Paste, as the menu entry does it (pasteIntoPage)
//   tX,Y         a tap at card X,Y (device y - 28), as one click pair on the plugin node
//   H            the open submenu's colours: a plain row's, and a "selected" row's (the class added and removed)
//   ?            report url, secure flag, open popups
(function () {
	function log(name, text) { Mojo.Log.error("[JIHAD-TEST] " + name + ": " + text); }

	var origActivate = MainAssistant.prototype.activate;
	MainAssistant.prototype.activate = function (result) {
		origActivate.apply(this, arguments);
		// The stage hands the scene only the url; the scenario is read from the launch
		// parameters directly (the system keeps them in PalmSystem.launchParams).
		var name = null;
		try { var lp = window.PalmSystem && PalmSystem.launchParams ? Mojo.parseJSON(PalmSystem.launchParams) : null; name = lp && lp.jihadTest; } catch (e) { name = null; }
		if (!name || this.__jihadTestStarted) { return; }
		this.__jihadTestStarted = true;
		log(name, "start");
		run(this, String(name));
	};

	function run(app, name) {
		var spec = name.indexOf(":") > 0 ? name.slice(name.indexOf(":") + 1) : "";
		var steps = spec ? spec.split(";") : [];
		var sc = app.controller, win = sc.window, doc = sc.document;
		var popups = [], alerts = [];
		// The scene's own popupSubmenu / showAlertDialog, wrapped: the model is what the
		// shell asked to show; the element is what the framework drew.
		var origPopup = sc.popupSubmenu, origAlert = sc.showAlertDialog;
		sc.popupSubmenu = function (model) {
			var el = origPopup.apply(sc, arguments);
			popups.push({model: model, el: el});
			return el;
		};
		sc.showAlertDialog = function (model) {
			var el = origAlert.apply(sc, arguments);
			alerts.push({model: model, el: el});
			return el;
		};
		// When the engine's answer to a hold reaches this card, and in what state — the one
		// place "the menu did not open" can be told apart (not delivered, card not active, threw).
		if (!app.__holdProbe) {
			app.__holdProbe = app.handlePageHold;
			app.handlePageHold = function (text) {
				var stc = sc.stageController, active;
				try { active = stc.isActiveAndHasScenes ? stc.isActiveAndHasScenes() : stc.active; } catch (e) { active = "threw " + e; }
				log(name, "hold answer arrives: active=" + active + " answer=" + text);
				try { return app.__holdProbe.apply(this, arguments); }
				catch (e) { log(name, "handlePageHold THREW " + e); }
			};
		}
		function plugin() {
			var view = sc.get(app.WEB_VIEW_ID);
			return view ? view.querySelector("object") : null;
		}
		// Every adapter call the shell makes, with what the widget or the plugin said back.
		if (!app.__callProbe) {
			app.__callProbe = app.callBrowserAdapter;
			app.callBrowserAdapter = function (method, args) {
				var r, view = sc.get(app.WEB_VIEW_ID), widget = view && view.mojo;
				var ob = plugin(), road = widget && typeof widget[method] === "function" ? "widget" : (ob && typeof ob[method] === "function" ? "plugin" : "nowhere");
				try { r = app.__callProbe.apply(this, arguments); }
				catch (e) { log(name, "adapter call " + method + " by " + road + " THREW " + e); throw e; }
				log(name, "adapter call " + method + " by " + road + " -> " + r);
				return r;
			};
		}
		// A popup is "open" while its element is in the document. Nothing here closes one by
		// hand: a submenu entry is chosen by a tap event on its node, an alert by the dialog's
		// own close(value) — so the widgets run their own handlers and clean up after
		// themselves, as a finger would have them do.
		function isOpen(el) { try { return !!el && !!el.parentNode && doc.body.contains(el); } catch (e) { return false; } }
		function lastOpenPopup() {
			for (var i = popups.length - 1; i >= 0; i--) { if (isOpen(popups[i].el)) { return popups[i]; } }
			return null;
		}
		function labels(items) {
			var out = [];
			for (var i = 0; i < (items || []).length; i++) { out.push(items[i].label); }
			return out.join(" | ");
		}
		function readClipboard(cb) {
			var ta = doc.createElement("textarea"), tries = 0;
			ta.style.cssText = "position:absolute;left:-2000px;top:0";
			doc.body.appendChild(ta);
			try { ta.select(); if (win.PalmSystem && win.PalmSystem.paste) { win.PalmSystem.paste(); } else { doc.execCommand("paste"); } } catch (e) {}
			(function look() {
				if (!ta.value && ++tries < 15) { win.setTimeout(look, 40); return; }
				var t = ta.value; try { ta.blur(); } catch (e) {}
				if (ta.parentNode) { ta.parentNode.removeChild(ta); }
				cb(t);
			})();
		}
		function state(tag) {
			var p = lastOpenPopup();
			log(name, tag + ": url=" + String(app.url).slice(0, 70) + " title=[" + String(app.title).slice(0, 40) + "] secure=" + Boolean(app.secure) +
				" popupOpen=" + Boolean(p) + " alertsShown=" + alerts.length);
		}
		function step(i) {
			if (i >= steps.length) { win.setTimeout(function () { state("end"); log(name, "DONE"); }, 3000); return; }
			var st = steps[i], wait = 3000, o, p, n;
			try {
				if (st.charAt(0) === "w") {
					wait = (parseInt(st.slice(1), 10) || 1) * 1000;
				} else if (/^\d+,\d+$/.test(st)) {   // tap at view X,Y through the adapter's clickAt (what a tap on the page becomes)
					o = plugin();
					var cxy = st.split(",");
					if (o && typeof o.clickAt === "function") { o.clickAt(parseInt(cxy[0], 10), parseInt(cxy[1], 10), 1); log(name, "click " + st); }
					else { log(name, "click: no plugin element with clickAt"); }
				} else if (st.charAt(0) === "h") {
					o = plugin();
					var hv = st.slice(1).split(",");
					if (o && typeof o.mouseHoldAt === "function") { o.mouseHoldAt(parseInt(hv[0], 10), parseInt(hv[1], 10)); log(name, "hold " + st.slice(1)); }
					else { log(name, "hold: no plugin element with mouseHoldAt"); }
				} else if (st.charAt(0) === "m") {
					p = lastOpenPopup();
					log(name, "submenu open=" + Boolean(p) + " items=[" + (p ? labels(p.model.items) : "") + "]");
					if (p && st.length > 1) {
						n = parseInt(st.slice(1), 10);
						if (p.model.items[n]) {
							var node = p.el.querySelector('[x-mojo-menu-cmd="' + p.model.items[n].command + '"]');
							log(name, "submenu tap [" + p.model.items[n].label + "] node=" + Boolean(node));
							if (node) { Mojo.Event.send(node, Mojo.Event.tap); }
						}
					}
				} else if (st === "l") {
					var field = sc.get(app.ADDRESS_ID), rect = field ? field.getBoundingClientRect() : null;
					var drawn = field ? field.hasClassName("jihad-secure") : false;
					log(name, "padlock drawn=" + drawn + " secure=" + Boolean(app.secure));
					app.handleAddressTap({down: {pageX: rect ? rect.left + 12 : 0}, stop: function () {}});
					wait = 6000;
				} else if (st === "d" || st === "a") {
					var al = alerts.length ? alerts[alerts.length - 1] : null;
					log(name, "alert: " + (al ? "open=" + isOpen(al.el) + " title=[" + al.model.title + "] message=[" + String(al.model.message).replace(/\n/g, " | ").slice(0, 400) + "]" : "none"));
					if (al && isOpen(al.el)) {
						var v = al.model.choices && al.model.choices[0] ? al.model.choices[0].value : undefined;
						log(name, "alert answered with [" + v + "] (" + (al.model.choices && al.model.choices[0] ? al.model.choices[0].label : "no choice") + ")");
						if (al.el.mojo && typeof al.el.mojo.close === "function") { al.el.mojo.close(v); }
						else { if (al.model.onChoose) { al.model.onChoose(v); } if (al.el.parentNode) { al.el.parentNode.removeChild(al.el); } }
					}
				} else if (st.charAt(0) === "t") {    // tX,Y: a tap at card X,Y (device y - 28): one click pair on the plugin node
					var tp = st.slice(1).split(","), tnode = plugin(), tx = parseInt(tp[0], 10), ty = parseInt(tp[1], 10);
					["mousedown", "mouseup", "click"].forEach(function (type) {
						var te = doc.createEvent("MouseEvents");
						te.initMouseEvent(type, true, true, win, 1, tx, ty, tx, ty, false, false, false, false, 0, null);
						if (tnode) { tnode.dispatchEvent(te); }
					});
					log(name, "tap " + st.slice(1) + (tnode ? "" : " (no plugin node)"));
				} else if (st.charAt(0) === "X") {    // XX,Y: a double tap on the plugin node at X,Y (two click pairs, then dblclick)
					var xp = st.slice(1).split(","), xnode = plugin(), xr = xnode ? xnode.getBoundingClientRect() : {left: 0, top: 0};
					var xx = xr.left + parseInt(xp[0], 10), xy = xr.top + parseInt(xp[1], 10);
					var xfire = function (type, det) {
						var e = doc.createEvent("MouseEvents");
						e.initMouseEvent(type, true, true, win, det, xx, xy, xx, xy, false, false, false, false, 0, null);
						if (xnode) { xnode.dispatchEvent(e); }
					};
					xfire("mousedown", 1); xfire("mouseup", 1); xfire("click", 1);
					win.setTimeout(function () { xfire("mousedown", 2); xfire("mouseup", 2); xfire("click", 2); xfire("dblclick", 2); }, 120);
					log(name, "double tap " + st.slice(1));
					wait = 3000;
				} else if (st.charAt(0) === "D") {    // DX,Y,DX,DY: a one-finger drag on the plugin node from X,Y by DX,DY (eight moves)
					var dp = st.slice(1).split(","), dnode = plugin(), drect = dnode ? dnode.getBoundingClientRect() : {left: 0, top: 0};
					var dx0 = drect.left + parseInt(dp[0], 10), dy0 = drect.top + parseInt(dp[1], 10), ddx = parseInt(dp[2], 10), ddy = parseInt(dp[3], 10), dn = 0;
					var dfire = function (type, x, y) {
						var e = doc.createEvent("MouseEvents");
						e.initMouseEvent(type, true, true, win, 1, x, y, x, y, false, false, false, false, 0, null);
						if (dnode) { dnode.dispatchEvent(e); }
					};
					dfire("mousedown", dx0, dy0);
					(function dtick() {
						dn++;
						dfire("mousemove", dx0 + ddx * dn / 8, dy0 + ddy * dn / 8);
						if (dn < 8) { win.setTimeout(dtick, 40); return; }
						dfire("mouseup", dx0 + ddx, dy0 + ddy);
					})();
					log(name, "drag " + st.slice(1) + " (plugin " + (dnode ? "present" : "MISSING") + ")");
					wait = 2500;
				} else if (st === "O") {              // the WebView widget's own zoom model: scale, page size, node size, scene scroll
					var wv = sc.get(app.WEB_VIEW_ID), wa = wv && wv._mojoController && wv._mojoController.assistant, ob2 = plugin();
					var sp = wa && wa.sceneScroller ? wa.sceneScroller.getScrollPosition() : null;
					log(name, "widget: " + (wa ? "scale=" + wa.currScale + " page=" + wa.currPageWidth + "x" + wa.currPageHeight + " fitWidth=" + wa.fitWidth +
						" inPinch=" + wa.inPinchZoom + " manip=" + wa.pageManipulated : "UNREACHABLE") +
						" node=" + (ob2 ? ob2.offsetWidth + "x" + ob2.offsetHeight + " style=" + ob2.style.width + "x" + ob2.style.height : "?") +
						" scroll=" + (sp ? sp.left + "," + sp.top : "?") + " win=" + win.innerWidth + "x" + win.innerHeight);
				} else if (st.charAt(0) === "Z") {    // ZSCALE: a two-finger pinch as the hardware delivers one — the first finger down on
					// the page (WebKit hands the plugin its pen-down, which arms the adapter's 350 ms hold
					// timer), then the gesture through the WebView widget's own document listeners, held for
					// 1.4 s, then the finger up. A hold answer during it is the bug this step exists for.
					// Z0 is the CONTROL: the same finger down, held still 1.2 s, no gesture — it must give a
					// hold, or this step cannot reach the adapter's pen-down and proves nothing.
					var zs = parseFloat(st.slice(1)), zob = plugin();
					var zx = Math.round(win.innerWidth / 2), zy = Math.round(win.innerHeight / 2);
					var zmouse = function (type) {
						var me = doc.createEvent("MouseEvents");
						me.initMouseEvent(type, true, true, win, 1, zx, zy, zx, zy, false, false, false, false, 0, null);
						if (zob) { zob.dispatchEvent(me); }
					};
					var zgest = function (type, scale) {
						var ge = doc.createEvent("Event");
						ge.initEvent(type, true, true);
						ge.scale = scale; ge.rotation = 0; ge.centerX = zx; ge.centerY = zy;
						doc.dispatchEvent(ge);
					};
					log(name, "pinch to " + zs + " at " + zx + "," + zy + " (plugin " + (zob ? "present" : "MISSING") + ")");
					zmouse("mousedown");
					if (!zs) {
						win.setTimeout(function () { zmouse("mouseup"); log(name, "held still: popupOpen=" + Boolean(lastOpenPopup())); }, 1200);
						wait = 3500;
					} else win.setTimeout(function () {
						zgest("gesturestart", 1);
						var zk = 0;
						(function more() {
							zk++;
							zgest("gesturechange", 1 + (zs - 1) * zk / 12);
							if (zk < 12) { win.setTimeout(more, 110); return; }
							zgest("gestureend", zs);
							zmouse("mouseup");
							log(name, "pinch done: popupOpen=" + Boolean(lastOpenPopup()));
						})();
					}, 120);
					wait = 3500;
				} else if (st === "J") {              // the address field's reload/stop button: where it is, what it shows, then tap it
					var rbt = sc.get("jihad-reload"), rr = rbt ? rbt.getBoundingClientRect() : null, ar = sc.get("jihad-address").getBoundingClientRect();
					log(name, "urlbar button " + (rbt ? "[" + rbt.className + "] at " + Math.round(rr.left) + "," + Math.round(rr.top) + " " + Math.round(rr.width) + "x" + Math.round(rr.height) +
						" field " + Math.round(ar.left) + "-" + Math.round(ar.right) + " loading=" + app.loading + " row=" + app.commandMenuModel.items.length + " items" : "MISSING"));
					if (rbt) { app.handleReloadTap(); }
				} else if (st.charAt(0) === "C") {    // tap a command-row circle as a finger does: Cshare, Clibrary, CnewCard (the item's own model)
					var want = app[st.slice(1) + "Item"], all = sc.document.getElementsByTagName("div"), cel = null, ci;
					for (ci = 0; ci < all.length && !cel; ci++) { if (all[ci]._mojoMenuItemModel === want) { cel = all[ci]; } }
					log(name, "circle tap " + st.slice(1) + " (button " + (cel ? "present" : "MISSING") + (want && want.disabled ? ", disabled" : "") + ")");
					if (cel) { Mojo.Event.send(cel, Mojo.Event.tap, {}); }
				} else if (st.charAt(0) === "A") {    // an app-menu / command-row command, as the menu sends it: Ajihad-print, Ajihad-find, ...
					log(name, "command " + st.slice(1));
					app.handleCommand({type: Mojo.Event.command, command: st.slice(1)});
				} else if (st === "V") {              // the scene on top and its text
					var as = sc.stageController.activeScene();
					log(name, "scene=" + (as ? as.sceneName : "?") + " text=[" + (as && as.sceneElement ? String(as.sceneElement.innerText).replace(/\s+/g, " ").slice(0, 300) : "") + "]");
				} else if (st === "Q") {              // back out of the scene on top (a back gesture)
					log(name, "pop scene");
					sc.stageController.popScene();
				} else if (st === "B") {              // take the focus off the address field (the keyboard covers the command row)
					try { sc.get(app.ADDRESS_ID).mojo.blur(); } catch (e) {}
					try { if (doc.activeElement && doc.activeElement.blur) { doc.activeElement.blur(); } } catch (e) {}
					log(name, "address field blurred");
				} else if (st === "T") {              // the toolbar's theme as painted (inline style set by applyTheme)
					var tb = sc.get("jihad-toolbar");
					var ic0 = doc.querySelector(".jihad-cmdmenu .palm-menu-button");
					log(name, "toolbar style: background=[" + (tb ? tb.style.background : "?").slice(0, 160) + "] color=[" + (tb ? tb.style.color : "?") + "]" +
						" glyphs: " + (function() { var n = 0, d = 0, all = doc.querySelectorAll(".jihad-cmdmenu .palm-menu-button, .jihad-cmdmenu .palm-menu-icon"), j;
							for (j = 0; j < all.length; j++) { if (all[j].getAttribute("data-jihad-sprite")) { n++; if (/\/dark\//.test(all[j].style.backgroundImage)) { d++; } } }
							return d + " of " + n + " ink"; })());
				} else if (st.charAt(0) === "f") {
					var fin = sc.get("jihad-find-input");
					app.showFind();
					if (fin && fin.mojo && fin.mojo.setValue) { fin.mojo.setValue(st.slice(1)); }
					log(name, "find [" + st.slice(1) + "] (as Next does it)");
					app.runFind();
				} else if (st === "n") {
					// The selection the engine's find left on the page, read as the page commands
					// read it: ask for a copy; the clip script carries the text.
					o = plugin();
					if (o) { o.copy(function (ok) { log(name, "find selection: engine answered " + ok); }); }
				} else if (st.charAt(0) === "k") {
					sc.stageController.setClipboard(st.slice(1), false);
					log(name, "clipboard set");
				} else if (st.charAt(0) === "c") {
					(function (want) {
						readClipboard(function (t) { log(name, "clipboard is the expected text: " + (String(t) === want) + " (length " + String(t).length + ", expected " + want.length + ")"); });
					})(st.slice(2));
				} else if (st === "p") {
					log(name, "paste (as the menu does it)");
					app.pasteIntoPage();
				} else if (st === "W") {              // which adapter methods the WebView widget exposes to the app
					var wv = sc.get(app.WEB_VIEW_ID), wm = wv && wv.mojo, keys = [], k, ob = plugin(), pk = [];
					for (k in wm) { if (typeof wm[k] === "function" && k.charAt(0) !== "_") { keys.push(k); } }
					for (k in ob) { if (typeof ob[k] === "function") { pk.push(k); } }
					log(name, "widget methods: " + keys.sort().join(" "));
					log(name, "plugin element methods (" + pk.length + "): " + pk.sort().join(" ").slice(0, 600));
				} else if (st === "P") {              // the plugin element, as the widget holds it and as the DOM finds it
					var wv2 = sc.get(app.WEB_VIEW_ID), wm2 = wv2 && wv2.mojo, ad = wm2 && wm2.adapter, ob2 = plugin(), ks = ["cut", "copy", "selectAll", "pageFocused", "mouseHoldAt", "insertStringAtCursor"], rep = [], k2;
					for (k2 = 0; k2 < ks.length; k2++) { rep.push(ks[k2] + ": widget.adapter=" + (ad ? typeof ad[ks[k2]] : "-") + " dom=" + (ob2 ? typeof ob2[ks[k2]] : "-")); }
					log(name, "same element=" + (ad === ob2) + " adapter.id=" + (ad && ad.id) + " type=" + (ad && ad.getAttribute && ad.getAttribute("type")) + " | " + rep.join("; "));
				} else if (st === "H") {              // the open submenu's colours: a plain row, and a row with the "selected" class a
					// touch gives it (added here and taken off again — the stylesheet's answer, not a finger)
					p = lastOpenPopup();
					var hrows = p ? p.el.querySelectorAll(".palm-row") : [];
					var hcol = function (r) {
						if (!r) { return "none"; }
						var rw = r.querySelector(".palm-row-wrapper"), lb = r.querySelector(".label");
						return "background " + win.getComputedStyle(r, null).backgroundColor + ", wrapper text " + (rw ? win.getComputedStyle(rw, null).color : "-") +
							", label text " + (lb ? win.getComputedStyle(lb, null).color : "-");
					};
					log(name, "submenu rows=" + hrows.length + "; plain row: " + hcol(hrows[1]));
					if (hrows[0]) {
						var hcls = hrows[0].className;
						hrows[0].className = hcls + " selected";
						log(name, "highlighted row: " + hcol(hrows[0]));
						hrows[0].className = hcls;
					}
					wait = 1000;
				} else if (st === "?") {
					state("now");
				} else {
					log(name, "unknown step " + st);
				}
			} catch (e) {
				log(name, "step " + st + " THREW " + e);
			}
			win.setTimeout(function () { step(i + 1); }, wait);
		}
		win.setTimeout(function () { state("opened"); step(0); }, 8000);
	}
})();
