#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Run a shell script on the device and get ALL of its output, as soon as it ends.
#
#   build/webos-oe/dev-sh.sh [-t MAXSECONDS] <local script> [args for it...]
#
# The script is pushed to /tmp on the device, run with /bin/sh, and removed.
#
# WHY NOT JUST `novacom run`. Two things about novacom cost this project time, repeatedly:
#   - it stops, and drops whatever the device had not yet printed, the moment its own stdin
#     reaches end-of-file. Hence the standing habit `sleep N | novacom run …`.
#   - but a pipeline lasts as long as its sleep: a 3-second probe held by `sleep 60` takes 60 s,
#     and a long job held by too short a sleep is cut off mid-output and still looks plausible.
# Holding stdin open with a sleep that is KILLED when novacom returns gets both right: the call
# ends when the device script ends, and MAXSECONDS (default 600) is only a ceiling.
#   - and `novacom run file://bin/sh -- -c '…'` mangles the quoting; a script file does not.
set -euo pipefail
. "$(dirname "$0")/host-path.sh"   # the repository's own novacom and palm-* first (R11)
MAX=600
if [ "${1:-}" = "-t" ]; then MAX="$2"; shift 2; fi
SRC="${1:?usage: dev-sh.sh [-t MAXSECONDS] <local script> [args...]}"; shift
[ -f "$SRC" ] || { echo "dev-sh: no such script: $SRC" >&2; exit 2; }
DEVPATH="/tmp/dev-sh.$$.$(basename "$SRC")"
novacom put "file://$DEVPATH" < "$SRC"
# The holder's STDOUT is the pipe: it must stay attached, or the pipe has no writer, stdin is at
# end-of-file from the start, and anything on the device that watches its stdin (luna-send
# does) quits before its answer arrives — measured: every `$(luna-send …)` came back empty.
exec 3< <(exec sleep "$MAX" 2>/dev/null); HOLDER=$!
set +e
novacom run file://bin/sh -- "$DEVPATH" "$@" <&3
RC=$?
set -e
exec 3<&-; kill "$HOLDER" 2>/dev/null || true
novacom run file://bin/rm -- -f "$DEVPATH" </dev/null >/dev/null 2>&1 || true
exit $RC
