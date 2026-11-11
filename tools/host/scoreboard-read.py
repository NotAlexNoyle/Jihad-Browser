#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# scoreboard-read.py <stream.fbs> [--trace <stream.fin>] (T-629): what one browser's launch and pans looked like on the
# screen, from fbstream's samples, the same reading for every engine.
#   first_ms    the launch to the first sample with more than 2 % of the screen's rows changed (the card appears)
#   settled_ms  the launch to the last such sample before the first drag (the page done drawing, as the eye sees it)
#   pan_fps     the frames LunaSysMgr presented (the framebuffer flips) over the drags, a second
#   pan_moved   the share of the drags' samples whose content had moved from the sample before (a frozen pan reads low)
# The launch is taken as the stream's start + 1 s (scoreboard-device.sh sleeps 1 s between the two).
import importlib.util, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location("finger_stream", os.path.join(HERE, "finger-stream.py"))
fs = importlib.util.module_from_spec(spec); spec.loader.exec_module(fs)

def main():
    a = sys.argv[1:]
    if not a: print(__doc__ or "usage: scoreboard-read.py stream.fbs [--trace stream.fin]"); sys.exit(2)
    path = a[0]; trace = a[a.index('--trace') + 1] if '--trace' in a else None
    hdr, samples, flips, trailer = fs.read_stream(path)
    launch = hdr['start_ms'] + 1000
    scans = fs.read_trace(trace) if trace and os.path.exists(trace) else []
    downs = [s[0] for s in scans if s[1] == 1]
    drag0 = min(downs) if downs else launch + 32000
    drag1 = max(s[0] for s in scans) if scans else None
    rows = hdr['rows']
    changed = []
    for i in range(1, len(samples)):
        p, c = samples[i - 1], samples[i]
        n = sum(1 for r in range(rows) if p['sig'][r] != c['sig'][r])
        changed.append((c['t'], n / float(rows)))
    big = [t for t, f in changed if f > 0.02 and launch <= t < drag0]
    first = (big[0] - launch) if big else None
    settled = (big[-1] - launch) if big else None
    pan_fps = moved = None
    if drag1:
        fl = [t for t, y in flips if drag0 <= t <= drag1]
        pan_fps = len(fl) * 1000.0 / (drag1 - drag0) if drag1 > drag0 else None
        win = [f for t, f in changed if drag0 <= t <= drag1]
        moved = sum(1 for f in win if f > 0.02) / float(len(win)) if win else None
    fmt = lambda v, f='%d': ('NA' if v is None else f % v)
    print("first_ms %s settled_ms %s pan_fps %s pan_moved %s" % (fmt(first), fmt(settled), fmt(pan_fps, '%.1f'), fmt(moved, '%.2f')))

if __name__ == '__main__':
    main()
