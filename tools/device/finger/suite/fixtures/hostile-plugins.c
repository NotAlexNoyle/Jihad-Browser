/* Copyright 2026 NotAlexNoyle. Apache-2.0.
 *
 * hostile-plugins.c -- the libraries of cavekit-flash.md R10's hostile plugin folder (T-580), built by hostile-plugins.sh
 * with the device toolchain, one per -D: HANG (a constructor that never returns), ABORT (a constructor that aborts),
 * NOTNP (a good library that is no NPAPI plugin). The fourth, the junk, is not built: it is random bytes named .so.
 */
#include <stdlib.h>
#include <unistd.h>

#if defined(HANG)
__attribute__((constructor)) static void hostile(void) { for (;;) sleep(1); }
#elif defined(ABORT)
__attribute__((constructor)) static void hostile(void) { abort(); }
#endif

#if defined(HANG) || defined(ABORT)
/* what a scan looks for first, so a loader that stopped short of the constructor would still take it for a plugin */
const char* NP_GetMIMEDescription(void) { return "application/x-jihad-hostile:jhst:Hostile"; }
int NP_GetValue(void* f, int v, void* out) { (void)f; (void)v; *(const char**)out = "Hostile"; return 0; }
#else
int jihad_not_a_plugin(void) { return 1; }
#endif
