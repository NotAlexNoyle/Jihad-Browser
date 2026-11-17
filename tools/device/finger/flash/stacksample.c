// Copyright 2026 NotAlexNoyle. Apache-2.0.
// stacksample.c -- where a thread's time goes, with who called: the thread is stopped a moment (ptrace), its program
// counter, link register and the top of its stack read, and let go; the stack's words that point into code are the
// return addresses, more or less (a stale word reads as one too: this is a census, not an unwinder). It needs no
// symbols on the device. For each sample the libraries seen are counted once (a library's inclusive share), the
// program counter's library is the exclusive share, and the commonest code addresses of each library but the
// busiest are listed for a symbol table on the host.
//   stacksample TID SECONDS [PERIOD_US=20000] [WORDS=192]
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

typedef struct { unsigned long lo, hi, off; char name[64]; int x; } Map;
static Map maps[4096]; static int nmaps;
typedef struct { unsigned long a; unsigned n; int lib; } Slot;
enum { MAX = 1 << 16 };
static Slot tab[MAX];
static char libs[256][64]; static unsigned incl[256], excl[256]; static int nlibs;

static int lib_of(const char* n) {
  for (int i = 0; i < nlibs; i++) if (!strcmp(libs[i], n)) return i;
  if (nlibs >= 256) return 0;
  strncpy(libs[nlibs], n, 63); return nlibs++;
}
static void read_maps(int pid) {
  char p[64], l[512]; snprintf(p, sizeof p, "/proc/%d/maps", pid); FILE* f = fopen(p, "r"); nmaps = 0;
  while (f && fgets(l, sizeof l, f) && nmaps < 4096) {
    unsigned long lo, hi, off; char perm[8], path[256] = "";
    if (sscanf(l, "%lx-%lx %7s %lx %*s %*s %255s", &lo, &hi, perm, &off, path) < 4 || perm[2] != 'x') continue;
    const char* b = strrchr(path, '/'); b = b ? b + 1 : (path[0] ? path : "[anon-code]");
    maps[nmaps].lo = lo; maps[nmaps].hi = hi; maps[nmaps].off = off; strncpy(maps[nmaps].name, b, 63); nmaps++;
  }
  if (f) fclose(f);
}
static int find(unsigned long a) { for (int i = 0; i < nmaps; i++) if (a >= maps[i].lo && a < maps[i].hi) return i; return -1; }
static void add(unsigned long off, int lib) {
  unsigned h = ((off ^ (unsigned)lib * 40503u) * 2654435761u) >> 16;
  for (;;) { if (!tab[h].n) { tab[h].a = off; tab[h].lib = lib; tab[h].n = 1; return; } if (tab[h].a == off && tab[h].lib == lib) { tab[h].n++; return; } h = (h + 1) & (MAX - 1); }
}
static int cmp(const void* x, const void* y) { return (int)((const Slot*)y)->n - (int)((const Slot*)x)->n; }
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }

int main(int argc, char** argv) {
  if (argc < 3) { fprintf(stderr, "stacksample TID SECONDS [PERIOD_US] [WORDS]\n"); return 2; }
  int tid = atoi(argv[1]); double secs = atof(argv[2]); int period = argc > 3 ? atoi(argv[3]) : 20000; int words = argc > 4 ? atoi(argv[4]) : 192;
  read_maps(tid);
  unsigned samples = 0; double t0 = now(), stopped = 0;
  while (now() - t0 < secs) {
    double a = now();
    if (ptrace(PTRACE_ATTACH, tid, 0, 0) < 0) { fprintf(stderr, "attach: %s\n", strerror(errno)); break; }
    int st; if (waitpid(tid, &st, __WALL) < 0) { fprintf(stderr, "wait: %s\n", strerror(errno)); break; }
    struct user_regs r;
    if (ptrace(PTRACE_GETREGS, tid, 0, &r) == 0) {
      unsigned long pc = r.uregs[15], lr = r.uregs[14], sp = r.uregs[13]; unsigned char seen[256] = {0};
      int m = find(pc); samples++;
      if (m >= 0) { int l = lib_of(maps[m].name); excl[l]++; seen[l] = 1; } else excl[lib_of("?")]++;
      m = find(lr); if (m >= 0) { int l = lib_of(maps[m].name); seen[l] = 1; add(lr - maps[m].lo + maps[m].off, l); }
      for (int i = 0; i < words; i++) {
        errno = 0; unsigned long w = (unsigned long)ptrace(PTRACE_PEEKDATA, tid, (void*)(sp + 4 * i), 0); if (errno) break;
        m = find(w); if (m < 0) continue;
        int l = lib_of(maps[m].name); seen[l] = 1; add(w - maps[m].lo + maps[m].off, l);
      }
      for (int i = 0; i < nlibs; i++) incl[i] += seen[i];
    }
    ptrace(PTRACE_DETACH, tid, 0, 0); stopped += now() - a;
    usleep(period);
  }
  printf("samples %u, the thread held %.0f ms of %.0f\n", samples, stopped * 1e3, (now() - t0) * 1e3);
  for (int i = 0; i < nlibs; i++) printf("LIB %s pc %u (%.0f%%) seen %u (%.0f%%)\n", libs[i], excl[i], samples ? 100.0 * excl[i] / samples : 0, incl[i], samples ? 100.0 * incl[i] / samples : 0);
  qsort(tab, MAX, sizeof tab[0], cmp);
  for (int i = 0, out = 0; i < MAX && tab[i].n && out < 400; i++) {
    if (!strncmp(libs[tab[i].lib], "libflashplayer", 14)) continue;
    printf("RET %u %lx %s\n", tab[i].n, tab[i].a, libs[tab[i].lib]); out++;
  }
  return 0;
}
