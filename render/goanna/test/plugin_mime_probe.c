// Copyright 2026 NotAlexNoyle. Apache-2.0.
//
// Reproduce, standalone, what nsPluginFile::GetPluginInfo does to every plugin it finds:
// dlopen the .so and call NP_GetPluginVersion / NP_GetMIMEDescription / NP_GetValue on it.
//
// nsPluginsDirUnix.cpp does this IN THE PARENT PROCESS during the plugin scan, before any
// plugin-container exists. For the device's own libflashplayer.so that pulls libWebKitLuna,
// libPiranha, libLunaSysMgrIpc, libv8, libEGL and the rest of LunaSysMgr's WebKit into the
// daemon, and the daemon then wedges. This probe isolates WHICH call blocks, outside the
// daemon, so the answer does not cost a 45 s libxul rebuild per attempt.
//
// Every step prints before it runs and flushes immediately: the last line printed IS the call
// that hung. Do not buffer — a hang with a buffered stdout looks like a crash before step 1.
//
// Usage: plugin-mime-probe <plugin.so> [lazy|now] [local|global]
// Defaults are lazy+local, which is what NSPR's PR_LoadLibraryWithFlags(spec, 0) gives.

#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// From npapi.h. Declared here rather than included so this probe has no build dependency
// on the UXP tree beyond a C compiler.
#define NPPVpluginNameString        8
#define NPPVpluginDescriptionString 9

// ── The /PmLogLib named-semaphore ABI trap ───────────────────────────────────────────────
//
// libPmLogLib.so — pulled in by libWebKitLuna, which is a DT_NEEDED of the device's
// libflashplayer.so — opens the POSIX named semaphore "/PmLogLib" in its constructor and
// sem_wait()s it to guard a SysV shared-memory table of log contexts.
//
// glibc changed sem_t's layout in 2.21: the value word went from a raw token count to
// (tokens << 1) | has-waiters. webOS's userland is glibc 2.8 (old layout); everything we
// cross-build runs on the bundled glibc 2.23 (new layout). /dev/shm/sem.PmLogLib is created
// once per boot by whichever process gets there first. When that is a system process it
// holds the raw word 1, which our sem_wait reads as ZERO tokens with the waiters bit set —
// so it goes straight to futex_wait and never returns. dlopen never completes, and the
// process that called it (the daemon during its plugin scan, or plugin-container) wedges.
//
// Repairing the shared semaphore would silently weaken PmLogLib's exclusion for every other
// process on the device. Interposing ONE name does not: PmLogLib gets a process-private
// semaphore, every other sem_open (notably our own sem.browserserver.* shared buffers, which
// the adapter really does open from the other side) is forwarded untouched.
//
// -DJIHAD_PMLOG_SEM_FIX enables it, so one binary can measure both halves of the bisect.
#ifdef JIHAD_PMLOG_SEM_FIX
sem_t* sem_open(const char* name, int oflag, ...)
{
  static sem_t* (*real_sem_open)(const char*, int, ...);
  if (!real_sem_open)
    real_sem_open = (sem_t* (*)(const char*, int, ...))dlsym(RTLD_NEXT, "sem_open");

  mode_t mode = 0;
  unsigned int value = 0;
  int have_create_args = (oflag & O_CREAT) != 0;
  if (have_create_args) {
    va_list ap;
    va_start(ap, oflag);
    mode = (mode_t)va_arg(ap, unsigned int);
    value = va_arg(ap, unsigned int);
    va_end(ap);
  }

  if (name && (!strcmp(name, "/PmLogLib") || !strcmp(name, "PmLogLib"))) {
    static sem_t priv;
    static int inited = 0;
    if (!inited) {
      // pshared=0: never leaves this process, so no other glibc ever parses its layout.
      sem_init(&priv, 0, have_create_args ? value : 1);
      inited = 1;
    }
    fprintf(stderr, "[probe] sem_open(\"%s\") -> PRIVATE semaphore (ABI trap avoided)\n", name);
    fflush(stderr);
    return &priv;
  }

  if (!real_sem_open) return SEM_FAILED;
  if (have_create_args) return real_sem_open(name, oflag, mode, value);
  return real_sem_open(name, oflag);
}
#endif

static void step(const char* what)
{
  printf("[probe] -> %s\n", what);
  fflush(stdout);
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    fprintf(stderr, "usage: %s <plugin.so> [lazy|now] [local|global]\n", argv[0]);
    return 2;
  }

  const char* path = argv[1];
  int flags = RTLD_LAZY;
  if (argc > 2 && !strcmp(argv[2], "now")) flags = RTLD_NOW;
  if (argc > 3 && !strcmp(argv[3], "global")) flags |= RTLD_GLOBAL;
  else flags |= RTLD_LOCAL;

  printf("[probe] pid=%d path=%s flags=0x%x (%s|%s)\n", (int)getpid(), path, flags,
         (flags & RTLD_NOW) ? "now" : "lazy",
         (flags & RTLD_GLOBAL) ? "global" : "local");
  fflush(stdout);

  step("dlopen");
  void* h = dlopen(path, flags);
  if (!h) {
    printf("[probe] dlopen FAILED: %s\n", dlerror());
    return 1;
  }
  printf("[probe] dlopen ok handle=%p\n", h);
  fflush(stdout);

  step("dlsym NP_GetPluginVersion");
  const char* (*npGetPluginVersion)(void) =
      (const char* (*)(void))dlsym(h, "NP_GetPluginVersion");
  printf("[probe] NP_GetPluginVersion=%p\n", (void*)npGetPluginVersion);
  fflush(stdout);
  if (npGetPluginVersion) {
    step("call NP_GetPluginVersion");
    const char* v = npGetPluginVersion();
    printf("[probe] version=%s\n", v ? v : "(null)");
    fflush(stdout);
  }

  step("dlsym NP_GetMIMEDescription");
  const char* (*npGetMIMEDescription)(void) =
      (const char* (*)(void))dlsym(h, "NP_GetMIMEDescription");
  printf("[probe] NP_GetMIMEDescription=%p\n", (void*)npGetMIMEDescription);
  fflush(stdout);
  if (!npGetMIMEDescription) {
    printf("[probe] no NP_GetMIMEDescription -> plugin would be skipped\n");
    return 1;
  }

  step("call NP_GetMIMEDescription");
  const char* mime = npGetMIMEDescription();
  printf("[probe] mime=%s\n", mime ? mime : "(null)");
  fflush(stdout);

  step("dlsym NP_GetValue");
  int (*npGetValue)(void*, int, void*) = (int (*)(void*, int, void*))dlsym(h, "NP_GetValue");
  printf("[probe] NP_GetValue=%p\n", (void*)npGetValue);
  fflush(stdout);

  if (npGetValue) {
    const char* name = NULL;
    step("call NP_GetValue(name)");
    npGetValue(NULL, NPPVpluginNameString, &name);
    printf("[probe] name=%s\n", name ? name : "(null)");
    fflush(stdout);

    const char* desc = NULL;
    step("call NP_GetValue(description)");
    npGetValue(NULL, NPPVpluginDescriptionString, &desc);
    printf("[probe] description=%s\n", desc ? desc : "(null)");
    fflush(stdout);
  }

  step("dlclose");
  dlclose(h);
  printf("[probe] DONE — the full scan sequence completed\n");
  fflush(stdout);
  return 0;
}
