#!/usr/bin/env python3
"""Compose the classic-style STAGE SELECT screen for the Mega Drive.

PC reference: stage_select::pick_stage_classic_style (stage_select.cpp:357).
Layout (PC 320x240, top 8 + bottom 8 cropped to the MD 320x224 screen):
  * bg  classic_style_stage_select_bg.png (full screen, own palette)
  * 8 boss faces in a 3x3 grid, centre cell = the player face (PC faces_x_left
    64, faces_y_top 41, spacing 80x64; rounded to 8px tile alignment, the
    PC uses y=41 which is not tile-aligned)
  * cursor classic_style_stage_select_cursor.png, 2 frames 48x48 (black
    brackets + yellow pulse), drawn as 4 quadrant sprites x2 frames

Everything static is baked into ONE BG_A tilemap with per-cell palettes:
  bg -> PAL3, boss faces -> PAL0 (font line, 1..14), player face -> PAL2.
The cursor is 4x2 hardware sprites (48x48 = 2x2 of 24x24, sprite max 32).

Emits:
  src/ssel_data.c/h     packed 4bpp tile strip + u16 tilemap (palette bits) +
                        cursor sprite PNGs are separate (rescomp) + the 3
                        palettes + stage/boss name strings
  res/ssel_cursor_*.png 4 SPRITE sheets, 2 frames of 24x24 each

Usage: python3 tools/convert_stage_select.py
"""
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
DATA = os.path.join(ROCKBOT, "build/games/RockDroid1")
IMAGES = os.path.join(ROOT, "images")
RES = os.path.join(ROOT, "res")
SRC = os.path.join(ROOT, "src")

STAGE_STRUCT = 438
MAP_W, MAP_H = 40, 28
CROP_TOP = 8            # PC 240 -> MD 224: drop 8 rows top and bottom
TILE = 8
BASE_TILE = 16          # TILE_USER_INDEX: where ssel tiles are uploaded

PAL_BG, PAL_FACE, PAL_PLAYER, PAL_CURSOR = 3, 0, 2, 1
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)

# (stage 1..8) -> (col, row) of the 3x3 grid, matching draw_faces_classic_style
FACE_GRID = {
    1: (0, 0), 2: (1, 0), 3: (2, 0),
    4: (0, 1),           5: (2, 1),
    6: (0, 2), 7: (1, 2), 8: (2, 2),
}
FACE_PC_X = 64          # faces_x_left
FACE_PC_Y = 40          # faces_y_top rounded to 8 (PC 41)
FACE_DX, FACE_DY = 80, 64
PLAYER_FACE = (1, 1)    # grid cell of the centre slot


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def vdp(c):
    c = (md_round(c[0]), md_round(c[1]), md_round(c[2]))
    # MD CRAM word: R bits 1-3, G bits 5-7, B bits 9-11 (white = 0x0EEE).
    return ((MD_LEVELS.index(c[0]) << 1) | (MD_LEVELS.index(c[1]) << 5)
            | (MD_LEVELS.index(c[2]) << 9))


def load_p(path):
    im = Image.open(os.path.join(IMAGES, path))
    assert im.mode == "P", f"{path}: not indexed (run tools/import_pc_art.py)"
    pal = [tuple(im.getpalette()[i * 3:i * 3 + 3]) for i in range(16)]
    return im, pal


def read_stage(n):
    with open(os.path.join(DATA, "stages_v301.dat"), "rb") as f:
        f.seek(STAGE_STRUCT * n)
        raw = f.read(STAGE_STRUCT)
    return (raw[0:30].split(b"\0")[0].decode("latin1"),
            raw[362:382].split(b"\0")[0].decode("latin1"))


def main():
    bg, pal_bg = load_p("backgrounds/classic_style_stage_select_bg.png")
    assert bg.size == (320, 240), bg.size
    # canvas of (palette, index) per pixel, PC coordinates minus CROP_TOP
    canvas = [[(PAL_BG, bg.getpixel((x, y + CROP_TOP)))
               for x in range(320)] for y in range(224)]

    def paste(img, dx, dy, pal):
        for y in range(img.height):
            for x in range(img.width):
                canvas[dy + y][dx + x] = (pal, img.getpixel((x, y)))

    for stage, (col, row) in FACE_GRID.items():
        face, _ = load_p(f"faces/boss_{stage}.png")
        paste(face, FACE_PC_X + col * FACE_DX,
              FACE_PC_Y + row * FACE_DY - CROP_TOP, PAL_FACE)

    # player face with its eyes baked in (the PC nudges them a couple of
    # pixels with the cursor; we keep them centred -- see the plan notes)
    # Player face + eyes are SPRITES, one pair per player, chosen at runtime
    # by the player-select choice (the boss faces stay baked in the plane).
    for pn in (1, 2, 3, 4):
        face, _ = load_p(f"faces/p{pn}.png")
        face.save(os.path.join(RES, f"ssel_face_p{pn}.png"))
        eyes, pal_eyes = load_p(f"faces/p{pn}_eyes.png")
        eout = Image.new("P", (16, 8))          # 12x6 art in an 8-multiple cell
        eout.putpalette([v for c in pal_eyes for v in c] + [0] * (768 - 48))
        for y in range(eyes.height):
            for x in range(eyes.width):
                eout.putpixel((x, y), eyes.getpixel((x, y)))
        eout.save(os.path.join(RES, f"ssel_eyes_p{pn}.png"))
    print("wrote res/ssel_face_p1..4.png + ssel_eyes_p1..4.png")

    # ---- tiles: dedup by (palette, index bytes) --------------------------
    tiles, index = [], {}

    def tile_of(cx, cy):
        cell = [canvas[cy * TILE + y][cx * TILE + x]
                for y in range(TILE) for x in range(TILE)]
        pals = {p for p, _ in cell}
        assert len(pals) == 1, f"cell ({cx},{cy}) mixes palettes {pals}"
        pal = pals.pop()
        idx = bytes(i for _, i in cell)
        key = (pal, idx)
        n = index.get(key)
        if n is None:
            n = index[key] = len(tiles)
            tiles.append((pal, idx))
        return n | (pal << 13)

    tilemap = [[tile_of(cx, cy) for cx in range(MAP_W)] for cy in range(MAP_H)]
    print(f"stage select: {len(tiles)} unique tiles, tilemap {MAP_W}x{MAP_H}")

    # ---- packed 4bpp (tile-major: all 8 rows of tile 0, then tile 1, ...) --
    n = len(tiles)
    packed = []
    for _pal, idx in tiles:
        for y in range(TILE):
            for xb in range(TILE // 2):
                packed.append((idx[y * TILE + xb * 2] << 4)
                              | idx[y * TILE + xb * 2 + 1])
    words = [(packed[i] << 24) | (packed[i + 1] << 16) |
             (packed[i + 2] << 8) | packed[i + 3]
             for i in range(0, len(packed), 4)]

    # ---- cursor sprites: 4 quadrants x 2 frames, each a single frame -------
    # (one frame per SPRITE so the engine never auto-animates; the black and
    # yellow layers are toggled from code)
    cur, pal_cursor = load_p("backgrounds/classic_style_stage_select_cursor.png")
    assert cur.size == (96, 48), cur.size
    quads = {"tl": (0, 0), "tr": (24, 0), "bl": (0, 24), "br": (24, 24)}
    for name, (qx, qy) in quads.items():
        for f, suffix in ((0, "b"), (1, "y")):
            out = Image.new("P", (24, 24))
            out.putpalette([v for c in pal_cursor for v in c] + [0] * (768 - 48))
            for y in range(24):
                for x in range(24):
                    out.putpixel((x, y), cur.getpixel((f * 48 + qx + x, qy + y)))
            out.save(os.path.join(RES, f"ssel_cursor_{name}_{suffix}.png"))
    print("wrote res/ssel_cursor_{tl,tr,bl,br}_{b,y}.png (single frames)")

    # ---- names -----------------------------------------------------------
    names = [read_stage(i) for i in range(9)]
    for i in (1, 2, 8):
        print(f"  stage {i}: '{names[i][0]}' boss '{names[i][1]}'")

    # ---- emit C ----------------------------------------------------------
    h = """#ifndef SSEL_DATA_H
#define SSEL_DATA_H

#include <genesis.h>

// Generated by tools/convert_stage_select.py. Do not edit by hand.
#define SSEL_TILE_BASE %d
#define SSEL_TILE_COUNT %d
#define SSEL_MAP_W %d
#define SSEL_MAP_H %d

// Stage/boss names for stages 1..8 (index 0 unused).
extern const char *const ssel_stage_names[9];
extern const char *const ssel_boss_names[9];

// Static screen: packed 4bpp tiles + a u16 tilemap (palette bits included,
// local tile index -- add SSEL_TILE_BASE). PAL: 0 faces+font, 2 player,
// 3 background.
extern const u32 ssel_tiles_data[SSEL_TILE_COUNT * 8];
extern const u16 ssel_map[SSEL_MAP_W * SSEL_MAP_H];
extern const u16 ssel_pal_bg[16];
extern const u16 ssel_pal_cursor[16];

#endif
""" % (BASE_TILE, n, MAP_W, MAP_H)

    c = ['#include "ssel_data.h"\n\n']
    c.append(f"const u32 ssel_tiles_data[{n * 8}] = {{\n")
    for i in range(0, len(words), 8):
        c.append("    " + ", ".join(f"0x{w:08X}" for w in words[i:i + 8]) + ",\n")
    c.append("};\n\n")
    c.append(f"const u16 ssel_map[{MAP_W * MAP_H}] = {{\n")
    for row in tilemap:
        c.append("    " + ", ".join(f"0x{v:04X}" for v in row) + ",\n")
    c.append("};\n\n")

    def pal_lines(name, pal):
        c.append(f"const u16 {name}[16] = {{\n    "
                 + ", ".join(f"0x{vdp(x):03X}" for x in pal) + "\n};\n\n")

    pal_lines("ssel_pal_bg", pal_bg)
    pal_lines("ssel_pal_cursor", pal_cursor)

    def cstr(s):
        return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'

    c.append("const char *const ssel_stage_names[9] = {\n")
    for i in range(9):
        c.append(f"    {cstr(names[i][0])},  /* {i} */\n" if i else
                 f"    {cstr(names[i][0])},\n")
    c.append("};\n\n")
    c.append("const char *const ssel_boss_names[9] = {\n")
    for i in range(9):
        c.append(f"    {cstr(names[i][1])},  /* {i} */\n" if i else
                 f"    {cstr(names[i][1])},\n")
    c.append("};\n")

    with open(os.path.join(SRC, "ssel_data.h"), "w") as f:
        f.write(h)
    with open(os.path.join(SRC, "ssel_data.c"), "w") as f:
        f.writelines(c)
    print(f"wrote src/ssel_data.h/.c ({n} tiles, map {MAP_W}x{MAP_H})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
