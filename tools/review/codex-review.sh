#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Adversarial review of a range through Codex, with Cavekit's review prompt, read-only. Cavekit
# 3.1's own scripts/codex-review.sh fails silently on Codex 0.160, so this calls `codex exec`
# directly (the owner's standing rule: Codex reviews every tier; when Codex is out of credits the
# Cavekit inspector agent reviews instead, and Codex reviews the same range when credits return).
#
#   tools/review/codex-review.sh <repo> <base> <out.md> <log> [pathspec...]
#
# <repo> is this repository or the add-on site's (Riverstone-Addons); <base> the commit before the
# range; the range ends at HEAD, or at $HEADREF. <out.md> gets the findings table (or NO_FINDINGS),
# <log> codex's own output. Environment:
#   EXTRA      more context for this range (which kits and requirements apply, what to focus on)
#   DIFFOPTS   extra `git diff` options (e.g. --diff-filter=d)
#   HEADREF    the range's last commit (default HEAD)
#   CODEX_MODEL  the model (default gpt-5.6-sol)
# Example, a tier of this repository without the engine patches and pictures:
#   EXTRA="Relevant: cavekit-device-build.md R11." tools/review/codex-review.sh . 045c0fef \
#     /tmp/r25.md /tmp/r25.log build tools ':!build/desktop/patches' ':!*.png' ':!*.jpg'
set -uo pipefail
KITS="$(cd "$(dirname "$0")/../.." && pwd)/context/kits"
REPO=${1:?repo}; BASE=${2:?base}; OUT=${3:?out.md}; LOG=${4:?log}; shift 4
command -v codex >/dev/null || { echo "!! codex is not installed; ask the Cavekit inspector instead" >&2; exit 2; }
PROMPT='You are a senior engineer performing adversarial code review. Review the following diff for bugs, security issues, logic errors, and spec violations. For each finding output exactly one row in a markdown table with columns: Severity, File, Line, Description. Severity must be one of P0 (critical), P1 (high), P2 (medium), P3 (low). If no issues found, output exactly the word NO_FINDINGS on its own line and nothing else.'
CTX="Repository: $REPO (you may read any file in it). The requirements are in $KITS (start at cavekit-overview.md). The device shell is BusyBox 1.17 (no grep -x, sed -r not -E). The add-on site's server is Devuan with nginx, PHP-FPM 8.4, MariaDB and OpenRC. Do not search the web: the repository and the diff are the sources. Keep exploration to what a finding needs.${EXTRA:+ $EXTRA}"
# shellcheck disable=SC2086
DIFF=$(git -C "$REPO" diff --no-color ${DIFFOPTS:-} "$BASE".."${HEADREF:-HEAD}" -- "$@") \
  || { echo "!! git diff failed (is $BASE a commit of $REPO?); nothing reviewed" >&2; exit 2; }
# An empty diff would be reviewed as NO_FINDINGS: a silent pass of the gate. Refuse it.
[ -n "$DIFF" ] || { echo "!! the range $BASE..${HEADREF:-HEAD} is empty for these paths; nothing reviewed" >&2; exit 2; }
echo "diff lines: $(printf '%s\n' "$DIFF" | wc -l)"
printf '%s\n\n%s\n\nDIFF:\n%s\n' "$PROMPT" "$CTX" "$DIFF" \
  | codex exec -m "${CODEX_MODEL:-gpt-5.6-sol}" -s read-only -C "$REPO" -o "$OUT" - > "$LOG" 2>&1
RC=$?
echo "codex exit $RC"
exit $RC
