#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
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

UBUNTU_URL="${UBUNTU_URL:-https://cdimage.ubuntu.com/ubuntu-base/releases/14.04/release/ubuntu-base-14.04.6-base-amd64.tar.gz}"
# Pinned digest (review #2): the rootfs is extracted + executed as root, so verify it before use;
# reject any cached/downloaded tarball that doesn't match.
UBUNTU_SHA256="${UBUNTU_SHA256:-84a01ed9a5a6dd90d0a68b9d26b9bd92dacdbf221989bcf5549da22c01e5103a}"
UBUNTU_TB="$CACHE/$(basename "$UBUNTU_URL")"

# ---- ubuntu-toolchain-r PPA signing key (review #2) --------------------------
# The chroot installs gcc-9/g++-9 from the ubuntu-toolchain-r/test PPA (trusty is EOL and that PPA
# is the only source of the >= GCC 9.1 HOST compiler UXP's host tools require). apt authentication
# stays ON: the PPA key is pinned by FULL fingerprint and installed into its own keyring under
# /etc/apt/trusted.gpg.d/. trusty ships apt 1.0.1ubuntu2, which predates the sources.list
# `[signed-by=...]` option (apt 1.1), so a dedicated trusted.gpg.d keyring is the tightest scoping
# this apt supports — and gnupg is a hard dependency of apt, so it is always present.
#
# Key source, in order:
#   1. the pre-seeded copy in this repo ($PPA_KEY_SEED) — works offline, no keyserver round-trip;
#   2. otherwise $PPA_KEYSERVER, fetched over HTTPS.
# Either way the FULL fingerprint must match or provisioning ABORTS (fail closed — never fall back
# to [trusted=yes]). To (re-)seed the in-repo copy:
#   gpg --keyserver hkps://keyserver.ubuntu.com --recv-keys 60C317803A41BA51845E371A1E9377A2BA9EF27F
#   gpg --armor --export 60C317803A41BA51845E371A1E9377A2BA9EF27F > build/webos-oe/keys/ubuntu-toolchain-r.asc
PPA_KEY_FPR="60C317803A41BA51845E371A1E9377A2BA9EF27F"     # Launchpad "Toolchain builds"
PPA_KEY_SEED="${PPA_KEY_SEED:-$HERE/keys/ubuntu-toolchain-r.asc}"
PPA_KEYSERVER="${PPA_KEYSERVER:-https://keyserver.ubuntu.com}"
PPA_KEYRING="ubuntu-toolchain-r.gpg"                        # under /etc/apt/trusted.gpg.d/

# build-webos orchestrator: reuse a sibling clone if present, else clone it pinned.
BUILD_WEBOS="${BUILD_WEBOS:-$REPO/../../build-webos}"
BUILD_WEBOS_URL="https://github.com/openwebos/build-webos.git"
BUILD_WEBOS_REV="37540e592d3419e94b054b0bd3fd2206405f0256"

# The Jihad OE layer, as seen from INSIDE the chroot (repo is bound at /oe/Jihad-Browser).
JIHAD_LAYER_CHROOT="/oe/Jihad-Browser/build/webos-oe"

# Real user's uid/gid — the builder user + bind-mounted files use these. Prefer SUDO_UID/GID
# so `sudo oe-env.sh ...` still targets the invoking user (not root) rather than uid 0.
HOST_UID="${SUDO_UID:-$(id -u)}"; HOST_GID="${SUDO_GID:-$(id -g)}"
# review #11: run directly as root (no sudo) would make HOST_UID=0 → a root-owned builder + bound
# files. Require an explicit non-root uid instead of silently using 0.
if [ "$HOST_UID" -eq 0 ]; then
  if [ -n "${OE_HOST_UID:-}" ]; then HOST_UID="$OE_HOST_UID"; HOST_GID="${OE_HOST_GID:-$OE_HOST_UID}"
  else echo "!! running as root without SUDO_UID — set OE_HOST_UID=<your uid> (and OE_HOST_GID) so the build is not root-owned" >&2; exit 1; fi
fi
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
  # review #3: match only mounts AT or UNDER $rp on a path-component boundary (NOT a shared
  # prefix like /foo vs /foobar); deepest-first; three passes for nested rbinds.
  local pass
  for pass in 1 2 3; do
    while read -r mp; do
      case "$mp" in "$rp"|"$rp"/*) asroot umount -l "$mp" 2>/dev/null || true ;; esac
    done < <(awk '{print length, $2}' /proc/mounts | sort -rn | cut -d' ' -f2-)
  done
  # fail CLOSED: if anything is still mounted under $rp, refuse to let the caller rm through it.
  #
  # Captured, then tested — NOT `… | grep -q x`. Under `set -o pipefail` (line 35) `grep -q`
  # exits at the FIRST match, the `while` upstream takes SIGPIPE, pipefail promotes the 141 to
  # the pipeline's status, and the `if` therefore reads FALSE exactly when mounts ARE still
  # present: the guard fails OPEN and lets a recursive delete run through a live bind mount.
  # It only ever "worked" when the producer happened to finish before grep exited, i.e. with a
  # single leftover mount. (Seventh instance of this defect class in this tree; see review F-2.)
  local still
  still=$(awk '{print $2}' /proc/mounts | while read -r mp; do case "$mp" in "$rp"|"$rp"/*) echo "$mp" ;; esac; done)
  if [ -n "$still" ]; then
    echo "!! mounts still present under $rp after unmount — refusing the destructive op" >&2
    echo "$still" >&2
    exit 1
  fi
}

# review #3: refuse to rm/extract unless ROOTFS is a dedicated dir under our own build tree —
# never /, a home dir, the repo, a parent, or an approved-tree sibling with a shared prefix.
validate_rootfs() {
  local rp base; rp="$(readlink -f "$ROOTFS" 2>/dev/null || echo "$ROOTFS")"
  base="$(readlink -f "$HERE" 2>/dev/null || echo "$HERE")"
  case "$rp" in
    "$base"/oe-rootfs|"$base"/oe-rootfs/*) : ;;
    *) echo "!! refusing to operate on OE_ROOTFS='$rp' — it must live under $base/oe-rootfs" >&2; exit 1 ;;
  esac
}

need() { command -v "$1" >/dev/null 2>&1 || { echo "!! missing host tool: $1" >&2; exit 1; }; }
say()  { echo ">> $*" >&2; }

# ---- rootfs bring-up ---------------------------------------------------------
verify_sha256() {  # file expected-hex
  local got; got="$(sha256sum "$1" | awk '{print $1}')"
  [ "$got" = "$2" ] || { echo "!! SHA256 mismatch for $1 (got $got, expected $2)" >&2; return 1; }
}

fetch_tarball() {
  if [ -f "$UBUNTU_TB" ]; then
    need sha256sum
    verify_sha256 "$UBUNTU_TB" "$UBUNTU_SHA256" || { echo "!! cached rootfs failed digest — remove $UBUNTU_TB and retry" >&2; exit 1; }
    return
  fi
  need curl; need sha256sum; mkdir -p "$CACHE"
  say "fetching $(basename "$UBUNTU_TB") ..."
  curl -fL --retry 3 -o "$UBUNTU_TB.part" "$UBUNTU_URL"
  verify_sha256 "$UBUNTU_TB.part" "$UBUNTU_SHA256" || { rm -f "$UBUNTU_TB.part"; echo "!! downloaded rootfs failed digest — aborting" >&2; exit 1; }
  mv "$UBUNTU_TB.part" "$UBUNTU_TB"
}

extract_rootfs() {
  [ -e "$ROOTFS/etc/os-release" ] && return
  need tar
  validate_rootfs               # SAFETY: never rm/extract outside the sanctioned oe-rootfs dir
  fetch_tarball
  say "extracting rootfs -> $ROOTFS ..."
  umount_rootfs_tree            # SAFETY: never rm through a live bind of the repo
  asroot rm -rf "$ROOTFS"; mkdir -p "$ROOTFS"
  asroot tar -C "$ROOTFS" -xpf "$UBUNTU_TB"
}

write_resolv() { echo "nameserver 1.1.1.1" | asroot tee "$ROOTFS/etc/resolv.conf" >/dev/null; }

# Install the ubuntu-toolchain-r PPA signing key as a DEDICATED apt keyring inside the rootfs,
# pinned by full fingerprint (review #2 — replaces the old `[trusted=yes]`, which turned apt
# authentication off entirely). Runs host-side (host gpg, host network) and drops a binary keyring
# into $ROOTFS/etc/apt/trusted.gpg.d/. Fails CLOSED on any mismatch/failure.
install_ppa_key() {
  local dest="$ROOTFS/etc/apt/trusted.gpg.d/$PPA_KEYRING" tmp keyfile
  need gpg
  tmp="$(mktemp -d "${TMPDIR:-/tmp}/jihad-ppa-key.XXXXXX")" || { echo "!! mktemp failed" >&2; exit 1; }
  chmod 700 "$tmp"                     # gpg refuses a group/world-readable GNUPGHOME
  keyfile="$tmp/key.asc"
  if [ -f "$PPA_KEY_SEED" ]; then
    say "using the pre-seeded PPA key $PPA_KEY_SEED"
    cp "$PPA_KEY_SEED" "$keyfile"
  else
    need curl
    say "fetching the ubuntu-toolchain-r PPA key $PPA_KEY_FPR from $PPA_KEYSERVER ..."
    curl -fsSL --retry 3 --max-time 60 \
      "$PPA_KEYSERVER/pks/lookup?op=get&search=0x$PPA_KEY_FPR&options=mr" -o "$keyfile" || {
        rm -rf "$tmp"
        echo "!! could not fetch the PPA signing key (no network?). Seed it in-repo instead:" >&2
        echo "     gpg --keyserver hkps://keyserver.ubuntu.com --recv-keys $PPA_KEY_FPR" >&2
        echo "     gpg --armor --export $PPA_KEY_FPR > $PPA_KEY_SEED" >&2
        exit 1; }
  fi
  # Import into a THROWAWAY keyring, then demand the FULL 40-hex fingerprint: a short/long key id
  # is exactly the collision apt authentication exists to prevent.
  GNUPGHOME="$tmp" gpg --batch --quiet --import "$keyfile" 2>/dev/null || {
    rm -rf "$tmp"; echo "!! PPA key material is not a valid OpenPGP key" >&2; exit 1; }
  # Captured, then matched — same reason as the mount guard above. `gpg | grep -q` under
  # pipefail reports a MISMATCH when the fingerprint MATCHES (grep -q exits first, gpg takes
  # SIGPIPE, pipefail returns 141, `!` inverts it to true). This one fails CLOSED — it refuses a
  # good key rather than accepting a bad one — but a check that misfires on success is still a
  # check nobody will trust. `case` on the captured text has no pipeline at all.
  local fprs
  fprs=$(GNUPGHOME="$tmp" gpg --batch --with-colons --fingerprint 2>/dev/null)
  case $'\n'"$fprs"$'\n' in
    *$'\n'"fpr:::::::::$PPA_KEY_FPR:"*) : ;;
    *)
      echo "!! PPA key fingerprint MISMATCH — expected $PPA_KEY_FPR, the key material carries:" >&2
      printf '%s\n' "$fprs" | grep '^fpr' >&2 || true
      rm -rf "$tmp"; echo "!! refusing to provision with an unverified apt key" >&2; exit 1 ;;
  esac
  # Export ONLY the pinned key (so key material carrying extra keys cannot smuggle them in) as a
  # binary keyring — the format /etc/apt/trusted.gpg.d expects.
  GNUPGHOME="$tmp" gpg --batch --export "$PPA_KEY_FPR" > "$tmp/$PPA_KEYRING" 2>/dev/null || {
    rm -rf "$tmp"; echo "!! failed to export the pinned PPA key" >&2; exit 1; }
  [ -s "$tmp/$PPA_KEYRING" ] || { rm -rf "$tmp"; echo "!! exported PPA keyring is empty" >&2; exit 1; }
  asroot install -d -m 0755 "$ROOTFS/etc/apt/trusted.gpg.d"
  asroot install -m 0644 "$tmp/$PPA_KEYRING" "$dest"
  rm -rf "$tmp"
  say "PPA key $PPA_KEY_FPR installed as $PPA_KEYRING — apt authentication stays ON."
}

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
  elif [ "$(git -C "$BUILD_WEBOS" rev-parse HEAD 2>/dev/null)" != "$BUILD_WEBOS_REV" ]; then
    # review #10: an existing sibling checkout is reused in place — make sure it is the pinned rev,
    # not a stale/wrong/dirty tree, before we build against it.
    say "WARNING: build-webos HEAD != pinned $BUILD_WEBOS_REV — checking out the pin ..."
    git -C "$BUILD_WEBOS" fetch --quiet origin "$BUILD_WEBOS_REV" 2>/dev/null || true
    git -C "$BUILD_WEBOS" checkout -q "$BUILD_WEBOS_REV" || say "WARNING: could not check out the pin; proceeding with existing HEAD"
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
    install_ppa_key            # review #2: pin the PPA key BEFORE anything reads that repo
    # 14.04/trusty is still served from the live archive.ubuntu.com + security.ubuntu.com
    # (verified 200) — the ubuntu-base sources.list is correct as-is; no old-releases rewrite.
    asroot chroot "$ROOTFS" /bin/bash -euc '
      export DEBIAN_FRONTEND=noninteractive
      # UXP/Goanna requires a HOST compiler of GCC >= 9.1 to build its host tools, but the
      # dylan OE native recipes need the stock gcc 4.8 — so install BOTH: gcc-9/g++-9 from the
      # ubuntu-toolchain-r PPA (which does carry trusty builds) ALONGSIDE the default 4.8. The
      # goanna recipe points only its HOST_CC/HOST_CXX at gcc-9; OE natives keep 4.8.
      # review #2: the repo is AUTHENTICATED — install_ppa_key() (host-side) pinned the PPA
      # signing key by full fingerprint into /etc/apt/trusted.gpg.d/. Fail closed if it is not
      # there rather than silently installing unverified packages as root.
      [ -s /etc/apt/trusted.gpg.d/ubuntu-toolchain-r.gpg ] || {
        echo "!! ubuntu-toolchain-r keyring missing — refusing an unauthenticated PPA" >&2; exit 1; }
      echo "deb http://ppa.launchpad.net/ubuntu-toolchain-r/test/ubuntu trusty main" \
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
  # review #2: an ALREADY-provisioned rootfs from an older oe-env.sh still carries the
  # `[trusted=yes]` PPA line (apt authentication off). Repair it in place rather than leaving the
  # hole until someone runs `clean`. Idempotent, and skipped once the line is clean.
  local ppalist="$ROOTFS/etc/apt/sources.list.d/ubuntu-toolchain-r.list"
  if [ -f "$ppalist" ] && grep -q 'trusted=yes' "$ppalist" 2>/dev/null; then
    say "repairing a legacy [trusted=yes] ubuntu-toolchain-r entry (review #2) ..."
    install_ppa_key
    echo "deb http://ppa.launchpad.net/ubuntu-toolchain-r/test/ubuntu trusty main" \
      | asroot tee "$ppalist" >/dev/null
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
    validate_rootfs               # SAFETY: never rm outside the sanctioned oe-rootfs dir
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
