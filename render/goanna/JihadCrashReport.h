/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — self-reporting fatal-signal dump.
 *
 * WHY THIS EXISTS
 * The TouchPad has no debugger attached, no core dumps by default, and a daemon
 * that dies during engine bring-up leaves nothing behind but whatever reached the
 * log before it went. That cost several device round-trips of pure bisection, so
 * the daemon now reports its own death: the faulting address, PC/LR/SP, and the
 * load addresses of the mapped objects.
 *
 * Those two things together name the exact source line WITHOUT a debugger on the
 * device. The ARM libxul in the build tree
 * (build/webos-oe/out-arm/obj-jihad-goanna-arm/toolkit/library/libxul.so) is
 * ~1.1 GB and carries full .debug_info/.debug_line, so on the HOST:
 *
 *     arm-webos-linux-gnueabi-addr2line -f -C -i -e libxul.so <PC minus libxul base>
 *
 * turns a bare number into function + file:line. The /proc/self/maps dump is why
 * that works: libxul is position-independent, its load address changes per run,
 * and a PC without its base is meaningless.
 *
 * ── RE-ARMING IS MANDATORY, NOT BELT-AND-BRACES ─────────────────────────────
 * Installing this once at the top of main() DOES NOT WORK, and the failure is
 * silent. SpiderMonkey installs its own SIGSEGV handler during engine init (the
 * wasm/asm.js out-of-bounds trap) and replaces ours; a fault after that point is
 * handled by the engine, not by us. Measured: with a single install in main(), a
 * SIGSEGV delivered after engine init produced NO output at all.
 *
 * So JihadInstallCrashHandler() is called TWICE — once at process start, to cover
 * bring-up before the engine exists, and again after the engine is initialized so
 * we are the outermost handler for the window that actually matters. Calling it
 * again after any future engine-side init that might install handlers is correct
 * and cheap; it is idempotent in effect.
 *
 * TRADEOFF, deliberate: after the re-arm we own fatal signals, so a legitimate
 * SpiderMonkey wasm bounds-check trap would be reported as a crash instead of
 * being handled. This daemon runs no wasm (JIT-heavy content is not the target on
 * a 1 GB armv7 device), and knowing where the daemon dies is worth far more here
 * than preserving a trap path nothing uses.
 *
 * Header-only on purpose: the daemon is compiled by a dozen explicit per-file
 * build scripts plus the OE recipe, so a new .cpp would mean touching every one
 * of them (same reasoning as JihadRuntimePaths.h).
 */
#ifndef JIHAD_CRASH_REPORT_H
#define JIHAD_CRASH_REPORT_H

#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <ucontext.h>
#include <unwind.h>

namespace jihad {

// Everything below sticks to async-signal-safe primitives (write/open/read): no
// printf, no malloc, no locks.
inline void CrashPuts(const char* s) { if (s) { ssize_t r = write(2, s, strlen(s)); (void)r; } }

inline void CrashHex(unsigned long v) {
  char buf[2 + sizeof(unsigned long) * 2 + 1];
  static const char kDigits[] = "0123456789abcdef";
  buf[0] = '0'; buf[1] = 'x';
  for (unsigned i = 0; i < sizeof(unsigned long) * 2; i++)
    buf[2 + i] = kDigits[(v >> ((sizeof(unsigned long) * 2 - 1 - i) * 4)) & 0xf];
  buf[sizeof buf - 1] = '\0';
  CrashPuts(buf);
}

// Small decimal, for frame indices — a hex frame number is just noise in a log
// that a human has to read back out of a device console.
inline void CrashDec(unsigned v) {
  char buf[12];
  int i = (int)sizeof buf - 1;
  buf[i] = '\0';
  if (!v) buf[--i] = '0';
  while (v && i > 0) { buf[--i] = (char)('0' + (v % 10)); v /= 10; }
  CrashPuts(&buf[i]);
}

// Only the mappings that can host code we care about, so the report stays short
// enough to paste back out of a device log.
inline void CrashDumpMaps() {
  int fd = open("/proc/self/maps", O_RDONLY);
  if (fd < 0) return;
  static char buf[65536];
  ssize_t n = read(fd, buf, sizeof buf - 1);
  close(fd);
  if (n <= 0) return;
  buf[n] = '\0';
  for (char* line = buf; line && *line; ) {
    char* nl = strchr(line, '\n');
    if (nl) *nl = '\0';
    if (strstr(line, "libxul.so") || strstr(line, "jihad-browserserver") ||
        strstr(line, "ld-2.23.so") || strstr(line, "libmozsqlite3.so")) {
      CrashPuts("[jihad-bs]   "); CrashPuts(line); CrashPuts("\n");
    }
    line = nl ? nl + 1 : nullptr;
  }
}

struct CrashBtState { unsigned n; };

inline _Unwind_Reason_Code CrashBtFrame(struct _Unwind_Context* ctx, void* arg) {
  CrashBtState* st = (CrashBtState*)arg;
  _Unwind_Ptr ip = _Unwind_GetIP(ctx);
  if (ip) {
    CrashPuts("[jihad-bs]   #");
    CrashDec(st->n);
    CrashPuts(" ");
    CrashHex((unsigned long)ip);
    CrashPuts("\n");
  }
  return (++st->n >= 16) ? _URC_END_OF_STACK : _URC_NO_REASON;
}

inline void CrashBacktrace() {
  CrashBtState st = { 0 };
  _Unwind_Backtrace(CrashBtFrame, &st);
}

extern "C" inline void JihadFatalSignal(int sig, siginfo_t* si, void* ctx) {
  CrashPuts("\n[jihad-bs] *** FATAL SIGNAL sig=");
  CrashHex((unsigned long)sig);
  CrashPuts(" faultaddr=");
  CrashHex((unsigned long)(si ? si->si_addr : nullptr));
  CrashPuts("\n");

#if defined(__arm__)
  if (ctx) {
    const mcontext_t* mc = &((ucontext_t*)ctx)->uc_mcontext;
    CrashPuts("[jihad-bs] pc="); CrashHex(mc->arm_pc);
    CrashPuts(" lr=");           CrashHex(mc->arm_lr);
    CrashPuts(" sp=");           CrashHex(mc->arm_sp);
    CrashPuts("\n");
  }
#elif defined(__x86_64__)
  if (ctx) {
    const mcontext_t* mc = &((ucontext_t*)ctx)->uc_mcontext;
    CrashPuts("[jihad-bs] rip="); CrashHex((unsigned long)mc->gregs[16] /* REG_RIP */);
    CrashPuts("\n");
  }
#endif

  // Maps FIRST: without the load base the PC above cannot be resolved, so this is
  // the line that must survive even if anything after it faults again.
  CrashPuts("[jihad-bs] maps (subtract the libxul base from pc/lr for addr2line):\n");
  CrashDumpMaps();
  // Stack walk LAST: `lr` alone names only the immediate (often inlined) caller,
  // which is rarely the function that supplied the bad value. ARM carries
  // .ARM.exidx/.ARM.extab unwind tables, so _Unwind_Backtrace works here. Printed
  // after the maps because it is the step most likely to fault again — losing it
  // must not cost us the load bases, without which nothing is resolvable.
  CrashPuts("[jihad-bs] backtrace (raw return addresses; same base subtraction):\n");
  CrashBacktrace();

  CrashPuts("[jihad-bs] *** end fatal report\n");

  // Restore the default and re-raise so the process still dies with the correct
  // status (139 for SIGSEGV) and upstart's respawn accounting is unchanged.
  signal(sig, SIG_DFL);
  raise(sig);
}

inline void JihadInstallCrashHandler() {
  struct sigaction sa;
  memset(&sa, 0, sizeof sa);
  sa.sa_sigaction = JihadFatalSignal;
  // SA_NODEFER so the re-raise above is delivered immediately rather than sitting
  // pending behind our own handler; SA_RESETHAND so a fault inside the handler
  // dies outright instead of recursing.
  sa.sa_flags = SA_SIGINFO | SA_RESETHAND | SA_NODEFER;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGSEGV, &sa, nullptr);
  sigaction(SIGBUS,  &sa, nullptr);
  sigaction(SIGILL,  &sa, nullptr);
  sigaction(SIGFPE,  &sa, nullptr);
  sigaction(SIGABRT, &sa, nullptr);
}

// Self-test hook: $JIHAD_TEST_CRASH=1 makes the daemon fault deliberately once the
// engine is up. It exists because a crash reporter that has never been observed
// firing is worth nothing — this is what proves the handler survives the engine's
// own signal setup, which it did not on the first attempt.
inline void JihadMaybeTestCrash() {
  const char* e = getenv("JIHAD_TEST_CRASH");
  if (!e || !*e || !strcmp(e, "0")) return;
  CrashPuts("[jihad-bs] JIHAD_TEST_CRASH set — faulting deliberately\n");
  volatile int* p = (volatile int*)0x1;
  *p = 42;
}

}  // namespace jihad

#endif  // JIHAD_CRASH_REPORT_H
