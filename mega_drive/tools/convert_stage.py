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
IMAGES = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "images")  # hand-owned art
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

# Animated tiles: file_anim_block is char filename[30] + int frame_delay[6]
# with C++ alignment (56 bytes), stored as a raw array in anim_block_list.dat.
ANIM_BLOCK_SIZE = 56
ANIM_MAX_FRAMES = 6
# Fixed VRAM region for animated frames. Free zone: bg tiles end at 911
# (TILE_BG_INDEX 144 + 768), sprite engine starts at TILE_SPRITE_INDEX =
# TILE_FONT_INDEX - spriteVramSize = 1952 - 512 = 1440. One slot per
# animation: 6 frames x 4 tiles (a 16x16 frame = 2x2 tiles).
TILE_ANIM_BASE = 912
TILE_ANIM_STRIDE = 24

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


def read_anim_blocks():
    """anim_block_list.dat: raw array of file_anim_block (no count prefix)."""
    with open(os.path.join(DATA, "anim_block_list.dat"), "rb") as f:
        raw = f.read()
    blocks = []
    for off in range(0, len(raw) - ANIM_BLOCK_SIZE + 1, ANIM_BLOCK_SIZE):
        rec = raw[off:off + ANIM_BLOCK_SIZE]
        name = rec[:30].split(b"\0")[0].decode("latin1")
        delays = struct.unpack("<6i", rec[32:56])
        blocks.append((name, delays))
    return blocks


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
    bg0_auto = rec[33]      # E_BG_SCROLL_MODE: 0 = NONE (static, PC)
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
    return bg, (bg0_name, bg0_adjust_y, bg0_speed, bg0_auto), cells


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


def convert_map(map_n, ts_px, cells, used, palette, lut, frame0_px, slot_of):
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

    def anim_frame0(gid):
        # marker: x < -1 (tile1 also needs y == 0) -> anim id = -x - 2
        return frame0_px[slot_of[gid]] if gid in slot_of else None

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
            elif t1x < -1 and t1y == 0:
                src = anim_frame0((-t1x) - 2)
            if t3x >= 0 and t3y >= 0:
                src = tiles[idx_of[(t3x, t3y)]]
            elif t3x < -1:
                src = anim_frame0((-t3x) - 2)
            if src is None:
                continue
            for yy in range(TILE):
                base = (y * TILE + yy) * MAP_W * TILE + x * TILE
                data[base:base + TILE] = src[yy * TILE:(yy + 1) * TILE]
    img.putdata(data)

    # Difficulty-mode blocks are NOT part of this port: the game has no
    # Easy/Hard difficulty, so BOTH variants are ignored as geometry
    # (standing rule, stage0_plan.md: TERRAIN EASYBLOCK/HARDBLOCK are
    # pass-through). PC only materialises 12 in Easy and 13 in Hard
    # (classmap.cpp:284-289). Emitting the raw values made the player land
    # on the phantom 13s under the map1 left ladder.
    terrain = []
    for y in range(MAP_H):
        for x in range(MAP_W):
            v = cells[x][y][0]
            if v == 12 or v == 13:
                v = 0
            terrain.append(v)
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

    # ---- animated tiles + tile3 (drawn above the player) ----
    # tile3: PC showAbove() redraws these over the player; the MD patch
    # callback (stage.c) sets the per-tile priority bit instead (zero cost).
    # anim markers: tile1.x < -1 && y == 0, or tile3.x < -1
    # (graphicslib place_3rd_level_tile) -> id = -x - 2 into anim_block_list.dat.
    anim_blocks = read_anim_blocks()
    tile3_masks, anim_masks, anim_cell_lists = [], [], []
    anim_ids = set()
    mask_bytes = (MAP_W * MAP_H + 7) // 8
    for cells, _ in maps_data:
        mask3 = bytearray(mask_bytes)
        maska = bytearray(mask_bytes)
        cell_list = []
        for x in range(MAP_W):
            for y in range(MAP_H):
                _, t1x, t1y, t3x, t3y = cells[x][y]
                n = y * MAP_W + x
                if t3x >= 0 and t3y >= 0:
                    mask3[n >> 3] |= 1 << (n & 7)
                gid, t3flag = None, 0
                if t1x < -1 and t1y == 0:
                    gid = (-t1x) - 2
                if t3x < -1:
                    gid = (-t3x) - 2
                    t3flag = 1
                    mask3[n >> 3] |= 1 << (n & 7)   # anim on tile3 = above player
                if gid is None or gid >= len(anim_blocks):
                    continue
                maska[n >> 3] |= 1 << (n & 7)
                anim_ids.add(gid)
                cell_list.append([x, y, gid, t3flag])
        tile3_masks.append(mask3)
        anim_masks.append(maska)
        anim_cell_lists.append(cell_list)

    # compact slot per used global anim id
    slots = sorted(anim_ids)
    slot_of = {gid: s for s, gid in enumerate(slots)}
    for cell_list in anim_cell_lists:
        for c in cell_list:
            c[2] = slot_of[c[2]]

    # strip art joins the master palette so anim frames share it
    strips = {}
    for gid in slots:
        fn = anim_blocks[gid][0]
        im = Image.open(os.path.join(DATA, "images/tilesets/anim", fn)).convert("RGBA")
        strips[gid] = im
        nf = max(1, im.width // TILE)
        all_rgbs += collect_colors([(f, 0) for f in range(nf)], im.load())

    palette, lut = build_palette(all_rgbs)
    print(f"master palette: {len(set(all_rgbs))} unique colors -> {MAX_COLORS} slots max")

    def frame_tiles(im, frame):
        """16x16 strip frame -> 4 8x8 tiles (TL,TR,BL,BR), 16 4bpp words each."""
        px = im.load()
        out = []
        for qx, qy in ((0, 0), (8, 0), (0, 8), (8, 8)):
            words = []
            for yy in range(8):
                for k in (0, 4):
                    w = 0
                    for p in range(4):
                        r, g, b, a = px[frame * TILE + qx + k + p, qy + yy]
                        if a == 0 or (r, g, b) == COLORKEY:
                            idx = 0
                        else:
                            idx = lut.get((md_round(r), md_round(g), md_round(b)), 1)
                        w = (w << 4) | idx
                    words.append(w)
            out.append(words)
        return out

    def tiles_pixels(t4):
        """4 8x8 tiles (TL,TR,BL,BR) -> flat 16x16 indexed pixels."""
        px = [0] * (TILE * TILE)
        for q, words in enumerate(t4):
            qx, qy = (q & 1) * 8, (q >> 1) * 8
            for yy in range(8):
                for xi in range(8):
                    w = words[yy * 2 + (xi >> 2)]
                    shift = 12 - (xi & 3) * 4
                    px[(qy + yy) * TILE + qx + xi] = (w >> shift) & 0xF
        return px

    anim_frames, frame0_px = [], {}
    for s, gid in enumerate(slots):
        im = strips[gid]
        nf = max(1, im.width // TILE)
        f0 = frame_tiles(im, 0)
        frames = []
        for f in range(ANIM_MAX_FRAMES):
            t = frame_tiles(im, f) if f < nf else f0
            frames.append([w for tile in t for w in tile])
        anim_frames.append(frames)
        frame0_px[s] = tiles_pixels(f0)

    terrains, used_maps = [], []
    for m, (cells, used) in enumerate(maps_data):
        img, terrain = convert_map(m, ts_px, cells, used, palette, lut,
                                   frame0_px, slot_of)
        img.save(os.path.join(RES, f"stage0_map{m}.png"))
        print(f"    -> res/stage0_map{m}.png {img.size}")
        terrains.append(terrain)
        used_maps.append(m)
    for m, cl in enumerate(anim_cell_lists):
        print(f"  map {m}: tile3={sum(bin(b).count('1') for b in tile3_masks[m])} "
              f"anim_cells={len(cl)}")

    bg_infos = []
    for m in range(MAPS_PER_STAGE):
        _, bg0, _ = read_map(STAGE_INDEX, m)
        name, adj, spd, auto = bg0
        if name:
            info = convert_background(m, name, adj, spd)
            info["auto_scroll"] = auto
            bg_infos.append(info)
        else:
            bg_infos.append({"present": 0, "adjust_y": 0, "speed": 0,
                             "auto_scroll": 0})

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
            "typedef struct { u8 present; s16 adjust_y; s8 speed; u8 auto_scroll; } StageBgInfo;\n\n"
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
            "// ---- animated tiles + tile3 (above-player layer) ----\n"
            "// tile3 cells get TILE_ATTR_PRIORITY via the SGDK map patch\n"
            "// callback (stage.c) so the VDP draws them over sprites.\n"
            "// Anim cells point at a fixed VRAM slot (one per animation,\n"
            "// 6 frames x 4 tiles); frame changes DMA 32 bytes per slot.\n"
            f"#define TILE_ANIM_BASE {TILE_ANIM_BASE}\n"
            f"#define TILE_ANIM_STRIDE {TILE_ANIM_STRIDE}\n"
            f"#define STAGE0_ANIM_SLOT_COUNT {max(1, len(slots))}\n"
            f"#define STAGE0_ANIM_CELL_MAX {max(1, max((len(c) for c in anim_cell_lists), default=1))}\n\n"
            "typedef struct { u8 x, y, slot, tile3; } StageAnimCell;\n"
            "typedef struct { u8 frames; u16 delay[6]; } StageAnimSlot; // delay in frames\n\n"
            "extern const u8 stage0_tile3_mask[STAGE0_MAP_COUNT][(STAGE0_MAP_W * STAGE0_MAP_H + 7) / 8];\n"
            "extern const u8 stage0_anim_mask[STAGE0_MAP_COUNT][(STAGE0_MAP_W * STAGE0_MAP_H + 7) / 8];\n"
            "extern const StageAnimCell stage0_anim_cells[STAGE0_MAP_COUNT][STAGE0_ANIM_CELL_MAX];\n"
            "extern const u16 stage0_anim_cell_n[STAGE0_MAP_COUNT];\n"
            "extern const StageAnimSlot stage0_anim_slots[STAGE0_ANIM_SLOT_COUNT];\n"
            "extern const u16 stage0_anim_frames[STAGE0_ANIM_SLOT_COUNT][6][64];\n"
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
        lines.append(f"    {{ {info['present']}, {info['adjust_y']}, "
                     f"{info['speed']}, {info['auto_scroll']} }}, // {comment}\n")
    lines.append("};\n\n")

    lines.append("const u8 stage0_scroll_lock_n[STAGE0_MAP_COUNT] = {\n")
    lines.append("    " + ", ".join(str(len(l)) for l in scroll_locks) + "\n};\n\n")

    lines.append("const u8 stage0_scroll_lock[STAGE0_MAP_COUNT][STAGE0_SCROLL_LOCK_MAX] = {\n")
    lock_max = max(len(l) for l in scroll_locks)
    for m, locks in enumerate(scroll_locks):
        padded = locks + [0] * (lock_max - len(locks))
        lines.append("    { " + ", ".join(str(v) for v in padded) + f" }}, // map {m}\n")
    lines.append("};\n\n")

    def emit_mask(name, masks):
        lines.append(f"const u8 {name}[STAGE0_MAP_COUNT]"
                     "[(STAGE0_MAP_W * STAGE0_MAP_H + 7) / 8] = {\n")
        for m, mask in enumerate(masks):
            lines.append("    { // map %d\n" % m)
            for i in range(0, len(mask), 16):
                chunk = ", ".join("0x%02X" % b for b in mask[i:i + 16])
                lines.append("        " + chunk + ",\n")
            lines.append("    },\n")
        lines.append("};\n\n")

    emit_mask("stage0_tile3_mask", tile3_masks)
    emit_mask("stage0_anim_mask", anim_masks)

    cell_max = max(1, max((len(c) for c in anim_cell_lists), default=1))
    lines.append("const StageAnimCell stage0_anim_cells[STAGE0_MAP_COUNT][STAGE0_ANIM_CELL_MAX] = {\n")
    for m, cl in enumerate(anim_cell_lists):
        rows = ["{ %d, %d, %d, %d }" % tuple(c) for c in cl]
        rows += ["{ 0, 0, 0, 0 }"] * (cell_max - len(rows))
        lines.append("    { " + ", ".join(rows) + " }, // map %d\n" % m)
    lines.append("};\n\n")

    lines.append("const u16 stage0_anim_cell_n[STAGE0_MAP_COUNT] = {\n")
    lines.append("    " + ", ".join(str(len(c)) for c in anim_cell_lists) + "\n};\n\n")

    lines.append("const StageAnimSlot stage0_anim_slots[STAGE0_ANIM_SLOT_COUNT] = {\n")
    if not slots:
        lines.append("    { 1, { 1, 1, 1, 1, 1, 1 } }, // dummy (no anims)\n")
    for s, gid in enumerate(slots):
        fn, delays = anim_blocks[gid]
        nf = max(1, strips[gid].width // TILE)
        dfs = []
        for f in range(ANIM_MAX_FRAMES):
            ms = delays[f] if f < len(delays) else 200
            dfs.append(min(65535, max(1, (ms * 60 + 500) // 1000)))
        lines.append("    { %d, { %s } }, // %d: %s\n"
                     % (nf, ", ".join(str(d) for d in dfs), s, fn))
    lines.append("};\n\n")

    lines.append("const u16 stage0_anim_frames[STAGE0_ANIM_SLOT_COUNT][6][64] = {\n")
    if not slots:
        lines.append("    { { 0 } }, // dummy\n")
    for s, frames in enumerate(anim_frames):
        lines.append("    { // slot %d (%s)\n" % (s, anim_blocks[slots[s]][0]))
        for words in frames:
            hexs = ", ".join("0x%04X" % w for w in words)
            lines.append("        { " + hexs + " },\n")
        lines.append("    },\n")
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
