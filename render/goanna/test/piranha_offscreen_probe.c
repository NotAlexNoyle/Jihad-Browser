/* Jihad R7 feasibility probe: can a Piranha PGContext be created over memory WE own,
 * inside an ordinary process that is not LunaSysMgr and has no display?
 *
 * This is the load-bearing assumption behind the "give Flash a PGContext" route: the
 * device's Flash answers npPalmUseGraphicsContext = true, so it wants to draw through a
 * PGContext rather than into a raster buffer. If a context can be bound to a wrapped
 * memory surface offscreen, we can hand it one and read its pixels back.
 *
 * Everything is resolved by dlsym on the mangled names so this needs no webOS headers.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _GNU_SOURCE
#include <dlfcn.h>

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);   /* novacom pipes block-buffer stdout */
    /* libWebKitLuna's constructors touch GLib, which aborts with "The thread system is not
     * yet initialized" unless g_thread_init() ran first. This is glib < 2.32 behaviour. */
    void* gt = dlopen("libgthread-2.0.so.0", RTLD_NOW | RTLD_GLOBAL); /* g_thread_init lives here */
    void* g = dlopen("libglib-2.0.so.0", RTLD_NOW | RTLD_GLOBAL);
    if (g) {
        void (*g_thread_init)(void*) = dlsym(gt ? gt : g, "g_thread_init");
        int (*g_thread_supported)(void) = dlsym(g, "g_thread_get_initialized");
        void (*g_type_init)(void) = dlsym(g, "g_type_init");
        if (g_thread_init) { g_thread_init(NULL); printf("ok: g_thread_init called\n"); }
        else printf("warn: no g_thread_init symbol\n");
        void* go = dlopen("libgobject-2.0.so.0", RTLD_NOW|RTLD_GLOBAL);
        if (!g_type_init && go) g_type_init = dlsym(go, "g_type_init");
        if (g_type_init) { g_type_init(); printf("ok: g_type_init called\n"); }
        (void)g_thread_supported;
    } else {
        printf("warn: no libglib: %s\n", dlerror());
    }

    void* h = dlopen("libWebKitLuna.so", RTLD_NOW | RTLD_GLOBAL);
    if (!h) { printf("FAIL dlopen: %s\n", dlerror()); return 1; }
    printf("ok: libWebKitLuna loaded\n");

    void* (*pgctx_create)(void)                     = dlsym(h, "_ZN9PGContext6createEv");
    void* (*pgsurf_wrap)(unsigned, unsigned, const unsigned char*, int)
                                                    = dlsym(h, "_ZN9PGSurface4wrapEjjPKhb");
    void  (*pgctx_setSurface)(void*, void*)         = dlsym(h, "_ZN9PGContext10setSurfaceEP9PGSurface");
    void  (*pgctx_clearRect)(void*, int, int, int, int, unsigned)
                                                    = dlsym(h, "_ZN9PGContext9clearRectEiiii8PColor32");
    void* (*pgctx_getSurface)(void*)                = dlsym(h, "_ZNK9PGContext10getSurfaceEv");

    printf("syms: create=%p wrap=%p setSurface=%p clearRect=%p getSurface=%p\n",
           (void*)pgctx_create, (void*)pgsurf_wrap, (void*)pgctx_setSurface,
           (void*)pgctx_clearRect, (void*)pgctx_getSurface);
    if (!pgctx_create || !pgsurf_wrap || !pgctx_setSurface || !pgctx_clearRect) {
        printf("FAIL: missing symbols\n"); return 2;
    }

    const unsigned W = 64, H = 32;
    unsigned char* buf = calloc(W * H, 4);
    if (!buf) { printf("FAIL alloc\n"); return 3; }

    void* surf = pgsurf_wrap(W, H, buf, 1);
    printf("PGSurface::wrap -> %p\n", surf);
    if (!surf) { printf("FAIL: wrap returned null\n"); return 4; }

    void* ctx = pgctx_create();
    printf("PGContext::create -> %p\n", ctx);
    if (!ctx) { printf("FAIL: create returned null\n"); return 5; }

    pgctx_setSurface(ctx, surf);
    printf("setSurface done; getSurface -> %p\n",
           pgctx_getSurface ? pgctx_getSurface(ctx) : (void*)-1);

    /* Paint the whole thing opaque red and see whether OUR buffer changed. */
    pgctx_clearRect(ctx, 0, 0, (int)W, (int)H, 0xFFFF0000u);

    unsigned nonzero = 0;
    for (unsigned i = 0; i < W * H * 4; i++) if (buf[i]) nonzero++;
    printf("first px: %02x %02x %02x %02x   nonzero bytes: %u/%u\n",
           buf[0], buf[1], buf[2], buf[3], nonzero, W * H * 4);
    printf(nonzero ? "RESULT: PGContext RENDERED INTO OUR MEMORY\n"
                   : "RESULT: buffer untouched (offscreen render did NOT work)\n");
    return 0;
}

/* RESULT, measured on device 2026-08-07 (HP TouchPad, webOS 3.0.5):
 *
 *   ok: g_thread_init called
 *   ok: g_type_init called
 *   ok: libWebKitLuna loaded
 *   syms: create=... wrap=... setSurface=... clearRect=... getSurface=...
 *   PGSurface::wrap -> 0x9a2e0
 *   PGContext::create -> 0x9a680
 *   setSurface done; getSurface -> 0x9a2e0
 *   first px: 00 00 ff ff   nonzero bytes: 4096/8192
 *   RESULT: PGContext RENDERED INTO OUR MEMORY
 *
 * 00 00 ff ff is BGRA little-endian for opaque red, and exactly half the bytes are
 * non-zero because only the R and A bytes are set. So Piranha rasterises into an
 * ordinary heap buffer, offscreen, in a process that is neither LunaSysMgr nor a
 * browser — no display, no compositor, no PIpc connection.
 *
 * BUILD:  arm-webos-linux-gnueabi-gcc -O1 -o pgtest piranha_offscreen_probe.c -ldl
 * RUN:    push to /tmp on the device and execute it (it needs no bundled loader; it is
 *         linked against the DEVICE's own glibc 2.8, deliberately, so it tests the
 *         webOS libraries on their own terms).
 *
 * GOTCHAS, both of which cost a cycle:
 *   - g_thread_init() must be called BEFORE libWebKitLuna is loaded or its constructors
 *     abort with "GLib-ERROR **: The thread system is not yet initialized". It lives in
 *     libgthread-2.0.so.0, NOT in libglib-2.0.so.0.
 *   - stdout is block-buffered through a novacom pipe, so an abort swallows every
 *     printf that came before it. setvbuf(_IONBF) or you will misread where it died.
 */
