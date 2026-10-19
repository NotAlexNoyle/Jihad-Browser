A=/usr/lib/jihad/enyo/BrowserAdapterImpl.so
md5sum /tmp/BrowserAdapterImpl.so $A
mount -o remount,rw / && cp $A $A.orig 2>/dev/null; cp /tmp/BrowserAdapterImpl.so $A && sync && echo "adapter replaced: $(md5sum $A)"
mount -o remount,ro / 2>/dev/null || echo "(rootfs stays rw until a restart)"
# The adapter's paint log is NOT switched on here any more: its gate (<state>/adapterlog) has no
# size cap, and left on from 2026-10-17 it filled /var (62 MB) by 2026-10-19, which hung the
# daemon's main thread in a journal commit. Switch it on for a round with
#   touch /var/palm/jihad/enyo/adapterlog
# and off after it with
#   rm -f /var/palm/jihad/enyo/adapterlog; : > /var/palm/jihad/enyo/adapter.log; rm -f /var/palm/jihad/enyo/adapter.log
rm -f /var/palm/jihad/enyo/adapterlog; df -h /var | tail -n 1
