#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Jihad Browser — generate a SWF that PLAYS A SOUND (cavekit-addons-extensions.md R7).
#
# WHY THIS EXISTS. Every test asset in this tree so far is silent, so nothing in the port has
# ever exercised Flash's audio path — "Flash works" has only ever meant "Flash rasterises".
#
# CORRECTED 2026-08-10. This file used to argue that Flash cannot be using ALSA, because it has
# no audio symbol imports and no audio library in its NEEDED list. Both facts are true and the
# conclusion drawn from them was wrong: Flash reaches ALSA by runtime dlopen, so nothing about
# it shows up in the import table. It opens the PCM itself (loader 0x2f7bd0, open 0x2f71e0) and
# asks for plughw:0,0. Audio is NOT a Luna bus call and not PIpc. Device-verified: the tone from
# this generator is audible through the stock, unpatched plugin.
#
# The sound is deliberately the simplest thing the format allows: uncompressed little-endian
# PCM (SoundFormat 3). No MP3, no ADPCM, no codec dependency — if this does not play, the
# problem is the host, not a missing decoder, which is the whole point of a control asset.
#
# A square wave, not a sine: it is trivially recognisable by ear over a tinny speaker, it
# survives any resampling the stack does to it, and its amplitude is constant so a level meter
# anywhere in the chain reads a flat non-zero value rather than something that could be noise.
#
# The moving square from make-anim-swf.py is kept on the stage so one asset answers both
# questions at once — if the timeline stops the picture freezes, so "silent" can always be told
# apart from "the movie is not running at all".
#
# Usage: python3 render/goanna/test/make-audio-swf.py [-o out.swf] [--hz 440] [--seconds 1]
import argparse
import math
import struct

TWIPS = 20

# Import the shape/tag machinery from the animation generator rather than duplicating it: the
# bit-packing is the part that is easy to get subtly wrong, and it is already device-proven.
import os
import sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from importlib import import_module
_anim = import_module('make-anim-swf'.replace('-', '_')) if False else None

# make-anim-swf.py is not an importable module name (hyphens), so load it by path.
import importlib.util
_spec = importlib.util.spec_from_file_location(
    'jihad_anim_swf', os.path.join(os.path.dirname(os.path.abspath(__file__)), 'make-anim-swf.py'))
_anim = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(_anim)

BitWriter = _anim.BitWriter
rect = _anim.rect
tag = _anim.tag
define_shape_rect = _anim.define_shape_rect
place_object2 = _anim.place_object2

# SoundRate is a 2-bit enum, not a number — 5512.5 Hz is index 0 and doubles each step.
RATE_INDEX = {5512: 0, 11025: 1, 22050: 2, 44100: 3}


def pcm_square(hz, rate, seconds, amplitude=0.5):
    """Signed 16-bit little-endian mono square wave. Amplitude well under full scale so that
    nothing downstream clips it and reports a fault that looks like a plumbing failure."""
    n = int(rate * seconds)
    peak = int(32767 * amplitude)
    period = max(2, int(rate / hz))
    out = bytearray()
    for i in range(n):
        v = peak if (i % period) < (period // 2) else -peak
        out += struct.pack('<h', v)
    return bytes(out), n


def define_sound(sound_id, hz, rate, seconds):
    """DefineSound, tag 14. SoundFormat 3 = uncompressed little-endian PCM."""
    if rate not in RATE_INDEX:
        raise SystemExit(f'rate must be one of {sorted(RATE_INDEX)}')
    data, sample_count = pcm_square(hz, rate, seconds)

    b = BitWriter()
    b.ub(3, 4)                    # SoundFormat = uncompressed little-endian PCM
    b.ub(RATE_INDEX[rate], 2)     # SoundRate
    b.ub(1, 1)                    # SoundSize = 16-bit (required for format 3)
    b.ub(0, 1)                    # SoundType = mono
    body = struct.pack('<H', sound_id) + b.get() + struct.pack('<I', sample_count) + data
    return tag(14, body)


# LAME's encoder delay (576) plus the decoder's own (528+1). Skipping exactly this many samples is
# the standard gapless-MP3 correction, and SWF's SeekSamples is where a player takes it.
LAME_DELAY_SAMPLES = 1105


def define_sound_mp3(sound_id, mp3_path, rate, sample_count, stereo=False,
                     seek_samples=LAME_DELAY_SAMPLES):
    """DefineSound, tag 14, SoundFormat 2 = MP3. This is the format essentially all real Flash
    content uses, so it exercises the decode path Flash actually maintains — uncompressed PCM is
    accepted by the spec but is the least-travelled road in any player.

    THE GOTCHA: for an MP3 EVENT sound the SoundData is NOT the raw frame stream. It is a signed
    16-bit SeekSamples field FIRST, then the frames. Omitting it shifts the whole stream by two
    bytes and the player decodes garbage or silently drops the sound.

    THE SECOND GOTCHA, and it is audible: SeekSamples is NOT "normally 0". Every MP3 encoder
    prepends encoder delay and pads the final frame, so a LOOPED MP3 with SeekSamples=0 plays a
    short silence on EVERY iteration — a 1 s loop chops at ~1 Hz and sounds like morse code, and
    the gap tracks the sound's length, which is how it is told apart from a buffer underrun.
    Device-confirmed 2026-08-10 by ear at 1 s and 5 s. PCM never shows this because it has no
    encoder delay, so a clean PCM tone next to a chopping MP3 one is NOT a decode fault."""
    data = open(mp3_path, 'rb').read()
    if data[:3] == b'ID3':
        raise SystemExit('mp3 has an ID3 tag; write it with -id3v2_version 0')
    if data[0] != 0xFF:
        raise SystemExit('mp3 does not start with a frame sync (0xFF)')
    if rate not in RATE_INDEX:
        raise SystemExit(f'rate must be one of {sorted(RATE_INDEX)}')

    b = BitWriter()
    b.ub(2, 4)                    # SoundFormat = MP3
    b.ub(RATE_INDEX[rate], 2)
    b.ub(1, 1)                    # SoundSize (ignored for compressed, but 16-bit is conventional)
    b.ub(1 if stereo else 0, 1)   # SoundType
    body = (struct.pack('<H', sound_id) + b.get() + struct.pack('<I', sample_count)
            + struct.pack('<h', seek_samples)
            + data)
    return tag(14, body)


def start_sound(sound_id, loops, no_multiple=True):
    """StartSound, tag 15. Looped so the tone outlives any startup stall — a one-shot that
    fires while the plugin is still coming up is indistinguishable from no audio at all.

    THE THIRD GOTCHA, and it is the one that sounds like a PORT DEFECT: this tag sits on the main
    timeline, and a timeline with no Stop LOOPS. Every time frame 1 comes round the tag runs
    again, and with SyncNoMultiple=0 the player starts ANOTHER overlapping instance rather than
    ignoring the retrigger. At 60 frames / 30 fps that is a fresh voice every 2 s, each one 200
    loops long, so the voices only accumulate: ~15 of them by 30 s, summed into the same output.
    Phase-offset copies of one tone comb-filter against each other and the sum clips, which is
    heard as intermittent crackle — "occasional static, like radio chatter" — and looks exactly
    like an ALSA underrun. Device-diagnosed 2026-08-10, after the CPU clock had already been
    exonerated by pinning it at 1188 MHz and hearing the same static.

    SyncNoMultiple=1 makes the retrigger a no-op while the sound is already playing, which is what
    a single-voice test asset needs. A 440 Hz square at 11025 Hz hides this almost completely
    because 2 s is exactly 882 whole periods, so its copies stack IN PHASE and just get louder —
    which is why the PCM asset passed by ear and the MP3 ones did not, and why this looked like a
    codec problem for two sessions."""
    b = BitWriter()
    b.ub(0, 2)                    # Reserved
    b.ub(0, 1)                    # SyncStop
    b.ub(1 if no_multiple else 0, 1)  # SyncNoMultiple
    b.ub(0, 1)                    # HasEnvelope
    b.ub(1 if loops > 1 else 0, 1)  # HasLoops
    b.ub(0, 1)                    # HasOutPoint
    b.ub(0, 1)                    # HasInPoint
    body = struct.pack('<H', sound_id) + b.get()
    if loops > 1:
        body += struct.pack('<H', min(loops, 0xFFFF))
    return tag(15, body)


def build(width_px, height_px, frames, fps, square_px, hz, rate, seconds, loops, mp3_path=None,
          stereo=False, no_multiple=True):
    stage_w, stage_h = width_px * TWIPS, height_px * TWIPS
    sq = square_px * TWIPS

    body = b''
    body += tag(9, bytes([0x00, 0x00, 0x60]))                  # dark blue background
    body += define_shape_rect(1, sq, sq, (0xFF, 0x00, 0xFF))   # magenta square: audio asset

    # THE GOTCHA THAT COST A WHOLE SESSION: shapes and sounds live in ONE character dictionary,
    # so the sound may not reuse the shape's ID. A player that already has character 1 ignores
    # the redefinition, StartSound then resolves to a shape, and the sound is dropped SILENTLY —
    # no error, no audio device ever opened, which reads exactly like a broken host audio path.
    SOUND_ID = 2
    if mp3_path:
        body += define_sound_mp3(SOUND_ID, mp3_path, rate, int(rate * seconds), stereo)
    else:
        body += define_sound(SOUND_ID, hz, rate, seconds)

    span = stage_w - sq
    y = (stage_h - sq) // 2
    for i in range(frames):
        x = (span * i) // max(1, frames - 1)
        if i == 0:
            body += place_object2(1, char_id=1, translate=(x, y))
            # Runs on EVERY pass of the looping timeline, not once — see start_sound.
            body += start_sound(SOUND_ID, loops, no_multiple)
        else:
            body += place_object2(1, translate=(x, y), move=True)
        body += tag(1, b'')                    # ShowFrame
    body += tag(0, b'')                        # End

    header_tail = rect(0, stage_w, 0, stage_h)
    header_tail += struct.pack('<H', int(round(fps * 256)))
    header_tail += struct.pack('<H', frames)

    total = 3 + 1 + 4 + len(header_tail) + len(body)
    return b'FWS' + bytes([6]) + struct.pack('<I', total) + header_tail + body


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('-o', '--out', default='jihad-audio.swf')
    ap.add_argument('--width', type=int, default=320)
    ap.add_argument('--height', type=int, default=240)
    ap.add_argument('--frames', type=int, default=60)
    ap.add_argument('--fps', type=float, default=30.0)
    ap.add_argument('--square', type=int, default=48)
    ap.add_argument('--hz', type=float, default=440.0)
    ap.add_argument('--rate', type=int, default=11025)
    ap.add_argument('--seconds', type=float, default=1.0)
    ap.add_argument('--loops', type=int, default=200)
    ap.add_argument('--mp3', help='embed this bare MP3 frame stream as SoundFormat 2 instead of PCM')
    ap.add_argument('--stereo', action='store_true', help='SoundType = stereo')
    ap.add_argument('--multiple', action='store_true',
                    help='SyncNoMultiple=0: let the looping timeline stack overlapping voices. '
                         'This is the ASSET BUG that sounded like an ALSA underrun — only for '
                         'reproducing it deliberately.')
    a = ap.parse_args()

    data = build(a.width, a.height, a.frames, a.fps, a.square,
                 a.hz, a.rate, a.seconds, a.loops, a.mp3, a.stereo, not a.multiple)
    with open(a.out, 'wb') as f:
        f.write(data)

    declared = struct.unpack('<I', data[4:8])[0]
    assert declared == len(data), f'length field {declared} != actual {len(data)}'
    print(f'== wrote {a.out}: {len(data)} bytes, {a.width}x{a.height} @ {a.fps}fps, '
          f'{"MP3 " + a.mp3 if a.mp3 else str(int(a.hz)) + " Hz square"}, '
          f'{a.rate} Hz {"stereo" if a.stereo else "mono"}, {a.seconds}s x{a.loops}, '
          f'{"MULTI-VOICE (asset bug, deliberate)" if a.multiple else "single-voice"} ==')


if __name__ == '__main__':
    main()
