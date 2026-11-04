#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# threadstat-read.py <ft-round.txt> <fp-round-daemon-gpu.log> [--slow MS] [--fast MS]
#
# T-567: what the daemon's threads did while each express frame was made. The device's threadstat
# (tools/device/t502/threadstat.c) samples the daemon's tasks every 50 ms; the daemon log's
# "[jihad-gpu] express frame N: ... composed and read in M ms" lines, with the "[jihad-tl] t=... publish
# express" line that follows each, give every express frame's making interval. For the slow frames
# (M >= --slow, 150) and the fast ones (M <= --fast, 60) this prints what the samples inside the
# interval saw: the main thread running, the Compositor thread's states, the express thread's, the
# cores' idle share, the GPU interrupt rate.
import re
import sys


def read_samples(path):
    out = []
    for line in open(path, errors='replace'):
        m = re.match(r't=(\d+) irq=(-?\d+),(-?\d+) cpu0=(-?\d+),(-?\d+),(-?\d+) cpu1=(-?\d+),(-?\d+),(-?\d+) \|(.*)$', line.strip())
        if not m:
            continue
        t = int(m.group(1))
        irq = (int(m.group(2)), int(m.group(3)))
        cpu0 = tuple(int(m.group(k)) for k in (4, 5, 6))
        cpu1 = tuple(int(m.group(k)) for k in (7, 8, 9))
        tasks = {}
        for tok in m.group(10).split():
            parts = tok.split(':')
            if len(parts) != 5:
                continue
            tid, comm, state, cpu, ticks = parts
            # keyed by the thread id with its name: six JS helpers and two ld-2.23.so threads share a name,
            # and the process's first thread (the engine's main thread) is the one whose tid is the pid
            tasks[f'{comm}#{tid}'] = (state, int(cpu), int(ticks))
        out.append((t, irq, cpu0, cpu1, tasks))
    return out


def key_of(tasks, name):
    # the task named name: the main thread when name is 'main' (the lowest tid, the process's own)
    if name == 'main':
        ks = sorted((int(k.rsplit('#', 1)[1]), k) for k in tasks)
        return ks[0][1] if ks else None
    for k in tasks:
        if k.rsplit('#', 1)[0] == name:
            return k
    return None


def read_frames(path):
    # the express frames: the "[jihad-gpu] express frame N: ... composed and read in M ms" line, then its
    # publish line "[jihad-tl] t=T publish express ..." gives the end time
    frames = []
    pending = None
    for line in open(path, errors='replace'):
        m = re.search(r'\[jihad-gpu\] express frame (\d+): .*composed and read in (\d+) ms', line)
        if m:
            pending = (int(m.group(1)), int(m.group(2)))
            continue
        m = re.search(r'\[jihad-tl\] t=(\d+) publish express gap=(-?\d+) made=(\d+)', line)
        if m and pending:
            t = int(m.group(1))
            frames.append({'n': pending[0], 'made': pending[1], 't1': t, 't0': t - pending[1]})
            pending = None
    return frames


def summarize(samples, frames, label):
    if not frames:
        print(f'{label}: no frames')
        return
    n = 0
    mainR = compR = compD = compS = exprR = exprD = 0
    idle0 = idle1 = busy = irq = 0
    for f in frames:
        inside = [s for s in samples if f['t0'] <= s[0] <= f['t1']]
        if len(inside) < 2:
            continue
        for s in inside:
            n += 1
            t, _, _, _, tasks = s
            main = tasks.get(key_of(tasks, 'main'))
            comp = tasks.get(key_of(tasks, 'Compositor'))
            expr = tasks.get(key_of(tasks, 'JihadExpress'))
            if main and main[0] == 'R': mainR += 1
            if comp:
                if comp[0] == 'R': compR += 1
                elif comp[0] == 'D': compD += 1
                else: compS += 1
            if expr:
                if expr[0] == 'R': exprR += 1
                elif expr[0] == 'D': exprD += 1
        a, b = inside[0], inside[-1]
        dt = (b[0] - a[0]) / 10.0 or 1   # ticks are 10 ms
        idle0 += (b[2][2] - a[2][2]) / dt
        idle1 += (b[3][2] - a[3][2]) / dt
        irq += (b[1][0] + b[1][1] - a[1][0] - a[1][1]) / ((b[0] - a[0]) / 1000.0 or 1)
        busy += 1
    if not n:
        print(f'{label}: no samples inside the frames')
        return
    print(f'{label}: {len(frames)} frames, {n} samples inside them')
    print(f'  main thread running {100 * mainR / n:.0f} % of the samples; Compositor running {100 * compR / n:.0f} %, in the kernel (D) {100 * compD / n:.0f} %, sleeping {100 * compS / n:.0f} %; express thread running {100 * exprR / n:.0f} %, D {100 * exprD / n:.0f} %')
    print(f'  cpu0 idle {100 * idle0 / busy:.0f} %, cpu1 idle {100 * idle1 / busy:.0f} % of the interval; kgsl interrupts {irq / busy:.0f}/s')


def main():
    a = sys.argv[1:]
    if len(a) < 2:
        print(__doc__)
        sys.exit(2)
    slow, fast = 150, 60
    i = 2
    while i < len(a):
        if a[i] == '--slow': slow = int(a[i + 1]); i += 2
        elif a[i] == '--fast': fast = int(a[i + 1]); i += 2
        else: i += 1
    samples = read_samples(a[0])
    frames = read_frames(a[1])
    print(f'{len(samples)} samples, {len(frames)} express frames; made ms: median {sorted(f["made"] for f in frames)[len(frames) // 2] if frames else 0}')
    comms = sorted({c.rsplit('#', 1)[0] for s in samples for c in s[4]})
    print('threads seen:', ' '.join(comms))
    summarize(samples, [f for f in frames if f['made'] >= slow], f'slow frames (>= {slow} ms)')
    summarize(samples, [f for f in frames if f['made'] <= fast], f'fast frames (<= {fast} ms)')
    # which thread of the daemon took the CPU over the round: the ticks' growth per comm
    first, last = samples[0][4], samples[-1][4]
    growth = sorted(((last[c][2] - first[c][2]) * 10, c) for c in last if c in first)
    mainK = key_of(last, 'main')
    print('CPU ms over the round by thread (the main thread is', mainK, '):', ', '.join(f'{c} {ms}' for ms, c in reversed(growth[-8:])))
    print(f'  all threads {sum(ms for ms, c in growth)} ms over {samples[-1][0] - samples[0][0]} ms')


if __name__ == '__main__':
    main()
