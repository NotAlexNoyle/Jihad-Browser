#!/bin/bash
# Walk the rows of about:preferences on the device: for each row of the page's own table, read
# the engine pref, change it THROUGH THE ROW'S CONTROL, read it again, put it back the same way
# and read it a third time. This is the check behind cavekit-preferences-ui.md R2 ("every
# control maps to a real engine preference and changes it") — a row that shows a control and
# changes nothing fails here.
#
#   prefs-row-walk.sh [enyo|mochi|mojo] [row-filter-regex]
#
# The row table is taken out of packaging/prefsui/content/preferences.js, so a row added there
# is walked without touching this script. The whole table takes about 25 minutes; the filter
# (matched against "pane|group|row|pref|type") runs a part of it, e.g. 'privacy' or '\|string\|'.
#
# Needs JIHAD_INJECT=1 in the variant's job (the inject channel) and the display on. It restarts
# the daemon at the start and at the end: prefs.js is copied before the walk and compared after
# it, and the copy is put back if a row was left changed.
set -euo pipefail
. "$(dirname "$0")/host-path.sh"   # the repository's own novacom and palm-* first (R11)
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
V="${1:-enyo}"; FILTER="${2:-.}"
case "$V" in
  enyo)  ID=net.riverstonerelay.jihad-browser;       JOB=jihad;;
  mochi) ID=net.riverstonerelay.jihad-browser-mochi; JOB=jihad-mochi;;
  mojo)  ID=net.riverstonerelay.jihad-browser-mojo;  JOB=jihad-mojo;;
  *) echo "usage: $0 [enyo|mochi|mojo] [row-filter-regex]" >&2; exit 2;;
esac
NOVACOM=(novacom)
[ -n "${JIHAD_DEVICE:-}" ] && NOVACOM=(novacom -d "$JIHAD_DEVICE")
TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT

# pane|group|row|pref|type|invert|option values|option value type — one line per table row.
node -e '
const fs = require("fs"), src = fs.readFileSync(process.argv[1], "utf8");
let i = src.indexOf("[", src.indexOf("const PANES = [")), d = 0, j = i;
for (; j < src.length; j++) { if (src[j] == "[") d++; else if (src[j] == "]" && --d == 0) break; }
const PANES = eval("(" + src.slice(i, j + 1) + ")");
for (const p of PANES) { if (p.chrome || !p.groups) continue;
  p.groups.forEach((g, gi) => g.rows.forEach((r, ri) => console.log([p.id, gi + 1, ri + 1, r.pref, r.type,
    r.invert ? 1 : 0, (r.options || []).map(o => o[0]).join(","), typeof (r.options || [[0]])[0][0]].join("|")))); }
' "$ROOT/packaging/prefsui/content/preferences.js" | grep -E -- "$FILTER" > "$TMP/rows.txt" || true
N=$(grep -c '' "$TMP/rows.txt")
[ "$N" -gt 0 ] || { echo "no row matches '$FILTER'" >&2; exit 2; }

cat > "$TMP/walk.sh" <<'EOF'
#!/bin/sh
V=$1; ID=$2; JOB=$3
L=/var/palm/jihad/$V/daemon.log; I=/var/palm/jihad/$V/inject.cmd
P=/media/cryptofs/apps/usr/palm/applications/$ID/profile/prefs.js
seg() { E=$(wc -c < $L); tail -c $((E-$1)) $L; }
inj() { B=$(wc -c < $L); echo "$1" > $I; sleep ${2:-1}; }
gp() { inj "getpref $1 $2" 1; seg $B | grep 'inject getpref' | tail -1 | sed 's/.*= \[\(.*\)\]$/\1/'; }
st() { inj "gettext #status" 1; seg $B | grep 'inject gettext' | tail -1 | sed 's/.*#status = \[\(.*\)\]$/\1/' | cut -c1-70; }
# rect answers in viewport CSS px; with the page scrolled to 0,0 that is also document px, which
# is what `click` takes (the daemon subtracts the scroll). about:preferences is laid out at zoom 1.
cen() { inj "rect $1" 1; seg $B | grep 'inject rect' | tail -1 | sed -n 's/.*center=\([0-9]*\),\([0-9]*\).*/\1 \2/p'; }
tapc() { set -- $1; [ "$2" -gt 300 ] && inj "scroll 0 $(($2-200))" 1; inj "click $1 $2 1" 2; }
bs() { n=$1; while [ $n -gt 0 ]; do inj "key 8" 1; n=$((n-1)); done; }
popup() { seg $B | grep 'popupMenuShow id=' | tail -1 | sed 's/.*id=\([^ ]*\) .*/\1/'; }
KEY_END=57522   # 0xE0B2, measured (BrowserPageGoanna::keyDown)

grep -q 'JIHAD_INJECT=1' /etc/event.d/$JOB || { echo "no JIHAD_INJECT=1 in /etc/event.d/$JOB — the inject channel is off"; exit 1; }
stop $JOB >/dev/null 2>&1; sleep 3; cp $P /tmp/walk-prefs.before; start $JOB >/dev/null 2>&1; sleep 5
luna-send -n 1 palm://com.palm.display/control/setState '{"state":"on"}' >/dev/null 2>&1
luna-send -n 1 palm://com.palm.display/control/setState '{"state":"unlock"}' >/dev/null 2>&1
for p in $(luna-send -n 1 palm://com.palm.applicationManager/running '{}' 2>&1 | tr '{' '\n' | grep "\"id\": *\"$ID\"" | sed 's/.*"processid": *"\([0-9]*\)".*/\1/'); do
  luna-send -n 1 palm://com.palm.applicationManager/close "{\"processId\":\"$p\"}" >/dev/null 2>&1; done
sleep 4
A=$(wc -c < $L); try=1
while :; do   # LunaSysMgr sometimes drops a launch issued right after a close; retry once
  luna-send -n 1 palm://com.palm.applicationManager/launch "{\"id\":\"$ID\",\"params\":{\"target\":\"about:preferences\"}}" >/dev/null
  i=0; while [ $i -lt 20 ]; do seg $A | grep -q 'load done' && break; sleep 1; i=$((i+1)); done
  [ $i -lt 20 ] && break; [ $try -ge 2 ] && break; try=$((try+1))
done
sleep 3
echo "#### $V: card connected=$(seg $A | grep -c 'client connected') page loaded=$(seg $A | grep -c 'load done') daemon=$(md5sum /media/cryptofs/apps/usr/palm/applications/$ID/deviceroot/hl/jihad-browserserver | cut -c1-12)"
[ "$(seg $A | grep -c 'client connected')" -ge 1 ] || { echo "no card connected — nothing below would mean anything"; start $JOB >/dev/null 2>&1; exit 1; }
OKN=0; BAD=0; CUR=""
while IFS='|' read pane g r pref type inv opts otype <&3; do
  if [ "$pane" != "$CUR" ]; then inj "scroll 0 0" 1; C=$(cen "sel:button[data-pane=\"$pane\"]"); inj "click $C 1" 2; CUR=$pane; fi
  ROW="sel:#content section:nth-of-type($g) .row:nth-of-type($r)"
  case $type in bool) T=b; CTL="$ROW input";; int) T=i; CTL="$ROW input";; string) T=s; CTL="$ROW input";;
    choice) CTL="$ROW select"; [ "$otype" = number ] && T=i || T=s;; esac
  inj "scroll 0 0" 1; C=$(cen "$CTL"); b0=$(gp $T $pref); want="$b0"
  if [ -z "$C" ]; then echo "FAIL $pane $g.$r $pref ($type): no control"; BAD=$((BAD+1)); continue; fi
  case $type in
  bool)
    tapc "$C"; s1=$(st); b1=$(gp $T $pref); tapc "$C"; b2=$(gp $T $pref);;
  choice)
    N=$(echo "$opts" | awk -F, -v b="$b0" '{for(i=1;i<=NF;i++) if($i!=b){print i-1"|"$i; exit}}'); ni=${N%%|*}; nv=${N#*|}
    oi=$(echo "$opts" | awk -F, -v b="$b0" '{for(i=1;i<=NF;i++) if($i==b){print i-1; exit}}')
    tapc "$C"; inj "popupsel $(popup) $ni" 2; s1=$(st); b1=$(gp $T $pref)
    [ "$b1" = "$nv" ] || s1="$s1 (wanted $nv)"
    if [ -n "$oi" ]; then sleep 1; inj "scroll 0 0" 1; tapc "$C"; inj "popupsel $(popup) $oi" 2
    else inj "setpref $T $pref $b0" 1; s1="$s1 (stored value is not one of the options; put back with setpref)"; fi
    b2=$(gp $T $pref);;
  int)
    nv=$((b0+1)); tapc "$C"; inj "key $KEY_END" 1; bs $(( ${#b0} + 1 )); inj "text $nv" 1; inj "key 13" 2; s1=$(st); b1=$(gp $T $pref)
    inj "key $KEY_END" 1; bs $(( ${#nv} + 1 )); inj "text $b0" 1; inj "key 13" 2; b2=$(gp $T $pref);;
  string)
    # Append a character and take it away again. The stored value is not used to decide what to
    # type back: a LOCALIZED pref (intl.accept_languages) stores a chrome:// URL while the field
    # shows the list, so its restored value is the list itself, as a user value.
    tapc "$C"; inj "key $KEY_END" 1; inj "text x" 1; inj "key 13" 2; s1=$(st); b1=$(gp $T $pref)
    inj "key 8" 1; inj "key 13" 2; b2=$(gp $T $pref)
    case "$b0" in chrome://*) want="${b1%x}";; esac;;
  esac
  if [ "$b1" != "$b0" ] && [ "$b2" = "$want" ]; then OKN=$((OKN+1)); R=ok; else BAD=$((BAD+1)); R=FAIL; fi
  echo "$R $pane $g.$r $pref ($type): [$b0] -> [$b1] -> [$b2] | $s1"
done 3< /tmp/walk-rows.txt
echo "rows changed and put back through their own control: $OKN ; not: $BAD ; inject without a page: $(seg $A | grep -c 'inject: no page') ; log grew $(( $(wc -c < $L) - A )) bytes"
stop $JOB >/dev/null 2>&1; sleep 3
if cmp -s $P /tmp/walk-prefs.before; then echo "prefs.js after the walk: identical to before"
else echo "prefs.js after the walk differs (a localized row leaves its list as a user value) — the copy was put back:"
  grep -v -F -f /tmp/walk-prefs.before $P | cut -c1-150 | head -8; cp /tmp/walk-prefs.before $P; fi
start $JOB >/dev/null 2>&1; sleep 4; initctl list | grep "$JOB (" | sed 's/^([^)]*) //'
EOF

"${NOVACOM[@]}" put file:///tmp/walk-rows.txt < "$TMP/rows.txt"
"${NOVACOM[@]}" put file:///tmp/walk.sh < "$TMP/walk.sh"
echo "walking $N row(s) of about:preferences on $V — about $(( (N * 26 + 90) / 60 + 1 )) min"
# novacom ends the remote command when its stdin closes, so stdin is held open by a sleep that
# outlasts the walk and is ended as soon as the walk is.
exec 3< <(sleep $(( N * 40 + 180 ))); HOLD=$!
"${NOVACOM[@]}" run file://bin/sh -- /tmp/walk.sh "$V" "$ID" "$JOB" <&3 2>&1 | tee "$TMP/out.txt"
kill "$HOLD" 2>/dev/null || true; exec 3<&-
grep -q '^rows changed' "$TMP/out.txt" || { echo "the walk did not reach its end" >&2; exit 1; }
! grep -q '^FAIL' "$TMP/out.txt"
