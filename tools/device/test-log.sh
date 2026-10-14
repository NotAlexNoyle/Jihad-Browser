#!/bin/sh
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# On the TouchPad: the last JIHAD-TEST lines a card self-test hook wrote to the system log, and
# the device's clock (the hook's lines carry UTC times; card-selftest.sh marks the ones at or
# after its screenshot).
#   build/webos-oe/dev-sh.sh tools/device/test-log.sh [lines]
grep "JIHAD-TEST" /var/log/messages | tail -n "${1:-2}"
date -u +%T
