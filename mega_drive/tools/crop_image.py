#!/usr/bin/env python3
"""Crop a rectangle out of an image (build helper for the four-line pipeline).

The Mega Drive gives each 8x8 tile ONE palette line, so large art must be
split spatially into parts that each own a line (never colour-cluster, that
causes the "square patch" bug). title64.py --part has no per-part crop, so a
band that needs its own line is cut here first.

Usage: crop_image.py IN OUT X Y W H
"""
import sys

from PIL import Image

if len(sys.argv) != 7:
    sys.exit(__doc__)

src, dst = sys.argv[1], sys.argv[2]
x, y, w, h = (int(v) for v in sys.argv[3:])
Image.open(src).crop((x, y, x + w, y + h)).save(dst)
print(f"crop_image: {src} [{x},{y} {w}x{h}] -> {dst}")
