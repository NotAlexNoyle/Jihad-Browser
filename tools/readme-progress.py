#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# The README's "Status" section, counted from the records: each kit's acceptance criteria
# (context/kits/cavekit-*.md, "- [x]" met, "- [ ]" open, "- [~]" partial, counted as open) grouped into the browser's areas, the
# emulated finger's tests (docs/FINGER-TESTS.md, generated from the suite and its last report).
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
    ("Speed and memory", ["lowpower-uxp", "device-headroom"],
     "the engine tuned for the TouchPad; no page can starve the device; YouTube made light"),
    ("The three interfaces", ["ui-shell", "mochi-ui", "mojo-ui", "chrome-look"], "Enyo, Mochi and Mojo"),
    ("Touch and keyboard", ["input-bridging"], "taps, gestures, the on-screen keyboard"),
    ("Settings and services", ["preferences-ui", "browser-services"], "the settings panes, downloads, history, printing"),
    ("Add-ons and Personas", ["addons-extensions", "addons-site", "persona-formats", "personas-ui"],
     "XPI add-ons, plugins, Personas, the add-on site"),
    ("Flash, finished", ["flash"], "the device's own Flash Player proven on real games, apps and videos in every shell"),
    ("Privacy and security", ["content-blocking", "proxies-certs", "certificate-trust"],
     "the ad and tracker blocker, proxies, certificates"),
    ("Building and updating", ["desktop-build", "device-build", "engine-update", "licensing-branding", "engine-release-2026-10",
                               "webos-sdk-update", "atlas-references"],
     "one repository builds every package; engine updates; the SDK; licences"),
    ("Testing on the device", ["finger-emulation", "finger-testing"],
     "an emulated finger and screen checks that test the browser on the TouchPad unattended"),
    ("Tabs", ["tabbed-browsing"], "groundwork only, held by the owner's decision"),
]
GPU_KITS = {"gpu-rendering"}
HELD_KITS = {"tabbed-browsing"}
BAR = 20


# an open criterion that waits on the owner's decision, or one that waits on tabs: the rest is work still to build
# (2026-11-16: no test is a person's, cavekit-finger-testing.md R7; the kits carry no such criterion now and this count reads 0)
# (2026-11-07: a bare "finger" no longer counts; the finger-testing and Flash kits mean the emulated one that runs unattended)
OWNER = re.compile(r"\[human|human-only|\[OWNER\]|DEFERRED by the owner|\[decision\]|judged by eye|owner'?s|a person's finger|real finger|"
                   r"BLOCKED-EXTERNAL|a person at the device", re.I)
TABS = re.compile(r"waits on tabbed|tabbed-browsing\.md R", re.I)


def count_kit(name):
    """(met, open, open for a person, open waiting on tabs)"""
    path = os.path.join(ROOT, "context", "kits", "cavekit-%s.md" % name)
    text = open(path, encoding="utf-8").read()
    opens = re.findall(r"^- \[[ ~]\](.*)$", text, re.M)   # a partial one is not met (2026-11-07: they were left out of both counts)
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


def unassigned():
    """kits no area counts: the section would silently leave their criteria out (it did, 2026-11-07: nine kits)"""
    named = {k for _, kits, _ in AREAS for k in kits}
    have = {f[len("cavekit-"):-3] for f in os.listdir(os.path.join(ROOT, "context", "kits"))
            if f.startswith("cavekit-") and f.endswith(".md") and f != "cavekit-overview.md"}
    # Global Jihad is a later project of its own (it begins when the browser is finished): not this README's count
    return sorted(have - named - {"global-jihad"})


def section():
    missing = unassigned()
    if missing:
        raise SystemExit("readme-progress: kits in no area: " + ", ".join(missing))
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
    out.append("## Status")
    out.append("")
    out.append("**%s %s done** (everything but tabs, which are on hold)." % (bar(*everything), pct(*everything)))
    out.append("")
    out.append("| Area | Progress | What it covers |")
    out.append("|---|---|---|")
    for label, met, total, what, work, owner, tabs, held_area in rows:
        # the README is for readers of the browser: the test rig's own row and the counts stay in the kits
        if label == "Testing on the device":
            continue
        out.append("| %s | %s %s | %s |" % (label, bar(met, total), "held" if held_area else pct(met, total), what))
    out.append("")
    out.append("<sub>Generated by `tools/readme-progress.py`; don't edit it by hand.</sub>")
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
