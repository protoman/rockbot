#!/usr/bin/env python3
"""E/W/S tank icons for the weapon menu grid (PC icons_small.png cells 11..13).

Outputs res/menu_tanks.png (3 frames of 8x8, coloured, indexed on the menu icon
palette so they share PAL3 with the weapon icons) and res/menu_tanks_gray.png
(the same shapes on PAL0 index 13, shown while the cursor is elsewhere).
"""
import os

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
RES = os.path.join(ROOT, "res")
SRC = os.path.join(ROOT, "data/games/Rockbot1/images/icons_small.png")
FIRST = 11
N = 3


def main():
    icons = Image.open(os.path.join(RES, "menu_icons.png"))
    pal = icons.getpalette()[:48]
    cols = [tuple(pal[i * 3:i * 3 + 3]) for i in range(16)]
    src = Image.open(SRC).convert("RGBA")
    col = Image.new("P", (N * 8, 8))
    col.putpalette(pal)
    gray = Image.new("P", (N * 8, 8))
    gpal = [0] * 48
    gpal[13 * 3:13 * 3 + 3] = [109, 109, 109]
    gray.putpalette(gpal)
    for n in range(N):
        for y in range(8):
            for x in range(8):
                r, g, b, a = src.getpixel(((FIRST + n) * 8 + x, y))
                if a < 128 or (r, g, b) == (0, 0, 0):
                    col.putpixel((n * 8 + x, y), 0)
                    gray.putpixel((n * 8 + x, y), 0)
                    continue
                best = min(range(1, 15), key=lambda i: sum((p - q) ** 2 for p, q in zip(cols[i], (r, g, b))))
                col.putpixel((n * 8 + x, y), best)
                gray.putpixel((n * 8 + x, y), 13)
    col.save(os.path.join(RES, "menu_tanks.png"))
    gray.save(os.path.join(RES, "menu_tanks_gray.png"))
    print("wrote res/menu_tanks.png, res/menu_tanks_gray.png")


if __name__ == "__main__":
    main()
