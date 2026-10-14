# Jihad Browser — crosstool-NG builder image for the webOS-3 ARM target.
#
# Produces a modern GCC (>=9.1, required by UXP/Goanna) ARMv7 SOFTFP cross
# toolchain whose glibc runs on the TouchPad's kernel 2.6.35.
#
# WHY crosstool-NG (and not a prebuilt / distro toolchain):
#   * Device is armel SOFTFP (loader /lib/ld-linux.so.3), glibc 2.8, kernel 2.6.35.
#   * glibc >= 2.26 hard-requires Linux >= 3.2 -> won't load on 2.6.35.
#   * Debian/Ubuntu build ALL their glibc with --enable-kernel=3.2 regardless of
#     version, so no prebuilt distro glibc runs here either.
#   * The last prebuilt *softfp* glibc toolchains (Linaro) top out at GCC 7.x < 9.1.
#   => We must build glibc ourselves with --enable-kernel <= 2.6.16 and a modern GCC.
#
# Base = Ubuntu 20.04 (same base as the desktop engine build image; GCC 9.3 host).
FROM docker.io/library/ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        gcc g++ make bison flex texinfo help2man gawk libtool-bin \
        libncurses-dev unzip wget curl rsync git bzip2 xz-utils \
        python3 file patch autoconf automake gperf ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# --- crosstool-NG -----------------------------------------------------------
# Not in the image: it is the submodule third_party/crosstool-ng (pinned at crosstool-ng-1.25.0),
# mounted at /ctng-src and built by build-toolchain.sh each run (a minute), so the image holds
# only the host packages and the pin lives in git (cavekit-device-build.md R11). build.sh beside
# this file does the mounts.

# ct-ng refuses to run as root; work as an unprivileged user. build.sh runs the container as the
# invoking host user, whose uid need not be 1000: the home is world-writable so that any uid can
# make ct-ng's working tree there (the toolchain's paths stay /home/builder/..., as always).
RUN useradd -m -u 1000 builder && chmod 0777 /home/builder
USER builder
WORKDIR /home/builder
