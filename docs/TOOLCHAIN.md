# Toolchain notes

## Build host (Phase 1, desktop x86_64)

UXP/Goanna build host needs:
- **Python 3.3+** — verified against this UXP revision: `build/mach_bootstrap.py`
  prints "Python 3.3 or above is required to run mach" and rejects Python 2.
  (The maintained UXP master has migrated `mach`/mozbuild to Python 3; the old
  "ESR-52 is Python 2 only" guidance does NOT apply here.) `mach` is a
  shell/python polyglot that re-execs under `python3`.
- **autoconf 2.13** (specifically; not 2.6x) — `configure` is still autoconf-
  generated from `configure.in` into the source tree at build time, so the UXP
  mount must be **read-write**.
- **GCC ≥ 9.1** — verified: `build/moz.configure` raises `FatalCheckError`
  ("Only GCC 9.1 or newer is supported") on older compilers. (This is why the
  container base is Ubuntu 20.04 / GCC 9.3, not 18.04 / GCC 7.5.)
- ~25–40 GB free disk for an object dir; several GB RAM for linking libxul.
- yasm/nasm, zlib, pkg-config, GTK dev headers, NSPR/NSS (in-tree copies).

UXP has **no Rust requirement** (unlike contemporaneous mozilla-central), which
simplifies the toolchain.

### Use a pinned build container (recommended)

A modern host is the *wrong* environment in both directions: too new for `mach`
(it wants Python 2.7 + autoconf-2.13, not Python 3 / autoconf 2.7x) and too new
in compiler (GCC 14 / recent clang vs an ~2017 engine). Installing Python 2.7
and autoconf-2.13 on a bleeding-edge distro is itself painful (Python 2 is often
out of the repos).

Instead, build Goanna inside a pinned container with the era-correct baseline.
This repo provides one under `build/desktop/`:

- `Dockerfile` — Ubuntu 20.04 base (Python 3.8, autoconf2.13 2.13-68, GCC 9.3,
  yasm), fully-qualified (`docker.io/library/ubuntu:20.04`) so it resolves under
  podman without a configured unqualified-search registry. 20.04 satisfies the
  GCC ≥ 9.1 and Python 3 requirements (18.04's GCC 7.5 does not). The UXP source
  is **mounted**, never copied into the image (no vendoring).
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

The TouchPad ships **CodeSourcery gcc 4.4.x** (2011), which cannot build UXP (no C++14, old
libstdc++/binutils).

**This was decided and built: option 2, a standalone crosstool-NG toolchain** — gcc 9.4, ARMv7-A +
NEON, **softfp**, living in `build/webos-oe/toolchain/`. It is what every device build in this repo
uses. The section that stood here weighed three options as if the choice were still open; it was
made in 2026-07 and the other two were not taken.

The compatibility constraints it had to satisfy, all of which shaped the result and are still live
concerns when touching the build:

- **glibc symbol versioning** — the device runs glibc 2.8, so the engine cannot require anything
  newer. Resolved by bundling glibc 2.23 with the app and launching through its own `ld-2.23.so`
  rather than by matching the device. That choice has consequences of its own; the cross-ABI
  hazards it creates are `context/kits/cavekit-device-build.md` R9.
- **kernel headers** — 2.6.35. epoll/futex are fine; newer syscalls are not.
- **ARMv7 NEON/VFP flags** matching the APQ8060, and softfp throughout — mixing float ABIs with
  the device's own libraries is not an option.
