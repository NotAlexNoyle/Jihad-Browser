#!/usr/bin/env python3
# check-frame.py <frame.raw> <w> <h> [out.jpg]: is a dumped BGRA frame real content? Prints the
# non-black fraction, the number of distinct colours in a 1/8 sample and a verdict.
import sys
from PIL import Image
p, w, h = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
d = open(p, "rb").read()
if len(d) < w * h * 4:
    print("FRAME: short file (%d bytes)" % len(d)); sys.exit(1)
im = Image.frombytes("RGBA", (w, h), d[:w * h * 4], "raw", "BGRA"); px = im.load()
pts = [(x, y) for y in range(0, h, 8) for x in range(0, w, 8)]
nonblack = sum(1 for x, y in pts if sum(px[x, y][:3]) > 60)
colours = len(set(px[x, y][:3] for x, y in pts))
alpha0 = sum(1 for x, y in pts if px[x, y][3] != 255)
verdict = "CONTENT" if colours >= 8 and nonblack > len(pts) * 0.2 else ("UNIFORM" if colours < 8 else "DARK")
print("FRAME: %s  non-black %.2f  colours %d  alpha!=255 %d" % (verdict, nonblack / len(pts), colours, alpha0))
if len(sys.argv) > 4: im.convert("RGB").save(sys.argv[4], quality=85)
