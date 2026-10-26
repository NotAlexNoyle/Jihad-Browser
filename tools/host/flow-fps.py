#!/usr/bin/env python3
# flow-fps.py <daemon-gpu.log>...: T-505 round tool. The frame rate while frames flow (bursts of 5+ publishes with gaps
# under 300 ms, the pauses between drags left out), with the whole express reads, the parts pictures and the frames
# not shown. Used for the finger rounds in docs/rounds/t505-2026-10-26/notes.txt.
import sys, re, statistics
cut = 300
for path in sys.argv[1:]:
    ts = []; whole = 0; pics = 0; miss = 0
    for l in open(path, errors='replace'):
        m = re.search(r'\[jihad-tl\] t=(\d+) publish (express|main) gap=', l)
        if m: ts.append(int(m.group(1)))
        if re.search(r'express frame \d+: slot \d whole', l): whole += 1
        if 'came with the frame' in l: pics += 1
        if 'the frame not shown' in l: miss += 1
    ts.sort(); bursts = []; cur = [ts[0]] if ts else []
    for a, b in zip(ts, ts[1:]):
        if b - a < cut: cur.append(b)
        else:
            if len(cur) >= 5: bursts.append(cur)
            cur = [b]
    if len(cur) >= 5: bursts.append(cur)
    n = sum(len(b) - 1 for b in bursts); dur = sum(b[-1] - b[0] for b in bursts)
    gaps = [y - x for b in bursts for x, y in zip(b, b[1:])]
    per = ['%.1f' % ((len(b) - 1) * 1000.0 / (b[-1] - b[0])) for b in bursts if b[-1] > b[0]]
    print('%-28s %5.1f fps flowing (%d bursts, %d gaps, median gap %s ms) | whole express reads %d, pictures %d, frames not shown %d | per burst %s'
          % (path.split('/')[-1].replace('-daemon-gpu.log', ''), n * 1000.0 / dur if dur else 0, len(bursts), n,
             statistics.median(gaps) if gaps else '-', whole, pics, miss, ' '.join(per)))
