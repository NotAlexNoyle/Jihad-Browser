# Jihad Browser — the packaging and adapter host, so the host system does not matter.
#
# Runs the steps whose tools depend on the host (cavekit-device-build.md R11):
#   - build-adapter-pdk.sh: the Palm PDK's gcc 4.3.3 is a 32-bit x86 program (libc6-i386, lib32z1);
#   - build-variant-ipk.sh / build-mochi-ipk.sh: palm-package is Java (webOS Archive's SDK, the
#     third_party/webos-sdk-redux submodule), plus ar, rsync, file and readelf;
#   - make-device-bundle.sh: readelf, sha256sum, python3.
# build/webos-oe/in-container.sh builds this image on first use and mounts the repository at its
# own path, so the scripts run unchanged.
FROM docker.io/library/ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        bash coreutils findutils grep sed gawk binutils file rsync \
        tar gzip bzip2 xz-utils curl ca-certificates git python3 \
        openjdk-11-jre-headless libc6-i386 lib32z1 \
    && rm -rf /var/lib/apt/lists/*
