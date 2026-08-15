// Copyright 2026 NotAlexNoyle. Apache-2.0.
//
// Jihad Browser — why does Flash load libasound and then never open a PCM?
//
// ANSWERED 2026-08-10, and the premise was wrong: NOTHING was broken. The SWFs this was run
// against reused one character ID for DefineShape and DefineSound, so the player dropped the
// sound and never started playback. With a corrected asset Flash opens the PCM and the tone is
// audible. Keep this file for the technique, not for its conclusion.
//
// The measurement that misled: strace -f on a fresh plugin-container showed Flash resolving and
// opening /usr/lib/libasound.so.2 and then not one further ALSA syscall. That dlopen is Flash's
// startup AVAILABILITY PROBE (libflashplayer.so 0x2fbf08), which runs whether or not anything
// ever plays, so its presence says nothing and its silence afterwards only meant "no sound was
// started". Two traps made it look worse than it was: attaching to the CONTAINER rather than the
// daemon misses the bring-up entirely, because the container is spawned per navigation; and a
// container that never plays looks identical to one that cannot.
//
// So this traces the LIBRARY boundary instead:
//   dlopen  — every library Flash pulls in at runtime, and whether it succeeded
//   dlsym   — every "snd_*" lookup and, critically, every one that returns NULL, which is the
//             single most likely way an audio backend disables itself silently
//   snd_pcm_open / snd_pcm_open_lconf — whether the call is ever reached, and its error
//
// dlsym is called by everything, so it is filtered to snd_*/pa_* lookups plus all failures;
// unfiltered it would bury the answer and slow the child enough to change the timing.
//
// DIAGNOSTIC ONLY — LD_PRELOAD into plugin-container, never shipped. Note the standing trap:
// an LD_PRELOAD in the daemon's environment is inherited by every child it spawns, and a
// 2.23-linked .so preloaded into a /bin/sh would segfault it, so this goes in for a
// measurement and comes straight back out.
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void logline(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "[jihad-audio-shim] ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  fflush(stderr);
}

// Resolved lazily from dlsym: Flash asks for snd_pcm_open by name, so that lookup is where we
// capture the real implementation and substitute our own.
static int (*g_real_snd_pcm_open)(void**, const char*, int, int);
int snd_pcm_open(void** pcm, const char* name, int stream, int mode);

void* dlopen(const char* file, int mode)
{
  static void* (*real)(const char*, int);
  if (!real) real = (void* (*)(const char*, int))dlsym(RTLD_NEXT, "dlopen");
  void* h = real ? real(file, mode) : NULL;
  // Only the interesting ones: the child dlopens plenty of Gecko modules we do not care about.
  if (file && (strstr(file, "asound") || strstr(file, "pulse") || strstr(file, "audio") ||
               strstr(file, "snd"))) {
    logline("dlopen(\"%s\", 0x%x) -> %p%s", file, mode, h, h ? "" : " FAILED");
    if (!h) logline("  dlerror: %s", dlerror());
  }
  return h;
}

void* dlsym(void* handle, const char* name)
{
  static void* (*real)(void*, const char*);
  if (!real) {
    // Bootstrap: cannot dlsym our way to dlsym. The versioned private entry point is what
    // glibc's own dlsym resolves to, and dlvsym is not itself interposed here.
    real = (void* (*)(void*, const char*))dlvsym(RTLD_NEXT, "dlsym", "GLIBC_2.4");
    if (!real) real = (void* (*)(void*, const char*))dlvsym(RTLD_NEXT, "dlsym", "GLIBC_2.0");
  }
  if (!real) return NULL;
  void* p = real(handle, name);
  if (name && (!strncmp(name, "snd_", 4) || !strncmp(name, "pa_", 3))) {
    logline("dlsym(\"%s\") -> %p%s", name, p, p ? "" : "   *** NULL — this is how audio dies ***");
  } else if (!p && name) {
    logline("dlsym(\"%s\") -> NULL", name);
  }
  // Flash calls ALSA through the POINTER dlsym hands back, never through a PLT entry, so a
  // plain symbol interposition of snd_pcm_open can never fire — the first version of this shim
  // proved the API resolves and still could not say whether it is used. Hand back OUR wrapper
  // instead: this is the only way to see the call, its device name and its return code.
  if (p && name && !strcmp(name, "snd_pcm_open")) {
    g_real_snd_pcm_open = (int (*)(void**, const char*, int, int))p;
    logline("dlsym(snd_pcm_open): handing back the shim wrapper instead of %p", p);
    return (void*)snd_pcm_open;
  }
  return p;
}

// If Flash ever reaches these, audio is genuinely being attempted and the failure is inside
// ALSA rather than before it.
int snd_pcm_open(void** pcm, const char* name, int stream, int mode)
{
  int (*real)(void**, const char*, int, int) = g_real_snd_pcm_open;
  logline("*** snd_pcm_open(\"%s\", stream=%d, mode=0x%x) CALLED ***",
          name ? name : "(null)", stream, mode);
  if (!real) { logline("  no real snd_pcm_open captured"); return -1; }
  int rc = real(pcm, name, stream, mode);
  logline("snd_pcm_open -> %d%s", rc, rc < 0 ? "  *** FAILED ***" : "");
  return rc;
}
