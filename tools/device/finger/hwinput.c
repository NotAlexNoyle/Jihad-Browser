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
 *   build: $TC/arm-webos-linux-gnueabi-gcc -O2 -march=armv7-a -mfloat-abi=softfp -o hwinput hwinput.c -ldl
 *   on the device (root):
 *     hwinput orientation N     send orientation N to every subscriber (fingerlib's fl_turn names them)
 *     hwinput list              the subscribers' sockets
 *     hwinput key CODE [MS]     press and release a gpio-keys key (held MS, default 80)
 *     hwinput level FILE        "peak P rms R frames N hz H" of raw s16le stereo samples
 *     hwinput record SOURCE MS [STREAM]
 *                               the same line for MS milliseconds of a PulseAudio source (pcm_output.monitor: the
 *                               sound as it reaches the output sink, after every stream's volume — sound read with
 *                               no ear). hz is the pitch by the left channel's upward zero crossings. Recorded
 *                               through libpulse-simple (dlopen), because the device's parec refuses every stream
 *                               name ("Invalid stream name": its locale conversion fails, 2026-11-16).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <linux/input.h>
#include <dlfcn.h>
#include <stdint.h>
#include <time.h>

/* the level of s16le stereo samples, fed in pieces; the pitch from the left channel's first 4 s, kept: upward
 * crossings between -peak/4 and +peak/4 (a plain zero-crossing count read a 440 Hz Vorbis tone as 511) */
#define LV_KEEP (44100 * 4)
static long long lv_n; static double lv_sq; static int lv_peak; static short lv_left[LV_KEEP]; static int lv_kept;
static void level_feed(const short* b, size_t k) {
  size_t i;
  for (i = 0; i < k; i++) {
    int v = b[i] < 0 ? -b[i] : b[i]; if (v > lv_peak) lv_peak = v; lv_sq += (double)b[i] * b[i];
    if (!(lv_n & 1) && lv_kept < LV_KEEP) lv_left[lv_kept++] = b[i];
    lv_n++;
  }
}
static void level_print(int rate) {
  int rms = 0, hz = 0, i, low = 0, thr, first = -1, last = -1; long long frames = lv_n / 2, cross = 0;
  if (lv_n) { double m = lv_sq / lv_n, r = m; int j; if (m > 0) { for (j = 0; j < 40; j++) r = (r + m / r) / 2; rms = (int)r; } }
  /* the threshold from the rms (one loud click must not hide the tone); the rate over the span between the first and
   * the last crossing (a tone that starts late in the window reads its own pitch) */
  thr = rms / 2;
  if (thr > 8) for (i = 0; i < lv_kept; i++) {
    if (lv_left[i] < -thr) low = 1;
    else if (lv_left[i] > thr && low) { cross++; low = 0; if (first < 0) first = i; last = i; }
  }
  if (cross > 1 && last > first) hz = (int)(((cross - 1) * (long long)rate + (last - first) / 2) / (last - first));
  printf("peak %d rms %d frames %lld hz %d\n", lv_peak, rms, frames, hz);
}
struct pa_ss { int format; uint32_t rate; uint8_t channels; };
static int record(const char* source, int ms, const char* stream) {
  alarm(ms / 1000 + 8);   /* a server that does not answer, or a source that yields nothing, blocks for ever: die instead (no line printed) */
  void* h = dlopen("libpulse-simple.so.0", RTLD_NOW); if (!h) { fprintf(stderr, "hwinput: %s\n", dlerror()); return 1; }
  void* (*snew)(const char*, const char*, int, const char*, const char*, const struct pa_ss*, const void*, const void*, int*) = dlsym(h, "pa_simple_new");
  int (*sread)(void*, void*, size_t, int*) = dlsym(h, "pa_simple_read");
  void (*sfree)(void*) = dlsym(h, "pa_simple_free");
  if (!snew || !sread || !sfree) { fprintf(stderr, "hwinput: libpulse-simple lacks a symbol\n"); return 1; }
  struct pa_ss ss = { 3 /* PA_SAMPLE_S16LE */, 44100, 2 }; int err = 0;
  void* s = snew(NULL, "hwinput", 2 /* PA_STREAM_RECORD */, source, stream, &ss, NULL, NULL, &err);
  if (!s) { fprintf(stderr, "hwinput: pa_simple_new(%s, %s) failed: %d\n", source, stream, err); return 1; }
  short buf[2048]; long long want = (long long)ms * 44100 / 1000 * 2; int bad = 0;
  while (lv_n < want) { if (sread(s, buf, sizeof buf, &err) < 0) { fprintf(stderr, "hwinput: pa_simple_read failed: %d\n", err); bad = 1; break; } level_feed(buf, 2048); }
  alarm(0); sfree(s); if (bad) return 1;
  level_print(44100); return 0;
}

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
    short buf[4096]; size_t k;
    while ((k = fread(buf, sizeof(short), 4096, s)) > 0) level_feed(buf, k);
    fclose(s); level_print(44100); return 0;
  }
  if (argc >= 4 && !strcmp(argv[1], "record")) return record(argv[2], atoi(argv[3]), argc > 4 ? argv[4] : "hwinput");
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
    fprintf(stderr, "usage: hwinput orientation N | list | key CODE [MS] | level FILE | record SOURCE MS [STREAM]\n"); return 2;
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
