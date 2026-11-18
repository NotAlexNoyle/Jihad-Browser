#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# needs.py -- cavekit-eaglercraft.md R1 (T-665): what each fetched client asks of the browser, read from its own code in the
# host cache (eaglercraft-fetch.py first): the WebGL contexts it asks for and in which order, the WebGL extensions it
# names, and the platform's parts it reaches for. A markdown table on stdout. Run with python3 -I.
import json, os, re

HERE = os.path.dirname(os.path.abspath(__file__))
CACHE = os.path.join(os.path.expanduser("~"), ".cache", "jihad-eaglercraft")
API = [("IndexedDB", r"indexedDB"), ("Workers", r"new Worker\b|Worker\("), ("WebSocket", r"WebSocket"), ("Web Audio", r"AudioContext"),
       ("pointer lock", r"requestPointerLock"), ("full screen", r"requestFullscreen"), ("touch", r"touchstart"),
       ("BigInt", r"BigInt\("), ("WebAssembly", r"WebAssembly\."), ("SharedArrayBuffer", r"SharedArrayBuffer")]


def main():
    m = json.load(open(os.path.join(HERE, "clients.json")))
    print("| client | build | HTML | WebGL asked (in order) | extensions named | platform parts |")
    print("|---|---|---|---|---|---|")
    for c in m["clients"]:
        d = os.path.join(CACHE, c["id"], "unpacked")
        if not os.path.isdir(d): print("| %s | %s | not fetched | | | |" % (c["id"], c["build"])); continue
        html = [os.path.join(a, f) for a, _, fs in os.walk(d) for f in fs if f.endswith(".html")]
        s = "".join(open(h, errors="replace").read() for h in html)
        ctx = []
        for k in re.findall(r'getContext\("(webgl2|webgl|experimental-webgl)"', s):
            if k not in ctx: ctx.append(k)
        ext = sorted(set(re.findall(r'getExtension\("([A-Za-z0-9_]+)"\)', s)))
        parts = [n for n, p in API if re.search(p, s)]
        print("| %s (%s) | %s | %.1f MB | %s | %s | %s |" % (c["id"], c["release"], c["build"], sum(os.path.getsize(h) for h in html) / 1e6,
              " then ".join(ctx) or "none", ", ".join(ext) or "none", ", ".join(parts)))


if __name__ == "__main__":
    main()
