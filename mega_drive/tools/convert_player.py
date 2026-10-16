#!/usr/bin/env python3
"""Convert the RockBot player sheet (p1.png) into an SGDK SPRITE sheet.

The PC sheet is 928x29 = 32 frames of 29x29, laid out by raw frame index.
The green (55,255,0) and magenta (255,0,255) pixels are palette-swap
placeholders: classplayer.cpp:1004 replaces them at runtime with the
weapon_colors entry. Player 0 (RockBot) uses
  color1 = (143, 0, 119)  replaces COLORKEY1 (55, 255, 0)
  color2 = (0, 115, 239)  replaces COLORKEY2 (255, 0, 255)
See file/v4/file_game_v4.h:335 file_player_v3_1().

SGDK expects one animation per row, cells of equal size, frames in order.
Frames are therefore re-ordered and padded to 32x32 (MD hardware sprite limit).

Writes res/player.png. See res/resources.res for the matching SPRITE line.

Usage: python3 tools/convert_player.py
"""
import os
import sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
RES = os.path.join(ROOT, "res")

SRC = os.path.join(ROCKBOT, "build/games/RockDroid1/images/sprites/p1.png")
FRAME = 29          # source frame size
CELL = 32           # output cell size (multiple of 8, <= 32)
COLORKEY = (75, 125, 125)

# weapon_colors[0] for player 0 (RockBot), applied over the placeholders
SWAPS = {
    (55, 255, 0): (143, 0, 119),
    (255, 0, 255): (0, 115, 239),
}

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
]

MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)
MAX_COLORS = 15


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def main():
    sheet = Image.open(SRC).convert("RGB")
    sw, sh = sheet.size
    n = sw // FRAME
    print(f"source {SRC}: {sw}x{sh} -> {n} frames of {FRAME}x{FRAME}")

    src = sheet.load()

    def frame_rgba(f):
        out = []
        for y in range(FRAME):
            for x in range(FRAME):
                out.append(src[f * FRAME + x, y])
        return out

    # palette: colorkey reserved at index 0, swapped colors collected here
    rgbs = []
    for _, frames, _ in ANIMS:
        for f in frames:
            for c in frame_rgba(f):
                c = SWAPS.get(c, c)
                if c != COLORKEY:
                    rgbs.append((md_round(c[0]), md_round(c[1]), md_round(c[2])))
    uniq = list(dict.fromkeys(rgbs))
    if len(uniq) > MAX_COLORS:
        print(f"  WARNING: {len(uniq)} colors, truncating to {MAX_COLORS}")
        uniq = uniq[:MAX_COLORS]
    palette = [(0, 0, 0)] + uniq
    while len(palette) < 16:
        palette.append((0, 0, 0))
    lut = {c: i + 1 for i, c in enumerate(uniq)}
    print(f"  palette: {len(uniq)} colors")

    cols = max(len(a[1]) for a in ANIMS)
    rows = len(ANIMS)
    out = Image.new("P", (cols * CELL, rows * CELL))
    out.putpalette([v for c in palette for v in c])
    data = [0] * (cols * CELL * rows * CELL)

    def put_cell(row, col, rgba):
        ox, oy = col * CELL, row * CELL
        # art origin stays at (0,0); pad right/bottom only
        for y in range(FRAME):
            for x in range(FRAME):
                c = rgba[y * FRAME + x]
                c = SWAPS.get(c, c)
                idx = 0 if c == COLORKEY else lut[(md_round(c[0]), md_round(c[1]), md_round(c[2]))]
                data[(oy + y) * cols * CELL + ox + x] = idx

    for r, (name, frames, _) in enumerate(ANIMS):
        for c, f in enumerate(frames):
            put_cell(r, c, frame_rgba(f))
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
