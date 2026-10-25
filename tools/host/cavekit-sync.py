#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
"""Keep Cavekit 3.1's runtime files in step with the build site.

The build site (context/plans/build-site.md) is the plan the people and the kits agree on; the
runtime reads `.cavekit/tasks.json` for the task graph and `.cavekit/task-status.json` for each
task's state. Tiers added to the site by hand (84 … 90 in October 2026) never reached tasks.json,
so the loop could not see them (143 of 327 tasks on 2026-10-25). This tool regenerates tasks.json
from every tier table of the site, keeps the registry's statuses where the site has no Status
column, takes the site's Status where it has one, and prints what changed.

  tools/host/cavekit-sync.py            report only
  tools/host/cavekit-sync.py --write    write tasks.json and task-status.json
"""
import json, re, sys, datetime, os

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SITE = os.path.join(ROOT, "context/plans/build-site.md")
TASKS = os.path.join(ROOT, ".cavekit/tasks.json")
STATUS = os.path.join(ROOT, ".cavekit/task-status.json")

COLS = {"id": ("ID", "Task"), "title": ("Title", "Task"), "kit": ("Kit", "Cavekit"), "reqs": ("Reqs", "Requirement"),
        "depends": ("Depends", "blockedBy"), "status": ("Status",), "reason": ("Reason",), "device": ("Device",)}

def cell(row, key):
    # The site's tables grew over four months: "| Task | Title | Cavekit | Requirement | blockedBy | Device |
    # Effort |" (tiers 0–83, some with a Reason column), and "| ID | Task | Kit | Reqs | Depends | Parallel |
    # Size | Status |" (tier 90). In the old form "Task" is the id and "Title" the text; in the new form "ID"
    # is the id and "Task" the text.
    for name in COLS[key]:
        if name in row:
            if key == "title" and name == "Task" and "Title" in row: continue
            if key == "id" and name == "Task" and "ID" in row: continue
            return row[name]
    return ""

def site_tasks():
    tier, header = None, None
    out = []
    for line in open(SITE, encoding="utf-8"):
        m = re.match(r"^## Tier (\d+)", line)
        if m: tier = int(m.group(1)); header = None; continue
        if re.match(r"^\| (ID|Task) \|", line):
            header = [c.strip() for c in line.strip().strip("|").split("|")]; continue
        m = re.match(r"^\| (T-\d+) \|", line)
        if not m or header is None: continue
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) < len(header): cells += [""] * (len(header) - len(cells))
        row = dict(zip(header, cells))
        deps = re.findall(r"T-\d+", cell(row, "depends"))
        reqs = re.findall(r"R\d+[a-z]?", cell(row, "reqs"))
        kit = (re.findall(r"cavekit-[a-z0-9-]+\.md", cell(row, "kit")) or [""])[0]
        title = cell(row, "title").strip("*").strip()
        st = cell(row, "status"); reason = cell(row, "reason")
        status = None
        if st:
            u = st.upper()
            # The site's own words over four months: DONE / BUILT / SHIPPED / WITHDRAWN close a task;
            # PARTIAL and IN PROGRESS are under way; BLOCKED and "owner" (waiting on the owner) wait.
            status = ("complete" if re.match(r"(DONE|BUILT|SHIPPED|WITHDRAWN|MET|VERIFIED)", u)
                      else "in_progress" if re.match(r"(IN PROGRESS|PARTIAL|STARTED)", u)
                      else "blocked" if ("BLOCK" in u or u.startswith("OWNER") or u.startswith("DEFERRED")) else "pending")
        elif reason.strip():
            status = "blocked"
        elif re.search(r"\bBLOCKED\b", title): status = "blocked"
        # A tier before the Status column (0–83) was worked and closed before 2026-10-14; the registry
        # that held those states was lost on 2026-10-25 and they are taken as complete unless the row
        # says otherwise. The tiers since then carry their state in the table.
        out.append({"id": m.group(1), "title": title[:200], "tier": tier,
                    "depends_on": deps, "depth": "standard", "kit": kit, "requirement_ids": reqs,
                    "device": bool(re.search(r"device|finger|TouchPad", title + cell(row, "device"), re.I)),
                    "blocked_external": status == "blocked" and bool(reason.strip()),
                    "_site_status": status, "_old_tier": not st})
    return out

def main():
    write = "--write" in sys.argv
    site = site_tasks()
    old_tasks = json.load(open(TASKS)) if os.path.exists(TASKS) else []
    old_ids = {t["id"] for t in old_tasks}
    reg = json.load(open(STATUS)) if os.path.exists(STATUS) else {"tasks": []}
    reg_by = {e["id"]: e for e in reg.get("tasks", [])}
    new_tasks, new_status, changed = [], [], []
    for t in site:
        st = t.pop("_site_status"); old_tier = t.pop("_old_tier")
        new_tasks.append(t)
        prev = reg_by.get(t["id"])
        status = st or (prev["status"] if prev else ("complete" if old_tier else "pending"))
        if prev and prev["status"] != status: changed.append((t["id"], prev["status"], status))
        e = dict(prev) if prev else {"id": t["id"], "title": t["title"], "tier": t["tier"], "depends_on": t["depends_on"], "depth": "standard", "repo": None}
        e.update({"title": t["title"], "tier": t["tier"], "depends_on": t["depends_on"], "status": status})
        new_status.append(e)
    counts = {}
    for e in new_status: counts[e["status"]] = counts.get(e["status"], 0) + 1
    print("site tasks: %d (tasks.json had %d; %d were missing); statuses: %s" % (len(site), len(old_tasks), len([t for t in site if t["id"] not in old_ids]), counts))
    if changed: print("status changes from the site's Status column: %s" % changed[:30])
    missing_kit = [t["id"] for t in new_tasks if not t["kit"]]
    if missing_kit: print("rows naming no kit: %s" % missing_kit[:20])
    if write:
        json.dump(new_tasks, open(TASKS, "w"), indent=2)
        json.dump({"tasks": new_status, "updated_at": datetime.datetime.now(datetime.timezone.utc).isoformat()}, open(STATUS, "w"), indent=2)
        print("written: %s (%d), %s (%d)" % (TASKS, len(new_tasks), STATUS, len(new_status)))

if __name__ == "__main__":
    main()
