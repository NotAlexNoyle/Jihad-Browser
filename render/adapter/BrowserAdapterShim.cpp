/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — BrowserAdapterShim: a thin, STABLE NPAPI plugin.
 *
 * Why this exists:
 *   webOS WebKit (LunaSysMgr) scans /usr/lib/BrowserPlugins at boot and dlopen()s
 *   each plugin ONCE for the lifetime of the LunaSysMgr process. Card open/close
 *   (NPP_New / NPP_Destroy) never reloads the .so. That means shipping a new adapter
 *   build required overwriting the system plugin and REBOOTING — and made the adapter
 *   a system-level component, not something the app owns.
 *
 *   This shim breaks that: it is the plugin LunaSysMgr caches (registered once for
 *   MIME application/x-jihad-browser), and it contains no browser logic. All the real
 *   work lives in BrowserAdapterImpl.so, which the shim dlopen()s from an app-owned
 *   path on the FIRST card instance and dlclose()s when the LAST card closes. Because
 *   the impl is unloaded when idle, the very next card open dlopen()s whatever build is
 *   currently on disk. Result: reopening the card always relaunches the current adapter
 *   — no LunaSysMgr restart, no reboot, and the adapter is self-contained in the app.
 *
 * Contract: the impl exports the standard Linux NPAPI entry points (NP_Initialize fills
 * an NPPluginFuncs table with its NPP_* pointers; NP_Shutdown; NP_GetMIMEDescription).
 * The shim keeps that table (sImpl) and forwards each NPP_* call straight through, so
 * the BrowserAdapter<->BrowserServer YAP contract is byte-identical either way.
 */
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "npapi.h"
#include "npupp.h"

#define JIHAD_VISIBLE __attribute__((visibility("default")))

// Candidate locations for the impl, tried in order. The app installs it in its own
// bundle (self-contained); the /media/internal/jihad path is the dev/staging drop.
// JIHAD_ADAPTER_IMPL overrides everything for ad-hoc testing.
static const char* kImplFallbacks[] = {
    "/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/BrowserAdapterImpl.so",
    "/media/internal/jihad/BrowserAdapterImpl.so",
    0
};

static NPNetscapeFuncs sBrowser;     // browser funcs, handed to the impl on load
static NPPluginFuncs   sImpl;        // impl's NPP_* table (valid only while sHandle != 0)
static void*           sHandle = 0;  // dlopen handle for the currently-loaded impl
static int             sInstances = 0;

static void shimLog(const char* fmt, const char* a)
{
    FILE* f = fopen("/media/internal/jihad/shim.log", "a");
    if (f) { fprintf(f, fmt, a); fclose(f); }
}

// Try to load + initialize the impl at one path. Returns true on success (sHandle set).
static bool tryLoad(const char* path)
{
    if (!path || !*path)
        return false;

    void* h = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!h) { shimLog("[shim] dlopen fail: %s\n", dlerror()); return false; }

    typedef NPError (*InitFn)(NPNetscapeFuncs*, NPPluginFuncs*);
    InitFn init = (InitFn) dlsym(h, "NP_Initialize");
    if (!init) { shimLog("[shim] no NP_Initialize in %s\n", path); dlclose(h); return false; }

    memset(&sImpl, 0, sizeof(sImpl));
    sImpl.size = sizeof(sImpl);
    if (init(&sBrowser, &sImpl) != NPERR_NO_ERROR) {
        shimLog("[shim] impl NP_Initialize err on %s\n", path);
        dlclose(h);
        return false;
    }
    sHandle = h;
    shimLog("[shim] loaded impl: %s\n", path);
    return true;
}

// dlopen the impl (once) and pull its NPP_* function table via its NP_Initialize.
// JIHAD_ADAPTER_IMPL (if set) is tried first, then the fallback bundle paths.
static NPError loadImpl()
{
    if (sHandle)
        return NPERR_NO_ERROR;

    if (tryLoad(getenv("JIHAD_ADAPTER_IMPL")))
        return NPERR_NO_ERROR;
    for (int i = 0; kImplFallbacks[i]; ++i)
        if (tryLoad(kImplFallbacks[i]))
            return NPERR_NO_ERROR;

    shimLog("[shim] NO impl found%s\n", "");
    return NPERR_MODULE_LOAD_FAILED_ERROR;
}

// Unload the impl so the next card open picks up whatever build is on disk.
static void unloadImpl()
{
    if (!sHandle)
        return;
    typedef NPError (*ShutFn)(void);
    ShutFn sh = (ShutFn) dlsym(sHandle, "NP_Shutdown");
    if (sh) sh();
    dlclose(sHandle);
    sHandle = 0;
    memset(&sImpl, 0, sizeof(sImpl));
    shimLog("[shim] unloaded impl%s\n", "");
}

// ---- per-instance forwarders (stable addresses handed to LunaSysMgr) ----------------

static NPError sNewp(NPMIMEType type, NPP inst, uint16_t mode, int16_t argc,
                     char* argn[], char* argv[], NPSavedData* saved)
{
    NPError e = loadImpl();
    if (e != NPERR_NO_ERROR) return e;
    if (!sImpl.newp) return NPERR_GENERIC_ERROR;
    e = sImpl.newp(type, inst, mode, argc, argn, argv, saved);
    if (e == NPERR_NO_ERROR) ++sInstances;
    return e;
}

static NPError sDestroy(NPP inst, NPSavedData** save)
{
    NPError e = sImpl.destroy ? sImpl.destroy(inst, save) : NPERR_NO_ERROR;
    if (--sInstances <= 0) { sInstances = 0; unloadImpl(); }  // last card closed -> reload next time
    return e;
}

static NPError sSetWindow(NPP inst, NPWindow* window)
{ return sImpl.setwindow ? sImpl.setwindow(inst, window) : NPERR_GENERIC_ERROR; }

static NPError sNewStream(NPP inst, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype)
{ return sImpl.newstream ? sImpl.newstream(inst, type, stream, seekable, stype) : NPERR_GENERIC_ERROR; }

static NPError sDestroyStream(NPP inst, NPStream* stream, NPReason reason)
{ return sImpl.destroystream ? sImpl.destroystream(inst, stream, reason) : NPERR_GENERIC_ERROR; }

static void sStreamAsFile(NPP inst, NPStream* stream, const char* fname)
{ if (sImpl.asfile) sImpl.asfile(inst, stream, fname); }

static int32_t sWriteReady(NPP inst, NPStream* stream)
{ return sImpl.writeready ? sImpl.writeready(inst, stream) : 0; }

static int32_t sWrite(NPP inst, NPStream* stream, int32_t offset, int32_t len, void* buffer)
{ return sImpl.write ? sImpl.write(inst, stream, offset, len, buffer) : -1; }

static void sPrint(NPP inst, NPPrint* platformPrint)
{ if (sImpl.print) sImpl.print(inst, platformPrint); }

static int16_t sHandleEvent(NPP inst, void* event)
{ return sImpl.event ? sImpl.event(inst, event) : 0; }

static void sUrlNotify(NPP inst, const char* url, NPReason reason, void* notifyData)
{ if (sImpl.urlnotify) sImpl.urlnotify(inst, url, reason, notifyData); }

static NPError sGetValue(NPP inst, NPPVariable variable, void* ret)
{ return sImpl.getvalue ? sImpl.getvalue(inst, variable, ret) : NPERR_GENERIC_ERROR; }

static NPError sSetValue(NPP inst, NPNVariable variable, void* ret)
{ return sImpl.setvalue ? sImpl.setvalue(inst, variable, ret) : NPERR_GENERIC_ERROR; }

// ---- NPAPI plugin entry points (exported; this is what LunaSysMgr caches) ------------

extern "C" JIHAD_VISIBLE
char* NP_GetMIMEDescription()
{
    return const_cast<char*>("application/x-jihad-browser::;");
}

extern "C" JIHAD_VISIBLE
NPError NP_Initialize(NPNetscapeFuncs* browserFuncs, NPPluginFuncs* pluginFuncs)
{
    memcpy(&sBrowser, browserFuncs, sizeof(NPNetscapeFuncs));

    // Fill the plugin table with the shim's stable forwarders. These addresses never
    // change for the LunaSysMgr process lifetime, even as the impl is loaded/unloaded.
    pluginFuncs->newp          = sNewp;
    pluginFuncs->destroy       = sDestroy;
    pluginFuncs->setwindow     = sSetWindow;
    pluginFuncs->newstream     = sNewStream;
    pluginFuncs->destroystream = sDestroyStream;
    pluginFuncs->asfile        = sStreamAsFile;
    pluginFuncs->writeready    = sWriteReady;
    pluginFuncs->write         = sWrite;
    pluginFuncs->print         = sPrint;
    pluginFuncs->event         = sHandleEvent;
    pluginFuncs->urlnotify     = sUrlNotify;
    pluginFuncs->javaClass     = 0;
    pluginFuncs->getvalue      = sGetValue;
    pluginFuncs->setvalue      = sSetValue;
    return NPERR_NO_ERROR;
}

extern "C" JIHAD_VISIBLE
NPError NP_Shutdown(void)
{
    unloadImpl();
    return NPERR_NO_ERROR;
}

extern "C" JIHAD_VISIBLE
NPError NP_GetValue(void* /*future*/, NPPVariable variable, void* value)
{
    static char name[] = "Jihad Browser Adapter";
    static char desc[] = "Jihad/Goanna browser adapter (shim)";
    switch (variable) {
    case NPPVpluginNameString:        *static_cast<char**>(value) = name; return NPERR_NO_ERROR;
    case NPPVpluginDescriptionString: *static_cast<char**>(value) = desc; return NPERR_NO_ERROR;
    default:                          return NPERR_INVALID_PARAM;
    }
}
