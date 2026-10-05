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
				} else if (st === "b") {              // a bookmark of the test's own, to have one to clear
					enyo.jihad.dbPut([{_kind: enyo.jihad.kinds.bookmarks, title: "jihad test bookmark", url: "http://example.invalid/jihad-test", date: (new Date()).getTime()}]);
					log(name, "test bookmark added");
				} else if (st === "?") {              // report the state now
					state("now", function() {});
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
			app.menuSettings();
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
