#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
"""check-engine-api.py — does this UXP tree still offer everything Jihad Browser uses from it?

Run it against the engine you build today (it must pass) and against a candidate tree before an
engine update (it lists what the update would break, by touchpoint):

    build/engine/check-engine-api.py                       # third_party/uxp
    build/engine/check-engine-api.py --uxp /path/to/new/uxp
    build/engine/check-engine-api.py --write-baseline      # record today's signatures

What it checks (docs/ENGINE.md "Touchpoints" explains each):
  1. our engine exports — every jihad_* symbol in render/goanna/EngineCompat.h is defined in the
     engine (they come from our engine branch; an upstream tree without our commits fails here,
     which is the point: rebase jihad-engine-mods first);
  2. IDL methods the daemon calls with long positional argument lists, against the baseline;
  3. interfaces the daemon implements or redeclares (JihadAppInfo, the hand-copied
     nsIClipboardHelper IID) — a new attribute or a changed IID breaks them silently;
  4. engine JS the components depend on (LightweightThemeManager, AddonManager, imgITools, the
     about:addons markup the Personas pane decorates);
  5. every engine pref the project sets still exists in the engine's defaults (a renamed pref is
     silently ignored);
  6. the mozconfig options still exist in the configure scripts;
  7. the ICU data file the bundler hard-codes is the one the tree ships.
Exit status: 0 when nothing FAILs (WARNs are for a human to read), 1 otherwise.
"""
import argparse, glob, json, os, re, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
BASELINE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "engine-api-baseline.json")

# IDL methods called with positional argument lists: file, method.
IDL_METHODS = [
    ("docshell/base/nsIWebNavigation.idl", "loadURIWithOptions"),
    ("dom/interfaces/base/nsIDOMWindowUtils.idl", "sendMouseEventToWindow"),
    ("dom/interfaces/base/nsIDOMWindowUtils.idl", "sendMouseEvent"),
    ("dom/interfaces/base/nsIDOMWindowUtils.idl", "sendKeyEvent"),
    ("dom/interfaces/base/nsIDOMWindowUtils.idl", "sendTouchEventToWindow"),
    ("dom/interfaces/base/nsIDOMWindowUtils.idl", "getViewportInfo"),
    ("dom/interfaces/base/nsIDOMWindowUtils.idl", "nodesFromRect"),
    ("dom/interfaces/base/nsIDOMWindowUtils.idl", "elementFromPoint"),
    ("dom/interfaces/base/nsIDOMWindowUtils.idl", "loadSheetUsingURIString"),
    ("image/imgITools.idl", "decodeImage"),
    ("image/imgITools.idl", "encodeScaledImage"),
]
# Interfaces the daemon implements in full (JihadAppInfo, EngineHost.cpp): their member lists.
IMPLEMENTED = [
    ("xpcom/system/nsIXULAppInfo.idl", "nsIXULAppInfo"),
    ("xpcom/system/nsIXULRuntime.idl", "nsIXULRuntime"),
    ("xpcom/system/nsIPlatformInfo.idl", "nsIPlatformInfo"),
]
# Engine JS: file, the names that must still be there.
JS_NEEDS = [
    ("toolkit/mozapps/extensions/LightweightThemeManager.jsm",
     ["parseTheme", "previewTheme", "resetPreview", "currentTheme", "usedThemes", "forgetUsedTheme",
      '"lightweightThemes."', "usedThemes", "lightweight-theme-styling-update", "@personas.mozilla.org"]),
    ("toolkit/mozapps/extensions/AddonManager.jsm",
     ["getInstallForFile", "getAddonByID", "addAddonListener"]),
    ("toolkit/mozapps/extensions/content/extensions.xul",
     ['id="list-view"', 'id="addon-list"', 'id="addon-list-empty"', 'id="addons-page"']),
    ("toolkit/mozapps/extensions/content/extensions.xml",
     ['anonid="enable-btn"', 'anonid="disable-btn"', 'anonid="remove-btn"', 'anonid="version"',
      'anonid="description"', 'anonid="control-container"', 'class="content-inner-container"',
      'class="disabled-postfix"']),
    ("toolkit/mozapps/extensions/content/extensions.js",
     ["gViewController", "currentViewId", '"ViewChanged"', "extensions.ui.lastCategory",
      '"category-" + aId']),                         # the Personas pane renames #category-theme
]
CLIPBOARD_IDL = "widget/nsIClipboardHelper.idl"
# Prefs the project sets that the engine has no default for, on purpose — read by our own code.
OURS_ONLY = {
    "privacy.donottrackheader.enabled": "UXP dropped DNT; EngineHost.cpp sends the header itself",
    "plugin.state.flash": "per-plugin pref the engine composes at run time (plugin.state.<name>)",
}


def read(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read()


class Report:
    def __init__(self):
        self.fails = self.warns = 0

    def ok(self, msg):
        print("  ok    " + msg)

    def warn(self, msg):
        self.warns += 1
        print("  WARN  " + msg)

    def fail(self, msg):
        self.fails += 1
        print("  FAIL  " + msg)


def idl_method_params(text, method):
    m = re.search(r"\b" + re.escape(method) + r"\s*\(", text)
    if not m:
        return None
    depth, i = 1, m.end()
    while i < len(text) and depth:
        depth += {"(": 1, ")": -1}.get(text[i], 0)
        i += 1
    body = re.sub(r"/\*.*?\*/", "", text[m.end():i - 1], flags=re.S)
    params = [p.strip() for p in body.split(",") if p.strip()]
    return [re.sub(r"\s+", " ", p) for p in params]


def idl_members(text, iface):
    m = re.search(r"interface\s+" + re.escape(iface) + r"\b[^{]*\{", text)
    if not m:
        return None
    depth, i = 1, m.end()
    while i < len(text) and depth:
        depth += {"{": 1, "}": -1}.get(text[i], 0)
        i += 1
    body = re.sub(r"/\*.*?\*/", "", text[m.end():i - 1], flags=re.S)
    body = re.sub(r"//[^\n]*", "", body)
    names = set()
    for decl in body.split(";"):
        d = re.sub(r"\[[^\]]*\]", "", decl).strip()
        if not d or d.startswith("%{") or d.startswith("const "):
            continue
        a = re.search(r"attribute\s+[\w\s:<>]+?\s+(\w+)\s*$", d)
        f = re.search(r"(\w+)\s*\(", d)
        if a:
            names.add("attr " + a.group(1))
        elif f:
            names.add("method " + f.group(1))
    return sorted(names)


def gather(uxp):
    """Today's signatures, for the baseline."""
    out = {"idl_methods": {}, "implemented": {}}
    for f, m in IDL_METHODS:
        p = os.path.join(uxp, f)
        if os.path.exists(p):
            out["idl_methods"][m] = idl_method_params(read(p), m)
    for f, i in IMPLEMENTED:
        p = os.path.join(uxp, f)
        if os.path.exists(p):
            out["implemented"][i] = idl_members(read(p), i)
    return out


def check(uxp, rep):
    base = json.load(open(BASELINE)) if os.path.exists(BASELINE) else None
    if base is None:
        rep.warn("no baseline (%s): run with --write-baseline on the engine you build today" % BASELINE)

    print("1. our engine exports (render/goanna/EngineCompat.h)")
    compat = read(os.path.join(REPO, "render/goanna/EngineCompat.h"))
    names = sorted(set(re.findall(r"\b(jihad_\w+)\s*\(", compat)))
    sources = []
    for pat in ("widget/**/*.cpp", "dom/plugins/**/*.cpp", "media/libcubeb/**/*.c", "security/manager/**/*.cpp",
                "gfx/**/*.cpp", "layout/**/*.cpp", "toolkit/**/*.cpp", "xpcom/**/*.cpp"):
        sources += glob.glob(os.path.join(uxp, pat), recursive=True)
    texts = {}
    for n in names:
        found = False
        for s in sources:
            t = texts.get(s)
            if t is None:
                try:
                    t = texts[s] = read(s)
                except OSError:
                    continue
            if re.search(r"\b" + n + r"\s*\([^;{]*\)\s*\{", t):
                found = True
                rep.ok("%s defined in %s" % (n, os.path.relpath(s, uxp)))
                break
        if not found:
            rep.fail("%s is not defined in this engine tree (our engine branch not applied, or the export was dropped)" % n)

    print("2. IDL methods called with positional arguments")
    for f, m in IDL_METHODS:
        p = os.path.join(uxp, f)
        if not os.path.exists(p):
            rep.fail("%s: %s is gone" % (m, f)); continue
        params = idl_method_params(read(p), m)
        if params is None:
            rep.fail("%s no longer in %s" % (m, f)); continue
        want = base and base["idl_methods"].get(m)
        if want is not None and params != want:
            rep.fail("%s changed: %d params now, %d in the baseline — update every call (docs/ENGINE.md)" % (m, len(params), len(want)))
            for a, b in zip(want + [""] * 9, params + [""] * 9):
                if a != b:
                    print("          baseline: %-60s now: %s" % (a, b))
        else:
            rep.ok("%s (%d params)" % (m, len(params)))

    print("3. interfaces the daemon implements or redeclares")
    for f, i in IMPLEMENTED:
        p = os.path.join(uxp, f)
        members = idl_members(read(p), i) if os.path.exists(p) else None
        if members is None:
            rep.fail("%s not found in %s" % (i, f)); continue
        want = base and base["implemented"].get(i)
        if want is not None and members != want:
            added = sorted(set(members) - set(want)); gone = sorted(set(want) - set(members))
            rep.fail("%s changed (JihadAppInfo, EngineHost.cpp, implements it): added %s, removed %s" % (i, added, gone))
        else:
            rep.ok("%s (%d members)" % (i, len(members)))
    p = os.path.join(uxp, CLIPBOARD_IDL)
    uuid = re.search(r"uuid\(([0-9a-f-]+)\)", read(p)) if os.path.exists(p) else None
    host = read(os.path.join(REPO, "render/goanna/EngineHost.cpp"))
    hm = re.search(r"JIHAD_NS_ICLIPBOARDHELPER_IID\s*\\\s*\{\s*0x([0-9a-f]+),\s*0x([0-9a-f]+),\s*0x([0-9a-f]+),\s*\{([^}]*)\}", host)
    if not uuid or not hm:
        rep.fail("nsIClipboardHelper: IID not found (idl %s, EngineHost.cpp %s)" % (bool(uuid), bool(hm)))
    else:
        tail = "".join(x.strip()[2:] for x in hm.group(4).split(","))
        ours = "%s-%s-%s-%s-%s" % (hm.group(1), hm.group(2), hm.group(3), tail[:4], tail[4:])
        if ours.lower() != uuid.group(1).lower():
            rep.fail("nsIClipboardHelper IID changed: engine %s, EngineHost.cpp's copy %s" % (uuid.group(1), ours))
        else:
            rep.ok("nsIClipboardHelper IID matches EngineHost.cpp's copy")

    print("4. engine JS the components depend on")
    for f, needs in JS_NEEDS:
        p = os.path.join(uxp, f)
        if not os.path.exists(p):
            rep.fail("%s is gone" % f); continue
        t = read(p)
        missing = [n for n in needs if n not in t]
        if missing:
            rep.fail("%s: no longer has %s" % (f, missing))
        else:
            rep.ok("%s (%d names)" % (f, len(needs)))

    print("5. engine prefs the project sets")
    defaults = ""
    for pat in ("modules/libpref/init/all.js", "**/*-prefs.js", "**/greprefs.js", "toolkit/**/*.js",
                "xulrunner/**/*.js", "netwerk/**/*.js", "security/**/*.js", "dom/**/*.js"):
        for f in glob.glob(os.path.join(uxp, pat), recursive=True):
            try:
                defaults += read(f)
            except OSError:
                pass
    ours = []
    for f in glob.glob(os.path.join(REPO, "packaging/prefs/*.js")):
        ours += re.findall(r'pref\(\s*"([^"]+)"', read(f))
    unknown = sorted(set(n for n in ours if not n.startswith("jihad.") and ('"%s"' % n) not in defaults))
    # A pref with no default may still be one the engine READS (Preferences::GetBool("…") and
    # the like): look for the name anywhere in the engine's sources before warning.
    import subprocess
    def read_somewhere(name):
        try:
            r = subprocess.run(["git", "-C", uxp, "grep", "-q", "-F", '"%s"' % name], timeout=60)
            return r.returncode == 0
        except Exception:
            return False
    unknown = [n for n in unknown if re.match(r"^[\w.-]+$", n) and n not in OURS_ONLY and not read_somewhere(n)]
    for n in unknown:
        rep.warn("pref %s is not in the engine's defaults (renamed upstream, or one only we read)" % n)
    rep.ok("%d prefs set, %d not found in the engine's defaults" % (len(set(ours)), len(unknown)))

    print("6. mozconfig options")
    conf = ""
    for pat in ("**/*.configure", "old-configure.in", "js/src/old-configure.in", "build/autoconf/*.m4"):
        for f in glob.glob(os.path.join(uxp, pat), recursive=True):
            try:
                conf += read(f)
            except OSError:
                pass
    for mc in glob.glob(os.path.join(REPO, "build/**/mozconfig*"), recursive=True):
        for opt in re.findall(r"ac_add_options\s+--(?:enable|disable|with|without)-([\w-]+)", read(mc)):
            if not re.search(r"(enable|disable|with|without)-" + re.escape(opt) + r"\b", conf):
                rep.fail("%s: option --*-%s is not in the engine's configure" % (os.path.relpath(mc, REPO), opt))
    rep.ok("mozconfig options scanned")

    print("7. ICU data file")
    have = sorted(os.path.basename(p) for p in glob.glob(os.path.join(uxp, "config/external/icu/data/icudt*l.dat")))
    bundler = read(os.path.join(REPO, "build/webos-oe/make-device-bundle.sh"))
    want = sorted(set(re.findall(r"icudt\d+l\.dat", bundler)))
    if want and have and want[0] not in have:
        rep.fail("make-device-bundle.sh expects %s; the tree ships %s" % (want, have))
    else:
        rep.ok("ICU data %s" % (have or "?"))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--uxp", default=os.path.join(REPO, "third_party", "uxp"))
    ap.add_argument("--write-baseline", action="store_true")
    a = ap.parse_args()
    uxp = os.path.abspath(a.uxp)
    if a.write_baseline:
        with open(BASELINE, "w") as f:
            json.dump(gather(uxp), f, indent=1, sort_keys=True)
            f.write("\n")
        print("baseline written: " + BASELINE)
        return 0
    print("Checking %s against what Jihad Browser uses" % uxp)
    rep = Report()
    check(uxp, rep)
    print("%d FAIL, %d WARN" % (rep.fails, rep.warns))
    return 1 if rep.fails else 0


if __name__ == "__main__":
    sys.exit(main())
