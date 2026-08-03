/* Copyright 2026 NotAlexNoyle.
 * Licensed under the Apache License, Version 2.0; see ../../LICENSE.
 *
 * Route THIS app's WebView to the Jihad/Goanna engine:
 * swap the NPAPI plugin mime so Enyo loads OUR adapter (application/x-jihad-browser
 * -> BrowserAdapterJihad.so), which connects to our BrowserServer at
 * /tmp/yapserver.jihad-browser. Stock browser (application/x-palm-browser ->
 * /tmp/yapserver.browser) is untouched, so Jihad is a self-contained, redistributable
 * Goanna alternative to Atlas that coexists with the system browser.
 *
 * PER-VARIANT MIME (cavekit-device-build.md R7, ../../context/plans/plan-variant-identity.md):
 * application/x-jihad-browser belongs to the ENYO-1.0 variant alone. The Mochi and Mojo
 * front-ends ship their own packages with their own MIME (…-mochi / …-mojo), their own
 * adapter shim, YAP name, socket and upstart job, and never load this one. The literal is
 * declared here and only here — the three variants are independent packages and none
 * imports a shared constant from another.
 *
 * Typing is handled entirely in the daemon: the webOS VKB delivers each committed
 * character to the focused plugin as an npPalmKeyDownEvent, and BrowserPageGoanna::keyDown
 * inserts it into the focused editable. We do NOT bridge keys at the document level —
 * the VKB does not raise document key events for our swapped-type plugin, and forwarding
 * them here would double-insert with the daemon path and could leak keystrokes to the log. */
(function () {
	function log(m) { if (window.enyo && enyo.log) { enyo.log("[Jihad] " + m); } }

	// Execution-trace stamps. log() silently no-ops until enyo is loaded, so stamps are BUFFERED
	// and flushed from a point already proven to reach palm-log. Without this, "stamp missing"
	// conflates "code not reached" with "logger not ready yet" — the exact ambiguity that has
	// cost several round trips on this file already.
	window.__jihadStamps = window.__jihadStamps || [];
	function stamp(s) { try { window.__jihadStamps.push(s); } catch (e) {} }
	// Silent unless JIHAD_PROBE is on: the stamps are cheap to RECORD (an array push) and are
	// what proved the WebAppMgr source cache (context/impl/impl-stale-app-js.md), so the
	// mechanism stays; only the two log lines per launch are gated, so production logs stay clean.
	function flushStamps(tag) { if (!JIHAD_PROBE) { return; }
		try { log("stamps " + tag + " " + window.__jihadStamps.join(",")); } catch (e) {} }

	// ───────────────────────────────────────────────────────────────────────────────────
	// TEMPORARY DIAGNOSTIC — delete this constant and the JihadProbe block at the bottom
	// once the card→adapter thread is closed (2026-08-01 triage).
	//
	// It exists because "the <object> is in the DOM but no plugin was instantiated" has at
	// least four causes that are INDISTINGUISHABLE from outside the card: the type attribute
	// never reached the live node; the MIME is not in WebKit's plugin database; the database
	// has it but content cannot see it; or the element got no renderer/instance. On-device
	// evidence has already ruled out the adapter binary (the shim is mapped in WebAppMgr, and
	// its NPP_New — which now logs to syslog unconditionally — is never entered), so the
	// remaining question is entirely on this side of the boundary.
	// Everything it prints is prefixed [JihadProbe] and goes to console.log → palm-log.
	// ───────────────────────────────────────────────────────────────────────────────────
	// OFF. The card->adapter thread it existed for is CLOSED (context/impl/
	// impl-card-adapter-resolved.md): the MIME resolves, the prototype patch reaches the kind
	// actually instantiated, and the plugin instantiates as soon as the pane is visible and
	// non-zero-sized. Leaving it on is actively harmful, not merely noisy: probeControls()
	// creates extra 64x64 <object>s that each open their OWN connection to the daemon and
	// then get removed, which floods daemon.log with 64x64 geometry and leaves
	// "Broken pipe" / NS_BINDING_ABORTED behind that masks the real client's lines.
	var JIHAD_PROBE = false;
	stamp("A" + JIHAD_PROBE);
	// MUST go through enyo.log, not console.log. Measured on-device 2026-08-01: with the probe
	// installed and running (the [Jihad] patch line from log() below appears in palm-log), NOT ONE
	// console.log line reached palm-log — LunaSysMgrJS surfaces enyo.log but evidently not bare
	// console.log on this build. A probe whose output never arrives is indistinguishable from a
	// probe that never ran, which cost one full rebuild+reinstall round trip to notice.
	// Route through the SAME log() that demonstrably reaches palm-log, and chunk.
	// Measured on-device 2026-08-01, after three rebuild+reinstall round trips: log() emits
	// "[Jihad] WebView engine -> …" every launch, while plog() on the very next statement emitted
	// NOTHING — via console.log first, then via its own enyo.log call. Same file, same closure,
	// adjacent lines, verified byte-identical on device (md5) at version 1.0.1. The two differ
	// only in message SHAPE: the working line is short, the probe's first line is a long
	// concatenation. So this delegates to the known-good emitter and splits at 120 chars, and
	// emits a short fixed canary FIRST so "never called" and "message dropped" stop being
	// indistinguishable — which is exactly the ambiguity that cost those three round trips.
	function plog(m) {
		try {
			var s = "" + m;
			while (s.length > 120) { log("P " + s.slice(0, 120)); s = s.slice(120); }
			log("P " + s);
		} catch (e) {
			try { log("P plog-EXCEPTION"); } catch (e2) {}
		}
	}

	function patch() {
		if (!(window.enyo && enyo.BasicWebView && enyo.BasicWebView.prototype)) { return false; }
		var proto = enyo.BasicWebView.prototype;
		if (!proto._jihadPatched) {
			// engine selection: load OUR adapter for this WebView. (Do NOT touch editorFocused
			// or the VKB mode — the base BasicWebView already raises the keyboard on focus.)
			var origCreate = proto.create;
			proto.create = function () {
				origCreate.apply(this, arguments);
				this.domAttributes.type = "application/x-jihad-browser";
				if (JIHAD_PROBE) { probeCreate(this); }
			};
			// <select> dropdown (Atlas model): the framework handles the WHOLE card side.
			// BasicWebView.showPopupMenu -> doOpenSelect, and the enyo.WebView WRAPPER
			// consumes that event itself (showSelect -> createSelectPopup -> PopupList,
			// reply via selectPopupMenuItem) — it is never re-published to the app, so
			// neither this file nor Browser.js patches or handles anything here. The one
			// obligation is the daemon's: emit the isis JSON shape
			// (items[].text/isEnabled/isSeparator/isLabel + selectedIdx).
			proto._jihadPatched = true;
			log("WebView engine -> application/x-jihad-browser");
			stamp("E" + JIHAD_PROBE);
			flushStamps("at-patch");
			// Canary deliberately shaped EXACTLY like the line above (plain words, no ":" or "="):
			// on device that line logs and this one did not, from adjacent statements in a file whose
			// md5 matches local, so the remaining suspects are message CONTENT or a throw. The catch
			// distinguishes them — an exception here would abort the rest of patch() silently while
			// leaving the MIME swap installed, which is precisely what we observe.
			if (JIHAD_PROBE) {
				stamp("F");
				try { log("probe canary alive"); } catch (e1) {}
				try { probeInstall(proto); } catch (e2) { try { log("probe threw"); } catch (e3) {} }
			}
		}
		// Anchor the framework's <select> PopupList under the tapped box. Stock
		// enyo.WebView positions from this._selectRect, which is only ever set from
		// click info BasicWebView cannot publish (it declares no onClick event), so
		// every popup opened dead-centre. The daemon owns the element and ships its
		// card-px rect as an ADDITIVE "rect" key in the popup JSON (the framework's
		// own parse ignores it); capture it in showSelect and place the popup in
		// openSelect — delegating both ways, never replacing framework behaviour.
		var wv = window.enyo && enyo.WebView && enyo.WebView.prototype;
		if (wv && !wv._jihadAnchorPatched) {
			wv._jihadAnchorPatched = true;
			var origShowSelect = wv.showSelect;
			wv.showSelect = function(inSender, inId, inJson) {
				this._jihadSelectRect = null;
				try {
					var m = enyo.json.parse(inJson);
					if (m && m.rect && typeof m.rect.bottom === "number") {
						this._jihadSelectRect = m.rect;
					}
				} catch (e) {}
				return origShowSelect.apply(this, arguments);
			};
			var origOpenSelect = wv.openSelect;
			wv.openSelect = function(inPopup) {
				var r = this._jihadSelectRect;
				if (!r) { return origOpenSelect.apply(this, arguments); }
				var c = inPopup.calcSize(), d = this.getOffset();
				// horizontally centred on the box, top edge at the box's bottom;
				// flip above the box when it would run off the bottom of the card.
				var left = Math.max(0, (r.left + r.right) / 2 - c.width / 2 + d.left);
				if (left + c.width > window.innerWidth) {
					left = Math.max(0, window.innerWidth - c.width);
				}
				var top = r.bottom + d.top;
				if (top + c.height > window.innerHeight) {
					top = Math.max(0, r.top + d.top - c.height);
				}
				inPopup.openAt({left: left, top: top});
			};
		}
		return true;
	}

	// ---- probe: patch-time facts ------------------------------------------------------
	// Answers "is the kind we patched the kind that actually gets instantiated?". The app
	// declares {kind:"WebView"} (Browser.js), and enyo.WebView's chrome holds a DIRECT
	// constructor reference (`kind: enyo.BasicWebView`) captured when the framework was
	// parsed. Prototype patching survives that, but only if it is the same constructor —
	// if a second copy of the kind exists, we would be patching a prototype nobody uses.
	function probeInstall(proto) {
		try {
			var wv = window.enyo && enyo.WebView && enyo.WebView.prototype;
			var chromeKind = (wv && wv.chrome && wv.chrome[0]) ? wv.chrome[0].kind : null;
			plog("install: BasicWebView.prototype patched; enyo.WebView present=" + (!!wv) +
				"; WebView.chrome[0].kind===enyo.BasicWebView -> " +
				(chromeKind === enyo.BasicWebView) +
				"; chromeKind typeof=" + (typeof chromeKind));
			plog("install: proto.nodeTag=" + proto.nodeTag +
				"; proto.domAttributes.type(before instance)=" + proto.domAttributes.type);
		} catch (e) { plog("install: EXCEPTION " + e); }
	}

	// ---- probe: did OUR create() run, and was the node already generated? --------------
	// `generated`/hasNode() being true here would mean Enyo had already serialized
	// domAttributes into HTML and our assignment came too late to reach the live element.
	// (Enyo 1 reads domAttributes inside generateHtml(), which runs at render time and sets
	// `generated` — so this is the direct test of the "set after render" hypothesis.)
	function probeCreate(c) {
		try {
			plog("create: id=" + c.id + " kindName=" + c.kindName +
				" generated=" + c.generated + " hasNode=" + (!!c.hasNode()) +
				" domAttributes.type=" + c.domAttributes.type +
				" x-palm-cache-plugin=" + c.domAttributes["x-palm-cache-plugin"]);
			// Report the live node repeatedly: WebKit can defer plugin start until the page
			// is allowed to start media, so a null instance at `rendered` is not conclusive.
			var report = function (tag) { probeNode(c, tag); };
			var origRendered = c.rendered;
			c.rendered = function () {
				origRendered.apply(this, arguments);
				report("rendered");
				window.setTimeout(function () { report("+0.5s"); }, 500);
				window.setTimeout(function () { report("+3s"); }, 3000);
				window.setTimeout(function () { report("+8s"); probeRegistry("late"); }, 8000);
			};
		} catch (e) { plog("create: EXCEPTION " + e); }
	}

	// ---- probe: what the LIVE element actually is, and whether it has an NPObject -------
	function probeNode(c, tag) {
		try {
			var n = c.hasNode();
			if (!n) { plog(tag + ": NO NODE for id=" + c.id); return; }
			plog(tag + " node: id=" + c.id + " tag=" + n.tagName +
				" attr(type)=" + n.getAttribute("type") +
				" prop(type)=" + n.type +
				" cache-plugin=" + n.getAttribute("x-palm-cache-plugin"));
			// Layout size AND effective visibility. SubframeLoader::loadPlugin bails at
			// `if (!renderer || useFallback) return false;` — an element with no render object
			// (any ancestor display:none, which is exactly how Enyo's showingChanged hides a
			// control) never reaches createPlugin, and nothing later retries unless it is
			// re-rendered. Inline style is not enough to see that: walk the ancestors and
			// report the computed values.
			var r = n.getBoundingClientRect ? n.getBoundingClientRect() : null;
			var cs = window.getComputedStyle ? window.getComputedStyle(n, null) : null;
			plog(tag + " node: inDoc=" + (document.body.contains ? document.body.contains(n) : "n/a") +
				" offset=" + n.offsetWidth + "x" + n.offsetHeight +
				" client=" + n.clientWidth + "x" + n.clientHeight +
				" rect=" + (r ? (Math.round(r.width) + "x" + Math.round(r.height)) : "n/a") +
				" computed(display/visibility)=" +
				(cs ? (cs.display + "/" + cs.visibility) : "n/a"));
			probeAncestors(n, tag);
			// The single fact that decides everything: an instantiated BrowserAdapter exposes
			// its scriptable methods on the element.
			plog(tag + " npobject: openURL=" + (typeof n.openURL) +
				" connectBrowserServer=" + (typeof n.connectBrowserServer) +
				" setPageIdentifier=" + (typeof n.setPageIdentifier));
		} catch (e) { plog(tag + ": EXCEPTION " + e); }
	}

	// ---- probe: is any ancestor hiding the element (⇒ no render object ⇒ no plugin)? ----
	function probeAncestors(n, tag) {
		try {
			var hidden = [], depth = 0, p = n.parentNode;
			while (p && p.nodeType === 1 && depth < 40) {
				var s = window.getComputedStyle ? window.getComputedStyle(p, null) : null;
				if (s && (s.display === "none" || s.visibility === "hidden")) {
					hidden.push(p.tagName + "#" + (p.id || "-") + "." +
						((p.className || "").toString().split(" ")[0] || "-") +
						"[" + s.display + "/" + s.visibility + "]");
				}
				p = p.parentNode; depth++;
			}
			plog(tag + " ancestors: depth=" + depth + " hidden=" +
				(hidden.length ? hidden.join(" <- ") : "NONE"));
		} catch (e) { plog(tag + " ancestors: EXCEPTION " + e); }
	}

	// ---- probe: which bundle/app is actually running ------------------------------------
	function probeEnv() {
		try {
			var ps = window.PalmSystem;
			plog("env: href=" + document.location.href +
				" readyState=" + document.readyState);
			plog("env: PalmSystem=" + (!!ps) +
				" identifier=" + (ps && ps.identifier ? ps.identifier : "n/a") +
				" screen=" + screen.width + "x" + screen.height +
				" inner=" + window.innerWidth + "x" + window.innerHeight);
		} catch (e) { plog("env: EXCEPTION " + e); }
	}

	// ---- probe: WebKit's plugin database, as content can see it -------------------------
	// navigator.plugins.length is NOT merely informational here: WebCore Page::pluginData()
	// returns 0 unless SubframeLoader::allowPlugins() is true, and DOMPluginArray::length()
	// returns 0 for a null PluginData. allowPlugins() is the SAME gate
	// SubframeLoader::requestObject consults before it will instantiate anything. So:
	//   length === 0  ⇒ plugins are disabled for this page — no <object> of ANY type can ever
	//                   get an instance, and the MIME is irrelevant.
	//   length  >  0  ⇒ plugins are allowed; the failure is further down (MIME lookup,
	//                   renderer, or the canStartMedia deferral).
	// navigator.mimeTypes is built from the same PluginDatabase that
	// FrameLoaderClient::objectContentType consults, so a missing entry here means the
	// <object> could never have been routed to a Netscape plugin at all.
	function probeRegistry(tag) {
		try {
			var mt = navigator.mimeTypes, pl = navigator.plugins;
			plog(tag + " db: mimeTypes.length=" + (mt ? mt.length : "none") +
				" plugins.length=" + (pl ? pl.length : "none"));
			var names = ["application/x-jihad-browser", "application/x-palm-browser",
				"application/x-palm-mojo-browser"];
			for (var i = 0; i < names.length; i++) {
				var m = mt ? mt[names[i]] : null;
				var ep = m && m.enabledPlugin;
				plog(tag + " db: " + names[i] + " -> " + (m ? "PRESENT" : "ABSENT") +
					(ep ? (" plugin=" + ep.name + " file=" + ep.filename) : ""));
			}
			if (pl) {
				for (var j = 0; j < pl.length; j++) {
					var p = pl[j], types = [];
					for (var k = 0; k < p.length; k++) { types.push(p[k].type); }
					plog(tag + " db: plugin[" + j + "] " + p.name + " | " + p.filename +
						" | " + types.join(","));
				}
			}
		} catch (e) { plog(tag + " db: EXCEPTION " + e); }
	}

	// ---- probe: control elements, outside Enyo entirely ---------------------------------
	// Builds a bare <object> for our MIME and for the stock one, in the same document. This
	// separates "our MIME cannot instantiate" from "this app cannot instantiate any plugin"
	// and from "Enyo's element is built wrong" — the three are otherwise identical in the log.
	// Both are removed again; neither is ever told to connect to a BrowserServer.
	function probeControls() {
		try {
			var made = [];
			var mk = function (type) {
				var o = document.createElement("object");
				o.setAttribute("type", type);
				o.setAttribute("x-palm-cache-plugin", "false");
				o.style.cssText = "position:absolute;left:0;top:0;width:64px;height:64px;opacity:0";
				document.body.appendChild(o);
				made.push(o);
				return o;
			};
			var mine = mk("application/x-jihad-browser");
			var stock = mk("application/x-palm-browser");
			window.setTimeout(function () {
				try {
					plog("control jihad: attr=" + mine.getAttribute("type") +
						" openURL=" + (typeof mine.openURL) +
						" offset=" + mine.offsetWidth + "x" + mine.offsetHeight);
					plog("control stock: attr=" + stock.getAttribute("type") +
						" openURL=" + (typeof stock.openURL) +
						" offset=" + stock.offsetWidth + "x" + stock.offsetHeight);
				} catch (e) { plog("control: EXCEPTION " + e); }
				for (var i = 0; i < made.length; i++) {
					if (made[i].parentNode) { made[i].parentNode.removeChild(made[i]); }
				}
			}, 2000);
		} catch (e) { plog("control: EXCEPTION " + e); }
	}

	// Enyo 1 loads framework kinds before app source, so BasicWebView is usually ready.
	// Retry briefly in case app source is parsed before the palm controls register.
	stamp("B" + JIHAD_PROBE);
	if (!patch()) {
		var t = setInterval(function () { if (patch()) { clearInterval(t); } }, 50);
	}
	stamp("C");

	if (JIHAD_PROBE) {
		// Once at startup (before any refresh) and once after forcing a database rescan: if
		// the MIME only appears after refresh(), the registry was stale when the card's
		// element was laid out, which is a load-ORDER bug, not a packaging one.
		window.setTimeout(function () {
			probeEnv();
			probeRegistry("boot");
			try {
				if (navigator.plugins && navigator.plugins.refresh) {
					navigator.plugins.refresh(false);
					plog("boot: navigator.plugins.refresh(false) called");
					probeRegistry("post-refresh");
				}
			} catch (e) { plog("boot refresh: EXCEPTION " + e); }
			probeControls();
		}, 2500);
	}

	stamp("D" + JIHAD_PROBE);
	// Unguarded second flush: if this line never appears, the IIFE never reached its end.
	window.setTimeout(function () { flushStamps("at-end"); }, 4000);
})();
