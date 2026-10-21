#!/usr/bin/env python3
"""Turn a transcription's note events into a VGM for the Mega Drive's chips.

Reads the CSV Basic Pitch writes (start, end, MIDI pitch, velocity, ...),
puts the notes on a grid of sixteenths at a given tempo, deals them out to
five FM channels and the three square waves of the PSG, and writes a VGM
that loops, for rescomp's XGM2 resource. FM channel 6 is left free: the
sound driver plays its samples there. Only the standard library is used.

The lowest note sounding has an FM channel of its own, as the bass. The
rest go to whichever voice is free, the one that last played the nearest
pitch first; the PSG only takes notes high enough for it. What finds no
voice is dropped, the quietest first.

The track lasts as long as the original's recording, given in seconds, or
else to the end of the bar its last note ends in. A track to be played once
has no loop.

usage: notes2vgm.py NOTES.csv OUT.vgm BPM [SECONDS [once]]
"""

import csv
import math
import struct
import sys

VGM_RATE = 44100
FM_CLOCK = 7670453
PSG_CLOCK = 3579545
BASS_BELOW = 55   # a lowest note under G3 is the bass
PSG_FROM = 60     # the PSG takes notes from middle C up
FM_VOICES = [1, 2, 3, 4]  # channels for the rest; 0 is the bass, 5 the samples'
PSG_VOICES = [0, 1, 2]

# FM instruments: algorithm 4, two modulator-carrier pairs. Each operator is
# (detune/multiple, total level, attack, decay 1, decay 2, sustain/release),
# in register order: operator 1, 3, 2, 4. The carriers' levels are set by
# each note's velocity.
BASS = (0x30 | 4, [(0x01, 24, 31, 9, 0, 0x2F), (0x02, 38, 31, 11, 0, 0x3F),
                   (0x01, 0, 31, 6, 2, 0x2A), (0x01, 0, 31, 5, 2, 0x2A)])
KEYS = (0x18 | 4, [(0x02, 34, 31, 8, 0, 0x2F), (0x01, 44, 31, 10, 0, 0x3F),
                   (0x01, 0, 31, 7, 3, 0x37), (0x02, 0, 31, 6, 3, 0x37)])
CARRIERS = (2, 3)  # their places in the lists above


def frequency(pitch):
    return 440.0 * 2 ** ((pitch - 69) / 12.0)


class Vgm:
    def __init__(self):
        self.data = bytearray()
        self.samples = 0

    def fm(self, channel, register, value):
        port = 0x52 if channel < 3 else 0x53
        self.data += bytes((port, register + (channel % 3 if register >= 0x30 else 0), value & 0xFF))

    def fm_global(self, register, value):
        self.data += bytes((0x52, register, value & 0xFF))

    def psg(self, value):
        self.data += bytes((0x50, value & 0xFF))

    def wait(self, samples):
        self.samples += samples
        while samples > 0:
            part = min(samples, 65535)
            self.data += bytes((0x61,)) + struct.pack('<H', part)
            samples -= part


def set_instrument(vgm, channel, instrument):
    feedback_algorithm, operators = instrument
    for slot, (dt_mul, tl, ar, d1r, d2r, sl_rr) in enumerate(operators):
        offset = slot * 4
        vgm.fm(channel, 0x30 + offset, dt_mul)
        vgm.fm(channel, 0x40 + offset, tl if slot not in CARRIERS else 127)
        vgm.fm(channel, 0x50 + offset, ar)
        vgm.fm(channel, 0x60 + offset, d1r)
        vgm.fm(channel, 0x70 + offset, d2r)
        vgm.fm(channel, 0x80 + offset, sl_rr)
        vgm.fm(channel, 0x90 + offset, 0)
    vgm.fm(channel, 0xB0, feedback_algorithm)
    vgm.fm(channel, 0xB4, 0xC0)


def key_code(channel):
    return channel if channel < 3 else channel + 1


def fm_note_on(vgm, channel, pitch, velocity):
    f = frequency(pitch)
    block = 0
    while block < 7 and 144 * f * 2 ** 20 / FM_CLOCK / 2 ** (block - 1) >= 2048:
        block += 1
    fnum = int(round(144 * f * 2 ** 20 / FM_CLOCK / 2 ** (block - 1)))
    vgm.fm(channel, 0xA4, (block << 3) | (fnum >> 8))
    vgm.fm(channel, 0xA0, fnum & 0xFF)
    level = max(0, (127 - velocity) // 3)
    for slot in CARRIERS:
        vgm.fm(channel, 0x40 + slot * 4, level)
    vgm.fm_global(0x28, 0xF0 | key_code(channel))


def fm_note_off(vgm, channel):
    vgm.fm_global(0x28, key_code(channel))


def psg_note_on(vgm, channel, pitch, velocity):
    period = max(1, min(1023, int(round(PSG_CLOCK / (32 * frequency(pitch))))))
    vgm.psg(0x80 | (channel << 5) | (period & 0x0F))
    vgm.psg((period >> 4) & 0x3F)
    vgm.psg(0x90 | (channel << 5) | max(1, min(15, (127 - velocity) // 10)))


def psg_note_off(vgm, channel):
    vgm.psg(0x90 | (channel << 5) | 15)


def main():
    if len(sys.argv) not in (4, 5, 6) or (len(sys.argv) == 6 and sys.argv[5] != 'once'):
        sys.exit(__doc__)
    loops = len(sys.argv) != 6
    bpm = float(sys.argv[3])
    step = 60.0 / bpm / 4
    rows = list(csv.reader(open(sys.argv[1], encoding='utf-8')))[1:]
    notes = []
    for row in rows:
        start = int(round(float(row[0]) / step))
        end = max(start + 1, int(round(float(row[1]) / step)))
        notes.append((start, end, int(row[2]), int(row[3])))
    # As long as the recording, or to a whole number of bars, so that the
    # loop keeps time.
    if len(sys.argv) >= 5:
        length = int(round(float(sys.argv[4]) / step))
        notes = [(start, min(end, length), pitch, velocity) for start, end, pitch, velocity in notes if start < length]
    else:
        length = -(-max(end for _, end, _, _ in notes) // 16) * 16

    # Which note each voice plays, step by step: ('fm', channel) or ('psg', channel).
    voices = [('fm', 0)] + [('fm', c) for c in FM_VOICES] + [('psg', c) for c in PSG_VOICES]
    busy_until = {voice: 0 for voice in voices}
    last_pitch = {voice: None for voice in voices}
    events = {}  # step -> list of (kind, voice, pitch, velocity)
    dropped = 0

    by_start = {}
    for note in notes:
        by_start.setdefault(note[0], []).append(note)
    for start in sorted(by_start):
        group = sorted(by_start[start], key=lambda n: (-n[3]))
        sounding = [n for n in notes if n[0] <= start < n[1]]
        lowest = min(n[2] for n in sounding)
        for begin, end, pitch, velocity in group:
            if pitch == lowest and pitch < BASS_BELOW:
                choices = [('fm', 0)]
            else:
                choices = [v for v in voices[1:] if v[0] == 'fm' or pitch >= PSG_FROM]
            free = [v for v in choices if busy_until[v] <= begin]
            if not free:
                dropped += 1
                continue
            voice = min(free, key=lambda v: abs((last_pitch[v] or pitch) - pitch))
            busy_until[voice] = end
            last_pitch[voice] = pitch
            events.setdefault(begin, []).append(('on', voice, pitch, velocity))
            events.setdefault(end, []).append(('off', voice, pitch, velocity))

    vgm = Vgm()
    vgm.fm_global(0x22, 0)  # no LFO
    vgm.fm_global(0x27, 0)  # channel 3 in normal mode
    for channel in range(5):
        set_instrument(vgm, channel, BASS if channel == 0 else KEYS)
    for channel in range(4):
        vgm.psg(0x90 | (channel << 5) | 15)
    loop_offset = len(vgm.data)
    loop_samples_start = vgm.samples

    step_samples = int(round(step * VGM_RATE))
    playing = {}
    for position in range(length):
        # Notes ending here stop first, unless the voice starts again.
        for kind, voice, pitch, velocity in sorted(events.get(position, []), key=lambda e: e[0] != 'off'):
            if kind == 'off':
                if playing.get(voice) == pitch:
                    (fm_note_off if voice[0] == 'fm' else psg_note_off)(vgm, voice[1])
                    del playing[voice]
            else:
                if voice[0] == 'fm':
                    fm_note_off(vgm, voice[1])
                    fm_note_on(vgm, voice[1], pitch, velocity)
                else:
                    psg_note_on(vgm, voice[1], pitch, velocity)
                playing[voice] = pitch
        vgm.wait(step_samples)
    for voice in list(playing):
        (fm_note_off if voice[0] == 'fm' else psg_note_off)(vgm, voice[1])
    vgm.data += b'\x66'

    header = bytearray(0x40)
    header[0:4] = b'Vgm '
    struct.pack_into('<I', header, 0x04, 0x40 + len(vgm.data) - 4)
    struct.pack_into('<I', header, 0x08, 0x150)
    struct.pack_into('<I', header, 0x0C, PSG_CLOCK)
    struct.pack_into('<I', header, 0x18, vgm.samples)
    if loops:
        struct.pack_into('<I', header, 0x1C, 0x40 + loop_offset - 0x1C)
        struct.pack_into('<I', header, 0x20, vgm.samples - loop_samples_start)
    struct.pack_into('<I', header, 0x24, 60)
    struct.pack_into('<H', header, 0x28, 0x0009)
    header[0x2A] = 16
    struct.pack_into('<I', header, 0x2C, FM_CLOCK)
    struct.pack_into('<I', header, 0x34, 0x40 - 0x34)
    open(sys.argv[2], 'wb').write(bytes(header) + bytes(vgm.data))
    print('%s: %d notes, %d dropped, %.1f s' % (sys.argv[2], len(notes), dropped, vgm.samples / VGM_RATE))


if __name__ == '__main__':
    main()
