#!/bin/bash
# hide-client-addresses.sh — Riverstone Relay (squid-sslbump): sites are no longer told who the
# Relay's users are (the owner's decision, 2026-10-13; the PC's note RELAY2.md). Sets, in
# /usr/local/squid/etc/squid.conf, just above `cache deny all`:
#     forwarded_for delete
# The whole X-Forwarded-For header is removed, one a client sent itself included (`off` would send
# "unknown" instead, which still says an address was withheld). `via` is left alone: it names the
# Relay, not the user. An existing forwarded_for line, if any, is changed rather than doubled.
#
#   doas bash /home/<admin>/squid-hardening/hide-client-addresses.sh          apply (or confirm), parse, reconfigure, check
#   doas bash /home/<admin>/squid-hardening/hide-client-addresses.sh --undo   take the line out, parse, reconfigure, check
#
# Idempotent. squid.conf is backed up first (squid.conf.bak.<date>-xff, never overwritten); if
# `squid -k parse` rejects the result the backup is put back and nothing is reloaded. Evidence for
# the reply to the PC (the line in context, the parse result, what httpbin.org/ip sees for a plain,
# a bumped and a spliced request before and after, and the access-log facts with addresses masked)
# lands world-readable in /home/<admin>/squid-diag/hide-client-addresses-<stamp>.txt.
set -euo pipefail
SQ=/usr/local/squid
CONF=$SQ/etc/squid.conf
SQUID=$SQ/sbin/squid
LOGS=$SQ/var/logs
OUTDIR=/home/<admin>/squid-diag
STAMP=$(date -u +%Y%m%dT%H%M%SZ)
OUT=$OUTDIR/hide-client-addresses-$STAMP.txt
MARK="# Sites are not told who the Relay's users are (the owner, 2026-10-13)."
LINE='forwarded_for delete'
ANCHOR='^cache deny all'

[ "$(id -u)" = 0 ] || { echo "run as root:  doas bash $0 $*" >&2; exit 1; }
[ -f "$CONF" ] || { echo "no $CONF" >&2; exit 1; }
mkdir -p "$OUTDIR"

have() { grep -qxF "$LINE" "$CONF"; }
backup() {
  local b="$CONF.bak.$(date -u +%F)-xff" n=1
  while [ -e "$b" ]; do b="$CONF.bak.$(date -u +%F)-xff.$n"; n=$((n+1)); done
  cp -p "$CONF" "$b"; echo "$b"
}
parse_or_restore() {   # parse_or_restore BACKUP: squid -k parse must succeed and report no error line
  local log; log=$(mktemp)
  if ! "$SQUID" -k parse >"$log" 2>&1 || grep -qE '\| (FATAL|ERROR): ' "$log"; then
    cat "$log"; rm -f "$log"
    cp -p "$1" "$CONF"
    echo "squid -k parse rejected the configuration (above): $1 is back in place, nothing reloaded." >&2
    exit 1
  fi
  rm -f "$log"
}
probe() {   # what httpbin.org/ip is told, for the three kinds of request (client here is 127.0.0.1)
  local a b c
  a=$(curl -s --max-time 25 -x http://127.0.0.1:3128 http://httpbin.org/ip | tr -d '\n ' || true)
  b=$(curl -sk --max-time 25 -x http://127.0.0.1:3128 -A 'Mozilla/5.0' https://httpbin.org/ip | tr -d '\n ' || true)
  c=$(curl -s --max-time 25 -x http://127.0.0.1:3128 -A 'Mozilla/5.0 JihadBrowser/1.0' https://httpbin.org/ip | tr -d '\n ' || true)
  printf '  http  plain               : %s\n' "${a:-(no answer)}"
  printf '  https bumped (-k)         : %s\n' "${b:-(no answer)}"
  printf '  https spliced (Jihad UA)  : %s\n' "${c:-(no answer)}"
}

exec > >(tee "$OUT") 2>&1
echo "# hide-client-addresses.sh ${1:-apply} on $(hostname) at $STAMP"
echo; echo "== before =="; probe

if [ "${1:-}" = --undo ]; then
  if ! have; then echo; echo "the line is not in $CONF: nothing to undo"
  else
    B=$(backup); echo; echo "backup: $B"
    tmp=$(mktemp); grep -vxF -e "$MARK" -e "$LINE" "$CONF" > "$tmp"
    install -m 0644 -o root -g root "$tmp" "$CONF"; rm -f "$tmp"
    parse_or_restore "$B"
    "$SQUID" -k reconfigure; sleep 3
    echo "undone: X-Forwarded-For is sent again (Squid's default)"
  fi
elif have; then
  echo; echo "already in $CONF: nothing to change"
else
  B=$(backup); echo; echo "backup: $B"
  tmp=$(mktemp)
  if grep -qE '^[[:space:]]*forwarded_for[[:space:]]' "$CONF"; then
    echo "an existing forwarded_for line is changed: $(grep -nE '^[[:space:]]*forwarded_for[[:space:]]' "$CONF")"
    sed -E "s/^[[:space:]]*forwarded_for[[:space:]].*/$LINE/" "$CONF" > "$tmp"
  elif grep -qE "$ANCHOR" "$CONF"; then
    awk -v m="$MARK" -v l="$LINE" -v a="$ANCHOR" 'BEGIN{done=0} !done && $0 ~ a { print m; print l; done=1 } { print }' "$CONF" > "$tmp"
  else
    { cat "$CONF"; echo; echo "$MARK"; echo "$LINE"; } > "$tmp"
  fi
  install -m 0644 -o root -g root "$tmp" "$CONF"; rm -f "$tmp"
  parse_or_restore "$B"
  "$SQUID" -k reconfigure; sleep 3
  echo "applied: the Relay no longer passes its users' addresses on"
fi

echo; echo "== the line as it sits in $CONF (one line of context above and below) =="
grep -n -B1 -A1 -F "$LINE" "$CONF" || echo "(not present)"
echo; echo "== squid -k parse: error lines, if any =="
"$SQUID" -k parse 2>&1 | grep -E '\| (FATAL|ERROR): ' || echo "(none)"; echo "parse done"
echo; echo "== after =="; probe
echo; echo "== squid still up =="; pgrep -x -a squid | head -3 || echo "NO squid process"

echo; echo "== the access log, unchanged (report only) =="
echo "-- access_log / logformat / logfile_rotate / via lines in squid.conf (none = Squid's defaults):"
grep -nE '^[[:space:]]*(access_log|logformat|logfile_rotate|via)[[:space:]]' "$CONF" || echo "(none)"
echo "-- one line of $LOGS/access.log, the client address (field 3) masked:"
tail -n 1 "$LOGS/access.log" 2>/dev/null | awk '{ $3 = "<client-address>"; print }' || echo "(unreadable)"
echo "-- files:"; ls -la --time-style=long-iso "$LOGS"/access.log* "$LOGS"/cache.log* 2>/dev/null
echo "-- rotation ($SQ/etc/logrotate.conf, run by /etc/cron.d/squid-logrotate):"
grep -vE '^\s*#|^\s*$' "$SQ/etc/logrotate.conf"; grep -vE '^\s*#|^\s*$' /etc/cron.d/squid-logrotate
echo; echo "evidence: $OUT"
