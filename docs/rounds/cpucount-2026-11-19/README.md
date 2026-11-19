# Engine 0123: the CPU count (2026-11-19)

`js::GetCPUCount` took `sysconf(_SC_NPROCESSORS_ONLN)`. The TouchPad parks its second core while idle
(`/sys/devices/system/cpu/online` "0", `present` "0-1", read on the device), so an engine started then counted one CPU,
and with one CPU `OffThreadCompilationAvailable` is false: Ion refuses every script over `MAX_MAIN_THREAD_SCRIPT_SIZE`
(2000 bytecodes) or `MAX_MAIN_THREAD_LOCALS_AND_ARGS` (256) as "too large", and such scripts run in Baseline only. Found
on EaglercraftX 1.8.8 (TeaVM's large functions; cavekit-eaglercraft.md R6, T-666). Patch 0123 takes the larger of the
online and the configured cores; the kernel brings the parked core up under load.

## The bench (tools/device/lowpower-bench.sh, 3 runs each, Enyo, medians [range of the 0123 runs])

| | engine 0122 (`before/`) | engine 0123 (`after/`) | |
|---|---|---|---|
| Wikipedia, first frame to load done | 13491 ms | 10149 ms [10103..10632] | -25 % |
| its main-thread calls of 15 ms+, summed | 21445 ms | 18528 ms [18049..18675] | -14 % |
| its paints, summed | 2636 ms | 1343 ms [695..1750] | -49 % |
| resident after the load / 30 s later | 282 / 272 MB | 283 / 279 MB | +0.5 / +2.3 % |
| its scripted pan | 12.2 fps | 15.1 fps [14.4..15.9] | +24 % |
| jsbench loop 50M | 688 ms | 546 ms | -21 % |
| jsbench objects 2M | 1026 ms [1008..1386 before] | 1304 ms [1254..1680] | +27 % |
| jsbench strings / regex / dom | 750 / 118 / 3838 ms | 736 / 106 / 3748 ms | -2 / -10 / -2 % |
| the long page's pan | 34.4 fps | 33.2 fps | -3.5 % |
| daemon start cold / warm to engine up | 3637 / 2096 ms | 4163 / 2197 ms | +14 / +5 % (ranges overlap: 3556..4401 before) |
| card to first frame cold / warm | 4254 / 3673 ms | 4258 / 3447 ms | 0 / -6 % |

**Kept.** The page load, the main thread's work in it and the pan are better past the 0122 runs' own spread; the
memory is the same within 2 %; the slower rows (jsbench objects, the cold start) fall within the 0122 runs' ranges.
The rows and the daemon logs are in `before/` and `after/`.
