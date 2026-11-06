#!/usr/bin/env python3
"""Pad an image at the TOP with transparent rows up to a multiple of 8, so its art stays
flush with whatever sits below it (img2tiles pads at the bottom).

CASTLE ending walk, layer 2 (53 px): padded at the bottom it left 3 px of the sky layer
showing between the trees and the ground; the PC draws them touching.

usage: tools/pad_top.py in.png out.png
"""
import sys
from PIL import Image

src, dst = sys.argv[1], sys.argv[2]
im = Image.open(src).convert("RGBA")
h = (im.height + 7) & ~7
out = Image.new("RGBA", (im.width, h), (0, 0, 0, 0))
out.paste(im, (0, h - im.height))
out.save(dst)
print(f"{dst}: {im.width}x{im.height} -> {im.width}x{h} (padded {h - im.height} px on top)")
