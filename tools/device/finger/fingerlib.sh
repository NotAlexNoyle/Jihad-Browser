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
#   fl_type TEXT | fl_kb_hold NAME [MS]   the system keyboard by finger (fl_kb_key CHAR is the map); the field read after
#   fl_tap SEL | fl_tap_at X Y      a finger tap on an element (scrolled into the view first) or a screen point, then
#                                   the clickAt read back (FL_HIT); fl_chrome_tap NAME taps the shell's table entry
#   fl_pick SEL INDEX [ROWS]        a <select>'s INDEXth option through the card's own picker (the picker rule: the
#                                   select mid-screen, the list waited for by a still frame region, the row tapped —
#                                   under the select on Enyo and Mochi, in Mojo's centred popup by the row count)
#   fl_wait_still REGION [BOUND]    a frame region unchanged over 150 ms (the card's own animations' oracle)
#   fl_wait_changed REGION [BOUND]  a frame region changed from now (a video playing, a canvas drawing)
#   fl_wait_text SEL TEXT [BOUND]   an element's text holding TEXT (the page's readout as a mark)
#   fl_drag X0 Y0 X1 Y1 [MS]        a drag, then the card's scroll settled (two equal reads); fl_drag_slow (2 px a scan),
#                                   fl_drag_jitter (+-2 px a scan), fl_drag_paused X0 Y0 X1 Y1 [PAUSE] [MS] (held mid-way)
#   fl_stream_start [S] [opts] / fl_stream_stop   the frame sampler over the gestures between (the report reads it)
#   fl_daemon_restart URL           the daemon stopped as upstart does and started again on URL (a persistence check)
#   fl_card_to Y                    the card's own view to Y (cardscroll), settled
#   fl_load URL [BOUND]             a page through the inject channel, its load done and a content frame painted
#   fl_wait PATTERN [BOUND] [sys]   a line since the mark (fl_mark) in the daemon log (or the syslog), BOUND s at most
#   fl_wait_pref T NAME OLD [BOUND] a pref read until it differs from OLD; fl_wait_rect SEL [BOUND]: an element present
#   fl_check_scroll NAME EY KY TOL  the engine's and the card's scroll y within TOL; fl_check_card_scroll NAME KY TOL
#   fl_check_zoom NAME Z TOL        the zoom (the last frame's mZoom); fl_check_zoom_min NAME Z; fl_check_card_scroll_min NAME MIN
#   fl_check_zoom_max NAME Z        the zoom at most Z
#   fl_check_visible NAME SEL       the element on the screen within the finger's reach (above the keyboard while it is up)
#   fl_check_hit NAME SEL           the last tap's clickAt landed inside the element's rect
#   fl_check_top NAME SEL [TOL]     the page top the model has now against a tap's clickAt (after a gesture)
#   fl_check_element_at NAME SEL X Y   the element's screen rect holds the point
#   fl_check_text NAME SEL TEXT     the element's text is TEXT; fl_check_text_has: holds TEXT
#   fl_check_url NAME PART          the last load done uri holds PART
#   fl_flash_point SEL SW SH MODE SX SY   a movie's stage point on the screen (showAll, noBorder, exactFit, noScale); fl_flash_tap
#                                   taps it; fl_check_flash_at NAME SX SY [TOL] reads the instrumented movie's report (3 px)
#   fl_check_flash_drawn NAME REGION yes|no   8+ colours in a movie's rectangle; fl_flash_rate REGION S the content rate apart
#                                   from the composite; fl_flash_control REGION S, then fl_check_flash_reacted NAME REGION S yes|no
#   fl_check_screen NAME REGION REF [TOL]  a screen's signature against a stored reference; fl_screen_sig REGION
#   fl_plugin_pid, fl_check_plugin NAME [S] alive | wedged | gone; fl_check_audio NAME open|closed|running  the movie's audio stream
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
# the last row of Enyo's dialog: the column just inside its left edge is light only there, the page behind it dimmed
fl_dialog_last() {
  local r last n
  n=20; last=""     # the pipe stands before the card has drawn the dialog: two equal reads of a dialog's shape, 200 ms apart
  while [ $n -gt 0 ]; do
    r=$(/tmp/fbstream /tmp/fl-dlg -Q 238,82,242,1024 2>/dev/null | sed -n 's/.*light \([0-9]*\)-\([0-9]*\).*/\1 \2/p')
    if [ -n "$r" ] && [ "${r% *}" -ge 200 ] && [ "${r#* }" -le 880 ] && [ "$r" = "$last" ]; then echo "${r#* }"; return 0; fi
    last=$r; usleep 200000; n=$((n-1))
  done
  echo 591
}
# Mojo's sheet at rest: its top row, the only light thing over the dimmed page, read the same twice 200 ms apart
fl_dialog_mojo() {
  local r last n
  n=20; last=""
  while [ $n -gt 0 ]; do
    r=$(/tmp/fbstream /tmp/fl-dlg -Q 20,300,24,1024 2>/dev/null | sed -n 's/.*light \([0-9]*\)-.*/\1/p')
    [ -n "$r" ] && [ "$r" -ge 400 ] && [ "$r" = "$last" ] && { echo "$r"; return 0; }
    last=$r; usleep 200000; n=$((n-1))
  done
  echo 446
}
# the row of Mochi's dialog buttons: the middle of its blue OK, two equal reads 200 ms apart
fl_dialog_mochi() {
  local r last n a b
  n=20; last=""
  while [ $n -gt 0 ]; do
    r=$(/tmp/fbstream /tmp/fl-dlg -Q 610,90,614,800 2>/dev/null | sed -n 's/.*blue \([0-9]*\)-\([0-9]*\).*/\1 \2/p')
    if [ -n "$r" ] && [ "$r" = "$last" ]; then a=${r% *}; b=${r#* }; [ $((b - a)) -ge 25 ] && [ $((b - a)) -le 70 ] && { echo $(((a + b) / 2)); return 0; }; fi
    last=$r; usleep 200000; n=$((n-1))
  done
  echo 193
}
fl_chrome() {
  case $FL_V:$1 in
    enyo:back) echo 30 52;;  enyo:forward) echo 78 52;;  enyo:home) echo 130 52;;  enyo:address) echo 385 52;;
    enyo:reload) echo 582 52;; enyo:share) echo 640 52;; enyo:newcard) echo 690 52;; enyo:history) echo 740 52;;
    mochi:back) echo 28 52;; mochi:forward) echo 74 52;; mochi:home) echo 120 52;; mochi:address) echo 385 52;;
    mochi:reload) echo 596 52;; mochi:share) echo 646 52;; mochi:newcard) echo 692 52;; mochi:history) echo 738 52;;
    mojo:address) echo 385 51;; mojo:reload) echo 740 51;; mojo:back) echo 222 983;; mojo:forward) echo 276 983;;
    mojo:home) echo 330 983;; mojo:newcard) echo 384 983;; mojo:history) echo 438 983;; mojo:share) echo 492 983;;
    mojo:search) echo 546 983;;
    # Downloads from the history button: Enyo's toaster has it as the third tab icon, Mochi's and Mojo's button opens a
    # three-row menu (Bookmarks, History, Downloads) at the top right / above the bottom bar (measured 2026-11-08)
    enyo:downloads-tab) echo 708 55;; mochi:downloads-tab) echo 650 206;; mojo:downloads-tab) echo 552 979;;
    # the card's dialog (a page's confirm/alert/prompt; measured 2026-11-05 on jihad-dialogs.html): Enyo a centred
    # popup, Mochi a box under the toolbar, Mojo a sheet at the bottom
    # Enyo's popup is centred and as tall as its text: its buttons counted from its last light row (fl_dialog_last)
    enyo:dialog-ok) echo 384 $(($(fl_dialog_last) - 71));; enyo:dialog-cancel|enyo:dialog-close) echo 384 $(($(fl_dialog_last) - 29));;
    # Mochi's box hangs under the toolbar and is as tall as its text: its button row found by the blue OK (fl_dialog_mochi)
    mochi:dialog-ok) echo 612 $(fl_dialog_mochi);; mochi:dialog-cancel) echo 523 $(fl_dialog_mochi);;
    # Mojo's sheet rises from the bottom: a tap while it slides lands on the wrong button (Cancel answered ACCEPT, 2026-11-05)
    mojo:dialog-ok) fl_dialog_mojo >/dev/null; echo 384 935;; mojo:dialog-cancel) fl_dialog_mojo >/dev/null; echo 384 990;;
    # the system's app menu (the app's name at the top left; measured 2026-11-05 on Mochi and Mojo, the same menu
    # in every shell): Find on Page, Settings, Print
    *:appmenu) echo 50 14;; *:appmenu-find) echo 70 57;; *:appmenu-settings) echo 51 105;; *:appmenu-print) echo 36 153;;
    # T-602's screen lock while a video plays: the button at the view's top right and the notice beside it (placed by
    # each shell's CSS, 2026-11-09; checked on the screen-lock script's shots)
    enyo:lock-button|mochi:lock-button) echo 732 116;; mojo:lock-button) echo 732 124;;
    enyo:lock-notice|mochi:lock-notice) echo 630 116;; mojo:lock-notice) echo 630 123;;
    # the address field's Cut / Copy / Paste popup (a second tap on the focused field; Mojo's field has none: one tap
    # selects, typing replaces)
    enyo:cutcopy-cut) echo 275 111;; enyo:cutcopy-copy) echo 342 111;; enyo:cutcopy-paste) echo 409 111;;
    mochi:cutcopy-cut) echo 229 108;; mochi:cutcopy-copy) echo 296 108;; mochi:cutcopy-paste) echo 363 108;;
    # the system keyboard's special keys (fb0; the same in every shell; fl_kb_key has the letters)
    *:kb-hide) echo 732 993;; *:kb-backspace) echo 732 813;; *:kb-enter) echo 715 873;; *:kb-space) echo 384 993;;
    # the padlock on an https page, and the SSL dialog of a host with a questionable certificate (measured 2026-11-05 on
    # self-signed.badssl.com: Enyo a box mid-screen, Mochi a box under the toolbar, Mojo a sheet at the bottom)
    enyo:padlock) echo 184 54;; mochi:padlock) echo 172 53;; mojo:padlock) echo 27 51;;
    enyo:ssl-always) echo 293 547;; enyo:ssl-once) echo 473 547;; enyo:ssl-deny) echo 653 547;;
    mochi:ssl-deny) echo 309 270;; mochi:ssl-once) echo 442 270;; mochi:ssl-always) echo 579 270;;
    mojo:ssl-always) echo 384 880;; mojo:ssl-once) echo 384 935;; mojo:ssl-deny) echo 384 990;;
    # the proxy's login dialog (measured 2026-11-05): Enyo's centres in what the keyboard leaves, so these are its places
    # with the keyboard down (its script drops the keyboard between fields); Mojo's sheet comes up with the keyboard up
    enyo:auth-user) echo 384 520;; enyo:auth-pass) echo 384 566;; enyo:auth-ok) echo 453 608;; enyo:auth-cancel) echo 313 608;;
    mochi:auth-user) echo 383 208;; mochi:auth-pass) echo 383 250;; mochi:auth-ok) echo 612 297;; mochi:auth-cancel) echo 523 297;;
    # Mojo's login sheet stands on the keyboard when that is up and at the bottom when not: counted from its measured top
    mojo:auth-user) echo 383 $(($(fl_dialog_mojo) + 80));; mojo:auth-pass) echo 383 $(($(fl_dialog_mojo) + 131));;
    mojo:auth-ok) echo 383 $(($(fl_dialog_mojo) + 198));; mojo:auth-cancel) echo 383 $(($(fl_dialog_mojo) + 253));;
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
# the absence of a line for SECS (a check on what must not happen: a video starting by itself): 0 when PATTERN never
# came since the mark within SECS, 1 (the line printed) when it did
fl_wait_quiet() {
  local n m
  n=${2:-10}
  while [ $n -gt 0 ]; do m=$(fl_since | grep -a "$1" | tail -1); [ -n "$m" ] && { echo "$m"; return 1; }; sleep 1; n=$((n-1)); done
  return 0
}
fl_inject() { umask 077; echo "$1" > $FL_INJ; }
# a command and its answer line (the mark set first)
fl_ask() { fl_mark; fl_inject "$1"; fl_wait "$2" ${3:-5}; }

# --- the oracles ------------------------------------------------------------------------------------------------
# "docX docY W H cardX cardY sX sY cX cY": the element's document rect (its viewport rect plus the engine's scroll),
# the card's scroll, the engine's scroll, the centre (document px). Nothing (1) when there is no such element.
fl_drag_off() { # SEL DX0 DY0 DX1 DY1 [MS]: a drag between two offsets in document px from the element's top left (a slider's thumb)
  local r p0 p1
  fl_bring "$1" || { echo "  drag: no element $1"; return 1; }
  r=$(fl_screen_rect "$1") || return 1; set -- "$1" "$2" "$3" "$4" "$5" "${6:-600}" $r
  p0=$(awk -v x=$7 -v y=$8 -v dx=$2 -v dy=$3 -v z=$(fl_zoom) 'BEGIN { printf "%d %d", x + dx * z + 0.5, y + dy * z + 0.5 }')
  p1=$(awk -v x=$7 -v y=$8 -v dx=$4 -v dy=$5 -v z=$(fl_zoom) 'BEGIN { printf "%d %d", x + dx * z + 0.5, y + dy * z + 0.5 }')
  echo "  drag on $1 from $p0 to $p1 ($6 ms)"; /tmp/finger drag $p0 $p1 $6; FL_HDR=1; fl_wait_settled 6
}
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
# the URL: the last titleAndUrl line (a page back from the back-forward cache sends one, and no load done)
fl_url() {
  local u
  u=$(grep -a 'titleAndUrl ' $FL_LOG | tail -1 | sed -n 's/.* uri=\(.*\) back=.*/\1/p')
  [ -n "$u" ] || u=$(grep -a 'load done uri=' $FL_LOG | tail -1 | sed 's/.*load done uri=//')
  echo "$u"
}
fl_wait_url() { # PART [BOUND]
  local n u; n=$(( ${2:-10} * 3 ))
  while [ $n -gt 0 ]; do u=$(fl_url); case "$u" in *"$1"*) echo "$u"; return 0;; esac; usleep 300000; n=$((n-1)); done
  return 1
}
# the zoom asked for last (a pinch sends many); fl_zoom is the zoom of the last frame painted
fl_zoom_asked() { grep -a 'setZoomAndScroll zoom=' $FL_LOG | tail -1 | sed -n 's/.*zoom=\([0-9.]*\) .*/\1/p'; }
fl_card_scroll() { set -- $(fl_rect "sel::root") || return 1; echo "$5 $6"; }   # :root: a XUL page has no body, and an empty answer read as a card that moved
# the keyboard now: 1 up, 0 down — the window server's last IME line for this app (the daemon's last editorFocused
# line while the syslog is away). A tap on a field raises it and it covers the screen's lower 290 rows: the rows a
# finger can reach end at 600 while it is up (fl_bring; Enyo lost taps at 771 and 849, Mojo one at 673), and a check
# that needs it down says so
fl_kb() {
  local v
  [ -f $FL_SYS ] && v=$(grep -a "IME: Window got focus change in sysmgr $FL_ID focused:" $FL_SYS | tail -1 | sed -n 's/.*focused: \([01]\).*/\1/p')
  [ -n "$v" ] || v=$(grep -a 'editorFocused=' $FL_LOG | tail -1 | sed -n 's/.*editorFocused=\([01]\).*/\1/p')   # the syslog rotates
  echo "${v:-0}"
}
# the last clickAt since the mark: "x y" (document px)
fl_last_hit() { fl_since | grep -a 'clickAt x=' | tail -1 | sed -n 's/.*clickAt x=\(-*[0-9]*\) y=\(-*[0-9]*\).*/\1 \2/p'; }

# --- the geometry (R3) -------------------------------------------------------------------------------------------
# the screen point of a document point: X = docX * zoom - cardX; Y = top + docY * zoom - cardY - header, the header
# Mojo's address bar while it is scrolled away (FL_HDR)
fl_hdr() { [ "$FL_HDR" = 1 ] && echo $FL_HEADER || echo 0; }
# the card's scroll comes in page units (the daemon's debugCardScroll divides by the zoom), so it is scaled with the page:
# x*z - kx put a zoomed column 490 px right of where it stood (T-643's first run, 2026-11-12); at zoom 1 the two agree
fl_to_screen() { # docX docY cardX cardY
  awk -v x=$1 -v y=$2 -v kx=$3 -v ky=$4 -v z=$(fl_zoom) -v top=$FL_TOP -v h=$(fl_hdr) \
    'BEGIN { printf "%d %d\n", (x - kx) * z + 0.5, top + (y - ky) * z - h + 0.5 }'
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
  awk -v ty=$2 -v got=$got -v ky=$ky -v z=$(fl_zoom) -v h=$(fl_hdr) 'BEGIN { printf "%d %d %d\n", ty - (got - ky) * z + h + 0.5, ty, got }'
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
  m=$(fl_measure_top "$1") || { echo "CALIBRATION FAIL ($FL_V): the tap on $1 aimed with the page top $FL_TOP0 reached no clickAt — the round stops"; fl_keep calibration; exit 3; }
  set -- $m
  d=$(($1 - FL_TOP0)); [ $d -lt 0 ] && d=$((-d))
  if [ $d -gt $tol ]; then
    echo "CALIBRATION FAIL ($FL_V): the page's top measured $1 on the screen (the tap at y $2 landed on document y $3), the table says $FL_TOP0: off by $d px — the round stops"
    fl_keep calibration
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
    # the card's scroll reads in page units, cardscroll takes screen px: scaled by the zoom (2026-11-12)
    want=$(awk -v ky=$3 -v sy=$2 -v mid=$mid -v z=$(fl_zoom) 'BEGIN { printf "%d", ky * z + sy - mid + 0.5 }'); [ $want -lt 0 ] && want=0
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
# the slow human's drags (T-565): a slow drag at 2 px a scan (the panel's 100 Hz: 200 px a second), a jittered path
# (+-2 px each way on every scan), a drag that pauses mid-way (PAUSE ms, 700 by default); each settled after
fl_drag_slow() { /tmp/finger drag $1 $2 $3 $4 $(( ( ($4 - $2) < 0 ? ($2 - $4) : ($4 - $2) ) * 5 )); FL_HDR=1; fl_wait_settled 8; }
fl_drag_jitter() { FINGER_JITTER=2 /tmp/finger drag $1 $2 $3 $4 ${5:-600}; FL_HDR=1; fl_wait_settled 6; }
fl_drag_paused() { FINGER_PAUSE_MS=${5:-700} /tmp/finger drag $1 $2 $3 $4 ${6:-600}; FL_HDR=1; fl_wait_settled 6; }
# the frame sampler over the gestures that follow (T-565, R6: the stream's numbers in the report): the stream under
# /media/internal/jihad-finger/stream.fbs, the finger's trace joined; fl_stream_stop waits for it
fl_stream_start() { # [SECONDS] [fbstream options...]
  local s; s=${1:-30}; shift 2>/dev/null; mkdir -p $FL_KEEP; rm -f $FL_KEEP/stream.fbs $FL_KEEP/stream.fin
  /tmp/fbstream $FL_KEEP/stream.fbs -s $s -f $FL_KEEP/stream.fin "$@" > /tmp/fl-stream.out 2>&1 & FL_SP=$!
  export FINGER_TRACE=$FL_KEEP/stream.fin; usleep 500000; echo "  stream on for $s s ($*)"
}
fl_stream_stop() { [ -n "${FL_SP:-}" ] && { kill $FL_SP 2>/dev/null; wait $FL_SP 2>/dev/null; }; unset FINGER_TRACE FL_SP; echo "  stream: $(cat /tmp/fl-stream.out 2>/dev/null | head -1 | cut -c1-160)"; }
# the daemon stopped as upstart stops it (SIGTERM) and started again on the same page: a persistence check's restart
# (/tmp/fl-daemon-start.sh is daemon-start.sh, put there by the runner; KEEP=1 leaves the profile and the log alone)
fl_daemon_restart() { # URL
  [ -x /tmp/fl-daemon-start.sh ] || [ -f /tmp/fl-daemon-start.sh ] || { echo "  restart: no /tmp/fl-daemon-start.sh"; return 1; }
  KEEP=1 sh /tmp/fl-daemon-start.sh $FL_V "$1" 2>&1 | sed 's/^/  restart: /'; fl_mark
}
# Mojo's address bar back: the card at the page's top, then a drag down there (120 px, more than the bar)
fl_pull_header() { [ $FL_HEADER -gt 0 ] || return 0; fl_card_to 0; /tmp/finger drag 480 300 480 420 300; FL_HDR=0; fl_wait_settled 5; }
fl_tap_at() { # X Y: a tap, the clickAt read back into FL_HIT (document px); 1 when none
  local l
  FL_TX=$1; FL_TY=$2; fl_mark; /tmp/finger tap $1 $2
  l=$(fl_wait 'clickAt x=' 4) || { FL_HIT=""; echo "  tap at $1,$2: no clickAt"; return 1; }
  FL_HIT=$(echo "$l" | sed -n 's/.*clickAt x=\(-*[0-9]*\) y=\(-*[0-9]*\).*/\1 \2/p'); echo "  tap at $1,$2 -> clickAt $FL_HIT"
}
fl_tap_off() { # SEL DX DY: a tap at an offset in document px from the element's top left (a field inside a frame the rect cannot see)
  local p r
  fl_bring "$1" || { echo "  tap: no element $1"; FL_HIT=""; return 1; }
  r=$(fl_screen_rect "$1") || return 1; set -- "$1" "$2" "$3" $r
  p=$(awk -v x=$4 -v y=$5 -v dx=$2 -v dy=$3 -v z=$(fl_zoom) 'BEGIN { printf "%d %d\n", x + dx * z + 0.5, y + dy * z + 0.5 }'); fl_tap_at $p
}
# a tap on an element's centre with NO clickAt awaited: a control the daemon reports as interactive (a button in a
# video's bar) takes the pen pair itself and the adapter sends no clickAt for it; the caller's wait is the mark
fl_touch() { # SEL
  local p
  fl_bring "$1" || { echo "  touch: no element $1"; return 1; }
  p=$(fl_screen "$1") || return 1; fl_mark; /tmp/finger tap $p; echo "  touch $1 at $p"
}
fl_tap() { # SEL
  local p
  fl_bring "$1" || { echo "  tap: no element $1"; FL_HIT=""; return 1; }
  p=$(fl_screen "$1") || return 1; fl_tap_at $p
}
# the system keyboard (fb0, the same in every shell; measured 2026-11-05): the rows at y 759 (digits), 813, 873, 933,
# 993; a key every 70 px in the letter rows from x 34 (q) / 69 (a) / 104 (z), every 64 px in the digit row from 34
fl_kb_key() { # CHAR -> "X Y"
  local c i row x0 pitch
  c=$1
  case "$c" in
    " ") echo 384 993; return;; .) echo 662 933; return;; ,) echo 592 933; return;; -) echo 662 993; return;;
    /) echo 244 993; return;; "'") echo 592 993; return;; [0-9]) i=$(( (${c#0} + 9) % 10 )); [ "$c" = 0 ] && i=9; echo $(( 34 + 64 * i )) 759; return;;
  esac
  for row in "qwertyuiop 34 813" "asdfghjkl 69 873" "zxcvbnm 104 933"; do
    set -- $row; i=$(expr index "$1" "$c" 2>/dev/null)
    [ "$i" -gt 0 ] && { echo $(( $2 + 70 * (i - 1) )) $3; return; }
  done
  echo "fl_kb_key: no key for '$c'" >&2; return 1
}
# TEXT typed on the system keyboard, a tap a character (lower-case letters, digits, space . , - / '), 130 ms apart;
# the caller reads the field afterwards (the page's readout, a pref) — the keys themselves have no mark
fl_type() {
  local s c p
  s=$1
  usleep 400000   # the keyboard slides in after its IME line, and it is on fb0 where the sampler cannot watch it
  while [ -n "$s" ]; do c=${s%"${s#?}"}; s=${s#?}; p=$(fl_kb_key "$c") || return 1; /tmp/finger tap $p; usleep 130000; done
  echo "  typed [$1] on the keyboard"
}
fl_kb_hold() { # NAME [MS]: a key held (kb-backspace: the repeat)
  local p; p=$(fl_chrome $1) || return 1; /tmp/finger hold $p ${2:-1500}; echo "  $1 held ${2:-1500} ms"
}
fl_hold() { # SEL [MS]: a long press on an element's centre (the context menu); the clickAt is not awaited (a hold has none)
  local p
  fl_bring "$1" || { echo "  hold: no element $1"; return 1; }
  p=$(fl_screen "$1") || return 1; FL_TX=${p% *}; FL_TY=${p#* }; fl_mark; /tmp/finger hold $p ${2:-1500}; echo "  hold on $1 at $p"
}
fl_chrome_tap() { # NAME: the shell's table entry; Mojo's address bar pulled back first for its entries
  local p
  p=$(fl_chrome $1) || return 1
  case $FL_V:$1 in mojo:address|mojo:reload|mojo:padlock) fl_pull_header;; esac
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
# a frame region changed from what it shows now, BOUND s at most (a video playing, a canvas drawing: its mark)
fl_wait_log() { # PATTERN [BOUND]: a line anywhere in this daemon's log (what came before the last mark counts)
  local n l
  n=$(( ${2:-5} * 3 ))
  while [ $n -gt 0 ]; do l=$(grep -a -- "$1" $FL_LOG | tail -1); [ -n "$l" ] && { echo "$l"; return 0; }; usleep 300000; n=$((n-1)); done
  return 1
}
# --- a movie (cavekit-flash.md R2, T-572) -------------------------------------------------------------------------
# a stage point on the screen: the element's screen rect, the stage's size and how the movie scales it
fl_flash_point() { # SEL SW SH MODE SX SY
  local r
  r=$(fl_screen_rect "$1") || return 1
  set -- "$2" "$3" "$4" "$5" "$6" $r $(fl_zoom)
  awk -v sw=$1 -v sh=$2 -v m=$3 -v sx=$4 -v sy=$5 -v x0=$6 -v y0=$7 -v x1=$8 -v y1=$9 -v z=${10} 'BEGIN {
    w = x1 - x0; h = y1 - y0; ax = w / sw; ay = h / sh
    if (m == "exactFit") { kx = ax; ky = ay }
    else if (m == "noScale") { kx = z; ky = z }
    else if (m == "noBorder") { kx = ky = (ax > ay ? ax : ay) }
    else { kx = ky = (ax < ay ? ax : ay) }
    printf "%d %d\n", x0 + (w - sw * kx) / 2 + sx * kx + 0.5, y0 + (h - sh * ky) / 2 + sy * ky + 0.5 }'
}
# a tap on a stage point; the report's count changed is its mark (a movie's tap has no clickAt when the lock holds it)
fl_flash_tap() { # SEL SW SH MODE SX SY
  local p c0 n
  p=$(fl_flash_point "$@") || { echo "  flash tap: no element $1"; return 1; }
  c0=$(fl_text count); FL_TX=${p% *}; FL_TY=${p#* }; /tmp/finger tap $p; echo "  flash tap on stage $5,$6 ($4) at $p"
  n=15; while [ $n -gt 0 ]; do [ "$(fl_text count)" != "$c0" ] && return 0; usleep 200000; n=$((n-1)); done
  return 1
}
fl_check_flash_at() { # NAME SX SY [TOL]: the last down the instrumented movie reported, in stage pixels
  local l tol n
  tol=${4:-3}; n=15
  while [ $n -gt 0 ]; do l=$(fl_text down); case "$l" in down*) break;; esac; usleep 200000; n=$((n-1)); done
  set -- "$1" "$2" "$3" "$tol" $l
  case "$5" in down) ;; *) fl_check FAIL "$1" "the movie reported no down: [$l]"; return 1;; esac
  if awk -v a=$6 -v b=$7 -v x=$2 -v y=$3 -v t=$4 'BEGIN { d = a - x; e = b - y; if (d < 0) d = -d; if (e < 0) e = -e; exit !(d <= t && e <= t) }'; then
    fl_check ok "$1" "the movie reported down $6,$7 for stage $2,$3 (within $4)"
  else fl_check FAIL "$1" "the movie reported down $6,$7 for stage $2,$3 (more than $4 off)"; fi
}
fl_check_flash_drawn() { # NAME REGION yes|no: drawn is 8 or more colours in the rectangle (never brightness)
  local reg c
  reg=$(fl_region "$2"); c=$(echo "$reg" | sed -n 's/.*colours \([0-9]*\).*/\1/p'); c=${c:-0}
  if [ "$3" = yes ]; then [ $c -ge 8 ] && fl_check ok "$1" "drawn: $reg" || fl_check FAIL "$1" "not drawn: $reg"
  else [ $c -lt 8 ] && fl_check ok "$1" "reported not drawn, as it should be: $reg" || fl_check FAIL "$1" "reported drawn: $reg"; fi
}
fl_flash_region() { fl_region_of "$1" | tr -d ' '; }
fl_flash_rate() { # REGION SECONDS: "content ...: flips F (f/s), changes C (c/s) ..."
  /tmp/fbstream /tmp/fl-content -C "$(fl_flash_region "$1")" -s ${2:-5} -P 2000 2>/dev/null
}
fl_rate_of() { echo "$1" | sed -n 's/.*changes [0-9]* (\([0-9.]*\)\/s).*/\1/p'; }
fl_check_flash_rate() { # NAME REGION SECONDS MIN MAX: the content rate within the bounds
  local l r
  l=$(fl_flash_rate "$2" "$3"); r=$(fl_rate_of "$l")
  if awk -v r=${r:-0} -v a=$4 -v b=$5 'BEGIN { exit !(r >= a && r <= b) }'; then fl_check ok "$1" "$l (wanted $4..$5/s)"; else fl_check FAIL "$1" "$l (wanted $4..$5/s)"; fi
}
fl_check_flash_rate_report() { # NAME REGION SECONDS: the content rate against the movie's own count of its frames, within 5 %
  local a b l r m
  a=$(fl_text frames); l=$(fl_flash_rate "$2" "$3"); b=$(fl_text frames); r=$(fl_rate_of "$l")
  m=$(echo "$a $b" | awk '{ print ($5 > $2 && $6 != $3) ? ($5 - $2) * 1000 / (substr($6, 3) - substr($3, 3)) : 0 }')
  if awk -v r=${r:-0} -v m=$m 'BEGIN { exit !(m > 0 && r >= m * 0.95 && r <= m * 1.05) }'; then fl_check ok "$1" "$l; the movie's own rate $m/s ($a .. $b)"
  else fl_check FAIL "$1" "$l; the movie's own rate $m/s ($a .. $b)"; fi
}
fl_flash_control() { # REGION SECONDS: what the movie shows with no input: the list the reacted check compares against
  /tmp/fbstream /tmp/fl-control -C "$(fl_flash_region "$1")" -s ${2:-4} -P 2000 2>/dev/null | sed 's/^/  control: /'
}
fl_check_flash_reacted() { # NAME REGION SECONDS yes|no: reads showing what the control never showed: half or more is a reaction
  local l a b v
  l=$(/tmp/fbstream /tmp/fl-after -C "$(fl_flash_region "$2")" -s ${3:-4} -P 2000 -B /tmp/fl-control 2>/dev/null)
  a=$(echo "$l" | sed -n 's/.* new \([0-9]*\) of .*/\1/p'); b=$(echo "$l" | sed -n 's/.* new [0-9]* of \([0-9]*\).*/\1/p')
  if [ "${b:-0}" -gt 0 ] && [ $((${a:-0} * 2)) -ge ${b:-0} ]; then v=yes; else v=no; fi
  [ "$v" = "$4" ] && fl_check ok "$1" "reacted: $v ($l)" || fl_check FAIL "$1" "reacted: $v, wanted $4 ($l)"
}
fl_check_file() { # NAME GLOB TEXT: the newest file the glob names on the device holds the text (a download's bytes over USB)
  local f
  f=$(ls -t $2 2>/dev/null | head -1)
  [ -n "$f" ] || { fl_check FAIL "$1" "no file $2 on the device"; return 1; }
  if grep -q -- "$3" "$f" 2>/dev/null; then fl_check ok "$1" "$f ($(wc -c < "$f") bytes) holds [$3]"; else fl_check FAIL "$1" "$f lacks [$3]"; fi
}
fl_check_number() { # NAME SEL MIN MAX: the number an element holds, within the bounds
  local v
  v=$(fl_text "$2")
  if awk -v v="${v:-x}" -v a=$3 -v b=$4 'BEGIN { exit !(v + 0 == v && v >= a && v <= b) }'; then fl_check ok "$1" "$2 = $v (wanted $3..$4)"
  else fl_check FAIL "$1" "$2 = [$v] (wanted $3..$4)"; fi
}
fl_check_flash_el() { # NAME EL SX SY TOL: the stage point of the last line of that kind the movie reported
  local l
  l=$(fl_text "$2"); set -- "$1" "$2" "$3" "$4" "$5" $l
  if awk -v a="${7:-x}" -v b="${8:-x}" -v x=$3 -v y=$4 -v t=$5 'BEGIN { d = a - x; e = b - y; if (d < 0) d = -d; if (e < 0) e = -e; exit !(d <= t && e <= t) }'; then fl_check ok "$1" "the movie's $2 at $7,$8 for $3,$4 (within $5)"
  else fl_check FAIL "$1" "the movie's $2 at [$7,$8] for $3,$4 (more than $5 off)"; fi
}
fl_check_press_rate() { # NAME MIN: the moves a press sent on, a second, by the daemon's line for the last press
  local l r
  l=$(grep -a "a press of" $FL_LOG | tail -1)
  r=$(echo "$l" | sed -n 's/.*moves from the card in \([0-9]*\) ms, \([0-9]*\) sent on.*/\2 \1/p' | awk '{ if ($2 > 0) printf "%.1f", $1 * 1000 / $2 }')
  if awk -v r="${r:-0}" -v m=$2 'BEGIN { exit !(r >= m) }'; then fl_check ok "$1" "$r moves a second sent on (at least $2): $(echo "$l" | cut -c10-)"
  else fl_check FAIL "$1" "${r:-no} moves a second sent on (at least $2): $(echo "$l" | cut -c10-)"; fi
}
fl_check_above_keyboard() { # NAME SEL: the element ends above the keyboard's top, which a shell's card is cut at (y 733)
  local r
  r=$(fl_screen_rect "$2") || { fl_check FAIL "$1" "no element $2"; return 1; }
  set -- "$1" "$2" $r
  if [ $4 -ge 0 ] && [ $6 -le 740 ]; then fl_check ok "$1" "$2 on the screen at $3,$4-$5,$6, above the keyboard"
  else fl_check FAIL "$1" "$2 on the screen at $3,$4-$5,$6, under the keyboard"; fi
}
fl_check_lock() { # NAME 0|1 [BOUND]: the adapter's gesture lock, by its own line in the system log since the mark
  local n l
  n=${3:-5}
  while [ $n -gt 0 ]; do l=$(grep -a "jihad: flashGestureLock ->" /var/log/messages | tail -1); case "$l" in *"-> $2 "*) fl_check ok "$1" "the gesture lock is $2"; return 0;; esac; sleep 1; n=$((n-1)); done
  fl_check FAIL "$1" "the gesture lock is not $2: [$(echo "$l" | sed 's/.*flashGestureLock/flashGestureLock/')]"
}
fl_check_flash_behind() { # NAME REGION RRGGBB yes|no: that colour, the page's behind the movie, in half the region's cells or more
  local g n
  g=$(fl_screen_sig "$2")
  n=$(echo "$g $3" | awk '{ r0 = hex(substr($2,1,2)); g0 = hex(substr($2,3,2)); b0 = hex(substr($2,5,2)); n = 0; for (i = 1; i <= length($1); i += 6) { r = hex(substr($1,i,2)); gg = hex(substr($1,i+2,2)); b = hex(substr($1,i+4,2)); if (abs(r-r0) <= 24 && abs(gg-g0) <= 24 && abs(b-b0) <= 24) n++ } print n }
    function abs(v) { return v < 0 ? -v : v }
    function hex(h,  i, v) { v = 0; h = tolower(h); for (i = 1; i <= length(h); i++) v = v * 16 + index("0123456789abcdef", substr(h, i, 1)) - 1; return v }')
  if { [ "$4" = yes ] && [ "${n:-0}" -ge 96 ]; } || { [ "$4" = no ] && [ "${n:-0}" -lt 96 ]; }; then fl_check ok "$1" "the colour $3 in ${n:-0} of 192 cells (wanted $4)"
  else fl_check FAIL "$1" "the colour $3 in ${n:-0} of 192 cells (wanted $4)"; fi
}
fl_screen_sig() { /tmp/fbstream /tmp/fl-sig -Q "$(fl_flash_region "$1")" -S 2>/dev/null | sed -n 's/.* grid \([0-9a-f]*\).*/\1/p'; }
fl_check_screen() { # NAME REGION REF want [TOL]: want is same|other; the mean difference per channel against the reference
  local l d tol
  tol=${5:-5}
  l=$(/tmp/fbstream /tmp/fl-sig -Q "$(fl_flash_region "$2")" -M "$3" 2>/dev/null); d=$(echo "$l" | sed -n 's/.* griddiff \([0-9.]*\).*/\1/p')
  if [ -z "$d" ]; then fl_check FAIL "$1" "no signature to compare"; return 1; fi
  if awk -v d=$d -v t=$tol -v w=$4 'BEGIN { s = (d <= t); exit !((w == "same" && s) || (w == "other" && !s)) }'; then fl_check ok "$1" "the screen against its reference: difference $d (same is $tol or less; wanted $4)"
  else fl_check FAIL "$1" "the screen against its reference: difference $d (same is $tol or less; wanted $4)"; fi
}
# the movie's process: this shell's plugin-container
fl_plugin_pid() { local p c; for p in $(pgrep -f plugin-container 2>/dev/null); do c=$(tr '\0' ' ' < /proc/$p/cmdline 2>/dev/null); case "$c" in *applications/$FL_ID/*plugin-container*) echo $p; return 0;; esac; done; return 1; }
fl_plugin_cpu() { set -- $(sed 's/.*) //' /proc/$1/stat 2>/dev/null); echo "$1 $(( ${12:-0} + ${13:-0} ))"; }   # state, utime + stime in ticks
fl_check_plugin() { # NAME [SECONDS] [want]: alive (it uses the CPU over the bound, a movie's own timer does) | wedged | gone
  local p a b v s0 c0 s1 c1
  p=$(fl_plugin_pid)
  if [ -z "$p" ]; then v=gone
  else
    set -- "$1" "${2:-3}" "${3:-alive}" $(fl_plugin_cpu $p); s0=$4; c0=$5
    sleep $2
    set -- "$1" "$2" "$3" $(fl_plugin_cpu $p); s1=$4; c1=$5
    if [ -z "$s1" ]; then v=gone; elif [ "$s1" = T ] || [ "$s1" = D ] || [ "$c1" = "$c0" ]; then v=wedged; else v=alive; fi
  fi
  [ "$v" = "${3:-alive}" ] && fl_check ok "$1" "plugin-container ${p:-none}: $v (state ${s1:-?}, cpu ticks ${c0:-?} to ${c1:-?})" || fl_check FAIL "$1" "plugin-container ${p:-none}: $v, wanted ${3:-alive} (state ${s1:-?}, cpu ticks ${c0:-?} to ${c1:-?})"
}
# the movie's audio: a stream of the player's process open on the device's playback device, and running
fl_audio_state() { # -> "open running|open|closed": the player's stream on PulseAudio (the ALSA plug-in's), the codec's PCM
  local p f st
  p=$(fl_plugin_pid)
  st=closed
  if [ -n "$p" ] && pactl list 2>/dev/null | grep -q "application.process.id = \"$p\""; then st=open; fi
  for f in /proc/asound/card*/pcm*p/sub*/status; do grep -q 'state: RUNNING' $f 2>/dev/null && { [ $st = open ] && st="open running"; break; }; done
  echo "$st"
}
fl_check_audio() { # NAME open|running|closed [BOUND]
  local n s
  n=$(( ${3:-6} * 2 ))
  while [ $n -gt 0 ]; do s=$(fl_audio_state); case "$2:$s" in open:open*|running:*running|closed:closed) fl_check ok "$1" "the movie's audio: $s"; return 0;; esac; usleep 500000; n=$((n-1)); done
  fl_check FAIL "$1" "the movie's audio: $s, wanted $2"
}
fl_wait_content() { # REGION [BOUND]: 8+ colours in the region (a slow page's first picture)
  local n c
  n=$(( ${2:-10} * 2 ))
  while [ $n -gt 0 ]; do c=$(fl_region "$1" | sed -n 's/.*colours \([0-9]*\).*/\1/p'); [ "${c:-0}" -ge 8 ] && return 0; usleep 500000; n=$((n-1)); done
  return 1
}
fl_wait_changed() { # REGION [BOUND]
  local n first cur
  n=$(( ${2:-5} * 5 )); first=$(fl_region "$1" | sed -n 's/.*hash \([0-9a-f]*\).*/\1/p')
  while [ $n -gt 0 ]; do usleep 200000; cur=$(fl_region "$1" | sed -n 's/.*hash \([0-9a-f]*\).*/\1/p'); [ -n "$cur" ] && [ "$cur" != "$first" ] && return 0; n=$((n-1)); done
  return 1
}
# an element's text holding TEXT, BOUND s at most (the page's own readout as a mark)
fl_wait_text() { # SEL TEXT [BOUND]
  local n t
  n=$(( ${3:-5} * 2 ))
  while [ $n -gt 0 ]; do t=$(fl_text "$1"); case "$t" in *"$2"*) echo "$t"; return 0;; esac; usleep 500000; n=$((n-1)); done
  return 1
}
fl_pick() { # SEL INDEX [ROWS]: the select brought mid-screen (an Enyo/Mochi list needs room below), tapped, the
            # card's picker open and its list still, its INDEXth row tapped (Mojo: by the centred rule, ROWS needed),
            # the picker gone (Mojo: its region still again; the caller's readback decides)
  local n p rx ry reg
  set -- "$1" "$2" "${3:-}" $(fl_rect "$1") || return 1; [ -n "${13:-}" ] || { echo "  pick: no element $1"; return 1; }
  p=$(fl_to_screen ${12} ${13} $8 $9); set -- "$1" "$2" "$3" $p $9
  if [ $5 -lt 200 ] || [ $5 -gt 420 ]; then n=$(awk -v ky=$6 -v sy=$5 -v z=$(fl_zoom) 'BEGIN { printf "%d", ky * z + sy - 350 + 0.5 }'); [ $n -lt 0 ] && n=0; fl_card_to $n; fi
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
# an element present and laid out (its rect answered with a size) within BOUND s: a hidden one has none
fl_wait_rect() {
  local n r; n=$(( ${2:-5} * 2 ))
  while [ $n -gt 0 ]; do r=$(fl_rect "$1") && { set -- "$1" "${2:-5}" $r; [ "$5" -gt 0 ] && [ "$6" -gt 0 ] && return 0; }; usleep 400000; n=$((n-1)); done
  return 1
}
fl_wait_load() { fl_wait 'load done uri=' ${1:-30}; }
fl_wait_painted() { fl_wait 'painted shmid=0x[0-9a-f]* bytes=[1-9]' ${1:-20}; }
# a zoom since the mark, settled: the asked zoom unchanged over 600 ms and a frame painted at it
fl_wait_zoom() {
  local n a last same
  fl_wait 'setZoomAndScroll zoom=' ${1:-10} >/dev/null || return 1
  n=$(( ${1:-10} * 3 )); last=""; same=0
  while [ $n -gt 0 ]; do
    a=$(fl_zoom_asked)
    if [ "$a" = "$last" ]; then same=$((same + 1)); else same=0; fi
    last=$a
    if [ $same -ge 2 ] && awk -v p=$(fl_zoom) -v a=$a 'BEGIN { d = p - a; if (d < 0) d = -d; exit !(d < 0.002) }'; then echo "$a"; return 0; fi
    usleep 300000; n=$((n-1))
  done
  return 1
}
fl_wait_gone() { # SEL [BOUND]: no such element, or one with no size
  local n r; n=$(( ${2:-5} * 2 ))
  while [ $n -gt 0 ]; do r=$(fl_rect "$1") || return 0; set -- "$1" "${2:-5}" $r; { [ "$5" = 0 ] || [ "$6" = 0 ]; } && return 0; usleep 400000; n=$((n-1)); done
  return 1
}
fl_wait_pref_has() { # T NAME TEXT [BOUND]: the pref's value holding TEXT
  local n v; n=$(( ${4:-5} * 2 ))
  while [ $n -gt 0 ]; do v=$(fl_pref $1 $2); case "$v" in *"$3"*) echo "$v"; return 0;; esac; usleep 500000; n=$((n-1)); done
  echo "$v"; return 1
}
fl_wait_differs() { # REGION SAMPLE [BOUND]: the region no longer shows the sample (a menu that came up during a hold)
  local n s cur
  s=$(eval "echo \$FL_S_$(echo $2 | tr -c 'A-Za-z0-9_' '_')"); n=$(( ${3:-5} * 5 ))
  [ -n "$s" ] || return 1
  while [ $n -gt 0 ]; do cur=$(fl_region "$1" | sed -n 's/.*hash \([0-9a-f]*\).*/\1/p'); [ -n "$cur" ] && [ "$cur" != "$s" ] && return 0; usleep 200000; n=$((n-1)); done
  return 1
}
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
fl_check_card_scroll_min() { # NAME MIN: the card's scroll y at least MIN (a flick coasted)
  local r ky
  r=$(fl_rect "sel:body") || { fl_check FAIL "$1" "no rect of body"; return 1; }
  set -- "$1" "$2" $r; ky=$8
  if [ $ky -ge $2 ]; then fl_check ok "$1" "the card's scroll y $ky (at least $2)"; else fl_check FAIL "$1" "the card's scroll y $ky (want at least $2)"; fi
}
fl_check_zoom_min() { # NAME Z: the zoom at least Z
  local z ok
  z=$(fl_zoom); ok=$(awk -v z=$z -v w=$2 'BEGIN { print (z >= w) ? 1 : 0 }')
  if [ "$ok" = 1 ]; then fl_check ok "$1" "zoom $z (at least $2)"; else fl_check FAIL "$1" "zoom $z (want at least $2)"; fi
}
fl_check_zoom_max() { # NAME Z: the zoom at most Z (T-643: a fit-zoomed page not zoomed onto a field)
  local z ok
  z=$(fl_zoom); ok=$(awk -v z=$z -v w=$2 'BEGIN { print (z <= w) ? 1 : 0 }')
  if [ "$ok" = 1 ]; then fl_check ok "$1" "zoom $z (at most $2)"; else fl_check FAIL "$1" "zoom $z (want at most $2)"; fi
}
fl_check_visible() { # NAME SEL: the element's screen rect inside what a finger can reach (above the keyboard while it is up)
  local r lim
  r=$(fl_screen_rect "$2") || { fl_check FAIL "$1" "no element $2"; return 1; }
  set -- "$1" "$2" $r; if [ "$(fl_kb)" = 1 ]; then lim=600; else lim=1024; fi
  # an element with no size is not on the screen (a hidden control bar read as visible at 0,82-0,82, 2026-11-08)
  [ $5 -gt $3 ] && [ $6 -gt $4 ] || { fl_check FAIL "$1" "$2 has no size on the screen ($3,$4-$5,$6)"; return 1; }
  if [ $4 -ge $FL_TOP ] && [ $6 -le $lim ]; then fl_check ok "$1" "$2 on the screen at $3,$4-$5,$6 (the reach ends at $lim, keyboard $(fl_kb))"; else fl_check FAIL "$1" "$2 on the screen at $3,$4-$5,$6 is not within $FL_TOP..$lim (keyboard $(fl_kb))"; fi
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
# a probe inside the page: the compat component's on-demand probe (jihad.compat.trace on for the run; the inject
# channel's javascript: URL is refused silently by a page's Content-Security-Policy, YouTube's). B64 is the script,
# base64 on the host (the device has no base64); the answer is FL_PROBE, printed to 300 chars
FL_PROBE=""
fl_probe() { # NAME B64
  local l
  fl_mark; fl_inject "setpref s jihad.compat.probe $2"; l=$(fl_wait 'probe \[' 30) || return 1
  FL_PROBE=$(echo "$l" | sed 's/.*probe \[[^]]*\] //'); echo "  probe $1: $(echo "$FL_PROBE" | cut -c1-300)"
}
fl_check_probe_has() { # NAME TEXT
  case "$FL_PROBE" in *"$2"*) fl_check ok "$1" "the probe's answer holds [$2]";; *) fl_check FAIL "$1" "the probe's answer [$(echo "$FL_PROBE" | cut -c1-200)] lacks [$2]";; esac
}
fl_check_url() { # NAME PART
  local u
  u=$(fl_url); case "$u" in *"$2"*) fl_check ok "$1" "load done uri=$u";; *) fl_check FAIL "$1" "load done uri=[$u] lacks $2";; esac
}
fl_check_dialog() { # NAME present|answered [BOUND]: since the mark
  local pat l n
  if [ "$2" = present ]; then
    # a dialog logs nothing until it is answered; its reply pipe stands in the state directory while it is up
    n=$(( ${3:-10} * 3 ))
    while [ $n -gt 0 ]; do l=$(ls $FL_STATE/dialog-*.fifo 2>/dev/null | tail -1); [ -n "$l" ] && { fl_check ok "$1" "the dialog's reply pipe stands: $l"; return 0; }; usleep 300000; n=$((n-1)); done
    fl_check FAIL "$1" "no dialog's reply pipe in $FL_STATE within ${3:-10} s"; return 1
  fi
  case $2 in answered) pat='dialog .* -> ';; *) pat="$2";; esac
  l=$(fl_wait "$pat" ${3:-10}) && fl_check ok "$1" "$l" || fl_check FAIL "$1" "no '$pat' in the daemon log within ${3:-10} s"
}
fl_check_keyboard() { # NAME up|down [BOUND]: the keyboard's state by fl_kb (a state, not an edge: already up is up)
  local f n
  case $2 in up) f=1;; down) f=0;; esac
  n=$(( ${3:-8} * 3 ))
  while [ $n -gt 0 ]; do [ "$(fl_kb)" = "$f" ] && { fl_check ok "$1" "the keyboard is $2 (the window server's last IME line for $FL_ID, or the daemon's editorFocused)"; return 0; }; usleep 300000; n=$((n-1)); done
  fl_check FAIL "$1" "the keyboard is not $2 within ${3:-8} s (its last reading: focused ${f:-?} wanted, $(fl_kb) read)"
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
