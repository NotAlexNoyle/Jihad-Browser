#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# site-report.py -- the real sites' table (cavekit-flash.md R4 and R8, T-578): each `site-*` script's runs in a finger round
# read back into one row a site and shell. What the page offered and by which player comes from the script's own record:
# the daemon's log (show-log: the plugin instantiated, its window set, its draws a second), the page (the probe: the
# device's Flash in navigator.plugins, embeds and objects, Ruffle's elements and script, video elements, the frames it
# could not look into) and the kept picture.
#   site-report.py ROUND_DIR [--write]      prints the table; --write puts it in ROUND_DIR/sites.md
# Run with python3 -I.
import glob, json, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__)); SUITE = os.path.join(os.path.dirname(HERE), "suite")


def outcome(txt, start=""):
    probe = None
    for l in txt.splitlines():
        m = re.match(r"\s*probe what: (\{.*)", l)
        if m:
            try: probe = json.loads(m.group(1))
            except ValueError: probe = {"cut": m.group(1)[:120]}
    # the page's load is the runner's, before the script (daemon-start.sh waits 60 s for it): its line says when
    m = re.search(r"connected at \+(\d+) s, load done at \+(\d+) s", start or "")
    loaded = bool(txt) and (int(m.group(2)) - int(m.group(1)) < 60 if m else not re.search(r"CHECK FAIL \d+-wait-load", txt))
    inst = len(re.findall(r"InstantiatePluginInstance rv=0x0", txt))
    setwin = len(re.findall(r"NPP_SetWindow \d+x\d+", txt))
    draws = re.findall(r"palm draw: (\d+) in (\d+)ms \(([\d.]+) fps\)", txt)
    fatal = "FATAL" in txt
    fps = max((float(d[2]) for d in draws), default=0.0)
    p = probe or {}
    if not txt: what = "not run"
    elif not loaded: what = "**down or not loaded**"
    elif fatal: what = "**the daemon fell over**"
    elif inst and fps > 0: what = "the device's Flash, drawing (up to %.1f draws a second)" % fps
    elif inst: what = "the device's Flash instantiated, no draw seen"
    elif p.get("ruffle") or p.get("rufflejs"): what = "Ruffle in the page (%d of its elements)" % p.get("ruffle", 0)
    elif p.get("videos"): what = "HTML5 video (%d elements), no movie" % p.get("videos")
    elif p.get("embeds"): what = "an embed or object, no plugin instantiated"
    else: what = "no movie on the page"
    seen = "flash %s, embeds %s, ruffle %s/%s, video %s%s" % (p.get("flash", "?"), p.get("embeds", "?"), p.get("ruffle", "?"),
            "script" if p.get("rufflejs") else "no script", p.get("videos", "?"), ", frames " + ", ".join(p["xframes"]) if p.get("xframes") else "")
    return what, seen, (p.get("title") or "")


def main():
    rd = sys.argv[1]; rows = []
    rep = json.load(open(os.path.join(rd, "finger-report.json")))
    runs = {(r["shell"], r["id"]): r for r in rep["tests"]}
    for path in sorted(glob.glob(os.path.join(SUITE, "site-*.test"))):
        sid = os.path.basename(path)[:-5]; src = open(path).read()
        url = re.search(r"^page: (.*)$", src, re.M).group(1)
        item = re.search(r"^item: .*?\((.*)\): visited", src, re.M)
        cells = []
        for sh in ("enyo", "mochi", "mojo"):
            f = os.path.join(rd, "finger-%s-%s.txt" % (sh, sid)); txt = open(f, errors="replace").read() if os.path.exists(f) else ""
            r = runs.get((sh, sid), {}); when = str(r.get("rerun") or rep.get("date", ""))[:10]
            what, seen, title = outcome(txt, r.get("start", ""))
            cells.append("%s (%s; %s)" % (what, seen, when))
        host = re.sub(r"^https?://(web\.archive\.org/web/\d+/https?://)?", "", url).split("/")[0]
        rows.append("| %s | %s | %s |" % (host, item.group(1) if item else sid, " | ".join(cells)))
    out = ["| host | the page | Enyo | Mochi | Mojo |", "|---|---|---|---|---|"] + rows
    print("\n".join(out))
    if "--write" in sys.argv:
        open(os.path.join(rd, "sites.md"), "w").write("# The real sites (cavekit-flash.md R4, R8), by `site-report.py`\n\n" + "\n".join(out) + "\n")


if __name__ == "__main__":
    main()
