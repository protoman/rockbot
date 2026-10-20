#!/usr/bin/env python3
"""Convert the PC 8x8 menu weapon icons (icons_small.png) for the MD grid.

The PC strips 18 icons; the port needs 0..10 (buster, eight bosses, COIL,
JET). One shared 15-colour palette is used because the MD sprite layer has
only four palette lines and the panel already uses PAL1/PAL2.

Outputs res/menu_icons.png (11 x 8x8).

Hand-owned override: if `images/menu_icons.png` exists, that indexed image is
copied to res/ unchanged. Edit that file to tune colours/pixels; the build
will keep the hand version instead of regenerating from the PC art.
"""
import os
import shutil

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
SRC = os.path.join(ROCKBOT, "build/games/RockDroid1/images/icons_small.png")
HAND = os.path.join(ROOT, "images/menu_icons.png")
RES = os.path.join(ROOT, "res")

ICONS = 11
CELL = 8
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)


def snap(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def main():
    if os.path.exists(HAND):
        im = Image.open(HAND)
        if im.width % 8 or im.height != 8:
            raise SystemExit(f"{HAND}: expected N*8 x 8, got {im.width}x{im.height}")
        shutil.copyfile(HAND, os.path.join(RES, "menu_icons.png"))
        print(f"kept hand-owned images/menu_icons.png "
              f"({im.width // 8} icons, {im.width}x{im.height})")
        print('PALETTE menu_icons_pal "menu_icons.png"')
        print('SPRITE menu_icon_sprite "menu_icons.png" 8p 8p FAST '
              '[[0,0,0,0,0,0,0,0,0,0,0]]')
        return

    src = Image.open(SRC).convert("RGB")
    strip = Image.new("RGB", (ICONS * CELL, CELL))
    for i in range(ICONS):
        strip.paste(src.crop((i * CELL, 0, (i + 1) * CELL, CELL)),
                    (i * CELL, 0))
    q = strip.quantize(colors=15, method=Image.Quantize.MEDIANCUT)
    pal = q.getpalette()
    cols = [(snap(pal[i * 3]), snap(pal[i * 3 + 1]), snap(pal[i * 3 + 2]))
            for i in range(15)]
    # Index 0 is transparent in a hardware sprite. The PC art uses black as
    # its key, so put black at 0 (quantize usually already did).
    black = min(range(15), key=lambda i: sum(cols[i]))
    if black != 0:
        cols[0], cols[black] = cols[black], cols[0]
        px = q.load()
        for y in range(CELL):
            for x in range(strip.width):
                v = px[x, y]
                if v == 0:
                    px[x, y] = black
                elif v == black:
                    px[x, y] = 0
    q.putpalette([v for c in cols for v in c] + [0] * (768 - 45))
    q.save(os.path.join(RES, "menu_icons.png"))
    print("wrote res/menu_icons.png (11 icons, 8x8)")
    print("hand-owned override: copy it to images/menu_icons.png to edit")
    print('PALETTE menu_icons_pal "menu_icons.png"')
    print('SPRITE menu_icon_sprite "menu_icons.png" 8p 8p FAST '
          '[[0,0,0,0,0,0,0,0,0,0,0]]')


if __name__ == "__main__":
    main()
