#!/bin/bash
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# about:settings by finger (cavekit-preferences-ui.md R6, docs/FINGER-TESTS.md): every row of the page's own
# table (packaging/prefsui/content/preferences.js) is reached and changed with the emulated finger's tap on
# its control, then put back the same way, the engine pref read each time — prefs-row-walk.sh's walk with the
# card's input path in place of the inject channel's click. The pane tabs are tapped by finger too.
#
#   tools/device/finger/prefs-finger-walk.sh [enyo|mochi|mojo] [row-filter-regex]
#
# Runs on the spike daemon (tools/device/t502/gpu-spike-start.sh: the shipped GPU path, the inject channel on;
# Enyo only, which is the variant the spike script starts). What the finger does: the tabs, the checkboxes, the
# pickers' opening tap, a field's focusing tap. What the inject channel does: the pick inside an open picker
# (popupsel), the keys typed into a focused field, the reads (rect, getpref) and the card's scroll (cardscroll:
# the card's own view, as for a page's own scroll). A tap's screen point is the control's document centre less
# the card's scroll, plus the page's top on the screen, measured first from one tap on the page's title.
# Needs /tmp/finger on the device (tools/device/finger/README.md). prefs.js is compared before and after.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../../.." && pwd)"
. "$ROOT/build/webos-oe/host-path.sh"
V="${1:-enyo}"; FILTER="${2:-.}"
[ "$V" = enyo ] || { echo "the spike daemon is Enyo's: run this with enyo" >&2; exit 2; }
ID=net.riverstonerelay.jihad-browser
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

cat > "$TMP/walk.sh" <<'EOF'
#!/bin/sh
ID=$1
L=/var/palm/jihad/enyo/daemon-gpu.log; I=/var/palm/jihad/enyo/inject.cmd
P=/media/cryptofs/apps/usr/palm/applications/$ID/profile/prefs.js
seg() { E=$(wc -c < $L); tail -c $((E-$1)) $L; }
inj() { B=$(wc -c < $L); echo "$1" > $I; sleep ${2:-1}; }
gp() { inj "getpref $1 $2" 1; seg $B | grep 'inject getpref' | tail -1 | sed 's/.*= \[\(.*\)\]$/\1/'; }
# "docX docY cardX cardY" for a selector: the rect's centre plus the engine's scroll (document px), and the card's scroll
loc() { inj "rect $1" 1; seg $B | grep 'inject rect' | tail -1 | sed -n 's/.*scroll=\(-*[0-9]*\),\(-*[0-9]*\) rect=.* center=\(-*[0-9]*\),\(-*[0-9]*\) card=\(-*[0-9]*\),\(-*[0-9]*\).*/\1 \2 \3 \4 \5 \6/p' | awk '{print $1+$3, $2+$4, $5, $6}'; }
popup() { seg $B | grep 'popupMenuShow id=' | tail -1 | sed 's/.*id=\([^ ]*\) .*/\1/'; }
bs() { n=$1; while [ $n -gt 0 ]; do inj "key 8" 1; n=$((n-1)); done; }
KEY_END=57522
# the finger's tap on a selector: the card scrolled so the control sits a third down the card, then the
# tap; TX,TY the screen point tapped (run in this shell, not in a $(...), so B and TX/TY stay set)
ftap() {
  set -- $(loc "$1"); [ -n "${2:-}" ] || { TX=""; return 1; }
  dx=$1; dy=$2; cy=$4
  if [ $((dy - cy)) -lt 80 ] || [ $((dy - cy)) -gt 600 ]; then
    inj "cardscroll 0 $(( dy > 300 ? dy - 300 : 0 ))" 2; set -- $(loc "$SEL"); dx=$1; dy=$2; cy=$4
  fi
  TX=$dx; TY=$((TOP + dy - cy))
  B=$(wc -c < $L); /tmp/finger tap $TX $TY; sleep 2
}
# a picker's item by finger: the card's own list opens below the select, centred under it (a wide one
# reaches the screen's right edge and still spans the select's middle), 44 px rows, the first 38 px under
# the select's bottom edge (measured on the Enyo card 2026-10-25)
fpick() { /tmp/finger tap $TX $(( TY + 22 + 38 + 44 * $1 )); sleep 2; }
luna-send -n 1 palm://com.palm.display/control/setState '{"state":"on"}' >/dev/null 2>&1
cp $P /tmp/walk-prefs.before
sleep 3
# the page's top on the screen: one tap on the title, the daemon's clickAt says where it landed
set -- $(loc "page-title"); ty=$2
B=$(wc -c < $L); /tmp/finger tap $1 $((ty + 82)); sleep 2
got=$(seg $B | grep 'clickAt x=' | tail -1 | sed -n 's/.*clickAt x=\([0-9]*\) y=\([0-9]*\).*/\2/p')
[ -n "$got" ] || { echo "no clickAt for the calibration tap"; exit 1; }
TOP=$((ty + 82 - got))
echo "#### the page's top on the screen: $TOP (title at document y $ty, the tap at $((ty + 82)) landed on $got)"
OKN=0; BAD=0; CUR=""
while IFS='|' read pane g r pref type inv opts otype <&3; do
  if [ "$pane" != "$CUR" ]; then
    inj "cardscroll 0 0" 2; SEL="sel:button[data-pane=\"$pane\"]"; ftap "$SEL"; CUR=$pane
    echo "tab $pane tapped at $TX,$TY"
  fi
  ROW="sel:.row[data-pref=\"$pref\"]"   # the row by its pref (preferences.js buildRow)
  case $type in bool) T=b; CTL="$ROW input";; int) T=i; CTL="$ROW input";; string) T=s; CTL="$ROW input";;
    choice) CTL="$ROW select"; [ "$otype" = number ] && T=i || T=s;; esac
  SEL="$CTL"; b0=$(gp $T $pref); want="$b0"; s1=""
  case $type in
  bool)
    ftap "$CTL" || { echo "FAIL $pane $g.$r $pref ($type): no control"; BAD=$((BAD+1)); continue; }; at="$TX,$TY"
    b1=$(gp $T $pref); ftap "$CTL"; b2=$(gp $T $pref);;
  choice)
    N=$(echo "$opts" | awk -F, -v b="$b0" '{for(i=1;i<=NF;i++) if($i!=b){print i-1"|"$i; exit}}'); ni=${N%%|*}; nv=${N#*|}
    oi=$(echo "$opts" | awk -F, -v b="$b0" '{for(i=1;i<=NF;i++) if($i==b){print i-1; exit}}')
    ftap "$CTL" || { echo "FAIL $pane $g.$r $pref ($type): no control"; BAD=$((BAD+1)); continue; }; at="$TX,$TY"
    [ -n "$(popup)" ] || s1="the tap opened no picker"
    fpick $ni; b1=$(gp $T $pref)
    if [ -n "$oi" ]; then ftap "$CTL"; fpick $oi
    else inj "setpref $T $pref $b0" 1; s1="$s1 (stored value is not one of the options; put back with setpref)"; fi
    b2=$(gp $T $pref);;
  int)
    nv=$((b0+1)); ftap "$CTL" || { echo "FAIL $pane $g.$r $pref ($type): no control"; BAD=$((BAD+1)); continue; }; at="$TX,$TY"
    inj "key $KEY_END" 1; bs $(( ${#b0} + 1 )); inj "text $nv" 1; inj "key 13" 2; b1=$(gp $T $pref)
    inj "key $KEY_END" 1; bs $(( ${#nv} + 1 )); inj "text $b0" 1; inj "key 13" 2; b2=$(gp $T $pref);;
  string)
    ftap "$CTL" || { echo "FAIL $pane $g.$r $pref ($type): no control"; BAD=$((BAD+1)); continue; }; at="$TX,$TY"
    inj "key $KEY_END" 1; inj "text x" 1; inj "key 13" 2; b1=$(gp $T $pref)
    inj "key 8" 1; inj "key 13" 2; b2=$(gp $T $pref)
    case "$b0" in chrome://*) want="${b1%x}";; esac;;
  esac
  if [ "$b1" != "$b0" ] && [ "$b2" = "$want" ]; then OKN=$((OKN+1)); R=ok; else BAD=$((BAD+1)); R=FAIL; fi
  echo "$R $pane $g.$r $pref ($type) tapped at $at: [$b0] -> [$b1] -> [$b2] $s1"
done 3< /tmp/walk-rows.txt
echo "rows changed and put back by the finger on their own control: $OKN ; not: $BAD"
if cmp -s $P /tmp/walk-prefs.before; then echo "prefs.js now: identical to before (the daemon has not saved since)"; fi
EOF

novacom put file:///tmp/walk-rows.txt < "$TMP/rows.txt"
"$ROOT/build/webos-oe/dev-sh.sh" -t 90 "$ROOT/tools/device/t502/gpu-spike-start.sh" "about:settings" "JIHAD_INJECT=1 PREF=jihad.viewport.followOverflow=true" 2>&1 | grep 'screenshot at\|FATAL'
echo "walking $N row(s) of about:settings by finger"
"$ROOT/build/webos-oe/dev-sh.sh" -t $(( N * 45 + 300 )) "$TMP/walk.sh" "$ID" 2>&1 | tee "$TMP/out.txt"
# the spike daemon stopped, the run's prefs stripped, the upstart job back; then prefs.js compared
"$ROOT/build/webos-oe/dev-sh.sh" -t 90 "$ROOT/tools/device/t502/gpu-spike-collect-stop.sh" 2>/dev/null | tail -n 2
cat > "$TMP/cmp.sh" <<'EOF'
# every walked pref, before the walk and now (a line each way: the daemon rewrites the file in its own order)
P=/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/profile/prefs.js
n=0
while IFS='|' read pane g r pref rest; do
  a=$(grep "\"$pref\"" /tmp/walk-prefs.before | head -1); b=$(grep "\"$pref\"" $P | head -1)
  [ "$a" = "$b" ] || { echo "CHANGED $pref: before [$a] now [$b]"; n=$((n+1)); }
done < /tmp/walk-rows.txt
echo "prefs left changed by the walk: $n"
EOF
"$ROOT/build/webos-oe/dev-sh.sh" -t 30 "$TMP/cmp.sh"
grep -q '^rows changed' "$TMP/out.txt" || { echo "the walk did not reach its end" >&2; exit 1; }
! grep -q '^FAIL' "$TMP/out.txt"
