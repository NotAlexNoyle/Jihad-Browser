/* Copyright 2026 NotAlexNoyle. Apache-2.0.
 *
 * gpu-bench-npixmap.h — Palm's native pixmaps (libnapp.so) for gpu-bench section 20 (T-616).
 *
 * The device EGL takes a Palm NWindowBuffer* as the EGLClientBuffer of
 * eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, buffer, {EGL_IMAGE_PRESERVED_KHR, EGL_FALSE})
 * (NRemoteTexture::create, libnrwindow.so:0x1880). The objects are GCC 4.x ARM C++ ones; a member
 * function is a C function whose first argument is `this`, so they are called through dlsym with the
 * mangled names. Everything below was read from the copies in build/webos-oe/out-arm/r9-closure/.
 *
 * Layout (NSharedWindowBuffer ctor 0xdce0, NPixmap ctor 0xddc0, NPixmapWrapped ctor 0x14a00,
 * NPixmapPrivate ctor 0x14fe0; sizes from the operator new calls: NPixmap 0x28, Wrapped/Private 0x30):
 *   +0  vptr      +4  PIpcBuffer* (NULL for Wrapped/Private)   +8  lock count   +12 size
 *   +16 header offset   +20 lockFree (byte)   +24 width   +28 height   +32 pitch (bytes)   +36 NYuvFormat (0 = RGB)
 *   +40 Wrapped: the caller's address / Private: the dlmalloc block   +44 Wrapped: byte size / Private: flag (byte)
 * Vtable (NPixmap's at 0x213a8): 0 TryLock, 1 Lock, 2 Unlock, 3 Handle, 4 Free, 5 Vaddr, 6 Size, 7/8 dtors,
 *   9 FreeNoDelete, 10 Resize, 11 Width, 12 Height, 13 Pitch, 14 YuvFormat.
 *
 * Three kinds, each a candidate for the readback:
 *   private  NPixmapPrivate(w, h, pitch): dlmalloc(h * pitch + page), Vaddr() = the next page boundary.
 *            The kind WebKit's ContentLayerPalm passes to NRemoteTexture::create (libWebKitLuna 0x5c4698),
 *            so the one known to work as a texture. Not shared memory.
 *   wrapped  NPixmapWrapped::Create(vaddr, w, h, pitch): wraps page-aligned memory the caller owns and
 *            mlock()s it. Section 20 hands it a SysV segment of its own (shmget/shmat), which is what the
 *            daemon would do with the card's buffer: a render into it would need no copy at all.
 *   shared   NPixmap::Create(w, h, 0): PIpcBuffer::create (libLunaSysMgrIpc 0x4570) = shmget + shmat +
 *            IPC_RMID, a 68-byte header, pixels from the next page boundary; Handle() is the shmid.
 */
#ifndef GPU_BENCH_NPIXMAP_H
#define GPU_BENCH_NPIXMAP_H
#include <dlfcn.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

enum { NPIX_PRIVATE = 0, NPIX_WRAPPED = 1, NPIX_SHARED = 2, NPIX_KINDS = 3 };
static const char* const npix_names[NPIX_KINDS] = { "private", "wrapped-sysv", "shared" };

struct npix_layout {   // read-only cross-check of what the calls return; never written
  void* vptr; void* ipc; int32_t locks; int32_t size; int32_t offset; uint8_t lockFree; uint8_t pad[3];
  int32_t width; int32_t height; int32_t pitch; int32_t yuv; void* mem; int32_t extra;
};

struct npix_api {
  void* lib;
  void* (*priv_ctor)(void* self, int w, int h, int pitch);   // _ZN14NPixmapPrivateC1Eiii       0x14fe0
  void* (*priv_vaddr)(void* self);                           // _ZN14NPixmapPrivate5VaddrEv     0x14f00
  void (*priv_free)(void* self);                             // _ZN14NPixmapPrivate4FreeEv      0x14f30 (dlfree; the object stays)
  void* (*wrap_create)(void* vaddr, int w, int h, int pitch);// _ZN14NPixmapWrapped6CreateEPviii 0x14cf0
  void* (*wrap_vaddr)(void* self);                           // _ZN14NPixmapWrapped5VaddrEv     0x147e0
  void* (*pix_create)(int w, int h, int yuv);                // _ZN7NPixmap6CreateEii10NYuvFormat 0xede0
  void* (*shared_vaddr)(void* self);                         // _ZN19NSharedWindowBuffer5VaddrEv 0xe200
  int (*shared_handle)(void* self);                          // _ZN19NSharedWindowBuffer6HandleEv 0xdda0
  void (*shared_free)(void* self);                           // _ZN19NSharedWindowBuffer4FreeEv  0xe0e0 (FreeNoDelete, then delete)
};

struct npix_buf {
  int kind; void* obj; uint8_t* vaddr; int pitch; int key;   // key: the SysV shmid (wrapped: ours; shared: Palm's), else -1
  void* storage; void* shmaddr;
};

// The crash guard: a fault inside Palm's or Qualcomm's code prints the step it happened in.
static sigjmp_buf npix_jmp; static volatile const char* npix_step = "start"; static volatile int npix_sig;
static void npix_on_fault(int sig) { npix_sig = sig; siglongjmp(npix_jmp, 1); }
static void npix_guard(int on) {
  struct sigaction sa; memset(&sa, 0, sizeof sa); sa.sa_handler = on ? npix_on_fault : SIG_DFL; sigemptyset(&sa.sa_mask);
  sigaction(SIGSEGV, &sa, NULL); sigaction(SIGBUS, &sa, NULL); sigaction(SIGILL, &sa, NULL); sigaction(SIGABRT, &sa, NULL);
}

static const char* npix_load(struct npix_api* a) {
  memset(a, 0, sizeof *a);
  a->lib = dlopen("/usr/lib/libnapp.so", RTLD_NOW | RTLD_GLOBAL); if (!a->lib) a->lib = dlopen("libnapp.so", RTLD_NOW | RTLD_GLOBAL);
  if (!a->lib) return dlerror();
#define NPIX_SYM(field, name) do { *(void**)&a->field = dlsym(a->lib, name); if (!a->field) return "missing " name; } while (0)
  NPIX_SYM(priv_ctor, "_ZN14NPixmapPrivateC1Eiii"); NPIX_SYM(priv_vaddr, "_ZN14NPixmapPrivate5VaddrEv"); NPIX_SYM(priv_free, "_ZN14NPixmapPrivate4FreeEv");
  NPIX_SYM(wrap_create, "_ZN14NPixmapWrapped6CreateEPviii"); NPIX_SYM(wrap_vaddr, "_ZN14NPixmapWrapped5VaddrEv");
  NPIX_SYM(pix_create, "_ZN7NPixmap6CreateEii10NYuvFormat"); NPIX_SYM(shared_vaddr, "_ZN19NSharedWindowBuffer5VaddrEv");
  NPIX_SYM(shared_handle, "_ZN19NSharedWindowBuffer6HandleEv"); NPIX_SYM(shared_free, "_ZN19NSharedWindowBuffer4FreeEv");
#undef NPIX_SYM
  return NULL;
}

// NPixmap::Calc (0xdec0): the pitch is the width rounded up to 32 pixels, times four bytes.
static int npix_pitch(int w) { return ((w + 31) & ~31) * 4; }

// NULL on success, else why not. Nothing is left allocated on failure.
static const char* npix_create(const struct npix_api* a, int kind, int w, int h, struct npix_buf* b) {
  const int pitch = npix_pitch(w); const size_t bytes = (size_t)pitch * h;
  memset(b, 0, sizeof *b); b->kind = kind; b->pitch = pitch; b->key = -1;
  if (kind == NPIX_PRIVATE) {
    b->storage = calloc(1, 64); if (!b->storage) return "no memory for the object";   // the object is 0x30 bytes
    a->priv_ctor(b->storage, w, h, pitch);
    if (!((struct npix_layout*)b->storage)->mem) { free(b->storage); b->storage = NULL; return "NPixmapPrivate: dlmalloc returned NULL"; }
    b->obj = b->storage; b->vaddr = a->priv_vaddr(b->obj);
  } else if (kind == NPIX_WRAPPED) {
    const long page = sysconf(_SC_PAGESIZE); const size_t len = (bytes + page - 1) & ~(size_t)(page - 1);
    int id = shmget(IPC_PRIVATE, len, IPC_CREAT | 0600); if (id < 0) return "shmget failed";
    void* p = shmat(id, NULL, 0); shmctl(id, IPC_RMID, NULL);   // gone with the last detach, also after a crash
    if (p == (void*)-1) return "shmat failed";
    b->shmaddr = p; b->key = id;
    b->obj = a->wrap_create(p, w, h, pitch);   // NULL when the address is not page-aligned or mlock fails
    if (!b->obj) { shmdt(p); b->shmaddr = NULL; return "NPixmapWrapped::Create returned NULL (mlock?)"; }
    b->vaddr = a->wrap_vaddr(b->obj);
  } else {
    b->obj = a->pix_create(w, h, 0); if (!b->obj) return "NPixmap::Create returned NULL";
    if (!((struct npix_layout*)b->obj)->ipc) return "NPixmap::Create: no PIpcBuffer (object leaked)";
    b->key = a->shared_handle(b->obj); b->vaddr = a->shared_vaddr(b->obj);
  }
  const struct npix_layout* l = (const struct npix_layout*)b->obj;
  if (l->width != w || l->height != h || l->pitch != pitch) return "the object's width/height/pitch are not at +24/+28/+32 (layout differs; object leaked)";
  if (!b->vaddr) return "Vaddr() is NULL (object leaked)";
  return NULL;
}

static void npix_destroy(const struct npix_api* a, struct npix_buf* b) {
  if (!b->obj) return;
  if (b->kind == NPIX_PRIVATE) { a->priv_free(b->obj); free(b->storage); }
  else a->shared_free(b->obj);   // Wrapped: munlock + delete; shared: munlock, ~PIpcBuffer (shmdt), delete
  if (b->shmaddr) shmdt(b->shmaddr);
  memset(b, 0, sizeof *b);
}
#endif
