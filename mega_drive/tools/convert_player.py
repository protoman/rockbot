#!/usr/bin/env python3
"""Convert the hand-owned player sheet (images/sprites/p1.png) into an SGDK
SPRITE sheet.

The hand art is indexed and keeps three palette-swap placeholders on
dedicated indices:
  index 13 = green   (55, 255, 0)   classplayer.cpp COLORKEY1
  index 14 = magenta (255, 0, 255)  classplayer.cpp COLORKEY2
  index 15 = cyan    (0, 255, 255)  classplayer.cpp COLORKEY3
classplayer.cpp:1004 replaces them at runtime with the weapon_colors entry.
Each player's real weapon_colors[0] comes from player_list_v3_1_1.dat (see
tools/convert_players.py) and is placed on 13/14/15 so pixel indices copy
1:1 (no RGB lookup). An unset colour keeps the art's placeholder.

Frames are re-ordered by animation row and padded to 32x32 (MD sprite limit).

Writes res/player_p1..p4.png. See res/resources.res for the SPRITE lines.

Usage: python3 tools/convert_player.py
"""
import os
import sys
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from convert_players import parse_players  # noqa: E402

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
IMAGES = os.path.join(ROOT, "images")
RES = os.path.join(ROOT, "res")

SRC_DIR = os.path.join(IMAGES, "sprites")

FRAME = 29          # source frame size
CELL = 32           # output cell size (multiple of 8, <= 32)

# palette-swap placeholders kept by images/sprites/p1..p4
PLACEHOLDER_WEAPON1 = 13
PLACEHOLDER_WEAPON2 = 14
PLACEHOLDER_WEAPON3 = 15



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
    cols = max(len(a[1]) for a in ANIMS)
    rows = len(ANIMS)

    players = [(i + 1, p["c1"], p["c2"], p["c3"])
               for i, p in enumerate(parse_players())]
    for n, c1, c2, c3 in players:
        src_path = os.path.join(SRC_DIR, "p%d.png" % n)
        sheet = Image.open(src_path)
        if sheet.mode != "P":
            print(f"{src_path}: not indexed -- re-run tools/import_pc_art.py")
            return 1
        sw, sh = sheet.size
        palette = [tuple(sheet.getpalette()[i * 3:i * 3 + 3]) for i in range(16)]
        # The art keeps the placeholders on 13/14/15; put THIS player's weapon
        # colours on those entries so pixel indices copy 1:1 (no RGB lookup).
        if c1:
            palette[PLACEHOLDER_WEAPON1] = tuple(md_round(v) for v in c1)
        if c2:
            palette[PLACEHOLDER_WEAPON2] = tuple(md_round(v) for v in c2)
        if c3:
            palette[PLACEHOLDER_WEAPON3] = tuple(md_round(v) for v in c3)

        src = sheet.load()

        def frame_idx(f):
            return [src[f * FRAME + x, y]
                    for y in range(FRAME) for x in range(FRAME)]

        out = Image.new("P", (cols * CELL, rows * CELL))
        out.putpalette([v for c in palette for v in c] + [0] * (768 - 48))
        data = [0] * (cols * CELL * rows * CELL)
        for r, (name, frames, _) in enumerate(ANIMS):
            for c, f in enumerate(frames):
                ox, oy = c * CELL, r * CELL
                idxs = frame_idx(f)
                for y in range(FRAME):
                    for x in range(FRAME):
                        data[(oy + y) * cols * CELL + ox + x] = idxs[y * FRAME + x]
        out.putdata(data)
        os.makedirs(RES, exist_ok=True)
        out.save(os.path.join(RES, "player_p%d.png" % n))
        print(f"player {n}: {sw}x{sh} -> res/player_p{n}.png {out.size} "
              f"c1={palette[13]} c2={palette[14]} c3={palette[15]}")

    times = "[" + "".join("[" + ",".join(str(t) for t in a[2]) + "]" for a in ANIMS) + "]"
    print(f"SPRITE time array: {times}")
    for n, _, _, _ in players:
        print(f'SPRITE player_p{n}_sprite "player_p{n}.png" 32p 32p FAST {times}')
    return 0


if __name__ == "__main__":
    sys.exit(main())
