/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Jihad Browser — the engine calls whose argument lists UXP changes between releases, each in one
 * place (docs/ENGINE.md §4.2, cavekit-engine-update.md R3).
 *
 * nsIDOMWindowUtils' event and hit-test methods, nsIWebNavigation::LoadURIWithOptions and
 * nsIPresShell::ResizeReflowIgnoreOverride take long positional argument lists, several of them
 * optional with a count ([optional_argc]). When an engine update adds, removes or reorders one,
 * the compiler may not say so (a bool where a float was, a count off by one), and every call site
 * would need the same edit. Here each argument is named, the values the daemon always passes are
 * set once, and build/engine/check-engine-api.py checks the IDL against what these wrappers expect.
 */
#ifndef JIHAD_ENGINE_CALLS_H
#define JIHAD_ENGINE_CALLS_H

#include "nsCOMPtr.h"
#include "nsIDOMElement.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMWindowUtils.h"
#include "nsIWebNavigation.h"

namespace engine_calls {

// A mouse event dispatched through the pres shell, as a tap is (sendMouseEventToWindow: not through
// the widget). The input source is left unknown, nothing is marked synthesized, and every optional
// argument is given, which [optional_argc] says with the count 6.
inline nsresult MouseEventToWindow(nsIDOMWindowUtils* u, const nsAString& type, float x, float y,
                                   int32_t button, int32_t clickCount, int32_t modifiers,
                                   int32_t buttons) {
  return u->SendMouseEventToWindow(type, x, y, button, clickCount, modifiers,
                                   /* aIgnoreRootScrollFrame */ false, /* aPressure */ 0.0f,
                                   /* aInputSourceArg */ 0, /* aIsDOMEventSynthesized */ false,
                                   /* aIsWidgetEventSynthesized */ false, buttons,
                                   /* optional arguments given */ 6);
}

// A key event; *consumed says whether a handler called preventDefault.
inline nsresult KeyEvent(nsIDOMWindowUtils* u, const nsAString& type, int32_t keyCode,
                         int32_t charCode, int32_t modifiers, bool* consumed) {
  return u->SendKeyEvent(type, keyCode, charCode, modifiers, /* aAdditionalFlags */ 0, consumed);
}

// A touch event of `count` points, through the pres shell; *consumed as for KeyEvent.
inline nsresult TouchEventToWindow(nsIDOMWindowUtils* u, const nsAString& type, uint32_t* ids,
                                   int32_t* xs, int32_t* ys, uint32_t* radiiX, uint32_t* radiiY,
                                   float* angles, float* forces, uint32_t count, int32_t modifiers,
                                   bool* consumed) {
  return u->SendTouchEventToWindow(type, ids, xs, ys, radiiX, radiiY, angles, forces, count,
                                   modifiers, /* aIgnoreRootScrollFrame */ false, consumed);
}

// The element at a point (CSS pixels of the window), layout flushed first.
inline already_AddRefed<nsIDOMElement> ElementAt(nsIDOMWindowUtils* u, float x, float y) {
  nsCOMPtr<nsIDOMElement> el;
  u->ElementFromPoint(x, y, /* aIgnoreRootScrollFrame */ false, /* aFlushLayout */ true,
                      getter_AddRefs(el));
  return el.forget();
}

// The nodes within `pad` CSS pixels of a point, in every direction, layout flushed first.
inline already_AddRefed<nsIDOMNodeList> NodesAround(nsIDOMWindowUtils* u, float x, float y,
                                                    float pad) {
  nsCOMPtr<nsIDOMNodeList> nodes;
  u->NodesFromRect(x, y, /* top */ pad, /* right */ pad, /* bottom */ pad, /* left */ pad,
                   /* aIgnoreRootScrollFrame */ false, /* aFlushLayout */ true,
                   getter_AddRefs(nodes));
  return nodes.forget();
}

// The page's <meta name=viewport>, as the engine resolves it for a display of w x h.
inline nsresult ViewportInfo(nsIDOMWindowUtils* u, uint32_t displayWidth, uint32_t displayHeight,
                             double* defaultZoom, bool* allowZoom, double* minZoom,
                             double* maxZoom, uint32_t* width, uint32_t* height, bool* autoSize) {
  return u->GetViewportInfo(displayWidth, displayHeight, defaultZoom, allowZoom, minZoom, maxZoom,
                            width, height, autoSize);
}

// A load with a referrer (or none) and the principal that triggers it; no post data, no extra
// headers, no base URI.
inline nsresult LoadURI(nsIWebNavigation* nav, const char16_t* uri, uint32_t loadFlags,
                        nsIURI* referrer, uint32_t referrerPolicy, nsIPrincipal* triggering) {
  return nav->LoadURIWithOptions(uri, loadFlags, referrer, referrerPolicy, /* aPostData */ nullptr,
                                 /* aHeaders */ nullptr, /* aBaseURI */ nullptr, triggering);
}

// Lay the document out for a viewport of w x h CSS pixels, whatever size override is set.
// nsIPresShell takes app units: 60 to a CSS pixel (nsPresContext.h, which says so, cannot be
// included from the frozen-API daemon). A template, so this header needs no internal one.
template <class PresShell>
inline void ResizeReflow(PresShell* shell, int width, int height) {
  shell->ResizeReflowIgnoreOverride((nscoord)width * 60, (nscoord)height * 60,
                                    /* aOldWidth */ 0, /* aOldHeight */ 0);
}

}  // namespace engine_calls

#endif  // JIHAD_ENGINE_CALLS_H
