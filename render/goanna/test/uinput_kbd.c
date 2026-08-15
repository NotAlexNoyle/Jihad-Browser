// Copyright 2026 NotAlexNoyle. Apache-2.0.
//
// Jihad Browser — a synthetic HARDWARE keyboard for the TouchPad, so that keyboard arbitration
// (cavekit-addons-extensions.md R7) can be tested at all.
//
// WHY THIS HAS TO EXIST. The criterion is "keys reach a focused plugin and do NOT simultaneously
// drive the card chrome". The chrome half lives in BrowserAdapter::handleKeyDown, which runs
// INSIDE LunaSysMgr — so it can only be exercised by a key that LunaSysMgr itself dispatches.
// The daemon's own inject channel (`key 65`) delivers straight to the page and bypasses the
// adapter completely, which is why it proved "Flash receives keys" and could never prove
// anything about the chrome.
//
// And this hardware has no keyboard to press: /proc/bus/input/devices lists only gpio-keys, the
// PMIC power key and the headset detect. The virtual keyboard webOS draws is rendered by
// LunaSysMgr and delivered in-process, not through /dev/input, so it is not a source either. A
// real Bluetooth keyboard would be — and uinput is the same path, which is the point: the kernel
// presents this as an ordinary input device and LunaSysMgr cannot tell the difference.
//
// Runs as a tiny daemon because the device must stay REGISTERED while LunaSysMgr enumerates its
// inputs: create it, restart LunaSysMgr, then feed it. Commands arrive as lines in a file so the
// whole thing is drivable over novacom, which has no usable interactive stdin.
//
//   uinput_kbd /tmp/uinput.cmd &        # create the device and poll for commands
//   echo "key 108 5" > /tmp/uinput.cmd  # 5 presses of KEY_DOWN (108)
//   echo "quit"      > /tmp/uinput.cmd  # destroy the device and exit
//
// Key codes are raw Linux input codes: KEY_UP=103, KEY_LEFT=105, KEY_RIGHT=106, KEY_DOWN=108,
// KEY_A=30, KEY_SPACE=57.
//
// DIAGNOSTIC ONLY — never shipped. Build with build/webos-oe/build-uinput-kbd.sh (PDK, so it
// links against the DEVICE's glibc 2.8 and needs no bundled loader).
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int g_fd = -1;

static int emit(int type, int code, int value)
{
  struct input_event ev;
  memset(&ev, 0, sizeof ev);
  ev.type = type;
  ev.code = code;
  ev.value = value;
  if (write(g_fd, &ev, sizeof ev) != (ssize_t)sizeof ev) {
    fprintf(stderr, "[uinput-kbd] write failed: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

// One complete press: down, sync, up, sync. The syncs are not optional — an input device that
// never reports EV_SYN looks to userspace like a key that is still being held.
static void tap(int code)
{
  emit(EV_KEY, code, 1);
  emit(EV_SYN, SYN_REPORT, 0);
  usleep(30000);
  emit(EV_KEY, code, 0);
  emit(EV_SYN, SYN_REPORT, 0);
  usleep(70000);
}

int main(int argc, char** argv)
{
  const char* cmdPath = (argc > 1) ? argv[1] : "/tmp/uinput.cmd";

  g_fd = open("/dev/input/uinput", O_WRONLY | O_NONBLOCK);
  if (g_fd < 0) {
    fprintf(stderr, "[uinput-kbd] open /dev/input/uinput: %s\n", strerror(errno));
    return 1;
  }

  if (ioctl(g_fd, UI_SET_EVBIT, EV_KEY) < 0 || ioctl(g_fd, UI_SET_EVBIT, EV_SYN) < 0) {
    fprintf(stderr, "[uinput-kbd] UI_SET_EVBIT: %s\n", strerror(errno));
    return 1;
  }
  // Advertise a broad key range: a device that only claims the four arrows can be classified as
  // something other than a keyboard by a host that is picky about capability bits.
  for (int k = 1; k < 128; k++) ioctl(g_fd, UI_SET_KEYBIT, k);

  struct uinput_user_dev dev;
  memset(&dev, 0, sizeof dev);
  snprintf(dev.name, UINPUT_MAX_NAME_SIZE, "Jihad Test Keyboard");
  dev.id.bustype = BUS_USB;
  dev.id.vendor = 0x1d6b;
  dev.id.product = 0x0104;
  dev.id.version = 1;
  if (write(g_fd, &dev, sizeof dev) != (ssize_t)sizeof dev) {
    fprintf(stderr, "[uinput-kbd] write uinput_user_dev: %s\n", strerror(errno));
    return 1;
  }
  if (ioctl(g_fd, UI_DEV_CREATE) < 0) {
    fprintf(stderr, "[uinput-kbd] UI_DEV_CREATE: %s\n", strerror(errno));
    return 1;
  }
  fprintf(stderr, "[uinput-kbd] created 'Jihad Test Keyboard'; polling %s\n", cmdPath);
  fflush(stderr);

  unlink(cmdPath);
  for (;;) {
    FILE* f = fopen(cmdPath, "r");
    if (f) {
      char line[256];
      while (fgets(line, sizeof line, f)) {
        int code = 0, count = 1;
        if (!strncmp(line, "quit", 4)) {
          fclose(f);
          unlink(cmdPath);
          ioctl(g_fd, UI_DEV_DESTROY);
          close(g_fd);
          fprintf(stderr, "[uinput-kbd] destroyed, exiting\n");
          return 0;
        }
        if (sscanf(line, "key %d %d", &code, &count) >= 1) {
          if (count < 1) count = 1;
          fprintf(stderr, "[uinput-kbd] key %d x%d\n", code, count);
          fflush(stderr);
          for (int i = 0; i < count; i++) tap(code);
        }
      }
      fclose(f);
      unlink(cmdPath);
    }
    usleep(200000);
  }
}
