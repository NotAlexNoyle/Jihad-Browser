// pcsample.c -- where a thread's time goes: its saved program counter from /proc, sampled, with the
// libraries it falls in. A running thread's value is as new as its last kernel entry, so this is a
// profile at the timer's rate, biased a little to system calls: it needs no symbols on the device and
// costs the thread nothing.
//
//   pcsample TID SECONDS [PERIOD_US]   prints: samples, then "COUNT PC-OFFSET LIBRARY" lines by 4 KB-free exact pc
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

typedef struct { unsigned long pc; unsigned n; } Slot;
enum { MAX = 65536 };
static Slot tab[MAX];

static void add(unsigned long pc) {
  unsigned h = (pc * 2654435761u) >> 16;
  for (;;) {
    if (!tab[h].n) { tab[h].pc = pc; tab[h].n = 1; return; }
    if (tab[h].pc == pc) { tab[h].n++; return; }
    h = (h + 1) & (MAX - 1);
  }
}
typedef struct { unsigned long lo, hi, off; char name[120]; } Map;
static Map maps[2048]; static int nmaps;

static void read_maps(int pid) {
  char p[64]; snprintf(p, sizeof p, "/proc/%d/maps", pid);
  FILE* f = fopen(p, "r"); if (!f) return;
  char line[512];
  while (fgets(line, sizeof line, f) && nmaps < 2048) {
    unsigned long lo, hi, off; char perm[8], dev[16]; unsigned long ino; char name[256] = "";
    if (sscanf(line, "%lx-%lx %7s %lx %15s %lu %255s", &lo, &hi, perm, &off, dev, &ino, name) >= 6 && perm[2] == 'x') {
      Map* m = &maps[nmaps++]; m->lo = lo; m->hi = hi; m->off = off;
      char* b = strrchr(name, '/'); snprintf(m->name, sizeof m->name, "%s", b ? b + 1 : name);
    }
  }
  fclose(f);
}
static int cmp(const void* a, const void* b) { return (int)((const Slot*)b)->n - (int)((const Slot*)a)->n; }

int main(int argc, char** argv) {
  if (argc < 3) { fprintf(stderr, "pcsample TID SECONDS [PERIOD_US]\n"); return 2; }
  int tid = atoi(argv[1]); double secs = atof(argv[2]); int period = argc > 3 ? atoi(argv[3]) : 2000;
  char p[64]; snprintf(p, sizeof p, "/proc/%d/stat", tid);
  read_maps(tid);
  struct timespec t0, t; clock_gettime(CLOCK_MONOTONIC, &t0);
  unsigned total = 0, running = 0;
  for (;;) {
    clock_gettime(CLOCK_MONOTONIC, &t);
    if ((t.tv_sec - t0.tv_sec) + (t.tv_nsec - t0.tv_nsec) / 1e9 >= secs) break;
    int fd = open(p, O_RDONLY); if (fd < 0) break;
    char b[1024]; int n = read(fd, b, sizeof b - 1); close(fd); if (n <= 0) break; b[n] = 0;
    char* q = strrchr(b, ')'); if (!q) continue; q += 2;
    char state = *q; int field = 3; char* s = q;    // state is field 3; kstkeip is field 30
    while (field < 30 && (s = strchr(s, ' '))) { s++; field++; }
    total++;
    if (state == 'R' && s) { running++; add(strtoul(s, NULL, 10)); }
    usleep(period);
  }
  qsort(tab, MAX, sizeof tab[0], cmp);
  printf("samples %u, running %u (%.0f%%)\n", total, running, total ? 100.0 * running / total : 0);
  for (int i = 0; i < MAX && tab[i].n; i++) {
    const char* name = "?"; unsigned long off = tab[i].pc;
    for (int j = 0; j < nmaps; j++) if (tab[i].pc >= maps[j].lo && tab[i].pc < maps[j].hi) { name = maps[j].name; off = tab[i].pc - maps[j].lo + maps[j].off; break; }
    printf("%u %lx %s\n", tab[i].n, off, name);
  }
  return 0;
}
