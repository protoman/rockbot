#!/usr/bin/env python3
"""Turn a MIDI arrangement into a VGM for the Mega Drive's chips.

Each melodic track, one note at a time, gets an FM channel of its own, in
the file's order, up to five; FM channel 6 is left free for the sound
driver's samples. The lowest track plays the bass instrument, the others the
keys, as loud as the track's channel volume. The drum track (MIDI channel
10) goes to the PSG: the kick is a falling square wave, the snare and hats
are noise. The track loops as a whole, and is played at the given tempo,
or else at the file's, and turned down by the given decibels (9 if not
given), to sit as loud as the other tracks. Only the standard library is
used.

usage: midi2vgm.py IN.mid OUT.vgm [BPM [DB]]
"""

import math
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from notes2vgm import (BASS, KEYS, CARRIERS, FM_CLOCK, PSG_CLOCK, VGM_RATE, Vgm,
                       fm_note_off, frequency, key_code, set_instrument)

FRAME = VGM_RATE // 60
KICK, SNARE, HAT_CLOSED, HAT_PEDAL, HAT_OPEN = 35, 38, 42, 44, 46
KICK_CHANNEL = 0  # PSG square wave for the kick
quieter = 9  # dB off everything
# Noise per drum: control byte (white noise at a rate), then the attenuation
# frame by frame.
NOISE = {
    SNARE: (0xE5, [2, 3, 4, 6, 8, 10, 12]),
    HAT_CLOSED: (0xE4, [6, 9, 12]),
    HAT_PEDAL: (0xE4, [6, 9, 12]),
    HAT_OPEN: (0xE4, [5, 6, 7, 8, 10, 12, 14]),
}
KICK_SWEEP = [(260, 2), (420, 3), (640, 5), (900, 8), (1023, 12)]  # (period, attenuation)


def read_var(data, i):
    value = 0
    while True:
        byte = data[i]
        i += 1
        value = (value << 7) | (byte & 0x7F)
        if not byte & 0x80:
            return value, i


def read_midi(path):
    """Ticks per beat, the first tempo in microseconds per beat, and each
    track as {'channel', 'volume', 'notes': [(start, end, pitch, velocity)]}."""
    data = open(path, 'rb').read()
    if data[0:4] != b'MThd':
        sys.exit('%s: not a MIDI file' % path)
    count, division = struct.unpack('>HH', data[10:14])
    if division & 0x8000:
        sys.exit('%s: SMPTE time is not supported' % path)
    tempo = None
    tracks = []
    i = 8 + struct.unpack('>I', data[4:8])[0]
    for _ in range(count):
        if data[i:i + 4] != b'MTrk':
            sys.exit('%s: bad track' % path)
        end = i + 8 + struct.unpack('>I', data[i + 4:i + 8])[0]
        i += 8
        now, status = 0, 0
        track = {'channel': None, 'volume': 127, 'notes': []}
        sounding = {}
        while i < end:
            delta, i = read_var(data, i)
            now += delta
            if data[i] & 0x80:
                status = data[i]
                i += 1
            if status == 0xFF:
                kind = data[i]
                length, i = read_var(data, i + 1)
                if kind == 0x51 and tempo is None:
                    tempo = int.from_bytes(data[i:i + 3], 'big')
                i += length
                continue
            if status in (0xF0, 0xF7):
                length, i = read_var(data, i)
                i += length
                continue
            kind, channel = status & 0xF0, status & 0x0F
            size = 1 if kind in (0xC0, 0xD0) else 2
            a, b = data[i], data[i + 1] if size == 2 else 0
            i += size
            if kind == 0x90 and b:
                track['channel'] = channel
                sounding[a] = (now, b)
            elif kind in (0x80, 0x90) and a in sounding:
                start, velocity = sounding.pop(a)
                track['notes'].append((start, now, a, velocity))
            elif kind == 0xB0 and a == 7:
                track['volume'] = b
        i = end
        if track['notes']:
            track['notes'].sort()
            tracks.append(track)
    return division, tempo or 500000, tracks


def carrier_level(velocity, volume):
    """Total level for the carriers: the note's velocity as in notes2vgm,
    less the channel volume's MIDI curve (40 log10) in 0.75 dB steps."""
    loss = quieter - 40 * math.log10(max(1, volume) / 127.0)
    return min(127, max(0, (127 - velocity) // 3) + int(round(loss / 0.75)))


def fm_note_on(vgm, channel, pitch, level):
    f = frequency(pitch)
    block = 0
    while block < 7 and 144 * f * 2 ** 20 / FM_CLOCK / 2 ** (block - 1) >= 2048:
        block += 1
    fnum = int(round(144 * f * 2 ** 20 / FM_CLOCK / 2 ** (block - 1)))
    vgm.fm(channel, 0xA4, (block << 3) | (fnum >> 8))
    vgm.fm(channel, 0xA0, fnum & 0xFF)
    for slot in CARRIERS:
        vgm.fm(channel, 0x40 + slot * 4, level)
    vgm.fm_global(0x28, 0xF0 | key_code(channel))


def psg_attenuation(attenuation):
    """An envelope's attenuation, turned down as the rest, in 2 dB steps."""
    return min(15, attenuation + int(round(quieter / 2.0)))


def psg_tone(vgm, channel, period, attenuation):
    vgm.psg(0x80 | (channel << 5) | (period & 0x0F))
    vgm.psg((period >> 4) & 0x3F)
    vgm.psg(0x90 | (channel << 5) | psg_attenuation(attenuation))


def main():
    global quieter
    if len(sys.argv) not in (3, 4, 5):
        sys.exit(__doc__)
    division, tempo, tracks = read_midi(sys.argv[1])
    bpm = float(sys.argv[3]) if len(sys.argv) >= 4 else 60e6 / tempo
    if len(sys.argv) == 5:
        quieter = float(sys.argv[4])

    def at(tick):
        return int(round(tick / division * 60.0 / bpm * VGM_RATE))

    drums = [t for t in tracks if t['channel'] == 9]
    melodic = [t for t in tracks if t['channel'] != 9]
    if len(melodic) > 5:
        sys.exit('%s: more than five melodic tracks' % sys.argv[1])
    bass = min(melodic, key=lambda t: min(n[2] for n in t['notes']))

    # Events: (time in samples, order, action); at the same time, notes
    # stop before others start.
    events = []
    for channel, track in enumerate(melodic):
        for start, end, pitch, velocity in track['notes']:
            level = carrier_level(velocity, track['volume'])
            events.append((at(start), 1, lambda v, c=channel, p=pitch, l=level: (fm_note_off(v, c), fm_note_on(v, c, p, l))))
            events.append((at(end), 0, lambda v, c=channel: fm_note_off(v, c)))
    hits = sorted((at(n[0]), n[2] != SNARE, n[2]) for t in drums for n in t['notes'])
    for time, _, pitch in hits:
        if pitch == KICK:
            for frame, (period, attenuation) in enumerate(KICK_SWEEP):
                events.append((time + frame * FRAME, 2, lambda v, p=period, a=attenuation: psg_tone(v, KICK_CHANNEL, p, a)))
            events.append((time + len(KICK_SWEEP) * FRAME, 2, lambda v: v.psg(0x90 | (KICK_CHANNEL << 5) | 15)))
    # One noise channel: a hat on a snare is left out, and each drum sounds
    # until the next one starts.
    noise = []
    for time, _, pitch in hits:
        if pitch in NOISE and not (noise and noise[-1][0] == time):
            noise.append((time, pitch))
    for i, (time, pitch) in enumerate(noise):
        control, envelope = NOISE[pitch]
        until = min(time + len(envelope) * FRAME, noise[i + 1][0] if i + 1 < len(noise) else time + len(envelope) * FRAME)
        events.append((time, 2, lambda v, c=control: v.psg(c)))
        for frame, attenuation in enumerate(envelope):
            if time + frame * FRAME < until:
                events.append((time + frame * FRAME, 3, lambda v, a=attenuation: v.psg(0xF0 | psg_attenuation(a))))
        if until == time + len(envelope) * FRAME:
            events.append((until, 3, lambda v: v.psg(0xFF)))
    events.sort(key=lambda e: (e[0], e[1]))

    # To the end of the bar the last note ends in, so that the loop keeps time.
    bar = division * 4
    length = at(-(-max(n[1] for t in tracks for n in t['notes']) // bar) * bar)

    vgm = Vgm()
    vgm.fm_global(0x22, 0)  # no LFO
    vgm.fm_global(0x27, 0)  # channel 3 in normal mode
    for channel, track in enumerate(melodic):
        set_instrument(vgm, channel, BASS if track is bass else KEYS)
    for channel in range(4):
        vgm.psg(0x90 | (channel << 5) | 15)
    loop_offset = len(vgm.data)

    now = 0
    for time, order, action in events:
        if time >= length:
            continue
        if time > now:
            vgm.wait(time - now)
            now = time
        action(vgm)
    vgm.wait(length - now)
    for channel in range(len(melodic)):
        fm_note_off(vgm, channel)
    vgm.data += b'\x66'

    header = bytearray(0x40)
    header[0:4] = b'Vgm '
    struct.pack_into('<I', header, 0x04, 0x40 + len(vgm.data) - 4)
    struct.pack_into('<I', header, 0x08, 0x150)
    struct.pack_into('<I', header, 0x0C, PSG_CLOCK)
    struct.pack_into('<I', header, 0x18, vgm.samples)
    struct.pack_into('<I', header, 0x1C, 0x40 + loop_offset - 0x1C)
    struct.pack_into('<I', header, 0x20, vgm.samples)
    struct.pack_into('<I', header, 0x24, 60)
    struct.pack_into('<H', header, 0x28, 0x0009)
    header[0x2A] = 16
    struct.pack_into('<I', header, 0x2C, FM_CLOCK)
    struct.pack_into('<I', header, 0x34, 0x40 - 0x34)
    open(sys.argv[2], 'wb').write(bytes(header) + bytes(vgm.data))
    print('%s: %d tracks, %.1f s at %g BPM' % (sys.argv[2], len(tracks), vgm.samples / VGM_RATE, bpm))


if __name__ == '__main__':
    main()
