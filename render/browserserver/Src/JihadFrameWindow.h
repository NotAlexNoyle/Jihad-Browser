/* Copyright 2026 NotAlexNoyle. Apache-2.0, as the isis-project sources beside it.
 *
 * T-517, the frame window: where in its shared segment a frame starts. Read by the adapter
 * (render/adapter/BrowserOffscreen.cpp), written by the daemon (render/goanna/BrowserPageGoanna.cpp).
 */
#ifndef JIHAD_FRAME_WINDOW_H
#define JIHAD_FRAME_WINDOW_H

#include <stddef.h>
#include <stdint.h>

// An isis segment is [BrowserOffscreenInfo][pixels], and a frame's rows began at the pixels'
// start: a frame the card had scrolled by dy rows was the last one moved dy rows within its buffer,
// a 3.7 MB copy for every frame of a pan (5-6 ms of the compositor thread). The segments hold four
// screens of pixels and a frame is about one, so the frame may start further into the pixels
// instead: the rows already there are the scrolled frame, and only what entered is drawn. Where it
// starts is in a trailer in the segment's last 64 bytes, past the pixel rows the adapter uses; the
// YAP messages and the header are as they were.
//   magic        kJihadWindowMagic once the daemon wrote the trailer (it does before every publish)
//   windowRows   the frame starts this many rows (of the header's renderedWidth pixels) in
//   adapterCaps  kJihadWindowCaps: the adapter that made the segment reads windows; the daemon moves
//                windows only then (an adapter from before reads every frame from the pixels' start)
//   dirtySeq     T-527: this frame's number (the daemon counts its publishes)
//   dirtyMagic   T-527: kJihadDirtyMagic when dirtyLeft..dirtyBottom (frame px, right and bottom
//                exclusive) hold all that differs from frame dirtyBase, at the same geometry
//                (renderedX/Y/Width/Height, contentZoom): an adapter showing frame dirtyBase may
//                invalidate only that rect (an empty one: nothing to redraw). Anything else: the whole
//                frame. An adapter from before never reads them; a daemon from before leaves them zero.
struct JihadFrameTrailer {
    uint32_t magic;
    int32_t  windowRows;
    uint32_t adapterCaps;
    uint32_t dirtyMagic;
    int32_t  dirtyLeft, dirtyTop, dirtyRight, dirtyBottom;
    uint32_t dirtySeq, dirtyBase;
    uint32_t reserved[6];
};

static const size_t   kJihadTrailerBytes = 64;
static const uint32_t kJihadWindowMagic  = 0x4e49574aU;   // "JWIN"
static const uint32_t kJihadWindowCaps   = 0x5041434aU;   // "JCAP"
static const uint32_t kJihadDirtyMagic   = 0x5452444aU;   // "JDRT"

// the trailer of a segment of aSegmentBytes bytes mapped at aSegment (0 when it is too small)
inline JihadFrameTrailer* jihadFrameTrailer(unsigned char* aSegment, size_t aSegmentBytes)
{
    return aSegmentBytes > kJihadTrailerBytes
        ? (JihadFrameTrailer*)(aSegment + aSegmentBytes - kJihadTrailerBytes) : (JihadFrameTrailer*)0;
}

#endif
