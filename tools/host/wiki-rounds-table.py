#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# wiki-rounds-table.py <round dir> <name>... : T-567's one table per Wikipedia round — the bottom band (samples with a
# one-colour band in rows 900+, its tallest), the express frames' making time in the first three drags and after
# (the daemon log), the rate (flow-fps), the main thread's CPU share when the threads were sampled (ft-<name>.txt).
import json, os, re, subprocess, sys
D = sys.argv[1]; names = sys.argv[2:]
R = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))   # the repository
print(f"{'round':10} {'bottom band: samples / tallest':30} {'express made ms drags 1-3 median/p90/max':42} {'drags 4-16':16} {'fps':5} {'main%':5}")
for n in names:
    fbs, fin, log = f'{D}/fs-{n}.fbs', f'{D}/fs-{n}.fin', f'{D}/fp-{n}-daemon-gpu.log'
    if not os.path.exists(fbs) or not os.path.exists(log): print(f'{n:10} (missing)'); continue
    jpath = os.path.join(os.environ.get('TMPDIR', '/tmp'), f'wiki-table-{n}.json')
    r = subprocess.run([sys.executable, '-I', f'{R}/tools/host/finger-stream.py', fbs, '--trace', fin, '--daemon', log, '--rows', '83-1024', '--still', '29-83', '--json', jpath], capture_output=True, text=True)
    if r.returncode or not os.path.exists(jpath): print(f'{n:10} (the stream tool failed: {r.stderr.strip().splitlines()[-1] if r.stderr.strip() else r.returncode})'); continue
    j = json.load(open(jpath))
    bottom = [b for b in j['uniform_bands'] if b['rows'][0] >= 900]
    samples = len({b['i'] for b in bottom}); tallest = max((b['rows'][1] - b['rows'][0] for b in bottom), default=0)
    t0 = int(re.search(r'at (\d+)', [l for l in open(fin) if l.startswith('# drag')][0]).group(1))
    made = []
    for line in open(log, errors='replace'):
        m = re.search(r'\[jihad-tl\] t=(\d+) publish express gap=-?\d+ made=(\d+)', line)
        if m: made.append((int(m.group(1)) - t0, int(m.group(2))))
    def stats(a, b):
        v = sorted(x[1] for x in made if a <= x[0] < b)
        return f'{v[len(v)//2]}/{v[int(len(v)*0.9)]}/{v[-1]} ({len(v)})' if v else '-'
    fps = subprocess.run([sys.executable, '-I', f'{R}/tools/host/flow-fps.py', log], capture_output=True, text=True).stdout.strip().split('\n')[-1]
    fps = re.search(r'([\d.]+) fps', fps).group(1) if 'fps' in fps else '?'
    mainpct = '-'
    ft = f'{D}/ft-{n}.txt'
    if os.path.exists(ft):
        out = subprocess.run([sys.executable, '-I', f'{R}/tools/host/threadstat-read.py', ft, log], capture_output=True, text=True).stdout
        m = re.search(r'the main thread is (\S+) \):.*?\1 (\d+)', out); dur = re.search(r'over (\d+) ms', out)
        if m and dur: mainpct = f'{100 * int(m.group(2)) / int(dur.group(1)):.0f}'
    print(f'{n:10} {samples:3} / {tallest:3} rows{"":16} {stats(0, 8000):42} {stats(8000, 36000):16} {fps:5} {mainpct:5}')
