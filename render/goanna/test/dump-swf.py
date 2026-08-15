#!/usr/bin/env python3
# Copyright 2026 NotAlexNoyle. Apache-2.0.
#
# Jihad Browser — walk a SWF's tag stream and print what is actually in it.
#
# WHY THIS EXISTS. Two separate audio investigations in this tree were sunk by a BAD TEST ASSET
# that no measurement could distinguish from a broken port: a DefineSound reusing a DefineShape's
# character ID (the player keeps the first definition and drops the sound, silently), and a
# SeekSamples of 0 on a looped MP3 (a gap on every iteration that reads as a buffer underrun).
# Neither is visible from the outside. Parse an asset back out before believing a negative result
# from it.
#
# With --extract-sound it writes the raw SoundData payload (MP3 frame stream with the SeekSamples
# prefix stripped, or PCM samples) so it can be decoded and listened to off-device — which
# separates "the encoder produced garbage" from "the player mishandles it".
#
# Usage: python3 render/goanna/test/dump-swf.py FILE.swf [--extract-sound OUT]
import argparse
import struct
import sys

TAG_NAMES = {
    0: 'End', 1: 'ShowFrame', 2: 'DefineShape', 4: 'PlaceObject', 9: 'SetBackgroundColor',
    14: 'DefineSound', 15: 'StartSound', 26: 'PlaceObject2', 39: 'DefineSprite',
    43: 'FrameLabel', 12: 'DoAction', 22: 'DefineShape2', 32: 'DefineShape3',
}
SOUND_FORMAT = {0: 'PCM(native-endian)', 1: 'ADPCM', 2: 'MP3', 3: 'PCM(little-endian)',
                6: 'Nellymoser'}
SOUND_RATE = {0: 5512, 1: 11025, 2: 22050, 3: 44100}

# MPEG-1/2 Layer III, the only thing Flash event sounds carry.
BITRATES_V1L3 = [0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0]
BITRATES_V2L3 = [0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0]
SRATES = {3: [44100, 48000, 32000], 2: [22050, 24000, 16000], 0: [11025, 12000, 8000]}


def mp3_frames(data):
    """Walk the frame headers. Returns (frames, samples, sample_rate, bad_syncs, stray_tail).

    A frame-level walk is what catches a truncated or spliced stream: the player decodes past the
    last complete frame and emits whatever is there, which is heard as static rather than silence."""
    i, frames, samples, rate, bad = 0, 0, 0, None, 0
    n = len(data)
    while i + 4 <= n:
        if data[i] != 0xFF or (data[i + 1] & 0xE0) != 0xE0:
            bad += 1
            i += 1
            continue
        ver = (data[i + 1] >> 3) & 0x03      # 3 = MPEG1, 2 = MPEG2, 0 = MPEG2.5
        layer = (data[i + 1] >> 1) & 0x03    # 1 = Layer III
        bri = (data[i + 2] >> 4) & 0x0F
        sri = (data[i + 2] >> 2) & 0x03
        pad = (data[i + 2] >> 1) & 0x01
        if layer != 1 or ver == 1 or sri == 3 or bri in (0, 15):
            bad += 1
            i += 1
            continue
        sr = SRATES[ver][sri]
        br = (BITRATES_V1L3 if ver == 3 else BITRATES_V2L3)[bri] * 1000
        spf = 1152 if ver == 3 else 576
        flen = int((spf // 8) * br / sr) + pad
        if flen <= 4:
            bad += 1
            i += 1
            continue
        rate = rate or sr
        frames += 1
        samples += spf
        i += flen
    return frames, samples, rate, bad, n - i


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('swf')
    ap.add_argument('--extract-sound', help='write the SoundData payload here')
    a = ap.parse_args()

    d = open(a.swf, 'rb').read()
    if d[:3] != b'FWS':
        sys.exit(f'{a.swf}: not an uncompressed SWF (magic {d[:3]!r}); CWS/ZWS not handled')
    declared = struct.unpack('<I', d[4:8])[0]
    print(f'== {a.swf}: {len(d)} bytes, header declares {declared}'
          f'{"" if declared == len(d) else "   <-- MISMATCH"}')

    # Skip the frame-size RECT (bit-packed), then frame rate + frame count.
    i = 8
    nbits = d[i] >> 3
    total_bits = 5 + nbits * 4
    i += (total_bits + 7) // 8
    fps = struct.unpack('<H', d[i:i + 2])[0] / 256.0
    fcount = struct.unpack('<H', d[i + 2:i + 4])[0]
    i += 4
    print(f'   {fps:g} fps, {fcount} frames declared')

    # Pre-scan for a main-timeline Stop (DoAction carrying ActionStop, 0x07). A Stop is the other
    # way to keep a looping timeline from re-firing StartSound, so the retrigger warning below has
    # to know about it — and it can appear AFTER the StartSound it would neutralise.
    saw_stop = False
    j = i
    while j < len(d):
        (cl,) = struct.unpack('<H', d[j:j + 2])
        c, n = cl >> 6, cl & 0x3F
        j += 2
        if n == 0x3F:
            (n,) = struct.unpack('<I', d[j:j + 4])
            j += 4
        if c == 12 and b'\x07' in d[j:j + n]:
            saw_stop = True
        j += n
        if c == 0:
            break

    chars = {}
    while i < len(d):
        (code_len,) = struct.unpack('<H', d[i:i + 2])
        code, ln = code_len >> 6, code_len & 0x3F
        i += 2
        if ln == 0x3F:
            (ln,) = struct.unpack('<I', d[i:i + 4])
            i += 4
        body = d[i:i + ln]
        i += ln
        name = TAG_NAMES.get(code, f'tag{code}')
        extra = ''

        if code in (2, 22, 32, 14, 39):
            cid = struct.unpack('<H', body[:2])[0]
            if cid in chars:
                extra += f'  <-- ID {cid} ALREADY DEFINED by {chars[cid]}: the player keeps the ' \
                         f'FIRST and silently drops this one'
            chars[cid] = name
            extra = f' id={cid}' + extra

        if code == 14:
            flags = body[2]
            fmt, srate = flags >> 4, (flags >> 2) & 0x03
            size16, stereo = (flags >> 1) & 1, flags & 1
            count = struct.unpack('<I', body[3:7])[0]
            payload = body[7:]
            extra += (f' fmt={SOUND_FORMAT.get(fmt, fmt)} rate={SOUND_RATE[srate]}'
                      f' {"16" if size16 else "8"}-bit {"stereo" if stereo else "mono"}'
                      f' SampleCount={count}')
            if fmt == 2:
                seek = struct.unpack('<h', payload[:2])[0]
                mp3 = payload[2:]
                fr, smp, sr, bad, tail = mp3_frames(mp3)
                extra += (f'\n      SeekSamples={seek}  mp3={len(mp3)}B frames={fr}'
                          f' decoded_samples={smp} mp3_rate={sr}')
                if bad:
                    extra += f'\n      {bad} byte(s) skipped at bad sync  <-- SPLICED/CORRUPT STREAM'
                if tail:
                    extra += f'\n      {tail} trailing byte(s) past the last complete frame'
                if sr and sr != SOUND_RATE[srate]:
                    extra += (f'\n      <-- RATE MISMATCH: SWF declares {SOUND_RATE[srate]},'
                              f' the frames are {sr}')
                avail = smp - max(0, seek)
                if count > avail:
                    extra += (f'\n      <-- SampleCount {count} EXCEEDS available {avail}'
                              f' (decoded {smp} - seek {seek}): the player runs off the end of the'
                              f' frame data, which is heard as static, not silence')
                else:
                    extra += f'\n      SampleCount {count} <= available {avail}  ok'
                if a.extract_sound:
                    open(a.extract_sound, 'wb').write(mp3)
                    print(f'   wrote {len(mp3)} bytes of MP3 -> {a.extract_sound}')
            else:
                extra += f' payload={len(payload)}B'
                if a.extract_sound:
                    open(a.extract_sound, 'wb').write(payload)
                    print(f'   wrote {len(payload)} bytes of PCM -> {a.extract_sound}')

        if code == 15:
            sid = struct.unpack('<H', body[:2])[0]
            f = body[2]
            loops = struct.unpack('<H', body[3:5])[0] if (f >> 2) & 1 else 1
            no_multiple = (f >> 4) & 1
            extra = (f' sound={sid} loops={loops} SyncStop={(f >> 5) & 1}'
                     f' SyncNoMultiple={no_multiple}')
            if sid in chars and chars[sid] != 'DefineSound':
                extra += f'  <-- resolves to a {chars[sid]}, NOT a sound'
            # A main-timeline StartSound runs again on every pass of a looping timeline. With
            # SyncNoMultiple=0 each pass starts ANOTHER overlapping voice and they accumulate,
            # which comb-filters and clips: heard as intermittent crackle, indistinguishable from
            # an ALSA underrun.
            if not no_multiple and not saw_stop:
                period = fcount / fps if fps else 0
                extra += (f'\n      <-- RETRIGGER: the timeline loops every {period:.2f}s and'
                          f' nothing stops it, so this starts a NEW overlapping voice each pass'
                          f' (~{int(30 / period) if period else 0} of them by 30s). Rebuild with'
                          f' SyncNoMultiple=1 before believing any static heard from this asset.')

        if code in (1, 26, 4):
            continue                      # per-frame noise; the counts above already cover it
        print(f'   [{code:>2}] {name}{extra}')
        if code == 0:
            break


if __name__ == '__main__':
    main()
