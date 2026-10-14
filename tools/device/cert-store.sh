#!/bin/sh
# Copyright 2026 NotAlexNoyle. Apache-2.0.
# On the TouchPad: put a test certificate in Certificate Manager's store the way a "trusted" one
# lands there (a file in /var/ssl/certs plus its hash link in /var/ssl/trustedcerts), take it out
# again, or count the store. It refuses a hash link that belongs to another certificate.
#   novacom put file:///tmp/jihad-NAME.pem < cert.pem
#   build/webos-oe/dev-sh.sh tools/device/cert-store.sh add NAME
#   build/webos-oe/dev-sh.sh tools/device/cert-store.sh rm NAME
#   build/webos-oe/dev-sh.sh tools/device/cert-store.sh count
if [ "${1:-}" = count ]; then
	echo "$(ls /var/ssl/certs | wc -l) files, $(ls /var/ssl/trustedcerts | wc -l) trusted links"; exit 0
fi
N="${2:?name}"; F=/tmp/jihad-$N.pem; C=/var/ssl/certs/jihadtest-$N.pem
if [ -f "$F" ]; then H=$(/usr/bin/openssl x509 -hash -noout -in "$F"); else H=$(/usr/bin/openssl x509 -hash -noout -in "$C"); fi
L=/var/ssl/trustedcerts/$H.0
case "${1:?add|rm|count}" in
	add) if [ -e "$L" ] && ! ls -l "$L" | grep -q "jihadtest-$N.pem"; then echo "REFUSED: $L is another certificate's"; exit 1; fi
	     cp "$F" "$C" && chmod 644 "$C" && ln -sf "../certs/jihadtest-$N.pem" "$L" ;;
	rm)  if ls -l "$L" 2>/dev/null | grep -q "jihadtest-$N.pem"; then rm -f "$L"; fi; rm -f "$C" "$F" ;;
	*) echo "usage: cert-store.sh add|rm NAME | count"; exit 2 ;;
esac
echo "$1 $N hash=$H: $(ls -l "$L" 2>/dev/null | sed 's/.* -> //')"
