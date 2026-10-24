# finger — the emulated finger for the TouchPad's finger rounds

`finger.c` sends the touch panel's own datagrams to LunaSysMgr, so a scripted tap, drag, flick, pinch, long
press or double tap travels the real path: hidd → LunaSysMgr (libhal's touchpanel module, the Qt QPA plugin) →
the card's WebKit → the BrowserAdapter plugin → the browser daemon → the page. Found and proven 2026-10-24
(`context/kits/cavekit-finger-emulation.md` has the path and the evidence).

## Build (the repository's toolchain) and install

```bash
TC=build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin
$TC/arm-webos-linux-gnueabi-gcc -O2 -march=armv7-a -mfpu=neon -mfloat-abi=softfp -std=gnu99 -o /tmp/finger tools/device/finger/finger.c
novacom put file:///tmp/finger < /tmp/finger          # then `chmod +x /tmp/finger` on the device
```

No binary is kept in the tree; `/tmp` on the device is wiped by a reboot.

## Commands (coordinates: the portrait screenshot's pixels, x 0..767, y 0..1023)

| | |
|---|---|
| `finger tap X Y [MS]` | a tap (down, 80 ms, up) |
| `finger dtap X Y [TAP_MS] [GAP_MS]` | a double tap |
| `finger hold X Y [MS]` | a long press (1500 ms): the context menu |
| `finger drag X0 Y0 X1 Y1 [MS]` | a drag at 100 scans a second (600 ms), lifted at rest |
| `finger flick X0 Y0 X1 Y1 [MS]` | a flick (160 ms), lifted while moving, with hidd's FLICK velocity: the scroller coasts |
| `finger pinch CX CY D0 D1 [MS]` | two fingers on the horizontal through CX,CY, from D0 to D1 apart |
| `finger raw TYPE CODE VALUE …` | one datagram of the given events (experiments); the EV_SYN is appended |

`FINGER_FRAME=raw` sends the coordinates in the panel's own frame (raw x = portrait y, raw y = 767 − portrait x);
`SCAN_HZ` changes the move rate; `FINGER_SOCKET` the socket path.

## A round

1. Open the card on the page with `luna-send -n 1 palm://com.palm.applicationManager/launch '{"id":"<app id>","params":{"target":"<url>"}}'`
   (the three app ids are in the workspace `CLAUDE.md`), wait for the load.
2. Gesture, wait, screenshot (`luna-send -n 1 palm://com.palm.systemmanager/takeScreenShot '{"file":"/tmp/x.png"}'`,
   `novacom get`, rotate −90° for portrait), judge by eye; for the GPU path read the daemon's timeline and frame
   dumps too (`tools/device/t502/README.md`).
3. The daemon's log is the oracle for what reached the engine (`/var/palm/jihad/<variant>/daemon.log`:
   `setZoomAndScroll` for a zoom, the scroll lines for a pan); LunaSysMgr's syslog (`/var/log/messages`) for what
   reached the window server (`editorFocused`, `IME:`, "Deadzone Tapped" for a touch in a dead zone,
   libhal's "Unexpected code" for a malformed datagram).
4. Delete the shots once judged (memory `clear-screenshots-when-done`); keep only what a kit note cites.

What it cannot do: the accelerometer (rotation), the volume keys, hearing audio, a physical keyboard. Those
stay a person's in `docs/FINGER-TESTS.md`.
