#!/usr/bin/env python3
"""Measure a captured Mega Drive frame for the "white stripes" pattern.

Prints numbers, not opinions: how many colours are on screen, how many colour
changes a scanline has, and whether the pattern is a function of x only (vertical
stripes) or of both axes (art). A solid fill screen must report 1-2 colours and
0-1 changes per row; the striped screens reported earlier show many colours and
a fixed period in x.

Usage: tools/stripe_metric.py capture.png [--crop x,y,w,h]
"""
import sys

import numpy as np
from PIL import Image


def main(argv):
    path = argv[1]
    img = Image.open(path).convert("RGB")
    a = np.asarray(img)
    if "--crop" in argv:
        x, y, w, h = (int(v) for v in argv[argv.index("--crop") + 1].split(","))
        a = a[y:y + h, x:x + w]
    # integer downscale to the game's 320x224 when the capture is a clean multiple
    h, w = a.shape[:2]
    for s in (3, 2):
        if (w, h) in ((320 * s, 224 * s), (320 * s, 240 * s)):
            a = a[::s, ::s]
            break
    h, w = a.shape[:2]
    cols = a.reshape(-1, 3)
    uniq, counts = np.unique(cols, axis=0, return_counts=True)
    order = np.argsort(-counts)
    changes = (a[:, 1:] != a[:, :-1]).any(axis=2).sum(axis=1)
    # vertical uniformity: does row y equal row y+1?
    rowdup = (a[1:] == a[:-1]).all(axis=2).mean()
    print(f"{path}: {w}x{h} game pixels, {len(uniq)} distinct colours")
    print("  top colours: " + ", ".join(
        f"{tuple(int(v) for v in uniq[i])}:{100*counts[i]//len(cols)}%"
        for i in order[:6]))
    print(f"  colour changes per row: min={changes.min()} "
          f"median={int(np.median(changes))} max={changes.max()}")
    print(f"  consecutive rows identical: {100*rowdup:.0f}%")
    stripes = np.median(changes) >= 8 and len(uniq) >= 6
    print("  verdict: " + ("VERTICAL STRIPES (pattern depends on x only)"
                           if stripes and rowdup > 0.5 else
                           "no stripe pattern" if not stripes else "mixed"))


if __name__ == "__main__":
    main(sys.argv)
