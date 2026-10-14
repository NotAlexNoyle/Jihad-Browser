#!/bin/bash
# jihad-splice.sh — Riverstone Relay (squid-sslbump): let Jihad Browser's HTTPS pass through the
# proxy untouched, so the browser verifies every site's certificate itself (the owner's decision,
# 2026-10-13; the PC's note RELAY.md). Adds, above the first ssl_bump line of
# /usr/local/squid/etc/squid.conf:
#     acl jihad_browser browser JihadBrowser/
#     ssl_bump splice jihad_browser
# Every other client is handled exactly as before.
#
#   doas /home/<admin>/squid-hardening/jihad-splice.sh          apply (or confirm), parse, reconfigure, check
#   doas /home/<admin>/squid-hardening/jihad-splice.sh --undo   take the lines out, parse, reconfigure, check
#
# Idempotent. squid.conf is backed up first (squid.conf.bak.<date>-jihad, never overwritten); if
# `squid -k parse` rejects the result the backup is put back and nothing is reloaded. Evidence for
# the reply to the PC lands world-readable in /home/<admin>/squid-diag/jihad-splice-<stamp>.txt.
set -euo pipefail
SQ=/usr/local/squid
CONF=$SQ/etc/squid.conf
SQUID=$SQ/sbin/squid
OUTDIR=/home/<admin>/squid-diag
STAMP=$(date -u +%Y%m%dT%H%M%SZ)
OUT=$OUTDIR/jihad-splice-$STAMP.txt
MARK='# Jihad Browser does its own TLS (1.3): its HTTPS passes through untouched (the owner, 2026-10-13).'
ACL='acl jihad_browser browser JihadBrowser/'
RULE='ssl_bump splice jihad_browser'

[ "$(id -u)" = 0 ] || { echo "run as root:  doas $0 $*" >&2; exit 1; }
[ -f "$CONF" ] || { echo "no $CONF" >&2; exit 1; }
mkdir -p "$OUTDIR"

have() { grep -qxF "$ACL" "$CONF" && grep -qxF "$RULE" "$CONF"; }
backup() {
  local b="$CONF.bak.$(date -u +%F)-jihad" n=1
  while [ -e "$b" ]; do b="$CONF.bak.$(date -u +%F)-jihad.$n"; n=$((n+1)); done
  cp -p "$CONF" "$b"; echo "$b"
}
parse_or_restore() {   # parse_or_restore BACKUP: squid -k parse must succeed and report no error
  local log; log=$(mktemp)
  # Squid's own error lines read "FATAL: ..." / "ERROR: ..."; a directive name such as
  # sslproxy_cert_error must not count (it did, on the first run, 2026-10-13 17:05).
  if ! "$SQUID" -k parse >"$log" 2>&1 || grep -qE '\| (FATAL|ERROR): ' "$log"; then
    cat "$log"; rm -f "$log"
    cp -p "$1" "$CONF"
    echo "squid -k parse rejected the configuration (above): $1 is back in place, nothing reloaded." >&2
    exit 1
  fi
  cat "$log"; rm -f "$log"
}

if [ "${1:-}" = --undo ]; then
  if ! have; then echo "the lines are not in $CONF: nothing to undo"; exit 0; fi
  B=$(backup); echo "backup: $B"
  tmp=$(mktemp); grep -vxF -e "$MARK" -e "$ACL" -e "$RULE" "$CONF" > "$tmp"
  install -m 0644 -o root -g root "$tmp" "$CONF"; rm -f "$tmp"
  parse_or_restore "$B"
  "$SQUID" -k reconfigure; sleep 3
  echo "undone: Jihad is bumped again like everyone else"
elif have; then
  echo "already in $CONF: nothing to change"
else
  grep -qE '^[[:space:]]*ssl_bump' "$CONF" || { echo "no ssl_bump line in $CONF to put the rule above" >&2; exit 1; }
  B=$(backup); echo "backup: $B"
  tmp=$(mktemp)
  awk -v m="$MARK" -v a="$ACL" -v r="$RULE" 'BEGIN{done=0} !done && /^[[:space:]]*ssl_bump/ { print m; print a; print r; done=1 } { print }' "$CONF" > "$tmp"
  install -m 0644 -o root -g root "$tmp" "$CONF"; rm -f "$tmp"
  parse_or_restore "$B"
  "$SQUID" -k reconfigure; sleep 3
  echo "applied: Jihad's HTTPS is spliced, everyone else is bumped"
fi

{
  echo "# jihad-splice.sh ${1:-apply} on $(hostname) at $STAMP"
  echo; echo "== the lines as they sit in $CONF (one line of context above and below) =="
  grep -n -B1 -A1 -F "$RULE" "$CONF" || echo "(not present)"
  echo; echo "== squid -k parse =="
  "$SQUID" -k parse 2>&1 || true
  echo; echo "== through the proxy at 127.0.0.1:3128: the issuer each User-Agent gets for https://example.com/ =="
  for ua in 'Mozilla/5.0 JihadBrowser/1.0' 'Mozilla/5.0'; do
    printf '%-30s -> ' "$ua"
    curl -sk -v -o /dev/null -x http://127.0.0.1:3128 -A "$ua" --max-time 20 https://example.com/ 2>&1 | grep -i 'issuer:' || echo "(no issuer line: the request failed)"
  done
  echo; echo "== squid still up =="; pgrep -x -a squid | head -3 || echo "NO squid process"
} | tee "$OUT"
chmod a+r "$OUT"
echo; echo "evidence: $OUT"
