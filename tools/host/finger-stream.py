#!/usr/bin/env python3
# finger-stream.py <stream.fbs> [--trace <finger.fin>] [--daemon <daemon-gpu.log>] [--still ROWS] [--rows LO-HI]
#                  [--dump] [--json <out>]
# The stream tool of the finger rounds (cavekit-finger-testing.md R2, T-561/T-562): reads what fbstream (the device
# sampler, tools/device/finger/fbstream.c) recorded of the app framebuffer while the emulated finger moved, and says
# what the card SHOWED: the shown scroll per sample (the shift of the portrait rows' signatures between samples), the
# shown-frame spacing (from the pan flips: every frame LunaSysMgr presented, to the millisecond), the tracking error
# against the finger (pixels and milliseconds, from the finger's trace joined by time), stale or repeated bands (a
# run of content rows that did not move when the page did, or reappeared where the shift says it should not), uniform
# bands inside the page (rows of one colour where the previous sample had content at that place), and whether the
# fixed parts' rows (--still) held still. All times are gettimeofday milliseconds, the daemon's timeline clock too.
import sys, struct, statistics, collections, json
try:
    import numpy as np
except ImportError:
    np = None

def read_stream(path):
    b = open(path, 'rb').read()
    assert b[:4] in (b'FBS1', b'FBS2', b'FBS3'), 'not an fbstream file'
    ver = int(b[3:4]); v2 = ver >= 2; v3 = ver >= 3
    hb, = struct.unpack_from('<I', b, 4)
    f = struct.unpack_from('<12I', b, 8)
    hdr = dict(zip(['xres', 'yres', 'vyres', 'line', 'bpp', 'rows', 'cols', 'stride', 'hz', 'orient', 'poll_us', 'pread'], f))
    hdr['start_ms'], = struct.unpack_from('<Q', b, 8 + 12 * 4)
    hdr['col_lo'], hdr['col_hi'] = struct.unpack_from('<II', b, 8 + 12 * 4 + 8) if v3 else (0, hdr['yres'])
    rows = hdr['rows']; at = hb; samples = []; flips = []; trailer = None
    while at + 8 <= len(b):
        typ, n = struct.unpack_from('<II', b, at); at += 8
        if typ == 1:
            t, y0, y1, rd, nf, tries, fx, fy, fph = struct.unpack_from('<QIIIIIiii', b, at)
            o = at + 8 + 5 * 4 + 3 * 4
            sig = struct.unpack_from('<%dI' % rows, b, o); first = struct.unpack_from('<%dI' % rows, b, o + rows * 4)
            prof = struct.unpack_from('<%dI' % rows, b, o + rows * 8) if v2 else None
            sig2 = struct.unpack_from('<%dI' % rows, b, o + rows * 12) if v3 else None
            o2 = o + rows * (16 if v3 else 12 if v2 else 8)
            ub = b[o2: o2 + (rows + 7) // 8]
            uni = [bool(ub[i >> 3] & (1 << (i & 7))) for i in range(rows)]
            samples.append(dict(t=t, yoff0=y0, yoff1=y1, read_us=rd, flips=nf, tries=tries, fx=fx, fy=fy, fph=fph, sig=sig, first=first, prof=prof, sig2=sig2, uni=uni))
        elif typ == 2:
            t, y = struct.unpack_from('<QI', b, at); flips.append((t, y))
        elif typ == 3:
            s, fl, po, cpu, end = struct.unpack_from('<IIIIQ', b, at); trailer = dict(samples=s, flips=fl, polls=po, cpu_ms=cpu, end_ms=end)
        at += n
    return hdr, samples, flips, trailer

def read_trace(path):
    scans = []
    for l in open(path, errors='replace'):
        if l.startswith('#'): continue
        p = l.split()
        if len(p) >= 5: scans.append((int(p[0]), int(p[1]), int(p[2]), int(p[3]), int(p[4])))
    return scans

def finger_at(scans, t):
    best = None
    for s in scans:
        if s[1] != 1: continue
        if s[0] <= t: best = s
        else: break
    return best

TOL = 200   # the profile tolerance (--tol): |prof difference| at or under it is the same content. Measured 2026-11-03 on
            # desktop Wikipedia and the long page (stride 8, 96 columns): rows composed afresh at the right shift differ
            # by a median of 6-9, p90 85-94, p99 333 (the long page); rows of different content by a median of 2265-9495

def shift_exact(prev, cur, lo, hi, maxd=400):
    """The shift d (rows) by which cur's content rows match prev's exactly (cur[r] == prev[r + d]): the shown scroll
    between the two samples, positive when the content moved up (a pan down the page). Votes from the non-uniform rows
    whose signature is rare in prev (at most 3 places). Returns (d, matched, voters)."""
    where = collections.defaultdict(list)
    for r in range(lo, hi):
        if not prev['uni'][r]: where[prev['sig'][r]].append(r)
    votes = collections.Counter(); voters = 0
    for r in range(lo, hi):
        if cur['uni'][r]: continue
        places = where.get(cur['sig'][r])
        if not places or len(places) > 3: continue
        voters += 1
        for p in places:
            d = p - r
            if -maxd <= d <= maxd: votes[d] += 1
    if not votes: return None, 0, voters
    d, n = votes.most_common(1)[0]
    return d, n, voters

def shift_profile(prev, cur, lo, hi, maxd=400, tol=None):
    """The shift by the rows' profiles: the d with the least mean |cur[r] - prev[r + d]| over the overlap (at least
    200 rows), robust to the pixel differences of a frame composed afresh. Returns (d, agreeing rows, overlap, score)."""
    if np is None or prev['prof'] is None or cur['prof'] is None: return None, 0, 0, None
    tol = TOL if tol is None else tol
    A = np.array(prev['prof'][lo:hi], dtype=np.float64); B = np.array(cur['prof'][lo:hi], dtype=np.float64); n = hi - lo
    best = None
    for d in range(-maxd, maxd + 1):
        if d >= 0: a = A[d:]; b = B[:n - d]
        else: a = A[:n + d]; b = B[-d:]
        if len(a) < 200: continue
        sc = np.mean(np.abs(a - b))
        if best is None or sc < best[1]: best = (d, sc, len(a), int(np.sum(np.abs(a - b) <= tol)))
    if best is None: return None, 0, 0, None
    return best[0], best[3], best[2], best[1]

def same(prev, cur, ra, rb, tol=None):
    """Row ra of prev shows the same content as row rb of cur: the exact signature, or the profile within tolerance."""
    if prev['sig'][ra] == cur['sig'][rb]: return True
    if prev['prof'] is None or cur['prof'] is None: return False
    return abs(prev['prof'][ra] - cur['prof'][rb]) <= (TOL if tol is None else tol)

def read_daemon(path):
    """The daemon's timeline publishes: [(t ms, lane, y)] from [jihad-tl] t=<ms> publish <lane> ... y=<y>; a publish
    whose trailer said nothing changed (changed since N: x 0..0 rows 0..0 — the frame after a drag settles) is left
    out: the card has nothing to present for it."""
    out = []; empty = False
    for l in open(path, errors='replace'):
        if '[jihad-tl] t=' not in l: continue
        if ' trailer: frame ' in l: empty = 'rows 0..0' in l; continue
        if ' publish ' not in l: continue
        try:
            t = int(l.split('t=')[1].split()[0]); lane = l.split(' publish ')[1].split()[0]; y = int(l.split(' y=')[1].split()[0])
        except (IndexError, ValueError): continue
        if not empty: out.append((t, lane, y))
        empty = False
    return out

def publish_to_show(pubs, flips, late_ms=100):
    """Each publish against the first flip after it: the card's delay in showing a frame; the publishes with no flip
    within late_ms are 'not shown in time' (the verify mode cannot see this: it watches the compositor's side)."""
    ft = [t for t, _ in flips]; import bisect
    delays = []; late = []
    for t, lane, y in pubs:
        k = bisect.bisect_right(ft, t)
        if k < len(ft) and ft[k] - t <= late_ms: delays.append(ft[k] - t)
        else: late.append((t, lane, y, ft[k] - t if k < len(ft) else None))
    return delays, late

def runs(flags, minlen):
    """[(start, end)) of runs of True at least minlen long"""
    out = []; s = None
    for i, v in enumerate(list(flags) + [False]):
        if v and s is None: s = i
        if not v and s is not None:
            if i - s >= minlen: out.append((s, i))
            s = None
    return out

def analyse(hdr, samples, flips, trailer, scans, lo, hi, still, minrun=8, pubs=None):
    rows = hdr['rows']; rep = {}
    rep['samples'] = len(samples); rep['flips'] = len(flips)
    if len(samples) >= 2:
        span = (samples[-1]['t'] - samples[0]['t']) / 1000.0
        rep['sample_rate'] = round((len(samples) - 1) / span, 1) if span > 0 else None
    rep['read_ms_median'] = round(statistics.median(s['read_us'] for s in samples) / 1000.0, 2) if samples else None
    rep['read_ms_max'] = round(max(s['read_us'] for s in samples) / 1000.0, 2) if samples else None
    rep['torn'] = sum(1 for s in samples if s['tries'] == 0)
    rep['reread'] = sum(1 for s in samples if s['tries'] > 1)
    if trailer:
        secs = (trailer['end_ms'] - hdr['start_ms']) / 1000.0
        rep['cpu_pct_of_a_core'] = round(100.0 * trailer['cpu_ms'] / 1000.0 / secs, 1) if secs > 0 else None
        rep['poll_rate'] = round(trailer['polls'] / secs) if secs > 0 else None
    # the shown-frame spacing: the flips
    gaps = [b[0] - a[0] for a, b in zip(flips, flips[1:])]
    moving = [g for g in gaps if g < 300]   # inside a gesture (the pauses between gestures left out)
    rep['frame_gap_ms'] = dict(n=len(moving), median=statistics.median(moving) if moving else None,
                               p90=sorted(moving)[int(0.9 * (len(moving) - 1))] if moving else None, max=max(moving) if moving else None)
    # the shown scroll per sample
    per = []; shown = 0; stale = []; uniform = []; still_moved = []; parts_changed = []; unknown = 0
    for i in range(1, len(samples)):   # first the shifts, sample by sample
        a, b = samples[i - 1], samples[i]
        changed = a['sig'][lo:hi] != b['sig'][lo:hi]
        de, ne, voters = shift_exact(a, b, lo, hi)
        how = 'exact'
        if not changed: d, n = 0, hi - lo
        elif de is not None and ne >= 30: d, n = de, ne
        else:
            dp, agree, overlap, score = shift_profile(a, b, lo, hi)
            if dp is not None and overlap and agree >= 0.5 * overlap: d, n, how = dp, agree, 'profile'
            elif de is not None and ne >= 8: d, n = de, ne
            else: d, n, how = None, 0, 'unknown'
        if d is None: unknown += 1
        else: shown += d
        fin = finger_at(scans, b['t']) if scans else None
        per.append(dict(i=i, t=b['t'], dt=b['t'] - a['t'], d=d, how=how, matched=n, voters=voters, changed=changed, shown=shown,
                        finger=(fin[2], fin[3], fin[4]) if fin else None))
    for p in per:   # then the verdicts, with the next sample's shift in hand
        i = p['i']; a, b = samples[i - 1], samples[i]; d = p['d']; changed = p['changed']
        dn = per[i]['d'] if i < len(per) else None   # the shift from b to the next sample
        c = samples[i + 1] if i + 1 < len(samples) else None
        torn = a['tries'] == 0 or b['tries'] == 0
        if d is None or torn: continue
        if d != 0:
            # stale: a run of content rows that show what was there before (b[r] = a[r]) and not what the page's move
            # says belongs there (b[r] differs from a[r + d]); the still rows excepted, and one-colour rows only when
            # they equal the row before exactly and content belongs there (a band under a fixed header that did not
            # move with the page, 2026-11-03)
            st = [not still[r] and lo <= r + d < hi and same(a, b, r, r) and not same(a, b, r + d, r)
                  and (not b['uni'][r] or (a['sig'][r] == b['sig'][r] and not a['uni'][r + d])) for r in range(rows)]
            for s, e in runs(st, minrun):
                if lo <= s and e <= hi: stale.append(dict(i=i, rows=(s, e)))
        # uniform now where content belongs: the previous sample's content, shifted by d, was not one colour there —
        # or, for rows that came in from the edge, the next sample shows content where this one showed one colour
        # (a band painted late: the eye sees it flash)
        # (both looked up inside the content rows only: what moved under a fixed header is the header's pixels now)
        unow = [b['uni'][r] and not still[r] and ((lo <= r + d < hi and not a['uni'][r + d]) or
                (c is not None and dn is not None and lo <= r - dn < hi and not c['uni'][r - dn] and c['tries'] > 0)) for r in range(rows)]
        for s, e in runs(unow, minrun):
            if lo <= s and e <= hi: uniform.append(dict(i=i, rows=(s, e), colour='%08x' % b['first'][s]))
        # the still rows: did they hold?
        if changed:
            moved = [still[r] and not same(a, b, r, r) for r in range(rows)]
            for s, e in runs(moved, 2): still_moved.append(dict(i=i, rows=(s, e)))
        # the columns outside the content (fixed sidebars): which rows of them changed
        if a['sig2'] is not None and b['sig2'] is not None:
            pm = [a['sig2'][r] != b['sig2'][r] for r in range(lo, hi)]
            for s, e in runs(pm, 2): parts_changed.append(dict(i=i, rows=(s + lo, e + lo)))
    rep['per_sample'] = per
    rep['shown_total'] = shown; rep['shift_unknown'] = unknown
    # the shown scroll going backwards under a finger that did not: a frame shown wrong (an older one), as the eye sees it
    rep['backwards'] = [p for p in per if p['d'] is not None and p['d'] <= -8 and p['finger'] and p['finger'][2] in (0, 1)]
    if pubs and samples:
        t0, t1 = samples[0]['t'], samples[-1]['t']
        delays, late = publish_to_show([q for q in pubs if t0 <= q[0] <= t1], flips)   # the publishes the stream watched
        rep['publish_to_show'] = dict(n=len(delays), median=statistics.median(delays) if delays else None,
                                      p90=sorted(delays)[int(0.9 * (len(delays) - 1))] if delays else None, max=max(delays, default=None),
                                      late=late)
    rep['stale_bands'] = stale; rep['uniform_bands'] = uniform; rep['still_rows_moved'] = still_moved
    rep['parts_changed'] = parts_changed
    # the tracking error: the shown scroll against the finger's travel while it was down
    if scans:
        # the gestures of finger 1: (t down, t up, [(t, travel)]) with the travel in rows from the down point
        # (a finger moving up the screen = the content moving up = a positive shift)
        gestures = []; cur = None
        for s in scans:
            if s[1] != 1: continue
            if s[4] == 1: cur = dict(t0=s[0], t1=None, y0=s[3], path=[]); gestures.append(cur)
            if cur is None: continue
            cur['path'].append((s[0], cur['y0'] - s[3]))
            if s[4] == -1: cur['t1'] = s[0]; cur = None
        err_px = []; lag_ms = []; per_gesture = []
        for g in gestures:
            if g['t1'] is None: continue
            base = None; last = None; n0 = len(err_px); unk = 0
            for p in per:
                if p['t'] < g['t0']: base = p['shown']; continue
                if p['t'] > g['t1']: break
                if base is None: base = p['shown'] - (p['d'] or 0)
                if p['d'] is None: unk += 1
                shown_now = p['shown'] - base
                travel = next((tr for (t, tr) in reversed(g['path']) if t <= p['t']), 0)
                err_px.append(shown_now - travel)
                t_pass = next((t for (t, tr) in g['path'] if tr >= shown_now), None)
                lag_ms.append(p['t'] - t_pass if t_pass is not None else p['t'] - g['t0'])
                last = (shown_now, travel)
            if last: per_gesture.append(dict(t0=g['t0'], finger=g['path'][-1][1], shown=last[0], at_up=last[1], samples=len(err_px) - n0, unknown=unk))
        rep['tracking'] = dict(n=len(err_px), err_px_median=statistics.median(err_px) if err_px else None,
                               err_px_max=max((abs(e) for e in err_px), default=None),
                               lag_ms_median=statistics.median(lag_ms) if lag_ms else None, lag_ms_max=max(lag_ms, default=None),
                               gestures=per_gesture)
    return rep

def main():
    a = sys.argv[1:]
    if not a: print(__doc__ or 'usage: finger-stream.py <stream.fbs> ...'); return 2
    path = a[0]; trace = None; daemon = None; still_spec = ''; rows_spec = None; dump = False; jout = None
    i = 1
    while i < len(a):
        if a[i] == '--trace': trace = a[i + 1]; i += 2
        elif a[i] == '--daemon': daemon = a[i + 1]; i += 2
        elif a[i] == '--still': still_spec = a[i + 1]; i += 2
        elif a[i] == '--rows': rows_spec = a[i + 1]; i += 2
        elif a[i] == '--dump': dump = True; i += 1
        elif a[i] == '--json': jout = a[i + 1]; i += 2
        elif a[i] == '--tol':
            global TOL
            TOL = int(a[i + 1]); i += 2
        else: print('unknown option', a[i]); return 2
    hdr, samples, flips, trailer = read_stream(path)
    rows = hdr['rows']
    still = [False] * rows
    for part in [p for p in still_spec.split(',') if p]:
        s, e = part.split('-'); 
        for r in range(int(s), int(e)): still[r] = True
    lo, hi = (0, rows) if not rows_spec else tuple(int(x) for x in rows_spec.split('-'))
    scans = []
    if trace: scans = read_trace(trace)
    elif samples and any(s['fph'] != -2 for s in samples):
        scans = [(s['t'], 1, s['fx'], s['fy'], s['fph']) for s in samples if s['fph'] != -2]
    pubs = read_daemon(daemon) if daemon else None
    rep = analyse(hdr, samples, flips, trailer, scans, lo, hi, still, pubs=pubs)
    print('%s: %d samples (%s/s asked %d, read median %s ms max %s ms, %s stride %d, re-read %d, torn %d), %d flips, cpu %s%% of a core, polls %s/s'
          % (path.split('/')[-1], rep['samples'], rep.get('sample_rate'), hdr['hz'], rep['read_ms_median'], rep['read_ms_max'],
             'pread' if hdr['pread'] else 'mmap', hdr['stride'], rep['reread'], rep['torn'], rep['flips'], rep.get('cpu_pct_of_a_core'), rep.get('poll_rate')))
    g = rep['frame_gap_ms']
    print('shown frames while moving: %d gaps, median %s ms, p90 %s ms, max %s ms' % (g['n'], g['median'], g['p90'], g['max']))
    print('shown scroll: %d rows over the stream (%d samples with the shift unknown); stale bands %d, uniform bands %d, still rows moved %d; the columns outside %d-%d changed in %d samples'
          % (rep['shown_total'], rep['shift_unknown'], len(rep['stale_bands']), len(rep['uniform_bands']), len(rep['still_rows_moved']),
             hdr['col_lo'], hdr['col_hi'], len({e['i'] for e in rep['parts_changed']})))
    if 'tracking' in rep:
        t = rep['tracking']
        print('tracking (finger down, %d samples): error median %s px, max %s px; lag median %s ms, max %s ms'
              % (t['n'], t['err_px_median'], t['err_px_max'], t['lag_ms_median'], t['lag_ms_max']))
        for g in t['gestures']:
            print('  gesture at %d: the finger travelled %d rows, the page showed %d by the last sample while down (finger then at %d), %d samples%s'
                  % (g['t0'], g['finger'], g['shown'], g['at_up'], g['samples'], ', %d unknown' % g['unknown'] if g['unknown'] else ''))
    if rep['backwards']:
        print('the shown scroll went backwards under the finger in %d samples: %s' % (len(rep['backwards']), [(p['i'], p['d'], p['how']) for p in rep['backwards']][:8]))
    if 'publish_to_show' in rep:
        q = rep['publish_to_show']
        print('publish to show (the daemon\'s frame to the card\'s flip): %d frames, median %s ms, p90 %s ms, max %s ms; %d published and not shown within 100 ms%s'
              % (q['n'], q['median'], q['p90'], q['max'], len(q['late']), (': ' + ', '.join('t=%d %s y=%d (%s ms)' % l for l in q['late'][:5])) if q['late'] else ''))
    for k in ('stale_bands', 'uniform_bands', 'still_rows_moved'):
        for e in rep[k][:12]: print('  %s: sample %d rows %d-%d %s' % (k, e['i'], e['rows'][0], e['rows'][1], e.get('colour', '')))
    if dump:
        for p in rep['per_sample']:
            print('  #%3d t=%d dt=%3d d=%4s %-7s matched=%4d/%4d shown=%5d %s finger=%s' % (p['i'], p['t'], p['dt'], p['d'], p['how'], p['matched'], p['voters'], p['shown'], 'changed' if p['changed'] else 'same   ', p['finger']))
    if jout:
        json.dump(rep, open(jout, 'w'), indent=1, default=str)
    return 0

if __name__ == '__main__': sys.exit(main())
