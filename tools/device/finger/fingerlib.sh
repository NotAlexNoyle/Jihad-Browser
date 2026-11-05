# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# fingerlib.sh — the check library and the geometry of the finger rounds (cavekit-finger-testing.md R1, R3; T-563).
# DEVICE SIDE, busybox sh. A test's host wrapper puts this file ahead of the test's own body and runs the whole as one
# script in one novacom session (build/webos-oe/dev-sh.sh), against the hand daemon daemon-start.sh started for the
# shell (the inject channel on, the GPU path with the express lane and its log, as shipped). Nothing here judges by
# eye or by brightness: every verdict reads an oracle (the daemon log, the inject channel's answers, the window
# server's syslog) or the shown frame's structure (fbstream -Q: distinct colours, a region's hash, one-colour rows).
#
#   fl_init VARIANT                 enyo | mochi | mojo: the ids, the log, the shell's chrome table and page top
#   fl_calibrate SEL [TOL]          the page-top calibration tap on a known element: FL_TOP measured from the daemon's
#                                   clickAt; off the table's constant by more than TOL (10) px: the round STOPS (exit 3)
#   fl_screen SEL                   "X Y": the element's centre on the screen (its rect, the engine's scroll, the zoom,
#                                   the card's scroll, the calibrated top, Mojo's header rule) — re-read every time;
#                                   fl_pull_header brings Mojo's address bar back (a drag down at the page's top)
#   fl_tap SEL | fl_tap_at X Y      a finger tap on an element (scrolled into the view first) or a screen point, then
#                                   the clickAt read back (FL_HIT); fl_chrome_tap NAME taps the shell's table entry
#   fl_pick SEL INDEX [ROWS]        a <select>'s INDEXth option through the card's own picker (the picker rule: the
#                                   select mid-screen, the list waited for by a still frame region, the row tapped —
#                                   under the select on Enyo and Mochi, in Mojo's centred popup by the row count)
#   fl_wait_still REGION [BOUND]    a frame region unchanged over 150 ms (the card's own animations' oracle)
#   fl_drag X0 Y0 X1 Y1 [MS]        a drag, then the card's scroll settled (two equal reads)
#   fl_card_to Y                    the card's own view to Y (cardscroll), settled
#   fl_load URL [BOUND]             a page through the inject channel, its load done and a content frame painted
#   fl_wait PATTERN [BOUND] [sys]   a line since the mark (fl_mark) in the daemon log (or the syslog), BOUND s at most
#   fl_wait_pref T NAME OLD [BOUND] a pref read until it differs from OLD; fl_wait_rect SEL [BOUND]: an element present
#   fl_check_scroll NAME EY KY TOL  the engine's and the card's scroll y within TOL; fl_check_card_scroll NAME KY TOL
#   fl_check_zoom NAME Z TOL        the zoom (the last frame's mZoom)
#   fl_check_hit NAME SEL           the last tap's clickAt landed inside the element's rect
#   fl_check_top NAME SEL [TOL]     the page top the model has now against a tap's clickAt (after a gesture)
#   fl_check_element_at NAME SEL X Y   the element's screen rect holds the point
#   fl_check_text NAME SEL TEXT     the element's text is TEXT; fl_check_text_has: holds TEXT
#   fl_check_url NAME PART          the last load done uri holds PART
#   fl_check_dialog NAME present|answered [BOUND]   the daemon's dialog line since the mark
#   fl_check_keyboard NAME up|down [BOUND]           the window server's IME line for this app since the mark
#   fl_kb                           the keyboard now (1 up, 0 down): the last IME line; fl_bring keeps taps above it
#   fl_check_content NAME REGION SEL [TEXT]   the region shows content (8+ distinct colours) AND the element's
#                                   screen rect meets the region (and its text holds TEXT when given) — never darkness
#   fl_sample NAME REGION           remember the region's hash; fl_check_region_equal / fl_check_region_changed NAME REGION
#                                   A REGION is "X0,Y0,X1,Y1" (portrait screen px) or an element (sel:css / id): its
#                                   screen rect as it is now, cut at column 756 (the card's scroll indicator stands in
#                                   757..767 and fades after a scroll) less one row at its top and bottom (the edge's
#                                   shade differs by a few units where it meets the card's chrome: Mojo's header,
#                                   measured 2026-11-04) — a fixed part is checked where it now is
#   fl_summary                      "CHECKS n FAILS m"; exit 1 when a check failed
# A check prints "CHECK ok NAME: evidence" or "CHECK FAIL NAME: evidence" and, failing, keeps the shown frame under
# $FL_KEEP (NAME.last.raw: the host wrapper pulls it); a passed check keeps nothing.

# busybox sh: one namespace for every function's variables — each function here declares its own `local`; a test's
# body does the same for its loop counters (2026-11-04: a nested wait reset a caller's counter, backspaces forever)
FL_FAILS=0; FL_CHECKS=0; FL_HIT=""; FL_TX=""; FL_TY=""
# Mojo's header rule: the card's address bar (45 px) scrolls away with the page's first move (a drag, a flick, a
# cardscroll that moves the view) and stays away — a cardscroll back to 0 leaves it hidden (the card keeps the bar
# above its 0) — until a page loads or a drag down at the page's top pulls it back (fl_pull_header). FL_HDR says
# whether it is hidden now; the geometry subtracts FL_HEADER while it is. Measured on the long page, 2026-11-04.
FL_HDR=0

fl_init() {
  FL_V=$1
  case $FL_V in
    enyo)  FL_ID=net.riverstonerelay.jihad-browser;       FL_JOB=jihad;       FL_TOP0=82; FL_HEADER=0;;
    mochi) FL_ID=net.riverstonerelay.jihad-browser-mochi; FL_JOB=jihad-mochi; FL_TOP0=78; FL_HEADER=0;;
    mojo)  FL_ID=net.riverstonerelay.jihad-browser-mojo;  FL_JOB=jihad-mojo;  FL_TOP0=73; FL_HEADER=45;;
    *) echo "fl_init: unknown variant '$1'"; exit 2;;
  esac
  # FL_PLANT_TOP: a planted error in the table's page-top constant (the calibration's own test, R3)
  FL_TOP0=$((FL_TOP0 + ${FL_PLANT_TOP:-0})); FL_TOP=$FL_TOP0
  FL_APP=/media/cryptofs/apps/usr/palm/applications/$FL_ID; FL_STATE=/var/palm/jihad/$FL_V
  FL_LOG=${FL_LOG:-$FL_STATE/daemon-gpu.log}; FL_INJ=$FL_STATE/inject.cmd; FL_SYS=/var/log/messages
  FL_KEEP=${FL_KEEP:-/media/internal/jihad-finger}; mkdir -p $FL_KEEP
  # the picker rule. Enyo and Mochi: the card's list opens under its <select>, the first row's centre FL_PICK0 px
  # under the select's centre, FL_PICKROW px a row (measured 2026-10-25). Mojo: a popup centred on the card area
  # wherever the select is — x FL_PICK_CX, the rows FL_PICKROW tall around y FL_PICK_CY (measured 2026-11-04 on
  # lists of 3 and 4 rows: 243..523 wide, rows' centres 480/531/582 and 454/505/556/607) — so fl_pick needs the
  # row count there; the daemon never counts that popup open (popups=0), the shown frame is its oracle
  # (the popup sits lower with the keyboard down and higher with it up: fl_pick measures its rows in the frame)
  case $FL_V in mojo) FL_PICK_CX=383; FL_PICK_CY=531; FL_PICKROW=51; FL_PICK0=0;; *) FL_PICK0=60; FL_PICKROW=44; FL_PICK_CX=0; FL_PICK_CY=0;; esac
  [ -x /tmp/finger ] || { echo "FATAL: no /tmp/finger (tools/device/finger/README.md)"; exit 2; }
  [ -x /tmp/fbstream ] || { echo "FATAL: no /tmp/fbstream (tools/device/finger/README.md)"; exit 2; }
  [ -f "$FL_LOG" ] || { echo "FATAL: no daemon log $FL_LOG (daemon-start.sh $FL_V first)"; exit 2; }
  fl_mark
}

# the shell's chrome controls (portrait screen px; measured on the 2026-11-04 screenshots of each shell on a page,
# checked by fl_chrome_check): the toolbar of Enyo and Mochi, Mojo's address bar (it scrolls away with the page: the
# card is put at its top first) and its bottom command bar
fl_chrome() {
  case $FL_V:$1 in
    enyo:back) echo 30 52;;  enyo:forward) echo 78 52;;  enyo:home) echo 130 52;;  enyo:address) echo 385 52;;
    enyo:reload) echo 582 52;; enyo:share) echo 640 52;; enyo:newcard) echo 690 52;; enyo:history) echo 740 52;;
    mochi:back) echo 28 52;; mochi:forward) echo 74 52;; mochi:home) echo 120 52;; mochi:address) echo 385 52;;
    mochi:reload) echo 596 52;; mochi:share) echo 646 52;; mochi:newcard) echo 692 52;; mochi:history) echo 738 52;;
    mojo:address) echo 385 51;; mojo:reload) echo 740 51;; mojo:back) echo 222 983;; mojo:forward) echo 276 983;;
    mojo:home) echo 330 983;; mojo:newcard) echo 384 983;; mojo:history) echo 438 983;; mojo:share) echo 492 983;;
    mojo:search) echo 546 983;;
    *:usbclose) echo 583 232;;   # the system's "Connected" alert's Close after a LunaSysMgr restart (fb0)
    *) echo "fl_chrome: no '$1' in the $FL_V table" >&2; return 1;;
  esac
}

# --- marks and waits ---------------------------------------------------------------------------------------------
fl_mark() { FL_B=$(wc -c < $FL_LOG); if [ -f $FL_SYS ]; then FL_SB=$(wc -c < $FL_SYS); else FL_SB=0; fi; }
fl_since() { E=$(wc -c < $FL_LOG); [ "$E" -gt "$FL_B" ] && tail -c $((E - FL_B)) $FL_LOG; }
# the syslog rotates under a round (messages -> messages.0 and a new file): a file shorter than the mark is read whole
fl_syssince() { [ -f $FL_SYS ] || return 0; E=$(wc -c < $FL_SYS); if [ "$E" -lt "$FL_SB" ]; then cat $FL_SYS; elif [ "$E" -gt "$FL_SB" ]; then tail -c $((E - FL_SB)) $FL_SYS; fi; }
# the last line matching PATTERN since the mark, within BOUND seconds (five reads a second); 1 when none came
fl_wait() {
  local n m
  n=$(( ${2:-5} * 5 ))
  while [ $n -gt 0 ]; do
    if [ "${3:-}" = sys ]; then m=$(fl_syssince | grep -a "$1" | tail -1); else m=$(fl_since | grep -a "$1" | tail -1); fi
    [ -n "$m" ] && { echo "$m"; return 0; }
    usleep 200000; n=$((n-1))
  done
  return 1
}
fl_inject() { umask 077; echo "$1" > $FL_INJ; }
# a command and its answer line (the mark set first)
fl_ask() { fl_mark; fl_inject "$1"; fl_wait "$2" ${3:-5}; }

# --- the oracles ------------------------------------------------------------------------------------------------
# "docX docY W H cardX cardY sX sY cX cY": the element's document rect (its viewport rect plus the engine's scroll),
# the card's scroll, the engine's scroll, the centre (document px). Nothing (1) when there is no such element.
fl_rect() {
  local l r
  l=$(fl_ask "rect $1" 'inject rect ' 5) || return 1
  r=$(echo "$l" | awk '{ for (i = 1; i <= NF; i++) {
      if ($i ~ /^scroll=/) split(substr($i, 8), s, ","); else if ($i ~ /^rect=/) split(substr($i, 6), r, ",");
      else if ($i ~ /^[0-9]+x[0-9]+$/) split($i, w, "x"); else if ($i ~ /^center=/) split(substr($i, 8), c, ",");
      else if ($i ~ /^card=/) split(substr($i, 6), k, ","); }
    if (r[1] != "" && w[1] != "") print r[1] + s[1], r[2] + s[2], w[1], w[2], k[1] + 0, k[2] + 0, s[1] + 0, s[2] + 0, c[1] + s[1], c[2] + s[2] }')
  [ -n "$r" ] || return 1
  echo "$r"
}
# the element's text: gettext takes a bare CSS selector (rect's "sel:" and id forms are mapped onto it)
fl_text() {
  local q l
  case "$1" in sel:*) q=${1#sel:};; anon:*) echo "(gettext has no anon: form)"; return 1;; *) q="#$1";; esac
  l=$(fl_ask "gettext $q" 'inject gettext ' 5) || return 1; echo "$l" | sed 's/.* = \[\(.*\)\]$/\1/'
}
fl_pref() { local l; l=$(fl_ask "getpref $1 $2" 'inject getpref ' 5) || return 1; echo "$l" | sed 's/.* = \[\(.*\)\]$/\1/'; }
fl_popups() { local l; l=$(fl_ask "popups" 'inject popups=' 3) || return 1; echo "$l" | sed 's/.*popups=\([0-9]*\).*/\1/'; }
fl_zoom() { local z; z=$(grep -a 'painted shmid' $FL_LOG | tail -1 | sed -n 's/.*mZoom=\([0-9.]*\).*/\1/p'); echo "${z:-1.0000}"; }
fl_url() { grep -a 'load done uri=' $FL_LOG | tail -1 | sed 's/.*load done uri=//'; }
fl_card_scroll() { set -- $(fl_rect "sel:body") || return 1; echo "$5 $6"; }
# the keyboard now: 1 up, 0 down — the window server's last IME line for this app (the daemon's last editorFocused
# line while the syslog is away). A tap on a field raises it and it covers the screen's lower 290 rows: the rows a
# finger can reach end at 600 while it is up (fl_bring; Enyo lost taps at 771 and 849, Mojo one at 673), and a check
# that needs it down says so
fl_kb() {
  local v
  if [ -f $FL_SYS ]; then v=$(grep -a "IME: Window got focus change in sysmgr $FL_ID focused:" $FL_SYS | tail -1 | sed -n 's/.*focused: \([01]\).*/\1/p')
  else v=$(grep -a 'editorFocused=' $FL_LOG | tail -1 | sed -n 's/.*editorFocused=\([01]\).*/\1/p'); fi
  echo "${v:-0}"
}
# the last clickAt since the mark: "x y" (document px)
fl_last_hit() { fl_since | grep -a 'clickAt x=' | tail -1 | sed -n 's/.*clickAt x=\(-*[0-9]*\) y=\(-*[0-9]*\).*/\1 \2/p'; }

# --- the geometry (R3) -------------------------------------------------------------------------------------------
# the screen point of a document point: X = docX * zoom - cardX; Y = top + docY * zoom - cardY - header, the header
# Mojo's address bar while it is scrolled away (FL_HDR)
fl_hdr() { [ "$FL_HDR" = 1 ] && echo $FL_HEADER || echo 0; }
fl_to_screen() { # docX docY cardX cardY
  awk -v x=$1 -v y=$2 -v kx=$3 -v ky=$4 -v z=$(fl_zoom) -v top=$FL_TOP -v h=$(fl_hdr) \
    'BEGIN { printf "%d %d\n", x * z - kx + 0.5, top + y * z - ky - h + 0.5 }'
}
fl_screen() { set -- $(fl_rect "$1") || return 1; fl_to_screen $9 ${10} $5 $6; }
# "X0 Y0 X1 Y1": the element's screen rect
fl_screen_rect() {
  local a b
  set -- $(fl_rect "$1") || return 1
  a=$(fl_to_screen $1 $2 $5 $6); b=$(fl_to_screen $(($1 + $3)) $(($2 + $4)) $5 $6); echo "$a $b"
}
# the page's top on the screen, measured: a tap on the element's centre aimed with the table's constant, the clickAt
# read back; prints "measured aimedY clickY" — nothing (1) when the tap reached no clickAt
fl_measure_top() {
  local p ky l got
  fl_bring "$1" || return 1
  set -- $(fl_rect "$1") || return 1
  p=$(fl_to_screen $9 ${10} $5 $6); ky=$6
  fl_mark; /tmp/finger tap $p
  l=$(fl_wait 'clickAt x=' 4) || return 1
  got=$(echo "$l" | sed -n 's/.*clickAt x=\(-*[0-9]*\) y=\(-*[0-9]*\).*/\2/p')
  set -- $p
  awk -v ty=$2 -v got=$got -v ky=$ky -v z=$(fl_zoom) -v h=$(fl_hdr) 'BEGIN { printf "%d %d %d\n", ty - got * z + ky + h + 0.5, ty, got }'
}
# the page top as the model has it now, against a tap: the header rule and the card's scroll checked after a gesture
fl_check_top() { # NAME SEL [TOL]
  local m d ev
  m=$(fl_measure_top "$2") || { fl_check FAIL "$1" "the tap on $2 reached no clickAt"; return 1; }
  set -- "$1" "$2" "${3:-10}" $m
  d=$(fl_abs $(($4 - FL_TOP))); ev="the page's top measured $4 (the tap at y $5 landed on document y $6), the model says $FL_TOP (header hidden: $FL_HDR)"
  if [ $d -le $3 ]; then fl_check ok "$1" "$ev"; else fl_check FAIL "$1" "$ev"; fi
}
# R3's calibration: the table's constant checked by one tap before any test; a disagreement stops the round
fl_calibrate() {
  local tol m d
  tol=${2:-10}
  fl_pull_header   # Mojo: the bar back first (a session may start where the last one left the page)
  m=$(fl_measure_top "$1") || { echo "CALIBRATION FAIL ($FL_V): the tap on $1 aimed with the page top $FL_TOP0 reached no clickAt — the round stops"; exit 3; }
  set -- $m
  d=$(($1 - FL_TOP0)); [ $d -lt 0 ] && d=$((-d))
  if [ $d -gt $tol ]; then
    echo "CALIBRATION FAIL ($FL_V): the page's top measured $1 on the screen (the tap at y $2 landed on document y $3), the table says $FL_TOP0: off by $d px — the round stops"
    exit 3
  fi
  FL_TOP=$1
  echo "calibrated ($FL_V): the page's top on the screen $FL_TOP (the table said $FL_TOP0, the tap at y $2 landed on document y $3)"
}

# --- gestures with their marks ---------------------------------------------------------------------------------
# the card scrolled so the element sits in the middle of what a finger can reach (outside rows 120..880, or
# 120..600 with the keyboard up), settled
fl_bring() {
  local p want lim mid
  set -- $(fl_rect "$1") || return 1
  p=$(fl_to_screen $9 ${10} $5 $6); set -- $p $6
  if [ "$(fl_kb)" = 1 ]; then lim=600; mid=350; else lim=880; mid=500; fi   # the keyboard's reach: Mojo's took a tap at 673
  if [ $2 -lt 120 ] || [ $2 -gt $lim ]; then
    want=$(( $3 + $2 - mid )); [ $want -lt 0 ] && want=0
    fl_card_to $want
  fi
}
fl_wait_settled() { # the card's scroll read twice the same, BOUND s at most
  local n last cur
  n=$(( ${1:-5} * 3 )); last=
  while [ $n -gt 0 ]; do cur=$(fl_card_scroll); [ -n "$cur" ] && [ "$cur" = "$last" ] && return 0; last=$cur; usleep 300000; n=$((n-1)); done
  echo "  (the card's scroll did not settle: $last)"; return 1
}
# a cardscroll that moves the view hides Mojo's bar; one to where the view already is moves nothing (measured 2026-11-04:
# the bar stayed after "cardscroll 0 0" on an unscrolled page, went after "cardscroll 0 300" and stayed away after the
# "cardscroll 0 0" that followed)
fl_card_to() { set -- $1 $(fl_card_scroll); [ "$1" != "${3:-}" ] && FL_HDR=1; fl_inject "cardscroll 0 $1"; usleep 300000; fl_wait_settled 5; }
fl_drag() { /tmp/finger drag $1 $2 $3 $4 ${5:-600}; FL_HDR=1; fl_wait_settled 6; }
fl_flick() { /tmp/finger flick $1 $2 $3 $4 ${5:-160}; FL_HDR=1; fl_wait_settled 10; }
# Mojo's address bar back: the card at the page's top, then a drag down there (120 px, more than the bar)
fl_pull_header() { [ $FL_HEADER -gt 0 ] || return 0; fl_card_to 0; /tmp/finger drag 480 300 480 420 300; FL_HDR=0; fl_wait_settled 5; }
fl_tap_at() { # X Y: a tap, the clickAt read back into FL_HIT (document px); 1 when none
  local l
  FL_TX=$1; FL_TY=$2; fl_mark; /tmp/finger tap $1 $2
  l=$(fl_wait 'clickAt x=' 4) || { FL_HIT=""; echo "  tap at $1,$2: no clickAt"; return 1; }
  FL_HIT=$(echo "$l" | sed -n 's/.*clickAt x=\(-*[0-9]*\) y=\(-*[0-9]*\).*/\1 \2/p'); echo "  tap at $1,$2 -> clickAt $FL_HIT"
}
fl_tap() { # SEL
  local p
  fl_bring "$1" || { echo "  tap: no element $1"; FL_HIT=""; return 1; }
  p=$(fl_screen "$1") || return 1; fl_tap_at $p
}
fl_chrome_tap() { # NAME: the shell's table entry; Mojo's address bar pulled back first for its entries
  local p
  p=$(fl_chrome $1) || return 1
  case $FL_V:$1 in mojo:address|mojo:reload) fl_pull_header;; esac
  fl_mark; /tmp/finger tap $p; echo "  chrome $1 tapped at $p"
}
# a frame region still: three reads 150 ms apart with one hash, after a 200 ms floor (a Mojo popup fades and slides
# in: two early reads agreed before it moved, 2026-11-04), BOUND s at most — the card's own animations have no line
# in any log; the shown frame is their oracle
fl_wait_still() { # REGION [BOUND]
  local n last cur same
  n=$(( ${2:-4} * 6 )); last=""; same=0; usleep 200000
  while [ $n -gt 0 ]; do
    cur=$(fl_region "$1" | sed -n 's/.*hash \([0-9a-f]*\).*/\1/p')
    if [ -n "$cur" ] && [ "$cur" = "$last" ]; then same=$((same + 1)); [ $same -ge 2 ] && return 0; else same=0; fi
    last=$cur; usleep 150000; n=$((n-1))
  done
  return 1
}
fl_pick() { # SEL INDEX [ROWS]: the select brought mid-screen (an Enyo/Mochi list needs room below), tapped, the
            # card's picker open and its list still, its INDEXth row tapped (Mojo: by the centred rule, ROWS needed),
            # the picker gone (Mojo: its region still again; the caller's readback decides)
  local n p rx ry reg
  set -- "$1" "$2" "${3:-}" $(fl_rect "$1") || return 1; [ -n "${13:-}" ] || { echo "  pick: no element $1"; return 1; }
  p=$(fl_to_screen ${12} ${13} $8 $9); set -- "$1" "$2" "$3" $p $9
  if [ $5 -lt 200 ] || [ $5 -gt 420 ]; then n=$(( $6 + $5 - 350 )); [ $n -lt 0 ] && n=0; fl_card_to $n; fi
  fl_tap "$1" || return 1
  fl_wait 'popupMenuShow id=' 4 >/dev/null || { echo "  pick: the tap on $1 opened no picker"; return 1; }
  if [ $FL_PICK_CX -gt 0 ]; then
    # Mojo's popup is centred on the visible card area — lower with the keyboard down, higher with it up — so it is
    # measured where it is: the span of light rows at the popup's centre column (white over the dimmed page)
    reg="$(( FL_PICK_CX - 140 )),60,$(( FL_PICK_CX + 140 )),1000"
    fl_wait_still "$reg" 4 || echo "  pick: the list never held still"
    set -- "$1" "$2" "$3" $(fl_region "$reg" | sed -n 's/.* light \([0-9]*\)-\([0-9]*\).*/\1 \2/p')
    [ -n "${5:-}" ] || { echo "  pick: no light rows where Mojo's popup should be"; return 1; }
    rx=$FL_PICK_CX; ry=$(( $4 + FL_PICKROW / 2 + FL_PICKROW * $2 ))
    echo "  pick: Mojo's popup at rows $4-$5, row $2 at $ry"
  else
    rx=$FL_TX; ry=$(( FL_TY + FL_PICK0 + FL_PICKROW * $2 ))
    reg="0,$(( FL_TY + 20 )),756,$(( FL_TY + 20 + FL_PICKROW * 8 ))"
    fl_wait_still "$reg" 4 || echo "  pick: the list never held still"
  fi
  /tmp/finger tap $rx $ry
  if [ $FL_PICK_CX -gt 0 ]; then fl_wait_still "$reg" 4 >/dev/null; return 0; fi
  n=15; while [ $n -gt 0 ]; do [ "$(fl_popups)" = 0 ] && return 0; usleep 200000; n=$((n-1)); done
  echo "  pick: the picker is still open after the row's tap"; return 1
}
# a pref read until it differs from OLD (BOUND s, 3 by default): prints the value read last — unchanged after the bound
# is the caller's failed check, not a skip
fl_wait_pref() { # T NAME OLD [BOUND]
  local n v
  n=$(( ${4:-3} * 3 )); v="$3"
  while [ $n -gt 0 ]; do v=$(fl_pref $1 $2); [ "$v" != "$3" ] && break; usleep 300000; n=$((n-1)); done; echo "$v"
}
# an element present (its rect answered) within BOUND s
fl_wait_rect() { local n; n=$(( ${2:-5} * 2 )); while [ $n -gt 0 ]; do fl_rect "$1" >/dev/null && return 0; usleep 400000; n=$((n-1)); done; return 1; }
fl_wait_load() { fl_wait 'load done uri=' ${1:-30}; }
fl_wait_painted() { fl_wait 'painted shmid=0x[0-9a-f]* bytes=[1-9]' ${1:-20}; }
fl_wait_zoom() { fl_wait 'setZoomAndScroll zoom=' ${1:-10}; }
fl_load() { fl_mark; fl_inject "url $1"; fl_wait_load ${2:-40} >/dev/null || { echo "  load: no load done for $1 in ${2:-40} s"; return 1; }; FL_HDR=0; fl_wait_painted 20 >/dev/null; }

# --- the checks (R1) ---------------------------------------------------------------------------------------------
fl_check() { # ok|FAIL NAME EVIDENCE
  FL_CHECKS=$((FL_CHECKS + 1))
  if [ "$1" = ok ]; then echo "CHECK ok $2: $3"; else FL_FAILS=$((FL_FAILS + 1)); echo "CHECK FAIL $2: $3"; fl_keep "$2"; fi
}
fl_keep() { # the shown frame, whole, for a failed check
  local n
  n=$(echo "$1" | tr -c 'A-Za-z0-9_-' '_' | cut -c1-60)
  /tmp/fbstream $FL_KEEP/$n -Q 0,0,768,1024 -L >/dev/null 2>&1 && echo "  frame kept: $FL_KEEP/$n.last.raw"
}
# a region: "X0,Y0,X1,Y1" as given, or an element (sel:/id) as its screen rect now, cut at column 756 (the card's
# scroll indicator stands in 757..767) — a fixed part is checked where it now is (Mojo's page moves up by its
# address bar once the card is scrolled)
fl_region_of() {
  local x1
  case "$1" in
    [0-9]*,*) echo "$1";;
    *) set -- $(fl_screen_rect "$1") || return 1; x1=$3; [ $x1 -gt 756 ] && x1=756; echo "$1,$(($2 + 1)),$x1,$(($4 - 1))";;
  esac
}
fl_region() { local r; r=$(fl_region_of "$1") || { echo "region (no element $1)"; return 1; }; /tmp/fbstream /tmp/fl-region -Q $r 2>/dev/null; }   # "region ...: colours N hash H rows R uniform U"
fl_abs() { if [ $1 -lt 0 ]; then echo $(( 0 - $1 )); else echo $1; fi; }
fl_check_scroll() { # NAME ENGINE_Y CARD_Y TOL
  local r ey ky de dk ev
  r=$(fl_rect "sel:body") || { fl_check FAIL "$1" "no rect of body"; return 1; }
  set -- "$1" "$2" "$3" "$4" $r
  ey=${12}; ky=${10}; de=$(fl_abs $((ey - $2))); dk=$(fl_abs $((ky - $3)))
  ev="the engine's scroll y $ey (want $2), the card's $ky (want $3), within $4"
  if [ $de -le $4 ] && [ $dk -le $4 ]; then fl_check ok "$1" "$ev"; else fl_check FAIL "$1" "$ev"; fi
}
fl_check_card_scroll() { # NAME CARD_Y TOL: the card's own scroll alone (the engine's reported beside it)
  local r ky dk ev
  r=$(fl_rect "sel:body") || { fl_check FAIL "$1" "no rect of body"; return 1; }
  set -- "$1" "$2" "$3" $r
  ky=$9; dk=$(fl_abs $((ky - $2))); ev="the card's scroll y $ky (want $2 within $3); the engine's ${11}"
  if [ $dk -le $3 ]; then fl_check ok "$1" "$ev"; else fl_check FAIL "$1" "$ev"; fi
}
fl_check_zoom() { # NAME Z TOL
  local z ok
  z=$(fl_zoom); ok=$(awk -v z=$z -v w=$2 -v t=$3 'BEGIN { d = z - w; if (d < 0) d = -d; print (d <= t) ? 1 : 0 }')
  if [ "$ok" = 1 ]; then fl_check ok "$1" "zoom $z (want $2 within $3)"; else fl_check FAIL "$1" "zoom $z (want $2 within $3)"; fi
}
fl_check_hit() { # NAME SEL: the last tap's clickAt inside the element's document rect
  local r
  [ -n "$FL_HIT" ] || { fl_check FAIL "$1" "the last tap reached no clickAt"; return 1; }
  r=$(fl_rect "$2") || { fl_check FAIL "$1" "no element $2"; return 1; }
  set -- "$1" "$2" $FL_HIT $r
  if [ $3 -ge $5 ] && [ $3 -lt $(($5 + $7)) ] && [ $4 -ge $6 ] && [ $4 -lt $(($6 + $8)) ]; then
    fl_check ok "$1" "clickAt $3,$4 inside $2 at $5,$6 $7x$8"; else fl_check FAIL "$1" "clickAt $3,$4 outside $2 at $5,$6 $7x$8"; fi
}
fl_check_element_at() { # NAME SEL X Y: the element's screen rect holds the point
  local r
  r=$(fl_screen_rect "$2") || { fl_check FAIL "$1" "no element $2"; return 1; }
  set -- "$1" "$2" "$3" "$4" $r
  if [ $3 -ge $5 ] && [ $3 -lt $7 ] && [ $4 -ge $6 ] && [ $4 -lt $8 ]; then
    fl_check ok "$1" "$2 on the screen at $5,$6-$7,$8 holds $3,$4"; else fl_check FAIL "$1" "$2 on the screen at $5,$6-$7,$8 does not hold $3,$4"; fi
}
fl_check_text() { # NAME SEL TEXT
  local t
  t=$(fl_text "$2") || { fl_check FAIL "$1" "no answer for $2"; return 1; }
  if [ "$t" = "$3" ]; then fl_check ok "$1" "$2 = [$t]"; else fl_check FAIL "$1" "$2 = [$t], want [$3]"; fi
}
fl_check_text_has() { # NAME SEL TEXT
  local t
  t=$(fl_text "$2") || { fl_check FAIL "$1" "no answer for $2"; return 1; }
  case "$t" in *"$3"*) fl_check ok "$1" "$2 = [$t] holds [$3]";; *) fl_check FAIL "$1" "$2 = [$t] lacks [$3]";; esac
}
fl_check_url() { # NAME PART
  local u
  u=$(fl_url); case "$u" in *"$2"*) fl_check ok "$1" "load done uri=$u";; *) fl_check FAIL "$1" "load done uri=[$u] lacks $2";; esac
}
fl_check_dialog() { # NAME present|answered [BOUND]: since the mark
  local pat l
  case $2 in present) pat='dialog .*: card picked up';; answered) pat='dialog .* -> ';; *) pat="$2";; esac
  l=$(fl_wait "$pat" ${3:-10}) && fl_check ok "$1" "$l" || fl_check FAIL "$1" "no '$pat' in the daemon log within ${3:-10} s"
}
fl_check_keyboard() { # NAME up|down [BOUND]: the window server's IME line for this app since the mark (the daemon's
  # own editorFocused line while the syslog is away: it rotates under a round and comes back on the next write)
  local f l
  case $2 in up) f=1;; down) f=0;; esac
  if [ -f $FL_SYS ]; then
    l=$(fl_wait "IME: Window got focus change in sysmgr $FL_ID focused: $f" ${3:-8} sys) && fl_check ok "$1" "$(echo "$l" | sed 's/.*LunaSysMgr: {LunaSysMgr}: //' | cut -c1-120)" \
      || fl_check FAIL "$1" "no IME 'focused: $f' line for $FL_ID in the syslog within ${3:-8} s"
  else
    l=$(fl_wait "editorFocused=$f" ${3:-8}) && fl_check ok "$1" "the syslog away; the daemon's $(echo "$l" | sed 's/.*\[jihad-bs\] //' | cut -c1-80)" \
      || fl_check FAIL "$1" "the syslog away and no 'editorFocused=$f' line in the daemon log within ${3:-8} s"
  fi
}
fl_check_content() { # NAME REGION SEL [TEXT]: content in the region (8+ colours), the element's rect meeting it
  local reg nc r meet ev t
  reg=$(fl_region "$2"); nc=$(echo "$reg" | sed -n 's/.*colours \([0-9]*\).*/\1/p'); [ -n "$nc" ] || nc=0
  r=$(fl_screen_rect "$3") || { fl_check FAIL "$1" "no element $3; $reg"; return 1; }
  set -- "$1" "$2" "$3" "${4:-}" $(fl_region_of "$2" | tr ',' ' ') $r
  meet=0; [ ${9} -lt $7 ] && [ ${11} -gt $5 ] && [ ${10} -lt $8 ] && [ ${12} -gt $6 ] && meet=1
  ev="$reg; $3 on the screen at $9,${10}-${11},${12}"
  if [ -n "$4" ]; then t=$(fl_text "$3"); case "$t" in *"$4"*) ev="$ev, text holds [$4]";; *) meet=0; ev="$ev, text [$t] LACKS [$4]";; esac; fi
  if [ $nc -ge 8 ] && [ $meet = 1 ]; then fl_check ok "$1" "$ev"; else fl_check FAIL "$1" "$ev"; fi
}
fl_sample() { # NAME REGION
  local h
  h=$(fl_region "$2" | sed -n 's/.*hash \([0-9a-f]*\).*/\1/p'); eval "FL_S_$(echo $1 | tr -c 'A-Za-z0-9_' '_')=$h"; echo "  sample $1 of $2: hash $h"
}
fl_check_region_equal() { # NAME REGION [SAMPLE-NAME]
  local s reg h
  s=$(eval "echo \$FL_S_$(echo ${3:-$1} | tr -c 'A-Za-z0-9_' '_')"); reg=$(fl_region "$2"); h=$(echo "$reg" | sed -n 's/.*hash \([0-9a-f]*\).*/\1/p')
  if [ -n "$s" ] && [ "$h" = "$s" ]; then fl_check ok "$1" "$reg = the sample's hash"; else fl_check FAIL "$1" "$reg, the sample's hash ${s:-(none)}"; fi
}
fl_check_region_changed() { # NAME REGION [SAMPLE-NAME]
  local s reg h
  s=$(eval "echo \$FL_S_$(echo ${3:-$1} | tr -c 'A-Za-z0-9_' '_')"); reg=$(fl_region "$2"); h=$(echo "$reg" | sed -n 's/.*hash \([0-9a-f]*\).*/\1/p')
  if [ -n "$s" ] && [ "$h" != "$s" ]; then fl_check ok "$1" "$reg against the sample's hash $s"; else fl_check FAIL "$1" "$reg = the sample's hash ${s:-(none)}"; fi
}
fl_summary() { echo "CHECKS $FL_CHECKS FAILS $FL_FAILS"; [ $FL_FAILS = 0 ]; }
