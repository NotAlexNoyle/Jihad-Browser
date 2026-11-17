// Copyright 2026 NotAlexNoyle. Apache-2.0.
// cputop.c -- every thread's CPU over a window, the busiest first, and the device's idle: where a frame's time goes
// (cavekit-flash.md R3). One small process reading /proc twice: the shell version (thread-top.sh) took three seconds
// and most of a core for each reading on a busy device, and so measured itself.
//   cputop SECONDS [N]    lines "CPU% (system S%) tid TID pid PID nice NICE NAME", then the device's busy share and the window
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>

typedef struct { int tid, pid, nice; unsigned long t, st; char name[24]; } T;
static T a[8192], b[8192];

static int snap(T* v) {
  int n = 0; DIR* d = opendir("/proc"); struct dirent* e;
  while (d && (e = readdir(d))) {
    if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
    char p[96]; snprintf(p, sizeof p, "/proc/%s/task", e->d_name);
    DIR* k = opendir(p); struct dirent* f;
    while (k && (f = readdir(k))) {
      if (f->d_name[0] < '0' || f->d_name[0] > '9' || n >= 8192) continue;
      snprintf(p, sizeof p, "/proc/%s/task/%s/stat", e->d_name, f->d_name);
      FILE* s = fopen(p, "r"); if (!s) continue;
      char l[1024]; size_t m = fread(l, 1, sizeof l - 1, s); fclose(s); l[m] = 0;
      char* o = strchr(l, '('); char* c = strrchr(l, ')'); if (!o || !c) continue;
      unsigned long ut = 0, st = 0; long ni = 0;
      sscanf(c + 2, "%*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %ld", &ut, &st, &ni);
      v[n].tid = atoi(f->d_name); v[n].pid = atoi(e->d_name); v[n].t = ut + st; v[n].st = st; v[n].nice = (int)ni;
      size_t w = (size_t)(c - o - 1); if (w > 23) w = 23; memcpy(v[n].name, o + 1, w); v[n].name[w] = 0; n++;
    }
    if (k) closedir(k);
  }
  if (d) closedir(d);
  return n;
}
static unsigned long kind[2][7];     // user nice system idle iowait irq softirq, at the two readings
static void cpu(unsigned long* idle, unsigned long* all) {
  static int k = 0; unsigned long* v = kind[k++ & 1];
  FILE* s = fopen("/proc/stat", "r");
  if (s) { if (fscanf(s, "cpu %lu %lu %lu %lu %lu %lu %lu", v, v + 1, v + 2, v + 3, v + 4, v + 5, v + 6) < 4) v[3] = 0; fclose(s); }
  *idle = v[3] + v[4]; *all = v[0] + v[1] + v[2] + v[3] + v[4] + v[5] + v[6];
}
// forks and clones, and context switches, since boot: a process or a thread that lived and died inside the window is in no reading
static void counts(unsigned long* forks, unsigned long* ctxt) {
  FILE* s = fopen("/proc/stat", "r"); char l[512]; *forks = *ctxt = 0;
  while (s && fgets(l, sizeof l, s)) { sscanf(l, "processes %lu", forks); sscanf(l, "ctxt %lu", ctxt); }
  if (s) fclose(s);
}
// each core's idle and total, and how long it was online: a core the governor takes away is not a core
// (a core that is offline has no line: its total reads 0 and the report says so, never a share since boot)
static void cores(unsigned long idle[2], unsigned long all[2]) {
  FILE* s = fopen("/proc/stat", "r"); char l[512]; idle[0] = idle[1] = all[0] = all[1] = 0;
  while (s && fgets(l, sizeof l, s)) {
    int c; unsigned long v[7] = {0};
    if (sscanf(l, "cpu%d %lu %lu %lu %lu %lu %lu %lu", &c, v, v + 1, v + 2, v + 3, v + 4, v + 5, v + 6) >= 5 && c >= 0 && c < 2) { idle[c] = v[3] + v[4]; all[c] = v[0] + v[1] + v[2] + v[3] + v[4] + v[5] + v[6]; }
  }
  if (s) fclose(s);
}
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
static int by(const void* x, const void* y) { long d = (long)((const T*)y)->t - (long)((const T*)x)->t; return d > 0 ? 1 : d < 0 ? -1 : 0; }

int main(int argc, char** argv) {
  double w = argc > 1 ? atof(argv[1]) : 5; int top = argc > 2 ? atoi(argv[2]) : 16;
  unsigned long i0, t0, i1, t1, f0, c0, f1, c1; unsigned long ci0[2], ca0[2], ci1[2], ca1[2]; cores(ci0, ca0); counts(&f0, &c0); cpu(&i0, &t0); double s0 = now(); int na = snap(a); double s1 = now();
  usleep((useconds_t)(w * 1e6));
  double e0 = now(); int nb = snap(b); double e1 = now(); cpu(&i1, &t1);
  double win = (e0 + e1) / 2 - (s0 + s1) / 2; long hz = sysconf(_SC_CLK_TCK);
  for (int j = 0; j < nb; j++) {
    unsigned long before = 0, sbefore = 0; int found = 0;
    for (int i = 0; i < na; i++) if (a[i].tid == b[j].tid) { before = a[i].t; sbefore = a[i].st; found = 1; break; }
    b[j].t = found && b[j].t >= before ? b[j].t - before : (found ? 0 : b[j].t);
    b[j].st = found && b[j].st >= sbefore ? b[j].st - sbefore : (found ? 0 : b[j].st);
  }
  qsort(b, nb, sizeof b[0], by);
  for (int j = 0; j < nb && j < top && b[j].t; j++) printf("%6.1f%% (system %.1f%%) tid %d pid %d nice %d %s\n", 100.0 * b[j].t / hz / win, 100.0 * b[j].st / hz / win, b[j].tid, b[j].pid, b[j].nice, b[j].name);
  printf("device: busy %.0f%% of 200%%, the window %.2f s, the readings %.0f and %.0f ms\n", t1 > t0 ? 200.0 * ((t1 - t0) - (i1 - i0)) / (t1 - t0) : 0, win, (s1 - s0) * 1e3, (e1 - e0) * 1e3);
  static const char* names[7] = { "user", "nice", "system", "idle", "iowait", "irq", "softirq" };
  printf("device:");
  for (int i = 0; i < 7; i++) printf(" %s %.0f%%", names[i], t1 > t0 ? 200.0 * (kind[1][i] - kind[0][i]) / (t1 - t0) : 0);
  counts(&f1, &c1); cores(ci1, ca1);
  for (int c = 0; c < 2; c++) {
    if (!ca0[c] || !ca1[c]) { printf(", core %d offline at a reading", c); continue; }
    printf(", core %d online %.0f%% of the window and busy %.0f%% of that", c, 100.0 * (ca1[c] - ca0[c]) / hz / win, ca1[c] > ca0[c] ? 100.0 * ((ca1[c] - ca0[c]) - (ci1[c] - ci0[c])) / (ca1[c] - ca0[c]) : 0);
  }
  printf(", new processes and threads %.0f/s, context switches %.0f/s\n", (f1 - f0) / win, (c1 - c0) / win);
  return 0;
}
