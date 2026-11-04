#!/usr/bin/env python3
# test-finger-stream.py <clean.fbs> [--trace <clean.fin>] [--still a-b,c-d] [--rows lo-hi]
# The stream tool's detectors against planted faults (cavekit-finger-testing.md R2: "tested against planted faults, not
# only clean runs"). Reads a clean stream of a drag, checks it reads clean (0 stale, 0 uniform, the still rows held),
# then plants in a copy, on a sample inside the drag: a duplicated band (40 content rows that did not move with the
# page), the header moved 10 rows, a grey band painted over 30 content rows — and checks each is reported where it was
# planted and nowhere else. Exit 0 when all pass.
import sys, copy, importlib.util, os
here = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location('fs', os.path.join(here, 'finger-stream.py')); fs = importlib.util.module_from_spec(spec); spec.loader.exec_module(fs)

def main():
    a = sys.argv[1:]
    if not a: print('usage: see the header'); return 2
    path = a[0]; trace = None; still_spec = ''; rows_spec = None; i = 1
    while i < len(a):
        if a[i] == '--trace': trace = a[i + 1]; i += 2
        elif a[i] == '--still': still_spec = a[i + 1]; i += 2
        elif a[i] == '--rows': rows_spec = a[i + 1]; i += 2
        else: print('unknown option', a[i]); return 2
    hdr, samples, flips, trailer = fs.read_stream(path)
    rows = hdr['rows']; still = [False] * rows
    for part in [p for p in still_spec.split(',') if p]:
        s, e = part.split('-')
        for r in range(int(s), int(e)): still[r] = True
    lo, hi = (0, rows) if not rows_spec else tuple(int(x) for x in rows_spec.split('-'))
    scans = fs.read_trace(trace) if trace else []
    fails = 0
    def check(name, cond, detail=''):
        nonlocal fails
        print('%s %s %s' % ('PASS' if cond else 'FAIL', name, detail)); fails += 0 if cond else 1
    clean = fs.analyse(hdr, samples, flips, trailer, scans, lo, hi, still)
    check('clean stream reads clean', not clean['stale_bands'] and not clean['uniform_bands'] and not clean['still_rows_moved'],
          '(stale %d, uniform %d, still moved %d)' % (len(clean['stale_bands']), len(clean['uniform_bands']), len(clean['still_rows_moved'])))
    key = lambda e: (e['i'], e['rows'])
    def new(rep, k):   # the reports a plant added over the clean reading
        base = {key(e) for e in clean[k]}
        return [e for e in rep[k] if key(e) not in base]
    # a sample inside the drag: the shift large and well matched
    moving = [p for p in clean['per_sample'] if p['d'] is not None and p['d'] >= 20 and p['matched'] >= 100 and samples[p['i']]['tries'] > 0 and samples[p['i'] - 1]['tries'] > 0]
    if not moving: print('FAIL: no moving sample in the stream'); return 1
    k = moving[len(moving) // 2]['i']
    # 1. a duplicated band: 40 content rows of sample k are sample k-1's same rows (they did not move with the page)
    s1 = copy.deepcopy(samples); band = (lo + (hi - lo) // 2, lo + (hi - lo) // 2 + 40)
    s1[k]['sig'] = list(s1[k]['sig']); s1[k]['first'] = list(s1[k]['first'])
    if s1[k]['prof'] is not None: s1[k]['prof'] = list(s1[k]['prof'])
    for r in range(*band):
        s1[k]['sig'][r] = s1[k - 1]['sig'][r]; s1[k]['first'][r] = s1[k - 1]['first'][r]; s1[k]['uni'][r] = s1[k - 1]['uni'][r]
        if s1[k]['prof'] is not None: s1[k]['prof'][r] = s1[k - 1]['prof'][r]
    r1 = fs.analyse(hdr, s1, flips, trailer, scans, lo, hi, still)
    added = new(r1, 'stale_bands')
    hits = [e for e in added if e['i'] == k and e['rows'][0] >= band[0] - 2 and e['rows'][1] <= band[1] + 2]
    check('duplicated band reported at sample %d rows %d-%d' % (k, *band), len(hits) >= 1 and len(added) == len(hits),
          '(added %s)' % [(e['i'], e['rows']) for e in added][:6])
    # 2. the header moved 10 rows: the still rows' signatures of sample k shifted down by 10
    srows = [r for r in range(rows) if still[r]]
    if srows:
        s2 = copy.deepcopy(samples); s2[k]['sig'] = list(s2[k]['sig'])
        if s2[k]['prof'] is not None: s2[k]['prof'] = list(s2[k]['prof'])
        for r in reversed(srows):
            if r - 10 >= 0:
                s2[k]['sig'][r] = samples[k]['sig'][r - 10]
                if s2[k]['prof'] is not None: s2[k]['prof'][r] = samples[k]['prof'][r - 10]
        r2 = fs.analyse(hdr, s2, flips, trailer, scans, lo, hi, still)
        added = new(r2, 'still_rows_moved')
        hits = [e for e in added if e['i'] == k]
        # the header is back in place at k + 1: that sample reports the move too, as it should
        check('header moved 10 rows reported at sample %d' % k, len(hits) >= 1 and all(e['i'] in (k, k + 1) for e in added),
              '(added %s)' % [(e['i'], e['rows']) for e in added][:6])
    else: print('SKIP header moved: no --still rows given')
    # 3. a grey band: 30 content rows of sample k one colour where sample k-1 (shifted) had content
    s3 = copy.deepcopy(samples); band3 = (lo + (hi - lo) // 3, lo + (hi - lo) // 3 + 30)
    s3[k]['sig'] = list(s3[k]['sig']); s3[k]['first'] = list(s3[k]['first'])
    if s3[k]['prof'] is not None: s3[k]['prof'] = list(s3[k]['prof'])
    for r in range(*band3):
        s3[k]['sig'][r] = 0x80808080; s3[k]['first'][r] = 0xff808080; s3[k]['uni'][r] = True
        if s3[k]['prof'] is not None: s3[k]['prof'][r] = hdr['cols'] * 3 * 128
    r3 = fs.analyse(hdr, s3, flips, trailer, scans, lo, hi, still)
    added = new(r3, 'uniform_bands')
    hits = [e for e in added if e['i'] == k and e['rows'][0] >= band3[0] - 2 and e['rows'][1] <= band3[1] + 2]
    check('grey band reported at sample %d rows %d-%d' % (k, *band3), len(hits) >= 1 and len(added) == len(hits),
          '(added %s)' % [(e['i'], e['rows'], e['colour']) for e in added][:6])
    print('%d failed' % fails)
    return 1 if fails else 0

if __name__ == '__main__': sys.exit(main())
