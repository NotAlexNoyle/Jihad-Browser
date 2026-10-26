#!/usr/bin/env python3
"""express-jobs.py <daemon log> [...]: T-505 (cavekit-gpu-rendering R3), the owner's "measure first" (2026-10-26).
Over the round's drags (between "[round] finger drags" and "[round] end", or the whole log), the frame stages from
the timeline (JIHAD_GPU_TIMELINE=1) and the compositor's own lines: each job's wait on the compositor loop and its
compose there (the compositor thread's CPU and its GL calls), the read thread's glReadPixels (the GPU finishing the
composite, behind whatever else the GPU has queued: LunaSysMgr's redraw of the card) and its swizzle, the publish's
gap and age. Medians, 90th percentiles and sums per stage, and the frame rate."""
import re, sys

def pct(v, p):
    if not v:
        return float('nan')
    v = sorted(v)
    return v[min(len(v) - 1, int(round(p * (len(v) - 1))))]

for path in sys.argv[1:]:
    lines = open(path, encoding='latin1').read().splitlines()
    a = next((i for i, l in enumerate(lines) if '[round] finger drags' in l), 0)
    b = next((i for i, l in enumerate(lines) if '[round] end' in l and i > a), len(lines))
    st = {k: [] for k in ('queued', 'composed', 'compose', 'handoff', 'readpx', 'rows', 'swizzle', 'gap', 'made', 'band')}
    pubs, kinds, lockwait, trans, idle = [], {}, [], [], {}
    for l in lines[a:b]:
        m = re.search(r'job \d+: queued ([\d.]+) ms on the compositor loop, composed in ([\d.]+) ms', l)
        if m: st['queued'].append(float(m.group(1))); st['composed'].append(float(m.group(2)))
        m = re.search(r'compositor thread: compose ([\d.]+) ms, handoff ([\d.]+) ms', l)
        if m: st['compose'].append(float(m.group(1))); st['handoff'].append(float(m.group(2)))
        m = re.search(r'band frame \d+: .* all ([\d.]+) ms', l)
        if m: st['band'].append(float(m.group(1)))
        # the band frame's own stages (every tenth band frame is logged)
        m = re.search(r'entry ([\d.]+) ms, surface in ([\d.]+) ms, compose ([\d.]+) ms, shift ([\d.]+) ms, read ([\d.]+) ms, surface out ([\d.]+) ms', l)
        if m:
            for k, g in zip(('b.entry', 'b.surfin', 'b.compose', 'b.shift', 'b.read', 'b.surfout'), m.groups()):
                st.setdefault(k, []).append(float(g))
        # a composite of 80 ms or more, by phase (JIHAD_GPU_PHASES=1, the first 40)
        m = re.search(r'slow composite (\d+) ms: shadow props ([\d.]+), async transforms ([\d.]+), post-process ([\d.]+), transforms\+diff ([\d.]+), begin frame ([\d.]+), prepare ([\d.]+), render layers ([\d.]+) \(draws ([\d.]+): ([\d.]+) calls, ([\d.]+) culled; uploads ([\d.]+)\), end frame ([\d.]+)', l)
        if m:
            g = m.groups()
            for k, v in zip(('s.all', 's.shadow', 's.async', 's.post', 's.diff', 's.begin', 's.prepare', 's.render', 's.draws', 's.calls', 's.culled', 's.uploads', 's.end'), g):
                st.setdefault(k, []).append(float(v))
        m = re.search(r'glReadPixels (\d+) rows ([\d.]+) ms, CPU swizzle\+flip ([\d.]+) ms', l)
        if m: st['rows'].append(int(m.group(1))); st['readpx'].append(float(m.group(2))); st['swizzle'].append(float(m.group(3)))
        m = re.search(r't=(\d+) publish (\w+) gap=(\d+) made=(\d+)', l)
        if m:
            pubs.append(int(m.group(1))); kinds[m.group(2)] = kinds.get(m.group(2), 0) + 1
            st['gap'].append(int(m.group(3))); st['made'].append(int(m.group(4)))
        m = re.search(r'express waited (\d+) ms for the frame lock', l)
        if m: lockwait.append(int(m.group(1)))
        m = re.search(r'layer transaction on the compositor thread: ([\d.]+) ms', l)
        if m: trans.append(float(m.group(1)))
        m = re.search(r'express idle: (.*)', l)
        if m:
            k = re.sub(r'\d+', 'N', m.group(1))[:60]
            idle[k] = idle.get(k, 0) + 1
    span = (pubs[-1] - pubs[0]) / 1000.0 if len(pubs) > 1 else 0
    print('== %s: %d frames published in %.1f s (%.1f fps), by lane %s' % (path.split('/')[-1], len(pubs), span,
          (len(pubs) - 1) / span if span else 0, kinds))
    for k in ('queued', 'composed', 'compose', 'handoff', 'band', 'b.entry', 'b.surfin', 'b.compose', 'b.shift', 'b.read',
              'b.surfout', 's.all', 's.shadow', 's.async', 's.post', 's.diff', 's.begin', 's.prepare', 's.render', 's.draws',
              's.calls', 's.uploads', 's.end', 'readpx', 'swizzle', 'rows', 'gap', 'made'):
        v = st.get(k, [])
        if v:
            print('  %-9s n=%3d  median %6.1f  p90 %6.1f  max %6.1f  sum %8.0f' % (k, len(v), pct(v, .5), pct(v, .9), max(v), sum(v)))
    if lockwait: print('  express waited for the frame lock (>=30 ms): n=%d, sum %d ms, max %d' % (len(lockwait), sum(lockwait), max(lockwait)))
    if trans: print('  the page\'s layer transactions on the compositor thread: n=%d, sum %.0f ms, max %.0f' % (len(trans), sum(trans), max(trans)))
    for k, n in sorted(idle.items(), key=lambda kv: -kv[1])[:6]:
        print('  idle note x%-4d %s' % (n, k))
