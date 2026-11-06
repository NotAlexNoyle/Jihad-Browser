#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# persona-file.sh up|down SHELL (host side): the test Persona carbon_light-2.0.xpi in /media/internal/downloads for about:personas'
# Install from file (up); removed after (down).
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; ROOT="$(cd "$HERE/../../../../.." && pwd)"; . "$ROOT/build/webos-oe/host-path.sh"
XPI=carbon_light-2.0.xpi
if [ "$1" = up ]; then
  novacom run file://bin/mkdir -- -p /media/internal/downloads </dev/null >/dev/null 2>&1
  novacom put file:///media/internal/downloads/$XPI < "$ROOT/render/goanna/test/personas/$XPI"; echo "persona file: put"
else novacom run file://bin/rm -- -f /media/internal/downloads/$XPI </dev/null >/dev/null 2>&1; echo "persona file: removed"; fi
