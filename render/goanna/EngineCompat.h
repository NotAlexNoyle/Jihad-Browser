/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// EngineCompat.h — every entry point this daemon takes from OUR OWN changes to the engine.
//
// The engine (third_party/uxp, branch jihad/uxp-<UXP_TAG>) exports a small C API that upstream
// UXP does not have: offscreen rendering, popup compositing, plugin frame pacing, NSS start-up,
// the audio stream count. They are declared here and nowhere else, so an engine update has
// one list to check (docs/ENGINE.md "The jihad_* exports"; build/engine/check-engine-api.py
// checks each one is still defined in the engine tree).
//
// REQUIRED exports are plain declarations: a libxul without one fails at load, loudly.
// OPTIONAL exports are weak: on a libxul without one the feature falls back (documented per
// symbol) instead of the daemon failing to start. Weak is safe only for a NEW symbol; a changed
// signature gets a new name (the F1 ABI lesson). Because a missing weak symbol is silent,
// ReportEngineExports() logs which optional ones the running libxul lacks, once at start-up.

#ifndef JIHAD_ENGINE_COMPAT_H
#define JIHAD_ENGINE_COMPAT_H

#include <cstdint>
#include <cstdio>

class nsIWidget;
class nsIDocShell;

extern "C" {
  // ── Offscreen rendering (engine: widget/PuppetWidget.cpp; patches 0003/0005) ──────────────
  // A memory-backed PuppetWidget (no X/GTK/native window) handed to nsIBaseWindow::InitWindow,
  // painted on demand and read back as ARGB32. Active when JIHAD_OFFSCREEN is set.
  nsIWidget* jihad_offscreen_create(int aWidth, int aHeight);                          // REQUIRED
  void jihad_offscreen_resize(nsIWidget* aWidget, int aWidth, int aHeight);            // REQUIRED
  void jihad_offscreen_paint(nsIWidget* aWidget);                                      // REQUIRED
  bool jihad_offscreen_render_document_v2(nsIWidget* aWidget, nsIDocShell* aDocShell,  // REQUIRED
                                          double aZoom, double aPanX, double aPanY);
  bool jihad_offscreen_readback(nsIWidget* aWidget, void* aDest, int aStride,          // REQUIRED
                                int aWidth, int aHeight);
  void jihad_offscreen_release(nsIWidget* aWidget);                                    // REQUIRED
  // Force PSM/NSS to construct on the main thread before the first https load (patch 0007).
  bool jihad_init_nss();                                                               // REQUIRED

  // OPTIONAL: paint a region larger than the viewport (overscan). Fallback: the caller paints
  // the viewport exactly.
  bool jihad_offscreen_render_region(nsIWidget* aWidget, nsIDocShell* aDocShell, double aZoom,
                                     double aDocX, double aDocY, void* aDest, int aStride,
                                     int aWidth, int aHeight) __attribute__((weak));
  // OPTIONAL (T-502, the GPU compositor spike, patch 0040): paint the document as layers and
  // fetch the frame the in-process compositor composes (JIHAD_GPU_COMPOSITOR in the daemon's
  // environment). Fallback: the CPU exports above.
  bool jihad_offscreen_compose(nsIWidget* aWidget, nsIDocShell* aDocShell, void* aDest,
                               int aStride, int aWidth, int aHeight) __attribute__((weak));
  // OPTIONAL (T-502): the same frame in two halves, so the main thread does not wait out the
  // compositor thread's read: start paints the layers and posts the compose + read of rows
  // [aRowLo, aRowHi) (0,0 = none asked for) joined with the rows the composite changed, and
  // returns a token (0 = failed); poll returns 1 done (aInvLo/aInvHi = the rows the composite
  // changed), 0 pending, -1 failed; forget drops a token whose buffer went away.
  uint32_t jihad_offscreen_compose_start(nsIWidget* aWidget, nsIDocShell* aDocShell, void* aDest,
                                         int aStride, int aWidth, int aHeight, int aRowLo,
                                         int aRowHi, int aRow2Lo, int aRow2Hi, int aShift,
                                         const void* aSrc, int aSrcY, int aEngineShift) __attribute__((weak));
  // aOut: ten ints (the rows the tree changed, the rows fixed layers cover, the two bands read, the
  // frame's position and its valid flag)
  int jihad_offscreen_compose_poll(nsIWidget* aWidget, uint32_t aToken, int* aOut,
                                   int* aScrollOnly) __attribute__((weak));
  void jihad_offscreen_compose_forget(nsIWidget* aWidget, uint32_t aToken) __attribute__((weak));
  // OPTIONAL (T-502): the card's zoom and the visual viewport's residual offset (CSS px, what the
  // engine scroll could not reach), applied by the compositor to the whole layer tree.
  void jihad_offscreen_set_view(nsIWidget* aWidget, double aScale, double aOffsetX,
                                double aOffsetY) __attribute__((weak));
  // OPTIONAL (T-502): the card's scroll (CSS px, absolute), applied by the compositor to the
  // root scroll frame's content against the engine's own scroll; aValid 0 turns it off.
  void jihad_offscreen_set_scroll(nsIWidget* aWidget, double aX, double aY, int aValid) __attribute__((weak));
  // OPTIONAL (T-502): the displayport (margins in screen px) on the root scroll frame's content
  // of the docshell's document, from layout itself; 0 on success (see nsLayoutUtils.cpp).
  int jihad_layout_set_displayport(nsIDocShell* aDocShell, float aVisW, float aVisH, float aSide, float aTop,
                                   float aBottom) __attribute__((weak));
  // OPTIONAL (T-505): the rows the GPU path's frame carries above and below what the card shows
  // (CSS px); the root scroll frame's clip is widened by them. 0 on success.
  int jihad_layout_set_overscan(nsIDocShell* aDocShell, float aTop, float aBottom) __attribute__((weak));
  // OPTIONAL (T-502 round 108): the fd the compositor writes a byte to when a compose job finishes.
  void jihad_offscreen_set_done_fd(int aFd) __attribute__((weak));
  // OPTIONAL (T-502): the compositor's composite-request count; a frame is due when it moves.
  uint32_t jihad_offscreen_compositor_seq(nsIWidget* aWidget) __attribute__((weak));
  // OPTIONAL (patch 0012): the sticky content-invalidation flag, and its damage rectangle.
  // Fallback: input/load-driven paints, whole-viewport repaints — slower, never wrong.
  bool jihad_offscreen_take_dirty(nsIWidget* aWidget) __attribute__((weak));
  bool jihad_offscreen_take_dirty_rect(nsIWidget* aWidget, int32_t* aX, int32_t* aY,
                                       int32_t* aW, int32_t* aH) __attribute__((weak));

  // ── XUL popups (a popup is its own display root; the main render never contains it) ───────
  // OPTIONAL: composite open popups over a painted buffer (returns how many). Fallback: popups
  // do not appear.
  int jihad_offscreen_composite_popups(nsIWidget* aWidget, void* aDest, int aStride,
                                       int aWidth, int aHeight, double aOriginX,
                                       double aOriginY, double aZoom) __attribute__((weak));
  // OPTIONAL: deliver a mouse event into an open popup (true = it was inside one).
  bool jihad_offscreen_popup_mouse(int aMsg, double aX, double aY) __attribute__((weak));
  // OPTIONAL: how many popups are open (tells "this tap opened a menu" from "it did not").
  int jihad_offscreen_popup_count() __attribute__((weak));
  // OPTIONAL: close every open popup (no native mouse capture here to roll them up).
  bool jihad_offscreen_popup_rollup() __attribute__((weak));

  // ── NPAPI / Flash (dom/plugins; patches 0016–0029) ────────────────────────────────────────
  // OPTIONAL: Palm's spotlight system event to every running plugin (fullscreen). Fallback:
  // the frozen YAP spotlight commands stay no-ops.
  int jihad_plugin_palm_spotlight(int aOn, int aLeft, int aTop, int aRight,
                                  int aBottom) __attribute__((weak));
  // OPTIONAL: plugin frames received (PluginInstanceParent::RecvShow). Fallback: the dirty flag.
  uint32_t jihad_plugin_frame_seq(void) __attribute__((weak));
  // OPTIONAL: ask every live plugin for one frame. Fallback: the child's own repaint timer.
  int jihad_plugin_request_frame(void) __attribute__((weak));

  // ── Audio (media/libcubeb pulse backend; patch 0030 and later) ────────────────────────────
  // OPTIONAL: how many engine audio streams are playing. Fallback: the speaker route is left
  // as the system set it.
  int jihad_pulse_active_streams(void) __attribute__((weak));
}

namespace jihad {

// The optional exports, by name, for the start-up report and the engine-update checker
// (build/engine/check-engine-api.py reads this header for the full list, required ones too).
inline void ReportEngineExports() {
  struct Opt { const char* name; bool present; };
  const Opt opts[] = {
    {"jihad_offscreen_render_region", jihad_offscreen_render_region != nullptr},
    {"jihad_offscreen_compose", jihad_offscreen_compose != nullptr},
    {"jihad_offscreen_compose_start", jihad_offscreen_compose_start != nullptr},
    {"jihad_offscreen_compose_poll", jihad_offscreen_compose_poll != nullptr},
    {"jihad_offscreen_compose_forget", jihad_offscreen_compose_forget != nullptr},
    {"jihad_offscreen_compositor_seq", jihad_offscreen_compositor_seq != nullptr},
    {"jihad_offscreen_set_view", jihad_offscreen_set_view != nullptr},
    {"jihad_offscreen_set_scroll", jihad_offscreen_set_scroll != nullptr},
    {"jihad_offscreen_set_done_fd", jihad_offscreen_set_done_fd != nullptr},
    {"jihad_layout_set_displayport", jihad_layout_set_displayport != nullptr},
    {"jihad_layout_set_overscan", jihad_layout_set_overscan != nullptr},
    {"jihad_offscreen_take_dirty", jihad_offscreen_take_dirty != nullptr},
    {"jihad_offscreen_take_dirty_rect", jihad_offscreen_take_dirty_rect != nullptr},
    {"jihad_offscreen_composite_popups", jihad_offscreen_composite_popups != nullptr},
    {"jihad_offscreen_popup_mouse", jihad_offscreen_popup_mouse != nullptr},
    {"jihad_offscreen_popup_count", jihad_offscreen_popup_count != nullptr},
    {"jihad_offscreen_popup_rollup", jihad_offscreen_popup_rollup != nullptr},
    {"jihad_plugin_palm_spotlight", jihad_plugin_palm_spotlight != nullptr},
    {"jihad_plugin_frame_seq", jihad_plugin_frame_seq != nullptr},
    {"jihad_plugin_request_frame", jihad_plugin_request_frame != nullptr},
    {"jihad_pulse_active_streams", jihad_pulse_active_streams != nullptr},
  };
  int missing = 0;
  for (const Opt& o : opts) {
    if (!o.present) {
      fprintf(stderr, "[jihad-bs] engine: optional export %s is MISSING from libxul (its fallback is in use)\n", o.name);
      ++missing;
    }
  }
  fprintf(stderr, "[jihad-bs] engine: %d of %d optional jihad_* exports present\n",
          (int)(sizeof(opts) / sizeof(opts[0])) - missing, (int)(sizeof(opts) / sizeof(opts[0])));
}

}  // namespace jihad

#endif  // JIHAD_ENGINE_COMPAT_H
