#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# site-round.py -- real sites that carry movies, in each shell (cavekit-flash.md R4, T-574): by host, what the page has (an embed or
# object, Ruffle), whether a player runs for it and whether it paints, and a picture to judge by eye. A site that is down is recorded
# as down, which is no verdict on Flash; one that forces Ruffle is recorded as that.
#   site-round.py DIR [--shells enyo,mochi,mojo] [--only host,host]   writes DIR/flash-sites.md and DIR/sites/*.jpg
# Run with python3 -I.
import argparse, os, re, subprocess, sys, tempfile, time

HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(HERE))))
DEVSH = os.path.join(ROOT, "build", "webos-oe", "dev-sh.sh"); F = os.path.join(ROOT, "tools", "device", "finger")
# (host, url, the page's own Play button on the screen if it has one)
SITES = [("www.ferryhalim.com", "http://www.ferryhalim.com/orisinal/g3/bells.htm", None),
         ("www.bubblebox.com", "https://www.bubblebox.com/play/puzzle/1136.htm", (387, 528)),
         ("www.kbhgames.com", "https://www.kbhgames.com/game/bloxorz", (419, 539)),
         ("www.primarygames.com", "https://www.primarygames.com/puzzles/strategy/bloxorz/", (227, 400)),
         ("www.silvergames.com", "https://www.silvergames.com/en/bloxorz", (365, 520)),
         ("www.crazygames.com", "https://www.crazygames.com/game/bloxorz", None),
         ("www.flasharch.com", "https://www.flasharch.com/", None)]
BODY = r'''
sleep 30
PLAYTAP
r=$(fl_url 2>/dev/null); echo "URL $(grep -a 'load done uri=\|loaderr failed=[1-9]' $FL_LOG | tail -1 | cut -c1-140)"
for q in embed object ruffle-player ruffle-embed ruffle-object; do l=$(fl_rect "sel:$q" 2>/dev/null); echo "EL $q ${l:-none}"; done
echo "PLUGIN $(fl_plugin_pid)"
grep -a 'flashRect add' $FL_LOG | tail -1 | cut -c1-80 | sed 's/^/RECT /'
luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/fl-site.png"}' >/dev/null 2>&1; usleep 800000
'''


def run(cmd, t):
    return subprocess.run(cmd, capture_output=True, text=True, errors="replace", timeout=t)


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("dir"); ap.add_argument("--shells", default="enyo,mochi,mojo"); ap.add_argument("--only", default=""); o = ap.parse_args()
    from PIL import Image
    os.makedirs(os.path.join(o.dir, "sites"), exist_ok=True)
    only = set(x for x in o.only.split(",") if x); rows = []; tmp = tempfile.mkdtemp()
    for host, url, play in SITES:
        if only and host not in only: continue
        for shell in o.shells.split(","):
            run([DEVSH, "-t", "150", os.path.join(F, "daemon-start.sh"), shell, url], 170)
            p = os.path.join(tmp, "b.sh"); open(p, "w").write(open(os.path.join(F, "fingerlib.sh")).read() + "\nfl_init %s\n" % shell + BODY.replace("PLAYTAP", "/tmp/finger tap %d %d; sleep 30" % play if play else ""))
            out = run([DEVSH, "-t", "70", p], 90).stdout
            png = os.path.join(tmp, "s.png"); raw = subprocess.run(["novacom", "get", "file:///tmp/fl-site.png"], capture_output=True).stdout
            open(png, "wb").write(raw); subprocess.run(["novacom", "run", "file://bin/rm", "--", "-f", "/tmp/fl-site.png"], stdin=subprocess.DEVNULL, capture_output=True)
            name = "%s-%s.jpg" % (host, shell)
            try: Image.open(png).convert("RGB").rotate(-90, expand=True).resize((384, 512)).save(os.path.join(o.dir, "sites", name), quality=60)
            except Exception: name = ""
            run([DEVSH, "-t", "90", os.path.join(F, "daemon-stop.sh"), shell], 110)
            els = dict(re.findall(r"^EL (\S+) (.*)$", out, re.M))
            has = lambda k: els.get(k, "none") not in ("none", "") and not els.get(k, "").startswith("0 0 0 0")
            ruffle = any(has(k) for k in ("ruffle-player", "ruffle-embed", "ruffle-object")); movie = has("embed") or has("object")
            plugin = bool(re.search(r"^PLUGIN \d+", out, re.M)); down = "loaderr" in out and "load done" not in out
            what = "down" if down else "Ruffle" if ruffle else "Flash runs" if plugin else "a movie, no player" if movie else "no movie on the page"
            rows.append((host, shell, what, re.sub(r".*(load done uri=|loaderr )", "", (re.search(r"^URL (.*)$", out, re.M) or [None, ""])[1])[:70], name))
            print(host, shell, what, flush=True)
    with open(os.path.join(o.dir, "flash-sites.md"), "w") as f:
        f.write("# Real sites that carry movies (cavekit-flash.md R4), %s\n\n| Host | Shell | Outcome | Reached | Picture |\n|---|---|---|---|---|\n" % time.strftime("%Y-%m-%d"))
        for r in rows: f.write("| %s | %s | %s | %s | sites/%s |\n" % r)


if __name__ == "__main__":
    main()
