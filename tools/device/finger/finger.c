/* Copyright 2026 NotAlexNoyle. Apache-2.0.
 *
 * finger — an emulated finger for the HP TouchPad (webOS 3.0.5 / CE 3.1.0), for the project's
 * scripted finger rounds (docs/FINGER-TESTS.md). It travels the REAL input path: on this device
 * hidd reads the Cypress panel over /dev/ctp_uart and SENDS each scan as a datagram of Linux
 * `struct input_event`s (16 bytes each) to the Unix datagram socket LunaSysMgr binds at
 * /var/run/hidd/TouchpanelEventSocket (libhal's touchpanel module reads it, the Qt QPA plugin turns
 * it into QTouchEvents, the card's WebKit and the BrowserAdapter plugin see what a finger makes).
 * Nothing stops a root process from sending the same datagrams, so this tool does, with the
 * timing of a hand: no hidd, LunaSysMgr or daemon change.
 *
 *   build: $TC/arm-webos-linux-gnueabi-gcc -O2 -march=armv7-a -mfpu=neon -mfloat-abi=softfp -o finger finger.c
 *          (TC = build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin)
 *   on the device (root):
 *     finger tap X Y [MS]                 a tap (down, MS default 80, up)
 *     finger dtap X Y [TAP_MS] [GAP_MS]   a double tap (two taps of TAP_MS, default 70, GAP_MS apart, default 120)
 *     finger hold X Y [MS]                a long press (default 1500 ms)
 *     finger drag X0 Y0 X1 Y1 [MS]        a drag, the finger moving at the panel's scan rate (default 600 ms), lifted at rest
 *     finger flick X0 Y0 X1 Y1 [MS]       a flick: the same move but fast (default 160 ms) and lifted while moving
 *     finger pinch CX CY D0 D1 [MS]       two fingers on the horizontal through (CX,CY), D0 apart to D1 apart (default 600 ms)
 *     finger raw TYPE CODE VALUE ...      one datagram of the given events (for experiments; the EV_SYN is appended)
 *     Coordinates are the PORTRAIT screen's pixels as a screenshot shows them (x 0..767 across, y 0..1023
 *     down, the status bar at the top): the tool turns them into the panel's own frame, which is what
 *     hidd reports and LunaSysMgr maps — raw x = y, raw y = 767 - x (found 2026-10-24: a tap at raw
 *     (10, 737) opened the app menu at portrait (30, 10); a drag in that frame panned a page).
 *     FINGER_FRAME=raw sends the coordinates as given; SCAN_HZ (env, default 100) sets the move rate;
 *     FINGER_SOCKET (env) another socket path.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <linux/input.h>

/* The datagram libhal's touchpanel module (/lib/hal/modules/halTouchpanelMain.module,
 * touchpanel_get_event, read from the binary 2026-10-24) takes: a run of 16-byte input_events, one
 * EV_SYN at the end of the scan. Per finger: EV_FINGERID (type 7) opens a touch item whose id is
 * value*1000 + code; EV_ABS code 0 / 1 set x / y (any other code is "Unexpected"); EV_KEY BTN_TOUCH
 * value 1 marks the finger down, 0 up — sent on the first and the last scan only (the QPA plugin adds
 * a finger again on every "down"); EV_GESTURE (type 6) sets the gesture code and, in two events, the
 * x then y velocity (hidd's FLICK is code 6, with the Gesture FLICK minimum of the HidTouchpanel xml).
 * Up to 20 fingers a scan. Coordinates are the panel's LCD pixels, portrait: x 0..767, y 0..1023. */
#define EV_GESTURE  0x06
#define EV_FINGERID 0x07
#define GESTURE_FLICK 6

static const char* SOCK = "/var/run/hidd/TouchpanelEventSocket";
static int g_fd = -1;
static struct sockaddr_un g_addr;
static int g_hz = 100;
static int g_raw = 0;   /* FINGER_FRAME=raw: no portrait-to-panel conversion */

static void die(const char* m) { fprintf(stderr, "finger: %s: %s\n", m, strerror(errno)); exit(1); }
static void sleep_ms(int ms) { struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L }; nanosleep(&ts, NULL); }
static void stamp(struct input_event* e) { gettimeofday(&e->time, NULL); }
static void ev(struct input_event* out, int* n, uint16_t type, uint16_t code, int32_t value) {
  struct input_event* e = &out[(*n)++]; stamp(e); e->type = type; e->code = code; e->value = value;
}
static void send_scan(struct input_event* evs, int n) {
  ev(evs, &n, EV_SYN, SYN_REPORT, 0);
  if (sendto(g_fd, evs, n * sizeof(struct input_event), 0, (struct sockaddr*)&g_addr, sizeof g_addr) < 0) die("sendto");
}

/* One finger's part of a scan. The emission scheme must match what libhal's touchpanel module
 * (/lib/hal/modules/halTouchpanel*.module) parses: FINGER_SCHEME is set after reading it. */
/* phase: 1 = the first scan (down), 0 = a move scan, -1 = the last scan (up) */
typedef struct { int id; int x; int y; int phase; int xvel; int yvel; } Finger;
static void finger_events(struct input_event* evs, int* n, const Finger* f) {
  int rx = g_raw ? f->x : f->y, ry = g_raw ? f->y : 767 - f->x;   /* the panel's frame */
  ev(evs, n, EV_FINGERID, (uint16_t)f->id, 0);       /* id = 0*1000 + code */
  ev(evs, n, EV_ABS, ABS_X, rx);
  ev(evs, n, EV_ABS, ABS_Y, ry);
  if (f->phase == 1) ev(evs, n, EV_KEY, BTN_TOUCH, 1);
  if (f->phase == -1) {
    if (f->xvel || f->yvel) {                       /* the velocities in the panel's frame too */
      int vx = g_raw ? f->xvel : f->yvel, vy = g_raw ? f->yvel : -f->xvel;
      ev(evs, n, EV_GESTURE, GESTURE_FLICK, vx); ev(evs, n, EV_GESTURE, GESTURE_FLICK, vy);
    }
    ev(evs, n, EV_KEY, BTN_TOUCH, 0);
  }
}
static void scan(const Finger* fs, int count) {
  struct input_event evs[128]; int n = 0;
  for (int i = 0; i < count; i++) finger_events(evs, &n, &fs[i]);
  send_scan(evs, n);
}

static int lerp(int a, int b, double t) { return (int)(a + (b - a) * t + 0.5); }
static void move_one(int id, int x0, int y0, int x1, int y1, int ms, int lift_moving) {
  Finger f = { id, x0, y0, 1, 0, 0 };
  scan(&f, 1);                                     /* down */
  f.phase = 0;
  int steps = ms * g_hz / 1000; if (steps < 2) steps = 2;
  for (int i = 1; i <= steps; i++) {
    sleep_ms(1000 / g_hz);
    f.x = lerp(x0, x1, (double)i / steps); f.y = lerp(y0, y1, (double)i / steps);
    scan(&f, 1);                                   /* move */
  }
  if (lift_moving) {                               /* a flick: lifted while moving, hidd's FLICK gesture with the velocity (px/s) */
    f.xvel = (int)((long)(x1 - x0) * 1000 / ms); f.yvel = (int)((long)(y1 - y0) * 1000 / ms);
  } else sleep_ms(60);
  f.phase = -1; scan(&f, 1);                       /* up */
}

int main(int argc, char** argv) {
  if (argc < 2) { fprintf(stderr, "usage: see the header of finger.c\n"); return 2; }
  const char* path = getenv("FINGER_SOCKET"); if (path) SOCK = path;
  const char* hz = getenv("SCAN_HZ"); if (hz) g_hz = atoi(hz) > 0 ? atoi(hz) : 100;
  const char* fr = getenv("FINGER_FRAME"); if (fr && !strcmp(fr, "raw")) g_raw = 1;
  g_fd = socket(AF_UNIX, SOCK_DGRAM, 0); if (g_fd < 0) die("socket");
  memset(&g_addr, 0, sizeof g_addr); g_addr.sun_family = AF_UNIX; strncpy(g_addr.sun_path, SOCK, sizeof g_addr.sun_path - 1);
  const char* cmd = argv[1];
  int a[8] = {0}; for (int i = 2; i < argc && i < 10; i++) a[i - 2] = atoi(argv[i]);
  if (!strcmp(cmd, "tap")) {
    Finger f = { 1, a[0], a[1], 1, 0, 0 }; scan(&f, 1); sleep_ms(a[2] > 0 ? a[2] : 80); f.phase = -1; scan(&f, 1);
  } else if (!strcmp(cmd, "dtap")) {
    int tms = a[2] > 0 ? a[2] : 70, gms = a[3] > 0 ? a[3] : 120;
    for (int k = 0; k < 2; k++) { Finger f = { 1, a[0], a[1], 1, 0, 0 }; scan(&f, 1); sleep_ms(tms); f.phase = -1; scan(&f, 1); if (!k) sleep_ms(gms); }
  } else if (!strcmp(cmd, "hold")) {
    int ms = a[2] > 0 ? a[2] : 1500; Finger f = { 1, a[0], a[1], 1, 0, 0 }; scan(&f, 1); f.phase = 0;
    for (int t = 0; t < ms; t += 1000 / g_hz) { sleep_ms(1000 / g_hz); scan(&f, 1); }
    f.phase = -1; scan(&f, 1);
  } else if (!strcmp(cmd, "drag")) {
    move_one(1, a[0], a[1], a[2], a[3], a[4] > 0 ? a[4] : 600, 0);
  } else if (!strcmp(cmd, "flick")) {
    move_one(1, a[0], a[1], a[2], a[3], a[4] > 0 ? a[4] : 160, 1);
  } else if (!strcmp(cmd, "pinch")) {
    int cx = a[0], cy = a[1], d0 = a[2], d1 = a[3], ms = a[4] > 0 ? a[4] : 600;
    Finger f[2] = { { 1, cx - d0 / 2, cy, 1, 0, 0 }, { 2, cx + d0 / 2, cy, 1, 0, 0 } };
    scan(f, 2); f[0].phase = f[1].phase = 0;
    int steps = ms * g_hz / 1000; if (steps < 2) steps = 2;
    for (int i = 1; i <= steps; i++) {
      sleep_ms(1000 / g_hz); int d = lerp(d0, d1, (double)i / steps);
      f[0].x = cx - d / 2; f[1].x = cx + d / 2; scan(f, 2);
    }
    sleep_ms(60); f[0].phase = f[1].phase = -1; scan(f, 2);
  } else if (!strcmp(cmd, "raw")) {
    struct input_event evs[64]; int n = 0;
    for (int i = 2; i + 2 < argc && n < 60; i += 3) ev(evs, &n, (uint16_t)strtol(argv[i], 0, 0), (uint16_t)strtol(argv[i + 1], 0, 0), (int32_t)strtol(argv[i + 2], 0, 0));
    send_scan(evs, n);
  } else { fprintf(stderr, "finger: unknown command %s\n", cmd); return 2; }
  close(g_fd);
  return 0;
}
