#!/bin/bash
# Stage the Flash test SWFs and their HTML wrappers into the device's /tmp.
# /tmp is volatile: a reboot clears it, so this has to be re-run after every one.
# The wrappers are generated here rather than kept in the repo -- they are three
# lines each and the SWF they point at is the only thing that ever varies.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
ASSETS="$REPO/render/goanna/test"
STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT

NOVACOM=(novacom)
[ -n "${JIHAD_DEVICE:-}" ] && NOVACOM=(novacom -d "$JIHAD_DEVICE")

# SWFs to push. Each also gets a wrapper page named after it.
SWFS=(jihad-anim jihad-audio jihad-audio-mp3 jihad-audio-44k jihad-audio-5s jihad-audio-long
      jihad-key)

embed() {   # $1 = swf basename
  cat <<EOF
<!DOCTYPE html><html><head><meta charset="utf-8"><title>$1</title>
<style>body{margin:0;background:#222;color:#eee;font:14px sans-serif}</style></head>
<body><embed type="application/x-shockwave-flash" src="$1.swf" wmode="opaque" width="320" height="240"></body></html>
EOF
}

# The SWF-free page. Navigate here to release the patch-0027 CPU boost and to get
# a FRESH plugin-container on the next navigation to a Flash page.
plain() {
  cat <<'EOF'
<!DOCTYPE html><html><head><meta charset="utf-8"><title>jihad-plain</title>
<style>body{margin:0;background:#111;color:#0f0;font:20px monospace;padding:2em}</style></head>
<body><p id="marker">jihad-plain: no plugin on this page</p></body></html>
EOF
}

# Keyboard-arbitration page (cavekit-addons-extensions.md R7, keyboard criterion).
#
# THE OBSERVABLE IS THE ADDRESS BAR, NOT THIS PAGE. Arbitration is `handleKeyDown`'s
# RETURN VALUE inside LunaSysMgr; the key is forwarded to the daemon either way
# (BrowserAdapter.cpp:1613), so nothing the page or the SWF shows can answer it. The
# answer is a screenshot of the card's URL field. Everything below is instrumentation
# for the SUPPORTING questions -- did a key arrive at all, is the latch set, is the
# editor confound live -- so that a "no change in the URL field" reading is trustworthy
# instead of merely quiet. Full procedure: cavekit-addons-extensions.md, 2026-08-15.
#
#   #id  magenta banner. takeScreenShot captures the FOREGROUND card, and the
#        double-launch dance can make that the wrong one. If this banner is not in the
#        shot, the shot is of another card and proves nothing.
#   #sy  pageYOffset. NOT the arbitration observable (that assumption is a recorded dead
#        end). It is the pan instrument for the latch check: once the flash gesture lock
#        is set, a drag starting inside the plugin rect must stop panning the card.
#   #kc  count/last keyCode seen by a capturing window listener -- keys that reached the
#        CONTENT. Reset by a reload, unlike the SWF's one-shot green->red latch.
#   #fo  document.activeElement. The editor confound: the gate is
#        `bEditorFocused || mFlashGestureLock`, so if #box is focused the keys are
#        swallowed for the wrong reason and the plugin half is not under test.
keyarb() {
  cat <<'EOF'
<!DOCTYPE html><html><head><meta charset="utf-8"><title>jihad-keyarb</title>
<style>body{margin:0;background:#222;color:#eee;font:16px monospace}
#id{background:#f0f;color:#000;font:bold 26px monospace;padding:6px;letter-spacing:3px}
#hud{position:fixed;top:0;right:0;background:#000;color:#0f0;padding:4px;z-index:9;
     font:14px monospace;text-align:right}
embed{display:block}
#pad{height:3000px}</style></head>
<body><div id="id">JIHAD-KEYARB</div>
<div id="hud">sy=<span id="sy">0</span> kc=<span id="kc">0/-</span> fo=<span id="fo">-</span></div>
<embed type="application/x-shockwave-flash" src="jihad-key.swf" wmode="opaque" width="320" height="240">
<p>editor-focus control below -- must read fo=BODY for the plugin test</p>
<input id="box" size="40" value="">
<div id="pad">scroll region</div>
<script>
var sy=document.getElementById('sy'),kc=document.getElementById('kc'),
    fo=document.getElementById('fo'),n=0,last='-';
function upd(){
  sy.textContent=String(window.pageYOffset);
  kc.textContent=n+'/'+last;
  var a=document.activeElement;
  fo.textContent=a?(a.id||a.tagName||'?'):'none';
}
// Capture phase: fires before the <embed> target, so it counts keys the plugin also gets.
window.addEventListener('keydown',function(e){n++;last=e.keyCode;upd();},true);
window.addEventListener('scroll',upd,false);
setInterval(upd,250);upd();
</script></body></html>
EOF
}

plain  > "$STAGE/jihad-plain.html"
keyarb > "$STAGE/jihad-keyarb.html"
for s in "${SWFS[@]}"; do
  [ -f "$ASSETS/$s.swf" ] || { echo "missing asset: $ASSETS/$s.swf" >&2; exit 1; }
  cp "$ASSETS/$s.swf" "$STAGE/"
  embed "$s" > "$STAGE/$s.html"
done

fail=0
for f in "$STAGE"/*; do
  b="$(basename "$f")"
  "${NOVACOM[@]}" put "file:///tmp/$b" < "$f"
  # Verify: a short put can succeed and still land truncated.
  want="$(md5sum < "$f" | cut -d' ' -f1)"
  got="$(sleep 3 | "${NOVACOM[@]}" run file://usr/bin/md5sum -- "/tmp/$b" 2>/dev/null | cut -d' ' -f1)"
  if [ "$want" = "$got" ]; then
    echo "ok   $b  $want"
  else
    echo "FAIL $b  host=$want device=${got:-<none>}" >&2
    fail=1
  fi
done
exit $fail
