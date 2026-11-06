#!/usr/bin/env python3
"""Offline check for the transient-effect sprites (src/effects.c).

The PNG in res/ and the SPRITE line in res/resources.res must agree exactly:
rescomp slices the image by the declared frame size and stops at the declared
frame count, so a one-pixel mismatch (or a wrong frame count) silently shifts
or drops frames — the failure mode the intro logos already had.

Checks every SPRITE whose name starts with fx_:
  png width  == frames * frame_w
  png height == frame_h
  and the animation list in the SPRITE line has `frames` entries.
"""
import os
import re
import sys
from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RES = os.path.join(ROOT, "res")


def main():
    src = open(os.path.join(RES, "resources.res")).read()
    problems = 0
    seen = 0
    for m in re.finditer(r'^SPRITE\s+(\w+)\s+"([^"]+)"\s+(\d+)p\s+(\d+)p'
                         r'\s+\w+\s+\[(.*)\]\s*$', src, re.M):
        name, png, fw, fh, anims = m.groups()
        if not name.startswith("fx_"):
            continue
        seen += 1
        fw, fh = int(fw), int(fh)
        # rescomp: every timing entry is one frame (the inner brackets only
        # group animations over the same frames), so [[4,4,4]] is 3 frames.
        nframes = len(re.findall(r"\d+", anims))
        path = os.path.join(RES, png)
        if not os.path.exists(path):
            print(f"  MISSING {png}")
            problems += 1
            continue
        im = Image.open(path)
        want = (fw * nframes, fh)
        ok = im.size == want
        print(f"  {'ok ' if ok else 'BAD'} {name}: {png} {im.size}"
              f" declared {nframes} x {fw}x{fh} -> {want}")
        if not ok:
            problems += 1
    if not seen:
        print("  no fx_ sprites found in res/resources.res")
        return 1
    print(f"effect sprites: {seen} checked, {problems} problem(s)")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
