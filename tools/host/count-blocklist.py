#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
"""Count, independently of the browser's compiler, what an ABP-format list holds in the categories
components/jihadContentBlocker.js compiles (cavekit-content-blocking.md R2): host rules, third-party
host rules, host exceptions, generic simple cosmetic rules, generic cosmetic exceptions, and the
rest (skipped). Usage: count-blocklist.py easylist.txt [easyprivacy.txt ...]

Two classifiers: the regexes below (the compiler's own family), and tokenize(), which reads each rule
a character at a time and shares nothing with them (inspector F-21). --cross <list...> counts with
both and prints every rule they put in different categories; --check runs the fixed vectors with
both."""
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

# The independent tokenizer (F-21): no regular expression, one pass over the characters.
HOST_CHARS = set("abcdefghijklmnopqrstuvwxyz0123456789.-")
SEL_FIRST = set("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_")
SEL_REST = SEL_FIRST | set("0123456789-")
DOMAIN_CHARS = HOST_CHARS | set(",")

def _host_ok(h):
    return len(h) >= 2 and all(ch in HOST_CHARS for ch in h) and h[0] not in ".-" and h[-1] not in ".-"

def _simple_selector(sel):
    # "#id" or ".class": the marker, an optional "-", a letter or "_", then name characters
    if len(sel) < 2 or sel[0] not in "#.": return False
    i = 1
    if sel[i] == "-": i += 1
    if i >= len(sel) or sel[i] not in SEL_FIRST: return False
    return all(ch in SEL_REST for ch in sel[i + 1:])

def tokenize(line):
    """The category of one rule line (not a comment or header), read character by character."""
    # cosmetic: the first "##" or "#@#" splits the domains from the selector
    hh, he = line.find("##"), line.find("#@#")
    if hh >= 0 or he >= 0:
        exc = he >= 0 and (hh < 0 or he < hh)
        at = he if exc else hh
        doms, sel = line[:at], line[at + (3 if exc else 2):]
        if not doms:
            if _simple_selector(sel): return "cosmeticExc" if exc else "cosmetic"
            return "skipped"
        if not sel or not all(ch in DOMAIN_CHARS or ch == "~" for ch in doms):
            return "skipped"
        if any(ch in '{}"\'\\\n' for ch in sel): return "skipped"
        if exc: return "siteCosmeticExc" if "~" not in doms else "skipped"
        return "siteCosmetic" if "~" not in doms else "skipped"
    # network: [@@] "||" host, then "^" [$options] or "/" path [$third-party]
    exc = line.startswith("@@")
    body = line[2:] if exc else line
    if not body.startswith("||"): return "skipped"
    i = 2
    while i < len(body) and body[i] in HOST_CHARS: i += 1
    host, rest = body[2:i], body[i:]
    if not _host_ok(host) or not rest: return "skipped"
    if rest[0] == "^":
        opts = rest[1:]
        if exc: return "exc" if opts == "" else "skipped"
        if opts == "": return "host"
        if opts[0] != "$" or len(opts) == 1: return "skipped"
        parts = opts[1:].split(",")
        if parts == ["third-party"]: return "host3p"
        return "hostTyped" if all(p in TYPES for p in parts) else "skipped"
    if rest[0] == "/" and not exc:
        dollar = rest.find("$")
        path, opts = (rest, "") if dollar < 0 else (rest[:dollar], rest[dollar:])
        if any(ch in "*^|" for ch in path): return "skipped"
        return "path" if opts in ("", "$third-party") else "skipped"
    return "skipped"

def regex_category(line):
    if R_HOST.match(line): return "host"
    if R_HOST_3P.match(line): return "host3p"
    m = R_HOST_OPT.match(line)
    if m and all(o in TYPES for o in m.group(2).split(",")): return "hostTyped"
    if R_PATH.match(line): return "path"
    if R_EXC.match(line): return "exc"
    if R_COS.match(line): return "cosmetic"
    if R_COS_EXC.match(line): return "cosmeticExc"
    m = R_SITE_COS.match(line)
    if m and "~" not in m.group(1): return "siteCosmetic"
    if R_SITE_EXC.match(line): return "siteCosmeticExc"
    return "skipped"

# --check: run the fixed vectors next to this script with both classifiers and compare each with
# the "! expected:" line. --cross: count with both and list the rules they disagree on.
cross = check = False
if sys.argv[1:] == ["--check"]:
    import os
    v = os.path.join(os.path.dirname(os.path.abspath(__file__)), "count-blocklist-vectors.txt")
    sys.argv = [sys.argv[0], v]
    cross = check = True
elif sys.argv[1:2] == ["--cross"]:
    sys.argv = [sys.argv[0]] + sys.argv[2:]
    cross = True
for path in sys.argv[1:]:
    c = dict(rules=0, host=0, host3p=0, hostTyped=0, path=0, exc=0, cosmetic=0, cosmeticExc=0, siteCosmetic=0, siteCosmeticExc=0, skipped=0)
    ct = dict(c)
    diffs = []
    version = ""
    with open(path, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            line = line.strip()
            if not line or line.startswith("["): continue
            if line.startswith("!"):
                if line.startswith("! Version:"): version = line.split(":", 1)[1].strip()
                continue
            c["rules"] += 1
            cat = regex_category(line)
            c[cat] += 1
            if cross:
                t = tokenize(line)
                ct[t] += 1
                if t != cat and len(diffs) < 40: diffs.append("  regex %s, tokenizer %s: %s" % (cat, t, line))
    line = " ".join("%s=%d" % kv for kv in c.items())
    print(path, "version", version, line)
    if cross:
        ct["rules"] = c["rules"]
        tline = " ".join("%s=%d" % kv for kv in ct.items())
        print("  tokenizer:", "the same" if tline == line else tline)
        for d in diffs: print(d)
        if tline != line and check: sys.exit(1)
    exp = [l for l in open(path, encoding="utf-8", errors="replace") if l.startswith("! expected:")]
    if exp:
        want = exp[0].split(":", 1)[1].strip()
        print("vectors:", "OK" if want == line else "MISMATCH\n  want " + want + "\n  got  " + line)
        if want != line: sys.exit(1)
