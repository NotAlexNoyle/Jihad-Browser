/* @@@LICENSE
*
*      Copyright (c) 2026 NotAlexNoyle.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
LICENSE@@@ */

/*
 * Minimal, ABI-compatible declaration of the webOS "Piranha" 2D graphics surface
 * (PGSurface) and its ref-counted base (PGShared) — declaring ONLY what the Jihad
 * BrowserAdapter needs. The real classes ship with webOS (Palm/HP) and are implemented
 * in the device's libWebKitLuna.so; the adapter is dlopen'd into the WebKit process.
 *
 * This is NOT a copy of Palm's header or of the Atlas project — it is an independent
 * reconstruction of the public ABI so the adapter's symbols resolve correctly at LOAD
 * time (the link is intentionally NOT --no-undefined). Two ABI facts are load-bearing:
 *   - `wrap`/`createFromPNGFile` are real out-of-line members of PGSurface (resolved from
 *     libWebKitLuna at load); their parameter types fix the Itanium mangling.
 *   - `addRef`/`releaseRef` are INLINE members of the PGShared BASE (the standard COM-style
 *     retain/release idiom) — they have NO exported symbol, so they MUST be reproduced inline
 *     here. Declaring releaseRef() as a PGSurface member instead created a bogus
 *     `_ZN9PGSurface10releaseRefEv` undefined reference that RTLD_NOW cannot resolve, which
 *     would block the whole plugin from loading (Codex review 2026-07-26, Finding 1).
 * Verify against the device lib's exported symbols with the dlopen_probe (RTLD_NOW).
 *
 * See NOTICE for the rotation-aware compositing path ported from Atlas Browser.
 */
#ifndef JIHAD_PGSURFACE_H
#define JIHAD_PGSURFACE_H

#include <stdint.h>

#ifndef PGEXPORT
#define PGEXPORT
#endif

/* Ref-counted base. addRef/releaseRef are inline (match the device's inline definitions so
 * the compiler emits no undefined symbol for them). A wrapper starts at refcount 1 and frees
 * itself at 0; the virtual dtor routes `delete this` to the real subclass dtor via the vtable
 * that libWebKitLuna installed on the object. */
class PGEXPORT PGShared {
public:
    PGShared() : m_refCount(1) {}
    void addRef()          { ++m_refCount; }
    void releaseRef()      { if (--m_refCount == 0) delete this; }
    int  refCount() const  { return m_refCount; }
protected:
    virtual ~PGShared() {}
private:
    int m_refCount;
};

class PGEXPORT PGSurface : public PGShared {
public:
    /* Zero-copy wrap of an existing 32-bit raster (rowBytes == width*4). hasAlpha=true for our
     * ARGB32 offscreen. Returns a new reference (count 1); releaseRef() it after the blit.
     * (hasAlpha defaults false in the SDK; the adapter passes true explicitly.) */
    static PGSurface* wrap(uint32_t width, uint32_t height,
                           const unsigned char* pixelData, bool hasAlpha = false);

    /* Load a PGSurface from a PNG file (used by the plugin-spotlight path). */
    static PGSurface* createFromPNGFile(const char* filePath);

    /* releaseRef()/addRef() are inherited (inline) from PGShared — do NOT redeclare them here
     * or the adapter links an unresolvable PGSurface::releaseRef symbol. */
};

#endif /* JIHAD_PGSURFACE_H */
