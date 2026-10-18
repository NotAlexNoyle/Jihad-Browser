#!/usr/bin/env bash
# Copyright 2026 NotAlexNoyle. MPL-2.0.
# check-engine-calls.sh — cavekit-engine-update.md R3 (T-385): the engine calls whose positional
# argument lists UXP changes between releases go through render/goanna/EngineCalls.h and nowhere
# else. Fails (exit 1) on a call to one of the wrapped methods in the daemon's code outside the
# wrapper header. The method list is read from EngineCalls.h itself, so a method wrapped later is
# guarded too. Comments are removed first (block comments inline or across lines, line comments),
# and a call may have spaces before its "(" or be split across lines (inspector, Tier 74 review).
# update-engine.sh runs it with check-engine-api.py.
set -euo pipefail
here="$(cd "$(dirname "$0")/../.." && pwd)"
exec python3 - "$here" <<'PYEOF'
import os, re, sys
here = sys.argv[1]
wrapper = os.path.join(here, "render", "goanna", "EngineCalls.h")
if not os.path.isfile(wrapper):
    print("FAIL: %s missing" % wrapper); sys.exit(1)
methods = sorted(set(re.findall(r"->\s*([A-Z][A-Za-z]*)\s*\(", open(wrapper).read())))
if not methods:
    print("FAIL: no wrapped methods found in EngineCalls.h"); sys.exit(1)
call = re.compile(r"(?:->|\.)\s*(" + "|".join(methods) + r")\s*\(")
comments = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)
def blank(m):   # keep the line count so a hit reports its line
    return re.sub(r"[^\n]", " ", m.group(0))
fail = False
n = 0
for root, dirs, files in os.walk(os.path.join(here, "render")):
    dirs[:] = [d for d in dirs if d != "third_party"]
    for f in sorted(files):
        if not f.endswith((".cpp", ".h")):
            continue
        p = os.path.join(root, f)
        text = open(p, encoding="utf-8", errors="replace").read()
        n += len(re.findall(r"engine_calls::[A-Za-z]+", text))
        if f == "EngineCalls.h":
            continue
        code = comments.sub(blank, text)
        hits = [(code.count("\n", 0, m.start()) + 1, m.group(1)) for m in call.finditer(code)]
        if hits:
            fail = True
            print("FAIL: %s calls a wrapped engine method directly:" % os.path.relpath(p, here))
            for line, name in hits:
                print("  %d: %s" % (line, name))
if not fail:
    print("ok: the wrapped methods (%s) are called only from EngineCalls.h; %d call sites use engine_calls::" % (" ".join(methods), n))
sys.exit(1 if fail else 0)
PYEOF
