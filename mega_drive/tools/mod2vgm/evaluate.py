#!/usr/bin/env python3
"""Score mod2vgm output against the original module, offline.

Renders the MOD with libxmp (the reference) and the converted VGM with the
cycle-accurate Nuked-OPN2 + SN76489 renderer (build/vgm_render), then prints:

  chroma  cosine similarity of 12-bin pitch-class energy (melody / harmony)
  mel     similarity of the log-mel spectrum shape (timbre / balance)
  env     correlation of the RMS envelope (dynamics)
  onset   correlation of the onset strength (rhythm), best lag in ms
  len     duration ratio VGM / MOD

Usage: evaluate.py [--secs N] [--keep] file.mod [file.mod ...]
"""
import ctypes
import os
import subprocess
import sys
import tempfile

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
CONV = os.environ.get("M2V_CONV", os.path.join(HERE, "build/mod2vgm"))
REND = os.path.join(HERE, "build/vgm_render")
SR = 22050


def render_mod(path, secs):
    xmp = ctypes.CDLL("libxmp.so.4")
    xmp.xmp_create_context.restype = ctypes.c_void_p
    xmp.xmp_load_module.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    xmp.xmp_start_player.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
    xmp.xmp_play_buffer.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
    ctx = xmp.xmp_create_context()
    if xmp.xmp_load_module(ctx, path.encode()) != 0:
        raise RuntimeError("libxmp cannot load " + path)
    xmp.xmp_start_player(ctx, 44100, 0)
    buf = ctypes.create_string_buffer(44100 * 4)
    out = []
    total = 0
    while total < secs * 44100 * 4:
        if xmp.xmp_play_buffer(ctx, buf, len(buf), 1) != 0:
            break
        out.append(buf.raw)
        total += len(buf)
    # loop=1 stops at the first repeat, so a shorter result is a full pass
    a = np.frombuffer(b"".join(out), dtype=np.int16).astype(np.float32)
    return (a[0::2] + a[1::2]) * 0.5, 44100


def read_wav(path):
    d = open(path, "rb").read()
    sr = int.from_bytes(d[24:28], "little")
    a = np.frombuffer(d[44:], dtype=np.int16).astype(np.float32)
    return (a[0::2] + a[1::2]) * 0.5, sr


def resample(x, sr):
    n = int(len(x) * SR / sr)
    return np.interp(np.linspace(0, len(x) - 1, n), np.arange(len(x)), x)


def stft_mag(x, n=1024, hop=512):
    w = np.hanning(n)
    f = (len(x) - n) // hop
    idx = np.arange(n)[None, :] + hop * np.arange(f)[:, None]
    return np.abs(np.fft.rfft(x[idx] * w, axis=1))


def chroma(mag):
    freqs = np.fft.rfftfreq(1024, 1.0 / SR)
    ok = (freqs > 60) & (freqs < 2000)
    pc = np.round(12 * np.log2(freqs[ok] / 440.0) + 69).astype(int) % 12
    c = np.zeros((mag.shape[0], 12))
    for k in range(12):
        c[:, k] = (mag[:, ok][:, pc == k] ** 2).sum(axis=1)
    return c


def mel(mag, bands=24):
    freqs = np.fft.rfftfreq(1024, 1.0 / SR)
    edges = np.geomspace(60, 10000, bands + 1)
    m = np.zeros((mag.shape[0], bands))
    for k in range(bands):
        sel = (freqs >= edges[k]) & (freqs < edges[k + 1])
        m[:, k] = (mag[:, sel] ** 2).sum(axis=1)
    return m


def cosine_rows(a, b):
    num = (a * b).sum(axis=1)
    den = np.linalg.norm(a, axis=1) * np.linalg.norm(b, axis=1) + 1e-12
    return num / den


def score(ref, test):
    n = min(len(ref), len(test))
    ref, test = ref[:n], test[:n]
    ref = ref / (np.abs(ref).max() + 1e-9)
    test = test / (np.abs(test).max() + 1e-9)
    mr, mt = stft_mag(ref), stft_mag(test)
    f = min(len(mr), len(mt))
    mr, mt = mr[:f], mt[:f]
    cr, ct = chroma(mr), chroma(mt)
    live = (cr.sum(axis=1) > 1e-3 * cr.sum(axis=1).max()) & (ct.sum(axis=1) > 1e-3 * ct.sum(axis=1).max())
    chroma_s = cosine_rows(cr, ct)[live].mean() if live.any() else 0.0
    lr, lt = np.log10(mel(mr) + 1e-6), np.log10(mel(mt) + 1e-6)
    lr -= lr.mean(); lt -= lt.mean()
    mel_s = float((cosine_rows(lr, lt)).mean())
    er = np.sqrt((mr ** 2).sum(axis=1)); et = np.sqrt((mt ** 2).sum(axis=1))
    env_s = float(np.corrcoef(er, et)[0, 1])
    orr = np.maximum(0, np.diff(np.log1p(er * 20))); ot = np.maximum(0, np.diff(np.log1p(et * 20)))
    best, lag = -1, 0
    for l in range(-8, 9):
        a = orr[max(0, l):len(orr) + min(0, l)]
        b = ot[max(0, -l):len(ot) + min(0, -l)]
        k = min(len(a), len(b))
        c = np.corrcoef(a[:k], b[:k])[0, 1]
        if c > best:
            best, lag = c, l
    return chroma_s, mel_s, env_s, best, lag * 512 * 1000 // SR


def main(argv):
    secs = 60
    keep = False
    files = []
    i = 0
    while i < len(argv):
        if argv[i] == "--secs":
            secs = int(argv[i + 1]); i += 2
        elif argv[i] == "--keep":
            keep = True; i += 1
        else:
            files.append(argv[i]); i += 1
    tot = np.zeros(4)
    for path in files:
        tmp = tempfile.mkdtemp(prefix="m2v_")
        vgm = os.path.join(tmp, "o.vgm"); wav = os.path.join(tmp, "o.wav")
        subprocess.run([CONV, path, vgm], check=True, stdout=subprocess.DEVNULL)
        subprocess.run([REND, vgm, wav, str(secs)], check=True, stdout=subprocess.DEVNULL)
        ref, rsr = render_mod(path, secs)
        tst, tsr = read_wav(wav)
        ref = resample(ref[: rsr * secs], rsr)
        tst = resample(tst[: tsr * secs], tsr)
        c, m, e, o, lag = score(ref, tst)
        tot += (c, m, e, o)
        print("%-34s chroma %.3f  mel %.3f  env %.3f  onset %.3f (lag %+4d ms)  len %.2f"
              % (os.path.basename(path), c, m, e, o, lag, len(tst) / max(1, len(ref))))
        if keep:
            print("   kept", tmp)
            np.save(os.path.join(tmp, "ref.npy"), ref); np.save(os.path.join(tmp, "tst.npy"), tst)
    if len(files) > 1:
        t = tot / len(files)
        print("%-34s chroma %.3f  mel %.3f  env %.3f  onset %.3f" % ("MEAN", *t))


if __name__ == "__main__":
    main(sys.argv[1:])
