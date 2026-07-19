# Shared HP TouchPad-family machine definition for the Jihad Browser OE layer.
# webOS 3.0.5, ARMv7. Both models (TouchPad = topaz/tenderloin, TouchPad Go = opal)
# derive from this file and override ONLY what differs (physical screen size / DPI).
#
# This file is required by tenderloin.conf and opal.conf. It is the single source of
# truth for everything the two models SHARE; per-model differences are captured in the
# including .conf, never assumed identical (device-build R6).
#
# NOTE: build/webos-oe is not a full OE image layer — the working device pipeline is the
# direct cross-build scripts (build-goanna-arm.sh / build-daemon-arm.sh /
# build-adapter-pdk.sh, crosstool-NG GCC 9.4 / glibc 2.23 softfp). These machine confs
# capture machine identity + geometry for the recipes-jihad/*.bb and for anyone standing
# up a real bitbake tree; see docs/DEVICE-BUILD.md for the runnable invocation.

# ARMv7-A + NEON, imported ARM tune from meta-webos (resolves when meta-webos is on
# BBPATH/BBLAYERS).
require conf/machine/include/arm/arch-armv7a.inc

# webOS 3 shipped a SOFTFP userland (soft-float ABI, VFP/NEON hardware present). The
# Jihad ARM pipeline builds  -march=armv7-a -mfpu=neon -mfloat-abi=softfp  to match the
# device libc/ABI. DO NOT switch to hard-float (callconvention-hard) — a hard-float
# binary will not load against the device's softfp glibc 2.8.
DEFAULTTUNE = "armv7a-neon"

# Qualcomm APQ8060 (Snapdragon S3, dual-core Scorpion) — the SAME SoC family on both
# models, which is why one ARMv7 softfp binary set serves both.
SOC_FAMILY = "apq8060"

# Palm 2.6.35 kernel line (per-board string set in the including .conf).
PREFERRED_PROVIDER_virtual/kernel ?= "linux-palm"
KERNEL_IMAGETYPE ?= "uImage"

# webOS 3 device capabilities shared by both TouchPad models. /media/internal is VFAT
# (no symlinks, no journaling) — the device bundle assembler relies on this fact.
MACHINE_FEATURES = "touchscreen usbgadget wifi bluetooth vfat"

# Both panels are 1024x768 (identical XRES/YRES); only the physical diagonal and the
# resulting DPI differ. Diagonal is exactly 1280 px (1024x768 is a 3-4-5 triangle),
# so DPI = 1280 / diagonal_inches. Set JIHAD_SCREEN_* per-model in the including .conf.
JIHAD_SCREEN_XRES ?= "1024"
JIHAD_SCREEN_YRES ?= "768"

# webOS apps (the two Jihad UI .ipks) are density-independent — Enyo scales layout to the
# machine DPI at runtime — so the UI packages are model-agnostic (build once, install on
# both). Geometry/DPI below is captured for render/viewport tuning, not for branching the
# package build.
