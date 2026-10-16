#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
"""Read a Gecko memory report (the gzipped JSON nsIMemoryInfoDumper writes; about:memory's data):
the detached and ghost windows with their sizes, and the largest paths. Usage:
memory-report-read.py report.json.gz [N]"""
import gzip, json, sys, collections
data = json.load(gzip.open(sys.argv[1], "rt", encoding="utf-8"))
n = int(sys.argv[2]) if len(sys.argv) > 2 else 25
reports = data["reports"]
KB = lambda b: "%.1f MB" % (b / 1048576) if b >= 1048576 else "%d KB" % (b // 1024)
def total(prefix):
    return sum(r["amount"] for r in reports if r["path"].startswith(prefix) and r["units"] == 0)
print("resident:", KB(next((r["amount"] for r in reports if r["path"] == "resident"), 0)),
      "| heap-allocated:", KB(next((r["amount"] for r in reports if r["path"] == "heap-allocated"), 0)),
      "| JS gc-heap committed:", KB(total("js-main-runtime-gc-heap-committed") or total("js-main-runtime/gc-heap")),
      "| explicit:", KB(next((r["amount"] for r in reports if r["path"] == "explicit"), 0) or total("explicit/")),
      "| heap-overhead:", KB(total("explicit/heap-overhead")),
      "| ghost-windows:", next((r["amount"] for r in reports if r["path"] == "ghost-windows"), "n/a"))
# windows: explicit/window-objects/top(<url>, id=N)/active|cached|detached/window(<url>)/...
wins = collections.defaultdict(int); kinds = collections.Counter()
for r in reports:
    p = r["path"]
    if p.startswith("explicit/window-objects/") and r["units"] == 0:
        parts = p.split("/")
        if len(parts) >= 5:
            top, state, win = parts[2], parts[3], parts[4]
            wins[(state, top, win)] += r["amount"]; kinds[state] += r["amount"]
print("\nwindow memory by state:", {k: KB(v) for k, v in kinds.items()})
print("\n| state | top | window | bytes |"); print("|---|---|---|---|")
for (state, top, win), b in sorted(wins.items(), key=lambda kv: -kv[1])[:n]:
    print(f"| {state} | {top[:60]} | {win[:70]} | {KB(b)} |")
print("\nlargest explicit paths:")
for r in sorted((r for r in reports if r["path"].startswith("explicit/") and r["units"] == 0), key=lambda r: -r["amount"])[:n]:
    print(f"  {KB(r['amount']):>10}  {r['path'][:150]}")
