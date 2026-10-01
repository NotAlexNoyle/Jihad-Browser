#!/bin/bash
# Stage the ENGINE media + input test pages and freshly generated clips into the device's
# /tmp/jm, md5-verified. /tmp is volatile, and a clip left there by an earlier session is not
# evidence of anything -- the clips are regenerated on every run (make-media-assets.sh).
#
#   stage-media-pages.sh [--keep <dir>]     --keep leaves the generated set in <dir>, e.g. to
#                                           serve the same files over http for the http:// runs:
#                                           (cd <dir> && python3 -m http.server 8088)
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ASSETS="$(cd "$HERE/../.." && pwd)/render/goanna/test"
if [ "${1:-}" = "--keep" ]; then STAGE="${2:?--keep <dir>}"; mkdir -p "$STAGE"
else STAGE="$(mktemp -d)"; trap 'rm -rf "$STAGE"' EXIT; fi

NOVACOM=(novacom)
[ -n "${JIHAD_DEVICE:-}" ] && NOVACOM=(novacom -d "$JIHAD_DEVICE")

bash "$ASSETS/make-media-assets.sh" "$STAGE"
cp "$ASSETS/jihad-media.html" "$ASSETS/jihad-codecs.html" "$ASSETS/jihad-input-echo.html" "$STAGE/"

"${NOVACOM[@]}" run file://bin/mkdir -- -p /tmp/jm >/dev/null 2>&1 || true
fail=0
for f in "$STAGE"/*; do
  n=$(basename "$f"); want=$(md5sum "$f" | cut -c1-32)
  "${NOVACOM[@]}" put "file:///tmp/jm/$n" < "$f"
  got=$("${NOVACOM[@]}" run file://usr/bin/md5sum -- "/tmp/jm/$n" 2>/dev/null | cut -c1-32 || true)
  if [ "$got" = "$want" ]; then echo "staged /tmp/jm/$n"; else echo "MD5 MISMATCH /tmp/jm/$n ($got != $want)" >&2; fail=1; fi
done
exit $fail
