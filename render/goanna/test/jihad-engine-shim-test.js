// Copyright 2026 NotAlexNoyle. MPL-2.0.
// jihad-engine-shim-test.js — T-386 (cavekit-engine-update R4b): JihadEngine.jsm outside the engine,
// with a fake Components and document. Run: node render/goanna/test/jihad-engine-shim-test.js render/goanna/modules/JihadEngine.jsm
// unit test of the shim outside the engine: a fake Components and document
const fs = require("fs");
const src = fs.readFileSync(process.argv[2], "utf8");
const imported = [];
const Components = { utils: { import(url, scope) { imported.push(url); const n = url.replace(/.*\//, "").replace(".jsm", ""); return { [n]: { name: n } }; } } };
const ctx = {};
new Function("Components", src + "\nreturn this;").call(ctx, Components);
const E = ctx.JihadEngine;
let fails = 0; const ok = (c, m) => { if (!c) { fails++; console.log("FAIL", m); } };
ok(E.LightweightThemeManager.name === "LightweightThemeManager", "LTM by name");
ok(E.AddonManager.name === "AddonManager", "AM by name");
ok(imported[1] === "resource://gre/modules/AddonManager.jsm", "AM url");
let threw = false; try { E.module("Nope"); } catch (e) { threw = true; } ok(threw, "unknown module throws");
ok(E.addons.selector("page") === "#addons-page", "selector");
ok(E.addons.isDocument({ documentURI: "about:addons" }) && !E.addons.isDocument({ documentURI: "about:blank" }), "isDocument");
const doc = { getElementById: id => ({ id }), getAnonymousElementByAttribute: (i, a, v) => ({ a, v }) };
ok(E.addons.element(doc, "categoryTheme").id === "category-theme", "element");
ok(E.addons.part(doc, {}, "anonid", "remove-btn").v === "remove-btn", "part");
threw = false; try { E.addons.part(doc, {}, "anonid", "not-listed"); } catch (e) { threw = true; } ok(threw, "unlisted part throws");
ok(E.addons.currentViewId({ gViewController: { currentViewId: "addons://list/theme" } }) === "addons://list/theme", "view id");
console.log(fails ? fails + " FAILED" : "shim test: all passed");
process.exit(fails ? 1 : 0);
