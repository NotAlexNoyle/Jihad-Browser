# Shared definitions for the Jihad Browser OE recipes.
#
# The recipe SOURCES live at the REPO ROOT (this layer is build/webos-oe/ *inside* the
# repo, while app/, render/, packaging/, LICENSE, … are at the top). Under oe-env.sh the
# repo is bind-mounted at /oe/Jihad-Browser inside the chroot, so pointing the file://
# fetcher's search path (FILESEXTRAPATHS) there lets `file://app/`, `file://render/`, etc.
# resolve to the real trees. Override JIHAD_REPO if you build outside oe-env.sh.
JIHAD_REPO ?= "/oe/Jihad-Browser"
FILESEXTRAPATHS_prepend := "${JIHAD_REPO}:"

# One license anchor for all Jihad recipes (the composite Apache-2.0 + MPL-2.0 text at the
# repo root). Recipes still set their own LICENSE string; this is the change-detection file.
LIC_FILES_CHKSUM ?= "file://${JIHAD_REPO}/LICENSE;md5=be353ef4e243581e528e4524ffa92d97"

# webOS on-device install locations (this 2014 meta-webos has no webos-app class; the app
# dir is the standard webOS 3 path).
webos_applicationsdir ?= "/usr/palm/applications"

# The native cross-toolchain + target sysroot for the ARM engine/daemon builds. Both are
# assembled in-repo (gitignored, 219M/189M) and bound in the chroot, so recipes reference them
# in place. The toolchain is crosstool-NG GCC 9.4 (arm-webos-linux-gnueabi, glibc 2.23 → runs
# on the device 2.6.35 kernel); the sysroot is Debian-Jessie armel (GTK/X/cairo/pango/fontconfig).
JIHAD_TC ?= "${JIHAD_REPO}/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi"
JIHAD_ARM_SYSROOT ?= "${JIHAD_REPO}/build/webos-oe/arm-sysroot/root"

# ---- bitbake task-signature inputs (review #8) --------------------------------------------
# The Jihad recipes read build-control files and prebuilt trees STRAIGHT from ${JIHAD_REPO}
# rather than through SRC_URI, so the fetcher never checksums them and sstate would happily
# re-serve a stale result after they change. Fix: every task that reads such an input declares
# it in THAT task's signature with the per-task `file-checksums` varflag.
#
# VERIFIED SUPPORTED by the pinned bitbake 1.18.0 (build-webos 37540e5 -> bitbake 0f7b6a0):
#   lib/bb/cache.py:135            self.file_checksums = self.flaglist('file-checksums',
#                                  self.tasks, metadata, True)     <- collected per task, expanded
#   lib/bb/siggen.py:189-193       checksums = bb.fetch2.get_file_checksums(...)
#                                  for (f,cs) in checksums: ... data = data + cs
#                                  <- every listed file's md5 is folded into the task hash
#   lib/bb/fetch2/__init__.py:917  get_file_checksums(): `*` globs are expanded, directories are
#                                  os.walk'd; results are md5-cached by mtime
#                                  (lib/bb/checksum.py FileChecksumCache), so re-hashing is rare.
# Two rules that the 1.18 implementation imposes on the lists below:
#   * a glob matching nothing is silently skipped, while a bare missing path only warns -> prefer
#     globs for the git-ignored prebuilt inputs so a clean checkout stays quiet;
#   * a glob must NEVER match a DIRECTORY — glob results bypass the isdir branch and md5_file()
#     would raise a python-2 IOError that get_file_checksums does not catch (it catches OSError).
#     Only glob file patterns; pass whole directories as a bare path.
#
# The LARGE prebuilt inputs are represented by a small identity set instead of the whole tree —
# walking + md5-ing 219 MB of toolchain / 127 MB of sysroot at every runqueue prep is not
# acceptable:
#   JIHAD_TC_SIG   the gcc driver set + crosstool-NG's saved build log; both are rewritten by any
#                  re-assembly of the toolchain, so the pair identifies the toolchain build.
#   JIHAD_SYS_SIG  the sysroot's pkg-config manifest (89 .pc files, ~330 KB) — literally what the
#                  builds consume from it (PKG_CONFIG_LIBDIR points at exactly these two dirs),
#                  and each .pc carries its library's Version, so the set changes whenever the
#                  sysroot is re-assembled from different .debs.
JIHAD_TC_SIG ?= "${JIHAD_TC}/bin/arm-webos-linux-gnueabi-gcc* ${JIHAD_TC}/build.log*"
JIHAD_SYS_SIG ?= "${JIHAD_ARM_SYSROOT}/usr/lib/arm-linux-gnueabi/pkgconfig/*.pc ${JIHAD_ARM_SYSROOT}/usr/share/pkgconfig/*.pc"

# The Palm PDK (proprietary, fetched by fetch-pdk.sh — git-ignored) plus the vendored device
# libs/headers the PDK adapter build compiles and links against. The PDK's gcc 4.3.3 drivers
# stand in for the whole SDK tree. CAVEAT: build-adapter-pdk.sh also honours PDK_ROOT and a
# legacy sibling location; those alternate roots are NOT covered by this signature.
JIHAD_PDK ?= "${JIHAD_REPO}/build/webos-oe/pdk/opt/PalmPDK"
JIHAD_PDK_SIG ?= "${JIHAD_PDK}/arm-gcc/bin/arm-none-linux-gnueabi-gcc*"
JIHAD_DEPS ?= "${JIHAD_REPO}/build/webos-oe/adapter-deps"
# The staged device .so's (the actual link inputs) + the npapi/pbnjson headers + the glib shim.
# The Qt4 header drop (15 MB / 2857 files) is deliberately NOT walked: it is extracted from the
# same device Qt 4.8.0 package as the staged libQt*.so.4.8.0, which ARE hashed here.
JIHAD_DEPS_SIG ?= "${JIHAD_DEPS}/staging/lib/*.so* ${JIHAD_DEPS}/staging/include ${JIHAD_DEPS}/libpbnjson/include/public ${JIHAD_DEPS}/glib_compat.c"
