/* Copyright 2026 NotAlexNoyle. Apache-2.0.
 *
 * hwinput — the TouchPad turned, and its volume keys pressed, with no hand (T-653: no test is a person's).
 * Both travel the REAL path.
 * The turn:
 * sensord reads the accelerometer over I2C and PUBLISHES each orientation change on webOS's
 * publish/subscribe channel (libpsc): one datagram to every subscriber's own Unix datagram socket,
 * /var/run/psc/orientation/sensord/data-<uuid> (LunaSysMgr binds one through libhal's
 * halSensorOrientationDefault.module). The datagram is the event alone, no header
 * (psc_publisher_send -> io_send_data -> sendto, read from libpsc.so 2026-11-16): an LCM-encoded
 * orientation_t, its 8-byte type hash 99 4d 0d 3e 09 8f 4e c8 and one int8, the orientation
 * (orientation_t_encode in libsensor.so). Nothing stops a root process from sending the same
 * datagram, so this tool does: no sensord, LunaSysMgr or browser change. sensord itself only
 * speaks when the device really turns, so what is sent here stands until the next send.
 *
 * Values seen on CE 3.1.0 (2026-11-16, screenshots): 3 portrait upside down, 4 portrait upright, 5 and 6 the two
 * landscapes; 0, 1, 2 and 7 change nothing.
 * The keys: the volume rocker is the kernel's gpio-keys input device (KEY_VOLUMEDOWN 114, KEY_VOLUMEUP 115), read by
 * hidd, which publishes com.palm.keys/audio for audiod. An input_event WRITTEN to the device's /dev/input/eventN is
 * injected by the kernel to every reader (evdev's write), so hidd sees a key as from the rocker. (A datagram to
 * LunaSysMgr's /var/run/hidd/KeypadEventSocket reaches the window server only: the volume did not move.)
 *
 *   build: $TC/arm-webos-linux-gnueabi-gcc -O2 -march=armv7-a -mfloat-abi=softfp -o hwinput hwinput.c
 *   on the device (root):
 *     hwinput orientation N     send orientation N to every subscriber (fingerlib's fl_turn names them)
 *     hwinput list              the subscribers' sockets
 *     hwinput key CODE [MS]     press and release a gpio-keys key (held MS, default 80)
 *     hwinput level FILE        "peak P rms R frames N" of raw s16le stereo samples (what `parec -d pcm_output.monitor`
 *                               wrote: the sound as it reaches the output sink — sound read with no ear)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <linux/input.h>

static const char* DIR = "/var/run/psc/orientation/sensord/data-";
static const unsigned char HASH[8] = { 0x99, 0x4d, 0x0d, 0x3e, 0x09, 0x8f, 0x4e, 0xc8 };

/* /dev/input/eventN of the device named NAME in /proc/bus/input/devices */
static int open_input(const char* name) {
  FILE* f = fopen("/proc/bus/input/devices", "r"); if (!f) return -1;
  char line[512], want[128], path[64]; int hit = 0, fd = -1;
  snprintf(want, sizeof want, "N: Name=\"%s\"", name);
  while (fgets(line, sizeof line, f)) {
    if (!strncmp(line, "N: ", 3)) hit = !strncmp(line, want, strlen(want));
    else if (hit && !strncmp(line, "H: ", 3)) {
      char* e = strstr(line, "event"); if (!e) break;
      snprintf(path, sizeof path, "/dev/input/event%d", atoi(e + 5)); fd = open(path, O_WRONLY); break;
    }
  }
  fclose(f); return fd;
}
static int ev(int fd, int type, int code, int value) {
  struct input_event e; memset(&e, 0, sizeof e); e.type = type; e.code = code; e.value = value;
  return write(fd, &e, sizeof e) == sizeof e ? 0 : -1;
}

int main(int argc, char** argv) {
  if (argc == 3 && !strcmp(argv[1], "level")) {
    FILE* s = fopen(argv[2], "rb"); if (!s) { perror(argv[2]); return 1; }
    short buf[4096]; size_t k, i; long long n = 0; double sq = 0; int peak = 0;
    while ((k = fread(buf, sizeof(short), 4096, s)) > 0)
      for (i = 0; i < k; i++) { int v = buf[i] < 0 ? -buf[i] : buf[i]; if (v > peak) peak = v; sq += (double)buf[i] * buf[i]; n++; }
    fclose(s);
    int rms = 0; if (n) { double m = sq / n, r = m; int j; if (m > 0) { for (j = 0; j < 40; j++) r = (r + m / r) / 2; rms = (int)r; } }
    printf("peak %d rms %d frames %lld\n", peak, rms, n / 2); return 0;
  }
  if (argc >= 3 && !strcmp(argv[1], "key")) {
    int code = atoi(argv[2]), ms = argc > 3 ? atoi(argv[3]) : 80, fd = open_input("gpio-keys");
    if (fd < 0) { perror("gpio-keys"); return 1; }
    if (ev(fd, EV_KEY, code, 1) || ev(fd, EV_SYN, 0, 0)) { perror("write"); return 1; }
    usleep(ms * 1000);
    if (ev(fd, EV_KEY, code, 0) || ev(fd, EV_SYN, 0, 0)) { perror("write"); return 1; }
    printf("hwinput: key %d for %d ms\n", code, ms); return 0;
  }
  int list = argc == 2 && !strcmp(argv[1], "list");
  if (!list && !(argc == 3 && !strcmp(argv[1], "orientation"))) {
    fprintf(stderr, "usage: hwinput orientation N | list | key CODE [MS] | level FILE\n"); return 2;
  }
  unsigned char msg[9]; memcpy(msg, HASH, 8); msg[8] = list ? 0 : (unsigned char)atoi(argv[2]);
  FILE* f = fopen("/proc/net/unix", "r"); if (!f) { perror("/proc/net/unix"); return 1; }
  int fd = socket(AF_UNIX, SOCK_DGRAM, 0); if (fd < 0) { perror("socket"); return 1; }
  char line[512]; int n = 0, bad = 0;
  while (fgets(line, sizeof line, f)) {
    char* p = strstr(line, DIR); if (!p) continue;
    p[strcspn(p, "\n")] = 0;
    if (list) { printf("%s\n", p); n++; continue; }
    struct sockaddr_un a; memset(&a, 0, sizeof a); a.sun_family = AF_UNIX; strncpy(a.sun_path, p, sizeof a.sun_path - 1);
    if (sendto(fd, msg, sizeof msg, 0, (struct sockaddr*)&a, sizeof a) < 0) { perror(p); bad++; } else n++;
  }
  fclose(f);
  if (!list) printf("hwinput: orientation %d to %d subscriber(s)%s\n", msg[8], n, bad ? ", some failed" : "");
  return (n && !bad) ? 0 : 1;
}
