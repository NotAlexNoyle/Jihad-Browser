#!/bin/bash
# Copyright 2026 NotAlexNoyle. Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# The engine's patch queue (build/desktop/patches/) and the engine branch (third_party/uxp,
# jihad/uxp-<UXP_TAG>) must say the same thing. The BRANCH is the source of truth: every engine
# change is a commit there; the queue is generated from those commits for builds that start from
# pristine UXP (the desktop build, ENGINE-SOURCE.md's record) and to carry them to the next release
# (update-engine.sh).
#
#   patch-queue.sh --check        apply the whole queue to UXP_BASE in a scratch git index and
#                                 compare the result with UXP_PIN's tree (no checkout, ~1 min)
#   patch-queue.sh --add <commit> write the next numbered patch from one engine commit, as
#                                 update-engine.sh --finish writes them (then update-engine.sh --pins
#                                 moves UXP_PIN to the branch's head)
#
# UXP_BASE / UXP_PIN come from build/engine/engine-version.env.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd); REPO=$(cd "$HERE/../.." && pwd)
UXP="$REPO/third_party/uxp"; Q="$REPO/build/desktop/patches"
# shellcheck disable=SC1091
. "$HERE/engine-version.env"

case "${1:-}" in
--check)
  # The index must not exist yet: older git refuses an empty file ("index file smaller than
  # expected"), which is what mktemp leaves (Codex r13).
  tmp=$(mktemp -d); idx="$tmp/index"; err="$tmp/err"; trap 'rm -rf "$tmp"' EXIT
  export GIT_INDEX_FILE="$idx"
  git -C "$UXP" read-tree "$UXP_BASE"
  n=0
  for p in "$Q"/[0-9][0-9][0-9][0-9]-*.patch; do
    if ! git -C "$UXP" apply --cached "$p" 2>"$err"; then
      echo "FAIL: $(basename "$p") does not apply on top of the ones before it:"; sed 's/^/  /' "$err"; exit 1
    fi
    n=$((n + 1))
  done
  got=$(git -C "$UXP" write-tree); want=$(git -C "$UXP" rev-parse "$UXP_PIN^{tree}")
  unset GIT_INDEX_FILE
  if [ "$got" = "$want" ]; then
    echo "ok: the $n patches reproduce UXP_PIN ($UXP_PIN) exactly"
  else
    echo "DRIFT: the $n patches do not reproduce UXP_PIN; these files differ (queue vs branch):"
    git -C "$UXP" diff --stat "$got" "$want" | sed 's/^/  /'
    exit 1
  fi ;;
--add)
  c="${2:?commit}"
  last=$(ls "$Q" | grep -E '^[0-9]{4}-.*\.patch$' | sort | tail -1 | cut -c1-4)
  next=$((10#$last + 1))
  git -C "$UXP" format-patch -q --zero-commit --no-signature --start-number "$next" -o "$Q" -1 "$c"
  ls "$Q" | grep -E "^$(printf %04d "$next")-" ;;
*)
  sed -n '4,16p' "$0"; exit 2 ;;
esac
