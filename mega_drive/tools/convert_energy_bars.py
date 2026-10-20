#!/usr/bin/env python3
"""Build the classic vertical HUD bars from art/energy_bar_vertical.png.

The source art is 8x51: black gaps plus purple/blue columns.  It is emitted as
three indexed 8x56 strips (7 x 8px frames) so the sprite engine can stack the
segments and reveal them from the bottom up.

Palette indices are chosen from free entries in the player palette (PAL2):
  8/9   HP bar, weapon 0 colour1/colour2
  12    bar black
 13/14  weapon bar, selected weapon colour1/colour2
The boss strip uses PAL0's fixed black/white entries.
"""
import os
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ART = os.path.join(ROOT, "art", "energy_bar_vertical.png")
RES = os.path.join(ROOT, "res")

# source RGB -> palette index
BAR_VARIANTS = {
    "hud_bar_hp":  {(139, 0, 115): 8,  (0, 113, 238): 9,  (0, 0, 0): 12},
    "hud_bar_wpn": {(139, 0, 115): 13, (0, 113, 238): 14, (0, 0, 0): 12},
    "hud_bar_boss": {(139, 0, 115): 2, (0, 113, 238): 3,  (0, 0, 0): 1},
}


def main():
    src = Image.open(ART).convert("RGB")
    if src.size != (8, 51):
        raise SystemExit("energy_bar_vertical: expected 8x51, got %s" % (src.size,))
    for name, mapping in BAR_VARIANTS.items():
        out = Image.new("P", (8, 56))
        # Give the used indices real colours so rescomp keeps the indexed
        # image mapping; the runtime sprite still selects PAL2/PAL0.
        pal = [0] * 48
        pal[1 * 3:1 * 3 + 3] = [0, 0, 0]
        pal[2 * 3:2 * 3 + 3] = [255, 255, 255]
        pal[3 * 3:3 * 3 + 3] = [146, 219, 255]
        pal[8 * 3:8 * 3 + 3] = [139, 0, 115]
        pal[9 * 3:9 * 3 + 3] = [0, 113, 238]
        pal[12 * 3:12 * 3 + 3] = [0, 0, 0]
        pal[13 * 3:13 * 3 + 3] = [139, 0, 115]
        pal[14 * 3:14 * 3 + 3] = [0, 113, 238]
        out.putpalette(pal)
        for y in range(56):
            for x in range(8):
                rgb = src.getpixel((x, y)) if y < 51 else (0, 0, 0)
                out.putpixel((x, y), mapping.get(rgb, 0))
        path = os.path.join(RES, name + ".png")
        out.save(path)
        print("wrote", path)


if __name__ == "__main__":
    main()
