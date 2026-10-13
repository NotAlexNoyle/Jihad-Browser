#!/bin/bash
# Copyright 2026 NotAlexNoyle. Licensed under the Apache License, Version 2.0 (see ../../LICENSE).
#
# Move the engine to another upstream UXP release in two commands (docs/ENGINE.md §6).
#
# The engine is upstream UXP at a release tag plus Jihad's commits on a branch jihad/uxp-<tag>
# (third_party/uxp); build/desktop/patches/ is the same commits as numbered git mails, so they
# carry to the next release with git's three-way merge. build/engine/engine-version.env holds the
# pins; check-engine-pins.sh, patch-queue.sh and check-engine-api.py check them.
#
#   update-engine.sh [TAG]     fetch upstream; put the queue on TAG (default: the newest RB_* release)
#                              in a worktree beside this repository; list the files upstream ALSO
#                              changed since the old base (read those diffs — a clean merge can still
#                              meet code that now behaves differently); run check-engine-api.py on
#                              the result. A conflict stops it: resolve in the worktree, git add,
#                              git am --continue there, then --finish.
#   update-engine.sh --finish  write the queue again from the worktree, check the new branch out in
#                              third_party/uxp, and bring every pin along (engine-version.env, the OE
#                              recipe, the user agent, the three start pages); then run
#                              patch-queue.sh --check and check-engine-pins.sh.
#   update-engine.sh --abort   drop an unfinished update's worktree and branch.
#   update-engine.sh --pins    only bring the pins to what third_party/uxp is checked out at
#                              (UXP_TAG must already name its release).
#
# It builds nothing and pushes nothing. After --finish: build the engine and the daemon, bundle, push
# to the device and run the checklist (docs/ENGINE.md §6–7), then commit the queue, the pins and the
# gitlink together.
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd); REPO=$(cd "$HERE/../.." && pwd)
UXP="$REPO/third_party/uxp"; Q="$REPO/build/desktop/patches"
ENV="$HERE/engine-version.env"; STATE="$HERE/.update-in-progress"
# shellcheck disable=SC1090
. "$ENV"

die() { echo "update-engine: $*" >&2; exit 1; }
setpin() {   # setpin NAME VALUE: rewrite one line of engine-version.env
  grep -q "^$1=" "$ENV" || die "engine-version.env has no $1"
  sed -i "s|^$1=.*|$1=$2|" "$ENV"
}
replace_in() {   # replace_in FILE OLD NEW (fixed strings; the file must contain OLD)
  python3 - "$REPO/$1" "$2" "$3" <<'PY'
import sys
p, old, new = sys.argv[1:]
s = open(p, encoding="utf-8").read()
if old not in s:
    sys.exit("update-engine: %s does not contain %r" % (p, old))
open(p, "w", encoding="utf-8").write(s.replace(old, new))
PY
}

# Every pin from the engine checked out in third_party/uxp; TAG names its release.
pins() {
  local tag="$1" base pin short goanna icu
  base=$(git -C "$UXP" rev-parse "$tag^{commit}")
  pin=$(git -C "$UXP" rev-parse HEAD)
  git -C "$UXP" merge-base --is-ancestor "$base" "$pin" || die "third_party/uxp is not on $tag"
  short=$(git -C "$UXP" rev-parse --short=7 "$base")
  goanna=$(grep -v '^#' "$UXP/config/milestone.txt" | grep -m1 . | cut -d. -f1,2)
  icu=$(cd "$UXP/config/external/icu/data" && ls icudt*l.dat | head -1)
  [ -n "$goanna" ] && [ -n "$icu" ] || die "could not read the Goanna version or the ICU data file"
  # The user agent and the start pages say "Goanna/<version> UXP/<short base>".
  local olds="Goanna/$GOANNA_VERSION UXP/$UXP_SHORT" news="Goanna/$goanna UXP/$short"
  if [ "$olds" != "$news" ]; then
    for f in render/goanna/JihadUserAgent.h app/source/StartPage.js app-mochi/source/JihadBrowser.js app-mojo/start.html; do
      replace_in "$f" "$olds" "$news"
    done
  fi
  [ "$UXP_PIN" = "$pin" ] || replace_in build/webos-oe/recipes-jihad/goanna/goanna_1.0.bb "SRCREV = \"$UXP_PIN\"" "SRCREV = \"$pin\""
  setpin UXP_TAG "$tag"; setpin UXP_BASE "$base"; setpin UXP_PIN "$pin"; setpin UXP_SHORT "$short"
  setpin GOANNA_VERSION "$goanna"; setpin ICU_DATA "$icu"
  echo "update-engine: pins now $tag ($short), engine branch head $pin, Goanna $goanna, $icu"
}

case "${1:-}" in
--abort)
  [ -f "$STATE" ] || die "no update in progress"
  WT=$(sed -n 's/^worktree=//p' "$STATE"); BR=$(sed -n 's/^branch=//p' "$STATE")
  (cd "$WT" 2>/dev/null && git am --abort 2>/dev/null) || true
  git -C "$UXP" worktree remove --force "$WT" 2>/dev/null || true
  git -C "$UXP" branch -D "$BR" 2>/dev/null || true
  rm -f -- "$STATE"; echo "update-engine: abandoned $BR"; exit 0 ;;
--pins)
  pins "$UXP_TAG"; exit 0 ;;
--finish)
  [ -f "$STATE" ] || die "no update in progress"
  TAG=$(sed -n 's/^tag=//p' "$STATE"); WT=$(sed -n 's/^worktree=//p' "$STATE"); BR=$(sed -n 's/^branch=//p' "$STATE")
  [ -d "$(git -C "$WT" rev-parse --git-path rebase-apply)" ] && die "git am is still in progress in $WT: finish it there"
  [ -z "$(git -C "$WT" status --porcelain)" ] || die "$WT has uncommitted changes"
  TMP=$(mktemp -d); trap 'rm -rf -- "${TMP:?}"' EXIT
  git -C "$WT" format-patch -q --zero-commit --no-signature -N -o "$TMP" "$TAG..HEAD"
  rm -f -- "$Q"/[0-9][0-9][0-9][0-9]-*.patch
  cp "$TMP"/*.patch "$Q/"
  git -C "$UXP" worktree remove "$WT"
  git -C "$UXP" checkout -q "$BR"
  rm -f -- "$STATE"
  pins "$TAG"
  # shellcheck disable=SC1090
  . "$ENV"
  "$HERE/patch-queue.sh" --check
  "$HERE/check-engine-pins.sh" || true   # the gitlink check passes once the gitlink is committed
  echo "update-engine: next — build, push, test (docs/ENGINE.md §6–7); then commit build/desktop/patches,"
  echo "build/engine/engine-version.env, the recipe, the user agent, the start pages and third_party/uxp."
  exit 0 ;;
-*)
  die "usage: update-engine.sh [TAG] | --finish | --abort | --pins" ;;
esac

[ -f "$STATE" ] && die "an update is in progress ($STATE): --finish or --abort it"
git -C "$UXP" fetch -q origin --tags
TAG="${1:-$(git -C "$UXP" tag -l 'RB_20[0-9][0-9][0-9][0-9][0-9][0-9]' --sort=-creatordate | head -1)}"
[ -n "$TAG" ] || die "no release tag found"
git -C "$UXP" rev-parse -q --verify "$TAG^{commit}" >/dev/null || die "no such tag: $TAG"
[ "$TAG" != "$UXP_TAG" ] || die "already on $TAG"
BR="jihad/uxp-$TAG"; WT="$(dirname "$REPO")/uxp-update-$TAG"
[ -e "$WT" ] && die "$WT exists"
git -C "$UXP" rev-parse -q --verify "refs/heads/$BR" >/dev/null && die "branch $BR exists"
printf 'tag=%s\nworktree=%s\nbranch=%s\n' "$TAG" "$WT" "$BR" > "$STATE"
echo "update-engine: $UXP_TAG -> $TAG: $(git -C "$UXP" rev-list --count --no-merges "$UXP_BASE..$TAG") upstream commits"
git -C "$UXP" worktree add -q -b "$BR" "$WT" "$TAG"
if ! (cd "$WT" && git am -q -3 --keep-cr "$Q"/[0-9][0-9][0-9][0-9]-*.patch); then
  echo; echo "update-engine: a patch did not apply (above). In $WT: fix the files, git add them,"
  echo "git am --continue (for each stop), then update-engine.sh --finish (or --abort)."
  exit 1
fi
UP=$(mktemp); OURS=$(mktemp)
git -C "$UXP" diff --name-only "$UXP_BASE" "$TAG" | sort > "$UP"
git -C "$WT" diff --name-only "$TAG" HEAD | sort > "$OURS"
echo; echo "update-engine: the queue applied. Files upstream also changed since $UXP_TAG — read"
echo "  git -C third_party/uxp diff $UXP_BASE $TAG -- <file>   against what our commits do there:"
comm -12 "$UP" "$OURS" | sed 's/^/  /'; rm -f -- "$UP" "$OURS"
echo; python3 "$HERE/check-engine-api.py" --uxp "$WT" | tail -3 || true
echo; echo "Then: build/engine/update-engine.sh --finish"
