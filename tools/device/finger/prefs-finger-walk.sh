#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# about:settings by finger (cavekit-preferences-ui.md R6, docs/FINGER-TESTS.md; cavekit-finger-testing.md R3): every
# row of the page's own table (packaging/prefsui/content/preferences.js) is reached and changed with the emulated
# finger's tap on its control, then put back the same way, the engine pref read each time — prefs-row-walk.sh's walk
# with the card's input path in place of the inject channel's click. The pane tabs are tapped by finger too.
#
#   tools/device/finger/prefs-finger-walk.sh [enyo|mochi|mojo] [row-filter-regex]
#
# One device session a pane (the hand daemon stays up across them, each pane's script calibrates again): a session
# that hangs or is cut short costs one pane, not the walk.
#
# Since 2026-11-04 (T-563) the walk runs through the check library (fingerlib.sh) on all three shells: the geometry
# is the library's (the calibration tap on the page's title, every control's screen point from its rect, the card's
# scroll and the shell's page top, the picker rule, Mojo's header rule) and every wait is on an oracle (the clickAt,
# the picker's open and close, the pref's readback) — no hand-worked number and no sleep in this file. What the
# finger does: the tabs, the checkboxes, the pickers' opening tap and their rows, a field's focusing tap. What the
# inject channel does: the keys typed into a focused field, the reads (rect, getpref). The hand daemon of
# daemon-start.sh (the shipped GPU path plus the inject channel) runs the walk; prefs.js is compared before and after.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
. "$ROOT/build/webos-oe/host-path.sh"
V="${1:-enyo}"; FILTER="${2:-.}"
case "$V" in
  enyo)  ID=net.riverstonerelay.jihad-browser;;
  mochi) ID=net.riverstonerelay.jihad-browser-mochi;;
  mojo)  ID=net.riverstonerelay.jihad-browser-mojo;;
  *) echo "usage: $0 [enyo|mochi|mojo] [row-filter-regex]" >&2; exit 2;;
esac
D=${ROUND_DIR:-$ROOT/docs/rounds/finger-$(date +%F)}; mkdir -p "$D"
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT

# pane|group|row|pref|type|invert|option values|option value type — one line per table row.
node -e '
const fs = require("fs"), src = fs.readFileSync(process.argv[1], "utf8");
let i = src.indexOf("[", src.indexOf("const PANES = [")), d = 0, j = i;
for (; j < src.length; j++) { if (src[j] == "[") d++; else if (src[j] == "]" && --d == 0) break; }
// the table names the page'"'"'s own functions (identityText, blockerUpdate…): any name it uses is a stub here
const code = "(" + src.slice(i, j + 1) + ")", locals = ["src", "i", "j", "d", "fs", "code", "locals", "scope"];
const scope = new Proxy({}, {has: (t, k) => !(k in globalThis) && locals.indexOf(k) < 0, get: () => function () {}});
const PANES = (function () { with (scope) { return eval(code); } })();
for (const p of PANES) { if (p.chrome || !p.groups) continue;
  p.groups.forEach((g, gi) => g.rows.forEach((r, ri) => { if (!r.pref) return; console.log([p.id, gi + 1, ri + 1, r.pref, r.type,
    r.invert ? 1 : 0, (r.options || []).map(o => o[0]).join(","), typeof (r.options || [[0]])[0][0]].join("|")); })); }
' "$ROOT/packaging/prefsui/content/preferences.js" | grep -E -- "$FILTER" > "$TMP/rows.txt" || true
N=$(grep -c '' "$TMP/rows.txt")
[ "$N" -gt 0 ] || { echo "no row matches '$FILTER'" >&2; exit 2; }

{ echo "chmod +x /tmp/fbstream /tmp/finger 2>/dev/null"; cat "$HERE/fingerlib.sh"; cat <<'BODY'
V=$1; PANE=$3
P=/media/cryptofs/apps/usr/palm/applications/$2/profile/prefs.js
fl_init $V
[ -f /tmp/walk-prefs.before ] || cp $P /tmp/walk-prefs.before
KEY_END=57522
# the keys typed into the focused field, each one's echo awaited
key() { fl_ask "key $1" 'inject key ' 3 >/dev/null; }
text() { fl_ask "text $1" 'inject text ' 3 >/dev/null; }
bs() { local k; k=$1; while [ $k -gt 0 ]; do key 8; k=$((k-1)); done; }
fl_wait_rect "page-title" 20 || { echo "FATAL: about:settings shows no page-title"; exit 1; }
fl_calibrate "page-title"
CUR=""
grep "^$PANE|" /tmp/walk-rows.txt > /tmp/walk-pane.txt
while IFS='|' read pane g r pref type inv opts otype <&3; do
  if [ "$pane" != "$CUR" ]; then
    fl_card_to 0
    TAB="sel:button.pane-tab[data-pane=\"$pane\"]"
    if fl_tap "$TAB" && fl_wait_rect "sel:button.pane-tab.on[data-pane=\"$pane\"]" 5; then
      fl_check ok "tab-$pane" "tapped at $FL_TX,$FL_TY (clickAt $FL_HIT); the pane's first heading [$(fl_text 'sel:h2.group-title')]"
    else fl_check FAIL "tab-$pane" "the tab's tap did not show the pane"; fi
    CUR=$pane
  fi
  ROW="sel:.row[data-pref=\"$pref\"]"   # the row by its pref (preferences.js buildRow)
  case $type in bool) T=b; CTL="$ROW input";; int) T=i; CTL="$ROW input";; string) T=s; CTL="$ROW input";;
    choice) CTL="$ROW select"; [ "$otype" = number ] && T=i || T=s;; esac
  b0=$(fl_pref $T $pref); want="$b0"; s1=""
  case $type in
  bool)
    fl_tap "$CTL" || { fl_check FAIL "$pane $g.$r $pref ($type)" "no control"; continue; }; at="$FL_TX,$FL_TY"
    b1=$(fl_wait_pref $T $pref "$b0"); fl_tap "$CTL"; b2=$(fl_wait_pref $T $pref "$b1");;
  choice)
    N=$(echo "$opts" | awk -F, -v b="$b0" '{for(i=1;i<=NF;i++) if($i!=b){print i-1"|"$i; exit}}'); ni=${N%%|*}; nv=${N#*|}
    oi=$(echo "$opts" | awk -F, -v b="$b0" '{for(i=1;i<=NF;i++) if($i==b){print i-1; exit}}')
    NR=$(echo "$opts" | awk -F, '{print NF}')
    fl_pick "$CTL" $ni $NR || { fl_check FAIL "$pane $g.$r $pref ($type)" "the picker did not open or close"; continue; }; at="$FL_TX,$FL_TY"
    b1=$(fl_wait_pref $T $pref "$b0")
    if [ -n "$oi" ]; then fl_pick "$CTL" $oi $NR || s1="(the second pick did not open or close)"
    else fl_ask "setpref $T $pref $b0" 'inject setpref ' 3 >/dev/null; s1="(stored value is not one of the options; put back with setpref)"; fi
    b2=$(fl_wait_pref $T $pref "$b1");;
  int)
    nv=$((b0+1)); fl_tap "$CTL" || { fl_check FAIL "$pane $g.$r $pref ($type)" "no control"; continue; }; at="$FL_TX,$FL_TY"
    key $KEY_END; bs $(( ${#b0} + 1 )); text $nv; key 13; b1=$(fl_wait_pref $T $pref "$b0")
    key $KEY_END; bs $(( ${#nv} + 1 )); text $b0; key 13; b2=$(fl_wait_pref $T $pref "$b1");;
  string)
    fl_tap "$CTL" || { fl_check FAIL "$pane $g.$r $pref ($type)" "no control"; continue; }; at="$FL_TX,$FL_TY"
    key $KEY_END; text x; key 13; b1=$(fl_wait_pref $T $pref "$b0")
    key 8; key 13; b2=$(fl_wait_pref $T $pref "$b1")
    # a chrome:// default (a localized pointer) cannot be typed back: the readback's resolved text is the pass, the
    # pointer put back through the inject channel so the profile is left as found
    case "$b0" in chrome://*) want="${b1%x}"; fl_ask "setpref $T $pref $b0" 'inject setpref ' 3 >/dev/null; s1="(the chrome:// pointer put back with setpref)";; esac;;
  esac
  echo "$pref|$b0" >> /tmp/walk-b0.txt
  if [ "$b1" != "$b0" ] && [ "$b2" = "$want" ]; then fl_check ok "$pane $g.$r $pref ($type)" "tapped at $at: [$b0] -> [$b1] -> [$b2] $s1"
  else
    fl_check FAIL "$pane $g.$r $pref ($type)" "tapped at $at: [$b0] -> [$b1] -> [$b2] $s1"
    # the device left as found: the value put back through the inject channel when the finger's put-back failed
    [ "$b2" = "$want" ] || { fl_ask "setpref $T $pref $want" 'inject setpref ' 3 >/dev/null; echo "  put back with setpref: [$(fl_pref $T $pref)]"; }
  fi
done 3< /tmp/walk-pane.txt
fl_summary; RC=$?
if cmp -s $P /tmp/walk-prefs.before; then echo "prefs.js now: identical to before (the daemon has not saved since)"; fi
exit $RC
BODY
} > "$TMP/walk.sh"
# the sampler from its source (no binary in the tree), pushed every run: 48 KB (fl_pick measures Mojo's popup with it)
TC=$ROOT/build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin
$TC/arm-webos-linux-gnueabi-gcc -O2 -march=armv7-a -mfpu=neon -mfloat-abi=softfp -std=gnu99 -o "$TMP/fbstream" "$HERE/fbstream.c" || exit 1
novacom put file:///tmp/fbstream < "$TMP/fbstream"
novacom put file:///tmp/walk-rows.txt < "$TMP/rows.txt"
novacom run file://bin/rm -- -f /tmp/walk-prefs.before /tmp/walk-b0.txt </dev/null >/dev/null 2>&1
"$ROOT/build/webos-oe/dev-sh.sh" -t 90 "$HERE/daemon-start.sh" "$V" "about:settings" 2>&1 | tail -n 2
echo "walking $N row(s) of about:settings by finger in $V"
OUT="$D/prefs-finger-walk-$V.txt"; : > "$OUT"
# one device session a pane (the daemon stays up across them; each pane's script calibrates again): a session that
# hangs or is cut short costs one pane, not the walk
for PANE in $(cut -d'|' -f1 "$TMP/rows.txt" | awk '!seen[$0]++'); do
  NP=$(grep -c "^$PANE|" "$TMP/rows.txt")
  echo "=== pane $PANE ($NP rows)" | tee -a "$OUT"
  "$ROOT/build/webos-oe/dev-sh.sh" -t $(( NP * 40 + 120 )) "$TMP/walk.sh" "$V" "$ID" "$PANE" 2>&1 | tee -a "$OUT" || echo "(the pane's session ended with status ${PIPESTATUS[0]})" | tee -a "$OUT"
done
for f in $(grep -a -o '/media/internal/jihad-finger/[^ ]*\.last\.raw' "$OUT" | sort -u); do
  novacom get "file://$f" > "$D/walk-kept-$V-$(basename "$f" .last.raw).raw" && echo "pulled $f"
done
novacom run file://bin/rm -- -rf /media/internal/jihad-finger </dev/null >/dev/null 2>&1
"$ROOT/build/webos-oe/dev-sh.sh" -t 90 "$HERE/daemon-stop.sh" "$V" 2>/dev/null | tail -n 1
cat > "$TMP/cmp.sh" <<EOS
# every walked pref, before the walk and now (a line each way: the daemon rewrites the file in its own order); a pref
# that had no user line (its default) and has one now — the engine writes a user line once a pref is set, even back
# to its default — is checked against the value the engine read before the walk (/tmp/walk-b0.txt)
P=/media/cryptofs/apps/usr/palm/applications/$ID/profile/prefs.js
n=0; m=0
while IFS='|' read pane g r pref rest; do
  a=\$(grep "\"\$pref\"" /tmp/walk-prefs.before | head -1); b=\$(grep "\"\$pref\"" \$P | head -1)
  if [ -z "\$a" ] && [ -n "\$b" ]; then
    v=\$(echo "\$b" | sed 's/.*", *//; s/);\$//; s/^"//; s/"\$//'); b0=\$(grep "^\$pref|" /tmp/walk-b0.txt | head -1 | cut -d'|' -f2-)
    if [ "\$v" = "\$b0" ]; then echo "NEW LINE \$pref: now [\$b], the value the engine read before (\$b0)"; m=\$((m+1))
    else echo "CHANGED \$pref: no user line before, the engine read [\$b0]; now [\$b]"; n=\$((n+1)); fi
  elif [ "\$a" != "\$b" ]; then echo "CHANGED \$pref: before [\$a] now [\$b]"; n=\$((n+1)); fi
done < /tmp/walk-rows.txt
echo "prefs left changed by the walk: \$n; user lines added at their old value: \$m"
EOS
"$ROOT/build/webos-oe/dev-sh.sh" -t 30 "$TMP/cmp.sh" | tee -a "$OUT"
NPANES=$(cut -d'|' -f1 "$TMP/rows.txt" | awk '!seen[$0]++' | wc -l)
echo "panes $NPANES, ended $(grep -c '^CHECKS' "$OUT"); checks $(grep -c '^CHECK ok' "$OUT") ok, $(grep -c '^CHECK FAIL' "$OUT") failed" | tee -a "$OUT"
[ "$(grep -c '^CHECKS' "$OUT")" = "$NPANES" ] || { echo "a pane's session did not reach its end" >&2; exit 1; }
! grep -q '^CHECK FAIL' "$OUT"
