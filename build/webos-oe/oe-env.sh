#!/bin/bash
# Copyright 2026 the Jihad Browser project. Apache-2.0.
#
# oe-env.sh — self-contained OpenEmbedded "dylan" (2013) / BitBake 1.18 build host.
#
# The full Open webOS (build-webos) bitbake of Jihad Browser needs a Python-2,
# Ubuntu-14.04-era GNU/Linux userland. This driver provides exactly that WITHOUT a
# container runtime: it downloads Ubuntu's official ubuntu-base rootfs tarball once,
# provisions the OE host tools into it, and enters it with plain POSIX `chroot`.
#
# WHY chroot (one clean solution that spans Linux):
#   * chroot exists on EVERY Linux and EVERY init system (runit/OpenRC/s6/systemd) —
#     no systemd, no docker/podman, no debootstrap, no distro packages.
#   * the rootfs carries its own glibc, so this runs identically on a glibc host or a
#     musl host (Alpine, musl-Void). Native speed (no ptrace).
#   * OE/bitbake is Linux-host-only by design (pseudo = LD_PRELOAD fakeroot); this is
#     the widest-reaching way to give it the Linux userland it requires.
# The one cost: chroot needs root. Privilege is taken via `sudo` OR `doas` (whichever
# the host has), or run directly if already root. Override with OE_ROOTCMD=...
#
# Usage:
#   build/webos-oe/oe-env.sh provision          # fetch + build the rootfs (idempotent)
#   build/webos-oe/oe-env.sh shell              # interactive bash inside (as builder)
#   build/webos-oe/oe-env.sh run <cmd...>       # run a command inside (as builder)
#   build/webos-oe/oe-env.sh root <cmd...>      # run as root inside (provisioning/debug)
#   build/webos-oe/oe-env.sh mcf  <mcf-args...> # ./mcf inside build-webos
#   build/webos-oe/oe-env.sh make <make-args...># make inside build-webos
#   build/webos-oe/oe-env.sh clean              # remove the rootfs (keeps downloads/sstate)
#
# Layout inside the chroot:
#   /oe/Jihad-Browser   <- this repo (bind)
#   /oe/build-webos     <- the build-webos orchestrator (bind)
#   /oe/downloads       <- persistent DL_DIR   (bind, survives `clean`)
#   /oe/sstate          <- persistent SSTATE   (bind, survives `clean`)
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"                 # .../build/webos-oe
REPO="$(cd "$HERE/../.." && pwd)"                      # Jihad-Browser
ROOTFS="${OE_ROOTFS:-$HERE/oe-rootfs}"
CACHE="$HERE/oe-cache"
STAMP="$ROOTFS/.oe-provisioned"

UBUNTU_URL="${UBUNTU_URL:-http://cdimage.ubuntu.com/ubuntu-base/releases/14.04/release/ubuntu-base-14.04.6-base-amd64.tar.gz}"
UBUNTU_TB="$CACHE/$(basename "$UBUNTU_URL")"

# build-webos orchestrator: reuse a sibling clone if present, else clone it pinned.
BUILD_WEBOS="${BUILD_WEBOS:-$REPO/../../build-webos}"
BUILD_WEBOS_URL="https://github.com/openwebos/build-webos.git"
BUILD_WEBOS_REV="37540e592d3419e94b054b0bd3fd2206405f0256"

# The Jihad OE layer, as seen from INSIDE the chroot (repo is bound at /oe/Jihad-Browser).
JIHAD_LAYER_CHROOT="/oe/Jihad-Browser/build/webos-oe"

# Real user's uid/gid — the builder user + bind-mounted files use these. Prefer SUDO_UID/GID
# so `sudo oe-env.sh ...` still targets the invoking user (not root) rather than uid 0.
HOST_UID="${SUDO_UID:-$(id -u)}"; HOST_GID="${SUDO_GID:-$(id -g)}"
MACHINE="${MACHINE:-tenderloin}"

# ---- privilege escalation: sudo | doas | direct-if-root ---------------------
if [ -n "${OE_ROOTCMD:-}" ]; then ROOT="$OE_ROOTCMD"
elif [ "$HOST_UID" -eq 0 ]; then   ROOT=""
elif command -v sudo >/dev/null 2>&1; then ROOT="sudo"
elif command -v doas >/dev/null 2>&1; then ROOT="doas"
else echo "!! need root: install sudo or doas, or run as root" >&2; exit 1; fi
asroot() { if [ -z "$ROOT" ]; then "$@"; else $ROOT "$@"; fi; }

# ---- mount bookkeeping (unmounted in reverse on any exit) --------------------
MOUNTS=()
cleanup() {
  local i
  for (( i=${#MOUNTS[@]}-1 ; i>=0 ; i-- )); do
    asroot umount -R -l "${MOUNTS[$i]}" 2>/dev/null || true
  done
}
trap cleanup EXIT INT TERM

# SAFETY: unmount everything currently mounted under $ROOTFS, deepest-first. The repo is
# bind-mounted INSIDE the rootfs, so a stale mount left by a crashed run would make a naive
# `rm -rf $ROOTFS` recurse THROUGH the bind and delete the real repo. Always run before rm.
umount_rootfs_tree() {
  local mp rp; rp="$(readlink -f "$ROOTFS" 2>/dev/null || echo "$ROOTFS")"
  [ -n "$rp" ] && [ "$rp" != "/" ] || return 0
  while read -r mp; do
    asroot umount -l "$mp" 2>/dev/null || true
  done < <(awk '{print $2}' /proc/mounts | grep -F "$rp" | awk '{print length, $0}' | sort -rn | cut -d' ' -f2-)
}

need() { command -v "$1" >/dev/null 2>&1 || { echo "!! missing host tool: $1" >&2; exit 1; }; }
say()  { echo ">> $*" >&2; }

# ---- rootfs bring-up ---------------------------------------------------------
fetch_tarball() {
  [ -f "$UBUNTU_TB" ] && return
  need curl; mkdir -p "$CACHE"
  say "fetching $(basename "$UBUNTU_TB") ..."
  curl -fL --retry 3 -o "$UBUNTU_TB.part" "$UBUNTU_URL"
  mv "$UBUNTU_TB.part" "$UBUNTU_TB"
}

extract_rootfs() {
  [ -e "$ROOTFS/etc/os-release" ] && return
  need tar
  fetch_tarball
  say "extracting rootfs -> $ROOTFS ..."
  umount_rootfs_tree            # SAFETY: never rm through a live bind of the repo
  asroot rm -rf "$ROOTFS"; mkdir -p "$ROOTFS"
  asroot tar -C "$ROOTFS" -xpf "$UBUNTU_TB"
}

write_resolv() { echo "nameserver 1.1.1.1" | asroot tee "$ROOTFS/etc/resolv.conf" >/dev/null; }

mount_kernel_fs() {
  for fs in proc sys dev; do asroot mkdir -p "$ROOTFS/$fs"; done
  asroot mount -t proc proc "$ROOTFS/proc"; MOUNTS+=("$ROOTFS/proc")
  asroot mount --rbind /sys "$ROOTFS/sys";  MOUNTS+=("$ROOTFS/sys")
  asroot mount --rbind /dev "$ROOTFS/dev";  MOUNTS+=("$ROOTFS/dev")
  write_resolv
}

bind_at() {  # host-src  chroot-relpath
  local src="$1" dst="$ROOTFS/$2"
  [ -d "$src" ] || { echo "!! bind source missing: $src" >&2; exit 1; }
  asroot mkdir -p "$dst"; asroot mount --bind "$src" "$dst"; MOUNTS+=("$dst")
}

mount_sources() {
  ensure_build_webos
  bind_at "$REPO"        "oe/Jihad-Browser"
  bind_at "$BUILD_WEBOS" "oe/build-webos"
  mkdir -p "$HERE/oe-downloads" "$HERE/oe-sstate"
  # bitbake writes DL_DIR/SSTATE as the builder (invoking uid); keep these user-owned even
  # when the script itself runs as root (single-prompt mode).
  [ "$(id -u)" -eq 0 ] && chown "$HOST_UID:$HOST_GID" "$HERE/oe-downloads" "$HERE/oe-sstate" 2>/dev/null || true
  bind_at "$HERE/oe-downloads" "oe/downloads"
  bind_at "$HERE/oe-sstate"    "oe/sstate"
}

ensure_build_webos() {
  if [ ! -d "$BUILD_WEBOS/.git" ]; then
    need git; say "cloning build-webos ($BUILD_WEBOS_REV) -> $BUILD_WEBOS ..."
    git clone "$BUILD_WEBOS_URL" "$BUILD_WEBOS"
    git -C "$BUILD_WEBOS" checkout -q "$BUILD_WEBOS_REV"
  fi
  # Install our vendored layer config (2013 "dylan" pins + TouchPad machines) so the
  # build is reproducible from THIS repo, not from a hand-edited sibling checkout.
  install -m 0644 "$HERE/weboslayers.py" "$BUILD_WEBOS/weboslayers.py"
  # Route downloads + sstate to the persistent binds (gitignored, under our tree, and
  # surviving `clean`). bblayers.conf `include`s ${TOPDIR}/webos-local.conf; mcf never
  # rewrites it. Chroot paths (bitbake runs inside).
  cat > "$BUILD_WEBOS/webos-local.conf" <<'EOF'
# Written by build/webos-oe/oe-env.sh — do not hand-edit (regenerated each run).
DL_DIR ?= "/oe/downloads"
SSTATE_DIR ?= "/oe/sstate"
EOF
  # When the whole script runs under one `sudo` (single-prompt mode), files we just wrote
  # would be root-owned — hand them back to the invoking user so later user-mode runs work.
  [ "$(id -u)" -eq 0 ] && chown "$HOST_UID:$HOST_GID" \
    "$BUILD_WEBOS/weboslayers.py" "$BUILD_WEBOS/webos-local.conf" 2>/dev/null || true
}

# Add the Jihad layer to bblayers.conf AFTER mcf (so mcf's git layer-management never
# touches it). Idempotent; runs host-side (the bound conf file is owned by the host uid).
wire_layer() {
  # build-webos' oe-init-build-env hardcodes ${TOPDIR}/oe-core/scripts on PATH (that is where the
  # `bitbake` WRAPPER lives — it re-execs bitbake under pseudo so do_install can fake root). But
  # the oe-core layer clones as `openembedded-core`, so without this symlink the wrapper is never
  # found, pseudo never engages, and every do_install fails with "chown: Operation not permitted".
  if [ -d "$BUILD_WEBOS/openembedded-core" ] && [ ! -e "$BUILD_WEBOS/oe-core" ]; then
    say "symlinking oe-core -> openembedded-core (pseudo wrapper discovery) ..."
    ln -sfn openembedded-core "$BUILD_WEBOS/oe-core"
  fi
  local bbl="$BUILD_WEBOS/conf/bblayers.conf"
  [ -f "$bbl" ] || { echo "!! $bbl not found — run 'oe-env.sh mcf <machine>' first" >&2; exit 1; }
  if grep -q 'Jihad-Browser/build/webos-oe' "$bbl"; then
    say "jihad layer already wired into bblayers.conf"
  else
    say "wiring jihad layer into bblayers.conf ..."
    printf '\n# Jihad Browser layer (added by oe-env.sh)\nBBLAYERS += "%s"\n' \
      "$JIHAD_LAYER_CHROOT" >> "$bbl"
  fi
}

provision() {
  extract_rootfs
  mount_kernel_fs
  if [ ! -f "$STAMP" ]; then
    say "provisioning OE host tools (once) ..."
    # 14.04/trusty is still served from the live archive.ubuntu.com + security.ubuntu.com
    # (verified 200) — the ubuntu-base sources.list is correct as-is; no old-releases rewrite.
    asroot chroot "$ROOTFS" /bin/bash -euc '
      export DEBIAN_FRONTEND=noninteractive
      # UXP/Goanna requires a HOST compiler of GCC >= 9.1 to build its host tools, but the
      # dylan OE native recipes need the stock gcc 4.8 — so install BOTH: gcc-9/g++-9 from the
      # ubuntu-toolchain-r PPA (which does carry trusty builds) ALONGSIDE the default 4.8. The
      # goanna recipe points only its HOST_CC/HOST_CXX at gcc-9; OE natives keep 4.8. [trusted=yes]
      # avoids a flaky keyserver round-trip on this EOL release.
      echo "deb [trusted=yes] http://ppa.launchpad.net/ubuntu-toolchain-r/test/ubuntu trusty main" \
        > /etc/apt/sources.list.d/ubuntu-toolchain-r.list
      apt-get update
      apt-get install -y --no-install-recommends \
        gawk wget git-core diffstat unzip texinfo gcc-multilib build-essential \
        chrpath socat libsdl1.2-dev xterm python2.7 python2.7-dev cpio file \
        ca-certificates locales sudo bzip2 xz-utils make python-minimal \
        autoconf2.13 yasm zip perl pkg-config m4 libtool automake gettext \
        gcc-9 g++-9
      # The Palm PDK toolchain (browser-adapter-jihad recipe) is i386 (32-bit x86). Enable
      # multiarch + its runtime libs so its gcc/as/ld run on the x86_64 rootfs (else
      # "as: error while loading shared libraries: libz.so.1").
      dpkg --add-architecture i386
      apt-get update
      apt-get install -y --no-install-recommends libc6:i386 zlib1g:i386 libstdc++6:i386
      locale-gen en_US.UTF-8
      ln -sf /usr/bin/python2.7 /usr/bin/python
      echo "dash dash/sh boolean false" | debconf-set-selections
      DEBIAN_FRONTEND=noninteractive dpkg-reconfigure dash
      # GitHub dropped git:// (2022) — weboslayers.py + recipes use it. Rewrite to https.
      git config --system url."https://github.com/".insteadOf "git://github.com/"
      git config --system url."https://git.openembedded.org/".insteadOf "git://git.openembedded.org/"
      git config --system url."https://git.yoctoproject.org/git/".insteadOf "git://git.yoctoproject.org/"
    '
    # builder user with the HOST uid/gid so bind-mounted files stay writable + owned.
    asroot chroot "$ROOTFS" /bin/bash -euc "
      groupadd -g $HOST_GID builder 2>/dev/null || true
      id -u builder >/dev/null 2>&1 || useradd -m -u $HOST_UID -g $HOST_GID -s /bin/bash builder
      echo 'builder ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/builder
      install -d -o $HOST_UID -g $HOST_GID /oe /oe/downloads /oe/sstate
    "
    asroot touch "$STAMP"
    say "provisioned."
  fi
}

# ---- run inside --------------------------------------------------------------
enter_user() {  # runs "$*" as builder in /oe/build-webos with a clean env
  asroot chroot --userspec="$HOST_UID:$HOST_GID" "$ROOTFS" \
    /usr/bin/env -i \
      HOME=/home/builder USER=builder LOGNAME=builder TERM="${TERM:-xterm}" \
      LANG=en_US.UTF-8 LC_ALL=en_US.UTF-8 \
      PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
      BB_ENV_EXTRAWHITE="MACHINE" MACHINE="$MACHINE" \
      /bin/bash -lc "cd /oe/build-webos 2>/dev/null || cd /oe; $*"
}
enter_root() { asroot chroot "$ROOTFS" /bin/bash -lc "cd /oe/build-webos 2>/dev/null || cd /oe; $*"; }

# ---- CLI ---------------------------------------------------------------------
cmd="${1:-shell}"; shift || true
case "$cmd" in
  provision) provision ;;
  clean)
    say "removing rootfs $ROOTFS (downloads/sstate kept) ..."
    umount_rootfs_tree            # SAFETY: never rm through a live bind of the repo
    asroot rm -rf "$ROOTFS"; say "done." ;;
  shell)   provision; mount_sources; enter_user "exec bash -i" ;;
  root)    provision; mount_sources; enter_root  "${*:-exec bash -i}" ;;
  run)     provision; mount_sources; enter_user  "$*" ;;
  mcf)     provision; mount_sources; enter_user  "./mcf $*" ;;
  wire)    ensure_build_webos; wire_layer ;;
  make)    provision; mount_sources; enter_user  "make $*" ;;
  bringup) # one shot: rootfs -> clone/pin layers + fetch (mcf) -> wire jihad layer
           provision; mount_sources
           enter_user "./mcf -p 0 -b 0 ${*:-$MACHINE}"
           wire_layer
           say "bring-up done. Build with:  build/webos-oe/oe-env.sh make webos-image" ;;
  *)       provision; mount_sources; enter_user  "$cmd $*" ;;
esac
