# Evidence, 2026-10-05

Device: HP TouchPad, webOS CE 3.1.0. Build on all three variants at the end of the day: bundle id
`a4a2f8e2b8bfdacab13129ed0d175d61`, libxul `65e3cc84891c…`, daemon `889263013e40…`.

| File | What it shows |
| --- | --- |
| `enyo_print-dialog-lists-the-epson.jpg`, `mochi_print-dialog-lists-the-epson.jpg` | Each shell's Print dialog listing the owner's Epson ET-2800 (192.168.1.253) and a Mac-shared ET-2720, found by the daemon. |
| `enyo_print-dialog-sent-one-page-to-the-sink.jpg`, `mochi_print-dialog-sent-three-pages-to-the-sink.jpg` | The dialogs after printing to the test sink (`render/goanna/test/ipp-sink.py`). |
| `sink_one-sheet-as-received.jpg`, `sink_three-sheets-of-the-tall-test-page.jpg` | What the sink received: the one-sheet test page; the three sheets of the tall page, no band missing or repeated. |
| `mochi_fresh-install-first-card.jpg` | First card of the Mochi variant installed from nothing. |
| `mochi-fresh-install-audit.diff` | `device-citizen-audit.sh` before/after that install: only the variant's own files; stock and Atlas checksums unchanged. |
| `mochi_share-image-opens-email.jpg` | Long-press sheet → Share Image: Email launched (its set-up screen; the device has no mail account). |
| `atlas-card-renders-beside-a-jihad-card.jpg` | An Atlas card rendering a page while a Jihad Enyo card is connected. |
| `atlas-fault-report-2026-10-02.txt` | The fault report from the 2026-10-02 reboot: SIGBUS in Atlas's own `BMScavenger` thread; "jihad" occurs nowhere in it. |
| `usb-alert_before_after-signal_20s-later.jpg` | The USB alert a LunaSysMgr restart leaves on screen; gone after the `MSMAvail` signal; still gone 20 s later. |
| `enyo_toolbar_share_newcard_bookmarks-icons_x6.jpg` | The Enyo toolbar's right-hand icons on the device, x6: Share, New Card, and the redrawn Bookmarks/History. |

Lines quoted in the kits, as read on the device:

```
printmgrd: _resolve_reply: not HP printer and doing printer filtering, address 192.168.1.253, ID "EPSON ET-2800 Series"
iptables -S: -P INPUT DROP
iptables -S: -A INPUT -m state --state RELATED,ESTABLISHED -j ALLOWED_PACKETS
iptables -L INPUT -n -v: 16549 3795K ALLOWED_PACKETS  udp  --  *  *  0.0.0.0/0  0.0.0.0/0  udp dpt:5353
print: mdns: EPSON ET-2800 Series._ipps._tcp.local -> 192.168.1.253:631 (answer from 192.168.1.253)
print: accepted the printer's own certificate for 192.168.1.253:631 (flags 1, session only)
print: printer: EPSON ET-2800 Series at 192.168.1.253:631/ipp/print (TLS)
print: sheet 1/3: 319981 bytes of JPEG, drawn in 250 ms, encoded in 916 ms
luna: printPage (REFUSED: caller is not this variant's card)
printer (Get-Jobs): job 31 state completed | name: jihad-plain | by: Jihad Browser | completed-successfully | impressions: 1

startup cache: bundle unchanged (id 2828…, 54 files, d51cc9175ca4f8b9, 34 ms) — kept
startup cache: bundle CHANGED (id 2828…, 54 files, d51cc9175ca4f8b9 -> 564b968b74635601, 46 ms) — 1 cached file(s) dropped
print: request [marker 7731]: DRY RUN (nothing is sent) to ipp://192.168.1.245:8090/ipp/print      <- the hand-replaced copy ran
startup cache: bundle CHANGED (id a4a2f8e2…, 54 files, 9551b5bc7cb9431f -> d2e83680d5f7d300, 53 ms) — 1 cached file(s) dropped   <- after the ipk upgrade (Enyo)
startup cache: bundle not seen before (id a4a2f8e2…, 54 files, none -> d2e83680d5f7d300, 23 ms) — 0 cached file(s) dropped       <- fresh install (Mochi)

root fs before: ro
ipkg: (offline root mode: not running net.riverstonerelay.jihad-browser.postinst)
jihad-postinst(enyo): / was already read-write (installer-driven)  leaving the mount state to the caller
jihad-postinst(enyo): leaving / read-write as found
root fs after: rw

getAppInfo (fresh Mochi, before the installer's rescan): "Invalid appId specified: net.riverstonerelay.jihad-browser-mochi"
ipkgservice/rescan: {"stdOut": ["{ \"returnValue\": true }"], "returnValue": true}     then the card launched, connected and painted

[PRV] signal  com.palm.lunasend  /storaged/MSMAvail «{"mode-avail":false}»      <- closes the USB alert
[PRV] call    com.palm.systemui 1000  /diskmode/hostIsConnected                  <- 23 s after a LunaSysMgr restart; the alert opens right after

JIHAD-TEST] holdpick:wallpaper: toast=[Wallpaper set.] wallpaper now="jihad-test.png" DONE     (restored to 19.jpg afterwards)
```
