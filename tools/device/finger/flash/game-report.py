#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# game-report.py DIR -- the minute's verdicts (cavekit-flash.md R6) from game-round.py's DIR/<shell>.json, as one markdown table on stdout.
# A game passes in a shell when: its first screen matched its reference (griddiff under 8), every five-second window under input showed
# pictures the no-input control never did (half the reads or more), the content rate is at least 90 % of the lower of its authored rate
# and the stock browser's in the same minute (R3; where the stock browser could not play it, drawn and taking the input in at least one
# window, of its authored rate and this browser's software path's in the same minute, DIR/soft.json; where neither was measured, of its
# authored rate, and the table says so), the shown-frame spacing's p90 is at most twice the authored period or within 10 % of the bar's
# browser's, the movie is drawn
# at the end, and plugin-container, the card and the daemon lived through it. A game the stock browser fails the same way is a limit, with
# the reason from tools/host/flash-games.json. HOST side. Run with python3 -I.
import json, os, sys

HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(HERE))))
M = {t["id"]: t for t in json.load(open(os.path.join(ROOT, "tools", "host", "flash-corpus.json")))["titles"]}
G = json.load(open(os.path.join(ROOT, "tools", "host", "flash-games.json")))


def bar_of(id_, st, sf):
    # which minute the game is held to: the stock browser's where it played the game, else the software path's, else none
    if st and st.get("windows") and st.get("reacted") and st.get("colours", 0) >= 8: return "stock", st
    if sf and sf.get("windows"): return "software path", sf
    return "authored", None


def verdict(id_, r, st, sf=None):
    t = M[id_]; fps = t.get("fps") or 24.0; why = []
    if r is None: return "not run", why
    if r["ref"] < 0 or r["ref"] >= 8: why.append("first screen %.1f off its reference" % r["ref"])
    if r["windows"] == 0 or r["reacted"] < r["windows"]: why.append("reacted in %d of %d windows" % (r["reacted"], r["windows"]))
    _, b = bar_of(id_, st, sf)
    bar = 0.9 * min(fps, b["content"]) if b else 0.9 * fps
    if r["content"] < bar: why.append("content %.1f/s under the bar %.1f" % (r["content"], bar))
    p90_ok = r["p90"] <= 2 * 1000.0 / fps or (b and b.get("p90") and r["p90"] <= 1.1 * b["p90"])
    if not p90_ok: why.append("spacing p90 %d ms" % r["p90"])
    if r.get("colours", 99) < 8: why.append("blank at the end")
    if not r["same_player"]: why.append("plugin-container changed or gone")
    if not r.get("card", True): why.append("the card closed")
    if r["faults"] > 0: why.append("%d faults in the daemon's log" % r["faults"])
    if r.get("orphan"): why.append("the player outlived its card")
    if not why: return "pass", why
    return ("limit" if G.get(id_, {}).get("limit") else "fail"), why


def main():
    d = sys.argv[1]; R = {}
    for sh in ("stock", "soft", "enyo", "mochi", "mojo"):
        p = os.path.join(d, sh + ".json")
        if os.path.exists(p): R[sh] = json.load(open(p))
    shells = [s for s in ("enyo", "mochi", "mojo") if s in R]
    print("| game | input | authored | stock in play | software path | bar | " + " | ".join(shells) + " |")
    print("|---|---|---|---|---|---|" + "---|" * len(shells))
    tally = {}
    for id_ in sorted(k for k in G if k != "_"):
        st = R.get("stock", {}).get(id_); sf = R.get("soft", {}).get(id_); cells = []
        for sh in shells:
            r = R[sh].get(id_); v, why = verdict(id_, r, st, sf); tally[v] = tally.get(v, 0) + 1
            if r is None: cells.append("not run"); continue
            cells.append("**%s** %d/%d, %.1f/s, p90 %d%s" % (v, r["reacted"], r["windows"], r["content"], r["p90"], (": " + "; ".join(why)) if why else ""))
        one = lambda x: ("%d/%d, %.1f/s, p90 %d" % (x["reacted"], x["windows"], x["content"], x["p90"])) if x and x.get("windows") else "-"
        sp = one(st) + " | " + one(sf) + " | " + bar_of(id_, st, sf)[0]
        lim = G[id_].get("limit")
        print("| %s%s | %s | %g fps | %s | %s |" % (id_, (" (limit: %s)" % lim) if lim else "", G[id_]["input"], M[id_].get("fps") or 0, sp, " | ".join(cells)))
    print("\n" + ", ".join("%s %d" % kv for kv in sorted(tally.items())))


if __name__ == "__main__":
    main()
