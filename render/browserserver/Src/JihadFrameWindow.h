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
// T-567 (b), the owner's choice of 2026-11-10: reserved[0] is kJihadBgMagic when reserved[1] holds the page's canvas
// background as the frame's pixels hold a colour (0xAARRGGBB, B-G-R-A bytes); the card fills its rows past the frame's
// end with it rather than leaving its own background there (a grey band under a fast drag). A card from before
// never reads it; a daemon from before leaves it zero.
static const uint32_t kJihadBgMagic      = 0x4347424aU;   // "JBGC"

// T-544, the stuck parts. Between two frames of a pan the card moves the frame it holds, and the
// frame's fixed and stuck parts (desktop Wikipedia's sidebars) moved with the page and jumped back at
// the next frame. The daemon writes the frame's fixed and stuck layers' rects and where the card's view
// sat in it into the kJihadStuckBytes before the trailer; the card draws them where the frame did.
// A daemon from now keeps its frames' rows out of those bytes (kJihadTrailerAreaBytes); the card
// takes the rects only when seq is the trailer's dirtySeq (an older daemon's rows there never match).
//   magic        kJihadStuckMagic when the rest holds this frame's rects
//   seq          the frame they are of (the trailer's dirtySeq)
//   viewX/viewY  the card's view's top-left in the frame when it was composed (frame px)
//   count        rects (<= kJihadStuckMax): left, top, right, bottom (frame px, right and bottom exclusive)
// T-568 (2026-11-03), what lay beneath the held rects. The rows a held rect's moved copy covers outside its place
// belong to the content UNDER the rect in the frame, which the frame does not have, so the card filled them in one
// colour: a band one shift tall under the long page's fixed header on a third of a drag's samples whenever the
// express frame led the card (T-527's prediction). A frame dressed on the CPU (T-505: composed without its parts,
// the parts drawn over on the daemon's thread) knows what lay beneath each part, so the daemon writes it into the
// kJihadBeneathBytes before the stuck block — rect by rect in rect order from beneathBase (bytes from the
// segment's start): the first (beneathRows[k] & 0xffff) rows and the last (beneathRows[k] >> 16) rows of rect k,
// each row the rect's width x 4 bytes (the whole rect when the first count is its height) — and the card draws
// those rows where its fill went. A card from before never reads them (its reserved words); a daemon from before
// leaves beneathMagic as it found it and its frames may use the area as pixel rows, which the magic does not match.
static const int kJihadStuckMax = 8;
struct JihadStuckBlock {
    uint32_t magic;
    uint32_t seq;
    int32_t  viewX, viewY;
    int32_t  count;
    uint32_t beneathMagic;                  // T-568: kJihadBeneathMagic when beneathBase and beneathRows are this frame's
    int32_t  beneathBase;                   // T-568: the beneath pixels' offset from the segment's start
    int32_t  reserved;
    int32_t  rects[kJihadStuckMax][4];
    int32_t  beneathRows[kJihadStuckMax];   // T-568: per rect, the first rows kept (low 16 bits) and the last (high 16); 0: none
};

static const size_t   kJihadStuckBytes       = 192;
static const size_t   kJihadTrailerAreaBytes = kJihadTrailerBytes + kJihadStuckBytes;   // the trailer and the stuck block
static const uint32_t kJihadStuckMagic       = 0x4b54534aU;   // "JSTK"
// T-568: the beneath area before the stuck block; a daemon from now keeps its frames' rows out of it as well
static const size_t   kJihadBeneathBytes     = 512 * 1024;
static const size_t   kJihadReservedBytes    = kJihadTrailerAreaBytes + kJihadBeneathBytes;   // all that is not pixel rows
static const uint32_t kJihadBeneathMagic     = 0x544e424aU;   // "JBNT"
typedef char jihad_stuck_block_is_its_bytes[sizeof(JihadStuckBlock) == kJihadStuckBytes ? 1 : -1];

// the trailer of a segment of aSegmentBytes bytes mapped at aSegment (0 when it is too small)
inline JihadFrameTrailer* jihadFrameTrailer(unsigned char* aSegment, size_t aSegmentBytes)
{
    return aSegmentBytes > kJihadTrailerBytes
        ? (JihadFrameTrailer*)(aSegment + aSegmentBytes - kJihadTrailerBytes) : (JihadFrameTrailer*)0;
}

// T-544: the stuck block of the same segment
inline JihadStuckBlock* jihadStuckBlock(unsigned char* aSegment, size_t aSegmentBytes)
{
    return aSegmentBytes > kJihadTrailerAreaBytes
        ? (JihadStuckBlock*)(aSegment + aSegmentBytes - kJihadTrailerAreaBytes) : (JihadStuckBlock*)0;
}

#endif
