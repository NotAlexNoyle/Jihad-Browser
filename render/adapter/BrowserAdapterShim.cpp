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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "npapi.h"
#include "npupp.h"

#define JIHAD_VISIBLE __attribute__((visibility("default")))

// The impl ships INSIDE the app bundle (installed by the signed package, root-owned + not writable).
// Loading a native .so into privileged LunaSysMgr from a world-writable path is arbitrary code
// execution (Jihad review F-161/F-180), so:
//   - PRODUCTION (default): the trusted app-bundle path is the ONLY source, and before dlopen the
//     opened file MUST be a regular file, root-owned, and not group/world-writable (fail closed).
//   - DEV builds (compile -DJIHAD_DEV_ADAPTER — never shipped): additionally honor JIHAD_ADAPTER_IMPL
//     or a /media/internal drop, and skip the root-owner check (that partition is vfat: no real perms).
// This replaces the earlier runtime /media/internal/jihad/DEV marker, which was itself in the
// world-writable location it was meant to distrust.
static const char* kImplTrusted =
    "/media/cryptofs/apps/usr/palm/applications/net.riverstonerelay.jihad-browser/BrowserAdapterImpl.so";
#ifdef JIHAD_DEV_ADAPTER
static const char* kImplDevPath = "/media/internal/jihad/BrowserAdapterImpl.so";
#endif

static NPNetscapeFuncs sBrowser;     // browser funcs, handed to the impl on load
static NPPluginFuncs   sImpl;        // current impl's NPP_* table (valid once sHandle != 0)
static void*           sHandle = 0;  // dlopen handle for the currently-loaded impl (never dlclose'd)
static int             sInstances = 0;
// Identity of the impl file at last load. Reload when ANY of these change — mtime alone is
// second-resolution and package tools can preserve timestamps, so a same-tick replacement would be
// missed (Jihad review F-185). sLoadedSize = -1 also acts as a "force reload" sentinel.
static dev_t  sLoadedDev   = 0;
static ino_t  sLoadedIno   = 0;
static off_t  sLoadedSize  = -1;
static time_t sLoadedMtime = 0;
static long   sLoadedMtimeNs = -1;

static void shimLog(const char* fmt, const char* a)
{
    FILE* f = fopen("/media/internal/jihad/shim.log", "a");
    if (f) { fprintf(f, fmt, a); fclose(f); }
}

// Resolve the impl source: the trusted app-bundle path (dev builds also allow an override).
static const char* implSourcePath()
{
#ifdef JIHAD_DEV_ADAPTER
    const char* env = getenv("JIHAD_ADAPTER_IMPL");
    if (env && *env && access(env, R_OK) == 0) return env;
    if (access(kImplDevPath, R_OK) == 0) return kImplDevPath;
#endif
    if (access(kImplTrusted, R_OK) == 0) return kImplTrusted;
    return 0;
}

// Reject an impl that is not a regular file, is group/world-writable, or (production) not root-owned
// — verified on the OPEN fd so the check applies to the exact bytes we are about to dlopen (F-180).
static bool fdIsTrusted(int fd)
{
    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) return false;
    if (st.st_mode & (S_IWGRP | S_IWOTH)) return false;
#ifndef JIHAD_DEV_ADAPTER
    if (st.st_uid != 0) return false;
#endif
    return true;
}

// dlopen a FRESH mapping of `src`. glibc dlopen dedups by path, so reopening the same path returns
// the cached handle and would NOT pick up an updated build; copying to a unique temp forces a new
// mapping. The temp is created 0600 (owner-only) and unlinked right after dlopen — the mapping
// persists, the on-disk name is gone, which also closes the copy→dlopen substitution window.
static void* dlopenFreshCopy(const char* src)
{
    int in = open(src, O_RDONLY);
    if (in < 0) { shimLog("[shim] open src fail: %s\n", src); return 0; }
    if (!fdIsTrusted(in)) { shimLog("[shim] REJECT untrusted impl: %s\n", src); close(in); return 0; }
    char tmpl[] = "/tmp/jihad-adapter-XXXXXX";
    int out = mkstemp(tmpl);
    if (out < 0) { shimLog("[shim] mkstemp fail%s\n", ""); close(in); return 0; }
    char buf[65536]; ssize_t n; bool ok = true;
    while ((n = read(in, buf, sizeof buf)) > 0) {
        for (ssize_t off = 0; off < n; ) {
            ssize_t w = write(out, buf + off, (size_t)(n - off));
            if (w <= 0) { ok = false; break; }
            off += w;
        }
        if (!ok) break;
    }
    if (n < 0) ok = false;
    close(in); close(out);
    void* h = ok ? dlopen(tmpl, RTLD_NOW | RTLD_LOCAL) : 0;
    unlink(tmpl);
    if (ok && !h) shimLog("[shim] dlopen copy fail: %s\n", dlerror());
    return h;
}

// Ensure the impl is loaded and current. Loads on first use, and RELOADS when the on-disk impl's
// mtime changed AND no card is currently open (never swap the impl out from under a live NPP
// instance). NEVER dlclose the previous handle: the browser may retain scriptable NPObjects and
// the impl may leave GSource timers on LunaSysMgr's mainloop past NPP_Destroy — unmapping their
// code would crash LunaSysMgr (Jihad review F-160). The previous mapping is intentionally leaked
// (bounded: one per impl update per LunaSysMgr lifetime).
static NPError loadImpl()
{
    const char* src = implSourcePath();
    if (!src) { shimLog("[shim] NO impl found%s\n", ""); return NPERR_MODULE_LOAD_FAILED_ERROR; }

    struct stat st;
    bool haveStat = (stat(src, &st) == 0);
    bool changed = !haveStat ||
                   st.st_dev != sLoadedDev || st.st_ino != sLoadedIno ||
                   st.st_size != sLoadedSize || st.st_mtime != sLoadedMtime ||
                   st.st_mtim.tv_nsec != sLoadedMtimeNs;
    if (sHandle && (!changed || sInstances > 0))
        return NPERR_NO_ERROR;   // current build already loaded, or a card is live — keep it

    void* h = dlopenFreshCopy(src);
    if (!h) return sHandle ? NPERR_NO_ERROR : NPERR_MODULE_LOAD_FAILED_ERROR;

    typedef NPError (*InitFn)(NPNetscapeFuncs*, NPPluginFuncs*);
    InitFn init = (InitFn) dlsym(h, "NP_Initialize");
    NPPluginFuncs pf; memset(&pf, 0, sizeof(pf)); pf.size = sizeof(pf);
    if (!init || init(&sBrowser, &pf) != NPERR_NO_ERROR) {
        shimLog("[shim] impl init fail: %s\n", src);
        // A failed init registered no NPObjects/GSources, so this handle is safe to unmap now —
        // dlclose it rather than leak one mapping per retry (Jihad review F-186). Any previously
        // good impl stays loaded.
        dlclose(h);
        return sHandle ? NPERR_NO_ERROR : NPERR_MODULE_LOAD_FAILED_ERROR;
    }
    sImpl = pf;              // switch to the new table (previous sHandle intentionally leaked — F-160)
    sHandle = h;
    if (haveStat) {
        sLoadedDev = st.st_dev; sLoadedIno = st.st_ino; sLoadedSize = st.st_size;
        sLoadedMtime = st.st_mtime; sLoadedMtimeNs = st.st_mtim.tv_nsec;
    }
    shimLog("[shim] loaded impl: %s\n", src);
    return NPERR_NO_ERROR;
}

// ---- per-instance forwarders (stable addresses handed to LunaSysMgr) ----------------

static NPError sNewp(NPMIMEType type, NPP inst, uint16_t mode, int16_t argc,
                     char* argn[], char* argv[], NPSavedData* saved)
{
    NPError e = loadImpl();
    if (e != NPERR_NO_ERROR) return e;
    if (!sImpl.newp) return NPERR_GENERIC_ERROR;
    e = sImpl.newp(type, inst, mode, argc, argn, argv, saved);
    if (e == NPERR_NO_ERROR)
        ++sInstances;
    else
        sLoadedSize = -1;   // failed create: don't wedge — force a reload attempt next open (F-168)
    return e;
}

static NPError sDestroy(NPP inst, NPSavedData** save)
{
    NPError e = sImpl.destroy ? sImpl.destroy(inst, save) : NPERR_NO_ERROR;
    if (sInstances > 0) --sInstances;
    // NEVER dlclose here (F-160). Reloading an updated impl happens on the next NPP_New via the
    // mtime check in loadImpl; the current mapping stays valid for any browser-retained NPObjects.
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
    // LunaSysMgr is tearing the plugin down (process exit). Do NOT dlclose the impl (F-160) —
    // the OS reclaims the mappings on exit; unmapping here could still fire retained callbacks.
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
