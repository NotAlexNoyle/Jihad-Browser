#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# The README's "Where it stands" section, counted from the records: each kit's acceptance criteria
# (context/kits/cavekit-*.md, "- [x]" met, "- [ ]" open) grouped into the browser's areas, the
# owner's finger tests (docs/FINGER-TESTS.md), and the road to the release drawn from them.
#
#   tools/readme-progress.py          rewrite the section between the README's progress markers
#   tools/readme-progress.py --check  exit 1 when the README's section is not what the records say
#   tools/readme-progress.py --print  print the section only
#
# Nothing in the section is typed by hand: change a kit (a criterion met) or the finger-test list
# and run this again.
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
README = os.path.join(ROOT, "README.md")
START, END = "<!-- progress:start -->", "<!-- progress:end -->"

# (the area as a reader knows it, its kits, what it covers)
AREAS = [
    ("Engine and page drawing", ["engine-embedding", "offscreen-rendering", "ipc-contract", "gre-widgets",
                                 "navigation-events"], "Goanna inside the webOS card, frames to the card, the card's messages"),
    ("Drawing on the GPU", ["gpu-rendering"], "the Adreno 220 composes the page; WebGL; pan and zoom speed"),
    ("The modern web", ["modern-web"], "today's sites: TLS, missing features filled in, video"),
    ("The three interfaces", ["ui-shell", "mochi-ui", "mojo-ui", "chrome-look"], "Enyo, Mochi and Mojo"),
    ("Touch and keyboard", ["input-bridging"], "taps, gestures, the on-screen keyboard"),
    ("Settings and services", ["preferences-ui", "browser-services"], "the settings panes, downloads, history, printing"),
    ("Add-ons and Personas", ["addons-extensions", "addons-site", "persona-formats", "personas-ui"],
     "XPI add-ons, plugins, Personas, the add-on site"),
    ("Privacy and security", ["content-blocking", "proxies-certs"], "the ad and tracker blocker, proxies, certificates"),
    ("Building and updating", ["desktop-build", "device-build", "engine-update", "licensing-branding"],
     "one repository builds every package; engine updates; licences"),
    ("Tabs", ["tabbed-browsing"], "groundwork only, held by the owner's decision"),
]
GPU_KITS = {"gpu-rendering"}
HELD_KITS = {"tabbed-browsing"}
BAR = 20


# an open criterion only a person can close (a finger on the device, the owner's decision), or one
# that waits on tabs: the rest is work still to build
OWNER = re.compile(r"\[human|human-only|finger|\[OWNER\]|DEFERRED by the owner|\[decision\]|judged by eye|owner's finger", re.I)
TABS = re.compile(r"waits on tabbed|tabbed-browsing\.md R", re.I)


def count_kit(name):
    """(met, open, open for a person, open waiting on tabs)"""
    path = os.path.join(ROOT, "context", "kits", "cavekit-%s.md" % name)
    text = open(path, encoding="utf-8").read()
    opens = re.findall(r"^- \[ \](.*)$", text, re.M)
    owner = sum(1 for o in opens if OWNER.search(o))
    tabs = sum(1 for o in opens if not OWNER.search(o) and TABS.search(o))
    return (len(re.findall(r"^- \[x\]", text, re.M)), len(opens), owner, tabs)


def count_finger():
    text = open(os.path.join(ROOT, "docs", "FINGER-TESTS.md"), encoding="utf-8").read()
    return (len(re.findall(r"^- \[x\]", text, re.M)), len(re.findall(r"^- \[ \]", text, re.M)))


def bar(met, total):
    if total <= 0:
        return "`" + "░" * BAR + "`"
    full = BAR * met // total   # a full bar only when every one is met
    return "`" + "█" * full + "░" * (BAR - full) + "`"


def pct(met, total):
    return "%d %%" % (100 * met // total) if total else "—"


def section():
    rows, everything = [], [0, 0]
    complete, owner_only, working = [], [], []
    for label, kits, what in AREAS:
        c = [count_kit(k) for k in kits]
        met, open_ = sum(x[0] for x in c), sum(x[1] for x in c)
        owner, tabs = sum(x[2] for x in c), sum(x[3] for x in c)
        work = open_ - owner - tabs
        total = met + open_
        held = bool(set(kits) & HELD_KITS)
        rows.append((label, met, total, what, work, owner, tabs, held))
        if held:
            continue
        everything[0] += met
        everything[1] += total
        if open_ == 0:
            complete.append(label)
        elif work == 0:
            owner_only.append(label)
        else:
            working.append((label, met, total, work))
    f_met, f_open = count_finger()
    f_total = f_met + f_open
    held = [r for r in rows if r[7]][0]

    def left(work, owner, tabs, held_area):
        if held_area:
            return "held"
        parts = []
        if work:
            parts.append("%d to do" % work)
        if owner:
            parts.append("%d for a person" % owner)
        if tabs:
            parts.append("%d with tabs" % tabs)
        return ", ".join(parts) if parts else "done"

    out = []
    out.append("## Where it stands")
    out.append("")
    out.append("Counted from the project's own records: every part of the browser has a list of acceptance "
               "criteria, each one met only when it was shown working on a TouchPad (or, for the build, on a "
               "clean machine). What is left is either still to build or waits for a person: a finger on the "
               "device or the owner's decision. The road below runs to the first public release.")
    out.append("")
    out.append("**Everything but tabs: %s %d of %d criteria met (%s).**" % (bar(*everything), everything[0], everything[1],
                                                                             pct(*everything)))
    out.append("")
    out.append("| Area | Progress | Met | Left | What it covers |")
    out.append("|---|---|---:|---|---|")
    for label, met, total, what, work, owner, tabs, held_area in rows:
        out.append("| %s | %s %s | %d / %d | %s | %s |" % (label, bar(met, total), pct(met, total), met, total,
                                                         left(work, owner, tabs, held_area), what))
    out.append("| The owner's finger round | %s %s | %d / %d | %s | the checks only a finger on the device can make |"
               % (bar(f_met, f_total), pct(f_met, f_total), f_met, f_total, "%d for a person" % f_open if f_open else "done"))
    out.append("")
    out.append("### The road to release")
    out.append("")
    out.append("```mermaid")
    out.append("flowchart LR")
    out.append('  nComplete["Complete<br/>%s"]:::done' % "<br/>".join(complete))
    out.append('  nWaiting["Built, waiting on a person<br/>%s"]:::owner' % "<br/>".join(owner_only))
    for i, (label, met, total, work) in enumerate(working):
        out.append('  nWork%d["%s<br/>%d of %d, %d to do"]:::doing' % (i, label, met, total, work))
    out.append('  nFinger["The owner\'s finger round<br/>%d checks"]:::owner' % f_total)
    out.append('  nReview["Last review and fixes"]:::todo')
    out.append('  nRelease(("Release")):::goal')
    out.append('  nTabs["Tabs: groundwork<br/>%d of %d, held"]:::held' % (held[1], held[2]))
    out.append("  nComplete --> nFinger")
    out.append("  nWaiting --> nFinger")
    for i in range(len(working)):
        out.append("  nWork%d --> nFinger" % i)
    out.append("  nFinger --> nReview --> nRelease")
    out.append("  nTabs -.-> nRelease")
    out.append("  classDef done fill:#2e7d32,stroke:#1b5e20,color:#ffffff")
    out.append("  classDef doing fill:#f9a825,stroke:#f57f17,color:#000000")
    out.append("  classDef owner fill:#1565c0,stroke:#0d47a1,color:#ffffff")
    out.append("  classDef todo fill:#eeeeee,stroke:#9e9e9e,color:#000000")
    out.append("  classDef held fill:#bdbdbd,stroke:#757575,color:#000000,stroke-dasharray: 4 3")
    out.append("  classDef goal fill:#6a1b9a,stroke:#4a148c,color:#ffffff")
    out.append("```")
    out.append("")
    out.append("Green is complete, amber still has work to do, blue waits for a person at the device (the "
               "finger tests are gathered into one round at the end), grey is not started or held. Whether "
               "tabs come before or after the release is the owner's call. Regenerated by "
               "`tools/readme-progress.py` from `context/kits/` and `docs/FINGER-TESTS.md`.")
    return "\n".join(out)


def main(argv):
    text = open(README, encoding="utf-8").read()
    body = section()
    if "--print" in argv:
        print(body)
        return 0
    if START not in text or END not in text:
        print("readme-progress: the README has no %s ... %s markers" % (START, END), file=sys.stderr)
        return 2
    i, j = text.index(START) + len(START), text.index(END)
    new = text[:i] + "\n" + body + "\n" + text[j:]
    if "--check" in argv:
        if new != text:
            print("readme-progress: the README's progress section is out of date (run tools/readme-progress.py)")
            return 1
        print("readme-progress: up to date")
        return 0
    if new != text:
        open(README, "w", encoding="utf-8").write(new)
        print("readme-progress: README updated")
    else:
        print("readme-progress: already up to date")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
