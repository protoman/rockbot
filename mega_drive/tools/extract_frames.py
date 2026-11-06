#!/usr/bin/env python3
"""Split an `emu_record` video into per-frame PNGs and label each frame.

Frames are only kept when their geometry is a plain multiple of the 320x224
screen (see AGENTS.md "Captures are evidence only if they are game frames"), and
each frame gets a row-composition score so the room-transition frames can be
picked out of a burst:

  seam[y] = sum |row(y) - row(y-1)| over the frame
  transition frames are the ones where a *single* large seam moves through the
  frame over consecutive frames (a slide), as opposed to static content.

Usage: tools/extract_frames.py capture.mkv outdir/
Prints one line per frame: index, kept/dropped, seam row, seam strength.
"""
import os
import subprocess
import sys

from PIL import Image
import numpy as np

VALID = {(320, 224), (320, 240), (640, 448), (640, 480), (960, 672), (960, 720)}


def main(argv):
    if len(argv) != 3:
        sys.exit("usage: tools/extract_frames.py <capture.mkv> <outdir>")
    video, outdir = argv[1], argv[2]
    os.makedirs(outdir, exist_ok=True)
    pattern = os.path.join(outdir, "f%04d.png")
    subprocess.run(["ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
                    "-i", video, pattern], check=True)
    frames = sorted(f for f in os.listdir(outdir) if f.startswith("f"))
    kept, dropped = 0, 0
    seams = []
    for name in frames:
        path = os.path.join(outdir, name)
        im = Image.open(path)
        size = im.size
        if size not in VALID:
            os.remove(path)
            dropped += 1
            print(f"{name}: dropped ({size[0]}x{size[1]})")
            continue
        kept += 1
        a = np.asarray(im.convert("RGB"), dtype=int)
        k = max(1, a.shape[0] // 224)
        a = a[::k, ::k][:224, :320]
        rows = a[1:] - a[:-1]
        score = np.abs(rows).sum(axis=(1, 2))          # per row-pair change
        y = int(score.argmax())
        s = int(score[y])
        seams.append((name, y, s))
    print(f"\nkept {kept} frames, dropped {dropped}")
    print("frame   seam_y  seam_strength")
    for name, y, s in seams:
        mark = "  <-- slide" if s > 40000 else ""
        print(f"{name}  {y:5d}  {s:10d}{mark}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
