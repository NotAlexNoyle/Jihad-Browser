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
 * Minimal declaration of the webOS "Piranha" 2D graphics context (PGContext),
 * declaring ONLY the bitblt overload the Jihad BrowserAdapter calls. The real class
 * ships with webOS (HP) as <PGContext.h> and is implemented in libWebKitLuna.so; the
 * WebKit compositor hands the plugin a PGContext* that already carries the card's
 * rotation/scale transform, so a bitblt THROUGH it maps our logical offscreen to the
 * physical screen correctly in BOTH portrait and landscape. Symbols resolve at LOAD
 * time (the link is intentionally NOT --no-undefined).
 *
 * This header is NOT copied from HP's SDK or from the Atlas project — it is a
 * reconstruction of the public ABI from luna-sysmgr's bitblt() calls and the Atlas
 * BrowserAdapter's 8-coordinate src->dst bitblt. See NOTICE.
 */
#ifndef JIHAD_PGCONTEXT_H
#define JIHAD_PGCONTEXT_H

class PGSurface;

class PGContext {
public:
    /* Blit the source rectangle (srcLeft,srcTop)-(srcRight,srcBottom) of `src` into the
     * destination rectangle (dstLeft,dstTop)-(dstRight,dstBottom) of this context. The
     * context's current transform (rotation/scale set by the compositor for the card's
     * orientation) is applied, so no manual stride/rotation juggling is needed. Sizes may
     * differ (the blit scales); an identity 1:1 rect pair is a straight transformed copy. */
    void bitblt(PGSurface* src,
                int srcLeft, int srcTop, int srcRight, int srcBottom,
                int dstLeft, int dstTop, int dstRight, int dstBottom);
};

#endif /* JIHAD_PGCONTEXT_H */
