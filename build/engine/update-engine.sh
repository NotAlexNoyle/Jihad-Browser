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
#   update-engine.sh [TAG]     fetch upstream; refuse unless third_party/uxp is at UXP_PIN and the
#                              queue reproduces it (an engine commit not yet in the queue would be
#                              left behind); list the files upstream ALSO changed since the old base
#                              (read those diffs — a clean merge can still meet code that now behaves
#                              differently); put the queue on TAG (default: the newest RB_* release)
#                              in a worktree beside this repository; run check-engine-api.py on the
#                              result. A conflict stops it: resolve in the worktree, git add, git am
#                              --continue there, then --finish (which runs the API check itself).
#   update-engine.sh --finish  check the worktree (API check included) and that third_party/uxp has
#                              no local changes; check the new branch out there; write the queue
#                              again from it and bring every pin along (engine-version.env, the OE
#                              recipe, the user agent, the three start pages, .gitmodules' branch);
#                              then run patch-queue.sh --check and check-engine-pins.sh. Stopped
#                              part-way, it can be run again.
#   update-engine.sh --abort   drop an unfinished update's worktree and branch (not once --finish
#                              has checked the branch out: then the branch is the only copy).
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
state() { sed -n "s/^$1=//p" "$STATE"; }
setpin() {   # setpin NAME VALUE: rewrite one line of engine-version.env
  grep -q "^$1=" "$ENV" || die "engine-version.env has no $1"
  sed -i "s|^$1=.*|$1=$2|" "$ENV"
}
replace_in() {   # replace_in OLD NEW FILE...: every file must hold OLD or already NEW; checked before any is written
  python3 - "$REPO" "$@" <<'PY'
import os, sys
repo, old, new, files = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4:]
texts = {}
for f in files:
    s = open(os.path.join(repo, f), encoding="utf-8").read()
    if old not in s and new not in s:
        sys.exit("update-engine: %s holds neither %r nor %r; nothing was rewritten" % (f, old, new))
    texts[f] = s
for f, s in texts.items():
    if old in s:
        open(os.path.join(repo, f), "w", encoding="utf-8").write(s.replace(old, new))
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
    replace_in "$olds" "$news" render/goanna/JihadUserAgent.h app/source/StartPage.js \
      app-mochi/source/JihadBrowser.js app-mojo/start.html
  fi
  [ "$UXP_PIN" = "$pin" ] || replace_in "SRCREV = \"$UXP_PIN\"" "SRCREV = \"$pin\"" build/webos-oe/recipes-jihad/goanna/goanna_1.0.bb
  git -C "$REPO" config -f .gitmodules submodule.third_party/uxp.branch "jihad/uxp-$tag"
  setpin UXP_TAG "$tag"; setpin UXP_BASE "$base"; setpin UXP_PIN "$pin"; setpin UXP_SHORT "$short"
  setpin GOANNA_VERSION "$goanna"; setpin ICU_DATA "$icu"
  echo "update-engine: pins now $tag ($short), engine branch head $pin, Goanna $goanna, $icu"
}

# The files our commits change that upstream also changed between the old base and TAG.
overlap() {
  local up ours; up=$(mktemp); ours=$(mktemp)
  git -C "$UXP" diff --name-only "$UXP_BASE" "$1" | sort > "$up"
  git -C "$UXP" diff --name-only "$UXP_BASE" "$UXP_PIN" | sort > "$ours"
  echo "update-engine: files upstream also changed since $UXP_TAG — read"
  echo "  git -C third_party/uxp diff $UXP_BASE $1 -- <file>   against what our commits do there:"
  comm -12 "$up" "$ours" | sed 's/^/  /'
  rm -f -- "$up" "$ours"
}
apicheck() {   # apicheck TREE: the whole report; a FAIL stops
  python3 "$HERE/check-engine-api.py" --uxp "$1" ||
    die "check-engine-api.py FAILs above: fix them in $1 (git commit --fixup <commit>, then git rebase -i --autosquash $(state tag)), then --finish"
}

case "${1:-}" in
--abort)
  [ -f "$STATE" ] || die "no update in progress"
  WT=$(state worktree); BR=$(state branch)
  if [ "$(state phase)" = finishing ]; then
    die "--finish has checked $BR out in third_party/uxp and it is the only copy of the carried commits: run --finish again, or check $UXP_PIN out there and delete $BR by hand"
  fi
  (cd "$WT" 2>/dev/null && git am --abort 2>/dev/null) || true
  git -C "$UXP" worktree remove --force "$WT" 2>/dev/null || true
  git -C "$UXP" branch -D "$BR" 2>/dev/null || true
  rm -f -- "$STATE"; echo "update-engine: abandoned $BR"; exit 0 ;;
--pins)
  pins "$UXP_TAG"; exit 0 ;;
--finish)
  [ -f "$STATE" ] || die "no update in progress"
  TAG=$(state tag); WT=$(state worktree); BR=$(state branch)
  if [ "$(state phase)" != finishing ]; then
    [ -d "$(git -C "$WT" rev-parse --git-path rebase-apply)" ] && die "git am is still in progress in $WT: finish it there"
    [ -z "$(git -C "$WT" status --porcelain)" ] || die "$WT has uncommitted changes"
    [ -z "$(git -C "$UXP" status --porcelain --untracked-files=no)" ] ||
      die "third_party/uxp has local changes: commit or stash them there first (nothing was changed)"
    apicheck "$WT"
    echo "phase=finishing" >> "$STATE"
    git -C "$WT" checkout -q --detach          # frees the branch for third_party/uxp
    git -C "$UXP" checkout -q "$BR"
  fi
  # From here a run stopped part-way can be run again: the branch is checked out in third_party/uxp.
  [ "$(git -C "$UXP" rev-parse --abbrev-ref HEAD)" = "$BR" ] || die "third_party/uxp is not on $BR"
  TMP=$(mktemp -d); trap 'rm -rf -- "${TMP:?}"' EXIT
  git -C "$UXP" format-patch -q --zero-commit --no-signature -N -o "$TMP" "$TAG..$BR"
  ls "$TMP"/*.patch >/dev/null 2>&1 || die "$TAG..$BR has no commits"
  rm -f -- "$Q"/[0-9][0-9][0-9][0-9]-*.patch
  cp "$TMP"/*.patch "$Q/"
  [ -d "$WT" ] && git -C "$UXP" worktree remove "$WT"
  pins "$TAG"
  rm -f -- "$STATE"
  # shellcheck disable=SC1090
  . "$ENV"
  "$HERE/patch-queue.sh" --check
  "$HERE/check-engine-pins.sh" || true   # the gitlink check passes once the gitlink is committed
  echo "update-engine: next — build, push, test (docs/ENGINE.md §6–7); then commit build/desktop/patches,"
  echo "build/engine/engine-version.env, .gitmodules, the recipe, the user agent, the start pages and third_party/uxp."
  exit 0 ;;
-*)
  die "usage: update-engine.sh [TAG] | --finish | --abort | --pins" ;;
esac

[ -f "$STATE" ] && die "an update is in progress ($STATE): --finish or --abort it"
[ "$(git -C "$UXP" rev-parse HEAD)" = "$UXP_PIN" ] ||
  die "third_party/uxp is not at UXP_PIN: put each new engine commit in the queue (patch-queue.sh --add), then update-engine.sh --pins"
"$HERE/patch-queue.sh" --check || die "the queue does not reproduce UXP_PIN (above)"
git -C "$UXP" fetch -q origin --tags
TAG="${1:-$(git -C "$UXP" tag -l 'RB_20[0-9][0-9][0-9][0-9][0-9][0-9]' --sort=-creatordate | head -1)}"
[ -n "$TAG" ] || die "no release tag found"
git -C "$UXP" rev-parse -q --verify "$TAG^{commit}" >/dev/null || die "no such tag: $TAG"
[ "$TAG" != "$UXP_TAG" ] || die "already on $TAG"
BR="jihad/uxp-$TAG"; WT="$(dirname "$REPO")/uxp-update-$TAG"
[ -e "$WT" ] && die "$WT exists"
git -C "$UXP" rev-parse -q --verify "refs/heads/$BR" >/dev/null && die "branch $BR exists"
echo "update-engine: $UXP_TAG -> $TAG: $(git -C "$UXP" rev-list --count --no-merges "$UXP_BASE..$TAG") upstream commits"
overlap "$TAG"; echo
printf 'tag=%s\nworktree=%s\nbranch=%s\n' "$TAG" "$WT" "$BR" > "$STATE"
git -C "$UXP" worktree add -q -b "$BR" "$WT" "$TAG"
if ! (cd "$WT" && git am -q -3 --keep-cr "$Q"/[0-9][0-9][0-9][0-9]-*.patch); then
  echo; echo "update-engine: a patch did not apply (above). In $WT: fix the files, git add them,"
  echo "git am --continue (for each stop), then update-engine.sh --finish (or --abort)."
  exit 1
fi
echo "update-engine: the queue applied."; echo
apicheck "$WT"
echo; echo "Then: build/engine/update-engine.sh --finish"
