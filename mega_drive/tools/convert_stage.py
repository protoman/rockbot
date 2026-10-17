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
STAGE_INDEX = int(sys.argv[1]) if len(sys.argv) > 1 else 0
PFX = "stage%d" % STAGE_INDEX
UP = PFX.upper()
MAPS_PER_STAGE = 3

COLORKEY = (75, 125, 125)
KEY_TOL = 6   # art sometimes paints the backdrop with the colorkey a few units
              # off (palmtree anim = 72,126,124); punch those too


def is_key(c):
    return (abs(c[0] - COLORKEY[0]) <= KEY_TOL
            and abs(c[1] - COLORKEY[1]) <= KEY_TOL
            and abs(c[2] - COLORKEY[2]) <= KEY_TOL)
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
                if a == 0 or is_key((r, g, b)):
                    continue
                rgbs.append((md_round(r), md_round(g), md_round(b)))
    return rgbs


def convert_map(map_n, ts_px, cells, used, palette, lut, frame0_px, slot_of):
    def tile_indexed(cx, cy):
        out = []
        for yy in range(TILE):
            for xx in range(TILE):
                r, g, b, a = ts_px[cx * TILE + xx, cy * TILE + yy]
                if a == 0 or is_key((r, g, b)):
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
    out = os.path.join(RES, f"{PFX}_bg{map_n}.png")
    img.save(out)
    print(f"  bg{map_n}: {name} {src.size} -> {img.size} "
          f"(adjust_y={adjust_y} speed={speed}) -> res/{PFX}_bg{map_n}.png")
    return {"present": 1, "adjust_y": adjust_y, "speed": speed}


def main():
    stage = read_stage(STAGE_INDEX)
    links = read_links(STAGE_INDEX)
    print(f"stage {STAGE_INDEX}: '{stage['name']}' tileset={stage['tileset']} "
          f"boss={stage['boss']}")
    print(f"links: {links}")

    ts = Image.open(os.path.join(DATA, "images/tilesets", stage["tileset"])).convert("RGBA")
    ts_px = ts.load()
    os.makedirs(RES, exist_ok=True)
    os.makedirs(SRC, exist_ok=True)

    maps_data, all_rgbs, scroll_locks = [], [], []
    lock_terrain = {1, 3, 4, 6, 10}
    for m in range(MAPS_PER_STAGE):
        cells, used = collect_map(m)
        all_rgbs += collect_colors(used, ts_px)
        maps_data.append((cells, used))
        scroll_locks.append([
            x for x in range(MAP_W)
            if all(cells[x][y][0] in lock_terrain for y in range(MAP_H_SRC))
        ])
    maps_data = [(crop_top_row(cells, links, m), used)
                 for m, (cells, used) in enumerate(maps_data)]

    anim_blocks = read_anim_blocks()
    mask_bytes = (MAP_W * MAP_H + 7) // 8
    tile3_masks = []
    anim_cell_slot = []          # per map: u8 per cell, 0 = none, else slot+1
    anim_slot_gids = []          # per map: gid per compacted slot
    for cells, _ in maps_data:
        mask3 = bytearray(mask_bytes)
        cell_slot = bytearray(MAP_W * MAP_H)
        gid_to_slot, slot_gids = {}, []
        for x in range(MAP_W):
            for y in range(MAP_H):
                _, t1x, t1y, t3x, t3y = cells[x][y]
                n = y * MAP_W + x
                if t3x >= 0 and t3y >= 0:
                    mask3[n >> 3] |= 1 << (n & 7)
                gid = None
                if t1x < -1 and t1y == 0:
                    gid = (-t1x) - 2
                if t3x < -1:
                    gid = (-t3x) - 2
                    mask3[n >> 3] |= 1 << (n & 7)   # anim on tile3 = above player
                if gid is None or gid >= len(anim_blocks):
                    continue
                if gid not in gid_to_slot:
                    gid_to_slot[gid] = len(slot_gids)
                    slot_gids.append(gid)
                cell_slot[n] = gid_to_slot[gid] + 1
        tile3_masks.append(mask3)
        anim_cell_slot.append(cell_slot)
        anim_slot_gids.append(slot_gids)
    anim_slot_max = max(1, max((len(g) for g in anim_slot_gids), default=0))
    print(f"anim slots per map: {[len(g) for g in anim_slot_gids]} (max {anim_slot_max})")

    all_gids = sorted({g for gs in anim_slot_gids for g in gs})
    strips = {}
    for gid in all_gids:
        fn = anim_blocks[gid][0]
        im = Image.open(os.path.join(DATA, "images/tilesets/anim", fn)).convert("RGBA")
        strips[gid] = im
        nf = max(1, im.width // TILE)
        all_rgbs += collect_colors([(f, 0) for f in range(nf)], im.load())

    palette, lut = build_palette(all_rgbs)
    print(f"master palette: {len(set(all_rgbs))} unique colors -> {MAX_COLORS} slots max")

    def frame_tiles(im, frame):
        px = im.load()
        out = []
        for qx, qy in ((0, 0), (8, 0), (0, 8), (8, 8)):
            words = []
            for yy in range(8):
                for k in (0, 4):
                    w = 0
                    for p in range(4):
                        r, g, b, a = px[frame * TILE + qx + k + p, qy + yy]
                        if a == 0 or is_key((r, g, b)):
                            idx = 0
                        else:
                            idx = lut.get((md_round(r), md_round(g), md_round(b)), 1)
                        w = (w << 4) | idx
                    words.append(w)
            out.append(words)
        return out

    def tiles_pixels(t4):
        px = [0] * (TILE * TILE)
        for q, words in enumerate(t4):
            qx, qy = (q & 1) * 8, (q >> 1) * 8
            for yy in range(8):
                for xi in range(8):
                    w = words[yy * 2 + (xi >> 2)]
                    shift = 12 - (xi & 3) * 4
                    px[(qy + yy) * TILE + qx + xi] = (w >> shift) & 0xF
        return px

    # per-map: slot -> frame0 pixels, and slot -> (frames, delays, name)
    frame0_map, slots_meta = [], []
    for slot_gids in anim_slot_gids:
        f0map, meta = {}, []
        for s, gid in enumerate(slot_gids):
            im = strips[gid]
            nf = max(1, im.width // TILE)
            f0 = frame_tiles(im, 0)
            frames = []
            for f in range(ANIM_MAX_FRAMES):
                t = frame_tiles(im, f) if f < nf else f0
                frames.append([w for tile in t for w in tile])
            f0map[s] = tiles_pixels(f0)
            fn, delays = anim_blocks[gid]
            dfs = [min(65535, max(1, (ms * 60 + 500) // 1000))
                   for ms in (delays[f] if f < len(delays) else 200
                              for f in range(ANIM_MAX_FRAMES))]
            meta.append({"nf": nf, "frames": frames, "delays": dfs, "name": fn})
        frame0_map.append(f0map)
        slots_meta.append(meta)

    terrains = []
    for m, (cells, used) in enumerate(maps_data):
        slot_of = {g: s for s, g in enumerate(anim_slot_gids[m])}
        img, terrain = convert_map(m, ts_px, cells, used, palette, lut,
                                   frame0_map[m], slot_of)
        img.save(os.path.join(RES, f"{PFX}_map{m}.png"))
        print(f"    -> res/{PFX}_map{m}.png {img.size}")
        terrains.append(terrain)
    for m in range(MAPS_PER_STAGE):
        n = sum(1 for v in anim_cell_slot[m] if v)
        print(f"  map {m}: tile3={sum(bin(b).count('1') for b in tile3_masks[m])} "
              f"anim_cells={n}")

    bg_infos, bg_names = [], []
    for m in range(MAPS_PER_STAGE):
        _, bg0, _ = read_map(STAGE_INDEX, m)
        name, adj, spd, auto = bg0
        if name:
            info = convert_background(m, name, adj, spd)
            info["auto_scroll"] = auto
            bg_infos.append(info)
            bg_names.append(name)
        else:
            bg_infos.append({"present": 0, "adjust_y": 0, "speed": 0,
                             "auto_scroll": 0})
            bg_names.append("")

    lock_max = max(1, max(len(l) for l in scroll_locks))

    # ---- header ----
    with open(os.path.join(SRC, f"{PFX}_data.h"), "w") as f:
        f.write(f"#ifndef {UP}_DATA_H\n#define {UP}_DATA_H\n\n"
                "#include <genesis.h>\n#include \"stage_data.h\"\n\n"
                f"// Generated by tools/convert_stage.py from stage "
                f"{STAGE_INDEX} (\"{stage['name']}\").\n"
                f"extern const StageDef {PFX}_def;\n\n#endif\n")

    # ---- source ----
    c = [f'#include "{PFX}_data.h"\n',
         '#include "resources.h"\n',
         f'#include "{PFX}_enemies.h"\n',
         f'#include "{PFX}_objects.h"\n']
    for m in range(MAPS_PER_STAGE):
        if bg_names[m]:
            c.append(f'#include "{PFX}_bg{m}_tiles.h"\n')
            c.append(f'#include "{PFX}_bg{m}_map.h"\n')
    c.append("\n")

    c.append(f"static const u8 {PFX}_terrain[{MAPS_PER_STAGE}]"
             f"[{MAP_W * MAP_H}] = {{\n")
    for terr in terrains:
        c.append("    {\n")
        for y in range(MAP_H):
            row = ",".join(str(terr[y * MAP_W + x]) for x in range(MAP_W))
            c.append("        " + row + ",\n")
        c.append("    },\n")
    c.append("};\n\n")

    c.append(f"static const StageLink {PFX}_links[{max(1,len(links))}] = {{\n")
    for (i, frm, to, ox, oy, dx, dy) in links:
        oy = 0 if oy == 0 else oy - 1
        dy = 0 if dy == 0 else dy - 1
        c.append(f"    {{ {frm}, {ox}, {oy}, {to}, {dx}, {dy} }}, // link{i}\n")
    c.append("};\n\n")

    c.append(f"static const StageBgInfo {PFX}_bg_info[{MAPS_PER_STAGE}] = {{\n")
    for m, info in enumerate(bg_infos):
        c.append(f"    {{ {info['present']}, {info['adjust_y']}, "
                 f"{info['speed']}, {info['auto_scroll']} }}, // map {m}\n")
    c.append("};\n\n")

    c.append(f"static const u8 {PFX}_scroll_lock_n[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(str(len(l)) for l in scroll_locks))
    c.append("\n};\n\n")
    c.append(f"static const u8 {PFX}_scroll_lock[{MAPS_PER_STAGE}][{lock_max}] = {{\n")
    for locks in scroll_locks:
        padded = locks + [0] * (lock_max - len(locks))
        c.append("    { " + ", ".join(str(v) for v in padded) + " },\n")
    c.append("};\n\n")

    def emit_mask(name, masks):
        c.append(f"static const u8 {name}[{MAPS_PER_STAGE}][{mask_bytes}] = {{\n")
        for mask in masks:
            c.append("    {\n")
            for i in range(0, len(mask), 16):
                chunk = ", ".join("0x%02X" % b for b in mask[i:i + 16])
                c.append("        " + chunk + ",\n")
            c.append("    },\n")
        c.append("};\n\n")

    emit_mask(f"{PFX}_tile3_mask", tile3_masks)

    c.append(f"static const u8 {PFX}_anim_cell_slot[{MAPS_PER_STAGE}]"
             f"[{MAP_W * MAP_H}] = {{\n")
    for cs in anim_cell_slot:
        c.append("    {\n")
        for i in range(0, len(cs), 32):
            c.append("        " + ", ".join(str(b) for b in cs[i:i + 32]) + ",\n")
        c.append("    },\n")
    c.append("};\n\n")
    c.append(f"static const u8 {PFX}_anim_slot_n[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(str(len(g)) for g in anim_slot_gids) + "\n};\n\n")

    c.append(f"static const StageAnimSlot {PFX}_anim_slots"
             f"[{MAPS_PER_STAGE}][{anim_slot_max}] = {{\n")
    for meta in slots_meta:
        rows = [f"{{ {m['nf']}, {{ " + ", ".join(str(d) for d in m['delays'])
                + " } }" for m in meta]
        rows += ["{ 1, { 1, 1, 1, 1, 1, 1 } }"] * (anim_slot_max - len(rows))
        c.append("    { " + ", ".join(rows) + " },\n")
    c.append("};\n\n")

    c.append(f"static const u16 {PFX}_anim_frames[{MAPS_PER_STAGE}]"
             f"[{anim_slot_max}][6][64] = {{\n")
    for meta in slots_meta:
        c.append("    {\n")
        for m in meta:
            c.append("      {\n")
            for words in m["frames"]:
                c.append("        { " + ", ".join("0x%04X" % w for w in words) + " },\n")
            c.append("      },\n")
        for _ in range(anim_slot_max - len(meta)):
            c.append("      { { 0 } },\n")
        c.append("    },\n")
    c.append("};\n\n")

    c.append(f"static const MapDefinition *const {PFX}_map_defs[{MAPS_PER_STAGE}] = {{\n")
    c.append("    " + ", ".join(f"&{PFX}_map{m}" for m in range(MAPS_PER_STAGE)) + "\n};\n")
    c.append(f"static const TileSet *const {PFX}_tile_sets[{MAPS_PER_STAGE}] = {{\n")
    c.append("    " + ", ".join(f"&{PFX}_tiles{m}" for m in range(MAPS_PER_STAGE)) + "\n};\n")
    c.append(f"static const Palette *const {PFX}_palettes[{MAPS_PER_STAGE}] = {{\n")
    c.append("    " + ", ".join(f"&{PFX}_pal{m}" for m in range(MAPS_PER_STAGE)) + "\n};\n")
    c.append(f"static const Palette *const {PFX}_bg_pal[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(f"&{PFX}_bg{m}_pal" if bg_names[m] else "NULL"
                       for m in range(MAPS_PER_STAGE)))
    c.append("\n};\n")
    c.append(f"static const u32 *const {PFX}_bg_tiles[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(f"{PFX}_bg{m}_tiles_data" if bg_names[m] else "NULL"
                       for m in range(MAPS_PER_STAGE)))
    c.append("\n};\n")
    c.append(f"static const u16 {PFX}_bg_tiles_n[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(f"{UP}_BG{m}_TILE_COUNT" if bg_names[m] else "0"
                       for m in range(MAPS_PER_STAGE)))
    c.append("\n};\n")
    c.append(f"static const u16 *const {PFX}_bg_maps[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(f"{PFX}_bg{m}_map" if bg_names[m] else "NULL"
                       for m in range(MAPS_PER_STAGE)))
    c.append("\n};\n")
    c.append(f"static const u16 {PFX}_bg_map_h[{MAPS_PER_STAGE}] = {{\n    ")
    c.append(", ".join(f"{UP}_BG{m}_MAP_H" if bg_names[m] else "0"
                       for m in range(MAPS_PER_STAGE)))
    c.append("\n};\n\n")

    c.append(f"const StageDef {PFX}_def = {{\n")
    c.append(f"    .map_count = {MAPS_PER_STAGE}, .map_w = {MAP_W}, "
             f".map_h = {MAP_H}, .tile = {TILE},\n")
    c.append(f"    .map_defs = {PFX}_map_defs, .tile_sets = {PFX}_tile_sets, "
             f".palettes = {PFX}_palettes,\n")
    c.append(f"    .terrain = &{PFX}_terrain[0][0],\n")
    c.append(f"    .links = {PFX}_links, .link_count = {len(links)},\n")
    c.append(f"    .bg_info = {PFX}_bg_info, .bg_pal = {PFX}_bg_pal, "
             f".bg_tiles = {PFX}_bg_tiles,\n")
    c.append(f"    .bg_tiles_n = {PFX}_bg_tiles_n, .bg_maps = {PFX}_bg_maps, "
             f".bg_map_h = {PFX}_bg_map_h,\n")
    c.append(f"    .scroll_lock_n = {PFX}_scroll_lock_n, "
             f".scroll_lock = &{PFX}_scroll_lock[0][0], .scroll_lock_max = {lock_max},\n")
    c.append(f"    .tile3_mask = &{PFX}_tile3_mask[0][0],\n")
    c.append(f"    .anim_cell_slot = &{PFX}_anim_cell_slot[0][0], "
             f".anim_slot_n = {PFX}_anim_slot_n, .anim_slot_max = {anim_slot_max},\n")
    c.append(f"    .anim_slots = &{PFX}_anim_slots[0][0], "
             f".anim_frames = &{PFX}_anim_frames[0][0],\n")
    c.append(f"    .enemy_defs = {PFX}_enemy_defs, "
             f".enemy_def_count = {UP}_ENEMY_DEF_COUNT,\n")
    c.append(f"    .enemy_spawns = {PFX}_enemy_spawns, "
             f".enemy_spawn_count = {UP}_ENEMY_SPAWN_COUNT,\n")
    c.append(f"    .enemy_ai = {PFX}_enemy_ai, .enemy_proj = {PFX}_enemy_proj, "
             f".enemy_proj_max = {UP}_ENEMY_PROJ_MAX,\n")
    c.append(f"    .object_defs = {PFX}_object_defs, "
             f".object_def_count = {UP}_OBJECT_DEF_COUNT,\n")
    c.append(f"    .object_spawns = {PFX}_object_spawns, "
             f".object_spawn_count = {UP}_OBJECT_SPAWN_COUNT,\n")
    c.append("};\n")

    with open(os.path.join(SRC, f"{PFX}_data.c"), "w") as f:
        f.write("".join(c))
    print(f"wrote src/{PFX}_data.c/h ({len(terrains)} maps, {len(links)} links, "
          f"{anim_slot_max} anim slots)")
    print(f"\n--- res/resources.res ---")
    for m in range(MAPS_PER_STAGE):
        print(f'TILESET {PFX}_tiles{m} "{PFX}_map{m}.png" FAST ALL')
        print(f'PALETTE {PFX}_pal{m}   "{PFX}_map{m}.png"')
        print(f'MAP     {PFX}_map{m}   "{PFX}_map{m}.png"  {PFX}_tiles{m} FAST 0')
    for m in range(MAPS_PER_STAGE):
        if bg_names[m]:
            print(f'PALETTE {PFX}_bg{m}_pal "{PFX}_bg{m}_tiles.png"')
    return 0


if __name__ == "__main__":
    sys.exit(main())
