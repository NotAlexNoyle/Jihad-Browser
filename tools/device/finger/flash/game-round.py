#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# game-round.py -- every game played for a minute, in a shell (cavekit-flash.md R6, T-576): its first screen against its reference, started
# by the finger as tools/host/flash-games.json says, then its input for 60 s: five-second windows, each read against the pictures before
# the game was touched, the content rate and the shown frames' spacing over the minute, whether the player and the card lived, the
# daemon's faults. HOST side, one device session a game; flash-serve.py running.
#   game-round.py stock|enyo|mochi|mojo DIR [--only id,id] [--seconds 60] [--make-refs] [--steps] [--winshots]
#     DIR/<shell>-<id>-{first,play}.jpg (s<k> after each start step, w<k> after each window), DIR/<shell>.json
# Run with python3 -I.
import argparse, json, os, re, subprocess, tempfile, time
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(HERE))))
DEVSH = os.path.join(ROOT, "build", "webos-oe", "dev-sh.sh"); F = os.path.join(ROOT, "tools", "device", "finger")
MANIFEST = os.path.join(ROOT, "tools", "host", "flash-corpus.json"); GAMES = os.path.join(ROOT, "tools", "host", "flash-games.json")
SHELLS = {"stock": ("net.riverstonerelay.stockbench", 82), "enyo": ("net.riverstonerelay.jihad-browser", 82), "mochi": ("net.riverstonerelay.jihad-browser-mochi", 78), "mojo": ("net.riverstonerelay.jihad-browser-mojo", 73)}
HOST = "192.168.1.245:8097"; REFS = os.path.expanduser("~/.cache/jihad-flash/refs")
ARROW = {"up": "702 757 702 700", "down": "702 757 702 810", "left": "702 757 640 757", "right": "702 757 760 757"}


def keypos(c):
    # as fl_kb_key: the system keyboard's letters
    for letters, x0, y in (("qwertyuiop", 34, 813), ("asdfghjkl", 69, 873), ("zxcvbnm", 104, 933)):
        if c in letters: return f"{x0 + 70 * letters.index(c)} {y}"
    return "384 993"


def body(t, g, app, top, seconds, ref, steps=False, winshots=False):
    sw, sh = t.get("width") or 640, t.get("height") or 480; k = min(1.0, 768.0 / sw); w, h = round(sw * k), round(sh * k)
    # under the lock the keyboard is up: read above it, its keys light up as they are tapped
    seq = g.get("seq"); needs_lock = g["input"] != "tap" or bool(seq and any(a[0] != "tap" for a in seq))
    bottom = 730 if needs_lock or any(f[0] == "dtap" for f in g.get("start", [])) else top + min(h, 700)
    R = "0,%d,%d,%d" % (top, w, min(top + h, bottom)); R0 = "0,%d,%d,%d" % (top, w, top + min(h, 700)); load = 14 + min(30, t["size"] // 400000) + g.get("extra", 0)
    pt = lambda f: "%d %d" % (round(f[0] * w), top + round(f[1] * h))
    s = [f"""
close_all() {{ for pid in $(luna-send -n 1 palm://com.palm.applicationManager/running '{{}}' 2>/dev/null | tr ',' '\\n' | grep -A1 '"{app}"' | grep processid | sed 's/.*"processid": *"\\([^"]*\\)".*/\\1/'); do luna-send -n 1 palm://com.palm.applicationManager/close '{{"processId":"'"$pid"'"}}' >/dev/null 2>&1; done; }}
shot() {{ luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{{"file":"/tmp/fl-'$1'.png"}}' >/dev/null 2>&1; usleep 700000; }}
J=$(echo {app} | sed 's/net.riverstonerelay.jihad-browser/jihad/'); V=enyo; case {app} in *mochi) V=mochi;; *mojo) V=mojo;; esac
# the faults from this game on: the job daemon's daemon.log (rotated to daemon.log.old at some 2 MB, so its inode and size are kept) and
# a hand-started daemon's daemon-gpu.log; 'FAULT' is the plugin child's signal handler, SIGSEGV and FATAL the daemon's
D=/var/palm/jihad/$V; I0=$(ls -i $D/daemon.log 2>/dev/null | awk '{{print $1}}'); B0=$(wc -c < $D/daemon.log 2>/dev/null); G0=$(wc -c < $D/daemon-gpu.log 2>/dev/null)
newlog() {{ if [ "$(ls -i $D/daemon.log 2>/dev/null | awk '{{print $1}}')" = "$I0" ]; then tail -c +$((${{B0:-0}} + 1)) $D/daemon.log; else tail -c +$((${{B0:-0}} + 1)) $D/daemon.log.old 2>/dev/null; cat $D/daemon.log; fi; tail -c +$((${{G0:-0}} + 1)) $D/daemon-gpu.log 2>/dev/null; }}
close_all; usleep 500000
# a player left from an earlier game would be read as this game's, and it takes the CPU: named and stopped
for p in $(pgrep -f plugin-container); do echo "STALE $p"; kill -9 $p 2>/dev/null; done
luna-send -n 1 palm://com.palm.applicationManager/launch '{{"id":"{app}","params":{{"target":"http://{HOST}/title/{t['id']}"}}}}' >/dev/null
sleep {load}
"""]
    # a game's first screen comes when it comes (a sponsor's logos, and a tap on one leaves the page): by its signature
    if ref: s.append(f"""i=0; while [ $i -lt 75 ]; do d=$(/tmp/fbstream /tmp/fl-sig -Q {R0} -M {ref} 2>/dev/null | sed -n 's/.* griddiff \\([0-9.]*\\).*/\\1/p'); awk -v d="${{d:-99}}" 'BEGIN {{ exit !(d < 8) }}' && break; sleep 1; i=$((i+1)); done; echo "REF ${{d:-99}} $i"; sleep 2\n""")
    else: s.append(f"sleep {g.get('ref_wait', 30)}\n")
    s.append(f"""shot first\necho "SIG $(/tmp/fbstream /tmp/fl-sig -Q {R0} -S 2>/dev/null | sed -n 's/.* grid \\([0-9a-f]*\\).*/\\1/p')"\n""")
    if not ref: s.append("close_all\n"); return "".join(s), w, h
    for k, f in enumerate(g.get("start", [])):
        if f[0] == "wait": s.append(f"sleep {f[1]}\n")
        elif f[0] == "dtap": s.append(f"/tmp/finger dtap {pt(f[1:3])}; sleep {f[3]}\n")
        elif f[0] == "type": s.append("".join(f"/tmp/finger tap {keypos(c)}; usleep 200000\n" for c in f[1]) + f"sleep {f[2]}\n")
        else: s.append(f"/tmp/finger tap {pt(f)}{' ' + str(f[3]) if len(f) > 3 else ''}; sleep {f[2]}\n")
        if steps: s.append(f"shot s{k}\n")
    # the pictures before it is touched any further: what each window is read against
    # a press held and a drag are the movie's only under the gesture lock, as a person has it: a double tap, and the keyboard comes with it
    locked = any(f[0] == "dtap" for f in g.get("start", []))
    # the stock browser has no lock: a double tap zooms its page, and the round would read a zoomed page
    if needs_lock and not locked and "stockbench" not in app:
        s.append(f"/tmp/finger dtap {pt(g.get('lock_at', [0.5, 0.5]))}; sleep 6\n")
    s.append("""PID=$(pgrep -n -f plugin-container); echo "PLUGIN0 $PID"\n""")
    def act(a):
        # a game's own mixed sequence (flash-games.json "seq"): ["tap",x,y], ["drag",x1,y1,x2,y2(,ms)], ["hold",x,y,ms], ["key",name]
        if a[0] == "tap": return f"/tmp/finger tap {pt(a[1:3])}"
        if a[0] == "drag": return f"/tmp/finger drag {pt(a[1:3])} {pt(a[3:5])} {a[5] if len(a) > 5 else 500}"
        if a[0] == "hold": return f"/tmp/finger hold {pt(a[1:3])} {a[3]}"
        return f"/tmp/finger drag {ARROW[a[1]]} 500" if a[1] in ARROW else f"/tmp/finger tap {keypos(a[1])}"
    def one(i):
        if seq: return act(seq[i % len(seq)])
        if g["input"] == "tap": return f"/tmp/finger tap {pt(g['points'][i % len(g['points'])])}"
        if g["input"] == "hold": return f"/tmp/finger hold {pt(g['at'])} {g['ms']}"
        if g["input"] == "drag":
            a, b = (g["from"], g["to"]) if i % 2 == 0 else (g["to"], g["from"])
            return f"/tmp/finger drag {pt(a)} {pt(b)} 500"
        key = g["keys"][i % len(g["keys"])]
        if key in ARROW: return f"/tmp/finger drag {ARROW[key]} 500"
        return f"/tmp/finger tap {keypos(key)}"
    win = 5; n = max(1, seconds // win)
    def block(w):
        return "; ".join(f"{one(w * 4 + i)}; usleep 900000" for i in range(4))
    # each window under input is read against a control of the same length just before it, with nothing touched: what the game
    # does on its own in that stretch (a background drifting, a timer) is in the control, and only the input's work is new
    # a tap that reaches an ad's link takes the card off the game (2026-11-07: Learn to Fly 3's sponsor, then a YouTube page
    # that starved the device): after each window the shell's last top-level address is read, and a card that left its
    # game is closed at once and the rest skipped (the stock browser keeps no such file)
    away = "" if "stockbench" in app else f"""if [ -f $D/last-load ] && ! sed -n 2p $D/last-load | grep -q '/title/{t['id']}'; then echo "AWAY $(sed -n 2p $D/last-load | cut -c1-150)"; AWAY=1; close_all; fi\n"""
    s.append("AWAY=\n")
    for w_i in range(n):
        s.append(f"""if [ -z "$AWAY" ]; then
/tmp/fbstream /tmp/fl-ctl -C {R} -s {win} -P 2000 >/dev/null 2>&1
/tmp/fbstream /tmp/fl-win -C {R} -s {win} -P 4000 -B /tmp/fl-ctl > /tmp/fl-win.out 2>&1 &
{block(w_i)}
wait $!; {"shot w%d; " % w_i if winshots else ""}echo "WIN $(sed -n 's/.* new \\([0-9]*\\) of \\([0-9]*\\).*/\\1 \\2/p' /tmp/fl-win.out) $(sed -n 's/.*changes [0-9]* (\\([0-9.]*\\)\\/s).*/\\1/p' /tmp/fl-win.out) $(sed -n 's/.* p90 \\([0-9]*\\) .*/\\1/p' /tmp/fl-win.out)"
{away}fi
""")
    s.append(f"""
shot play
echo "SHOWN $(/tmp/fbstream /tmp/fl-sig -Q {R} 2>/dev/null | sed -n 's/.* colours \\([0-9]*\\).*/\\1/p')"
echo "CARD $(luna-send -n 1 palm://com.palm.applicationManager/running '{{}}' 2>/dev/null | grep -c '"{app}"')"
PID2=$(pgrep -n -f plugin-container); echo "PLUGIN1 $PID2"; [ -n "$PID2" ] && echo "RSS $(grep VmRSS /proc/$PID2/status | tr -s ' ' | cut -d' ' -f2)"
newlog | grep -a -E 'FAULT|SIGSEGV|FATAL' | head -5 | sed 's/^/FAULTLINE /' | cut -c1-200
echo "FAULTS $(newlog | grep -a -c -E 'FAULT sig|SIGSEGV|FATAL')"
# the player's own account of the input: events given to it, and those it took (handled=1)
echo "EVENTS $(newlog | grep -a -c 'palm event') $(newlog | grep -a -c 'palm event.*handled=1')"
close_all; rm -f /tmp/fl-ctl /tmp/fl-win /tmp/fl-win.out /tmp/fl-sig
# the player goes with its card (cavekit-flash.md R10): the engine keeps it 30 s for the next movie
# (dom.ipc.plugins.unloadTimeoutSecs), so one still there 35 s after the card closed is named
i=0; while [ $i -lt 35 ] && pgrep -f plugin-container >/dev/null; do sleep 1; i=$((i+1)); done; for p in $(pgrep -f plugin-container); do echo "ORPHAN $p"; done
""")
    return "".join(s), w, h


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("shell", choices=list(SHELLS)); ap.add_argument("dir"); ap.add_argument("--only", default="")
    ap.add_argument("--seconds", type=int, default=60); ap.add_argument("--make-refs", action="store_true"); ap.add_argument("--steps", action="store_true")
    ap.add_argument("--winshots", action="store_true")
    # --label soft: the results and pictures under another name, for the shell's daemon on its software path (R3's bar where the
    # stock browser cannot play a game: daemon-start.sh enyo URL JIHAD_GPU_COMPOSITOR=0 first, daemon-stop.sh after)
    ap.add_argument("--label", default=""); o = ap.parse_args(); name = o.label or o.shell
    app, top = SHELLS[o.shell]; os.makedirs(o.dir, exist_ok=True); os.makedirs(REFS, exist_ok=True)
    m = {t["id"]: t for t in json.load(open(MANIFEST))["titles"]}; g = json.load(open(GAMES)); only = set(x for x in o.only.split(",") if x)
    out_path = os.path.join(o.dir, name + ".json"); results = json.load(open(out_path)) if os.path.exists(out_path) else {}; tmp = tempfile.mkdtemp()
    for id_ in sorted(k for k in g if k != "_"):
        if only and id_ not in only: continue
        t = m[id_]; rp = os.path.join(REFS, id_ + ".first"); ref = open(rp).read().strip() if os.path.exists(rp) and not o.make_refs else ""
        text, w, h = body(t, g[id_], app, top, o.seconds, ref, o.steps, o.winshots)
        p = os.path.join(tmp, "s.sh"); open(p, "w").write(text); t0 = time.time()
        for attempt in (1, 2):
            out = subprocess.run([DEVSH, "-t", str(200 + o.seconds * 2 + g[id_].get("extra", 0)), p], capture_output=True, text=True, errors="replace").stdout
            if re.search(r"^FAULTS ", out, re.M) or ref == "": break
            # novacom now and then loses the device for a while with it up: wait for it, close what the lost run left, play it again
            open(os.path.join(o.dir, f"{name}-{id_}-incomplete{attempt}.out"), "w").write(out)
            print(id_, "incomplete (the device lost?): waiting and playing it again", flush=True)
            for _ in range(60):
                if subprocess.run(["novacom", "-l"], capture_output=True, text=True).stdout.strip(): break
                time.sleep(5)
            time.sleep(10)
            # a USB drop brings the system's "Connected / USB Drive" alert up in the upper right: closed first (the owner's rule)
            subprocess.run(["novacom", "run", "file:///tmp/finger", "--", "tap", "583", "232"], stdin=subprocess.DEVNULL, capture_output=True)
        for shot in ["first", "play"] + ([f"s{k}" for k in range(len(g[id_].get("start", [])))] if o.steps else []) + ([f"w{k}" for k in range(max(1, o.seconds // 5))] if o.winshots else []):
            raw = subprocess.run(["novacom", "get", f"file:///tmp/fl-{shot}.png"], capture_output=True).stdout
            q = os.path.join(tmp, "x.png"); open(q, "wb").write(raw); subprocess.run(["novacom", "run", "file://bin/rm", "--", "-f", f"/tmp/fl-{shot}.png"], stdin=subprocess.DEVNULL, capture_output=True)
            try:
                im = Image.open(q).convert("RGB").rotate(-90, expand=True).crop((0, top, min(768, w), top + min(h, 700))); im.thumbnail((256, 256)); im.save(os.path.join(o.dir, f"{name}-{id_}-{shot}.jpg"), quality=60)
            except Exception: pass
        sig = (re.search(r"^SIG (\S+)", out, re.M) or [None, ""])[1]
        if o.make_refs and sig: open(rp, "w").write(sig + "\n")
        wins = [(int(a), int(b), float(c), int(d) if d else 0) for a, b, c, d in re.findall(r"^WIN (\d+) (\d+) ([\d.]+) ?(\d*)", out, re.M)]
        # the minute's rate and spacing are the windows under input: the median window
        med = lambda v: sorted(v)[len(v) // 2] if v else 0
        p0 = (re.search(r"^PLUGIN0 (\d+)", out, re.M) or [None, ""])[1]; p1 = (re.search(r"^PLUGIN1 (\d+)", out, re.M) or [None, ""])[1]
        r = {"ref": float((re.search(r"^REF ([\d.]+)", out, re.M) or [None, -1])[1]), "windows": len(wins), "reacted": sum(1 for a, b, c, d in wins if b and a * 2 >= b), "changed": sum(1 for a, b, c, d in wins if c >= 1),
             "content": med([c for a, b, c, d in wins]), "p90": med([d for a, b, c, d in wins if d]), "rates": [c for a, b, c, d in wins],
             "same_player": bool(p0) and p0 == p1, "rss": int((re.search(r"^RSS (\d+)", out, re.M) or [None, 0])[1]), "faults": int((re.search(r"^FAULTS (-?\d+)", out, re.M) or [None, -1])[1]), "colours": int((re.search(r"^SHOWN (\d+)", out, re.M) or [None, 0])[1]), "card": int((re.search(r"^CARD (\d+)", out, re.M) or [None, 0])[1]) > 0,
             "stale": re.findall(r"^STALE (\d+)", out, re.M), "orphan": re.findall(r"^ORPHAN (\d+)", out, re.M), "fault_lines": re.findall(r"^FAULTLINE (.*)", out, re.M), "away": (re.search(r"^AWAY (.*)", out, re.M) or [None, ""])[1], "events": [int(x) for x in ((re.search(r"^EVENTS (\d+) (\d+)", out, re.M) or None) and re.search(r"^EVENTS (\d+) (\d+)", out, re.M).groups() or (0, 0))], "secs": round(time.time() - t0)}
        results[id_] = r; json.dump(results, open(out_path, "w"), indent=1)
        print("%-24s ref %5.1f reacted %2d/%2d changed %2d content %5.1f p90 %4d player %-5s rss %6d faults %2d (%d s) ev %d/%d%s%s" % (id_, r["ref"], r["reacted"], r["windows"], r["changed"], r["content"], r["p90"], r["same_player"], r["rss"], r["faults"], r["secs"], r["events"][1], r["events"][0], " STALE" if r["stale"] else "", " ORPHAN" if r["orphan"] else ""), flush=True)


if __name__ == "__main__":
    main()
