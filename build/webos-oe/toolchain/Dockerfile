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

# --- crosstool-NG (pinned) -------------------------------------------------
ARG CTNG_VER=1.25.0
RUN cd /tmp \
    && wget -q "http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-${CTNG_VER}.tar.xz" \
    && tar xf "crosstool-ng-${CTNG_VER}.tar.xz" \
    && cd "crosstool-ng-${CTNG_VER}" \
    && ./configure --prefix=/opt/ctng \
    && make -j"$(nproc)" \
    && make install \
    && rm -rf /tmp/crosstool-ng-*
ENV PATH=/opt/ctng/bin:${PATH}

# ct-ng refuses to run as root; work as an unprivileged user.
RUN useradd -m -u 1000 builder
USER builder
WORKDIR /home/builder
