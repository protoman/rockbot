#!/usr/bin/env python3
"""Quantise the PC title art to 60 colours over FOUR palette lines (BUG-001).

The Mega Drive has 4 CRAM lines of 16 colours, but every converted screen so far
uses one line (15 colours + transparency). A tilemap entry carries a 2-bit
palette field, so each 8x8 tile may pick any line - 60 colours in total, as long
as each tile's own colours fit inside a single line.

The title is drawn from C-side tile + map arrays (not a rescomp IMAGE), so the
palette field is ours to write: the emitted map already holds `(line << 13)`.

Emits for `<name>`:
  src/<name>_tiles.h/.c   u32 tile data, pixel values are the SLOT in a line
  src/<name>_map.h/.c     u16 per 8x8 cell: (line << 13) | tile index
  src/<name>_pal.h/.c     4 x 16 CRAM words; line 0 slot 15 is white so
                          VDP_drawText keeps working over the screen

Slot 0 of every line is blank (alpha 0 and the game colorkey map to it).

Usage: python3 tools/title64.py <image.png> [--name title64] [--crop-h 224]
"""
import argparse
import os
import sys
from PIL import Image

TILE = 8
COLORKEY = (75, 125, 125)
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)
LINE_SLOTS = 15                 # slot 0 is blank -> 15 usable per line
WHITE = (255, 255, 255)


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def to_cram(c):
    r, g, b = (MD_LEVELS.index(v) for v in c)
    return (r << 1) | (g << 5) | (b << 9)


def dist(a, b):
    return sum((a[i] - b[i]) ** 2 for i in range(3))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("image")
    ap.add_argument("--name", default="title64")
    ap.add_argument("--crop-h", type=int, default=224)
    ap.add_argument("--codedir", default=None)
    ap.add_argument("--preview", default=None, help="write the emitted data back as a PNG")
    args = ap.parse_args()

    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(here)
    code = args.codedir or os.path.join(root, "src")

    im = Image.open(args.image).convert("RGBA")
    w = (im.width + TILE - 1) // TILE * TILE
    h = (min(args.crop_h, im.height) + TILE - 1) // TILE * TILE
    art = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    art.paste(im.crop((0, 0, min(im.width, w), min(im.height, h))), (0, 0))
    px = art.load()

    def cell_colour(x, y):
        r, g, b, a = px[x, y]
        if a == 0 or (r, g, b) == COLORKEY:
            return None
        return (md_round(r), md_round(g), md_round(b))

    # ---- the art as 8x8 tiles ----------------------------------------------
    tiles = []
    for ty in range(h // TILE):
        for tx in range(w // TILE):
            tiles.append([cell_colour(tx * TILE + x, ty * TILE + y)
                          for y in range(TILE) for x in range(TILE)])
    art_colours = {c for t in tiles for c in t if c}

    # ---- 60-colour master palette ------------------------------------------
    # Median cut over the opaque pixels, then snapped to the MD 3:3:3 ramp. The
    # ramp snap can merge entries, so fall back to the art's most frequent
    # colours if that leaves the palette short.
    budget = LINE_SLOTS * 4 - 1               # line 0 keeps slot 15 for the font
    pal_art = []
    if len(art_colours) > budget:
        opaque = [c for t in tiles for c in t if c]
        strip = Image.new("RGB", (len(opaque), 1))
        strip.putdata(opaque)
        q = strip.quantize(colors=budget, method=Image.Quantize.MEDIANCUT)
        qp = q.getpalette()
        for idx in sorted({v for v in q.getdata()}):
            c = (md_round(qp[idx * 3]), md_round(qp[idx * 3 + 1]),
                 md_round(qp[idx * 3 + 2]))
            if c not in pal_art:
                pal_art.append(c)
    counts = {}
    for t in tiles:
        for c in t:
            if c:
                counts[c] = counts.get(c, 0) + 1
    for c in sorted(counts, key=lambda c: -counts[c]):
        if len(pal_art) >= budget:
            break
        if c not in pal_art:
            pal_art.append(c)
    pal_art = pal_art[:budget]

    def nearest(c, pool):
        return min(pool, key=lambda p: dist(c, p))

    # ---- assign each tile to a palette line --------------------------------
    # Fit first (a line that can hold all of the tile's colours), then prefer the
    # line that already has most of them. Only when no line can fit does the tile
    # fold its extra shades into the closest slots.
    lines = [[WHITE], [], [], []]
    room = lambda L: (LINE_SLOTS - 1 if L == 0 else LINE_SLOTS) - len(lines[L])
    tile_line, folded = [], 0

    for cells in tiles:
        cols = {c for c in cells if c}
        # a tile may itself hold more shades than one line: keep the most used
        if len(cols) > LINE_SLOTS:
            keep = sorted(cols, key=lambda c: -counts[c])[:LINE_SLOTS]
            cols = set(keep)
            folded += 1
        fitting = []
        for L in range(4):
            missing = [c for c in cols if c not in lines[L]]
            if len(missing) <= room(L):
                have = len(cols) - len(missing)
                fitting.append((have, room(L), L, missing))
        if fitting:
            _, _, L, missing = max(fitting)
            for c in sorted(missing):
                lines[L].append(c)
        else:
            L = max(range(4), key=lambda x: len(cols) - len([c for c in cols
                                                             if c not in lines[x]]))
            folded += 1
        tile_line.append(L)

    # ---- slot lookup per line ---------------------------------------------
    def slot_of(c, L):
        pal = lines[L]
        if not pal:
            pal.append(nearest(c, pal_art))
        if c in pal:
            return pal.index(c)
        return pal.index(nearest(c, pal))

    # ---- rebuild tiles (slot values), dedup, build the map ------------------
    blank = bytes([0] * (TILE * TILE))
    seen, tile_data, map_entries = {blank: 0}, [[0] * 8], []
    for i, cells in enumerate(tiles):
        L = tile_line[i]
        idx = [0 if c is None else slot_of(c, L) for c in cells]
        key = bytes(idx)
        if key not in seen:
            seen[key] = len(tile_data)
            words = []
            for y in range(TILE):
                # one longword per row: 8 pixels, high nibble leftmost
                hi = lo = 0
                for p in range(4):
                    hi = (hi << 4) | idx[y * TILE + p]
                    lo = (lo << 4) | idx[y * TILE + 4 + p]
                words.append((hi << 16) | lo)
            tile_data.append(words)
        map_entries.append((L << 13) | seen[key])

    per_line = [0, 0, 0, 0]
    for L in tile_line:
        per_line[L] += 1
    print(f"{os.path.basename(args.image)}: {w}x{h}, {len(tile_data)} tiles, "
          f"{len(pal_art)} colours kept of the art's {len(art_colours)} "
          f"(slots per line {[len(l) for l in lines]}), cells per line {per_line}, "
          f"folded tiles {folded}")

    # ---- emit -------------------------------------------------------------
    up = args.name.upper()
    with open(os.path.join(code, f"{args.name}_tiles.h"), "w") as f:
        f.write(f"#ifndef {up}_TILES_H\n#define {up}_TILES_H\n\n#include <genesis.h>\n\n"
                f"extern const u32 {args.name}_tiles_data[{len(tile_data)} * 8];\n"
                f"#define {up}_TILE_COUNT {len(tile_data)}\n\n#endif\n")
    with open(os.path.join(code, f"{args.name}_tiles.c"), "w") as f:
        f.write(f'#include "{args.name}_tiles.h"\n\n'
                f"const u32 {args.name}_tiles_data[{len(tile_data)} * 8] = {{\n")
        for words in tile_data:
            f.write("    " + ", ".join("0x%08X" % v for v in words) + ",\n")
        f.write("};\n")
    with open(os.path.join(code, f"{args.name}_map.h"), "w") as f:
        f.write(f"#ifndef {up}_MAP_H\n#define {up}_MAP_H\n\n#include <genesis.h>\n\n"
                f"#define {up}_W {w // TILE}\n#define {up}_H {h // TILE}\n"
                f"extern const u16 {args.name}_map[{len(map_entries)}];\n\n#endif\n")
    with open(os.path.join(code, f"{args.name}_map.c"), "w") as f:
        f.write(f'#include "{args.name}_map.h"\n\n'
                f"const u16 {args.name}_map[{len(map_entries)}] = {{\n")
        for i in range(0, len(map_entries), 16):
            f.write("    " + ", ".join("0x%04X" % v
                                       for v in map_entries[i:i + 16]) + ",\n")
        f.write("};\n")
    with open(os.path.join(code, f"{args.name}_pal.h"), "w") as f:
        f.write(f"#ifndef {up}_PAL_H\n#define {up}_PAL_H\n\n#include <genesis.h>\n\n"
                f"extern const u16 {args.name}_pal[4][16];\n\n#endif\n")
    with open(os.path.join(code, f"{args.name}_pal.c"), "w") as f:
        f.write(f'#include "{args.name}_pal.h"\n\n'
                f"const u16 {args.name}_pal[4][16] = {{\n")
        for L in range(4):
            row = [0] + [to_cram(c) for c in lines[L]]
            row += [0] * (16 - len(row))
            f.write("    { " + ", ".join("0x%04X" % v for v in row) + " },\n")
        f.write("};\n")
    print(f"  wrote src/{args.name}_{{tiles,map,pal}}.{{h,c}}")

    if args.preview:
        # Rebuild the screen from exactly what was emitted (tiles + map + the
        # four palettes). If this looks right, the data is right and any problem
        # on hardware is in the draw path.
        def slots_of(words):
            out = []
            for y in range(TILE):
                for x in range(TILE):
                    out.append((words[y] >> (28 - (x & 7) * 4)) & 0xF)
            return out
        pals = [[c for c in lines[L]] + [(0, 0, 0)] * (16 - len(lines[L]))
                for L in range(4)]
        out = Image.new("RGB", (w, h), (0, 0, 0))
        op = out.load()
        for ty in range(h // TILE):
            for tx in range(w // TILE):
                e = map_entries[ty * (w // TILE) + tx]
                L, t = (e >> 13) & 3, e & 0x7FF
                slots = slots_of(tile_data[t])
                for y in range(TILE):
                    for x in range(TILE):
                        sv = slots[y * TILE + x]
                        op[tx * TILE + x, ty * TILE + y] = pals[L][sv]
        out.save(args.preview)
        print(f"  preview written to {args.preview}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
