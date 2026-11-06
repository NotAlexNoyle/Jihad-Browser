/* Copyright 2026 NotAlexNoyle. Apache-2.0.
 *
 * fbstream — the frame sampler of the finger rounds (cavekit-finger-testing.md R2, T-561). What the card
 * SHOWED while the emulated finger moved, read from the app framebuffer itself: the TouchPad draws the
 * cards into /dev/fb1 (1024 x 768 x 32 bpp in the panel's landscape frame, three pages of 768 rows, the
 * shown one named by the pan offset, flipped at every frame LunaSysMgr presents). A portrait row of the
 * screen is a COLUMN of the framebuffer (portrait y = fb x; the finger's frame too), so the sampler reads
 * every STRIDE-th framebuffer row of the shown page and folds each pixel into its column's signature: per
 * sample 1024 portrait-row signatures (an order-sensitive 32-bit hash down the sampled columns), each row's
 * profile (the sum of r+g+b over the sampled columns: a frame composed afresh at a zoom other than 1 has every
 * pixel a little different from the last — the bilinear phase — so the exact hash matches only copied rows
 * and the profile, which averages the difference away, matches the rest), each row's first pixel and whether
 * the row was one colour, the pan offset before and after the read (a read the card flipped under is taken
 * again, twice at most: the page it read may be the next frame's canvas), the read's cost. The signatures
 * take the portrait columns of -c (the page's scrolling content: a fixed sidebar in the same rows would make a
 * blank content row look like one that never moves); the columns outside -c get a signature of their own
 * (sig2), so a fixed part that stands in columns is checked for holding still as a fixed header is in rows. Between samples it polls the pan offset (a cheap ioctl) and records every flip: the exact time of
 * every frame shown, which the samples alone (10-20 a second) cannot give. Signatures only, by default; the
 * raw page on request. The daemon's timeline, the finger's trace and this tool share one clock (gettimeofday
 * ms) so the host joins them.
 *
 *   build: $TC/arm-webos-linux-gnueabi-gcc -O2 -march=armv7-a -mfpu=neon -mfloat-abi=softfp -std=gnu99 -o fbstream fbstream.c
 *          (TC = build/webos-oe/toolchain/out-toolchain/x-tools/arm-webos-linux-gnueabi/bin; GLIBC_2.4 symbols only,
 *          so it runs on the device's own libc — no clock_gettime, which would need the bundled loader)
 *   on the device (root):
 *     fbstream OUT [-s SECONDS] [-z HZ] [-k STRIDE] [-d /dev/fb1] [-p] [-P POLL_US] [-R N] [-L] [-f TRACE]
 *       -s   how long (default 12 s); SIGTERM or SIGINT ends it early, the file complete
 *       -z   samples a second (default 15)              -k  every STRIDE-th framebuffer row read (default 8: 96 of 768;
 *                                                            measured 2026-11-03: 2.7 ms a read and 5.6 % of a core at 15/s,
 *                                                            stride 4 13 ms and 12 %, pread twice mmap's cost)
 *       -p   read with pread instead of mmap             -P  the pan poll's period in microseconds (default 1000)
 *       -R N also write every N-th sample's whole shown page to OUT.rawNNNN (3 MB each: /tmp is a 40 MB tmpfs
 *            that the daemon's dumps share — a round's OUT belongs under /media/internal; a failed write is fatal)
 *       -L   write the last shown page whole to OUT.last.raw at the end
 *       -f   the finger's trace (FINGER_TRACE of finger.c): each sample gets the finger's last scan at or before it
 *       -c X0-X1 the portrait columns (0..767) the content signatures take (default all); the rest go to sig2
 *       -U ROWS  shoot on the event: when a run of ROWS or more one-colour rows appears where the previous sample had
 *            content, the whole shown page goes to OUT.eventNNNN.raw (three at most): the frame a person would
 *            have to see, kept without a person
 *       -Q X0,Y0,X1,Y1  ONE SHOT (T-563, the check library): no stream; the shown page is read once and the portrait
 *            region [X0,X1) x [Y0,Y1) summed up on stdout — "region X0,Y0-X1,Y1: colours N hash H rows R uniform U"
 *            (N distinct 24-bit colours over every other pixel, 4096 at most; H an order-sensitive hash of every pixel;
 *            U of the R portrait rows one colour), then "light F-L" — the first and last portrait row of the region
 *            whose pixel at the region's centre column is light (every channel 0xd0 or more: a card's white popup
 *            over its dimmed page), or "light none", and "blue F-L" the same for a blue pixel (Mochi's OK button), or
 *            "blue none" — with -L the page goes whole to OUT.last.raw (a failed check's frame)
 *     The summary on stdout at the end: samples, rate, the read's cost, flips, polls, this process's CPU.
 *
 *   OUT (little-endian): a header, then records of (u32 type, u32 bytes, payload):
 *     header: "FBS3", u32 header bytes, xres, yres, vyres, line_length, bpp, rows (= xres), cols (sampled fb rows),
 *             stride, hz, orient (1: portrait row = fb x), poll_us, pread, u64 start ms, u32 column lo, column hi
 *     type 1 sample: u64 t ms, u32 yoffset before, yoffset after, read us, flips so far, tries (1: clean; 2-3: read
 *             again after a flip; 0: still torn after three), i32 finger x, y, phase (1 down, 0 moving, -1 up, -2 none),
 *             u32 sig[rows], u32 first[rows], u32 prof[rows], u32 sig2[rows], u8 uniform bits[(rows+7)/8]
 *             ("FBS1", 2026-11-03 morning, had no prof; "FBS2" no sig2 and no column range in the header)
 *     type 2 flip: u64 t ms, u32 yoffset
 *     type 3 trailer: u32 samples, flips, polls, cpu ms (user + system), u64 end ms
 *   tools/host/finger-stream.py reads it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <linux/fb.h>

static volatile sig_atomic_t g_stop = 0;
static void on_sig(int s) { (void)s; g_stop = 1; }
static int64_t now_us(void) { struct timeval tv; gettimeofday(&tv, NULL); return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec; }
static void sleep_us(long us) { if (us <= 0) return; struct timespec ts = { us / 1000000, (us % 1000000) * 1000L }; nanosleep(&ts, NULL); }
static void die(const char* m) { fprintf(stderr, "fbstream: %s: %s\n", m, strerror(errno)); exit(1); }
static void w32(FILE* f, uint32_t v) { fwrite(&v, 4, 1, f); }
/* the whole shown page to a file; read again (twice at most) when the card flipped during the copy — a torn page has
 * the portrait screen's left from one frame and its right from the next (the copy walks the framebuffer's rows, which
 * are the portrait's columns). Returns the tries (0: still torn). */
static int grab_page(int fd, const unsigned char* map, int use_pread, size_t line, size_t page, unsigned char** pagebuf) {
  struct fb_var_screeninfo vi; int tries = 0;
  if (!*pagebuf) *pagebuf = malloc(page);
  if (!*pagebuf) return 0;
  for (;;) {
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) return 0;
    const uint32_t yoff = vi.yoffset; tries++;
    if (use_pread) { if (pread(fd, *pagebuf, page, (size_t)yoff * line) != (ssize_t)page) return 0; }
    else memcpy(*pagebuf, map + (size_t)yoff * line, page);
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) return 0;
    if (vi.yoffset == yoff) break;
    if (tries >= 3) { tries = 0; break; }
  }
  return tries;
}
static int dump_page(const char* path, int fd, const unsigned char* map, int use_pread, size_t line, size_t page, unsigned char** pagebuf) {
  const int tries = grab_page(fd, map, use_pread, line, page, pagebuf);
  if (!*pagebuf) return 0;
  FILE* rf = fopen(path, "wb"); if (!rf) return 0;
  fwrite(*pagebuf, 1, page, rf); fclose(rf);
  return tries;
}
static void w64(FILE* f, uint64_t v) { fwrite(&v, 8, 1, f); }
static void wi32(FILE* f, int32_t v) { fwrite(&v, 4, 1, f); }

typedef struct { int64_t t; int id, x, y, phase; } Scan;
static Scan* g_trace = NULL; static int g_ntrace = 0;
static void read_trace(const char* path) {
  FILE* f = fopen(path, "r"); if (!f) { fprintf(stderr, "fbstream: no trace %s\n", path); return; }
  char line[160]; int cap = 0;
  while (fgets(line, sizeof line, f)) {
    if (line[0] == '#') continue;
    char* e = line; long long t = strtoll(e, &e, 10); int id = (int)strtol(e, &e, 10), x = (int)strtol(e, &e, 10);
    int y = (int)strtol(e, &e, 10), ph = (int)strtol(e, &e, 10);   /* strtol, not sscanf: GLIBC_2.4 only */
    if (e == line) continue;
    if (g_ntrace == cap) { cap = cap ? cap * 2 : 1024; g_trace = realloc(g_trace, cap * sizeof(Scan)); if (!g_trace) die("realloc"); }
    Scan* s = &g_trace[g_ntrace++]; s->t = t; s->id = id; s->x = x; s->y = y; s->phase = ph;
  }
  fclose(f);
}
/* the finger's last scan (finger 1) at or before t ms */
static const Scan* scan_at(int64_t t) {
  const Scan* best = NULL;
  for (int i = 0; i < g_ntrace; i++) { if (g_trace[i].id != 1) continue; if (g_trace[i].t <= t) best = &g_trace[i]; else break; }
  return best;
}

int main(int argc, char** argv) {
  if (argc < 2) { fprintf(stderr, "usage: see the header of fbstream.c\n"); return 2; }
  const char* out = argv[1]; const char* dev = "/dev/fb1"; const char* trace = NULL;
  double seconds = 12; int hz = 15, stride = 8, use_pread = 0, poll_us = 1000, raw_every = 0, last_raw = 0, uni_event = 0;
  int col_lo = 0, col_hi = 768; const char* region = NULL;
  for (int i = 2; i < argc; i++) {
    const char* a = argv[i]; const char* v = (i + 1 < argc) ? argv[i + 1] : "";
    if (!strcmp(a, "-s")) { seconds = atof(v); i++; } else if (!strcmp(a, "-z")) { hz = atoi(v); i++; }
    else if (!strcmp(a, "-k")) { stride = atoi(v); i++; } else if (!strcmp(a, "-d")) { dev = v; i++; }
    else if (!strcmp(a, "-p")) use_pread = 1; else if (!strcmp(a, "-P")) { poll_us = atoi(v); i++; }
    else if (!strcmp(a, "-R")) { raw_every = atoi(v); i++; } else if (!strcmp(a, "-L")) last_raw = 1;
    else if (!strcmp(a, "-f")) { trace = v; i++; }
    else if (!strcmp(a, "-U")) { uni_event = atoi(v); i++; }
    else if (!strcmp(a, "-c")) { col_lo = atoi(v); const char* h = strchr(v, '-'); col_hi = h ? atoi(h + 1) : 768; i++; }
    else if (!strcmp(a, "-Q")) { region = v; i++; }
    else { fprintf(stderr, "fbstream: unknown option %s\n", a); return 2; }
  }
  if (hz < 1) hz = 1;
  if (stride < 1) stride = 1;
  if (poll_us < 100) poll_us = 100;

  int fd = open(dev, O_RDONLY); if (fd < 0) die("open fb");
  struct fb_var_screeninfo vi; struct fb_fix_screeninfo fi;
  if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) die("FBIOGET_VSCREENINFO");
  if (ioctl(fd, FBIOGET_FSCREENINFO, &fi) < 0) die("FBIOGET_FSCREENINFO");
  if (vi.bits_per_pixel != 32) { fprintf(stderr, "fbstream: %s is %u bpp, not 32\n", dev, vi.bits_per_pixel); return 1; }
  const int rows = vi.xres;                       /* portrait rows = framebuffer columns */
  const int cols = (vi.yres + stride - 1) / stride; /* sampled framebuffer rows per page */
  const size_t line = fi.line_length, page = line * vi.yres, whole = line * vi.yres_virtual;
  const unsigned char* map = NULL;
  if (!use_pread) {
    map = mmap(NULL, whole, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) { map = NULL; use_pread = 1; fprintf(stderr, "fbstream: mmap failed (%s), pread\n", strerror(errno)); }
  }
  if (region) {   /* -Q: one shot, the region summed up, no stream (T-563) */
    int x0 = 0, y0 = 0, x1 = 0, y1 = 0; const char* e = region;
    x0 = (int)strtol(e, (char**)&e, 10); if (*e == ',') e++; y0 = (int)strtol(e, (char**)&e, 10); if (*e == ',') e++;
    x1 = (int)strtol(e, (char**)&e, 10); if (*e == ',') e++; y1 = (int)strtol(e, (char**)&e, 10);
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > (int)vi.yres) x1 = vi.yres;
    if (y1 > rows) y1 = rows;
    if (x1 <= x0 || y1 <= y0) { fprintf(stderr, "fbstream: -Q wants X0,Y0,X1,Y1 inside 768x1024 with X1>X0, Y1>Y0\n"); return 2; }
    unsigned char* pagebuf = NULL;
    const int tries = grab_page(fd, map, use_pread, line, page, &pagebuf);
    if (!pagebuf) die("grab");
    /* distinct colours over every other pixel each way: an open-addressing table, 4096 distinct at most */
    enum { CAP = 4096, TAB = 8192 };
    static uint32_t tab[TAB]; static unsigned char used[TAB]; int ncol = 0, nuni = 0;
    uint32_t hash = 2166136261u;
    const int cx = (x0 + x1) / 2; int lightF = -1, lightL = -1, blueF = -1, blueL = -1;
    for (int py = y0; py < y1; py++) {          /* a portrait row is a framebuffer column */
      uint32_t firstp = 0; int uni = 1;
      for (int px = x0; px < x1; px++) {
        const int r = (int)vi.yres - 1 - px;      /* the fb row of portrait column px */
        const uint32_t p = *(const uint32_t*)(pagebuf + (size_t)r * line + (size_t)py * 4) & 0xffffffu;
        hash = (hash * 16777619u) ^ p;
        if (px == cx && (p & 0xff) >= 0xd0 && ((p >> 8) & 0xff) >= 0xd0 && ((p >> 16) & 0xff) >= 0xd0) { if (lightF < 0) lightF = py; lightL = py; }
        if (px == cx && (p & 0xff) >= 0x90 && (p & 0xff) >= ((p >> 16) & 0xff) + 0x30) { if (blueF < 0) blueF = py; blueL = py; }
        if (px == x0) firstp = p; else if (p != firstp) uni = 0;
        if (((px - x0) & 1) == 0 && ((py - y0) & 1) == 0 && ncol < CAP) {
          uint32_t h = (p * 2654435761u) >> 19; int found = 0;
          for (int k = 0; k < TAB; k++) { const uint32_t i = (h + k) & (TAB - 1); if (!used[i]) { used[i] = 1; tab[i] = p; break; } if (tab[i] == p) { found = 1; break; } }
          if (!found) ncol++;
        }
      }
      if (uni) nuni++;
    }
    printf("region %d,%d-%d,%d: colours %d hash %08x rows %d uniform %d", x0, y0, x1, y1, ncol, hash, y1 - y0, nuni);
    if (lightF >= 0) printf(" light %d-%d", lightF, lightL); else printf(" light none");
    if (blueF >= 0) printf(" blue %d-%d", blueF, blueL); else printf(" blue none");
    printf("%s\n", tries == 0 ? " (TORN)" : "");
    if (last_raw) {
      char path[512]; snprintf(path, sizeof path, "%s.last.raw", out);
      FILE* rf = fopen(path, "wb"); if (!rf) die("open last.raw");
      fwrite(pagebuf, 1, page, rf); if (fclose(rf) != 0) die("write last.raw");
    }
    free(pagebuf); if (map) munmap((void*)map, whole); close(fd);
    return 0;
  }
  uint32_t* row = malloc(line); uint32_t* sig = malloc(rows * 4); uint32_t* first = malloc(rows * 4);
  uint32_t* prof = malloc(rows * 4); uint32_t* sig2 = malloc(rows * 4);
  unsigned char* uni = malloc((rows + 7) / 8); unsigned char* uni_prev = malloc((rows + 7) / 8);
  unsigned char* pagebuf = NULL; int events = 0;
  if (!row || !sig || !first || !prof || !sig2 || !uni || !uni_prev) die("malloc");
  if (col_lo < 0) col_lo = 0;
  if (col_hi > (int)vi.yres) col_hi = vi.yres;
  if (col_hi <= col_lo) { col_lo = 0; col_hi = vi.yres; }
  memset(uni_prev, 0, (rows + 7) / 8);

  FILE* f = fopen(out, "wb"); if (!f) die("open out");
  const int64_t start = now_us();
  fwrite("FBS3", 4, 1, f); w32(f, 4 + 13 * 4 + 8 + 8); w32(f, vi.xres); w32(f, vi.yres); w32(f, vi.yres_virtual); w32(f, line);
  w32(f, vi.bits_per_pixel); w32(f, rows); w32(f, cols); w32(f, stride); w32(f, hz); w32(f, 1); w32(f, poll_us); w32(f, use_pread);
  w64(f, start / 1000); w32(f, col_lo); w32(f, col_hi);
  signal(SIGTERM, on_sig); signal(SIGINT, on_sig);

  const int64_t period = 1000000 / hz, end = start + (int64_t)(seconds * 1000000);
  int64_t next = start; uint32_t last_yoff = vi.yoffset, samples = 0, flips = 0, polls = 0;
  int64_t read_sum = 0, read_max = 0;
  while (!g_stop) {
    const int64_t t = now_us(); if (t >= end) break;
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) die("FBIOGET_VSCREENINFO");
    polls++;
    if (vi.yoffset != last_yoff) { last_yoff = vi.yoffset; flips++; w32(f, 2); w32(f, 12); w64(f, t / 1000); w32(f, vi.yoffset); }
    if (t < next) { int64_t d = next - t; sleep_us(d < poll_us ? d : poll_us); continue; }
    next += period; if (next < t) next = t + period;   /* a late sample does not pile up */
    const int64_t r0 = now_us(); uint32_t yoff0 = vi.yoffset, tries = 0; int64_t r1 = r0;
    for (;;) {   /* the shown page; read again (twice at most) when the card flipped during the read */
      yoff0 = vi.yoffset; tries++;
      memset(uni, 0xff, (rows + 7) / 8); int started = 0;
      for (int x = 0; x < rows; x++) sig2[x] = 2166136261u;
      for (int i = 0; i < cols; i++) {
        const int y = i * stride;                      /* the fb row: portrait column 767 - y */
        const int px = (int)vi.yres - 1 - y;
        const size_t off = (size_t)(yoff0 + y) * line;
        if (use_pread) { if (pread(fd, row, line, off) != (ssize_t)line) die("pread"); }
        else memcpy(row, map + off, line);
        if (px < col_lo || px >= col_hi) {             /* outside the content columns: sig2 only */
          for (int x = 0; x < rows; x++) sig2[x] = (sig2[x] * 16777619u) ^ row[x];
          continue;
        }
        if (!started) { started = 1; for (int x = 0; x < rows; x++) { const uint32_t p = row[x]; first[x] = p; sig[x] = 2166136261u ^ p; prof[x] = (p & 0xff) + ((p >> 8) & 0xff) + ((p >> 16) & 0xff); } }
        else for (int x = 0; x < rows; x++) {
          const uint32_t p = row[x];
          sig[x] = (sig[x] * 16777619u) ^ p;
          prof[x] += (p & 0xff) + ((p >> 8) & 0xff) + ((p >> 16) & 0xff);
          if (p != first[x]) uni[x >> 3] &= (unsigned char)~(1u << (x & 7));
        }
      }
      r1 = now_us();
      if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) die("FBIOGET_VSCREENINFO");
      polls++;
      if (vi.yoffset != last_yoff) { last_yoff = vi.yoffset; flips++; w32(f, 2); w32(f, 12); w64(f, r1 / 1000); w32(f, vi.yoffset); }
      if (vi.yoffset == yoff0) break;
      if (tries >= 3) { tries = 0; break; }
    }
    const int64_t rd = r1 - r0; read_sum += rd; if (rd > read_max) read_max = rd;
    w32(f, 1); w32(f, 8 + 5 * 4 + 3 * 4 + rows * 16 + (rows + 7) / 8);
    w64(f, r0 / 1000); w32(f, yoff0); w32(f, vi.yoffset); w32(f, (uint32_t)rd); w32(f, flips); w32(f, tries);
    wi32(f, -1); wi32(f, -1); wi32(f, -2);          /* the finger: filled at the end from the trace */
    fwrite(sig, 4, rows, f); fwrite(first, 4, rows, f); fwrite(prof, 4, rows, f); fwrite(sig2, 4, rows, f); fwrite(uni, 1, (rows + 7) / 8, f);
    samples++;
    if (uni_event > 0 && samples > 1 && events < 3 && tries > 0) {   /* a new one-colour run where there was content */
      int run = 0, hit = 0;
      for (int x = 0; x < rows && !hit; x++) {
        const int u = (uni[x >> 3] >> (x & 7)) & 1, up = (uni_prev[x >> 3] >> (x & 7)) & 1;
        if (u && !up) { if (++run >= uni_event) hit = 1; } else run = 0;
      }
      if (hit) {
        char path[512]; snprintf(path, sizeof path, "%s.event%04u.raw", out, samples);
        const int tr = dump_page(path, fd, map, use_pread, line, page, &pagebuf);
        events++;
        fprintf(stderr, "fbstream: sample %u: a one-colour run of %d+ rows where the last sample had content: %s%s\n", samples, uni_event, path,
                tr == 0 ? " (TORN: the card flipped during each of three copies)" : tr > 1 ? " (copied again after a flip)" : "");
      }
    }
    if (tries > 0) memcpy(uni_prev, uni, (rows + 7) / 8);
    if (raw_every > 0 && (samples % raw_every) == 0) {
      char path[512]; snprintf(path, sizeof path, "%s.raw%04u", out, samples);
      if (!dump_page(path, fd, map, use_pread, line, page, &pagebuf)) fprintf(stderr, "fbstream: sample %u: the raw page is torn or failed: %s\n", samples, path);
    }
  }
  const int64_t endt = now_us();
  if (last_raw) {
    char path[512]; snprintf(path, sizeof path, "%s.last.raw", out);
    if (!dump_page(path, fd, map, use_pread, line, page, &pagebuf)) fprintf(stderr, "fbstream: the last page is torn or failed: %s\n", path);
  }
  struct rusage ru; getrusage(RUSAGE_SELF, &ru);
  const uint32_t cpu_ms = (uint32_t)(ru.ru_utime.tv_sec * 1000 + ru.ru_utime.tv_usec / 1000 + ru.ru_stime.tv_sec * 1000 + ru.ru_stime.tv_usec / 1000);
  w32(f, 3); w32(f, 24); w32(f, samples); w32(f, flips); w32(f, polls); w32(f, cpu_ms); w64(f, endt / 1000);
  if (ferror(f) || fclose(f) != 0) { fprintf(stderr, "fbstream: FATAL: writing %s failed (%s): the stream is short — is the file system full?\n", out, strerror(errno)); return 1; }
  if (trace) {   /* the finger's scan at each sample's time, written into the records in place */
    read_trace(trace);
    f = fopen(out, "r+b");
    if (f && g_ntrace) {
      fseek(f, 4 + 13 * 4 + 8 + 8, SEEK_SET);
      for (;;) {
        uint32_t type, bytes; if (fread(&type, 4, 1, f) != 1 || fread(&bytes, 4, 1, f) != 1) break;
        const long at = ftell(f);
        if (type == 1) {
          uint64_t t; if (fread(&t, 8, 1, f) != 1) break;
          const Scan* s = scan_at((int64_t)t);
          if (s) { fseek(f, at + 8 + 5 * 4, SEEK_SET); wi32(f, s->x); wi32(f, s->y); wi32(f, s->phase); }
        }
        fseek(f, at + bytes, SEEK_SET);
      }
    }
    if (f) fclose(f);
  }
  const double secs = (endt - start) / 1e6;
  printf("fbstream: %u samples in %.2f s (%.1f/s, asked %d), read median~%.1f ms max %.1f ms (%s, stride %d: %d rows of %u), "
         "%u flips (frames shown, %.1f/s), %u polls (%.0f/s), cpu %u ms (%.1f%% of a core)%s%s\n",
         samples, secs, samples / secs, hz, samples ? read_sum / 1000.0 / samples : 0.0, read_max / 1000.0,
         use_pread ? "pread" : "mmap", stride, cols, vi.yres, flips, flips / secs, polls, polls / secs, cpu_ms,
         100.0 * cpu_ms / 1000.0 / secs, trace ? " finger joined from " : "", trace ? trace : "");
  if (map) munmap((void*)map, whole);
  close(fd);
  return 0;
}
