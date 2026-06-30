# Copyright 2026 the Jihad Browser project.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Pinned build environment for the UXP / Goanna engine (Phase-1 desktop x86_64).
#
# This UXP revision (maintained master) requires, verified against the tree:
#   - Python 3.3+  (build/mach_bootstrap.py rejects Python 2; mach re-execs py3)
#   - GCC >= 9.1   (build/moz.configure/checks.configure FatalCheckError)
#   - autoconf 2.13 (configure is autoconf-generated into the source tree)
#   - yasm/nasm, GTK, dbus-glib, pulse, etc.
# UXP has NO Rust requirement.
#
# Ubuntu 20.04 (focal) is the smallest base that satisfies all of the above:
# default GCC 9.3 (>= 9.1), Python 3.8, autoconf2.13 2.13-68, yasm. (Ubuntu
# 18.04 was too old — GCC 7.5 — and its mach-era assumptions were wrong.)
#
# The engine source is NOT copied into the image — mount it at build time so the
# repo never vendors UXP (see cavekit-engine-embedding R4). Example:
#   podman build -t jihad-goanna-build build/desktop
#   mkdir -p build/desktop/out
#   podman run --rm --user 0 -e HOME=/out \
#     -v <abs>/Jihad/UXP:/src/uxp \
#     -v <abs>/Jihad/Jihad-Browser/build/desktop:/cfg:ro \
#     -v <abs>/Jihad/Jihad-Browser/build/desktop/out:/out \
#     jihad-goanna-build /cfg/build-goanna.sh all
#
# Base image is fully qualified so it resolves under podman without a configured
# unqualified-search registry. Focal is served from the default archive mirrors.
FROM docker.io/library/ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      python3 python3-dev python3-pip python3-setuptools python3-distutils \
      autoconf2.13 \
      yasm nasm \
      zip unzip \
      git \
      ccache pkg-config \
      libgtk2.0-dev libgtk-3-dev \
      libdbus-glib-1-dev \
      libpulse-dev \
      libgconf2-dev \
      libasound2-dev \
      uuid-dev \
      zlib1g-dev \
      ca-certificates \
      libxt-dev libx11-xcb-dev libxcb1-dev \
      libxcomposite-dev libxdamage-dev libxrender-dev libxext-dev \
      libxfixes-dev libxrandr-dev libxcursor-dev libxi-dev libxtst-dev \
      libxss-dev libxinerama-dev libxkbcommon-dev \
    && rm -rf /var/lib/apt/lists/*

# This UXP revision's mach requires Python 3; point `python` at python3.
RUN ln -sf /usr/bin/python3 /usr/bin/python

# Unprivileged build user (the build itself must not assume root).
RUN useradd -ms /bin/bash builder
USER builder
WORKDIR /home/builder

ENV MOZCONFIG=/cfg/mozconfig.goanna
CMD ["/bin/bash"]
