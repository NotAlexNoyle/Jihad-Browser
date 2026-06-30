# Toolchain notes

## Build host (Phase 1, desktop x86_64)

UXP/Goanna build host needs:
- Python **2.7** (the ESR-52-era build system is Python 2 only) + `mach`.
- **autoconf 2.13** (specifically; not 2.6x) for the configure step.
- A C++14-capable compiler (gcc ≥ 5 / clang ≥ 3.9) and matching libstdc++.
- ~25–40 GB free disk for an object dir; several GB RAM for linking libxul.
- yasm/nasm, zlib, pkg-config, GTK dev headers (for the desktop widget path),
  NSPR/NSS (can use in-tree copies).

UXP deliberately has **no Rust requirement** (unlike contemporaneous
mozilla-central), which simplifies the toolchain considerably.

### Use a pinned build container (recommended)

A modern host is the *wrong* environment in both directions: too new for `mach`
(it wants Python 2.7 + autoconf-2.13, not Python 3 / autoconf 2.7x) and too new
in compiler (GCC 14 / recent clang vs an ~2017 engine). Installing Python 2.7
and autoconf-2.13 on a bleeding-edge distro is itself painful (Python 2 is often
out of the repos).

Instead, build Goanna inside a pinned container with the era-correct baseline.
This repo provides one under `build/desktop/`:

- `Dockerfile` — Ubuntu 18.04 base (Python 2.7, autoconf2.13, GCC 7, yasm).
  Apt is repointed at `old-releases.ubuntu.com` since 18.04 is archived. The
  UXP source is **mounted**, never copied into the image (no vendoring).
- `mozconfig.goanna` — embedding-oriented config (xulrunner target, GTK2 +
  basic layers, trimmed subsystems) producing libxul for the render backend.
- `build-goanna.sh` — runs `mach configure`/`build` as an unprivileged user
  against the read-only source mount, writing artifacts to `/out`.

```
docker build -t jihad-goanna-build build/desktop
docker run --rm -it \
  -v <abs>/Jihad/UXP:/src/uxp:ro \
  -v <abs>/Jihad/Jihad-Browser/build/desktop:/cfg:ro \
  -v $PWD/out:/out \
  jihad-goanna-build /cfg/build-goanna.sh all
```

The same container is the basis for the Phase-2 ARM cross-build: add the
cross-toolchain (below) and a target mozconfig.

### This workstation (observed)

Void Linux host: Python 3.14, autoconf 2.72, GCC 14.2, rustc 1.96; no python2,
no autoconf-2.13, no yasm. ~507 GB free, 31 GB RAM. **Cannot build UXP directly**
— use the container above.

## Cross toolchain (Phase 2, webOS 3 ARMv7) — feasibility gate

The TouchPad ships **CodeSourcery gcc 4.4.x** (2011). That compiler **cannot**
build UXP (no C++14, old libstdc++/binutils). Options, roughly in order of
preference:

1. **OE/meta-toolchain cross-gcc** — have the `meta-webos` OE build produce a
   modern cross-gcc (gcc ≥ 6) targeting the TouchPad's glibc/kernel ABI, and
   build Goanna + the daemon inside bitbake. Highest integration, most setup.
2. **Standalone crosstool-NG toolchain** — build a gcc ≥ 6 cross-toolchain
   pinned to the TouchPad's glibc (≈2.x) and kernel headers (2.6.x), used by a
   CMake toolchain file. Decoupled from OE; good for iterating the engine alone.
3. **Linaro/CodeSourcery newer release** — a newer prebuilt ARM toolchain whose
   glibc/kernel baseline is compatible with webOS 3. Fastest if one matches.

Key compatibility risks to verify early:
- glibc symbol versioning: the engine must not require glibc newer than the
  device's. May need to build against the device sysroot.
- kernel headers (2.6.x) vs. APIs Goanna expects (epoll/futex are fine;
  newer syscalls are not).
- C++ runtime: ship a compatible `libstdc++` alongside, or static-link.
- ARMv7 NEON/VFP flags matching the APQ8060.

This milestone is intentionally separate from the engine-integration work; the
desktop PoC (Phase 1) does not depend on it.
