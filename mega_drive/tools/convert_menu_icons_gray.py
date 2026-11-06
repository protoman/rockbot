#!/usr/bin/env python3
"""Create a grey version of the small menu icons for unselected weapons."""
import os
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
RES = os.path.join(ROOT, "res")
SRC = os.path.join(RES, "menu_icons.png")
OUT = os.path.join(RES, "menu_icons_gray.png")


def main():
    src = Image.open(SRC).convert("RGB")
    out = Image.new("P", (src.width, 8))
    pal = [0] * 48
    pal[13 * 3:13 * 3 + 3] = [109, 109, 109]   # PAL0 grey
    out.putpalette(pal)
    for y in range(8):
        for x in range(src.width):
            rgb = src.getpixel((x, y))
            out.putpixel((x, y), 0 if rgb == (0, 0, 0) else 13)
    out.save(OUT)
    print("wrote", OUT)


if __name__ == "__main__":
    main()
