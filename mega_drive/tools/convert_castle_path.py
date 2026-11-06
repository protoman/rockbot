#!/usr/bin/env python3
"""Castle-path data for the castle intro (PC stage_select/sceneslib classic_style_castle_intro).

Writes src/castle_path_data.{h,c}:
  - the 8 stage points (PC pixel coordinates, 240-line screen; the port art is the
    same image shifted up 8 rows, Makefile castle_b0 starts at y=8),
  - the 8x8 point marker and the 16x16 skull marker as nibble rows in PAL0 indices
    (nearest colour of castle64's palette line 0; the colourkey is index 0),
  - the path colours as PAL0 indices (PC grey 220 / dark 19),
and tests/host/fixtures/castle_path_ref.h: an INDEPENDENT Python transliteration of
PC draw::draw_castle_path (graphic/draw.cpp:1048) reduced to a per-segment step
count and a CRC32 over the ordered plot sequence, which tests/host/castle_path_test.c
compares with src/castle_path_draw.c. Also bounds the overlay tile count.

    python3 tools/convert_castle_path.py
"""
import os
import re
import zlib

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PCROOT = os.path.dirname(ROOT)
BG = os.path.join(PCROOT, "build/games/RockDroid1/images/backgrounds")
CROP_Y = 8

# classic_style_castle_intro: first point, then the end of each segment (sceneslib.cpp:871-901)
POINTS = [(11, 223), (74, 169), (92, 100), (132, 65), (149, 20), (212, 78),
          (245, 132), (160, 197)]
GREY, DARK = (220, 220, 220), (19, 19, 19)


def pal0():
    t = open(os.path.join(ROOT, "src/castle64_pal.c")).read()
    v = [int(x, 16) for x in re.findall(r"0x([0-9A-Fa-f]{4})", t)][:16]
    lv = [0, 36, 73, 109, 146, 182, 219, 255]
    return [(lv[(c & 0xF) >> 1], lv[((c >> 4) & 0xF) >> 1], lv[((c >> 8) & 0xF) >> 1]) for c in v]


def nearest(pal, c):
    return min(range(1, 16), key=lambda i: sum((a - b) ** 2 for a, b in zip(pal[i], c)))


def bitmap(path, pal):
    im = Image.open(path).convert("RGBA")
    rows = []
    for y in range(im.height):
        row = []
        for x in range(im.width):
            r, g, b, a = im.getpixel((x, y))
            row.append(0 if (a == 0 or (r, g, b) == (75, 125, 125)) else nearest(pal, (r, g, b)))
        rows.append(row)
    return rows


def words(rows):
    out = []
    for row in rows:
        for k in range(0, len(row), 8):
            w = 0
            for x in range(8):
                w = (w << 4) | row[k + x]
            out.append(w)
    return out


# ---- PC draw::draw_castle_path, transliterated (independent of the C) ---------
def pc_segment(ix, iy, fx, fy):
    """Yield (x, y, w, h, colour) in PC call order. colour: 'g' grey, 'd' dark."""
    dist_x, dist_y = ix - fx, iy - fy
    pos_y, pos_x = iy - 1, ix + 2
    if dist_y < 0:
        pos_y += 9
    if dist_y != 0:
        for _ in range(abs(dist_y) - 2):
            yield (pos_x - 1, pos_y, 1, 1, "d")
            yield (pos_x, pos_y, 4, 1, "g")
            yield (pos_x + 4, pos_y, 1, 1, "d")
            pos_y += -1 if dist_y > 0 else 1
    if dist_y > 0:
        pos_y += 1
    else:
        pos_y -= 4
    temp_x, max_x = pos_x, abs(dist_x) - 2
    if dist_x > 0:
        temp_x -= 1
        max_x -= 4
    elif dist_x < 0:
        temp_x += 4
        max_x -= 4
    if dist_x != 0:
        for _ in range(max_x):
            yield (temp_x, pos_y - 1, 1, 1, "d")
            yield (temp_x, pos_y, 1, 4, "g")
            yield (temp_x, pos_y + 4, 1, 1, "d")
            temp_x += -1 if dist_x > 0 else 1


def seg_iterations(ix, iy, fx, fy):
    """PC loop iterations (3 rect calls each) of a segment."""
    dist_x, dist_y = ix - fx, iy - fy
    v = max(abs(dist_y) - 2, 0) if dist_y else 0
    h = 0
    if dist_x:
        h = max(abs(dist_x) - 2 - 4, 0)
    return v, h


def main():
    pal = pal0()
    grey_i, dark_i = nearest(pal, GREY), nearest(pal, DARK)
    point = bitmap(os.path.join(BG, "classic_style_castle_point.png"), pal)
    skull = bitmap(os.path.join(BG, "classic_style_castle_skull_point.png"), pal)

    # reference sequence per segment: step count + CRC32 of ordered plotted pixels
    ref = []
    touched = set()
    for s in range(len(POINTS) - 1):
        ix, iy = POINTS[s]
        fx, fy = POINTS[s + 1]
        v, h = seg_iterations(ix, iy, fx, fy)
        crc = 0
        n = 0
        for (x, y, w, hh, c) in pc_segment(ix, iy, fx, fy):
            for yy in range(y, y + hh):
                for xx in range(x, x + w):
                    ycr = yy - CROP_Y
                    crc = zlib.crc32(bytes([xx & 0xFF, xx >> 8, ycr & 0xFF, (ycr >> 8) & 0xFF,
                                            1 if c == "g" else 2]), crc)
                    n += 1
                    if 0 <= xx < 320 and 0 <= ycr < 224:
                        touched.add((xx // 8, ycr // 8))
        ref.append((v, h, n, crc))
    # markers touch cells too
    for (px, py) in POINTS:
        for yy in range(8):
            for xx in range(8):
                touched.add(((px + xx) // 8, (py + yy - CROP_Y) // 8))
    for yy in range(16):
        for xx in range(16):
            touched.add(((POINTS[7][0] + xx) // 8, (POINTS[7][1] + yy - CROP_Y) // 8))
    print(f"overlay cells touched by the whole path + markers: {len(touched)}")
    assert len(touched) <= 400, "overlay slot budget exceeded"

    with open(os.path.join(ROOT, "src/castle_path_data.h"), "w") as f:
        f.write(f"""// Generated by tools/convert_castle_path.py. Do not edit.
#ifndef CASTLE_PATH_DATA_H
#define CASTLE_PATH_DATA_H

#include <genesis.h>

#define CASTLE_POINTS      {len(POINTS)}
#define CASTLE_PATH_CROP_Y {CROP_Y}   // port art = PC image shifted up 8 rows
#define CASTLE_PATH_GREY   {grey_i}   // PAL0 index nearest to the PC grey {GREY}
#define CASTLE_PATH_DARK   {dark_i}   // PAL0 index nearest to the PC border {DARK}
#define CASTLE_OVL_CELLS   {len(touched)}   // overlay cells the whole path + markers touch

typedef struct {{ s16 x, y; }} CastlePt;
extern const CastlePt castle_points[CASTLE_POINTS];
extern const u32 castle_point_bmp[8];       // 8x8, one nibble row each, 0 = transparent
extern const u32 castle_skull_bmp[16 * 2];  // 16x16, two words per row

#endif
""")
    with open(os.path.join(ROOT, "src/castle_path_data.c"), "w") as f:
        f.write('// Generated by tools/convert_castle_path.py. Do not edit.\n#include "castle_path_data.h"\n\n')
        f.write("const CastlePt castle_points[CASTLE_POINTS] = {\n")
        for (x, y) in POINTS:
            f.write(f"    {{ {x}, {y} }},\n")
        f.write("};\n\nconst u32 castle_point_bmp[8] = {\n    "
                + ", ".join(f"0x{w:08X}" for w in words(point)) + "\n};\n\n")
        f.write("const u32 castle_skull_bmp[16 * 2] = {\n    "
                + ", ".join(f"0x{w:08X}" for w in words(skull)) + "\n};\n")
    os.makedirs(os.path.join(ROOT, "tests/host/fixtures"), exist_ok=True)
    with open(os.path.join(ROOT, "tests/host/fixtures/castle_path_ref.h"), "w") as f:
        f.write("/* Generated by tools/convert_castle_path.py (PC draw_castle_path transliteration). */\n"
                "static const struct { int v, h; unsigned n; unsigned crc; } castle_ref[] = {\n")
        for (v, h, n, crc) in ref:
            f.write(f"    {{ {v}, {h}, {n}u, 0x{crc:08X}u }},\n")
        f.write("};\n")
    print("segments:", [(v, h, n) for v, h, n, _ in ref])
    print(f"colours: grey -> PAL0[{grey_i}] {pal[grey_i]}, dark -> PAL0[{dark_i}] {pal[dark_i]}")


if __name__ == "__main__":
    main()
