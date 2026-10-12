#!/usr/bin/env python3
"""Convert the INTRO stage (stage 0 / CHAOS CITY) from the PC data files
into SGDK resources.

Reads:
  build/games/RockDroid1/stages_v301.dat      -> stage 0 record + map links
  build/games/RockDroid1/maps_v2.dat          -> all 3 maps of stage 0
  build/games/RockDroid1/images/tilesets/<ts> -> tileset PNG

Writes:
  res/stage0_map<N>.png     full map rendered with the stage tileset
  src/stage0_data.h/.c      terrain grids, links, map metadata

Stage 0 progression (from stages_v301.dat links, positions are Uint8):
  link2: map 0 (136,14) -> map 1 (198,0)   climb down the ladder at x=136
  link3: map 1 (1,0)    -> map 0 (139,14)  climb up the ladder at x=1

Usage: python3 tools/convert_stage.py
"""
import os
import struct
import sys
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
DATA = os.path.join(ROCKBOT, "build/games/RockDroid1")
RES = os.path.join(ROOT, "res")
SRC = os.path.join(ROOT, "src")

STAGE_STRUCT = 438
MAP_STRUCT = 15078
MAP_TILES_OFF = 78
TILE_STRUCT = 5
MAP_W, MAP_H = 200, 15
TILE = 16
STAGE_INDEX = 0
MAPS_PER_STAGE = 3

COLORKEY = (75, 125, 125)
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)
MAX_COLORS = 15  # index 0 stays reserved for transparency

TERRAIN_NAMES = {
    0: "UNBLOCKED", 1: "SOLID", 2: "STAIR", 3: "DOOR", 4: "SPIKE",
    5: "WATER", 6: "ICE", 7: "MOVE_LEFT", 8: "MOVE_RIGHT", 9: "QUICKSAND",
    10: "SCROLL_LOCK", 11: "CHECKPOINT", 12: "EASYMODEBLOCK", 13: "HARDMODEBLOCK",
}


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def read_stage(n):
    with open(os.path.join(DATA, "stages_v301.dat"), "rb") as f:
        f.seek(STAGE_STRUCT * n)
        raw = f.read(STAGE_STRUCT)
    return {
        "name": raw[0:30].split(b"\0")[0].decode("latin1"),
        "bgm": raw[30:60].split(b"\0")[0].decode("latin1"),
        "tileset": raw[382:412].split(b"\0")[0].decode("latin1"),
        "boss": raw[362:382].split(b"\0")[0].decode("latin1"),
    }


def read_links(n):
    """file_link = 10 bytes at offset 60, 30 of them.
    { Sint8 from_map, Sint8 to_map, st_position_uint8 origin,
      st_position_uint8 destiny, Uint8 size, bool bidir, bool door, Sint8 type }
    Positions are unsigned; the PC data stores some as negative bytes."""
    with open(os.path.join(DATA, "stages_v301.dat"), "rb") as f:
        f.seek(STAGE_STRUCT * n + 60)
        raw = f.read(30 * 10)
    links = []
    for i in range(30):
        b = raw[i * 10:(i + 1) * 10]
        frm, to, ox, oy, dx, dy, size, bidir, door, typ = struct.unpack("bbbbbbbbbb", b)
        if frm < 0 or to < 0:
            continue  # unused slot
        links.append((i, frm & 0xFF, to & 0xFF, ox & 0xFF, oy & 0xFF, dx & 0xFF, dy & 0xFF))
    return links


def read_map(stage_n, map_n):
    with open(os.path.join(DATA, "maps_v2.dat"), "rb") as f:
        f.seek(MAP_STRUCT * (stage_n * MAPS_PER_STAGE + map_n))
        raw = f.read(MAP_STRUCT)
    bg = struct.unpack("<hhh", raw[72:78])
    t = raw[MAP_TILES_OFF:MAP_TILES_OFF + MAP_W * MAP_H * TILE_STRUCT]
    cells = []
    for x in range(MAP_W):
        col = []
        for y in range(MAP_H):
            v = t[(x * MAP_H + y) * TILE_STRUCT:(x * MAP_H + y) * TILE_STRUCT + TILE_STRUCT]
            locked = v[0]
            t1x, t1y, t3x, t3y = struct.unpack("bbbb", v[1:5])
            col.append((locked, t1x, t1y, t3x, t3y))
        cells.append(col)
    return bg, cells


def build_palette(rgbs):
    uniq = []
    for c in rgbs:
        if c not in uniq:
            uniq.append(c)
    if len(uniq) > MAX_COLORS:
        uniq = uniq[:MAX_COLORS]
    pal = [(0, 0, 0)] + uniq
    while len(pal) < 16:
        pal.append((0, 0, 0))
    return pal, {c: i + 1 for i, c in enumerate(uniq)}


def convert_map(stage, map_n, ts_px):
    bg, cells = read_map(STAGE_INDEX, map_n)

    used, used_set = [], set()
    for x in range(MAP_W):
        for y in range(MAP_H):
            _, t1x, t1y, t3x, t3y = cells[x][y]
            for (cx, cy) in ((t1x, t1y), (t3x, t3y)):
                if cx < 0 or cy < 0:
                    continue
                if (cx, cy) not in used_set:
                    used_set.add((cx, cy))
                    used.append((cx, cy))

    rgbs = []
    for (cx, cy) in used:
        for yy in range(TILE):
            for xx in range(TILE):
                r, g, b, a = ts_px[cx * TILE + xx, cy * TILE + yy]
                if a == 0 or (r, g, b) == COLORKEY:
                    continue
                rgbs.append((md_round(r), md_round(g), md_round(b)))

    palette, lut = build_palette(rgbs)

    def tile_indexed(cx, cy):
        out = []
        for yy in range(TILE):
            for xx in range(TILE):
                r, g, b, a = ts_px[cx * TILE + xx, cy * TILE + yy]
                if a == 0 or (r, g, b) == COLORKEY:
                    out.append(0)
                else:
                    c = (md_round(r), md_round(g), md_round(b))
                    out.append(lut.get(c, 1))
        return out

    tiles = [tile_indexed(cx, cy) for (cx, cy) in used]
    idx_of = {coord: i for i, coord in enumerate(used)}

    img = Image.new("P", (MAP_W * TILE, MAP_H * TILE))
    img.putpalette([v for c in palette for v in c])
    data = [0] * (MAP_W * MAP_H * TILE * TILE)
    for x in range(MAP_W):
        for y in range(MAP_H):
            _, t1x, t1y, t3x, t3y = cells[x][y]
            src = None
            if t1x >= 0 and t1y >= 0:
                src = tiles[idx_of[(t1x, t1y)]]
            if t3x >= 0 and t3y >= 0:
                src = tiles[idx_of[(t3x, t3y)]]
            if src is None:
                continue
            for yy in range(TILE):
                base = (y * TILE + yy) * MAP_W * TILE + x * TILE
                data[base:base + TILE] = src[yy * TILE:(yy + 1) * TILE]
    img.putdata(data)

    terrain = [cells[x][y][0] for y in range(MAP_H) for x in range(MAP_W)]
    hist = {}
    for v in terrain:
        hist[v] = hist.get(v, 0) + 1
    print(f"  map {map_n}: bg={bg}, {len(used)} tiles, {len(set(rgbs))} colors, "
          f"{ {TERRAIN_NAMES.get(k, k): v for k, v in sorted(hist.items())} }")
    return img, terrain


def main():
    stage = read_stage(STAGE_INDEX)
    links = read_links(STAGE_INDEX)
    print(f"stage {STAGE_INDEX}: '{stage['name']}' tileset={stage['tileset']} boss={stage['boss']}")
    print(f"links: {links}")

    ts = Image.open(os.path.join(DATA, "images/tilesets", stage["tileset"])).convert("RGBA")
    ts_px = ts.load()

    os.makedirs(RES, exist_ok=True)
    os.makedirs(SRC, exist_ok=True)

    terrains, used_maps = [], []
    for m in range(MAPS_PER_STAGE):
        img, terrain = convert_map(stage, m, ts_px)
        img.save(os.path.join(RES, f"stage0_map{m}.png"))
        print(f"    -> res/stage0_map{m}.png {img.size}")
        terrains.append(terrain)
        used_maps.append(m)

    n = len(used_maps)
    with open(os.path.join(SRC, "stage0_data.h"), "w") as f:
        f.write(
            "#ifndef STAGE0_DATA_H\n"
            "#define STAGE0_DATA_H\n\n"
            "#include <genesis.h>\n\n"
            f"// Generated by tools/convert_stage.py from stage {STAGE_INDEX} "
            f"(\"{stage['name']}\").\n"
            "// Terrain values match the original TERRAIN_TYPES (defines.h:285).\n\n"
            f"#define STAGE0_MAP_COUNT {n}\n"
            f"#define STAGE0_MAP_W {MAP_W}\n"
            f"#define STAGE0_MAP_H {MAP_H}\n"
            "#define STAGE0_TILE 16\n"
            "#define STAGE0_PIXEL_W (STAGE0_MAP_W * STAGE0_TILE)\n"
            "#define STAGE0_PIXEL_H (STAGE0_MAP_H * STAGE0_TILE)\n\n"
            "#define TERRAIN_UNBLOCKED 0\n"
            "#define TERRAIN_SOLID     1\n"
            "#define TERRAIN_STAIR     2\n"
            "#define TERRAIN_EASYBLOCK 12\n"
            "#define TERRAIN_HARDBLOCK 13\n\n"
            "// Terrain that blocks movement (matches character.cpp:1940 on normal\n"
            "// difficulty: unblocked/water/scroll-lock/checkpoint/stair and the\n"
            "// easy/hard mode blocks all pass through).\n"
            "#define TERRAIN_BLOCKS(t) ((t) != TERRAIN_UNBLOCKED && \\\n"
            "                           (t) != TERRAIN_STAIR && \\\n"
            "                           (t) != TERRAIN_EASYBLOCK && \\\n"
            "                           (t) != TERRAIN_HARDBLOCK)\n\n"
            "typedef struct { u8 map_from, tile_x, tile_y; u8 map_to, dst_x, dst_y; } StageLink;\n\n"
            f"extern const u8 stage0_terrain[STAGE0_MAP_COUNT][STAGE0_MAP_W * STAGE0_MAP_H];\n"
            f"extern const StageLink stage0_links[{len(links)}];\n"
            f"#define STAGE0_LINK_COUNT {len(links)}\n\n"
            "u16 stage_terrain_at(u8 map, s16 tileX, s16 tileY);\n\n"
            "#endif\n"
        )

    lines = ['#include "stage0_data.h"\n\n']
    lines.append(f"const u8 stage0_terrain[STAGE0_MAP_COUNT][STAGE0_MAP_W * STAGE0_MAP_H] = {{\n")
    for idx, terr in enumerate(terrains):
        lines.append("    { // map %d\n" % used_maps[idx])
        for y in range(MAP_H):
            row = ",".join(str(terr[y * MAP_W + x]) for x in range(MAP_W))
            lines.append("        " + row + ",\n")
        lines.append("    },\n")
    lines.append("};\n\n")

    lines.append(f"const StageLink stage0_links[STAGE0_LINK_COUNT] = {{\n")
    for (i, frm, to, ox, oy, dx, dy) in links:
        lines.append(f"    {{ {frm}, {ox}, {oy}, {to}, {dx}, {dy} }}, // link{i}\n")
    lines.append("};\n\n")

    lines.append(
        "u16 stage_terrain_at(u8 map, s16 tileX, s16 tileY)\n"
        "{\n"
        "    if (map >= STAGE0_MAP_COUNT) return TERRAIN_SOLID;\n"
        "    if (tileX < 0 || tileX >= STAGE0_MAP_W) return TERRAIN_SOLID;\n"
        "    if (tileY < 0 || tileY >= STAGE0_MAP_H) return TERRAIN_UNBLOCKED;\n"
        "    return stage0_terrain[map][tileY * STAGE0_MAP_W + tileX];\n"
        "}\n"
    )

    with open(os.path.join(SRC, "stage0_data.c"), "w") as f:
        f.write("".join(lines))
    print(f"wrote src/stage0_data.c/h ({n} maps, {len(links)} links)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
