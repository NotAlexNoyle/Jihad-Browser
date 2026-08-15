#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Jihad Browser — a SWF that REACTS TO THE KEYBOARD (cavekit-input-bridging.md R7).
#
# WHY THIS EXISTS. Key events were shown to reach the plugin long ago — the daemon logs
# `palm key 0x8 raw=65 handled=1`. That proves DELIVERY and nothing else. What a Flash game
# needs is that the CONTENT reacts, and that the card chrome does not react at the same time,
# and neither of those can be seen from a delivery log. Every other asset in this tree is a
# pure timeline with no ActionScript at all, so none of them can answer it.
#
# So this one carries real AVM1 bytecode. A sprite with two frames — frame 1 GREEN, frame 2 RED
# — is placed with a `onClipEvent(keyDown)` handler whose whole body is GotoFrame(1) + Stop.
# Press any key and the stage goes from green to red, permanently. That makes the check a pixel
# count on a screenshot rather than a human judgement, and it is unambiguous in a way a log line
# is not: green means no key ever arrived, red means the content itself handled one.
#
# keyDown on a clip (not a button `keyPress` condition) is deliberate: a button condition
# matches ONE key code per record and the spec's code table disagrees with itself about letters,
# whereas onClipEvent(keyDown) fires for anything — which is what "does the game get keys at
# all" actually asks.
#
# Usage: python3 render/goanna/test/make-key-swf.py [-o out.swf]
import argparse
import importlib.util
import os
import struct

TWIPS = 20

_spec = importlib.util.spec_from_file_location(
    'jihad_anim_swf', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'make-anim-swf.py'))
_anim = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_anim)
BitWriter, rect, tag = _anim.BitWriter, _anim.rect, _anim.tag
define_shape_rect, place_object2 = _anim.define_shape_rect, _anim.place_object2

# AVM1. ActionGotoFrame is 0x81 with a UI16 payload and is ZERO-BASED, so frame 2 is 1; it also
# resumes playback, hence the ActionStop (0x07) right after it. A zero byte ends the record.
ACTION_GOTO_FRAME_2_AND_STOP = bytes([0x81, 0x02, 0x00, 0x01, 0x00, 0x07, 0x00])

# ClipEventKeyDown is bit 6 of the SWF6 UI32 CLIPEVENTFLAGS. 0x00020000 is ClipEventKEYPRESS,
# which is a different event AND carries a trailing KeyCode UI8 after the action-size field —
# using it without that byte mis-parses the whole record, which is why the first attempt drew
# nothing at all rather than merely ignoring the keyboard.
CLIP_EVENT_KEY_DOWN = 0x00000040


def define_sprite_two_frames(sprite_id, green_id, red_id):
    """DefineSprite (tag 39): frame 1 shows the green shape, frame 2 replaces it with the red
    one at the same depth. The sprite's own timeline is what the clip's GotoFrame acts on."""
    body = struct.pack('<H', sprite_id) + struct.pack('<H', 2)     # id, FrameCount
    body += place_object2(1, char_id=green_id, translate=(0, 0))
    # STOP on frame 1. Without it the sprite runs its own two-frame loop forever and the stage
    # just flashes green/red, which looks like a keyboard response and is not one — observed on
    # device 2026-08-10. Resting on green is what makes the later red unambiguous.
    body += tag(12, bytes([0x07, 0x00]))                           # DoAction: Stop
    body += tag(1, b'')                                            # ShowFrame (frame 1)
    body += place_object2(1, char_id=red_id, translate=(0, 0), move=True)
    body += tag(1, b'')                                            # ShowFrame (frame 2)
    body += tag(0, b'')                                            # End of sprite
    return tag(39, body)


def place_sprite_with_keydown(depth, sprite_id):
    """PlaceObject2 carrying CLIPACTIONS. The layout is fixed and unforgiving: Reserved UI16,
    AllEventFlags UI32, then one record per handler (EventFlags UI32, size UI32, bytecode),
    then a UI32 zero terminator. A wrong size field makes the player skip the handler in
    silence, which is indistinguishable from the keyboard never arriving."""
    flags = 0x02 | 0x80                      # HasCharacter | HasClipActions
    body = bytes([flags]) + struct.pack('<H', depth) + struct.pack('<H', sprite_id)
    body += struct.pack('<H', 0)                       # Reserved
    body += struct.pack('<I', CLIP_EVENT_KEY_DOWN)     # AllEventFlags
    body += struct.pack('<I', CLIP_EVENT_KEY_DOWN)     # this record's EventFlags
    body += struct.pack('<I', len(ACTION_GOTO_FRAME_2_AND_STOP))
    body += ACTION_GOTO_FRAME_2_AND_STOP
    body += struct.pack('<I', 0)                       # ClipActionEndFlag
    return tag(26, body)


def build(width_px, height_px):
    stage_w, stage_h = width_px * TWIPS, height_px * TWIPS
    body = b''
    body += tag(9, bytes([0x20, 0x20, 0x20]))                          # dark grey backdrop
    body += define_shape_rect(1, stage_w, stage_h, (0x00, 0xFF, 0x00))  # green = no key yet
    body += define_shape_rect(2, stage_w, stage_h, (0xFF, 0x00, 0x00))  # red   = key handled
    body += define_sprite_two_frames(3, 1, 2)
    body += place_sprite_with_keydown(1, 3)
    body += tag(1, b'')                                                 # ShowFrame
    body += tag(12, bytes([0x07, 0x00]))                                # DoAction: Stop
    body += tag(1, b'')
    body += tag(0, b'')

    header_tail = rect(0, stage_w, 0, stage_h)
    header_tail += struct.pack('<H', int(round(12.0 * 256)))
    header_tail += struct.pack('<H', 2)
    total = 3 + 1 + 4 + len(header_tail) + len(body)
    return b'FWS' + bytes([6]) + struct.pack('<I', total) + header_tail + body


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-o', '--out', default='jihad-key.swf')
    ap.add_argument('--width', type=int, default=320)
    ap.add_argument('--height', type=int, default=240)
    a = ap.parse_args()
    data = build(a.width, a.height)
    with open(a.out, 'wb') as f:
        f.write(data)
    declared = struct.unpack('<I', data[4:8])[0]
    assert declared == len(data), f'length field {declared} != actual {len(data)}'
    print(f'== wrote {a.out}: {len(data)} bytes, {a.width}x{a.height} — '
          f'GREEN until a key is handled, then RED ==')


if __name__ == '__main__':
    main()
