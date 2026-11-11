<!-- A review made for T-556 on 2026-11-11 by a Fable agent, read-only: the instrumented engine's cost, the profile's coverage, the PGO plan for GCC 9.4. Its evidence files (disassemblies, gcda statistics, test compiles) stayed in that session's scratch directory. -->

# PGO for Jihad Browser's libxul with GCC 9.4: why the instrumented engine is slow, and the best plan this toolchain can carry out

Date: 2026-11-11. Read-only review; nothing in the repo, the objdirs or the device was touched. Evidence files are listed in
the appendix. All GCC statements below are checked against the `releases/gcc-9` sources (fetched into
`fable-pgo/gcc9-src/`) and, where it mattered, against the cross compiler itself (`arm-webos-linux-gnueabi-gcc 9.4.0` runs on
this host; `gcov-dump` and `gcov-tool` exist beside it; `qemu-arm` is installed, so a small gen -> run -> use cycle was run
end to end with the real toolchain and sysroot).

## 0. Verdict in five lines

1. The gen engine is ~9x slower warm (28 s against 3 s) because every function entry calls `__tls_get_addr` through the PLT
   (the indirect-call profiler's TLS tuple), every arc is a 64-bit `ldrexd/strexd` loop, and -Os's inliner stops inlining the
   bloated bodies. Two GCC 9.4 flags remove all three effects at source: `-fprofile-update=single -fno-profile-values`
   (test TU: 673 -> 214 instructions, 2048 -> 772 bytes; plain is 88 / 220).
2. The collected profile is start-up only (6.1 % of functions, 5.0 % of arcs; layout/generic 4 of 8 548 functions) and
   two of its eleven runs lost part of their dump. Do not ship anything built from it.
3. -Os + PGO cannot speed anything up: GCC 9 gates every speed transformation on `optimize_size` (verified in source and
   by experiment: a hot loop stays byte-identical at -Os + profile). The real plan is -O2 in BOTH phases; the profile then
   shrinks untrained code back to -Os-like size and puts it in `.text.unlikely`, which gold groups.
4. Use UXP's own PGO machinery (`MOZ_PROFILE_GENERATE=1` / `MOZ_PROFILE_USE=1`, one mozconfig): it keeps the PGO flags out
   of configure (the js/src `config.status` trap cannot happen), compiles gen objects as `*.i_o` (no stale objects), and
   force-relinks. Relocate the device output with `GCOV_PREFIX`, not `-fprofile-generate=<path>`.
5. Expect -10..-25 % on CPU-bound page work against today's -Os build (most of it the -O2 the owner already measured,
   PGO adding 5-10 points), a cold start that is more likely to improve than regress (code locality), and libxul text
   +5..+20 % instead of -O2's +42 %. Main risks: untrained-but-hot paths become -Os-like + cold-placed; the two-build
   cycle costs ~1.5 h of host time plus a 1-2 h device session.

## 1. Causes of the slowdown, ranked by share

Evidence base: `objects/jsarray-{shipped,instrumented}.o` (disassembled with `-d -r`, saved as
`fable-pgo/jsarray-{ship,instr}.dis`), the two stripped libxul files, the toolchain's own `libgcov.a` and `ld-2.23.so`,
the daemon logs, and the test compiles in `fable-pgo/ctest/`.

### The numbers (jsarray.o, same source, same -Os)

| | shipped | instrumented | ratio |
|---|---|---|---|
| `.text` bytes (`size`) | 45 942 | 266 852 | 5.8x |
| `.data` / `.bss` | 1 480 / 0 | 50 916 / 46 584 | |
| instructions (objdump) | 18 853 | 102 863 | 5.5x |
| defined FUNC symbols | 308 | 667 | 2.2x |
| the 272 functions present in both, total bytes | 43 668 | 167 270 | 3.83x |
| functions only in the instrumented object (were inlined before) | - | 395 (112 408 bytes) | |
| `ldrexd`/`strexd` pairs | 0 | 5 682 | |
| `bl __tls_get_addr` (R_ARM_THM_CALL) / R_ARM_TLS_GD32 `__gcov_indirect_call` | 0 | 557 / 557 | |
| `bl __gcov_indirect_call_profiler_v3` | 0 | 545 | |
| R_ARM_GOT32 `__gcov_time_profiler_counter` | 0 | 608 | |
| `bl __gcov_{one_value,ior,average}_profiler_atomic` | 0 | 61 | |
| relocations | 155 818 | 174 419 | |

Of the 667 instrumented functions, 598 carry one or more entry probes (514 one, 15 two, 3 more); the 69 without are
functions GCC proved never called indirectly (`only_called_directly_p`, tree-profile.c:432).

Two functions side by side (`fable-pgo/jsarray-*.dis`):

- `JS::GCVector<jsid,8,TempAllocPolicy>::~GCVector()`: shipped 22 bytes (10 instructions, `push; ldr; cmp; beq; bl free; pop`).
  Instrumented 512 bytes: `bl __tls_get_addr` -> load `.callee` -> `cbz` -> `bl __gcov_indirect_call_profiler_v3`;
  `ldrexd/adds/adc/strexd/cmp/bne` for the entry arc; time profiler (`ldrd` counter, test, atomic increment of
  `__gcov_time_profiler_counter`, `strd`); then the SAME probe sequence a second time for the inlined `Vector::~Vector`
  body; and `beginNoCheck.isra.0`, inlined in the shipped build, is now an out-of-line `bl`.
- `GetElement<unsigned>(...)`: 20 -> 148 bytes; no IC probe (static), but three atomic arc updates and the time profiler.

### Ranked

**1. The per-function-entry probe: `__tls_get_addr` + indirect-call check + time profiler. Hot path. Largest share
(my estimate: 40-50 % of the CPU slowdown).**
With `-fprofile-values` (on by default under `-fprofile-generate`, opts.c:2561), `tree_profiling` inserts at the entry of
every function that might be called indirectly (tree-profile.c:795-797 -> `gimple_gen_ic_func_profiler`, :426-491):
`if (__gcov_indirect_call.callee != NULL) __gcov_indirect_call_profiler_v3(id, &fn)`. `__gcov_indirect_call` is a
`__thread` variable in libgcov (libgcov-profiler.c:306-309) with default visibility; in a -fPIC shared library GCC uses
the global-dynamic TLS model, i.e. a call to `__tls_get_addr` through the PLT (557 R_ARM_TLS_GD32 + 557 calls in
jsarray). libxul does not even define it: its dynsym has `__gcov_indirect_call` **UND** (TLS), bound at load to whichever
instrumented DSO (libnspr4, liblgpllibs, ...) the loader finds first, so every probe is a cross-module TLS lookup. The fast
path of the bundled loader's `__tls_get_addr` (sysroot `ld-2.23.so`, disassembled) is 17 instructions including an
`mrc p15,0,r,c13,c0,3` CP15 read and a chain of five dependent loads, plus the 3-instruction PLT stub, the GOT address
computation and the call/return: ~25 instructions and ~40-60 cycles per function entry on a Scorpion, before the function
has done anything. The time profiler adds a load/compare per entry (and an atomic add on first execution). Inlined
bodies carry their own probe (the destructor above has two). In GCC 9 the time profiler's result (`tp_first_run`) is only
consumed by `lto-partition.c` (and `predict.c:3553` for missing-profile repair); without LTO it buys nothing.

**2. Lost inlining under -Os + instrumentation. Hot path (~20-30 %).**
Instrumentation runs before IPA inlining, so the inliner sees bodies 3-6x their size. At -Os `cgraph_edge::maybe_hot_p`
returns false for every edge (cgraph.c:2783 `if (opt_for_fn (caller->decl, optimize_size)) return false;`), so
`want_inline_small_function_p` inlines only when the code does not grow (ipa-inline.c:851-854), and the early inliner
likewise (ipa-inline.c:647). Result: 395 formerly-inlined functions are out-of-line in jsarray (112 KB), each paying a
call, a prologue and its own probe. This is also why the function count doubled and why `.data` (one 76-byte
`gcov_fn_info` per function) and the relocations exploded in libxul.

**3. Atomic 64-bit arc counters. Hot path (~15-25 %).**
Mozilla passes `-pthread`; the GCC driver then adds `-fprofile-update=prefer-atomic` (gcc.c:1162-1163), resolved to
`atomic` because ARMv7 has `ldrexd/strexd` (tree-profile.c:729-731; arm.h `TARGET_HAVE_LDREXD`). Each arc is
`__atomic_fetch_add(counter, 1, RELAXED)` (tree-profile.c:253-263): a 6-instruction exclusive-monitor loop, 5 682 of
them in jsarray (8.5 per function), against `ldrd/adds/adc/strd` for `single`. gcov counters are 64-bit on ARM32
(`LONG_LONG_TYPE_SIZE > 32`), hence the `adds/adc` pairs even in the non-atomic form.

**4. Code size x5.4: I-cache, I-TLB and page-ins. Hot path and start-up (~10-20 % of the warm slowdown; most of the
extra cold-start time).** libxul `.text` 113 555 176 against 20 890 148 bytes. The Scorpion's 32 KB L1I / 256 KB L2 see a
5x larger hot working set, and every code page touched at start-up has to come through the encrypted app filesystem.
The shipped engine's cold-warm gap is ~2 s; the instrumented engine's is 12-23 s (50.9 / 39.6 s cold, 27.9 s warm).

**5. Load-time work: relocations, dirty data, constructors. Start-up only (~2-5 s cold, under 1 s warm).**
`.rel.dyn` 21 868 104 bytes (2 955 120 shipped) holding RELCOUNT 2 722 053 R_ARM_RELATIVE (361 799 shipped): the
`gcov_fn_info` key and counter pointers and the per-TU function tables. Applying them dirties the whole 37 102 144-byte
`.data` (273 464 shipped) plus a 582 144-byte `.got`: ~9 300 copy-on-write page faults. The RW LOAD segment is 39.6 MB in
the file and 67.9 MB in memory (2.2 / 2.4 MB shipped). `.init_array` has 2 026 entries (121 shipped): one `__gcov_init`
per TU, each a list prepend (libgcov-driver.c:656-657), trivial in CPU but one more page touched each.

**6. Memory on a 1 GB device that swaps. Start-up and everything after.** The instrumented libxul adds ~37 MB of dirty
private data at load plus up to 28 MB of counters (`.bss`) as they are touched, and a 5x larger file-backed text set. The
daemon stood at 118 MB RSS at engine-up with no page open (pgo-rows: 118 328 kB); the shipped rows only record RSS after
a page load (263-305 MB), so the start-up delta is not measurable from these rows. Under pressure the kernel evicts code
pages and re-reads them through cryptofs, which is consistent with the 11 s spread between the two cold starts.

**7. The exit: 2 172 gcda files merged onto FAT, and dumps cut short.** Each exit opens, reads, merges and rewrites every
TU's file (libgcov-driver.c `gcov_do_dump` -> `merge_one_data`), 64 MB of data (119 MB on the FAT partition) in one flat
directory on `/media/internal`. The per-directory `runs` in the pulled profile are **not equal**: js/src, dom, layout, gfx,
toolkit show 11; xpcom, netwerk/base, modules, db, memory, mozglue, chrome, intl, caps, config/external/{icu,nspr} show
9; netwerk/protocol, netwerk/build 10. The dump walks the list from the last-registered TU (`__gcov_init` prepends), so the
TUs linked first are written last and are the ones lost when the process is killed mid-dump: at least two of the eleven
exits were cut (the stop script's `W=300` then `kill -9`, or the start script's own hard 30 s + `kill -9`).

**8. Why every page row timed out (procedure, not the compiler).** `tools/device/t502/gpu-spike-start.sh` sleeps 6 s after
starting the daemon and then launches the card; with the instrumented engine "engine up" came 29-51 s later. The smoke log
shows the card connect, then `inject: no page`, then `tick: no card`: the card came and went before the engine could serve
it, no page ever loaded, `idle.sh` waited its 90 s for `load done` in vain. The profile therefore contains start-up only.

## 2. How useful is the profile we have

`fable-pgo/tools/gcda-stats.py` ran the cross `gcov-dump -l` over all 2 172 files (`fable-pgo/gcda-stats.txt`):

| area | functions | with any nonzero arc | arcs nonzero | arc-count sum |
|---|---|---|---|---|
| **total** | 543 852 | 33 110 (6.1 %) | 5.0 % | 1.45 G |
| js/src | 81 616 | 16 429 (20.1 %) | 19.5 % | 1.21 G (84 % of all counts) |
| xpcom | 8 124 | 37.5 % | 30.2 % | 89 M |
| js/xpconnect | 4 730 | 31.7 % | | 26 M |
| netwerk/base | 5 649 | 15.9 % | | 18 M |
| dom/base | 23 288 | 2.9 % | 2.2 % | 0.8 M |
| dom/bindings | 50 323 | 1.5 % | 1.3 % | 0.8 M |
| layout/style | 9 830 | 4.7 % | | 0.6 M (UA sheets at start-up) |
| gfx/thebes | 6 231 | 17.8 % | | 0.3 M (font init) |
| gfx/layers | 14 626 | 1.1 % | 0.7 % | 64 K |
| image | 3 542 | 3.7 % | | 10 K |
| parser | 2 631 | 2.1 % | | 79 K |
| layout/base | 9 204 | 0.6 % | | 9 K |
| **layout/generic** | 8 548 | **4 (0.05 %)** | 8 arcs | 220 |
| dom/html | 13 742 | 0.3 % | | 15 K |
| dom/canvas, gfx/skia, gfx/angle, dom/media, media/*, security/nss, accessible | ~90 000 | 0-0.3 % | | ~0 |

So: a decent **start-up** profile (the XPCOM/JS component load, the add-on manager, prefs, font and UA-sheet init), and
nothing for content: no layout, no painting, no image decoding, no HTML parsing to speak of, no network protocol work.
Combined with (a) the unequal `runs` and (b) the -Os use phase (section 3.1: at -Os the profile can only move code
around, not speed it up), a use build from this profile is worth at most one measurement: its cold-start row, to see the
locality effect of `.text.unlikely`/`.text.hot` grouping on a start-up-trained libxul. It must not be shipped and should
not be the basis of the -Os-vs--O2 decision. The third use build now running will yield exactly that object.

## 3. The best PGO plan for GCC 9.4

### 3.1 The decisions, and why

**-O2 in both phases.** `optimize_function_for_size_p` is true for every function when `optimize_size` is set
(predict.c:270-276, cgraph.h:3222-3230), `maybe_hot_p` is false for every edge (cgraph.c:2783), and
`optimize_bb_for_speed_p`/`optimize_loop_for_speed_p` derive from them. So under -Os, `-fprofile-use`'s unrolling,
peeling, tracer, vectorization, hot-call inlining and IPA-CP cloning never fire; what remains is block ordering by
probability and whole-function placement. Verified by experiment (`fable-pgo/ctest/e2e-*/mm2`): the same profile applied
at -Os leaves the hot loop at 26 bytes / 11 instructions, byte-identical to plain -Os, while at -O2 it becomes 204 bytes
(unrolled + vectorized) and the never-executed functions become 22 and 16 bytes, exactly their -Os sizes, in
`.text.unlikely.*`. That second half is the key to the owner's size worry: with a profile, every function whose entry
count is zero is `NODE_FREQUENCY_UNLIKELY_EXECUTED` (predict.c:3963-3966) and therefore `optimize_for_size_p`, i.e.
compiled the way the shipped build compiles everything, with its local branch estimates preserved
(profile.c:709-714 "If function was not trained, preserve local estimates"). -O2 + PGO is thus "-Os for what the
training never ran, -O2 + feedback for what it did". The owner already measured plain -O2 (jsbench strings -55 %, dom -21 %,
loop -13 %, longest main-thread call -16 %, text +42 %); PGO at -O2 keeps most of that speed and takes back most of the size.
Gen and use must be the same level anyway: the profile is keyed by a CFG checksum taken after early inlining, which -Os
and -O2 do differently (ipa-inline.c:647 gates early inlining on `maybe_hot_p`); the tiny test did not mismatch, the real
tree would in many functions, and the use mozconfig's `-Wno-coverage-mismatch` would hide every one of them.

**Whole-tree instrumentation**, with the two cheap flags, rather than hot directories only. Partial instrumentation
(`-fprofile-filter-files`/`-fprofile-exclude-files`, both in GCC 9.4, verified) is safe for the use phase: an
uninstrumented function is skipped by `tree_profiling` in both phases, keeps `PROFILE_ABSENT`, and is compiled as a normal
function (`compute_function_frequency`, predict.c:3937-3961; the e2e test's `plain.cpp` gets only
`-Wmissing-profile: profile count data file not found`). But "normal" at -O2 means full -O2 size for everything excluded:
media/, skia, angle, icu, nss (~250 K functions) would not get the "untrained = -Os-like" shrink. Whole-tree keeps the
size story. Fallback if the gen engine is still too slow after step 1: exclude the bulk that no training will touch and
accept its -O2 size, or give those directories `-Os` in their moz.build.

**Keep value profiling off** (`-fno-profile-values`). What is lost: profile-driven indirect-call promotion and the
div/mod/stringop specializations. What is kept: everything else PGO does. Note one side effect found in opts.c:2533-2535:
with `flag_value_profile_transformations` on, `-fprofile-use` turns `-fdevirtualize-speculatively` OFF (it assumes IC
profiling replaces it); with no IC profile we want the static one back, so pass `-fdevirtualize-speculatively` explicitly.
If the owner wants IC profiling after all, option B in 3.8 keeps it at a third of the cost.

### 3.2 Use UXP's PGO machinery instead of CFLAGS in the mozconfig

The tree already has it (verified in `third_party/uxp`):
- `old-configure.in:4203-4222` and `js/src/old-configure.in:1943-...`: `PROFILE_GEN_CFLAGS=-fprofile-generate`,
  `PROFILE_GEN_LDFLAGS=-fprofile-generate`, `PROFILE_USE_CFLAGS="-fprofile-use -fprofile-correction -Wcoverage-mismatch"`,
  `PROFILE_USE_LDFLAGS=-fprofile-use` (configure substs, set whenever the compiler accepts `-fprofile-generate`).
- `config/config.mk:197-215`: added to `OS_CFLAGS/OS_CXXFLAGS/OS_LDFLAGS` at MAKE time when the make variable
  `MOZ_PROFILE_GENERATE` or `MOZ_PROFILE_USE` is set, skipping directories with `NO_PGO = True` (moz.build ->
  `NO_PROFILE_GUIDED_OPTIMIZE`; already set for mozjemalloc, elfhack, xptcall, nss, js/src/shell, sctp, libaom).
- `config/config.mk:555-560`: in the gen phase with GNU_CC the object suffix becomes `.i_o`, so gen and use objects never
  collide and the use phase rebuilds everything from scratch without a clobber.
- `config/rules.mk:571-576`: programs and libraries are `FORCE`-relinked in both phases; `rules.mk:559-569`: the gen phase's
  `export` tier deletes stale `*.gcda` in each directory.
- `client.mk:228-244` (`profiledbuild`) shows the intended order: gen build, profile run, `maybe_clobber_profiledbuild`
  (= `make clean` for GCC, `Makefile.in:332-333`), use build. We replace only the profile run.

Because the PGO flags never pass through configure, the js/src `config.status` trap (section 5) cannot occur, and one
mozconfig serves both phases. `mach build` inherits the environment, so `MOZ_PROFILE_GENERATE=1 ./mach build` works
(`config.mk` tests `ifdef`, which sees environment variables).

### 3.3 The mozconfig: `build/webos-oe/mozconfig.goanna-arm-pgo` (one file, both phases)

Copy of `mozconfig.goanna-arm` with exactly these changes:

```
mk_add_options MOZ_OBJDIR=/out/obj-jihad-goanna-arm-pgo
ac_add_options --enable-optimize=-O2
# phase-invariant PGO options (identical in gen and use, so config.status is identical in both):
#  -fprofile-update=single       arc counters as ldrd/adds/adc/strd, no ldrexd/strexd loops (gen only; inert in use)
#  -fno-profile-values           no indirect-call/time/value profilers: no __tls_get_addr per function entry (both phases)
#  -fdevirtualize-speculatively  -fprofile-use switches it off when value transforms are on; we want the static one
#  -Wno-error=coverage-mismatch  a mismatching function loses its profile with a warning instead of failing the build
export CFLAGS="$CFLAGS -fprofile-update=single -fno-profile-values -fdevirtualize-speculatively -Wno-error=coverage-mismatch"
export CXXFLAGS="$CXXFLAGS -fprofile-update=single -fno-profile-values -fdevirtualize-speculatively -Wno-error=coverage-mismatch"
```

No `-fprofile-generate=...` / `-fprofile-use=...` in the mozconfig; no `-Wno-missing-profile`; no `-Wno-coverage-mismatch`.
(`MOZ_PGO_OPTIMIZE_FLAGS=-O3` in old-configure.in:821 is ignored because `--enable-optimize=<flags>` sets
`MOZ_OPTIMIZE=2`, config.mk:265.) Delete `mozconfig.goanna-arm-pgo-gen` and `-pgo-use`.

### 3.4 Build commands

1. Gen: `MOZ_PROFILE_GENERATE=1 build/webos-oe/build-goanna-arm.sh all "" mozconfig.goanna-arm-pgo` (the script must
   forward the variable into the container: `in-container.sh` forwards only a fixed list, so add `MOZ_PROFILE_GENERATE`
   and `MOZ_PROFILE_USE` to it, or set them inside build-goanna-arm.sh from a 4th argument `gen|use`). Objects come out as
   `*.i_o`; `dist/bin/*.so` are instrumented.
2. Profile on the device (3.5), pull (3.6).
3. Use: `MOZ_PROFILE_USE=1 build/webos-oe/build-goanna-arm.sh build "" mozconfig.goanna-arm-pgo` on the same objdir. If
   `configure` is re-run it produces the same config.status (same flags), so it is harmless. Nothing has to be deleted
   between the phases; the `.i_o` objects stay (and let a later gen rebuild be incremental).
4. Grep the use log and record: `grep -c 'does not match its profile data'`, `grep -c 'not found in profile data'`,
   `grep -c 'profile count data file not found'`. The first two should be near zero; the third names TUs that were built
   but never dumped (and the `NO_PGO` directories).

### 3.5 The device side

- **Where the counters go.** With plain `-fprofile-generate` the path compiled into each object is the objdir path
  (`/out/obj-jihad-goanna-arm-pgo/js/src/Unified_cpp_js_src19.gcda`, coverage.c:1221-1232: no mangling without a
  `-fprofile-dir`). On the device set `GCOV_PREFIX=<dir> GCOV_PREFIX_STRIP=0` in the daemon's environment (EXTRA_ENV
  words); libgcov then writes `<dir>/out/obj-jihad-goanna-arm-pgo/js/src/Unified_cpp_js_src19.gcda`, creating the tree
  (libgcov-driver-system.c:232-251, 317). Verified under qemu-arm. Put `<dir>` on a non-FAT volume with room: the app's own
  directory on `/media/cryptofs` (ext3 behind the crypto layer, 182 MB of libraries already live there) is the natural
  place; the tree layout also ends the 2 172-entries-in-one-FAT-directory scans. The files shrink too: no value or time
  counters.
- **Dump without dying.** libxul exports `__gcov_flush`, `__gcov_dump`, `__gcov_reset` (dynsym, `@@xul6`); `__gcov_flush`
  dumps every instrumented DSO (it walks `__gcov_master.root`, which is one process-wide list because `__gcov_master` has
  default visibility and is interposed to a single copy) and resets the counters. A ten-line daemon addition: on an inject
  command (e.g. `pgo-dump`) or SIGUSR2 handled from the main loop (not inside the signal handler: libgcov is not
  async-signal-safe), `dlsym(RTLD_DEFAULT, "__gcov_flush")` and call it when non-null. Then one daemon lifetime covers the
  whole workload, the counters are written while the process is idle, and neither the 8 s teardown (`Main.cpp:518-523`,
  `JIHAD_TEARDOWN_SEC`) nor the stop script's SIGKILL can lose them. Keep `JIHAD_TEARDOWN_SEC=300` as belt and braces for
  the final exit.
- **Fix the start script for a slow engine.** In `gpu-spike-start.sh` replace `sleep 6` before the card launch with a poll
  for `engine up` in `$LOG` (cap 240 s), and scale the 45 s content-frame wait. In `lowpower-bench.sh` scale `idle.sh`'s
  90 s, `title.sh`'s 150 s and the `dev-sh.sh -t` ceilings by the measured slowdown of the new gen engine (expect 2-3x after
  3.1, not 9x). Make `gpu-spike-start.sh`'s own kill loop honour `/tmp/spike-stop-wait` like the stop script does.
- **Workload.** Everything the kits measure, each at least twice in one daemon life: desktop Wikipedia load + rest + pan,
  the long page's pan, jsbench, the colours page, the lazy-images page, the image-decode page, apple.com / animate.style
  (R4/R5), a YouTube H.264 clip (video is otherwise untrained and would be compiled cold), a form and a zoom. Start-up is
  profiled anyway on every daemon start; keep its share modest (GCC's hot threshold is per TU, `sum_max / 10000`, so a
  start-up-dominated js/src profile would make content JS look cold). Then one `pgo-dump`, then the exit.
- **Several runs / devices.** libgcov merges into existing files on every dump, so repeated sessions into the same
  `GCOV_PREFIX` add up. Separately collected trees merge on the host with
  `arm-webos-linux-gnueabi-gcov-tool merge -o <out> <treeA> <treeB>` (exists; tested). A stock-3.0.5 session, when that
  device arrives, can simply be merged in.

### 3.6 Pull and verify

`tar` the `<GCOV_PREFIX>/out/obj-jihad-goanna-arm-pgo` subtree and extract it over
`build/webos-oe/out-arm/` (so the `.gcda` land beside the `.i_o` objects: that is where `-fprofile-use` with no path looks,
`<cwd>/<auxbase>.gcda`; the `.i_o` suffix is stripped by the driver's `-auxbase-strip`, gcc.c:1151, so gen and use agree
on the name). Before the use build run `fable-pgo/tools/gcda-stats.py` (adapt the prefix) and check: `runs` identical for
every directory (a cut dump shows up as a lower number, as it did this time), and nonzero-function shares in layout/,
gfx/layers, image/, parser/, dom/html that are no longer ~0.

### 3.7 Measure

As §7 step 5 says: the PGO libxul in a copied directory against a copied shipped libxul, interleaved, the bench's rows,
plus libxul size and the daemon's RSS at engine-up (add it to the `start` row; it is the number this report could not get).

### 3.8 Options and fallbacks

- **B: keep indirect-call profiling cheaply.** Instead of `-fno-profile-values`, pass `-ftls-model=initial-exec` in the
  gen phase: `decl_default_tls_model` raises GD to IE (varasm.c), the probe becomes GOT load + `mrc` + add, no call
  (test: `__tls_get_addr` 10 -> 0, IC probes kept). Cost: the `callee != NULL` check and a `__gcov_indirect_call_profiler_v3`
  call after every indirect call remain (a lot in a virtual-call-heavy engine), the time profiler remains (useless in
  GCC 9 without LTO), text stays ~8x. IE TLS in the dlopen'ed NSS modules (libsoftokn3, libfreeblpriv3, libnssckbi) needs
  static-TLS surplus for 8 bytes each; glibc 2.23 reserves 1 664 bytes, so it fits, but it is a change to how the engine's
  own TLS (40 bytes in libxul) is addressed too. Only worth it if the owner wants speculative devirtualization from real
  call targets; expected extra gain low single digits.
- **Size knobs if the -O2 + PGO libxul is too big:** `-fno-unroll-loops -fno-peel-loops` (both enabled by `-fprofile-use`,
  opts.c:1699-1702) in the use phase, then `-fno-tracer`. Each costs a little hot-loop speed; vectorization is worth
  keeping (NEON, cost model dynamic).
- **Partial instrumentation** (if the gen engine is still too slow): `-fprofile-exclude-files='/src/uxp/(media|security/nss|accessible|gfx/(skia|angle|cairo|harfbuzz)|dom/media)/|config/external/(icu|nss|nspr|ffi)'`
  as a phase-invariant flag. The regex is matched against the source path as the compiler sees it: unified files include
  by absolute `/src/uxp/...` path (the link errors show `/src/uxp/js/src/builtin/Stream.cpp:94`), headers from
  `dist/include` appear relative to the compile directory, so a `filter-files` whitelist would silently skip header-defined
  templates; prefer `exclude-files`. Mixed instrumented/uninstrumented objects link and run fine (e2e test). Size caveat
  as in 3.1.
- **If the owner nonetheless wants -Os + PGO:** same procedure with `--enable-optimize=-Os`; expect layout and start-up
  effects only (hot/cold grouping, branch layout), no speed-up of hot code, and no change of untrained code.
- **`-fprofile-correction`** is already in `PROFILE_USE_CFLAGS` and required with `single`.
- **gold, `--icf=safe`, `-ffunction-sections`**: compatible. GCC emits `.text.unlikely.<fn>` / `.text.hot.<fn>` /
  `.text.startup.<fn>` under `-freorder-functions` (on at -O2 and -Os, varasm.c:572-614) and gold 1.16's `--text-reorder`
  (default) groups them: in the test link the two never-executed functions land first, the three hot ones next, the rest
  after. ICF folds fewer functions in a PGO build (hot and cold copies differ); negligible. `-freorder-blocks-and-partition`
  is **not available on ARM** ("not supported on this architecture", EHABI unwinding; opts.c:995-1041), so cold blocks
  inside hot functions are not split out; only whole functions move.

### 3.9 GCC 9.4 support matrix (verified with the cross compiler)

| option | 9.4 | note |
|---|---|---|
| `-fprofile-update=single\|atomic\|prefer-atomic` | yes | common.opt:2099; driver adds prefer-atomic with `-pthread` |
| `-fno-profile-values` | yes | removes IC, time and value profilers (test: all three counts 0) |
| `-fprofile-filter-files=` / `-fprofile-exclude-files=` | yes | regex on the source path as seen by the compiler |
| `-fprofile-correction`, `-Wno-error=coverage-mismatch`, `-Wmissing-profile` | yes | |
| `-ftls-model=initial-exec` on the gcov TLS tuple | yes | TLS_IE32 relocs, no `__tls_get_addr` |
| `-fprofile-reorder-functions` | accepted, no effect without LTO | only lto-partition.c orders by `tp_first_run` |
| `-freorder-blocks-and-partition` | rejected on ARM | note from cc1plus |
| `-fprofile-partial-training` | **no** (GCC 10) | "unrecognized command line option" |
| `-fprofile-prefix-path=` | **no** (GCC 11) | not needed: `GCOV_PREFIX` does the relocation |
| `GCOV_PREFIX` / `GCOV_PREFIX_STRIP` | yes (libgcov) | verified under qemu-arm |
| `gcov-dump`, `gcov-tool merge` | yes | in the toolchain's bin |

### 3.10 A toolchain upgrade (crosstool-NG, GCC 10-12, same glibc 2.23 sysroot) as a separate option

What it would add for PGO: `-fprofile-partial-training` (GCC 10: untrained code compiled as without profile, i.e. FULL
-O2, the opposite of the size behaviour this project wants, so it is mainly a correctness-of-performance tool for
untrained hot paths), `.text.sorted.*` function ordering by first execution (GCC 10, real start-up locality without LTO,
needs a linker that sorts them; gold's handling would have to be checked), `-fprofile-prefix-path` (GCC 11), TOPN value
counters that make IC profiling cheaper (GCC 10), and generally better ARM code. Cost: a toolchain rebuild (glibc 2.23
under GCC >= 10 needs `-fcommon`/patches), a new bundled libstdc++, re-validation of every trap this project has recorded
around the bundled glibc, the softfp/Thumb interworking and the Flash plugin, and a day or two of work. Recommendation:
not for this task; GCC 9.4 can do everything above. Revisit when the engine is otherwise stable.

## 4. Expected gain and the risks

**Gain (estimate; only the bench decides).** CPU-bound page work (jsbench, Wikipedia's main-thread sum, paint sums):
-10..-25 % against the shipped -Os build, of which the move to -O2 is the bulk the owner already measured and PGO adds
5-10 points through hot-path inlining, layout and loop work in the interpreter, GC, parser, style, layout and painting
code (JIT-emitted code is untouched by PGO; only the compilers, GC and runtime around it benefit). Cold start: with
start-up in the training set the start-up code is packed into `.text.hot`/`.text.startup` and everything untrained into
`.text.unlikely`, so fewer pages are touched; given that a copied directory alone costs +51 % and plain -O2 +60 % (confounded),
expect -10..+10 %, more likely a gain than a loss. Warm start: roughly neutral. libxul `.text`: +5..+20 % over -Os
(the trained 10-20 % of functions at -O2 + unrolling; the rest -Os-like), against +42 % for plain -O2. RSS: +1-3 %.

**Risks.**
- Untrained-but-hot code is compiled for size and placed cold (video, canvas/WebGL, Flash, forms, IndexedDB, WebSockets,
  editing, printing...). That is today's -Os speed plus a far branch or page, not a regression beyond today, but it
  must be in the training set if it matters. GCC 9 has no `-fprofile-partial-training` to opt out of it.
- `-fprofile-update=single`: counts lost or torn between threads (ARMv7 without LPAE does not make a 64-bit `ldrd/strd`
  single-copy atomic); `-fprofile-correction` smooths it; worst case one function's hotness is wrong. Correctness of the
  built code is not affected by the profile's content.
- Miscompilation under `-fprofile-use` is rare with GCC; Mozilla shipped GCC-PGO Linux builds for years with these flags.
  Test with the full page set, not only the bench.
- Build time: two full engine builds (~35 min each on this host, judging by the 33 min to the failed link) plus a 1-2 h
  device session; the use phase does not need a clobber.
- Device memory during profiling: even the cheap gen engine is ~3x the text and keeps counters; close other cards, watch
  for the USB drop.

## 5. What is wrong or wasteful in §7 and the scripts

1. **The js/src subconfigure never re-ran, so the "use" builds compiled the JS engine instrumented (the coordinator's
   finding; here is why).** `old-configure.in:5196` registers the js/src subconfigure with `AC_OUTPUT_SUBDIRS(js/src,$cache_file)`
   (hooks.m4:18-55 -> `subconfigure.py --prepare ... --cache-file=./config.cache`, the top-level autoconf-2.13 cache in the
   objdir). `build/subconfigure.py run()` re-runs configure only if: `config.status` is older than the `configure` script,
   or the configure ARGUMENTS changed, or `maybe_clear_cache()` found a "precious" variable (CFLAGS, CXXFLAGS, LDFLAGS...)
   changed. That last check reads `ac_cv_env_<VAR>_set`/`_value` entries from the cache file, which only autoconf 2.5x
   writes; Mozilla's old-configure is autoconf 2.13 and writes none, so the function `continue`s past every variable and
   returns False. A CFLAGS change that lives only in the environment (the mozconfig's `export CFLAGS`) is therefore
   invisible to the skip logic; `js/src/config.status` kept the gen flags, `reader.py:1137-1140` builds js/src's build
   backend from that file, and libjs_static.a came out instrumented. (Unverified: that `<objdir>/config.cache` exists; if
   it did not, `cleared_cache` would be True and configure would have re-run. The outcome says it exists.) **What a
   gen->use switch must clear in a UXP objdir when the flags come from the mozconfig:** `config.cache`,
   `js/src/config.status`, `js/src/config.cache` (if any), every `*.o`, `*.a`, `*.so` and `dist/bin/*.so`, and any `*.gcda`
   pulled into the tree, then `mach configure` + `mach build`. **Or avoid it entirely** with 3.2/3.3: flags identical in both
   phases, `MOZ_PROFILE_GENERATE`/`MOZ_PROFILE_USE` at make time, `.i_o` objects.
2. `-Wno-coverage-mismatch -Wno-missing-profile` in the use mozconfig hide the only signals that say how much of the
   profile was consumed (`get_coverage_counts`, coverage.c:299-375: a mismatching or missing function silently loses its
   profile). Use `-Wno-error=coverage-mismatch` and count the warnings.
3. `-fprofile-generate=/media/internal/jihad-pgo` bakes a device path into every object and forces the `#`-mangled flat
   directory (coverage.c:1221-1232). `GCOV_PREFIX` at run time gives the same relocation with the objdir tree layout and
   objects identical to what UXP's own machinery produces.
4. The profile directory on FAT (`/media/internal`): one flat directory of 2 172 long-named files, read-modify-written
   on every exit; 64 MB became 119 MB; the dumps took minutes and two of eleven were cut (unequal `runs`). Non-FAT volume,
   tree layout, fewer and smaller files (3.1), and the `__gcov_flush` hook so the dump is never raced by a kill.
5. `gpu-spike-start.sh` launches the card 6 s after the daemon start; the gen engine needs 30-50 s, so no page ever
   loaded and the profile is start-up only. The bench's 90/150 s waits and `dev-sh.sh -t` ceilings are sized for the
   shipped engine. The start script's own kill loop ignores `/tmp/spike-stop-wait`.
6. §7's "GCC 9 names each .gcda after its object's absolute path with / as #" is true only because of the `=path` form.
7. §7's two "traps" are correctly described; the first one has a cause worth writing down: the driver appends `-lgcov`
   last, the instrumented shared libraries earlier on the link line export the libgcov profilers with default visibility,
   so the linker resolves libxul's references to them (and the TLS tuple) against those DSOs and never pulls the archive
   members. Shipping all 17 instrumented libraries is the right fix; `-Wl,-Bsymbolic` is not.
8. The procedure says "counters from threads race without atomic updates: the correction tolerates it" under step 4
   while step 1 builds WITH atomic updates; after 3.3 the sentence becomes true and necessary.
9. The 11-run profile already collected: the use build running now is only worth its cold-start row (section 2).

## 6. What was verified and what was not

Verified: every number in section 1 (readelf/objdump/nm/size on the given files; the toolchain's libgcov.a and
ld-2.23.so), the GCC 9 behaviour quoted with file:line from `releases/gcc-9`, the flag effects by compiling with the
project's own cross gcc, the gen -> run -> use cycle (GCOV_PREFIX layout, `.text.unlikely`/`.text.hot` placement, -Os-sized
cold functions, `-Wmissing-profile` text, gold grouping) under qemu-arm with the toolchain sysroot, the profile statistics
over all 2 172 gcda files, the UXP build-system machinery by reading `config/config.mk`, `config/rules.mk`, `client.mk`,
`Makefile.in`, `old-configure.in`, `build/subconfigure.py`, `build/autoconf/hooks.m4`, `python/mozbuild/mozbuild/frontend/reader.py`.

Not verified (no device, no objdir access): the shipped daemon's RSS at engine-up; cycle counts on the Scorpion (the
per-cause shares are estimates from instruction counts and the warm/cold timings); whether `<objdir>/config.cache` exists;
how large the CFG-checksum mismatch rate would be between an -Os gen and an -O2 use on the real tree (zero on the small
test, expected high on the tree); that `/media/cryptofs` is ext3 (it is a non-FAT, writable, roomy volume either way);
`gcov-tool merge`'s `runs` arithmetic across profiles from different builds (it printed runs=1 for two different-build
inputs; merge same-build trees only); how `mach`/`in-container.sh` forward `MOZ_PROFILE_*` (in-container.sh forwards a
fixed variable list; add the two names).

## Appendix: evidence files (all under `fable-pgo/`)

- `jsarray-ship.dis`, `jsarray-instr.dis`: full `objdump -d -r` of the two objects; `ship-funcs.txt`, `instr-funcs.txt`:
  per-function sizes.
- `gcda-stats.txt` (from `tools/gcda-stats.py`): per-directory functions/arcs hit, `runs`, hottest TUs.
- `ctest/`: the flag-set test compiles (`*.o`, `*.rdis`) and `ctest/e2e-*/`: the qemu-arm gen/run/use cycle (`prof/` tree,
  `use/hot.o` with `.text.hot/.text.unlikely`, `mm2/` the -Os->-O2 test, `use/libhot-gold.so` the gold ordering).
- `gcc9-src/`: the GCC 9 branch sources cited (opts.c, predict.c, cgraph.c/.h, tree-profile.c, value-prof.c, coverage.c,
  profile.c, varasm.c, gcc.c, common.opt, ipa-inline.c, bb-reorder.c, lto-partition.c, doc/invoke.texi,
  libgcov-{driver,driver-system,profiler,interface}.c, libgcov.h).
