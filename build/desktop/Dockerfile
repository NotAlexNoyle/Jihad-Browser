# Copyright 2026 the Jihad Browser project.
# Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Pinned build environment for the UXP / Goanna engine (Phase-1 desktop x86_64).
#
# UXP is an ESR-52-era codebase: its `mach` build system requires Python 2.7 and
# autoconf-2.13, and an era-appropriate GCC. Modern host distros (Python 3.x,
# autoconf 2.7x, GCC 14) cannot run it. Ubuntu 18.04 has the right baseline
# (Python 2.7, autoconf2.13, GCC 7, yasm) and is the documented Pale Moon/UXP
# build environment vintage.
#
# The engine source is NOT copied into the image — mount it at build time so the
# repo never vendors UXP (see cavekit-engine-embedding R4). Example:
#   docker build -t jihad-goanna-build build/desktop
#   docker run --rm -it \
#     -v /home/notalexnoyle/eclipse-workspace/Jihad/UXP:/src/uxp:ro \
#     -v $PWD/out:/out \
#     -v $PWD/build/desktop:/cfg:ro \
#     jihad-goanna-build /cfg/build-goanna.sh
FROM ubuntu:18.04

# 18.04 is archived: point apt at old-releases so the image still builds.
RUN sed -i -e 's|http://archive.ubuntu.com/ubuntu|http://old-releases.ubuntu.com/ubuntu|g' \
           -e 's|http://security.ubuntu.com/ubuntu|http://old-releases.ubuntu.com/ubuntu|g' \
           /etc/apt/sources.list

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      python2.7 python-pip \
      autoconf2.13 \
      yasm nasm \
      zip unzip \
      git mercurial \
      ccache pkg-config \
      libgtk2.0-dev libgtk-3-dev \
      libdbus-glib-1-dev \
      libpulse-dev \
      libgconf2-dev \
      libxt-dev \
      libasound2-dev \
      uuid-dev \
      zlib1g-dev \
      ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# mach expects `python2` on PATH.
RUN ln -sf /usr/bin/python2.7 /usr/bin/python2 && ln -sf /usr/bin/python2.7 /usr/bin/python

# Unprivileged build user (engine build must not run as root).
RUN useradd -ms /bin/bash builder
USER builder
WORKDIR /home/builder

# Source mounted at /src/uxp (ro); object dir + artifacts go to /out.
ENV MOZCONFIG=/cfg/mozconfig.goanna
CMD ["/bin/bash"]
