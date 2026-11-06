#!/usr/bin/env python3
"""Validate a batch of emulator captures before trusting them as evidence.

A capture is only game data if the window kept the game's geometry: on
2026-10-25 a burst of "transition" frames came out 1182x494 because the window
manager had resized/dialog-focused something else, and they looked like a black
screen with garbage edges - a false bug report waiting to happen. `emu_shot`
now refuses such captures and writes a `.size` sidecar; this tool does the same
check on a whole directory (and on captures taken before that guard existed).

Usage: tools/check_captures.py [dir-or-glob]
       default: /tmp/opencode/md-emu/*.png
"""
import glob
import os
import sys

from PIL import Image

VALID = {(320, 224), (320, 240), (640, 448), (640, 480), (960, 672), (960, 720)}


def main(argv):
    pattern = argv[1] if len(argv) > 1 else "/tmp/opencode/md-emu/*.png"
    files = sorted(glob.glob(pattern))
    good, bad = [], []
    for f in files:
        try:
            size = Image.open(f).size
        except OSError:
            continue
        (good if size in VALID else bad).append((os.path.basename(f), size))
    print(f"{len(good)} usable, {len(bad)} rejected")
    for name, size in bad:
        print(f"  reject {name}: {size[0]}x{size[1]} (not a game window size)")
    if bad:
        print("\nRe-run those frames with the window untouched; emu_shot writes a"
              " .size sidecar and drops alien captures since 2026-10-25.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
