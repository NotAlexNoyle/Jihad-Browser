#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Jihad Browser — generate an ANIMATED test SWF (cavekit-addons-extensions.md R7).
#
# WHY THIS EXISTS. render/goanna/test/jihad-test.swf is 29 bytes and does nothing but
# SetBackgroundColor(magenta). It proved that Flash rasterises AT ALL, which was the question
# at the time — but it is a single static frame, so it cannot distinguish "Flash renders" from
# "Flash renders at a usable frame rate". A frozen first frame and a 30fps game look identical
# to it. R7's real target is content that MOVES: games, video, animation.
#
# So this emits a genuine multi-frame timeline: a bright green square stepping across a dark
# blue stage, one step per frame, at a declared frame rate. Everything about it is chosen to be
# measurable without a human:
#   * flat, saturated colours          -> a screenshot can be pixel-counted, no tolerance games
#   * one square, one axis of motion   -> the x-centroid of the green pixels IS the frame number
#   * a full-width sweep that wraps    -> any two screenshots taken seconds apart must differ
#   * declared rate in the SWF header  -> the plugin's own clock is what gets measured
#
# There is no SWF compiler on this machine and none in the bundle, so the file is assembled
# here from the format spec. It is deliberately SWF 6 / DefineShape (not DefineShape3+), which
# is the oldest form every player accepts, because the target is Flash 10.3 on ARM and the
# point of the asset is to test the HOST, not the plugin's tolerance for modern tags.
#
# Usage: python3 render/goanna/test/make-anim-swf.py [-o out.swf] [--frames N] [--fps N]
import argparse
import struct

TWIPS = 20   # 1 px = 20 twips, the unit the whole format is in


class BitWriter:
    """MSB-first bit packer. SWF mixes bit fields and whole bytes, and every bit-packed
    structure is byte-aligned when it ends, so alignment is explicit rather than implied."""

    def __init__(self):
        self.buf = bytearray()
        self.acc = 0
        self.nbits = 0

    def ub(self, value, bits):
        for i in range(bits - 1, -1, -1):
            self.acc = (self.acc << 1) | ((value >> i) & 1)
            self.nbits += 1
            if self.nbits == 8:
                self.buf.append(self.acc)
                self.acc = 0
                self.nbits = 0

    def sb(self, value, bits):
        self.ub(value & ((1 << bits) - 1), bits)

    def align(self):
        if self.nbits:
            self.acc <<= (8 - self.nbits)
            self.buf.append(self.acc)
            self.acc = 0
            self.nbits = 0

    def bytes_(self, b):
        self.align()
        self.buf.extend(b)

    def get(self):
        self.align()
        return bytes(self.buf)


def sbits_needed(*values):
    """Bit width for the widest SIGNED value in the set (SWF stores everything signed)."""
    n = 1
    for v in values:
        need = 1
        while not (-(1 << (need - 1)) <= v < (1 << (need - 1))):
            need += 1
        n = max(n, need)
    return n


def rect(xmin, xmax, ymin, ymax):
    n = sbits_needed(xmin, xmax, ymin, ymax)
    w = BitWriter()
    w.ub(n, 5)
    for v in (xmin, xmax, ymin, ymax):
        w.sb(v, n)
    return w.get()


def tag(code, body):
    """Tag header: the short form packs the length into the low 6 bits; anything from 63 bytes
    up must use the long form, and a player will mis-parse the stream if you get this wrong."""
    if len(body) < 0x3F:
        return struct.pack('<H', (code << 6) | len(body)) + body
    return struct.pack('<H', (code << 6) | 0x3F) + struct.pack('<I', len(body)) + body


def define_shape_rect(shape_id, w_tw, h_tw, rgb):
    """DefineShape (tag 2): one solid-filled rectangle with its top-left at the origin."""
    body = struct.pack('<H', shape_id)
    body += rect(0, w_tw, 0, h_tw)

    b = BitWriter()
    b.bytes_(bytes([1]))                       # FillStyleCount = 1
    b.bytes_(bytes([0x00]) + bytes(rgb))       # FILLSTYLE: solid, RGB
    b.bytes_(bytes([0]))                       # LineStyleCount = 0
    b.ub(1, 4)                                 # NumFillBits
    b.ub(0, 4)                                 # NumLineBits

    # STYLECHANGERECORD: move to the origin and select fill style 1.
    b.ub(0, 1)                                 # TypeFlag = 0 (non-edge)
    b.ub(0, 1)                                 # StateNewStyles
    b.ub(0, 1)                                 # StateLineStyle
    b.ub(1, 1)                                 # StateFillStyle1
    b.ub(0, 1)                                 # StateFillStyle0
    b.ub(1, 1)                                 # StateMoveTo
    mb = sbits_needed(0, w_tw, h_tw)
    b.ub(mb, 5)
    b.sb(0, mb)                                # MoveDeltaX
    b.sb(0, mb)                                # MoveDeltaY
    b.ub(1, 1)                                 # FillStyle1 = index 1

    # Four straight edges, closing the rectangle clockwise.
    for dx, dy in ((w_tw, 0), (0, h_tw), (-w_tw, 0), (0, -h_tw)):
        nb = max(2, sbits_needed(dx, dy))
        b.ub(1, 1)                             # TypeFlag = 1 (edge)
        b.ub(1, 1)                             # StraightFlag
        b.ub(nb - 2, 4)                        # NumBits, biased by 2
        b.ub(1, 1)                             # GeneralLineFlag
        b.sb(dx, nb)
        b.sb(dy, nb)

    b.ub(0, 6)                                 # EndShapeRecord
    body += b.get()
    return tag(2, body)


def place_object2(depth, char_id=None, translate=None, move=False):
    """PlaceObject2 (tag 26). char_id on the first placement, move=True to re-position the
    object already at that depth on later frames."""
    flags = 0
    if move:
        flags |= 0x01
    if char_id is not None:
        flags |= 0x02
    if translate is not None:
        flags |= 0x04
    body = bytes([flags]) + struct.pack('<H', depth)
    if char_id is not None:
        body += struct.pack('<H', char_id)
    if translate is not None:
        tx, ty = translate
        m = BitWriter()
        m.ub(0, 1)                             # HasScale
        m.ub(0, 1)                             # HasRotate
        nb = sbits_needed(tx, ty)
        m.ub(nb, 5)
        m.sb(tx, nb)
        m.sb(ty, nb)
        body += m.get()
    return tag(26, body)


def build(width_px, height_px, frames, fps, square_px):
    stage_w, stage_h = width_px * TWIPS, height_px * TWIPS
    sq = square_px * TWIPS

    body = b''
    body += tag(9, bytes([0x00, 0x00, 0x60]))          # SetBackgroundColor: dark blue
    body += define_shape_rect(1, sq, sq, (0x00, 0xFF, 0x00))   # bright green square

    # The sweep is full-width and wraps, so the square's x position is a direct readout of
    # which frame is on screen: two screenshots that agree mean the timeline is not advancing.
    span = stage_w - sq
    y = (stage_h - sq) // 2
    for i in range(frames):
        x = (span * i) // max(1, frames - 1)
        if i == 0:
            body += place_object2(1, char_id=1, translate=(x, y))
        else:
            body += place_object2(1, translate=(x, y), move=True)
        body += tag(1, b'')                            # ShowFrame
    body += tag(0, b'')                                # End

    header_tail = rect(0, stage_w, 0, stage_h)
    header_tail += struct.pack('<H', int(round(fps * 256)))    # FIXED8 frame rate
    header_tail += struct.pack('<H', frames)

    total = 3 + 1 + 4 + len(header_tail) + len(body)
    return b'FWS' + bytes([6]) + struct.pack('<I', total) + header_tail + body


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-o', '--out', default='jihad-anim.swf')
    ap.add_argument('--width', type=int, default=320)
    ap.add_argument('--height', type=int, default=240)
    ap.add_argument('--frames', type=int, default=60)
    ap.add_argument('--fps', type=float, default=30.0)
    ap.add_argument('--square', type=int, default=48)
    a = ap.parse_args()

    data = build(a.width, a.height, a.frames, a.fps, a.square)
    with open(a.out, 'wb') as f:
        f.write(data)

    # Read the length field back out rather than trusting the arithmetic: a wrong file length
    # is the one error a player reports as a generic parse failure with nothing to go on.
    declared = struct.unpack('<I', data[4:8])[0]
    assert declared == len(data), f'length field {declared} != actual {len(data)}'
    print(f'== wrote {a.out}: {len(data)} bytes, {a.width}x{a.height}, '
          f'{a.frames} frames @ {a.fps}fps ==')


if __name__ == '__main__':
    main()
