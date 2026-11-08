/* Copyright 2026 NotAlexNoyle. Apache-2.0.
 *
 * stallwatch — a real-time witness of a device stall (cavekit-device-headroom.md R1, T-591). On 2026-11-07 and
 * 2026-11-08 the TouchPad's whole user space got no CPU for about three minutes while a YouTube card started its
 * next video beside a Flash game: syslog, novacomd (nice -4) and the window server all stopped, the kernel's own
 * lines kept their timestamps, and nothing in the logs says why. This runs as SCHED_FIFO (above every ordinary
 * process and above pulseaudio's round-robin 5) with its pages locked, and once a second writes to a tmpfs file:
 * how late its own wake-up was (a late witness means the kernel itself stalled), the load, the CPU split per core
 * (user nice system idle iowait irq softirq) over the second, swap-ins, major faults, allocation and compaction
 * stalls, free memory, the I2C error interrupt's count (this device's light sensor is dead and its controller
 * interrupts 240 times a second), the block devices' sectors read and written over the second (/proc/diskstats:
 * the eMMC whole and each dm-* volume: 2026-11-08's stall was the eMMC's queue, every page fault waiting on it),
 * and for the pids named on the command line their state, their CPU over the
 * second and, when they sleep, the kernel function they wait in (wchan); every other second the number of tasks in
 * D state system-wide with the names and wchans of the first few. Reads of /proc only; no I/O that can block.
 *
 *   stallwatch OUT SECONDS [pid ...]        (SIGTERM ends it early, the file complete)
 *
 * Build with the repository's toolchain beside finger.c (tools/device/finger/README.md); it links only symbols the
 * device's glibc 2.8 has (gettimeofday and nanosleep, not clock_gettime).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sched.h>
#include <dirent.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>

static volatile int gStop = 0;
static void onTerm(int s) { (void)s; gStop = 1; }

static long long nowMs(void) { struct timeval tv; gettimeofday(&tv, NULL); return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000; }

static int readFile(const char* path, char* buf, int cap) {
  int fd = open(path, O_RDONLY); if (fd < 0) return -1;
  int n = 0, r;
  while (n < cap - 1 && (r = read(fd, buf + n, cap - 1 - n)) > 0) n += r;
  close(fd); buf[n] = 0; return n;
}

static long long vmstatValue(const char* vm, const char* key) {
  const char* p = vm; size_t kl = strlen(key);
  while ((p = strstr(p, key)) != NULL) {
    if ((p == vm || p[-1] == '\n') && p[kl] == ' ') return atoll(p + kl + 1);
    p += kl;
  }
  return -1;
}

static long long meminfoKb(const char* mi, const char* key) {
  const char* p = strstr(mi, key); if (!p) return -1;
  return atoll(p + strlen(key));
}

typedef struct { long long v[8]; } CpuStat;
static int cpuLine(const char* stat, const char* name, CpuStat* out) {   /* name: "cpu ", "cpu0 " */
  const char* p = stat;
  while ((p = strstr(p, name)) != NULL) {
    if (p == stat || p[-1] == '\n') {
      const char* q = p + strlen(name);
      for (int i = 0; i < 8; i++) { out->v[i] = strtoll(q, (char**)&q, 10); }
      return 1;
    }
    p += strlen(name);
  }
  return 0;
}

/* /proc/diskstats: for each device named like mmcblk0 (whole) or dm-N, sectors read and written; "name:r/w" deltas */
typedef struct { char name[16]; long long rd, wr; } DiskStat;
static int diskStats(DiskStat* out, int cap) {
  static char buf[16384]; int n = 0;
  if (readFile("/proc/diskstats", buf, sizeof buf) <= 0) return 0;
  const char* p = buf;
  while (*p && n < cap) {
    const char* e = strchr(p, '\n'); if (!e) e = p + strlen(p);
    char name[16] = ""; long long f[11] = {0};
    int got = sscanf(p, " %*d %*d %15s %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld", name, &f[0], &f[1], &f[2], &f[3], &f[4], &f[5], &f[6], &f[7], &f[8], &f[9], &f[10]);
    if (got >= 8 && (strcmp(name, "mmcblk0") == 0 || strncmp(name, "dm-", 3) == 0)) { strcpy(out[n].name, name); out[n].rd = f[2]; out[n].wr = f[6]; n++; }
    p = *e ? e + 1 : e;
  }
  return n;
}

static long long irqCount(const char* irqs, const char* num) {   /* " 183:" */
  const char* p = strstr(irqs, num); if (!p) return -1;
  p += strlen(num); long long a = strtoll(p, (char**)&p, 10); long long b = strtoll(p, (char**)&p, 10);
  return a + b;
}

/* a pid's state, its utime+stime and, when sleeping, its wchan */
static void pidLine(int pid, long long* lastCpu, FILE* out) {
  char path[64], buf[1024], wchan[64] = "";
  snprintf(path, sizeof path, "/proc/%d/stat", pid);
  if (readFile(path, buf, sizeof buf) <= 0) { fprintf(out, " %d:gone", pid); return; }
  const char* rp = strrchr(buf, ')'); if (!rp) return;
  char st = ' '; long long ut = 0, stt = 0;
  /* after ')': state ppid pgrp session tty tpgid flags minflt cminflt majflt cmajflt utime stime */
  sscanf(rp + 1, " %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lld %lld", &st, &ut, &stt);
  long long cpu = ut + stt, d = *lastCpu >= 0 ? cpu - *lastCpu : 0; *lastCpu = cpu;
  if (st != 'R') { snprintf(path, sizeof path, "/proc/%d/wchan", pid); if (readFile(path, wchan, sizeof wchan) > 0) wchan[strcspn(wchan, "\n")] = 0; }
  fprintf(out, " %d:%c/%lld%s%s", pid, st, d, wchan[0] ? "/" : "", wchan);
}

/* tasks in D state across the system: the count and the first few names with their wchan */
static void dStateScan(FILE* out) {
  DIR* d = opendir("/proc"); if (!d) return;
  int n = 0, shown = 0; char path[96], buf[512], w[64];
  struct dirent* e;
  while ((e = readdir(d)) != NULL) {
    if (e->d_name[0] < '1' || e->d_name[0] > '9') continue;
    snprintf(path, sizeof path, "/proc/%s/task", e->d_name);
    DIR* td = opendir(path); if (!td) continue;
    struct dirent* t;
    while ((t = readdir(td)) != NULL) {
      if (t->d_name[0] < '1' || t->d_name[0] > '9') continue;
      snprintf(path, sizeof path, "/proc/%s/task/%s/stat", e->d_name, t->d_name);
      if (readFile(path, buf, sizeof buf) <= 0) continue;
      const char* rp = strrchr(buf, ')'); if (!rp || rp[2] != 'D') continue;
      n++;
      if (shown < 6) {
        const char* lp = strchr(buf, '('); char name[32] = ""; if (lp && rp > lp) { int l = rp - lp - 1; if (l > 31) l = 31; memcpy(name, lp + 1, l); name[l] = 0; }
        snprintf(path, sizeof path, "/proc/%s/task/%s/wchan", e->d_name, t->d_name); w[0] = 0;
        if (readFile(path, w, sizeof w) > 0) w[strcspn(w, "\n")] = 0;
        fprintf(out, " D:%s/%s(%s)", t->d_name, name, w); shown++;
      }
    }
    closedir(td);
  }
  closedir(d);
  fprintf(out, " dstate=%d", n);
}

int main(int argc, char** argv) {
  if (argc < 3) { fprintf(stderr, "usage: stallwatch OUT SECONDS [pid ...]\n"); return 2; }
  FILE* out = fopen(argv[1], "w"); if (!out) { perror(argv[1]); return 1; }
  int secs = atoi(argv[2]); int npid = argc - 3; int pids[32]; long long lastCpu[32];
  if (npid > 32) npid = 32;
  for (int i = 0; i < npid; i++) { pids[i] = atoi(argv[3 + i]); lastCpu[i] = -1; }
  signal(SIGTERM, onTerm); signal(SIGINT, onTerm);
  struct sched_param sp; memset(&sp, 0, sizeof sp); sp.sched_priority = 60;
  int rt = sched_setscheduler(0, SCHED_FIFO, &sp);
  int ml = mlockall(MCL_CURRENT | MCL_FUTURE);
  static char stat[4096], vm[8192], mi[4096], irqs[16384];
  CpuStat all0, c00, c10, all, c0, c1; long long swin0 = -1, swout0 = -1, majf0 = -1, alloc0 = -1, comp0 = -1, irq0 = -1;
  DiskStat ds0[12], ds[12]; int nds0 = diskStats(ds0, 12);
  fprintf(out, "# stallwatch start %lld ms rt=%s mlock=%s pids=%d; a line a second: lateMs load cpu(all/0/1: usr nice sys idle iow irq sirq, jiffies) swapin swapout majfault allocstall compactstall freeKb i2cirq disk=name:sectorsRead/Written... | pid:state/cpu/wchan ...\n",
          nowMs(), rt == 0 ? "FIFO60" : strerror(errno), ml == 0 ? "ok" : strerror(errno), npid);
  fflush(out);
  long long t0 = nowMs(), next = t0;
  for (int i = 0; i < secs && !gStop; i++) {
    next += 1000;
    long long late = -1;
    for (;;) { long long n = nowMs(); if (n >= next) { late = n - next; break; } struct timespec ts = { 0, (long)((next - n) * 1000000L) }; if (ts.tv_nsec > 999000000L) ts.tv_nsec = 999000000L; nanosleep(&ts, NULL); if (gStop) break; }
    if (gStop) break;
    readFile("/proc/stat", stat, sizeof stat); readFile("/proc/vmstat", vm, sizeof vm); readFile("/proc/meminfo", mi, sizeof mi); readFile("/proc/interrupts", irqs, sizeof irqs);
    cpuLine(stat, "cpu ", &all); cpuLine(stat, "cpu0 ", &c0); cpuLine(stat, "cpu1 ", &c1);
    long long swin = vmstatValue(vm, "pswpin"), swout = vmstatValue(vm, "pswpout"), majf = vmstatValue(vm, "pgmajfault"), alloc = vmstatValue(vm, "allocstall"), comp = vmstatValue(vm, "compact_stall"), irq = irqCount(irqs, " 183:");
    char loadavg[64] = ""; readFile("/proc/loadavg", loadavg, sizeof loadavg); loadavg[strcspn(loadavg, "\n")] = 0;
    fprintf(out, "t=%lld late=%lld load=%s", (nowMs() - t0) / 1000, late, loadavg);
    if (i > 0) {
      fprintf(out, " cpu=");
      for (int k = 0; k < 7; k++) fprintf(out, "%lld%s", all.v[k] - all0.v[k], k < 6 ? "," : "");
      fprintf(out, "/"); for (int k = 0; k < 7; k++) fprintf(out, "%lld%s", c0.v[k] - c00.v[k], k < 6 ? "," : "");
      fprintf(out, "/"); for (int k = 0; k < 7; k++) fprintf(out, "%lld%s", c1.v[k] - c10.v[k], k < 6 ? "," : "");
      fprintf(out, " swin=%lld swout=%lld majf=%lld alloc=%lld comp=%lld", swin - swin0, swout - swout0, majf - majf0, alloc - alloc0, comp - comp0);
      fprintf(out, " free=%lld i2c=%lld", meminfoKb(mi, "MemFree:"), irq - irq0);
      int nds = diskStats(ds, 12); fprintf(out, " disk=");
      for (int k = 0; k < nds; k++) { long long r0 = 0, w0 = 0; for (int j = 0; j < nds0; j++) if (strcmp(ds0[j].name, ds[k].name) == 0) { r0 = ds0[j].rd; w0 = ds0[j].wr; } fprintf(out, "%s%s:%lld/%lld", k ? "," : "", ds[k].name, ds[k].rd - r0, ds[k].wr - w0); }
      memcpy(ds0, ds, sizeof ds); nds0 = nds;
    }
    all0 = all; c00 = c0; c10 = c1; swin0 = swin; swout0 = swout; majf0 = majf; alloc0 = alloc; comp0 = comp; irq0 = irq;
    fprintf(out, " |"); for (int k = 0; k < npid; k++) pidLine(pids[k], &lastCpu[k], out);
    if (i % 2 == 0) dStateScan(out);
    fprintf(out, "\n"); fflush(out);
  }
  fprintf(out, "# stallwatch end %lld ms\n", nowMs()); fclose(out);
  return 0;
}
