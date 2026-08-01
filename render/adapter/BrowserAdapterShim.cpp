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
 *   this variant's MIME type), and it contains no browser logic. All the real work
 *   lives in BrowserAdapterImpl.so, which the shim dlopen()s from a root-owned rootfs
 *   path on the FIRST card instance and reloads (fresh copy) whenever the on-disk impl
 *   changes and no card is live. Result: reopening the card always runs the current
 *   adapter build — no LunaSysMgr restart, no reboot.
 *
 * Contract: the impl exports the standard Linux NPAPI entry points (NP_Initialize fills
 * an NPPluginFuncs table with its NPP_* pointers; NP_Shutdown; NP_GetMIMEDescription).
 * The shim keeps that table (sImpl) and forwards each NPP_* call straight through, so
 * the BrowserAdapter<->BrowserServer YAP contract is byte-identical either way.
 *
 * THREE VARIANTS, ONE SOURCE (cavekit-device-build.md R7, plan-variant-identity.md):
 *   The Enyo, Mochi, and Mojo packages are three fully independent browsers — no
 *   co-owned component, no refcount. Each therefore needs its OWN MIME type, shim
 *   filename, and impl path, or installing/removing one would silently hijack or break
 *   another. Rather than fork this file three ways (three copies to keep in sync = the
 *   exact drift the 2026-07-29 OE review flagged as finding #5, "adapter shim impl-path
 *   hard-coded to the Enyo appid"), the identity comes in on the compiler command line.
 *   See the JIHAD_* macro block below for the contract the build scripts must honor.
 */
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>

#include "npapi.h"
#include "npupp.h"

#define JIHAD_VISIBLE __attribute__((visibility("default")))

// ---- BUILD-TIME VARIANT IDENTITY ----------------------------------------------------
//
// The build scripts pass the variant in with -D (see build/webos-oe/build-adapter-*.sh):
//
//   JIHAD_VARIANT    "enyo" | "mochi" | "mojo"   — the variant token V from the identity table
//   JIHAD_MIME       the BARE NPAPI MIME type, e.g. "application/x-jihad-browser".
//                    NOT the description: NP_GetMIMEDescription appends the "::;" extension/
//                    description fields itself, so this one macro is the SAME value the impl
//                    (BrowserAdapter.cpp AdapterGetMIMEDescription) and the front-end's WebView
//                    plugin-type override use — one string, one place to get it wrong.
//   JIHAD_IMPL_PATH  absolute path of this variant's BrowserAdapterImpl.so on the ROOTFS.
//   JIHAD_STATE_DIR  this variant's root-owned runtime state dir (R8: never under /media).
//
// Why an all-or-nothing group check instead of independent defaults: a build that defines
// JIHAD_MIME but forgets JIHAD_IMPL_PATH would register as Mochi and then dlopen the ENYO
// impl — a card silently served by another package's adapter, which is exactly the
// cross-variant hijack R7 forbids, and it would look perfectly healthy in the logs. That
// failure must be a compile error, not a field report. An undefined build still compiles
// (defaults to enyo, the one deployment known to work) so a bare `g++ BrowserAdapterShim.cpp`
// syntax check needs no -D soup.
#if defined(JIHAD_VARIANT) || defined(JIHAD_MIME) || defined(JIHAD_IMPL_PATH) || defined(JIHAD_STATE_DIR)
#  if !defined(JIHAD_VARIANT) || !defined(JIHAD_MIME)
#    error "Jihad variant build is inconsistent: define BOTH -DJIHAD_VARIANT and -DJIHAD_MIME (or neither, for the enyo default). See context/plans/plan-variant-identity.md."
#  endif
#else
#  define JIHAD_VARIANT   "enyo"
#  define JIHAD_MIME      "application/x-jihad-browser"
#endif
// Layout macros default FROM the variant token by literal concatenation, so the path and the
// identity can never disagree — the only way to get a mismatched pair is to override these
// explicitly, which is deliberate. Paths per plan-variant-identity.md's identity table.
#ifndef JIHAD_IMPL_PATH
#  define JIHAD_IMPL_PATH "/usr/lib/jihad/" JIHAD_VARIANT "/BrowserAdapterImpl.so"
#endif
#ifndef JIHAD_STATE_DIR
#  define JIHAD_STATE_DIR "/var/palm/jihad/" JIHAD_VARIANT
#endif
// gcc 4.3.3 / C++98: no static_assert, so use the negative-array-size idiom. This catches the
// one class of mistake the preprocessor can see — an EMPTY macro (`-DJIHAD_VARIANT=""`, or a
// shell variable that expanded to nothing in the build script), which would otherwise yield a
// plausible-looking "/usr/lib/jihad//BrowserAdapterImpl.so" and a MIME of "::;".
typedef char jihadVariantMustBeNonEmpty[(sizeof(JIHAD_VARIANT) > 1) ? 1 : -1];
typedef char jihadMimeMustBeNonEmpty[(sizeof(JIHAD_MIME) > 1) ? 1 : -1];
typedef char jihadImplPathMustBeNonEmpty[(sizeof(JIHAD_IMPL_PATH) > 1) ? 1 : -1];
typedef char jihadStateDirMustBeNonEmpty[(sizeof(JIHAD_STATE_DIR) > 1) ? 1 : -1];

// THIS VARIANT's impl, on the root-owned rootfs (deployed by this package's postinst in a
// rootfs-rw window, like the shim itself). Loading a native .so into privileged LunaSysMgr from
// a world-writable path is arbitrary code execution (Jihad review F-161/F-180), so before dlopen
// the opened file MUST be a regular file, root-owned, and not group/world-writable (fail closed).
// Not under /usr/lib/BrowserPlugins (LunaSysMgr scans that for NPAPI plugins at boot), and NOT in
// the app bundle: cryptofs reports every file as 0777, so an app-bundle impl can never pass the
// trust check (plan-variant-identity.md rule 5). The former app-bundle fallback is gone for that
// reason — it was dead code that could only ever log "REJECT untrusted impl".
//   DEV builds (compile -DJIHAD_DEV_ADAPTER — never shipped) additionally honor $JIHAD_ADAPTER_IMPL
//   or a drop in this variant's state dir, and skip the root-owner check.
static const char* kImplSystem = JIHAD_IMPL_PATH;
#ifdef JIHAD_DEV_ADAPTER
// Was /media/internal/jihad/BrowserAdapterImpl.so. R8 forbids the shim touching /media/internal
// (the user's vfat USB volume), so the dev drop moved into the variant's own state dir too.
static const char* kImplDevPath = JIHAD_STATE_DIR "/BrowserAdapterImpl.so";
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

// Diagnostic channel for the load path (why an impl was rejected / which one won). It USED to
// write to /media/internal/jihad/shim.log — the user's vfat USB mass-storage volume, which R8
// puts off limits for app internals — so it now lives in this variant's root-owned state dir.
// Two properties, both deliberate:
//   - OFF BY DEFAULT (plan-variant-identity.md rule 3): nothing is written unless the operator
//     drops the marker file JIHAD_STATE_DIR "/shimlog". Same gate convention as the adapter's
//     paint log (Jihad review F-166). The marker is re-checked per call rather than cached: this
//     is called a handful of times per impl load, never per frame, so there is no cost to pay for
//     the convenience of enabling it without restarting LunaSysMgr.
//   - NEVER CREATES ANYTHING but the log file itself, and never a world-writable one: no mkdir
//     (the packaging owns the state dir; R8 forbids the shim widening it), O_NOFOLLOW so a
//     planted symlink cannot redirect a root-privileged write, and mode 0600 rather than
//     fopen()'s umask-dependent 0666.
// If the dir does not exist the marker check fails and this is a silent no-op.
//
// SYSLOG IS UNCONDITIONAL, and that is the point (2026-08-01 device triage). This code runs
// inside WebAppMgr — a host process whose uid is not ours to assume — and JIHAD_STATE_DIR is
// root-owned 0755, so if WebAppMgr is not root the open() below fails and EVERY load failure
// becomes invisible. That is exactly the hole that made "no shim.log" unreadable evidence: it
// could mean NPP_New never ran, or that it ran and could not tell anyone. syslog() needs no
// filesystem permission and lands in /var/log/messages, so the reason a card came up without a
// plugin is always recoverable. Volume is a handful of lines per impl load (never per frame).
static void shimLog(const char* fmt, const char* a)
{
    syslog(LOG_WARNING, fmt, a);
    if (access(JIHAD_STATE_DIR "/shimlog", F_OK) != 0) return;
    int fd = open(JIHAD_STATE_DIR "/shim.log",
                  O_WRONLY | O_APPEND | O_CREAT | O_NOFOLLOW, S_IRUSR | S_IWUSR);
    if (fd < 0) return;
    FILE* f = fdopen(fd, "a");
    if (f) { fprintf(f, fmt, a); fclose(f); }
    else   { close(fd); }
}

// Resolve the impl source: THIS variant's rootfs impl (dev builds also allow an override).
static const char* implSourcePath()
{
#ifdef JIHAD_DEV_ADAPTER
    const char* env = getenv("JIHAD_ADAPTER_IMPL");
    if (env && *env && access(env, R_OK) == 0) return env;
    if (access(kImplDevPath, R_OK) == 0) return kImplDevPath;
#endif
    // Exactly one production source. R7: this variant loads its OWN impl and nothing else — a
    // fallback to another path is how a Mochi card ends up driven by the Enyo adapter.
    if (access(kImplSystem, R_OK) == 0) return kImplSystem;
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
    // Variant in the template so a `ls /tmp` (or an lsof/maps dump) during on-device triage with
    // all three packages installed says WHICH adapter is mid-load. mkstemp already guarantees
    // uniqueness; this is purely legibility.
    char tmpl[] = "/tmp/jihad-" JIHAD_VARIANT "-adapter-XXXXXX";
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
    if (!ok)      shimLog("[shim] copy to /tmp failed for: %s\n", src);
    else if (!h)  shimLog("[shim] dlopen /tmp copy FAILED: %s\n", dlerror());
    return h;
}

// Fallback when the /tmp copy cannot be loaded. The copy exists ONLY to defeat glibc's dlopen
// path-dedup so a REPLACED impl is picked up without restarting LunaSysMgr; it is a
// hot-reload convenience, never a correctness requirement. But it silently makes the FIRST load
// depend on /tmp accepting an executable mapping and on the temp copy resolving the impl's
// NEEDED libraries — and if either is false, the card comes up with an <object> and no plugin,
// which is indistinguishable from "the MIME is not registered". So: if the copy path fails, load
// the impl where it actually lives. The trust check still runs first, on the open fd, exactly as
// before; the residual difference is that dlopen re-opens by path (a TOCTOU window that requires
// root to exploit, in a root-owned 0755 directory on the read-only rootfs — strictly weaker than
// the copy path, which is why it is the fallback and not the default). Hot reload degrades to
// "unchanged until LunaSysMgr restarts" on this path, and says so in the log.
static void* dlopenInPlace(const char* src)
{
    int in = open(src, O_RDONLY);
    if (in < 0) { shimLog("[shim] open src fail: %s\n", src); return 0; }
    if (!fdIsTrusted(in)) { shimLog("[shim] REJECT untrusted impl: %s\n", src); close(in); return 0; }
    close(in);
    void* h = dlopen(src, RTLD_NOW | RTLD_LOCAL);
    if (!h) shimLog("[shim] dlopen in place FAILED: %s\n", dlerror());
    else    shimLog("[shim] loaded IN PLACE (no /tmp copy; hot reload disabled): %s\n", src);
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
    if (!h) h = dlopenInPlace(src);
    if (!h) return sHandle ? NPERR_NO_ERROR : NPERR_MODULE_LOAD_FAILED_ERROR;

    typedef NPError (*InitFn)(NPNetscapeFuncs*, NPPluginFuncs*);
    InitFn init = (InitFn) dlsym(h, "NP_Initialize");
    if (!init) shimLog("[shim] impl has no NP_Initialize: %s\n", src);
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
    // Logged on EVERY instance creation, unconditionally (see shimLog). Without it, "the card has
    // an <object> but no plugin" cannot be told apart from "WebKit never asked us for an
    // instance" — the two have identical symptoms in the front-end (Enyo just queues
    // callBrowserAdapter forever) and only this line separates them.
    shimLog("[shim] NPP_New for %s\n", type ? type : "(null mime)");
    NPError e = loadImpl();
    if (e != NPERR_NO_ERROR) { shimLog("[shim] NPP_New ABORT: no usable impl%s\n", ""); return e; }
    if (!sImpl.newp) { shimLog("[shim] NPP_New ABORT: impl exports no NPP_New%s\n", ""); return NPERR_GENERIC_ERROR; }
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
    // "<mime>:<extensions>:<description>" — no file extensions or description, as before.
    // JIHAD_MIME is the bare type so the impl and the front-end's WebView plugin-type override
    // can share the exact same macro value (R7: each variant answers for its MIME and no other).
    return const_cast<char*>(JIHAD_MIME "::;");
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
    // Variant-stamped: with all three packages installed, LunaSysMgr's plugin registry and
    // about:plugins otherwise show three identical entries, and there is no way to tell which
    // .so answered for a MIME type. The description carries the resolved impl path too, so a
    // mis-parametrized build (right MIME, wrong impl) is visible without a disassembler.
    static char name[] = "Jihad Browser Adapter (" JIHAD_VARIANT ")";
    static char desc[] = "Jihad/Goanna browser adapter shim — variant " JIHAD_VARIANT
                         ", MIME " JIHAD_MIME ", impl " JIHAD_IMPL_PATH;
    switch (variable) {
    case NPPVpluginNameString:        *static_cast<char**>(value) = name; return NPERR_NO_ERROR;
    case NPPVpluginDescriptionString: *static_cast<char**>(value) = desc; return NPERR_NO_ERROR;
    default:                          return NPERR_INVALID_PARAM;
    }
}
