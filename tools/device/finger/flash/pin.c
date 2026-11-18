// Copyright 2026 NotAlexNoyle. Apache-2.0.
//
// pin.c -- an experiment's tool (T-576, the player's share of a core): the cores a process's threads may run on.
//   pin MASK PID [main]     every thread of PID (or its main thread alone) held to the cores of MASK (1 = cpu0, 2 = cpu1, 3 = both)
// Built static by game-round.py with the device toolchain, run on the device.
#define _GNU_SOURCE
#include <dirent.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
  if (argc < 3) { fprintf(stderr, "pin MASK PID [main]\n"); return 2; }
  unsigned long mask = strtoul(argv[1], 0, 0); int pid = atoi(argv[2]), n = 0, bad = 0;
  cpu_set_t set; CPU_ZERO(&set);
  for (int c = 0; c < 8; c++) if (mask & (1ul << c)) CPU_SET(c, &set);
  if (argc > 3 && !strcmp(argv[3], "main")) { if (sched_setaffinity(pid, sizeof set, &set)) { perror("sched_setaffinity"); return 1; } printf("pinned %d main to %lu\n", pid, mask); return 0; }
  char p[64]; snprintf(p, sizeof p, "/proc/%d/task", pid);
  DIR* d = opendir(p); if (!d) { perror(p); return 1; }
  for (struct dirent* e; (e = readdir(d));) { int t = atoi(e->d_name); if (t > 0) { if (sched_setaffinity(t, sizeof set, &set)) bad++; else n++; } }
  closedir(d); printf("pinned %d: %d threads to %lu, %d refused\n", pid, n, mask, bad); return 0;
}
