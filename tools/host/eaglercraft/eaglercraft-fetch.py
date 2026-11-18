#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# eaglercraft-fetch.py -- cavekit-eaglercraft.md R1 (T-665): each client of clients.json fetched into the host cache
# (~/.cache/jihad-eaglercraft/<id>/), its size checked, its SHA-256 recorded in clients.json at the first fetch and
# checked at every later one, and unpacked beside the archive. Then what each needs is read from its own code (needs.py
# does that, from what this leaves). Run with python3 -I.
#   eaglercraft-fetch.py [--only id,id]
import argparse, hashlib, json, os, sys, urllib.request, zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
MANIFEST = os.path.join(HERE, "clients.json")
CACHE = os.path.join(os.path.expanduser("~"), ".cache", "jihad-eaglercraft")


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("--only", default=""); o = ap.parse_args()
    only = set(x for x in o.only.split(",") if x)
    m = json.load(open(MANIFEST)); changed = False; bad = 0
    for c in m["clients"]:
        if only and c["id"] not in only: continue
        d = os.path.join(CACHE, c["id"]); os.makedirs(d, exist_ok=True)
        z = os.path.join(d, os.path.basename(c["url"]))
        if not os.path.exists(z) or os.path.getsize(z) != c["size"]:
            req = urllib.request.Request(c["url"], headers={"User-Agent": "jihad-eaglercraft-fetch"})
            with urllib.request.urlopen(req, timeout=120) as r, open(z + ".part", "wb") as f:
                while True:
                    b = r.read(1 << 16)
                    if not b: break
                    f.write(b)
            os.replace(z + ".part", z)
        size = os.path.getsize(z); h = hashlib.sha256(open(z, "rb").read()).hexdigest()
        if size != c["size"]: print("FAIL %s: %d bytes, %d wanted" % (c["id"], size, c["size"])); bad += 1; continue
        if not c["sha256"]: c["sha256"] = h; changed = True
        elif c["sha256"] != h: print("FAIL %s: sha256 %s, %s recorded" % (c["id"], h, c["sha256"])); bad += 1; continue
        out = os.path.join(d, "unpacked")
        if not os.path.isdir(out):
            with zipfile.ZipFile(z) as zf:
                for n in zf.namelist():
                    t = os.path.realpath(os.path.join(out, n))
                    if not t.startswith(os.path.realpath(out) + os.sep): raise SystemExit("unsafe path in %s: %s" % (z, n))
                zf.extractall(out)
        files = sorted(os.path.relpath(os.path.join(a, f), out) for a, _, fs in os.walk(out) for f in fs)
        print("%-20s %-18s %-10s %9d bytes  sha256 %s..  %d files: %s" % (c["id"], c["release"], c["build"], size, h[:12], len(files), ", ".join(files[:6])))
    if changed:
        json.dump(m, open(MANIFEST, "w"), indent=2); open(MANIFEST, "a").write("\n")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
