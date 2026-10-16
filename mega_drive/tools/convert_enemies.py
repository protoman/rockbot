#!/usr/bin/env python3
"""Convert stage 0 enemies + the normal projectile for the Mega Drive port.

Reads (never invents values):
  build/games/RockDroid1/game_enemy_list_3_1_2.dat   6048B records (g++ probe)
  build/games/RockDroid1/map_npc_data.dat            12B records
  build/games/RockDroid1/game_projectile_list.dat    104B records
  build/games/RockDroid1/images/sprites/enemies/*.png
  build/games/RockDroid1/images/projectiles/projectile_normal.png
  build/games/RockDroid1/maps_v2.dat                 terrain for the ground drop

Emits:
  res/enemy_<gfx>.png      SGDK SPRITE sheets: row0 = STAND, row1 = WALK
  res/shot.png             8x8 cell, 6x6 art (top-left), projectile_normal
  src/stage0_enemies.c/h   EnemyDef + EnemySpawn tables + shared PAL0 palette
  a SPRITE block to paste into res/resources.res (printed)

Simplifications (per user: precompute in converter, simple runtime AI):
  - difficulty: only spawns with difficulty_level <= 0 are emitted
    (PC rule classmap.cpp:545, game_save.difficulty = EASY).
  - stage boss spawns (is_boss) are skipped -> milestone M7.
  - walkers are pre-dropped onto the ground (PC
    initialize_npc_position_to_ground, character.cpp:3213).
  - patrol limits precomputed from file_npc.walk_range: spawn +/- range.
  - one shared 14-color enemy palette + white/black pins on PAL0
    (index 0 = transparent, 15 = font white) - font/HUD and enemies share
    the line, so VDP_drawText keeps working (idx15).

Usage: python3 tools/convert_enemies.py
"""
import os
import struct
import sys
from collections import Counter
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
DATA = os.path.join(ROCKBOT, "build/games/RockDroid1")
RES = os.path.join(ROOT, "res")
SRC = os.path.join(ROOT, "src")

STAGE_INDEX = 0
MAPS_PER_STAGE = 3
MAP_W = 200
MAP_H_SRC = 15          # source rows; convert_stage crops the top row
MAP_H = 14              # MD rows
MAP_STRUCT = 15078
MAP_TILES_OFF = 78
TILE_STRUCT = 5

NPC_SZ = 6048
PROJ_SZ = 104
MAP_NPC_SZ = 12

ANIM_STAND, ANIM_WALK = 0, 1
COLORKEY = (75, 125, 125)
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)
WHITE8 = 219           # md_round(219/238/235) == 219 -> CRAM 0x0EEE (font white)
MAX_POOL = 14          # indices 1..14; 0 = transparent, 15 = white


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def cstr(b, o, n):
    return b[o:o + n].split(b"\0")[0].decode("ascii", "replace")


def ceil8(v):
    return (v + 7) & ~7


# ---------------------------------------------------------------- read files

def read_enemy_defs(blob, ids):
    defs = {}
    for eid in sorted(ids):
        b = blob[eid * NPC_SZ:(eid + 1) * NPC_SZ]
        assert b[0] == eid, f"record {eid}: id byte {b[0]} != {eid}"
        stand, walk = [], []
        for anim, out in ((ANIM_STAND, stand), (ANIM_WALK, walk)):
            for f in range(10):
                o = 78 + (anim * 10 + f) * 14
                used, dur, px = b[o], struct.unpack_from("<H", b, o + 2)[0], b[o + 4]
                rect = struct.unpack_from("<4h", b, o + 6)
                if used:
                    out.append((px, dur, rect))
        defs[eid] = {
            "id": eid,
            "name": cstr(b, 3, 30),
            "gfx": cstr(b, 33, 30),
            "hp": struct.unpack_from("<h", b, 64)[0],
            "speed": b[69],
            "walk_range": struct.unpack_from("<h", b, 70)[0],
            "fw": struct.unpack_from("<b", b, 5958)[0],
            "fh": struct.unpack_from("<b", b, 5959)[0],
            "ia": struct.unpack_from("<b", b, 5962)[0],
            "fly": b[5963],
            "bgx": struct.unpack_from("<h", b, 6022)[0],
            "bgy": struct.unpack_from("<h", b, 6024)[0],
            "is_boss": b[6026],
            "stand": stand,
            "walk": walk,
        }
    return defs


def read_spawns():
    mb = open(os.path.join(DATA, "map_npc_data.dat"), "rb").read()
    out = []
    for i in range(len(mb) // MAP_NPC_SZ):
        r = mb[i * MAP_NPC_SZ:(i + 1) * MAP_NPC_SZ]
        if r[7] != STAGE_INDEX:
            continue
        out.append({
            "id": struct.unpack_from("<b", r, 0)[0],
            "x": struct.unpack_from("<h", r, 2)[0],
            "y": struct.unpack_from("<h", r, 4)[0],
            "dir": r[6],
            "map": r[8],
            "diffL": r[9],
        })
    return out


def read_terrain():
    """MD grid per map: source col[1:] (top row cropped, like convert_stage)."""
    grids = []
    with open(os.path.join(DATA, "maps_v2.dat"), "rb") as f:
        for m in range(MAPS_PER_STAGE):
            f.seek(MAP_STRUCT * (STAGE_INDEX * MAPS_PER_STAGE + m))
            raw = f.read(MAP_STRUCT)
            t = raw[MAP_TILES_OFF:MAP_TILES_OFF + MAP_W * MAP_H_SRC * TILE_STRUCT]
            col_major = []
            for x in range(MAP_W):
                col = []
                for y in range(MAP_H_SRC):
                    col.append(t[(x * MAP_H_SRC + y) * TILE_STRUCT])
                col_major.append(col)
            grids.append([col[1:] for col in col_major])   # crop top row
    return grids


def solid(grid, tx, ty):
    if tx < 0 or tx >= MAP_W or ty < 0 or ty >= MAP_H:
        return tx < 0 or tx >= MAP_W   # sides block, bottom does not
    return grid[tx][ty] == 1           # TERRAIN_SOLID


def drop_to_ground(grid, x, y, box_t, box_h):
    bottom = box_t + box_h
    feet_l = x >> 4
    feet_r = (x + 8) >> 4             # center-ish probe; refined by caller box
    for _ in range(MAP_H * 16 + 16):
        row = (y + bottom) >> 4
        if row >= MAP_H:
            return y, False
        hit = any(solid(grid, tx, row) for tx in range(feet_l, feet_r + 1))
        if hit:
            return row * 16 - bottom, True
        y += 1
    return y, False


# ---------------------------------------------------------------- sheets

def load_rgba(path):
    """RGBA with (75,125,125) colorkey punched to alpha 0 (mixed source modes)."""
    im = Image.open(path)
    if im.mode == "RGBA":
        rgba = im
    else:
        rgba = im.convert("RGBA")
    px = rgba.load()
    w, h = rgba.size
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a < 255 or (r, g, b) == COLORKEY:
                px[x, y] = (0, 0, 0, 0)
    return rgba


def collect_colors(images):
    counts = Counter()
    for im in images:
        w, h = im.size
        px = im.load()
        for y in range(h):
            for x in range(w):
                if px[x, y][3] > 0:
                    r, g, b = px[x, y][:3]
                    counts[(md_round(r), md_round(g), md_round(b))] += 1
    return counts


def build_pool(counts):
    """<= MAX_POOL colors; black stays (art), white merges into idx15."""
    pool = [c for c, _ in counts.most_common() if c != (WHITE8,) * 3]
    if (0, 0, 0) in pool:                    # keep black first: most frequent use
        pool.remove((0, 0, 0))
        pool.insert(0, (0, 0, 0))
    while len(pool) > MAX_POOL:              # merge nearest pair (convert_stage style)
        bi, bj, bd = 0, 1, 1 << 30
        for i in range(len(pool)):
            for j in range(i + 1, len(pool)):
                a, b = pool[i], pool[j]
                d = (a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2 + (a[2] - b[2]) ** 2
                if d < bd:
                    bd, bi, bj = d, i, j
        del pool[bj]
    return pool


def make_lut(pool):
    """transparent -> 0, white -> 15, otherwise nearest of 1..14 (never 0)."""
    def nearest(c):
        best, bd = 1, 1 << 30
        for i, p in enumerate(pool):
            d = (c[0] - p[0]) ** 2 + (c[1] - p[1]) ** 2 + (c[2] - p[2]) ** 2
            if d < bd:
                bd, best = d, i + 1
        return best
    return nearest


def to_vdp(c):
    r = MD_LEVELS.index(c[0])
    g = MD_LEVELS.index(c[1])
    b = MD_LEVELS.index(c[2])
    return r | (g << 4) | (b << 8)


# ---------------------------------------------------------------- main

def main():
    blob = open(os.path.join(DATA, "game_enemy_list_3_1_2.dat"), "rb").read()
    spawns_all = read_spawns()

    used_ids = []
    for s in spawns_all:
        if s["id"] < 0 or s["id"] * NPC_SZ >= len(blob):
            continue
        if s["diffL"] > 0:                    # PC classmap.cpp:545, easy mode
            print(f"skip spawn id={s['id']} map={s['map']} pos=({s['x']},{s['y']}): difficulty_level {s['diffL']} > 0")
            continue
        eid = blob[s["id"] * NPC_SZ]
        is_boss = blob[s["id"] * NPC_SZ + 6026]
        if is_boss:
            print(f"skip spawn id={s['id']} pos=({s['x']},{s['y']}): is_boss (M7)")
            continue
        used_ids.append(s["id"])

    defs = read_enemy_defs(blob, set(used_ids))
    grids = read_terrain()

    # --- sheets -------------------------------------------------------------
    res_names = {}          # id -> resources.res SPRITE name
    sheet_imgs = {}         # id -> indexed sheet (filled after palette)
    plans = {}              # id -> dict with cells/times/meta
    for eid, d in sorted(defs.items()):
        path = os.path.join(DATA, "images/sprites", "sprites", "enemies", d["gfx"])
        if not os.path.exists(path):
            path = os.path.join(DATA, "images/sprites/enemies", d["gfx"])
        rgba = load_rgba(path)
        fw, fh = d["fw"], d["fh"]
        cw, ch = ceil8(fw), ceil8(fh)
        assert cw <= 32 and ch <= 32, f"{d['gfx']}: cell {cw}x{ch} > 32"

        def frames_of(anim):
            return [(px, dur) for (px, dur, _rect) in anim]

        stand = frames_of(d["stand"]) or frames_of(d["walk"])
        walk = frames_of(d["walk"]) or stand
        # all stand frames / all walk frames, times in 1/60s
        def times(fr):
            return [max(1, round(dur * 6 / 100)) for _, dur in fr]

        # union collision rect over the frames we ship
        rects = [r for (_, _, r) in d["stand"] + d["walk"]]
        bl = min(r[0] for r in rects)
        bt = min(r[1] for r in rects)
        br = max(r[0] + r[2] for r in rects)
        bb = max(r[1] + r[3] for r in rects)

        cols = max(len(stand), len(walk))
        src = rgba.load()
        cells = []
        for row_frames in (stand, walk):
            row = []
            for (fcol, _dur) in row_frames:
                art = []
                for y in range(fh):
                    for x in range(fw):
                        art.append(src[fcol * fw + x, y])
                row.append(art)
            cells.append(row)

        plans[eid] = {
            "d": d, "cw": cw, "ch": ch, "fw": fw, "fh": fh, "cols": cols,
            "stand": stand, "walk": walk, "stand_t": times(stand),
            "walk_t": times(walk), "cells": cells,
            "box": (bl, bt, br - bl, bb - bt),
            "rgba": rgba,
            "res_name": "enemy_" + os.path.splitext(d["gfx"])[0],
        }
        res_names[eid] = plans[eid]["res_name"]
        print(f"id={eid} {d['name']} gfx={d['gfx']} hp={d['hp']} spd={d['speed']} "
              f"fly={d['fly']} ia={d['ia']} cell={cw}x{ch} box=(bl,bt,w,h)={plans[eid]['box']} "
              f"stand={len(stand)}f walk={len(walk)}f bg=({d['bgx']},{d['bgy']})")

    # --- shot ---------------------------------------------------------------
    shot_rgba = load_rgba(os.path.join(DATA, "images/projectiles/projectile_normal.png"))
    proj = open(os.path.join(DATA, "game_projectile_list.dat"), "rb").read()
    p0 = proj[0:PROJ_SZ]
    shot_speed = p0[68]
    shot_dmg = p0[69]
    print(f"projectile 0: '{cstr(p0,0,30)}' speed={shot_speed} damage={shot_dmg} "
          f"max_shots={p0[67]} size={struct.unpack_from('<hh', p0, 60)}")

    # --- shared palette (enemy art + shot art) ------------------------------
    all_imgs = [p["rgba"] for p in plans.values()] + [shot_rgba]
    pool = build_pool(collect_colors(all_imgs))
    lut = make_lut(pool)
    print(f"shared PAL0 pool: {len(pool)} colors -> {pool}")

    palette = [(0, 0, 0)] + [c for c in pool]
    while len(palette) < 16:
        palette.append((0, 0, 0))
    palette[15] = (WHITE8, WHITE8, WHITE8)

    # --- write sprite sheets ------------------------------------------------
    os.makedirs(RES, exist_ok=True)
    for eid, p in sorted(plans.items()):
        cols, cw, ch = p["cols"], p["cw"], p["ch"]
        out = Image.new("P", (cols * cw, 2 * ch))
        out.putpalette([v for c in palette for v in c])
        data = [0] * (cols * cw * 2 * ch)
        for r, row in enumerate(p["cells"]):
            for c, art in enumerate(row):
                ox, oy = c * cw, r * ch
                for i, px in enumerate(art):
                    x, y = i % p["fw"], i // p["fw"]
                    if px[3] == 0:
                        idx = 0
                    else:
                        idx = lut((md_round(px[0]), md_round(px[1]), md_round(px[2])))
                    data[(oy + y) * cols * cw + ox + x] = idx
        out.putdata(data)
        out.save(os.path.join(RES, p["res_name"] + ".png"))
        print(f"wrote res/{p['res_name']}.png {out.size}")

    # shot: 6x6 art in an 8x8 cell (SGDK cells like multiples of 8)
    out = Image.new("P", (8, 8))
    out.putpalette([v for c in palette for v in c])
    data = [0] * 64
    src = shot_rgba.load()
    for y in range(shot_rgba.size[1]):
        for x in range(shot_rgba.size[0]):
            px = src[x, y]
            idx = 0 if px[3] == 0 else lut((md_round(px[0]), md_round(px[1]), md_round(px[2])))
            data[y * 8 + x] = idx
    out.putdata(data)
    out.save(os.path.join(RES, "shot.png"))
    print("wrote res/shot.png 8x8")

    # charge shot (semi/full): 16x16, 4-frame pulse, same PAL0 pool.
    # ring = nearest cream, core = pinned white (idx 15).
    import math
    ring_idx = lut((md_round(255), md_round(218), md_round(145)))
    out = Image.new("P", (64, 16))
    out.putpalette([v for c in palette for v in c])
    data = [0] * (64 * 16)
    for f in range(4):
        r_out = 6.5 - (f % 2)          # pulse between 6.5 and 5.5
        r_in = 2.5 + (f % 2) * 0.5
        for y in range(16):
            for x in range(16):
                d = math.hypot(x - 7.5, y - 7.5)
                if d <= r_in:
                    idx = 15
                elif d <= r_out:
                    idx = ring_idx
                else:
                    idx = 0
                data[y * 64 + f * 16 + x] = idx
    out.putdata(data)
    out.save(os.path.join(RES, "shot_charge.png"))
    print("wrote res/shot_charge.png 16x16 x4 (charge shot)")

    # --- spawns -------------------------------------------------------------
    spawn_out = []
    for s in spawns_all:
        if s["id"] not in defs:
            continue
        d = defs[s["id"]]
        p = plans[s["id"]]
        box_l, box_t, box_w, box_h = p["box"]
        x = s["x"] * 16 + d["bgx"]
        y = max(0, (s["y"] - 1) * 16) + d["bgy"]
        if not d["fly"]:
            y, ok = drop_to_ground(grids[s["map"]], x + box_l, y, box_t, box_h)
            if not ok:
                print(f"  !! spawn id={s['id']} map={s['map']} pos=({s['x']},{s['y']}): no ground found")
        x_min = max(0, x - d["walk_range"])
        x_max = min(MAP_W * 16 - box_w, x + d["walk_range"])
        def_idx = sorted(defs).index(s["id"])
        spawn_out.append((def_idx, s["map"], x, y, x_min, x_max, s["dir"]))
        print(f"spawn def={def_idx} id={s['id']} map={s['map']} px=({x},{y}) "
              f"range=[{x_min},{x_max}] dir={s['dir']}")

    # --- stage0_enemies.h/.c ------------------------------------------------
    ids_sorted = sorted(defs)
    h = f"""#ifndef STAGE0_ENEMIES_H
#define STAGE0_ENEMIES_H

#include <genesis.h>

// Generated by tools/convert_enemies.py from game_enemy_list_3_1_2.dat and
// map_npc_data.dat. Do not edit by hand.

typedef struct {{
    const SpriteDefinition *spr;
    u8 hp;              // shots to kill (file_npc.hp.total)
    u8 speed;           // px/frame (file_npc.speed)
    u8 fly;             // 1 = no gravity (file_npc.fly_flag)
    s16 box_l, box_t;   // hitbox relative to sprite cell origin
    u16 box_w, box_h;
    u8 stand_n;         // frames in sheet row 0 (ANIM_STAND)
    u8 walk_n;          // frames in sheet row 1 (ANIM_WALK)
}} EnemyDef;

typedef struct {{
    u8 def;             // index into enemy_defs
    u8 map;             // map index (0..2)
    s16 x, y;           // spawn position in pixels (ground-dropped, walkers)
    s16 x_min, x_max;   // precomputed patrol limits (file_npc.walk_range)
    u8 dir;             // ANIM_DIRECTION: 0 = left, 1 = right
}} EnemySpawn;

#define ENEMY_DEF_COUNT {len(ids_sorted)}
#define ENEMY_SPAWN_COUNT {len(spawn_out)}

extern const EnemyDef enemy_defs[ENEMY_DEF_COUNT];
extern const EnemySpawn stage0_enemy_spawns[ENEMY_SPAWN_COUNT];

// PAL0 = font white (idx 15) + this palette; index 0 = transparent.
extern const u16 palette_ui_enemies[16];

#endif
"""
    with open(os.path.join(SRC, "stage0_enemies.h"), "w") as f:
        f.write(h)

    def_idx_name = {eid: i for i, eid in enumerate(ids_sorted)}
    lines = ['#include "stage0_enemies.h"\n', '#include "resources.h"\n\n']
    lines.append("const EnemyDef enemy_defs[ENEMY_DEF_COUNT] = {\n")
    for eid in ids_sorted:
        p = plans[eid]
        d = defs[eid]
        bl, bt, bw, bh = p["box"]
        lines.append(
            f"    /* {def_idx_name[eid]} */ {{ &{p['res_name']}_sprite, "
            f"{d['hp']}, {d['speed']}, {d['fly']}, "
            f"{bl}, {bt}, {bw}, {bh}, "
            f"{len(p['stand'])}, {len(p['walk'])} }},  // {d['name']}\n")
    lines.append("};\n\n")
    lines.append("const EnemySpawn stage0_enemy_spawns[ENEMY_SPAWN_COUNT] = {\n")
    for (di, mp, x, y, xm, xx, dr) in spawn_out:
        lines.append(f"    {{ {di}, {mp}, {x}, {y}, {xm}, {xx}, {dr} }},\n")
    lines.append("};\n\n")
    lines.append("const u16 palette_ui_enemies[16] = {\n    0x0000,")
    for c in pool:
        lines.append(f" 0x{to_vdp(c):03X},")
    lines.append("\n")
    lines.append("    " + "  ".join("0x0000," for _ in range(14 - len(pool))) + " 0x0EEE\n")
    lines.append("};\n")
    with open(os.path.join(SRC, "stage0_enemies.c"), "w") as f:
        f.writelines(lines)
    print(f"wrote src/stage0_enemies.c/h ({len(ids_sorted)} defs, {len(spawn_out)} spawns)")

    # --- resources.res block ------------------------------------------------
    print("\n--- paste into res/resources.res ---")
    for eid in ids_sorted:
        p = plans[eid]
        st = ",".join(str(t) for t in p["stand_t"])
        wk = ",".join(str(t) for t in p["walk_t"])
        print(f'SPRITE {p["res_name"]}_sprite "{p["res_name"]}.png" '
              f'{p["cw"]}p {p["ch"]}p FAST [[{st}][{wk}]]')
    print('SPRITE shot_sprite "shot.png" 8p 8p FAST [[1]]')
    print(f"shot_speed={shot_speed} shot_dmg={shot_dmg}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
