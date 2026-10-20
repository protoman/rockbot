#!/usr/bin/env python3
"""Convert the 11 small menu weapon icons to the shared PAL0 UI palette so
they can be used under the classic HUD bars without stealing PAL3 (the stage
background/colourcycle line)."""
import os
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
RES = os.path.join(ROOT, "res")
from pal_ui import PAL_UI_RGB

SRC = os.path.join(RES, "menu_icons.png")
OUT = os.path.join(RES, "hud_icon.png")
FRAMES = 11


def nearest(rgb):
    # index 0 is transparent; use the closest opaque UI colour.
    return min(range(1, 16), key=lambda i: sum((rgb[k] - PAL_UI_RGB[i][k]) ** 2
                                                for k in range(3)))


def main():
    src = Image.open(SRC).convert("RGB")
    out = Image.new("P", (src.width, 8))
    pal = []
    for rgb in PAL_UI_RGB:
        pal += list(rgb)
    out.putpalette(pal + [0] * (768 - len(pal)))
    for y in range(8):
        for x in range(src.width):
            rgb = src.getpixel((x, y))
            out.putpixel((x, y), 0 if rgb == (0, 0, 0) else nearest(rgb))
    out.save(OUT)
    print("wrote", OUT)


if __name__ == "__main__":
    main()
