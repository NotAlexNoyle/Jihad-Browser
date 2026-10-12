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

// What an action came to, shown over the bottom of the page for a while: it is never put above the
// list, where showing and hiding it moved every card (the owner, 2026-10-12: "the active persona
// seems to flicker in about:personas" while the page loads).
// A change of look offers Undo there while the component still holds the look before.
let statusTimer = 0;
function status(text, undoable) {
	let s = document.getElementById("status");
	document.getElementById("status-text").textContent = text;
	document.getElementById("status-undo").hidden = !undoable;
	s.hidden = !text;
	clearTimeout(statusTimer);
	if (text) { statusTimer = setTimeout(function () { s.hidden = true; }, undoable ? 30000 : 8000); }
}

function cssColor(c) {
	return (typeof c === "string" && /^[#a-zA-Z0-9(),.%\s-]{1,40}$/.test(c)) ? c : "";
}

function cssUrl(u, httpsOnly) {
	let schemes = httpsOnly ? /^https:[^"'()\\\s]+$/i : /^(https|file|resource):[^"'()\\\s]+$/i;
	return (typeof u === "string" && u.length < 2048 && schemes.test(u)) ? "url(\"" + u + "\")" : "";
}

// Whether a colour is light, read through a canvas's colour parser: putting an element into the page
// to measure it laid the whole page out again for every card.
let colourParser = null;
function isLight(c) {
	if (!colourParser) { colourParser = document.createElementNS("http://www.w3.org/1999/xhtml", "canvas").getContext("2d"); }
	colourParser.fillStyle = "#000000";
	colourParser.fillStyle = c;
	let v = String(colourParser.fillStyle), m = /^#([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})$/i.exec(v), rgb;
	if (m) { rgb = [parseInt(m[1], 16), parseInt(m[2], 16), parseInt(m[3], 16)]; }
	else { m = /rgba?\((\d+),\s*(\d+),\s*(\d+)/.exec(v); rgb = m ? [+m[1], +m[2], +m[3]] : null; }
	return !!rgb && (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]) > 150;
}

// A Persona's card: its picture over its accent colour, its names, and one button or several.
function card(p, button, online) {
	let c = document.createElement("div");
	c.className = "card";
	c.setAttribute("data-id", String(p.key || p.id || ""));
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
	by.textContent = p.note ? p.note : (p.author ? "by " + p.author : "Author unknown");
	names.appendChild(n);
	names.appendChild(by);
	if (p.license) {
		let l = document.createElement("div");
		l.className = "lic";
		l.textContent = "Licence: " + p.license;
		names.appendChild(l);
	}
	c.appendChild(names);
	(Array.isArray(button) ? button : [button]).forEach(function (b) { c.appendChild(b); });
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

// Every card's button, by the Persona's id: after a Wear or an Install only the labels change, so the
// cards already on the page are never drawn again.
let buttons = {};

function showButton(id, worn, have) {
	let b = buttons[id];
	if (!b) { return; }
	b.textContent = worn ? "In use" : (have ? "Wear" : "Install");
	b.disabled = !!worn;
}

function refreshButtons() {
	let state = installedState();
	(ask("list") || []).forEach(function (p) { showButton(p.key, p.worn, true); });
	Object.keys(buttons).forEach(function (id) {
		if (buttons[id].getAttribute("data-kind") === "site") { showButton(id, state.worn === id, !!state.ids[id]); }
		// (a theme from addons.mozilla.org gets its id from its picture when converted: its card
		// learns what became of it from the component's answer instead)
	});
}

// ── Your Personas: the ones installed, worn, taken off, removed and undone here (the owner,
// 2026-10-12: the card cannot scroll about:addons' list, so its Personas category is short and leads
// here). The worn one first; Remove asks again before it acts, as the settings page's Clear does.
function renderMine() {
	let box = document.getElementById("mine");
	while (box.firstChild) { box.firstChild.remove(); }
	let m = ask("mine") || {list: []};
	let list = (m.list || []).slice().sort(function (a, b) { return (b.worn ? 1 : 0) - (a.worn ? 1 : 0); });
	let anyWorn = list.some(function (t) { return t.worn; });
	let plain = button(anyWorn ? "Wear" : "In use", !anyWorn, function () {
		let r = ask("takeOff");
		status(r && r.ok ? "Persona taken off." : "The look could not be changed.", !!(r && r.ok));
		afterChange();
	});
	box.appendChild(card({name: "Default look", note: "The browser's own colours, no Persona.", accentcolor: "#2a2a2a", textcolor: "#e6e6e6"}, plain));
	list.forEach(function (t) {
		let wear = button(t.worn ? "Take off" : "Wear", false, function () {
			let r = ask(t.worn ? "takeOff" : "wearId", {id: t.id});
			status(r && r.ok ? (t.worn ? "Persona taken off." : "Wearing " + t.name + ".") : "The look could not be changed.", !!(r && r.ok));
			afterChange();
		});
		let sure = false;
		let remove = button("Remove", false, function () {
			if (!sure) { sure = true; remove.textContent = "Remove it?"; remove.className = "ask"; return; }
			let r = ask("remove", {id: t.id});
			status(r && r.ok ? "Removing " + t.name + "\u2026" : t.name + " could not be removed.");
		});
		box.appendChild(card({key: t.id, name: t.name, author: t.author, version: t.version, license: t.license,
		                      accentcolor: t.accentcolor, textcolor: t.textcolor, preview: t.preview}, [wear, remove]));
	});
}

// After a change: Your Personas again, and every other card's button.
function afterChange() {
	renderMine();
	refreshButtons();
}

function setupMine() {
	renderMine();
	document.getElementById("status-undo").addEventListener("click", function () {
		let r = ask("undo");
		status(r && r.ok ? "Undone." : "There is nothing to undo now.");
		afterChange();
	}, false);
	// Install from file: the theme files on the device (the downloads folder and the USB drive's top).
	document.getElementById("from-file").addEventListener("click", function () {
		let box = document.getElementById("files");
		while (box.firstChild) { box.firstChild.remove(); }
		let files = ask("files") || [];
		if (!files.length) {
			let d = document.createElement("div");
			d.className = "more";
			d.textContent = "No .xpi files in the downloads folder or on the USB drive.";
			box.appendChild(d);
		}
		files.forEach(function (f) {
			let b = button(String(f.name), false, function () {
				box.hidden = true;
				ask("installFile", {path: f.path});
				afterChange();
			});
			b.className = "file";
			box.appendChild(b);
		});
		box.hidden = !box.hidden;
	}, false);
	// A removal finishes in the add-on manager a moment later.
	let watcher = {observe: function () { afterChange(); }};
	Services.obs.addObserver(watcher, "jihad-personas-mine", false);
	window.addEventListener("unload", function () { Services.obs.removeObserver(watcher, "jihad-personas-mine"); }, false);
}

// The Personas that come with Jihad and are not installed (one removed can be had again here); the
// installed ones are under Your Personas.
function renderBundled() {
	let box = document.getElementById("bundled");
	let list = ask("list") || [];
	list.filter(function (p) { return !p.installed; }).forEach(function (p) {
		let b = button("Wear", false, function () {
			let r = ask("wear", {key: p.key});
			status(r && r.ok ? "Wearing " + p.name + "." : "That Persona could not be worn.", !!(r && r.ok));
			b.disabled = true;
			afterChange();
		});
		b.setAttribute("data-kind", "bundled");
		buttons[p.key] = b;
		box.appendChild(card(p, b));
	});
	document.getElementById("bundled-group").hidden = !box.firstChild;
	return list;
}

// ── The add-on site's Personas and addons.mozilla.org's themes (the owner, 2026-10-12: "preview only
// 2 personas per category + full, fast remote search and links to websites"): each shows two when the
// page opens — the site's newest, AMO's most popular, small answers that keep the page quick — a
// search of the whole of either in place, and a link to its website. AMO's web pages stop this
// engine with a robot check, so its themes come from its public search API. Search results come a
// page at a time, More results for the next; none is shown twice, and a Persona that comes with
// Jihad is not shown again under the site's copy (same id; an older site matched by name and author).
const PREF_SITE = "jihad.personas.siteURL";
const SITE_PER_PAGE = 24;
const AMO_PER_PAGE = 12;
const PREVIEW_COUNT = 2;
const AMO_API = "https://addons.mozilla.org/api/v5/addons/search/";
let online = {url: "", checkSeq: 0, bundled: {}, named: {}};

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

function installedState() {
	let s = ask("state") || {};
	let ids = {};
	(s.installed || []).forEach(function (t) { ids[t.id] = t; });
	return {worn: s.worn || null, ids: ids, list: s.installed || []};
}

// A searched list. o: {list, note, more, form, input, url(page, q), previewURL, read(answer) →
// {items, pages}, card(item, state), empty}. It opens with PREVIEW_COUNT items from previewURL; a
// search replaces them with the full results. A newer search drops an older one's late answers (seq).
function searchedList(o) {
	let src = {q: "", next: 1, pages: 1, loading: false, seq: 0};
	let note = function (text) { o.note.textContent = text; o.note.hidden = !text; };
	src.load = function () {
		if (src.loading || src.next > src.pages) { return; }
		src.loading = true;
		o.more.hidden = true;
		if (src.next === 1) { note("Searching\u2026"); }
		let mine = src.seq;
		fetchJSON(o.url(src.next, src.q), function (d) {
			if (mine !== src.seq) { return; }
			src.loading = false;
			let got = d ? o.read(d) : null;
			if (!got) {
				note("The site could not be reached. Check the connection and try again.");
				o.more.hidden = src.next === 1;
				return;
			}
			src.pages = got.pages;
			let state = installedState(), added = 0;
			got.items.forEach(function (it) {
				if (buttons[it.id]) { return; }                   // shown already (another page, or above)
				o.list.appendChild(o.card(it, state));
				added++;
			});
			src.next++;
			note(src.next === 2 && !added ? o.empty(src.q) : "");
			o.more.hidden = src.next > src.pages;
		});
	};
	src.search = function (q) {
		src.seq++;
		src.q = q;
		src.next = 1;
		src.pages = 1;
		src.loading = false;
		while (o.list.firstChild) {
			let id = o.list.firstChild.getAttribute && o.list.firstChild.getAttribute("data-id");
			if (id) { delete buttons[id]; }
			o.list.firstChild.remove();
		}
		src.load();
	};
	o.form.addEventListener("submit", function (ev) {
		ev.preventDefault();
		src.search(o.input.value.trim().slice(0, 64));
	}, false);
	o.more.addEventListener("click", src.load, false);
	// The preview: a few, no More results (a search is the way to the rest).
	let mine = src.seq;
	fetchJSON(o.previewURL, function (d) {
		if (mine !== src.seq) { return; }                   // a search came first
		let got = d ? o.read(d) : null;
		if (!got) { note("The site could not be reached. Check the connection and try again."); return; }
		let state = installedState();
		got.items.slice(0, PREVIEW_COUNT).forEach(function (it) {
			if (!buttons[it.id]) { o.list.appendChild(o.card(it, state)); }
		});
	});
	return src;
}

function siteCard(p, state) {
	let worn = state.worn === p.id, have = !!state.ids[p.id];
	let b = button(worn ? "In use" : (have ? "Wear" : "Install"), worn, function () {
		let r = ask("install", {json: JSON.stringify(p), origin: online.url});
		status(r && r.ok ? "Wearing " + (p.name || "the Persona") + "." : (r && r.message) || "Not installed.", !!(r && r.ok));
		afterChange();
	});
	b.setAttribute("data-kind", "site");
	buttons[p.id] = b;
	return card(p, b, true);
}

function setupSite() {
	online.url = galleryURL();
	if (!online.url) { return; }
	document.getElementById("online-group").hidden = false;
	let src = searchedList({
		list: document.getElementById("online"), note: document.getElementById("online-note"),
		more: document.getElementById("online-more"), form: document.getElementById("online-search"),
		input: document.getElementById("online-q"),
		// Six asked for, two shown: the ones that come with Jihad are left out of the site's preview.
		previewURL: online.url + (online.url.indexOf("?") < 0 ? "?" : "&") + "page=1&per_page=6",
		url: function (page, q) {
			let u = online.url + (online.url.indexOf("?") < 0 ? "?" : "&") + "page=" + page + "&per_page=" + SITE_PER_PAGE;
			return q ? u + "&q=" + encodeURIComponent(q) : u;
		},
		read: function (d) {
			let items = (Array.isArray(d.personas) ? d.personas.slice(0, 50) : []).filter(function (p) {
				return p && typeof p === "object" && typeof p.id === "string" && p.id &&
					!online.bundled[p.id] && !online.named[p.name + "\u0000" + p.author];
			});
			return {items: items, pages: Math.min(50, Math.max(1, parseInt(d.pages, 10) || 1))};
		},
		card: siteCard,
		empty: function (q) { return q ? "No Personas match that." : "The site has no other Personas right now."; }
	});
	// A search can be linked to: about:personas#site-q=WORDS searches the site for WORDS when the
	// page opens, and shows the results.
	let linked = /^#site-q=(.{0,64})$/.exec(location.hash || "");
	if (linked) {
		let q = "";
		try { q = decodeURIComponent(linked[1]); } catch (e) { q = ""; }
		document.getElementById("online-q").value = q;
		src.search(q.trim());
		document.getElementById("online-group").scrollIntoView();
	}
	document.getElementById("check-updates").addEventListener("click", checkUpdates, false);
	// The site's own Personas page, its website (its buttons work in this browser too).
	let site = "";
	try { site = Services.prefs.getCharPref(PREF_SITE); } catch (e) { site = ""; }
	if (!/^https:\/\/[^\s]+$/.test(site)) {
		try { site = Services.io.newURI(online.url, null, null).prePath + "/personas/"; } catch (e) { site = ""; }
	}
	let web = document.getElementById("online-web");
	if (site) { web.href = site; web.hidden = false; }
}

// addons.mozilla.org's translated fields are objects by language; the API is asked for English.
function amoText(v) {
	if (typeof v === "string") { return v; }
	if (v && typeof v === "object") { return String(v["en-US"] || v[Object.keys(v)[0]] || ""); }
	return "";
}

// Its themes, read from the API's answer: only what a card shows, as text, and an https file on
// addons.mozilla.org to install from.
function amoItem(r) {
	if (!r || typeof r !== "object" || !r.current_version || !r.current_version.file) { return null; }
	let file = String(r.current_version.file.url || "");
	if (!/^https:\/\/addons\.mozilla\.org\/[^\s"'<>]+\.xpi$/.test(file)) { return null; }
	let pv = Array.isArray(r.previews) && r.previews[0] ? (r.previews[0].thumbnail_url || r.previews[0].image_url) : "";
	let lic = r.current_version.license ? amoText(r.current_version.license.name) : "";
	return {id: "amo:" + String(r.id), name: amoText(r.name).slice(0, 80) || "Theme",
	        author: Array.isArray(r.authors) && r.authors[0] ? String(r.authors[0].name || "").slice(0, 60) : "",
	        version: String(r.current_version.version || "").slice(0, 20), license: lic.slice(0, 60) || "Licence not given",
	        previewURL: String(pv || ""), file: file};
}

// Install: the component fetches the theme file from addons.mozilla.org and converts it as it does
// any theme file, asking first; its answer comes back later ("jihad-personas-amo").
let amoPending = {};
function amoCard(it) {
	let b = button("Install", false, function () {
		let r = ask("amo", {url: it.file, name: it.name});
		if (r && r.ok) {
			amoPending[it.file] = b;
			b.textContent = "Fetching\u2026";
			b.disabled = true;
		} else {
			status((r && r.message) || "That theme could not be installed.");
		}
	});
	b.setAttribute("data-kind", "amo");
	buttons[it.id] = b;
	return card(it, b, true);
}

function setupAmo() {
	searchedList({
		list: document.getElementById("amo"), note: document.getElementById("amo-note"),
		more: document.getElementById("amo-more"), form: document.getElementById("amo-search"),
		input: document.getElementById("amo-q"),
		previewURL: AMO_API + "?app=firefox&type=statictheme&lang=en-US&sort=users&page_size=" + PREVIEW_COUNT,
		url: function (page, q) {
			return AMO_API + "?app=firefox&type=statictheme&lang=en-US&page_size=" + AMO_PER_PAGE + "&page=" + page +
				(q ? "&q=" + encodeURIComponent(q) : "&sort=users");
		},
		read: function (d) {
			if (!Array.isArray(d.results)) { return null; }
			let count = Math.max(0, parseInt(d.count, 10) || 0);
			return {items: d.results.slice(0, 50).map(amoItem).filter(Boolean),
			        pages: Math.min(40, Math.max(1, Math.ceil(count / AMO_PER_PAGE)))};
		},
		card: amoCard,
		empty: function (q) { return q ? "No themes match that." : "addons.mozilla.org sent no themes."; }
	});
	let watcher = {
		observe: function (subject, topic, data) {
			let r = null;
			try { r = JSON.parse(data); } catch (e) { r = null; }
			if (!r || !amoPending[r.url]) { return; }
			let b = amoPending[r.url];
			delete amoPending[r.url];
			b.textContent = r.ok ? "In use" : "Install";
			b.disabled = !!r.ok;
			status(r.message || (r.ok ? "Done." : "That theme could not be installed."), !!r.ok);
			if (r.ok) { afterChange(); }
		}
	};
	Services.obs.addObserver(watcher, "jihad-personas-amo", false);
	window.addEventListener("unload", function () { Services.obs.removeObserver(watcher, "jihad-personas-amo"); }, false);
}

// ── Updates, only when asked (cavekit-addons-site.md R13): each installed Persona whose update
// address is on the site is asked for its current version; a newer one is offered with Update.
function checkUpdates() {
	let box = document.getElementById("updates");
	while (box.firstChild) { box.firstChild.remove(); }
	let mineCheck = ++online.checkSeq;
	let origin = "";
	try { origin = Services.io.newURI(online.url, null, null).prePath; } catch (e) { origin = ""; }
	let mine = installedState().list.filter(function (t) {
		return typeof t.updateURL === "string" && t.updateURL.indexOf(origin + "/") === 0;
	}).slice(0, 30);
	if (!mine.length) { status("None of your Personas came from the site, so there is nothing to check."); return; }
	status("Checking " + mine.length + (mine.length === 1 ? " Persona…" : " Personas…"));
	let found = 0, failed = 0, left = mine.length;
	mine.forEach(function (t) {
		fetchJSON(t.updateURL, function (p) {
			if (mineCheck !== online.checkSeq) { return; }  // a newer check replaced this one
			// An answer about another Persona, or with no version, is no answer; only a HIGHER
			// version is an update, never an older or an equal one written differently (Codex
			// 2026-10-12, round 17).
			let usable = !!p && p.id === t.id && typeof p.version === "string" && p.version !== "";
			if (!usable) { failed++; }
			let newer = false;
			try { newer = usable && Services.vc.compare(p.version, t.version || "0") > 0; } catch (e) { newer = false; }
			if (newer) {
				found++;
				box.appendChild(card(p, button("Update", false, function () {
					let r = ask("update", {json: JSON.stringify(p), origin: t.updateURL});
					status(r && r.ok ? (p.name || "The Persona") + " is up to date." : (r && r.message) || "Not updated.");
					afterChange();
					checkUpdates();
				}), true));
			}
			if (--left === 0) {
				// No answer is not "up to date" (inspector 2026-10-11, round 15).
				if (failed === mine.length) { status("The site could not be reached. Check the connection and try again."); return; }
				let msg = found ? found + (found === 1 ? " update is" : " updates are") + " ready." : "Your Personas are up to date.";
				if (failed) { msg += " " + failed + (failed === 1 ? " Persona" : " Personas") + " could not be checked."; }
				status(msg);
			}
		});
	});
}

(function () {
	setupMine();
	renderBundled().forEach(function (b) { online.bundled[b.key] = true; online.named[b.name + "\u0000" + b.author] = true; });
	setupSite();
	setupAmo();
})();
