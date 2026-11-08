# TouchPad: light sensor off

A webOS patch package for a TouchPad whose ambient light sensor has died. Found on 2026-11-08 while chasing a stall of
the Jihad Browser's test tablet: the sensor (an ISL29023 at I2C address 0x44) no longer answers, `sensord` polls it ten
times a second and fails every time, the kernel logs 45 to 100 lines a second about it (`I2C slave addr:0x44 not
connected`, `Failed reading COMMAND I register`), `/var/log/messages` rotates every two to five minutes and keeps ten
minutes of history, and at idle `sensord` holds a fifth of a core with the system time at 16 %.

**Measured on that tablet (30 s at idle, stock against patched, twice each):** the kernel's log lines 101 and 99 a
second to 0; `sensord` 621 and 571 jiffies to 473 and 481 (21 % of a core to 16 %); system time 16.4 % and 16.9 % to
10.8 % and 12.4 %; idle 66 % to 71-76 %. What does NOT change: the I2C controller's interrupt (`qup_err_intr`, 183),
about 400 a second either way — it fires for the live sensors' own transfers (the accelerometer and gyroscope polled
by the window server and by Atlas's sensor bridge), not for the dead one.

**What it does:** comments the `<isl29023>` driver block out of the sensor framework's driver lists,
`/etc/usd/usd_topaz-Wifi_default.xml` and `/etc/usd/usd_topaz-3G_default.xml` (`/usr/bin/usd_choose_conf.sh` picks one
by the kernel's `boardtype`), backs the originals up beside them as `.webosinternals.orig`, and restarts `sensord`, which
reads the list at start. The accelerometer, gyroscope and compass (mpu3050, lsm303dlh) are untouched: rotation keeps
working. Automatic brightness, which needs the light sensor, is lost — on such a tablet it already was. Removing the
package restores the files and restarts `sensord`.

**Self-contained:** webOS CE 3.1.0 ships busybox's `patch` and none of the webOS Internals patch tools (the
`webos-patches` feed is disabled for 3.1.0), so the scripts use busybox's `patch -p1` from `/`, check with `--dry-run`
first, refuse a file that is neither stock nor already patched, and keep a record under
`/var/usr/lib/.webosinternals.patches/<package>/` and a line in `/media/internal/webos-patches.log`, the places the
community's AUSMT patches use.

**Build and install**

```bash
packaging/patches/build-patch-ipk.sh touchpad-light-sensor-off          # -> /tmp/ipk/net.riverstonerelay.patches.touchpad-light-sensor-off_1.0.0_all.ipk
palm-install /tmp/ipk/net.riverstonerelay.patches.touchpad-light-sensor-off_1.0.0_all.ipk
#   or put the .ipk where the tablet can reach it and open it: on webOS CE Preware is the handler for .ipk files
#   or on the tablet: ipkg -o /media/cryptofs/apps install <the .ipk>      (what Preware runs)
```

The raw `data/touchpad-light-sensor-off.patch` carries the WebOS Quick Install header and can be side-loaded there too.

**Check it took** (on the tablet):

```sh
a=$(awk '/^ *183:/{print $2+$3}' /proc/interrupts); sleep 10; b=$(awk '/^ *183:/{print $2+$3}' /proc/interrupts); echo "$((b-a)) I2C error interrupts in 10 s"   # 2400 before, 0 after
grep -c "slave addr:0x44" /var/log/messages      # stops growing
```

Made from the CE 3.1.0 files (md5 `ed63161905770498a89bd26a0d94c26b` and `079b6de8d9a1f02817e1f1088646ff69`), which are
the stock 3.0.5 files unchanged (dated 2011-12-21 in the image).
