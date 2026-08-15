/* Copyright 2026 NotAlexNoyle. Apache-2.0. */
/*
 * Jihad Browser — trivial NPAPI test plugin (cavekit-addons-extensions.md R7).
 *
 * WHY THIS EXISTS. The only real NPAPI binary on the TouchPad is the device's own
 * libflashplayer.so, and that build is linked against LunaSysMgr's WebKit host
 * (libWebKitLuna, libPiranha, libLunaSysMgrIpc). When it fails there is no way to tell
 * "our NPAPI port is broken" from "this particular plugin will not host anywhere but
 * LunaSysMgr". R7's last criterion asks for exactly this control: the NPAPI ARCHITECTURE
 * has to be generic enough that any platform-appropriate NPAPI binary works.
 *
 * So this plugin is deliberately as close to nothing as an NPAPI plugin can be: the four
 * entry points, a windowless instance, and a solid-colour paint through the async bitmap
 * surface drawing model (the only non-X drawing model this headless build can composite —
 * see the port status in the kit). Every step announces itself on stderr with a
 * [jihad-testplugin] tag, and stderr in the plugin child is inherited from the daemon, so
 * the whole sequence lands in the variant's daemon.log interleaved with the [jihad-npapi]
 * probes on the parent side.
 *
 * Build: build/webos-oe/build-test-plugin-arm.sh  (ARM, for the device)
 *        build/desktop/build-test-plugin.sh       (x86_64, for the desktop build)
 * Install: drop libjihadtestplugin.so in <profile>/plugins/ and load a page containing
 *        <embed type="application/x-jihad-test" width="200" height="150">
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "npapi.h"
#include "npfunctions.h"

#define LOG(...) do { fprintf(stderr, "[jihad-testplugin] " __VA_ARGS__); \
                      fputc('\n', stderr); fflush(stderr); } while (0)

static NPNetscapeFuncs sBrowser;

/* THE PALM EVENT STRUCTS, mirrored rather than included.
 *
 * On XP_WEBOS, Palm's own npapi.h defines NPEvent AS the Palm event union
 * (WebKit/Source/WebCore/plugins/npapi.h:748-762 in the webOS WebKit fork: a leading
 * NpPalmEventsEnum eventType followed by a union of key/pen/draw/system/gesture/touch).
 * UXP's npapi.h has no XP_WEBOS block, so compiling against it yields the X11/void NPEvent
 * instead. Mirroring the layout locally is what lets this plugin decode what the host sends
 * without dragging a second npapi.h into the build — the same approach, and the same field
 * order, as JihadNpPalmEvent in dom/plugins/ipc/PluginInstanceChild.cpp. */
typedef struct {
  int32_t chr;
  int32_t modifiers;
  int32_t rawkeyCode;
  int32_t rawModifier;
} JtpPalmKeyEvent;

typedef struct {
  int32_t xCoord, yCoord;
  int32_t modifiers;
} JtpPalmPenEvent;

typedef struct {
  int32_t type;
} JtpPalmSystemEvent;

typedef struct {
  uint32_t eventType;
  union {
    JtpPalmKeyEvent    keyEvent;
    JtpPalmPenEvent    penEvent;
    JtpPalmSystemEvent systemEvent;
    unsigned char      pad[256];   /* >= the largest member of Palm's union */
  } data;
} JtpPalmEvent;

/* NpPalmEventsEnum, npapi.h:609-629. Only the ones this control needs. */
#define JTP_PEN_DOWN        (1 << 0)
#define JTP_PEN_UP          (1 << 1)
#define JTP_PEN_MOVE        (1 << 2)
#define JTP_KEY_DOWN        (1 << 3)
#define JTP_KEY_UP          (1 << 4)
#define JTP_KEY_REPEAT      (1 << 5)
#define JTP_KEY_PRESS       (1 << 6)
#define JTP_DRAW            (1 << 7)
#define JTP_SYSTEM          (1 << 8)
#define JTP_PEN_DOUBLECLICK (1 << 15)
#define JTP_PEN_CLICK       (1 << 16)

/* Per-instance state. Kept trivial on purpose: the point of this plugin is to exercise the
 * host, so it holds only what a paint needs — plus the last input, because proving input
 * arrived is the other thing this control exists for. */
typedef struct {
  NPP        npp;
  int        width;
  int        height;
  int        windowless;      /* the host granted windowless mode */
  int        asyncSurface;    /* the host granted NPDrawingModelAsyncBitmapSurface */
  NPAsyncSurface surface;
  int        surfaceValid;
  int        markX, markY;    /* last pen-down, plugin-local; -1 = none yet */
  int        penDowns;
  int        keyDowns;
  int        eventLogged;     /* capped log counter */
} InstanceData;

/* ── the four entry points ────────────────────────────────────────────────────────────── */

NP_EXPORT(const char*)
NP_GetMIMEDescription(void)
{
  return "application/x-jihad-test:jtp:Jihad NPAPI test plugin";
}

NP_EXPORT(NPError)
NP_GetValue(void* future, NPPVariable variable, void* value)
{
  (void)future;
  switch (variable) {
    case NPPVpluginNameString:
      *((const char**)value) = "Jihad Test Plugin";
      return NPERR_NO_ERROR;
    case NPPVpluginDescriptionString:
      *((const char**)value) = "Solid-colour NPAPI control plugin for the Jihad port (R7).";
      return NPERR_NO_ERROR;
    default:
      return NPERR_INVALID_PARAM;
  }
}

/* ── instance lifecycle ───────────────────────────────────────────────────────────────── */

static NPError
jtpNew(NPMIMEType type, NPP instance, uint16_t mode, int16_t argc,
       char* argn[], char* argv[], NPSavedData* saved)
{
  (void)type; (void)mode; (void)saved;
  InstanceData* d;
  NPBool supportsWindowless = false;
  NPError err;
  int i;

  LOG("NPP_New enter argc=%d", (int)argc);

  d = (InstanceData*)calloc(1, sizeof(InstanceData));
  if (!d) {
    return NPERR_OUT_OF_MEMORY_ERROR;
  }
  d->npp = instance;
  d->width = 0;
  d->height = 0;
  d->markX = -1;
  d->markY = -1;
  instance->pdata = d;

  /* Width/height come from the element's attributes, not from the host, until the first
   * NPP_SetWindow. Read them so a paint before that still has a sane size. */
  for (i = 0; i < argc; i++) {
    if (argn[i] && argv[i]) {
      if (!strcasecmp(argn[i], "width"))  d->width  = atoi(argv[i]);
      if (!strcasecmp(argn[i], "height")) d->height = atoi(argv[i]);
    }
  }

  /* WINDOWLESS. There is no X on this device and no window a plugin could be given, so a
   * windowed instance cannot work at all — this is the request that used to be refused,
   * because NPNVSupportsWindowless answered false to every plugin on a headless toolkit
   * (patch 0016). Asking first, and logging the answer, is what makes that visible. */
  err = sBrowser.getvalue(instance, NPNVSupportsWindowless, &supportsWindowless);
  LOG("NPNVSupportsWindowless err=%d supported=%d", (int)err, (int)supportsWindowless);
  if (err == NPERR_NO_ERROR && supportsWindowless) {
    err = sBrowser.setvalue(instance, NPPVpluginWindowBool, (void*)false);
    LOG("set windowless err=%d", (int)err);
    d->windowless = (err == NPERR_NO_ERROR);
  }

  /* ASYNC BITMAP SURFACE — the drawing model. The X11 windowless paint path is compiled
   * out here (MOZ_X11 is undefined), so this is the one model whose output the offscreen
   * compositor can actually receive. gfxPlatformHeadless::SupportsPluginDirectBitmapDrawing
   * returns true, which is what makes it available. */
  err = sBrowser.setvalue(instance, NPPVpluginDrawingModel,
                          (void*)(intptr_t)NPDrawingModelAsyncBitmapSurface);
  LOG("set NPDrawingModelAsyncBitmapSurface err=%d", (int)err);
  d->asyncSurface = (err == NPERR_NO_ERROR);

  LOG("NPP_New ok windowless=%d async=%d size=%dx%d",
      d->windowless, d->asyncSurface, d->width, d->height);
  return NPERR_NO_ERROR;
}

static NPError
jtpDestroy(NPP instance, NPSavedData** save)
{
  InstanceData* d = (InstanceData*)instance->pdata;
  (void)save;
  LOG("NPP_Destroy");
  if (d) {
    if (d->surfaceValid) {
      sBrowser.finalizeasyncsurface(instance, &d->surface);
      d->surfaceValid = 0;
    }
    free(d);
    instance->pdata = NULL;
  }
  return NPERR_NO_ERROR;
}

/* Fill the surface with a flat colour and hand it to the host. Solid rather than a pattern
 * on purpose: what is being tested is position, size and orientation of the composited
 * result, and a single colour makes "the plugin drew" unambiguous in a screenshot. */
static void
jtpPaint(InstanceData* d)
{
  NPSize size;
  uint32_t* px;
  uint32_t bg;
  int32_t stride, x, y;

  if (!d->asyncSurface || d->width <= 0 || d->height <= 0) {
    return;
  }

  if (!d->surfaceValid) {
    size.width = d->width;
    size.height = d->height;
    memset(&d->surface, 0, sizeof(d->surface));
    if (sBrowser.initasyncsurface(d->npp, &size, NPImageFormatBGRA32, NULL,
                                  &d->surface) != NPERR_NO_ERROR) {
      LOG("initasyncsurface FAILED %dx%d", d->width, d->height);
      return;
    }
    d->surfaceValid = 1;
    LOG("initasyncsurface ok %dx%d stride=%d",
        d->width, d->height, (int)d->surface.bitmap.stride);
  }

  px = (uint32_t*)d->surface.bitmap.data;
  stride = d->surface.bitmap.stride;
  if (!px || stride <= 0) {
    LOG("surface has no bitmap data");
    return;
  }

  /* Opaque orange (BGRA32, premultiplied): distinct from anything the page or the chrome
   * paints, so "did the plugin's output composite?" is answerable from a screenshot alone.
   * A key-down flips it to cyan, which is how keyboard delivery is proven from a screenshot
   * with no log access at all. */
  bg = d->keyDowns ? 0xFF00C8FFu : 0xFFFF7F00u;
  for (y = 0; y < d->height; y++) {
    uint32_t* row = (uint32_t*)((unsigned char*)px + (size_t)y * stride);
    for (x = 0; x < d->width; x++) {
      row[x] = bg;
    }
  }

  /* THE COORDINATE ORACLE. R7 asks that plugin input be "mapped through the same coordinate
   * transform as page content", and a log line saying an event arrived cannot answer that —
   * only the position can. So the last pen-down paints a black marker at the coordinate the
   * plugin was given, in the plugin's own pixel space. Tap a known spot on the glass and the
   * marker either lands under the finger or it does not, which makes an off-by-zoom or an
   * off-by-scroll error visible in one screenshot instead of inferable from two. */
  if (d->markX >= 0 && d->markY >= 0) {
    int mx0 = d->markX - 8, mx1 = d->markX + 8;
    int my0 = d->markY - 8, my1 = d->markY + 8;
    if (mx0 < 0) mx0 = 0;
    if (my0 < 0) my0 = 0;
    if (mx1 > d->width)  mx1 = d->width;
    if (my1 > d->height) my1 = d->height;
    for (y = my0; y < my1; y++) {
      uint32_t* row = (uint32_t*)((unsigned char*)px + (size_t)y * stride);
      for (x = mx0; x < mx1; x++) {
        row[x] = 0xFF000000u;
      }
    }
  }

  sBrowser.setcurrentasyncsurface(d->npp, &d->surface, NULL);
  LOG("painted %dx%d mark=%d,%d pen=%d key=%d",
      d->width, d->height, d->markX, d->markY, d->penDowns, d->keyDowns);
}

static NPError
jtpSetWindow(NPP instance, NPWindow* window)
{
  InstanceData* d = (InstanceData*)instance->pdata;
  if (!d || !window) {
    return NPERR_GENERIC_ERROR;
  }
  LOG("NPP_SetWindow %ux%u at %d,%d type=%d",
      (unsigned)window->width, (unsigned)window->height,
      (int)window->x, (int)window->y, (int)window->type);

  if ((int)window->width != d->width || (int)window->height != d->height) {
    if (d->surfaceValid) {
      sBrowser.finalizeasyncsurface(instance, &d->surface);
      d->surfaceValid = 0;
    }
    d->width = (int)window->width;
    d->height = (int)window->height;
  }
  jtpPaint(d);
  return NPERR_NO_ERROR;
}

static int16_t
jtpHandleEvent(NPP instance, void* event)
{
  InstanceData* d = (InstanceData*)instance->pdata;
  const JtpPalmEvent* ev = (const JtpPalmEvent*)event;
  int repaint = 0;

  if (!d || !ev) {
    return 0;
  }

  /* Coordinates ARE logged here, unlike everywhere else in this port. This binary is a test
   * control that is hand-installed on a development device and never ships in a bundle, and
   * the coordinate is the entire content of the criterion being measured — withholding it
   * would leave "input arrived" and "input arrived at the right place" indistinguishable,
   * which is the exact confusion this plugin exists to remove. */
  if (d->eventLogged < 40) {
    d->eventLogged++;
    switch (ev->eventType) {
      case JTP_PEN_DOWN: case JTP_PEN_UP: case JTP_PEN_MOVE:
      case JTP_PEN_CLICK: case JTP_PEN_DOUBLECLICK:
        LOG("NPP_HandleEvent pen type=0x%x at %d,%d mods=0x%x",
            (unsigned)ev->eventType, (int)ev->data.penEvent.xCoord,
            (int)ev->data.penEvent.yCoord, (unsigned)ev->data.penEvent.modifiers);
        break;
      case JTP_KEY_DOWN: case JTP_KEY_UP: case JTP_KEY_REPEAT: case JTP_KEY_PRESS:
        LOG("NPP_HandleEvent key type=0x%x chr=%d raw=%d mods=0x%x",
            (unsigned)ev->eventType, (int)ev->data.keyEvent.chr,
            (int)ev->data.keyEvent.rawkeyCode, (unsigned)ev->data.keyEvent.modifiers);
        break;
      case JTP_SYSTEM:
        LOG("NPP_HandleEvent system type=%d", (int)ev->data.systemEvent.type);
        break;
      default:
        LOG("NPP_HandleEvent type=0x%x", (unsigned)ev->eventType);
        break;
    }
  }

  /* React VISIBLY, so the proof does not depend on reaching the log at all. */
  if (ev->eventType == JTP_PEN_DOWN) {
    d->penDowns++;
    d->markX = (int)ev->data.penEvent.xCoord;
    d->markY = (int)ev->data.penEvent.yCoord;
    repaint = 1;
  } else if (ev->eventType == JTP_KEY_DOWN) {
    d->keyDowns++;
    repaint = 1;
  }

  if (repaint) {
    jtpPaint(d);
  }

  /* 1 = consumed. A windowless plugin that answers 0 lets the host treat the gesture as page
   * input instead, which is the behaviour the flash-rect arbitration exists to arrange. */
  return 1;
}

static NPError
jtpGetValue(NPP instance, NPPVariable variable, void* value)
{
  (void)instance;
  switch (variable) {
    case NPPVpluginNameString:
      *((const char**)value) = "Jihad Test Plugin";
      return NPERR_NO_ERROR;
    case NPPVpluginDescriptionString:
      *((const char**)value) = "Solid-colour NPAPI control plugin for the Jihad port (R7).";
      return NPERR_NO_ERROR;
    default:
      return NPERR_INVALID_PARAM;
  }
}

static NPError jtpNewStream(NPP i, NPMIMEType t, NPStream* s, NPBool b, uint16_t* st)
{ (void)i;(void)t;(void)s;(void)b; *st = NP_NORMAL; return NPERR_NO_ERROR; }
static NPError jtpDestroyStream(NPP i, NPStream* s, NPReason r)
{ (void)i;(void)s;(void)r; return NPERR_NO_ERROR; }
static int32_t jtpWriteReady(NPP i, NPStream* s) { (void)i;(void)s; return 0x0FFFFFFF; }
static int32_t jtpWrite(NPP i, NPStream* s, int32_t o, int32_t l, void* b)
{ (void)i;(void)s;(void)o;(void)b; return l; }
static void    jtpStreamAsFile(NPP i, NPStream* s, const char* f) { (void)i;(void)s;(void)f; }
static void    jtpPrint(NPP i, NPPrint* p) { (void)i;(void)p; }
static void    jtpURLNotify(NPP i, const char* u, NPReason r, void* n)
{ (void)i;(void)u;(void)r;(void)n; }

/* ── NP_Initialize / NP_Shutdown ──────────────────────────────────────────────────────── */

NP_EXPORT(NPError)
NP_Initialize(NPNetscapeFuncs* bFuncs, NPPluginFuncs* pFuncs)
{
  LOG("NP_Initialize enter bFuncs=%p pFuncs=%p", (void*)bFuncs, (void*)pFuncs);
  if (!bFuncs || !pFuncs) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }
  if ((bFuncs->version >> 8) > NP_VERSION_MAJOR) {
    LOG("NP_Initialize: browser version %d too new", (int)(bFuncs->version >> 8));
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }

  /* Copy only as much of the browser table as this browser actually supplied. Copying the
   * full struct off the end of a shorter table is the classic NPAPI crash. */
  memset(&sBrowser, 0, sizeof(sBrowser));
  memcpy(&sBrowser, bFuncs,
         bFuncs->size < sizeof(sBrowser) ? bFuncs->size : sizeof(sBrowser));

  pFuncs->size          = sizeof(NPPluginFuncs);
  pFuncs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
  pFuncs->newp          = jtpNew;
  pFuncs->destroy       = jtpDestroy;
  pFuncs->setwindow     = jtpSetWindow;
  pFuncs->newstream     = jtpNewStream;
  pFuncs->destroystream = jtpDestroyStream;
  pFuncs->asfile        = jtpStreamAsFile;
  pFuncs->writeready    = jtpWriteReady;
  pFuncs->write         = jtpWrite;
  pFuncs->print         = jtpPrint;
  pFuncs->event         = jtpHandleEvent;
  pFuncs->urlnotify     = jtpURLNotify;
  pFuncs->getvalue      = jtpGetValue;
  pFuncs->setvalue      = NULL;

  LOG("NP_Initialize ok browser version=%d size=%u",
      (int)(bFuncs->version >> 8), (unsigned)bFuncs->size);
  return NPERR_NO_ERROR;
}

NP_EXPORT(NPError)
NP_Shutdown(void)
{
  LOG("NP_Shutdown");
  return NPERR_NO_ERROR;
}
