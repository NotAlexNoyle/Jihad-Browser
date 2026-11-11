# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# engine-dist.sh (sourced): which engine build is shipped, for make-device-bundle.sh and push-engine-update.sh alike.
# The PGO build (T-556, measured 2026-11-11: Wikipedia 13.7 s against 17.0, jsbench dom -35 %, the card's frame 1.1 s
# sooner cold; docs/rounds/pgo-2026-11-11/) when it exists: mozconfig.goanna-arm-pgo, use phase. A plain build newer
# than it carries engine changes the PGO build lacks, so that one is used, with a warning naming the PGO use build to
# redo. DIST=<dir> set by the caller still picks either.
#   jihad_engine_dist <build/webos-oe dir>   prints the dist directory
jihad_engine_dist() {
  local here=$1 pgo="$1/out-arm/obj-jihad-goanna-arm-pgo/dist" plain="$1/out-arm/obj-jihad-goanna-arm/dist"
  if [ -n "${DIST:-}" ]; then echo "$DIST"; return; fi
  if [ -e "$pgo/bin/libxul.so" ]; then
    if [ -e "$plain/bin/libxul.so" ] && [ "$(readlink -f "$plain/bin/libxul.so")" -nt "$(readlink -f "$pgo/bin/libxul.so")" ]; then
      echo "!! engine: the plain build is newer than the PGO build; using the plain one. Redo the PGO use build:" >&2
      echo "!!   build/webos-oe/in-container.sh --arm build/webos-oe/build-goanna-arm.sh build \"\" mozconfig.goanna-arm-pgo use" >&2
      echo "$plain"; return
    fi
    echo "engine: the PGO build ($pgo)" >&2; echo "$pgo"; return
  fi
  echo "$plain"
}
