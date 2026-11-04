/* Copyright 2026 NotAlexNoyle. Apache-2.0.
 *
 * threadstat: the daemon's threads sampled while a round runs (T-567, 2026-11-03), on the device.
 *
 *   threadstat <pid> <out> [-s SECONDS] [-p POLL_MS]
 *
 * Every POLL_MS (50) it writes one line to <out>:
 *   t=<ms since the epoch, gettimeofday: the daemon's [jihad-tl] clock> irq=<kgsl interrupts on cpu0>,<on cpu1>
 *   cpu0=<user>,<system>,<idle> cpu1=... (ticks, /proc/stat) | <tid>:<comm>:<state>:<cpu>:<utime+stime> ...
 * for every task of the pid (the comm with spaces kept, colons replaced). The state is the task's at the
 * instant (R running, S sleeping, D waiting in the kernel), the cpu the one it last ran on, the ticks
 * (10 ms) its CPU time so far. Reads the kgsl line of /proc/interrupts (the GPU's interrupt) so the
 * GPU's activity sits beside the threads. GLIBC_2.4 symbols only (the device's libc is 2.8): no
 * clock_gettime, no sscanf.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>

static long long now_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/* the kgsl interrupt counts per cpu from /proc/interrupts (the first line naming kgsl) */
static void irq_counts(long* c0, long* c1) {
    *c0 = *c1 = -1;
    FILE* f = fopen("/proc/interrupts", "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof line, f)) {
        if (!strstr(line, "kgsl")) continue;
        char* p = strchr(line, ':');
        if (!p) break;
        p++;
        *c0 = strtol(p, &p, 10);
        *c1 = strtol(p, &p, 10);
        break;
    }
    fclose(f);
}

/* cpuN's user, system and idle ticks from /proc/stat */
static void cpu_ticks(int cpu, long* u, long* s, long* i) {
    *u = *s = *i = -1;
    FILE* f = fopen("/proc/stat", "r");
    if (!f) return;
    char line[512], key[8];
    snprintf(key, sizeof key, "cpu%d ", cpu);
    while (fgets(line, sizeof line, f)) {
        if (strncmp(line, key, strlen(key))) continue;
        char* p = line + strlen(key);
        long user = strtol(p, &p, 10), nice = strtol(p, &p, 10), sys = strtol(p, &p, 10), idle = strtol(p, &p, 10);
        *u = user + nice; *s = sys; *i = idle;
        break;
    }
    fclose(f);
}

int main(int argc, char** argv) {
    if (argc < 3) { fprintf(stderr, "usage: threadstat <pid> <out> [-s SECONDS] [-p POLL_MS]\n"); return 2; }
    const long pid = strtol(argv[1], NULL, 10);
    const char* outPath = argv[2];
    long seconds = 30, pollMs = 50;
    for (int a = 3; a + 1 < argc; a += 2) {
        if (!strcmp(argv[a], "-s")) seconds = strtol(argv[a + 1], NULL, 10);
        else if (!strcmp(argv[a], "-p")) pollMs = strtol(argv[a + 1], NULL, 10);
    }
    FILE* out = fopen(outPath, "w");
    if (!out) { perror(outPath); return 1; }
    char taskDir[64];
    snprintf(taskDir, sizeof taskDir, "/proc/%ld/task", pid);
    const long long end = now_ms() + seconds * 1000;
    long samples = 0;
    while (now_ms() < end) {
        long c0, c1, u0, s0, i0, u1, s1, i1;
        irq_counts(&c0, &c1);
        cpu_ticks(0, &u0, &s0, &i0);
        cpu_ticks(1, &u1, &s1, &i1);
        fprintf(out, "t=%lld irq=%ld,%ld cpu0=%ld,%ld,%ld cpu1=%ld,%ld,%ld |", now_ms(), c0, c1, u0, s0, i0, u1, s1, i1);
        DIR* d = opendir(taskDir);
        if (!d) { fprintf(out, " gone\n"); break; }
        struct dirent* e;
        while ((e = readdir(d))) {
            if (e->d_name[0] < '0' || e->d_name[0] > '9') continue;
            char path[128], line[1024];
            snprintf(path, sizeof path, "%s/%s/stat", taskDir, e->d_name);
            FILE* f = fopen(path, "r");
            if (!f) continue;
            if (!fgets(line, sizeof line, f)) { fclose(f); continue; }
            fclose(f);
            /* pid (comm) state ppid ... : the comm may hold spaces, so split after its last ')' */
            char* open = strchr(line, '(');
            char* close = strrchr(line, ')');
            if (!open || !close || close < open) continue;
            *close = 0;
            char comm[32];
            snprintf(comm, sizeof comm, "%s", open + 1);
            for (char* q = comm; *q; ++q) if (*q == ':' || *q == ' ' || *q == '|') *q = '_';
            char* p = close + 1;
            /* fields from 3 on: state, then numbers; utime is field 14, stime 15, processor 39 */
            while (*p == ' ') p++;
            const char state = *p ? *p : '?';
            long fields[40];
            int n = 3;
            p++;
            while (n < 40 && *p) {
                char* q;
                long v = strtol(p, &q, 10);
                if (q == p) { while (*p && *p != ' ') p++; while (*p == ' ') p++; fields[++n] = 0; continue; }
                fields[++n] = v;
                p = q;
                while (*p == ' ') p++;
            }
            const long ticks = (n >= 15 ? fields[14] + fields[15] : -1);
            const long cpu = (n >= 39 ? fields[39] : -1);
            fprintf(out, " %s:%s:%c:%ld:%ld", e->d_name, comm, state, cpu, ticks);
        }
        closedir(d);
        fputc('\n', out);
        samples++;
        usleep((useconds_t)pollMs * 1000);
    }
    fclose(out);
    fprintf(stderr, "threadstat: %ld samples to %s\n", samples, outPath);
    return 0;
}
