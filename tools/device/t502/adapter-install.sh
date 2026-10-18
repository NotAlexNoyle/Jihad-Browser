A=/usr/lib/jihad/enyo/BrowserAdapterImpl.so
md5sum /tmp/BrowserAdapterImpl.so $A
mount -o remount,rw / && cp $A $A.orig 2>/dev/null; cp /tmp/BrowserAdapterImpl.so $A && sync && echo "adapter replaced: $(md5sum $A)"
mount -o remount,ro / 2>/dev/null || echo "(rootfs stays rw until a restart)"
touch /var/palm/jihad/enyo/adapterlog; rm -f /var/palm/jihad/enyo/adapter.log; echo gate
