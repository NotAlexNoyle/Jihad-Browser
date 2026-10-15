#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
"""Two browse-round logs side by side, per site: load-done, worst tick, fps, refusals, errors,
RSS — and what changed (cavekit-modern-web.md R6). Usage: browse-round-compare.py A.log B.log"""
import re, sys

def visits(path):
    out = {}
    for line in open(path, encoding="utf-8", errors="replace"):
        if not line.startswith("@@ ") or line.startswith("@@ END"): continue
        parts = [p.strip() for p in line[3:].split("|")]
        name = parts[0]
        kv = dict(re.findall(r'(\S+?)=(\S+)', " ".join(parts[3:5])))
        perf = dict(re.findall(r'(first-paint|load|worst-tick|rss|load1)=(\S+)', parts[5])) if len(parts) > 5 else {}
        fps = re.search(r'fps=(avg [\d.]+ min [\d.]+)', parts[5]) if len(parts) > 5 else None
        out[name] = dict(load=(perf.get("load", "-") or "-").rstrip("s") or "-", tick=(perf.get("worst-tick", "-") or "-").rstrip("ms") or "-",
                         fps=fps.group(1) if fps else "-", blocked=kv.get("blocked", "0"), errors=kv.get("errors", "0"),
                         rss=(perf.get("rss", "-") or "-").rstrip("kB") or "-", shot=int(kv.get("shot", "0") or 0))
    return out

a, b = visits(sys.argv[1]), visits(sys.argv[2])
def mb(x): return "%d MB" % (int(x) // 1024) if x.isdigit() else "-"
print("| Visit | Load done A → B | Worst tick A → B | fps A → B | Refused A → B | Errors A → B | RSS B | Shot B |")
print("|---|---|---|---|---|---|---|---|")
better = worse = 0
for name in list(a.keys()) + [n for n in b if n not in a]:
    x, y = a.get(name, {}), b.get(name, {})
    ta, tb = x.get("tick", "-"), y.get("tick", "-")
    if ta.isdigit() and tb.isdigit():
        if int(tb) < int(ta) * 0.7: better += 1
        elif int(tb) > int(ta) * 1.3: worse += 1
    print(f"| {name} | {x.get('load','-')} → {y.get('load','-')} s | {ta} → {tb} ms | {x.get('fps','-')} → {y.get('fps','-')} | {x.get('blocked','-')} → {y.get('blocked','-')} | {x.get('errors','-')} → {y.get('errors','-')} | {mb(y.get('rss','-'))} | {y.get('shot',0)//1024} KB |")
print(f"\nWorst tick: better (−30% or more) on {better} sites, worse (+30% or more) on {worse}.")
