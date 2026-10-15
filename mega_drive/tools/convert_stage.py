#!/usr/bin/env python3
"""Convert the INTRO stage (stage 0 / CHAOS CITY) from the PC data files
into SGDK resources.

Reads:
  build/games/RockDroid1/stages_v301.dat      -> stage 0 record + map links
  build/games/RockDroid1/maps_v2.dat          -> all 3 maps of stage 0
  build/games/RockDroid1/images/tilesets/<ts> -> tileset PNG

Writes:
  res/stage0_map<N>.png     full map rendered with the stage tileset
  res/stage0_bg<N>.png      parallax backgrounds baked to 512px (plane width)
  src/stage0_data.h/.c      terrain grids, links, background info

Stage 0 progression (from stages_v301.dat links, positions are Uint8;
y values after the top-row crop, see crop_top_row):
  link2: map 0 (136,13) -> map 1 (198,0)  climb down the ladder at x=136
  link3: map 1 (1,0)    -> map 0 (139,13) climb up the ladder at x=1

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
MAP_W, MAP_H = 200, 14   # output size: 320x224 NTSC (top PC row cropped)
MAP_H_SRC = 15           # PC maps are 15 rows (240px)
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
    # st_map_background: char filename[30], s16 adjust_y, s8 speed, u8 auto_scroll,
    # u8 gfx, pad -> 36 bytes (C++ alignment); two of them, bg0 at offset 0.
    rec = raw[0:36]
    bg0_name = rec[0:30].split(b"\0")[0].decode("latin1")
    bg0_adjust_y, = struct.unpack("<h", rec[30:32])
    bg0_speed = struct.unpack("<b", rec[32:33])[0]
    t = raw[MAP_TILES_OFF:MAP_TILES_OFF + MAP_W * MAP_H_SRC * TILE_STRUCT]
    cells = []
    for x in range(MAP_W):
        col = []
        for y in range(MAP_H_SRC):
            v = t[(x * MAP_H_SRC + y) * TILE_STRUCT:(x * MAP_H_SRC + y) * TILE_STRUCT + TILE_STRUCT]
            locked = v[0]
            t1x, t1y, t3x, t3y = struct.unpack("bbbb", v[1:5])
            col.append((locked, t1x, t1y, t3x, t3y))
        cells.append(col)
    return bg, (bg0_name, bg0_adjust_y, bg0_speed), cells


def build_palette(rgbs):
    """One master palette shared by every map of the stage.

    Colors beyond MAX_COLORS are merged into their nearest neighbour
    (squared RGB distance) instead of being truncated, so no art color
    silently collapses to palette index 1."""
    uniq = []
    for c in rgbs:
        if c not in uniq:
            uniq.append(c)
    while len(uniq) > MAX_COLORS:
        bi, bj, bd = 0, 1, 1 << 30
        for i in range(len(uniq)):
            for j in range(i + 1, len(uniq)):
                a, b = uniq[i], uniq[j]
                d = (a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2 + (a[2] - b[2]) ** 2
                if d < bd:
                    bd, bi, bj = d, i, j
        del uniq[bj]
    pal = [(0, 0, 0)] + uniq
    while len(pal) < 16:
        pal.append((0, 0, 0))
    lut = {}
    for c in set(rgbs):
        best, bd = 1, 1 << 30
        for i, p in enumerate(uniq):
            d = (c[0] - p[0]) ** 2 + (c[1] - p[1]) ** 2 + (c[2] - p[2]) ** 2
            if d < bd:
                bd, best = d, i + 1
        lut[c] = best
    return pal, lut


def collect_map(map_n):
    """Return (cells, used-tile coords) for one map."""
    _, _, cells = read_map(STAGE_INDEX, map_n)
    used, seen = [], set()
    for x in range(MAP_W):
        for y in range(MAP_H_SRC):
            _, t1x, t1y, t3x, t3y = cells[x][y]
            for (cx, cy) in ((t1x, t1y), (t3x, t3y)):
                if cx < 0 or cy < 0:
                    continue
                if (cx, cy) not in seen:
                    seen.add((cx, cy))
                    used.append((cx, cy))
    return cells, used


def crop_top_row(cells, links, map_n):
    """Drop the top row so the map fits 14 rows (224px, no vertical scroll).
    If any link touches this map at row 0, copy the top row onto row 1
    first so its tiles/links stay reachable. Link rows: 0 stays 0,
    everything else shifts up by one."""
    touches = any((frm == map_n and oy == 0) or (to == map_n and dy == 0)
                  for (_, frm, to, ox, oy, dx, dy) in links)
    if touches:
        for x in range(MAP_W):
            cells[x][1] = cells[x][0]
    return [col[1:] for col in cells]


def collect_colors(used, ts_px):
    rgbs = []
    for (cx, cy) in used:
        for yy in range(TILE):
            for xx in range(TILE):
                r, g, b, a = ts_px[cx * TILE + xx, cy * TILE + yy]
                if a == 0 or (r, g, b) == COLORKEY:
                    continue
                rgbs.append((md_round(r), md_round(g), md_round(b)))
    return rgbs


def convert_map(map_n, ts_px, cells, used, palette, lut):
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
    print(f"  map {map_n}: {len(used)} tiles, "
          f"{ {TERRAIN_NAMES.get(k, k): v for k, v in sorted(hist.items())} }")
    return img, terrain


BG_PLANE_W = 512  # BG_B plane is 64 tiles wide; art is baked to this width


def convert_background(map_n, name, adjust_y, speed):
    """Bake one parallax background to 512px wide (art + first 192 cols
    appended) so the plane wraps naturally at its own width — no runtime
    seam handling. Own palette (PAL3 at runtime)."""
    src = Image.open(os.path.join(DATA, "images/map_backgrounds", name)).convert("RGBA")
    h = (src.height + 7) & ~7   # SGDK wants height as a multiple of 8
    bg = Image.new("RGBA", (BG_PLANE_W, h), (0, 0, 0, 255))
    bg.paste(src, (0, 0))
    bg.paste(src.crop((0, 0, BG_PLANE_W - src.width, src.height)), (src.width, 0))
    px = bg.load()
    rgbs = [(md_round(px[x, y][0]), md_round(px[x, y][1]), md_round(px[x, y][2]))
            for y in range(bg.height) for x in range(bg.width)]
    pal, lut = build_palette(rgbs)
    img = Image.new("P", (BG_PLANE_W, bg.height))
    img.putpalette([v for c in pal for v in c])
    img.putdata([lut.get(c, 1) for c in rgbs])
    out = os.path.join(RES, f"stage0_bg{map_n}.png")
    img.save(out)
    print(f"  bg{map_n}: {name} {src.size} -> {img.size} "
          f"(adjust_y={adjust_y} speed={speed}) -> res/stage0_bg{map_n}.png")
    return {"present": 1, "adjust_y": adjust_y, "speed": speed}


def main():
    stage = read_stage(STAGE_INDEX)
    links = read_links(STAGE_INDEX)
    print(f"stage {STAGE_INDEX}: '{stage['name']}' tileset={stage['tileset']} boss={stage['boss']}")
    print(f"links: {links}")

    ts = Image.open(os.path.join(DATA, "images/tilesets", stage["tileset"])).convert("RGBA")
    ts_px = ts.load()

    os.makedirs(RES, exist_ok=True)
    os.makedirs(SRC, exist_ok=True)

    # One master palette for the whole stage: collect every map's colors
    # first, then render all maps against the same palette/lut. Per-map
    # palettes differ in color order, which flashed on map transitions.
    maps_data, all_rgbs, scroll_locks = [], [], []
    # PC scroll-lock columns: every one of the 15 data rows is
    # SOLID/DOOR/SPIKE/ICE/SCROLL_LOCK (classmap.cpp:184-193). The camera
    # edge may never sit on such a column (changeScrolling), so the player
    # never sees past a map divider.
    lock_terrain = {1, 3, 4, 6, 10}
    for m in range(MAPS_PER_STAGE):
        cells, used = collect_map(m)
        rgbs = collect_colors(used, ts_px)
        all_rgbs += rgbs
        maps_data.append((cells, used))
        scroll_locks.append([
            x for x in range(MAP_W)
            if all(cells[x][y][0] in lock_terrain for y in range(MAP_H_SRC))
        ])
    maps_data = [(crop_top_row(cells, links, m), used)
                 for m, (cells, used) in enumerate(maps_data)]
    palette, lut = build_palette(all_rgbs)
    print(f"master palette: {len(set(all_rgbs))} unique colors -> {MAX_COLORS} slots max")

    terrains, used_maps = [], []
    for m, (cells, used) in enumerate(maps_data):
        img, terrain = convert_map(m, ts_px, cells, used, palette, lut)
        img.save(os.path.join(RES, f"stage0_map{m}.png"))
        print(f"    -> res/stage0_map{m}.png {img.size}")
        terrains.append(terrain)
        used_maps.append(m)

    bg_infos = []
    for m in range(MAPS_PER_STAGE):
        _, bg0, _ = read_map(STAGE_INDEX, m)
        name, adj, spd = bg0
        if name:
            bg_infos.append(convert_background(m, name, adj, spd))
        else:
            bg_infos.append({"present": 0, "adjust_y": 0, "speed": 0})

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
            "typedef struct { u8 present; s16 adjust_y; s8 speed; } StageBgInfo;\n\n"
            f"extern const u8 stage0_terrain[STAGE0_MAP_COUNT][STAGE0_MAP_W * STAGE0_MAP_H];\n"
            f"extern const StageLink stage0_links[{len(links)}];\n"
            f"extern const StageBgInfo stage0_bg_info[STAGE0_MAP_COUNT];\n"
            f"#define STAGE0_LINK_COUNT {len(links)}\n\n"
            "// Full-wall columns: every one of the 15 PC data rows is\n"
            "// SOLID/DOOR/SPIKE/ICE/SCROLL_LOCK (PC classmap.cpp:184-193).\n"
            "// The camera edge may never sit on one (PC changeScrolling\n"
            "// classmap.cpp:459/475): the view never shows what is behind a\n"
            "// map divider.\n"
            f"#define STAGE0_SCROLL_LOCK_MAX {max(len(l) for l in scroll_locks)}\n"
            "extern const u8 stage0_scroll_lock_n[STAGE0_MAP_COUNT];\n"
            "extern const u8 stage0_scroll_lock[STAGE0_MAP_COUNT][STAGE0_SCROLL_LOCK_MAX];\n\n"
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
        oy = 0 if oy == 0 else oy - 1   # top PC row cropped
        dy = 0 if dy == 0 else dy - 1
        lines.append(f"    {{ {frm}, {ox}, {oy}, {to}, {dx}, {dy} }}, // link{i}\n")
    lines.append("};\n\n")

    lines.append("const StageBgInfo stage0_bg_info[STAGE0_MAP_COUNT] = {\n")
    for m, info in enumerate(bg_infos):
        comment = "map %d" % m
        lines.append(f"    {{ {info['present']}, {info['adjust_y']}, {info['speed']} }}, // {comment}\n")
    lines.append("};\n\n")

    lines.append("const u8 stage0_scroll_lock_n[STAGE0_MAP_COUNT] = {\n")
    lines.append("    " + ", ".join(str(len(l)) for l in scroll_locks) + "\n};\n\n")

    lines.append("const u8 stage0_scroll_lock[STAGE0_MAP_COUNT][STAGE0_SCROLL_LOCK_MAX] = {\n")
    lock_max = max(len(l) for l in scroll_locks)
    for m, locks in enumerate(scroll_locks):
        padded = locks + [0] * (lock_max - len(locks))
        lines.append("    { " + ", ".join(str(v) for v in padded) + f" }}, // map {m}\n")
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
