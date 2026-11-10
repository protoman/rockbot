#!/usr/bin/env python3
import sys
import os
import subprocess
import struct
import numpy as np

BASE_DIR = "/home/iuri/Desenvolvimento/rockbot/mega_drive/tools/mod2vgm"
MOD_FILE = sys.argv[1] if len(sys.argv) > 1 else "/home/iuri/Desenvolvimento/rockbot/build/games/Rockbot1/music/rockbot_stage_complete_00.mod"
VGM_FILE = os.path.join(BASE_DIR, "stage_complete.vgm")
WAV_VGM = os.path.join(BASE_DIR, "stage_complete_vgm.wav")
WAV_MOD = os.path.join(BASE_DIR, "stage_complete_mod.wav")

def render_mod_to_wav(mod_path, wav_path):
    import ctypes
    libxmp = ctypes.CDLL('libxmp.so.4')
    libxmp.xmp_create_context.restype = ctypes.c_void_p
    libxmp.xmp_load_module.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    libxmp.xmp_load_module.restype = ctypes.c_int
    libxmp.xmp_start_player.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
    libxmp.xmp_play_buffer.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
    libxmp.xmp_play_buffer.restype = ctypes.c_int

    ctx = libxmp.xmp_create_context()
    libxmp.xmp_load_module(ctx, mod_path.encode('utf-8'))
    libxmp.xmp_start_player(ctx, 44100, 0)

    buf_size = 4096
    buf = ctypes.create_string_buffer(buf_size)
    frames = []
    while True:
        ret = libxmp.xmp_play_buffer(ctx, buf, buf_size, 0)
        if ret != 0:
            break
        frames.append(buf.raw)
        if len(frames) > 200:
            break

    audio_bytes = b''.join(frames)
    wav_header = struct.pack('<4sI4s4sIHHIIHH4sI',
        b'RIFF', 36 + len(audio_bytes), b'WAVE',
        b'fmt ', 16, 1, 2, 44100, 44100 * 4, 4, 16,
        b'data', len(audio_bytes)
    )
    with open(wav_path, 'wb') as f:
        f.write(wav_header + audio_bytes)

def load_wav(path):
    with open(path, 'rb') as f:
        data = f.read()
    # Read 16-bit stereo PCM from offset 44
    samples = np.frombuffer(data[44:], dtype=np.int16).astype(np.float32)
    # Average to mono
    mono = (samples[0::2] + samples[1::2]) * 0.5
    return mono

def main():
    print(f"=== Converting: {os.path.basename(MOD_FILE)} ===")
    cmd_conv = [os.path.join(BASE_DIR, "build/mod2vgm"), MOD_FILE, VGM_FILE]
    subprocess.check_call(cmd_conv)

    print("=== Rendering VGM via cycle-accurate Nuked-OPN2 ===")
    cmd_render = [os.path.join(BASE_DIR, "build/vgm_render"), VGM_FILE, WAV_VGM]
    subprocess.check_call(cmd_render)

    print("=== Rendering Reference MOD audio via libxmp ===")
    render_mod_to_wav(MOD_FILE, WAV_MOD)

    sig_mod = load_wav(WAV_MOD)
    sig_vgm = load_wav(WAV_VGM)

    # Trim to matching length
    min_len = min(len(sig_mod), len(sig_vgm))
    s1 = sig_mod[:min_len]
    s2 = sig_vgm[:min_len]

    # Normalize
    max1 = np.max(np.abs(s1)) + 1e-6
    max2 = np.max(np.abs(s2)) + 1e-6
    s1_norm = s1 / max1
    s2_norm = s2 / max2

    # Spectral comparison (FFT)
    fft_mod = np.abs(np.fft.rfft(s1_norm))
    fft_vgm = np.abs(np.fft.rfft(s2_norm))

    freqs = np.fft.rfftfreq(min_len, 1.0 / 44100.0)

    # Energy bands: Sub (0-150Hz), Low (150-500Hz), Mid (500-2000Hz), High (2000-8000Hz), VeryHigh (>8000Hz)
    bands = [
        ("Sub-bass (0-150 Hz)", (freqs >= 0) & (freqs < 150)),
        ("Low-mid  (150-500 Hz)", (freqs >= 150) & (freqs < 500)),
        ("Midrange (500-2 kHz)", (freqs >= 500) & (freqs < 2000)),
        ("High-mid (2k-8 kHz)", (freqs >= 2000) & (freqs < 8000)),
        ("Treble   (8k-20 kHz)", (freqs >= 8000))
    ]

    print("\n--- Spectral Energy Distribution (MOD vs VGM) ---")
    mod_total = np.sum(fft_mod**2) + 1e-9
    vgm_total = np.sum(fft_vgm**2) + 1e-9

    for name, mask in bands:
        e_mod = np.sum(fft_mod[mask]**2) / mod_total * 100.0
        e_vgm = np.sum(fft_vgm[mask]**2) / vgm_total * 100.0
        diff = e_vgm - e_mod
        print(f"{name:22s} | MOD: {e_mod:5.1f}% | VGM: {e_vgm:5.1f}% | Diff: {diff:+5.1f}%")

    # Pearson correlation of envelope
    hop = 1024
    frames = min_len // hop
    env_mod = np.array([np.sqrt(np.mean(s1_norm[i*hop:(i+1)*hop]**2)) for i in range(frames)])
    env_vgm = np.array([np.sqrt(np.mean(s2_norm[i*hop:(i+1)*hop]**2)) for i in range(frames)])
    corr = np.corrcoef(env_mod, env_vgm)[0, 1]

    print(f"\nRhythmic Envelope Correlation: {corr*100.0:.1f}%")
    print(f"Original MOD WAV: {WAV_MOD}")
    print(f"Converted VGM WAV: {WAV_VGM}")

if __name__ == "__main__":
    main()
