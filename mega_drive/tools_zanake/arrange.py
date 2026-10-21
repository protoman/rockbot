#!/usr/bin/env python3
"""Arrange a transcription into a MIDI file like a hand-made one.

Reads the notes Basic Pitch wrote for a recording (start, end, MIDI pitch,
velocity, ...) and the recording itself, and writes a MIDI file for
midi2vgm.py with fixed parts, each one note at a time and evenly loud:

- the notes go on a grid of sixteenths at the given tempo, counted from the
  given offset in seconds; the quietest (under MIN_VELOCITY) are left out;
- the lowest note of each chord, if low enough and not above a bass note
  still sounding, is the bass; the highest,
  if high enough, the lead; the rest go to three middle parts, to the one
  that last played the nearest pitch. A part's new note ends its last one;
  a middle note with no part free ends the one that began first;
- the drums are found in the recording's percussive part (librosa's
  harmonic-percussive separation): low hits are kicks, high ones with a
  body are snares, high ones without are hats. A drum loop repeats, so each
  hit's strength is averaged over every LOOP_BARS bars.

Run once with the Python environment that has Basic Pitch (out/venv); the
MIDI file it writes is the track's source from then on, and can be touched
up by hand.

usage: arrange.py NOTES.csv RECORDING.wav OUT.mid BPM OFFSET BARS
"""

import csv
import sys

import librosa
import mido
import numpy as np

MIN_VELOCITY = 45
BASS_BELOW = 45
LEAD_FROM = 58
LOOP_BARS = 4
TICKS = 96  # per beat
KICK, SNARE, HAT = 35, 38, 42
VOLUMES = {'Bass': 127, 'Lead': 115, 'Middle': 97}  # as the cave's MIDI


def read_notes(path, step, offset, length):
    notes = []
    for row in list(csv.reader(open(path, encoding='utf-8')))[1:]:
        if int(row[3]) < MIN_VELOCITY:
            continue
        start = int(round((float(row[0]) - offset) / step))
        end = max(start + 1, int(round((float(row[1]) - offset) / step)))
        if 0 <= start < length:
            notes.append((start, min(end, length), int(row[2])))
    return notes


def arrange(notes):
    """Each part's notes as (start, end, pitch), one at a time."""
    parts = {'Bass': [], 'Lead': [], 'Middle 1': [], 'Middle 2': [], 'Middle 3': []}
    middles = ['Middle 1', 'Middle 2', 'Middle 3']

    def play(part, start, end, pitch):
        line = parts[part]
        if line and line[-1][1] > start:
            line[-1] = (line[-1][0], start, line[-1][2])
            if line[-1][0] >= start:
                line.pop()
        line.append((start, end, pitch))

    by_start = {}
    for start, end, pitch in notes:
        by_start.setdefault(start, {})[pitch] = max(end, by_start.get(start, {}).get(pitch, 0))
    for start in sorted(by_start):
        chord = sorted(by_start[start].items())
        bass = parts['Bass']
        if chord[0][0] < BASS_BELOW and (not bass or bass[-1][1] <= start or chord[0][0] <= bass[-1][2]):
            play('Bass', start, chord[0][1], chord[0][0])
            chord = chord[1:]
        if chord and chord[-1][0] >= LEAD_FROM:
            play('Lead', start, chord[-1][1], chord[-1][0])
            chord = chord[:-1]
        for pitch, end in chord:
            free = [p for p in middles if not parts[p] or parts[p][-1][1] <= start]
            if free:
                part = min(free, key=lambda p: abs(parts[p][-1][2] - pitch) if parts[p] else 0)
            else:
                part = min(middles, key=lambda p: parts[p][-1][0])
            play(part, start, end, pitch)
    return parts


def find_drums(recording, step, offset, length):
    """Drum hits as (step, pitch)."""
    y, rate = librosa.load(recording, sr=22050)
    _, percussive = librosa.effects.hpss(y, margin=3.0)
    spectrum = np.abs(librosa.stft(percussive, n_fft=1024, hop_length=128))
    frequencies = librosa.fft_frequencies(sr=rate, n_fft=1024)
    times = librosa.times_like(spectrum, sr=rate, hop_length=128)
    loop = LOOP_BARS * 16
    strength = {}
    for band, (low, high) in {'low': (20, 150), 'body': (150, 2000), 'high': (4000, 11000)}.items():
        energy = spectrum[(frequencies >= low) & (frequencies < high)].sum(0)
        rise = np.maximum(0, np.diff(energy, prepend=energy[0]))
        hits = np.zeros(length)
        for k in range(length):
            near = np.abs(times - (offset + k * step)) < 0.03
            hits[k] = rise[near].max() if near.any() else 0
        hits /= np.percentile(hits, 95)
        # The loop's average, over the whole loops in the recording.
        whole = length // loop * loop
        strength[band] = hits[:whole].reshape(-1, loop).mean(0)
    drums = []
    for k in range(length):
        position = k % loop
        if strength['low'][position] > 0.5:
            drums.append((k, KICK))
        if strength['high'][position] > 0.5 and strength['body'][position] > 0.5:
            drums.append((k, SNARE))
        elif strength['high'][position] > 0.3:
            drums.append((k, HAT))
    return drums


def write_midi(path, bpm, parts, drums):
    tick = TICKS // 4
    midi = mido.MidiFile(ticks_per_beat=TICKS)
    meta = mido.MidiTrack()
    meta.append(mido.MetaMessage('set_tempo', tempo=int(round(60e6 / bpm))))
    midi.tracks.append(meta)
    lines = [(name, i, VOLUMES[name.split()[0]], notes) for i, (name, notes) in enumerate(parts.items())]
    lines.append(('Drums', 9, 127, [(k, k + 1, pitch) for k, pitch in drums]))
    for name, channel, volume, notes in lines:
        track = mido.MidiTrack()
        track.append(mido.MetaMessage('track_name', name=name))
        track.append(mido.Message('control_change', channel=channel, control=7, value=volume))
        events = []
        for start, end, pitch in notes:
            events.append((start * tick, 1, pitch))
            events.append((end * tick, 0, pitch))
        now = 0
        for time, on, pitch in sorted(events):
            track.append(mido.Message('note_on' if on else 'note_off', channel=channel, note=pitch,
                                      velocity=127 if on else 0, time=time - now))
            now = time
        midi.tracks.append(track)
    midi.save(path)


def main():
    if len(sys.argv) != 7:
        sys.exit(__doc__)
    bpm, offset, bars = float(sys.argv[4]), float(sys.argv[5]), int(sys.argv[6])
    step = 60.0 / bpm / 4
    length = bars * 16
    notes = read_notes(sys.argv[1], step, offset, length)
    parts = arrange(notes)
    drums = find_drums(sys.argv[2], step, offset, length)
    write_midi(sys.argv[3], bpm, parts, drums)
    print('%s: %s, %d drum hits' % (sys.argv[3], ', '.join('%s %d' % (n, len(p)) for n, p in parts.items()), len(drums)))


if __name__ == '__main__':
    main()
