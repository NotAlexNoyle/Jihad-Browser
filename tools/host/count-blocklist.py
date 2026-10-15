#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
"""Count, independently of the browser's compiler, what an ABP-format list holds in the categories
components/jihadContentBlocker.js compiles (cavekit-content-blocking.md R2): host rules, third-party
host rules, host exceptions, generic simple cosmetic rules, generic cosmetic exceptions, and the
rest (skipped). Usage: count-blocklist.py easylist.txt [easyprivacy.txt ...]"""
import re, sys

HOST = r'[a-z0-9][a-z0-9.-]*[a-z0-9]'
R_HOST = re.compile(r'^\|\|(' + HOST + r')\^$')
R_HOST_3P = re.compile(r'^\|\|(' + HOST + r')\^\$third-party$')
R_EXC = re.compile(r'^@@\|\|(' + HOST + r')\^$')
R_HOST_OPT = re.compile(r'^\|\|(' + HOST + r')\^\$([a-z,~-]+)$')
R_PATH = re.compile(r'^\|\|(' + HOST + r')(/[^$*^|]*)(?:\$third-party)?$')
R_COS = re.compile(r'^##([#.]-?[A-Za-z_][A-Za-z0-9_-]*)$')
R_COS_EXC = re.compile(r'^#@#([#.]-?[A-Za-z_][A-Za-z0-9_-]*)$')
R_SITE_COS = re.compile(r'^([a-z0-9.,-]+)##([^{}"\'\\\n]+)$')
R_SITE_EXC = re.compile(r'^([a-z0-9.,-]+)#@#([^{}"\'\\\n]+)$')
TYPES = {"other","script","image","stylesheet","object","subdocument","ping","xmlhttprequest","font","media","websocket","beacon","fetch","third-party","~third-party"}

# --check: run the fixed vectors next to this script and compare with the "! expected:" line.
if sys.argv[1:] == ["--check"]:
    import os
    v = os.path.join(os.path.dirname(os.path.abspath(__file__)), "count-blocklist-vectors.txt")
    sys.argv = [sys.argv[0], v]
for path in sys.argv[1:]:
    c = dict(rules=0, host=0, host3p=0, hostTyped=0, path=0, exc=0, cosmetic=0, cosmeticExc=0, siteCosmetic=0, siteCosmeticExc=0, skipped=0)
    version = ""
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            line = line.strip()
            if not line or line.startswith("["): continue
            if line.startswith("!"):
                if line.startswith("! Version:"): version = line.split(":", 1)[1].strip()
                continue
            c["rules"] += 1
            m = None
            if R_HOST.match(line): c["host"] += 1
            elif R_HOST_3P.match(line): c["host3p"] += 1
            elif (m := R_HOST_OPT.match(line)) and all(o in TYPES for o in m.group(2).split(",")): c["hostTyped"] += 1
            elif R_PATH.match(line): c["path"] += 1
            elif R_EXC.match(line): c["exc"] += 1
            elif R_COS.match(line): c["cosmetic"] += 1
            elif R_COS_EXC.match(line): c["cosmeticExc"] += 1
            elif (m := R_SITE_COS.match(line)) and "~" not in m.group(1): c["siteCosmetic"] += 1
            elif R_SITE_EXC.match(line): c["siteCosmeticExc"] += 1
            else: c["skipped"] += 1
    line = " ".join("%s=%d" % kv for kv in c.items())
    print(path, "version", version, line)
    exp = [l for l in open(path, encoding="utf-8", errors="replace") if l.startswith("! expected:")]
    if exp:
        want = exp[0].split(":", 1)[1].strip()
        print("vectors:", "OK" if want == line else "MISMATCH\n  want " + want + "\n  got  " + line)
        if want != line: sys.exit(1)
