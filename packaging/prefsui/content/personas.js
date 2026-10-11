/* Copyright 2026 NotAlexNoyle. MPL-2.0 (matches the GRE this is registered into).
 *
 * about:personas — Jihad's "Get Personas" gallery (cavekit-personas-ui.md R8).
 *
 * The page asks components/jihadPersonas.js for the Personas that come with the browser (and
 * whether each is installed or worn), shows each as a card with its picture, author and licence,
 * and wears one through the component. When the pref jihad.personas.galleryURL names an online
 * index (the same JSON format; see render/goanna/personas/README.md), its Personas are shown too
 * and installed through the component's usual confirm. Everything a card shows from an index is
 * put in as text, never as markup.
 */
"use strict";

const { classes: Cc, interfaces: Ci, utils: Cu } = Components;
Cu.import("resource://gre/modules/Services.jsm");

const PREF_GALLERY = "jihad.personas.galleryURL";

function ask(action, extra) {
	let bag = Cc["@mozilla.org/hash-property-bag;1"].createInstance(Ci.nsIWritablePropertyBag2);
	bag.setPropertyAsAString("action", action);
	for (let k in extra || {}) { bag.setPropertyAsAString(k, String(extra[k])); }
	Services.obs.notifyObservers(bag, "jihad-personas-gallery", null);
	let out = "";
	try { out = bag.getPropertyAsAString("result"); } catch (e) { out = ""; }
	try { return JSON.parse(out || "null"); } catch (e) { return null; }
}

function status(text) {
	let s = document.getElementById("status");
	s.textContent = text;
	s.hidden = !text;
}

function cssColor(c) {
	return (typeof c === "string" && /^[#a-zA-Z0-9(),.%\s-]{1,40}$/.test(c)) ? c : "";
}

function cssUrl(u, httpsOnly) {
	let schemes = httpsOnly ? /^https:[^"'()\\\s]+$/i : /^(https|file|resource):[^"'()\\\s]+$/i;
	return (typeof u === "string" && u.length < 2048 && schemes.test(u)) ? "url(\"" + u + "\")" : "";
}

function isLight(c) {
	let d = document.createElement("div");
	d.style.color = c;
	document.body.appendChild(d);
	let m = /rgba?\((\d+),\s*(\d+),\s*(\d+)/.exec(getComputedStyle(d).color || "");
	d.remove();
	return !!m && (0.299 * m[1] + 0.587 * m[2] + 0.114 * m[3]) > 150;
}

function card(p, button, online) {
	let c = document.createElement("div");
	c.className = "card";
	c.style.backgroundColor = cssColor(p.accentcolor) || "#3a3a3a";
	// An online index's pictures must be https too (the index's https protects nothing if its
	// pictures come over plain http — inspector 2026-10-08). The site's feed names a preview.
	let img = cssUrl(p.previewURL || p.preview || p.headerURL, !!online);
	if (img) { c.style.backgroundImage = img; }
	let text = cssColor(p.textcolor) || "#ffffff";
	let names = document.createElement("div");
	names.className = "names";
	names.style.color = text;
	names.style.background = isLight(text) ? "rgba(0,0,0,0.45)" : "rgba(255,255,255,0.72)";
	let n = document.createElement("div");
	n.className = "name";
	n.textContent = String(p.name || "Persona") + (p.version ? "  " + p.version : "");
	let by = document.createElement("div");
	by.className = "by";
	by.textContent = p.author ? "by " + p.author : "Author unknown";
	names.appendChild(n);
	names.appendChild(by);
	if (p.license) {
		let l = document.createElement("div");
		l.className = "lic";
		l.textContent = "Licence: " + p.license;
		names.appendChild(l);
	}
	c.appendChild(names);
	c.appendChild(button);
	return c;
}

function button(label, disabled, onTap) {
	let b = document.createElement("button");
	b.type = "button";
	b.textContent = label;
	b.disabled = !!disabled;
	b.addEventListener("click", onTap, false);
	return b;
}

function renderBundled() {
	let box = document.getElementById("bundled");
	while (box.firstChild) { box.firstChild.remove(); }
	let list = ask("list") || [];
	list.forEach(function (p) {
		let label = p.worn ? "In use" : "Wear";
		box.appendChild(card(p, button(label, p.worn, function () {
			let r = ask("wear", {key: p.key});
			status(r && r.ok ? "Wearing " + p.name + "." : "That Persona could not be worn.");
			renderBundled();
		})));
	});
	if (!list.length) {
		let d = document.createElement("div");
		d.className = "more";
		d.textContent = "None are bundled with this build.";
		box.appendChild(d);
	}
}

// ── The site's Personas (cavekit-addons-site.md R12): the feed a page at a time, with a search.
// The feed answers {page, pages, total, personas}; a plain index ({personas}) is one page.
const PREF_SITE = "jihad.personas.siteURL";
const PER_PAGE = 12;
let online = {url: "", page: 1, q: ""};

function galleryURL() {
	let url = "";
	try { url = Services.prefs.getCharPref(PREF_GALLERY); } catch (e) { url = ""; }
	return /^https:\/\/[^\s]+$/.test(url) ? url : "";            // only an https index
}

function fetchJSON(url, done) {
	let x = new XMLHttpRequest();
	x.open("GET", url, true);
	x.timeout = 20000;
	x.onload = function () {
		let d = null;
		if (x.status >= 200 && x.status < 300) { try { d = JSON.parse(x.responseText); } catch (e) { d = null; } }
		done(d);
	};
	x.onerror = x.ontimeout = function () { done(null); };
	x.send();
}

function feedURL(page, q) {
	let u = online.url + (online.url.indexOf("?") < 0 ? "?" : "&") + "page=" + page + "&per_page=" + PER_PAGE;
	return q ? u + "&q=" + encodeURIComponent(q) : u;
}

function installedState() {
	let s = ask("state") || {};
	let ids = {};
	(s.installed || []).forEach(function (t) { ids[t.id] = t; });
	return {worn: s.worn || null, ids: ids, list: s.installed || []};
}

function renderOnline() {
	let group = document.getElementById("online-group"), box = document.getElementById("online");
	group.hidden = false;
	status("Loading Personas from the site…");
	fetchJSON(feedURL(online.page, online.q), function (d) {
		status("");
		while (box.firstChild) { box.firstChild.remove(); }
		let pager = document.getElementById("online-pager");
		if (!d) { pager.hidden = true; status("The site could not be reached. Check the connection and try again."); return; }
		let list = Array.isArray(d.personas) ? d.personas.slice(0, 50) : [];
		let state = installedState();
		// The site also lists the Personas that come with Jihad; one of those is worn as the
		// bundled one (same picture), not installed a second time under the site's id.
		let bundled = {};
		(ask("list") || []).forEach(function (b) { bundled[b.name + "\u0000" + b.author] = b; });
		list.forEach(function (p) {
			if (!p || typeof p !== "object" || typeof p.id !== "string") { return; }
			let b = bundled[p.name + "\u0000" + p.author];
			if (b) {
				box.appendChild(card(p, button(b.worn ? "In use" : "Wear", b.worn, function () {
					let r = ask("wear", {key: b.key});
					status(r && r.ok ? "Wearing " + b.name + "." : "That Persona could not be worn.");
					renderBundled();
					renderOnline();
				}), true));
				return;
			}
			let worn = state.worn === p.id, have = !!state.ids[p.id];
			box.appendChild(card(p, button(worn ? "In use" : (have ? "Wear" : "Install"), worn, function () {
				let r = ask("install", {json: JSON.stringify(p), origin: online.url});
				status(r && r.ok ? "Wearing " + (p.name || "the Persona") + "." : (r && r.message) || "Not installed.");
				renderBundled();
				renderOnline();
			}), true));
		});
		if (!list.length) {
			let e = document.createElement("div");
			e.className = "more";
			e.textContent = online.q ? "No Personas match that." : "The site has no Personas right now.";
			box.appendChild(e);
		}
		let pages = Math.max(1, parseInt(d.pages, 10) || 1), page = Math.max(1, parseInt(d.page, 10) || 1);
		pager.hidden = pages < 2;
		document.getElementById("online-page").textContent = "Page " + page + " of " + pages;
		document.getElementById("online-prev").disabled = page <= 1;
		document.getElementById("online-next").disabled = page >= pages;
	});
}

// ── Updates, only when asked (cavekit-addons-site.md R13): each installed Persona whose update
// address is on the site is asked for its current version; a newer one is offered with Update.
function checkUpdates() {
	let box = document.getElementById("updates");
	while (box.firstChild) { box.firstChild.remove(); }
	let origin = "";
	try { origin = Services.io.newURI(online.url, null, null).prePath; } catch (e) { origin = ""; }
	let mine = installedState().list.filter(function (t) {
		return typeof t.updateURL === "string" && t.updateURL.indexOf(origin + "/") === 0;
	}).slice(0, 30);
	if (!mine.length) { status("None of your Personas came from the site, so there is nothing to check."); return; }
	status("Checking " + mine.length + (mine.length === 1 ? " Persona…" : " Personas…"));
	let found = 0, left = mine.length;
	mine.forEach(function (t) {
		fetchJSON(t.updateURL, function (p) {
			if (p && p.id === t.id && typeof p.version === "string" && p.version !== (t.version || "")) {
				found++;
				box.appendChild(card(p, button("Update", false, function () {
					let r = ask("update", {json: JSON.stringify(p), origin: t.updateURL});
					status(r && r.ok ? (p.name || "The Persona") + " is up to date." : (r && r.message) || "Not updated.");
					renderBundled();
					checkUpdates();
				}), true));
			}
			if (--left === 0) {
				status(found ? found + (found === 1 ? " update is" : " updates are") + " ready." : "Your Personas are up to date.");
			}
		});
	});
}

function setupOnline() {
	online.url = galleryURL();
	if (!online.url) { return; }
	document.getElementById("online-search").addEventListener("submit", function (ev) {
		ev.preventDefault();
		online.q = document.getElementById("online-q").value.trim().slice(0, 64);
		online.page = 1;
		renderOnline();
	}, false);
	document.getElementById("online-prev").addEventListener("click", function () { online.page = Math.max(1, online.page - 1); renderOnline(); }, false);
	document.getElementById("online-next").addEventListener("click", function () { online.page++; renderOnline(); }, false);
	document.getElementById("check-updates").addEventListener("click", checkUpdates, false);
	// "More on the web": the site's own Personas page (its buttons work in this browser too).
	let site = "";
	try { site = Services.prefs.getCharPref(PREF_SITE); } catch (e) { site = ""; }
	if (!/^https:\/\/[^\s]+$/.test(site)) {
		try { site = Services.io.newURI(online.url, null, null).prePath + "/personas/"; } catch (e) { site = ""; }
	}
	let web = document.getElementById("online-web");
	if (site) { web.href = site; web.hidden = false; }
	renderOnline();
}

renderBundled();
setupOnline();
