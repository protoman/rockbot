#!/usr/bin/env python3
"""Convert the hand-owned player sheet (images/sprites/p1.png) into an SGDK
SPRITE sheet.

The hand art is indexed (PAL2 = res/md_pal2.gpl) and keeps two
palette-swap placeholders on dedicated indices:
  index 14 = green  (55, 255, 0)   classplayer.cpp COLORKEY1
  index 15 = magenta (255, 0, 255) classplayer.cpp COLORKEY2
classplayer.cpp:1004 replaces them at runtime with the weapon_colors entry.
Player 0 (RockBot): color1 = (143, 0, 119), color2 = (0, 115, 239)
(file_game_v4.h:335). The converter swaps them BY INDEX, so the input's exact
RGB values no longer matter (quantizing RGB first destroyed that identity).

Frames are re-ordered by animation row and padded to 32x32 (MD sprite limit).

Writes res/player.png. See res/resources.res for the matching SPRITE line.

Usage: python3 tools/convert_player.py
"""
import os
import sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
IMAGES = os.path.join(ROOT, "images")
RES = os.path.join(ROOT, "res")

SRC = os.path.join(IMAGES, "sprites/p1.png")
FRAME = 29          # source frame size
CELL = 32           # output cell size (multiple of 8, <= 32)

# palette-swap placeholders kept by images/sprites/p1..p4 (PAL2 indices)
PLACEHOLDER_WEAPON1 = 14
PLACEHOLDER_WEAPON2 = 15

# weapon_colors[0] for player 0 (RockBot), the placeholders' runtime colours
COLOR1 = (143, 0, 119)
COLOR2 = (0, 115, 239)

# animation row -> (source frame indices, per-frame time in 1/60s)
# order and delays from classplayer.cpp:545 initFrames()
# STAIRS uses frame 17 (addSpriteFrame ANIM_TYPE_STAIRS, 17)
# SLIDE uses frame 20 (addSpriteFrame ANIM_TYPE_SLIDE, 20)
ANIMS = [
    ("STAND",    [3, 4],         [120, 6]),
    ("WALK",     [7, 8, 7, 6],   [9, 9, 9, 9]),
    ("JUMP",     [9],            [9]),
    ("ATTACK",   [11],           [9]),
    ("HIT",      [15, 16],       [6, 6]),
    ("TELEPORT", [0, 1, 2],      [12, 6, 6]),
    ("STAIRS",   [17],           [0]),
    ("SLIDE",    [20],           [0]),
    # classplayer.cpp:606 addSpriteFrame(ANIM_TYPE_STAIRS_ATTACK, 19)
    ("STAIRS_ATTACK", [19],      [9]),
]

MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def nearest_idx(rgb, palette, first=1, last=13):
    c = (md_round(rgb[0]), md_round(rgb[1]), md_round(rgb[2]))
    best, bd = first, 1 << 30
    for i in range(first, last + 1):
        p = palette[i]
        d = (p[0] - c[0]) ** 2 + (p[1] - c[1]) ** 2 + (p[2] - c[2]) ** 2
        if d < bd:
            bd, best = d, i
    return best


def main():
    sheet = Image.open(SRC)
    if sheet.mode != "P":
        print(f"{SRC}: not indexed -- re-run tools/import_pc_art.py first")
        return 1
    sw, sh = sheet.size
    n = sw // FRAME
    print(f"source {SRC}: {sw}x{sh} -> {n} frames of {FRAME}x{FRAME}")

    palette = [tuple(sheet.getpalette()[i * 3:i * 3 + 3]) for i in range(16)]
    swap = {
        PLACEHOLDER_WEAPON1: nearest_idx(COLOR1, palette),
        PLACEHOLDER_WEAPON2: nearest_idx(COLOR2, palette),
    }
    assert swap[PLACEHOLDER_WEAPON1] != swap[PLACEHOLDER_WEAPON2], \
        "weapon colours collapsed to one palette index"
    print(f"  PAL2 indices {PLACEHOLDER_WEAPON1}/{PLACEHOLDER_WEAPON2} -> "
          f"{swap[PLACEHOLDER_WEAPON1]}/{swap[PLACEHOLDER_WEAPON2]} "
          f"(weapon colors)")

    src = sheet.load()

    def frame_idx(f):
        return [src[f * FRAME + x, y] for y in range(FRAME) for x in range(FRAME)]

    cols = max(len(a[1]) for a in ANIMS)
    rows = len(ANIMS)
    out = Image.new("P", (cols * CELL, rows * CELL))
    out.putpalette([v for c in palette for v in c] + [0] * (768 - 48))
    data = [0] * (cols * CELL * rows * CELL)

    def put_cell(row, col, idxs):
        ox, oy = col * CELL, row * CELL
        # art origin stays at (0,0); pad right/bottom only
        for y in range(FRAME):
            for x in range(FRAME):
                i = idxs[y * FRAME + x]
                data[(oy + y) * cols * CELL + ox + x] = swap.get(i, i)

    for r, (name, frames, _) in enumerate(ANIMS):
        for c, f in enumerate(frames):
            put_cell(r, c, frame_idx(f))
        print(f"  row {r} {name}: frames {frames}")

    out.putdata(data)
    os.makedirs(RES, exist_ok=True)
    out.save(os.path.join(RES, "player.png"))
    print(f"wrote res/player.png {out.size} ({cols}x{rows} cells of {CELL}px)")

    # print the SPRITE time array to paste into resources.res
    times = "[" + "".join("[" + ",".join(str(t) for t in a[2]) + "]" for a in ANIMS) + "]"
    print(f"SPRITE time array: {times}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
