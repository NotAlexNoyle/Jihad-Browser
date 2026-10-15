#!/usr/bin/env python3
"""Turn a browse-round log (the "@@" lines and the error lines under them) into a markdown table
and a list of issues by the kit's thresholds (cavekit-modern-web.md R6)."""
import re, sys
log = open(sys.argv[1], encoding="utf-8", errors="replace").read().split("\n")
visits = []; cur = None
for line in log:
    if line.startswith("@@ END"): break
    if line.startswith("@@ "):
        parts = [p.strip() for p in line[3:].split("|")]
        name, url, took = parts[0], parts[1], parts[2]
        kv = dict(re.findall(r'(\S+?)=(\S+)', " ".join(parts[3:5])))
        perf = dict(re.findall(r'(first-paint|load|worst-tick|rss|load1)=(\S+)', parts[5]))
        fps = re.search(r'fps=(avg [\d.]+ min [\d.]+ n=\d+|-)', parts[5]); fps = fps.group(1) if fps else "-"
        last = parts[6].replace("last:", "").strip() if len(parts) > 6 else ""
        for k in ("first-paint", "load"): perf[k] = (perf.get(k, "-") or "-").rstrip("s") or "-"
        if perf.get("rss", "-") in ("kB", "", "-kB"): perf["rss"] = "-"
        cur = dict(name=name, url=url, took=took, shot=int(kv.get("shot", "0") or 0), loaddone=int(kv.get("load-done", 0)),
                   painted=int(kv.get("painted", 0)), blocked=int(kv.get("blocked", 0)), errors=int(kv.get("errors", 0)),
                   fp=perf.get("first-paint", "-"), load=perf.get("load", "-"), fps=fps, tick=perf.get("worst-tick", "-"),
                   rss=perf.get("rss", "-"), load1=perf.get("load1", "-"), last=last, errlines=[])
        visits.append(cur)
    elif cur is not None and line.startswith("   ") and not line.strip().startswith("/tmp used"):
        cur["errlines"].append(line.strip())
print("| # | Visit | Address | First paint | Load done | Paints | fps (avg/min) | Worst tick | RSS after | Refused | Errors | Shot |")
print("|---|---|---|---|---|---|---|---|---|---|---|---|")
prev_rss = None; issues = []
for i, v in enumerate(visits, 1):
    rss_kb = int(v["rss"].rstrip("kB")) if v["rss"] not in ("-", "kB", "") and v["rss"].rstrip("kB").isdigit() else None
    fp = v['fp'] + " s" if v['fp'] != "-" else "-"; ld = v['load'] + " s" if v['load'] != "-" else "none"
    print(f"| {i} | {v['name']} | `{v['url']}` | {fp} | {ld} | {v['painted']} | {v['fps']} | {v['tick'].rstrip('ms')} ms | {str(rss_kb//1024) + ' MB' if rss_kb else '-'} | {v['blocked']} | {v['errors']} | `shot-r2-{v['name']}` ({v['shot']//1024} KB) |")
    why = []
    if v["load"] == "-": why.append("no load-done for the address within the dwell")
    elif int(v["load"]) > 30: why.append(f"load-done after {v['load']} s")
    m = re.search(r'min ([\d.]+)', v["fps"])
    if m and float(m.group(1)) < 5: why.append(f"frame rate fell to {m.group(1)} fps")
    if v["tick"] not in ("-", "", "ms") and int(v["tick"].rstrip("ms") or 0) > 500: why.append(f"a {v['tick'].rstrip('ms')} ms tick (the main thread stalled)")
    if rss_kb and prev_rss and rss_kb - prev_rss > 40 * 1024: why.append(f"RSS +{(rss_kb - prev_rss)//1024} MB over the previous visit")
    if v["shot"] and v["shot"] < 60 * 1024: why.append("a near-uniform screenshot (blank or a dialog)")
    if v["errors"]: why.append(f"{v['errors']} page error line(s)")
    if why: issues.append((v["name"], why, v["errlines"][:4]))
    if rss_kb: prev_rss = rss_kb
good = [int(v["rss"].rstrip("kB")) for v in visits if v["rss"] not in ("-", "kB", "") and v["rss"].rstrip("kB").isdigit()]
first = good[0] if good else None; last = good[-1] if good else None
print(f"\nSession memory: first visit {first//1024 if first else '-'} MB, last visit {last//1024 if last else '-'} MB; {len(visits)} visits.\n")
print("## Issues by threshold\n")
for name, why, errs in issues:
    print(f"- **{name}**: " + "; ".join(why))
    for e in errs: print(f"  - `{e[:200]}`")
