#!/usr/bin/env python3
"""gap-trace.py <daemon log> [min gap ms]: T-527 (cavekit-gpu-rendering R3). For every published frame of the pan (the
first frame that moved to the last) whose gap
to the previous one is at least MIN (56) ms, the timeline events (JIHAD_GPU_TIMELINE=1) between the two publishes:
the jobs (queued / composed on the compositor loop), the page's layer transactions on the compositor thread, the
main thread's long pumps and layer paints, the express lane's waits for the frame lock. Then a count of the gaps
by the event that took most of them."""
import re, sys
from collections import Counter

path = sys.argv[1]
mingap = int(sys.argv[2]) if len(sys.argv) > 2 else 56
ev = []   # (t_end, kind, ms, text)
for line in open(path, encoding='latin1'):
    m = re.search(r'\[jihad-tl\] t=(\d+) (.*)', line)
    if not m:
        continue
    t, rest = int(m.group(1)), m.group(2).strip()
    if rest.startswith('publish '):
        g = re.search(r'publish (\w+) gap=(\d+) made=(\d+)', rest)
        ev.append((t, 'publish', int(g.group(2)) if g else 0, rest))
    elif rest.startswith('job '):
        q = re.search(r'queued ([\d.]+) ms .* composed in ([\d.]+) ms', rest)
        ev.append((t, 'job', float(q.group(1)) + float(q.group(2)) if q else 0, rest))
    elif rest.startswith('layer transaction'):
        q = re.search(r'thread: ([\d.]+) ms', rest)
        ev.append((t, 'transaction', float(q.group(1)) if q else 0, rest))
    elif rest.startswith('main '):
        q = re.search(r'(\d+) ms', rest)
        ev.append((t, 'main', float(q.group(1)) if q else 0, rest))
    elif rest.startswith('express idle'):
        ev.append((t, 'idle', 0.0, rest))
    elif rest.startswith('express waited'):
        q = re.search(r'waited (\d+) ms', rest)
        ev.append((t, 'lockwait', float(q.group(1)) if q else 0, rest))
ev.sort(key=lambda e: e[0])
pubs = [e for e in ev if e[1] == 'publish']
# only the pan: from the first frame at another position than the first one to the last frame that moved
ys = [int(re.search(r' y=(-?\d+)', p[3]).group(1)) if re.search(r' y=(-?\d+)', p[3]) else 0 for p in pubs]
moved = [k for k in range(1, len(pubs)) if ys[k] != ys[k - 1]]
if moved:
    pubs = pubs[moved[0]:moved[-1] + 1]
    print('the pan: %d frames over %d ms' % (len(pubs), pubs[-1][0] - pubs[0][0]))
cause = Counter()
n = 0
for p in pubs:
    t, gap = p[0], p[2]
    if gap < mingap or gap > 2000:
        continue
    n += 1
    t0 = t - gap
    inside = [e for e in ev if t0 <= e[0] <= t and e is not p and e[1] != 'publish']
    # what overlaps the interval most: transactions and main-thread stalls by their duration inside it
    best = max((e for e in inside if e[1] != 'idle'), key=lambda e: min(e[2], e[0] - t0), default=None)
    cause[(best[1] + (' ' + best[3].split()[1] if best and best[1] == 'main' else '')) if best else 'none'] += 1
    print('gap %4d ms ending t=%d (%s)' % (gap, t, p[3][:60]))
    for e in inside:
        print('    %+5d %-11s %6.1f  %s' % (e[0] - t, e[1], e[2], e[3][:150]))
print('\n%d gaps >= %d ms; the longest event in each:' % (n, mingap))
for k, v in cause.most_common():
    print('  %3d  %s' % (v, k))
