/* pipc_shim.c — LD_PRELOAD interpose shim for the plugin-container.
 *
 * PURPOSE (2026-08-08): the Flash draw gate (libflashplayer.so 0x5c190) reduces to
 * "is Flash's own render target non-null", and that target is a PGContext Flash builds
 * ITSELF over a PIpcBuffer shared-memory bitmap in its surface factory
 * (0x2ef940 -> 0x554d0 -> 0x56390 -> 0x561b0 -> 0x571b0).  The running child holds NO
 * socket to /tmp/pipcserver.sysmgr, so either the factory never runs or it runs and
 * fails.  This shim answers which, by logging every call Flash (or anyone) makes to the
 * factory's external dependencies.  It changes no behavior — every call is forwarded.
 *
 * All interposed signatures are pointer/int only, so plain C can forward them.
 * Flash binds these via PLT out of the GLOBAL lookup scope, and an LD_PRELOADed object
 * is first in that scope, so interposition wins — the same mechanism that already
 * proved out for NPN_InvalidateRect (JihadNPNInterpose.cpp).
 *
 * Build:  build/webos-oe/build-pipc-shim-arm.sh
 * Deploy: novacom put into $HL, add LD_PRELOAD="$HL/jihad-pipc-shim.so" to the
 *         variant's upstart exec env line (the child inherits the daemon's environ).
 * Logs:   stderr, tagged [jihad-pipc-shim], so they land in the daemon log like the
 *         rest of the child's output.  stderr is unbuffered; no fflush games needed.
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define TAG "[jihad-pipc-shim] "

/* Dump the interesting lines of /proc/self/maps once, so the raw caller addresses
 * logged below can be turned into library+offset offline. */
static void dump_maps_once(void)
{
    static int done; /* races at worst duplicate the dump — harmless */
    if (done) return;
    done = 1;
    FILE *m = fopen("/proc/self/maps", "r");
    if (!m) return;
    char line[512];
    while (fgets(line, sizeof line, m)) {
        if (strstr(line, "flashplayer") || strstr(line, "WebKitLuna") ||
            strstr(line, "libxul") || strstr(line, "SysMgrIpc") ||
            strstr(line, "Piranha"))
            if (strstr(line, "r-xp"))
                fprintf(stderr, TAG "map %s", line);
    }
    fclose(m);
}

/* Resolve the real symbol: RTLD_NEXT first (works when the provider is in the global
 * scope), then the named provider library (works when it only lives in a dlopen'd
 * RTLD_LOCAL tree, e.g. pulled in as a dependency of libflashplayer). */
static void *resolve(const char *lib, const char *sym)
{
    void *p = dlsym(RTLD_NEXT, sym);
    if (p) return p;
    void *h = dlopen(lib, RTLD_LAZY | RTLD_NOLOAD);
    if (!h) h = dlopen(lib, RTLD_LAZY);
    if (h) p = dlsym(h, sym);
    if (!p) fprintf(stderr, TAG "UNRESOLVED %s in %s\n", sym, lib);
    return p;
}

#define LUNA_IPC "libLunaSysMgrIpc.so"
#define WEBKIT   "libWebKitLuna.so"

/* GCC-COW std::string: the object is a single pointer to the character data. */
static const char *str_cstr(const void *stdstring_ref)
{
    const char *const *p = (const char *const *)stdstring_ref;
    return (p && *p) ? *p : "(null)";
}

/* ── PIpcClient::PIpcClient(const std::string&, const std::string&, GMainLoop*) ── */
typedef void (*ctor4_t)(void *, const void *, const void *, void *);

void _ZN10PIpcClientC2ERKSsS1_P10_GMainLoop(void *self, const void *name,
                                            const void *appname, void *loop)
{
    static ctor4_t real;
    dump_maps_once();
    fprintf(stderr, TAG "PIpcClient ctor(C2) name=\"%s\" app=\"%s\" loop=%p caller=%p\n",
            str_cstr(name), str_cstr(appname), loop, __builtin_return_address(0));
    if (!real) real = (ctor4_t)resolve(LUNA_IPC, "_ZN10PIpcClientC2ERKSsS1_P10_GMainLoop");
    if (real) real(self, name, appname, loop);
    fprintf(stderr, TAG "PIpcClient ctor(C2) done self=%p\n", self);
}

void _ZN10PIpcClientC1ERKSsS1_P10_GMainLoop(void *self, const void *name,
                                            const void *appname, void *loop)
{
    static ctor4_t real;
    dump_maps_once();
    fprintf(stderr, TAG "PIpcClient ctor(C1) name=\"%s\" app=\"%s\" loop=%p caller=%p\n",
            str_cstr(name), str_cstr(appname), loop, __builtin_return_address(0));
    if (!real) real = (ctor4_t)resolve(LUNA_IPC, "_ZN10PIpcClientC1ERKSsS1_P10_GMainLoop");
    if (real) real(self, name, appname, loop);
    fprintf(stderr, TAG "PIpcClient ctor(C1) done self=%p\n", self);
}

/* ── PIpcBuffer* PIpcBuffer::create(int size) ─────────────────────────────────── */
typedef void *(*create_i_t)(int);

void *_ZN10PIpcBuffer6createEi(int size)
{
    static create_i_t real;
    dump_maps_once();
    if (!real) real = (create_i_t)resolve(LUNA_IPC, "_ZN10PIpcBuffer6createEi");
    void *r = real ? real(size) : 0;
    fprintf(stderr, TAG "PIpcBuffer::create(%d) -> %p caller=%p\n",
            size, r, __builtin_return_address(0));
    return r;
}

/* ── void* PIpcBuffer::data() const ───────────────────────────────────────────── */
typedef void *(*method0_t)(void *);

void *_ZNK10PIpcBuffer4dataEv(void *self)
{
    static method0_t real;
    if (!real) real = (method0_t)resolve(LUNA_IPC, "_ZNK10PIpcBuffer4dataEv");
    void *r = real ? real(self) : 0;
    fprintf(stderr, TAG "PIpcBuffer::data(%p) -> %p caller=%p\n",
            self, r, __builtin_return_address(0));
    return r;
}

/* ── PGContext* PGContext::create() ───────────────────────────────────────────── */
typedef void *(*create0_t)(void);

void *_ZN9PGContext6createEv(void)
{
    static create0_t real;
    dump_maps_once();
    if (!real) real = (create0_t)resolve(WEBKIT, "_ZN9PGContext6createEv");
    void *r = real ? real() : 0;
    fprintf(stderr, TAG "PGContext::create() -> %p caller=%p\n",
            r, __builtin_return_address(0));
    return r;
}

/* ── PGContextIface* PGThreadGlobalContext::graphicsContext() ─────────────────── */
void *_ZN21PGThreadGlobalContext15graphicsContextEv(void *self)
{
    static method0_t real;
    if (!real) real = (method0_t)resolve(WEBKIT, "_ZN21PGThreadGlobalContext15graphicsContextEv");
    void *r = real ? real(self) : 0;
    fprintf(stderr, TAG "PGThreadGlobalContext::graphicsContext(%p) -> %p caller=%p\n",
            self, r, __builtin_return_address(0));
    return r;
}

/* ── v2: the createSurface internals.  PGContext::createSurface itself is reached via
 * VTABLE and cannot be interposed; these are the PLT-bound calls it makes, so they can.
 * (luna 0x5ba000: new PGSurface -> _ZN9PGSurfaceC1Ev; pixmap alloc ->
 *  _ZN11PSoftPixmap3SetE7PFormatPKvjb, libPiranha 0xb0db0.) */

#define PIRANHA "libPiranha.so"

/* PGSurface::PGSurface() */
typedef void (*ctor0_t)(void *);

void _ZN9PGSurfaceC1Ev(void *self)
{
    static ctor0_t real;
    if (!real) real = (ctor0_t)resolve(WEBKIT, "_ZN9PGSurfaceC1Ev");
    if (real) real(self);
    fprintf(stderr, TAG "PGSurface ctor self=%p caller=%p\n",
            self, __builtin_return_address(0));
}

/* The primary Flash stage pixmap, captured at Set time so a SIGUSR1 can dump it from inside
 * the process (avoids the /proc/pid/mem attach race — the child often exits before an external
 * dd can seek).  We remember the biggest pixmap Set'd (the 320x240 stage, not font glyphs). */
static void *g_pix_self;
static unsigned g_pix_w, g_pix_h;

static void dump_pixmap(int sig)
{
    (void)sig;
    if (!g_pix_self || !g_pix_w || !g_pix_h) {
        fprintf(stderr, TAG "SIGUSR1: no pixmap captured yet\n");
        return;
    }
    void *data = *(void *const *)((const char *)g_pix_self + 0x48);
    if (!data) { fprintf(stderr, TAG "SIGUSR1: pixmap data null\n"); return; }
    unsigned bytes = g_pix_w * g_pix_h * 4u; /* assume 32bpp, tight rows */
    int fd = open("/tmp/flashpixmap.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { fprintf(stderr, TAG "SIGUSR1: open failed\n"); return; }
    ssize_t w = write(fd, data, bytes);
    close(fd);
    /* histogram the four corners + center so the log alone shows whether anything painted */
    const unsigned *p = (const unsigned *)data;
    unsigned center = p[(g_pix_h / 2) * g_pix_w + g_pix_w / 2];
    fprintf(stderr, TAG "SIGUSR1 dumped %ld/%u bytes from %p (%ux%u) "
            "px[0]=%08x center=%08x px[last]=%08x\n",
            (long)w, bytes, data, g_pix_w, g_pix_h,
            p[0], center, p[g_pix_w * g_pix_h - 1]);
}

/* bool PSoftPixmap::Set(PFormat, const void*, unsigned rowbytes, bool)
 * Layout facts used below (from luna 0x5ba050-0x5ba078, the field stores):
 *   +0x54/+0x56 halfwords = width/height, +0x4e byte = valid flag, +0x48 = pixel data. */
typedef int (*pixset_t)(void *, int, const void *, unsigned, int);

int _ZN11PSoftPixmap3SetE7PFormatPKvjb(void *self, int fmt, const void *pixels,
                                       unsigned rowbytes, int flag)
{
    static pixset_t real;
    if (!real) real = (pixset_t)resolve(PIRANHA, "_ZN11PSoftPixmap3SetE7PFormatPKvjb");
    const unsigned short *hw = (const unsigned short *)self;
    unsigned w = hw[0x54 / 2], h = hw[0x56 / 2];
    fprintf(stderr, TAG "PSoftPixmap::Set(%p fmt=%d pixels=%p rowbytes=%u flag=%d)"
            " w=%u h=%u caller=%p\n",
            self, fmt, pixels, rowbytes, flag, w, h, __builtin_return_address(0));
    int r = real ? real(self, fmt, pixels, rowbytes, flag) : 0;
    fprintf(stderr, TAG "PSoftPixmap::Set -> %d valid=%u data=%p\n",
            r, (unsigned)((const unsigned char *)self)[0x4e],
            *(void *const *)((const char *)self + 0x48));
    /* remember the stage-sized pixmap (>= 64x64 rules out font/glyph pixmaps) */
    if (r && w >= 64 && h >= 64) { g_pix_self = self; g_pix_w = w; g_pix_h = h; }
    return r;
}

__attribute__((constructor)) static void shim_init(void)
{
    signal(SIGUSR1, dump_pixmap);
    fprintf(stderr, TAG "loaded in pid %d (v3, SIGUSR1 dumps stage pixmap)\n", (int)getpid());
}
