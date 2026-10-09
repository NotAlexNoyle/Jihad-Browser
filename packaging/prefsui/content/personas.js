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

function cssUrl(u) {
	return (typeof u === "string" && u.length < 2048 && /^(https?|file|resource):[^"'()\\\s]+$/i.test(u)) ? "url(\"" + u + "\")" : "";
}

function isLight(c) {
	let d = document.createElement("div");
	d.style.color = c;
	document.body.appendChild(d);
	let m = /rgba?\((\d+),\s*(\d+),\s*(\d+)/.exec(getComputedStyle(d).color || "");
	d.remove();
	return !!m && (0.299 * m[1] + 0.587 * m[2] + 0.114 * m[3]) > 150;
}

function card(p, button) {
	let c = document.createElement("div");
	c.className = "card";
	c.style.backgroundColor = cssColor(p.accentcolor) || "#3a3a3a";
	let img = cssUrl(p.preview || p.headerURL);
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

function renderOnline() {
	let url = "";
	try { url = Services.prefs.getCharPref(PREF_GALLERY); } catch (e) { url = ""; }
	if (!/^https:\/\//.test(url)) { return; }                  // only an https index
	let group = document.getElementById("online-group"), box = document.getElementById("online");
	group.hidden = false;
	status("Loading the gallery…");
	let x = new XMLHttpRequest();
	x.open("GET", url, true);
	x.timeout = 20000;
	x.onload = function () {
		status("");
		let d = null;
		try { d = JSON.parse(x.responseText); } catch (e) { d = null; }
		let list = (d && Array.isArray(d.personas)) ? d.personas.slice(0, 200) : [];
		list.forEach(function (p) {
			if (!p || typeof p !== "object") { return; }
			box.appendChild(card(p, button("Install", false, function () {
				let r = ask("install", {json: JSON.stringify(p), origin: url});
				status(r && r.ok ? "Wearing " + (p.name || "the Persona") + "." : (r && r.message) || "Not installed.");
				renderBundled();
			})));
		});
		if (!list.length) { status("The gallery has no Personas right now."); }
	};
	x.onerror = x.ontimeout = function () { status("The gallery could not be reached."); };
	x.send();
}

renderBundled();
renderOnline();
