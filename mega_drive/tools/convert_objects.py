#!/usr/bin/env python3
"""Convert stage 0 objects (pills, 1UP, fall/fly platforms, checkpoint).

Reads (never invents values):
  build/games/RockDroid1/game_object_list.dat   84B records (struct file_object)
  build/games/RockDroid1/map_object_data.dat    18B records (file_map_object_v2)
  images/sprites/objects/*.png                   hand art (indexed PAL0)
  build/games/RockDroid1/maps_v2.dat            terrain for the load-time drop
  src/stage0_enemies.c                          shared PAL0 pool (parsed)

Emits:
  res/object_<gfx>.png     SGDK SPRITE sheet, single anim row
  src/stage0_objects.c/h   ObjectDef + ObjectSpawn tables (shares PAL0)
  a SPRITE block to paste into res/resources.res (printed)

Rules (mirrors PC, verified in source):
  - difficulty: no difficulty setting in this port — keep every spawn
    except hard-only ones (difficulty_level >= 2; 0=easy, 1=normal,
    2=hard, defines.h:1314). PC rule classmap.cpp:1326-1331.
  - boss teleporter (type 32) kept (M6b): dest tiles from the spawn's
    _lx/_ly/_mdest fields (PC game::object_teleport_boss destination).
  - spawn px: x = sx*16, y = sy*16 (the data is already cropped to 14 rows).
  - load-time drop for gravity types (pill/1UP/checkpoint):
    object::initialize_object_position_to_ground() runs gravity()
    h times; each step up to GRAVITY_SPEED=4px, blocked when the
    destination bottom corners (x+2, x+w-2) at row y+inc+h-2 hit
    SOLID. Fall/fly platforms are in gravity()'s skip list -> keep
    the authored position.
  - art uses the existing PAL0 pool parsed from palette_ui_enemies and is
    copied 1:1 from the indexed hand art (no re-quantizing, no new palette).

Usage: /home/iuri/python3/bin/python3 tools/convert_objects.py
"""
import os
import re
import struct
import sys

from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pal_ui import PAL_UI_RGB  # noqa: E402
from convert_enemies import (  # noqa: E402
    MD_LEVELS, ceil8, load_indexed, read_terrain, solid, MAP_W, MAP_H,
)

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
DATA = os.path.join(ROOT, "data/games/RockDroid1")
IMAGES = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "images")  # hand-owned art
RES = os.path.join(ROOT, "res")
SRC = os.path.join(ROOT, "src")

STAGE_INDEX = int(sys.argv[1]) if len(sys.argv) > 1 else 0
PFX = "stage%d" % STAGE_INDEX
UP = PFX.upper()
OBJ_SZ = 84
MAP_OBJ_SZ = 18
RES_H = 224
GRAV = 4

# PC OBJECT_TYPE values (defines.h:363)
OBJ_DISAPPEARING_BLOCK = 2
OBJ_MOVING_LR = 1
OBJ_ACTIVE_DISAPPEARING_BLOCK = 14
OBJ_ARMOR_ARMS = 15
OBJ_ARMOR_BODY = 16
OBJ_ARMOR_LEGS = 17
OBJ_RAY_H = 18
OBJ_RAY_V = 19
OBJ_BOSS_TELEPORTER = 21
OBJ_DESTRUCTIBLE = 22
OBJ_SPECIAL_TANK = 23
OBJ_DEATHRAY_V = 24
OBJ_DEATHRAY_H = 25
OBJ_FINAL_BOSS_TELEPORTER = 26
OBJ_BOSS_DOOR = 30
OBJ_PLATFORM_TELEPORTER = 31
OBJ_ENERGY_TANK = 3
OBJ_WEAPON_TANK = 4
OBJ_ENERGY_PILL_BIG = 5
OBJ_WEAPON_PILL_BIG = 6
OBJ_ENERGY_PILL_SMALL = 7
OBJ_WEAPON_PILL_SMALL = 8
OBJ_LIFE = 9
OBJ_FALL_PLATFORM = 10
OBJ_FLY_PLATFORM = 11
OBJ_CHECKPOINT = 29
OBJ_STAGE_BOSS_TELEPORTER = 32
OBJ_ITEM_FLY = 12
OBJ_ITEM_JUMP = 13
# RockDroid2 object types (no RockDroid1 stage spawns them; the behaviour is in
# src/object.c so its maps convert as soon as its stages are added).
OBJ_ACTIVE_SLIM_PLATFORM = 27
OBJ_DAMAGING_PLATFORM = 28
OBJ_CRUSHER = 33
EXTRA_TYPES = (OBJ_ACTIVE_SLIM_PLATFORM, OBJ_DAMAGING_PLATFORM, OBJ_CRUSHER)
# Player item objects (COIL FROG / JET EAGLE) are referenced by object id,
# not spawned by a map; stage 0 emits their art once for all stages.
ITEM_OBJECT_IDS = (16, 17)

# pickups with gravity (PC object::init runs the drop loop for every type
# except platforms/rays; these are the ones the converted stages spawn)
GRAVITY_TYPES = (OBJ_ENERGY_TANK, OBJ_WEAPON_TANK,
                 OBJ_ENERGY_PILL_BIG, OBJ_WEAPON_PILL_BIG,
                 OBJ_ENERGY_PILL_SMALL, OBJ_WEAPON_PILL_SMALL,
                 OBJ_LIFE, OBJ_CHECKPOINT, OBJ_ARMOR_ARMS, OBJ_ARMOR_BODY,
                 OBJ_ARMOR_LEGS, OBJ_SPECIAL_TANK)
PLATFORM_TYPES = (OBJ_MOVING_LR, OBJ_DISAPPEARING_BLOCK,
                  OBJ_ACTIVE_DISAPPEARING_BLOCK, OBJ_FALL_PLATFORM,
                  OBJ_FLY_PLATFORM, OBJ_DESTRUCTIBLE, OBJ_BOSS_DOOR)
HAZARD_TYPES = (OBJ_RAY_H, OBJ_RAY_V, OBJ_DEATHRAY_H, OBJ_DEATHRAY_V)
TELEPORT_TYPES = (OBJ_BOSS_TELEPORTER, OBJ_FINAL_BOSS_TELEPORTER,
                  OBJ_PLATFORM_TELEPORTER, OBJ_STAGE_BOSS_TELEPORTER)


def cstr(b, o, n):
    return b[o:o + n].split(b"\0")[0].decode("ascii", "replace")


def read_object_defs(blob, ids):
    defs = {}
    for oid in sorted(ids):
        b = blob[oid * OBJ_SZ:(oid + 1) * OBJ_SZ]
        defs[oid] = {
            "id": oid,
            "name": cstr(b, 0, 30),
            "gfx": cstr(b, 30, 30),
            "type": struct.unpack_from("<b", b, 60)[0],
            "speed": struct.unpack_from("<b", b, 68)[0],
            "timer": struct.unpack_from("<i", b, 64)[0],
            "limit": struct.unpack_from("<h", b, 70)[0],
            "w": struct.unpack_from("<b", b, 76)[0],
            "h": struct.unpack_from("<b", b, 77)[0],
            "fd": struct.unpack_from("<h", b, 78)[0],
            # file_object animation flags (offsets 80/81/82): the PC uses them
            # to decide whether an object animates on its own and whether the
            # animation loops (the checkpoint's flag loops once activated).
            "anim_auto": b[80],
            "anim_loop": b[82],
        }
    return defs


def read_spawns():
    mb = open(os.path.join(DATA, "map_object_data.dat"), "rb").read()
    out = []
    for i in range(len(mb) // MAP_OBJ_SZ):
        r = mb[i * MAP_OBJ_SZ:(i + 1) * MAP_OBJ_SZ]
        (oid, _pad, sx, sy, direction, _p2, _lx, _ly, _mdest,
         stage, mp, diffL, _diffM, _p3) = struct.unpack_from("<bbhhbBhhbbbbbb", r)
        if stage != STAGE_INDEX or oid == -1:
            continue
        # No difficulty in this port: keep every spawn except hard-only ones
        # (difficulty_level >= 2; 0=easy, 1=normal, 2=hard, defines.h:1314).
        # PC rule classmap.cpp:1326-1331 (mode+level).
        if diffL >= 2:
            print(f"skip spawn id={oid} map={mp} pos=({sx},{sy}): difficulty_level {diffL} (hard-only)")
            continue
        out.append({"id": oid, "x": sx, "y": sy, "dir": direction, "map": mp,
                    "dx": _lx, "dy": _ly, "dm": _mdest})
    return out


def blocks_fall(grid, tx, ty):
    """What stops a falling object box: the same rule as platform_can_move()
    in src/object.c (PC object::test_change_position) - everything except
    UNBLOCKED, SPIKE and WATER, so ice, conveyors and stairs are floors too."""
    if tx < 0 or tx >= MAP_W:
        return True
    if ty < 0 or ty >= MAP_H:
        return False                  # the bottom edge does not block
    return grid[tx][ty] not in (0, 4, 5)


def drop_to_ground(grid, x, y, w, h):
    """PC object gravity, settled completely at conversion time: fall GRAV=4px
    steps until a floor is under the box (it may rest with a 1..GRAV-1 px
    hover, as PC). The ROM then finds every object already at rest instead of
    dropping it in front of the player. Returns (y, fell_out)."""
    while True:
        dest = y + GRAV
        row = (dest + h - 2) >> 4
        if row > RES_H:               # PC: falling out of screen is allowed
            return dest, True
        c1 = (x + 2) >> 4
        c2 = (x + w - 2) >> 4
        if blocks_fall(grid, c1, row) or blocks_fall(grid, c2, row):
            return y, False
        y = dest


def main():
    blob = open(os.path.join(DATA, "game_object_list.dat"), "rb").read()
    spawns_all = read_spawns()

    used_ids = []
    for s in spawns_all:
        if s["id"] < 0 or s["id"] * OBJ_SZ >= len(blob):
            continue
        typ = struct.unpack_from("<b", blob, s["id"] * OBJ_SZ + 60)[0]
        if typ not in GRAVITY_TYPES and typ not in PLATFORM_TYPES and \
           typ not in TELEPORT_TYPES and typ not in HAZARD_TYPES and \
           typ not in EXTRA_TYPES and \
           typ not in (OBJ_ITEM_FLY, OBJ_ITEM_JUMP):
            print(f"skip spawn id={s['id']} map={s['map']} pos=({s['x']},{s['y']}): "
                  f"unsupported type {typ}")
            continue
        used_ids.append(s["id"])
    if STAGE_INDEX == 0:
        used_ids.extend(i for i in ITEM_OBJECT_IDS if i not in used_ids)

    defs = read_object_defs(blob, set(used_ids))
    grids = read_terrain()
    pal = list(PAL_UI_RGB)
    pool = pal[1:15]
    print(f"shared PAL0 pool: {len(pool)} colors -> {pool}")

    # --- sheets -------------------------------------------------------------
    plans = {}
    for oid, d in sorted(defs.items()):
        path = os.path.join(IMAGES, "sprites/objects", d["gfx"])
        sheet, spal = load_indexed(path)
        if spal != pal:
            print(f"  !! {path}: palette differs from the enemy PAL0")
        fw, fh = d["w"], d["h"]
        # MD sprite sheets top out at 64 wide / 80 tall in this project
        # (giant_fly is 64x48). The beams ship 80px wide: crop the ART to the
        # cell, keep the data size for the damage box.
        uw, uh = min(fw, 64), min(fh, 80)
        cw, ch = ceil8(uw), ceil8(uh)
        assert cw <= 64 and ch <= 80, f"{d['gfx']}: cell {cw}x{ch} too large"
        pw, ph = sheet.size
        # PC object art is usually a horizontal strip, but some sheets
        # (teleporter.png, platform_up/down) stack frames vertically.
        if pw % fw == 0 and ph % fh == 0:
            cols, rows = pw // fw, ph // fh
            frames = cols * rows
        else:
            raise AssertionError(
                f"{d['gfx']}: png {pw}x{ph} is not a grid of {fw}x{fh} frames")
        frame_t = max(1, round(d["fd"] * 6 / 100))
        src = sheet.load()
        cells = []
        if d["type"] == OBJ_DEATHRAY_H:
            # PC object::show_deathray_horizontal: base tile + `size` tiles of
            # body ending in a point; the beam grows to the first wall, so the
            # longest reach any spawn of this art can have bounds the sheet.
            # Frame = [(side*2 + flicker)] * N + size, N = maxlen + 1.
            maxlen = 2
            for sp in spawns_all:
                if sp["id"] not in defs or defs[sp["id"]]["gfx"] != d["gfx"]:
                    continue
                g = grids[sp["map"]]
                step = 1 if sp["dir"] == 1 else -1
                k = 1
                while k < 21:
                    tx = sp["x"] + step * k
                    if tx < 0 or tx >= len(g) or g[tx][sp["y"]] not in (0, 5):
                        break
                    k += 1
                maxlen = max(maxlen, min(14, k - 1))   # rescomp: frame width < 32 tiles
            n_sizes = maxlen + 1
            cw, ch = 16 * n_sizes, 16
            uw, uh = cw, 16
            for side in range(2):
                for rs in range(2):
                    for size in range(n_sizes):
                        art = [0] * (uw * uh)
                        # piece offsets inside a 64x16 art frame (row = side)
                        if side == 0:      # LEFT
                            base, body, point = 3, 1, 0
                        else:
                            base, body, point = 0, 2, 3

                        def put(k, piece):
                            cx = 16 * (maxlen - k) if side == 0 else 16 * k
                            if not 0 <= cx <= uw - 16:
                                return
                            for y in range(16):
                                for x in range(16):
                                    art[y * uw + cx + x] = src[rs * fw + piece * 16 + x, side * fh + y]
                        put(0, base)
                        if size == 1:
                            put(2, point)                  # PC draws the lone point 2 tiles out
                        elif size == 2:
                            put(2, point)
                            put(1, body)
                        elif size > 2:
                            put(size, point)
                            for k in range(1, size):
                                put(k, body)
                        cells.append(art)
            frames = len(cells)
            plans[oid] = {
                "d": d, "cw": cw, "ch": ch, "fw": fw, "fh": fh,
                "frames": frames, "frame_t": 1, "cells": cells,
                "uw": uw, "uh": uh,
                "res_name": "object_" + os.path.splitext(d["gfx"])[0],
            }
            print(f"id={oid} {d['name']} baked deathray sheet: {frames} frames of {cw}x{ch} (maxlen {maxlen})")
            continue
        if d["type"] == OBJ_DEATHRAY_V:
            # PC object::show_deathray_vertical (object.cpp:619): a base tile at the
            # top, then per tick `state` (1..30): the head runs down 15 tiles (state
            # 1: point at row 1; 2: body row 1 + point row 2; >2: body rows
            # 1..state-1 + point at `state`), then the tail follows (state 16..30:
            # body rows state-14..state-1 and a tail piece at row state-15).
            # Sheet pieces (16 px rows): 0 base, 1 tail, 2 body, 3 point; two
            # flicker columns of `fw`. Frame = flicker * 31 + state. Rows past the
            # 224 px screen are never visible, so the cell is 15 rows tall.
            ROWS = 15
            cw, ch = ceil8(fw), 16 * ROWS
            uw, uh = fw, ch
            for rs in range(2):
                for st in range(31):
                    art = [0] * (uw * uh)

                    def put(row, piece):
                        if not 0 <= row < ROWS:
                            return
                        for y in range(16):
                            for x in range(fw):
                                art[(row * 16 + y) * uw + x] = src[rs * fw + x, piece * 16 + y]
                    put(0, 0)
                    if st == 1:
                        put(1, 3)
                    elif st == 2:
                        put(2, 3)
                        put(1, 2)
                    elif 2 < st <= 15:
                        put(st, 3)
                        for r in range(1, st):
                            put(r, 2)
                    elif st > 15:
                        start = st - 15
                        for r in range(start + 1, st):
                            put(r, 2)
                        put(start, 1)
                    cells.append(art)
            frames = len(cells)
            plans[oid] = {
                "d": d, "cw": cw, "ch": ch, "fw": fw, "fh": fh,
                "frames": frames, "frame_t": 1, "cells": cells,
                "uw": uw, "uh": uh,
                "res_name": "object_" + os.path.splitext(d["gfx"])[0],
            }
            print(f"id={oid} {d['name']} baked vertical deathray sheet: {frames} frames of {cw}x{ch}")
            continue
        if d["type"] in (OBJ_RAY_V, OBJ_RAY_H):
            # PC object::show_vertical_ray / show_horizontal_ray draw a SLICE of
            # the sheet that grows with `state` (0..limit) and flickers between
            # two art columns every 70 ms. Bake every (flicker, state[, side])
            # combination as a frame of the full-size cell, so the game only
            # has to pick a frame number. Frame = [side*2+flicker]*5 + state.
            steps = (fh if d["type"] == OBJ_RAY_V else fw) // 16
            sides = 1 if d["type"] == OBJ_RAY_V else 2
            cw, ch = ceil8(fw), ceil8(fh)
            uw, uh = fw, fh
            for side in range(sides):
                for rs in range(2):
                    for st in range(steps):
                        art = [0] * (fw * fh)
                        for y in range(fh):
                            for x in range(fw):
                                if d["type"] == OBJ_RAY_V:
                                    if y < fh - 16 * (st + 1):
                                        continue
                                    art[y * fw + x] = src[rs * fw + x, y]
                                elif side == 0:      # LEFT: right-aligned slice of row 0
                                    if x < fw - 16 * (st + 1):
                                        continue
                                    art[y * fw + x] = src[rs * fw + x, y]
                                else:                # RIGHT: left-aligned slice of row 1
                                    if x >= 16 * (st + 1):
                                        continue
                                    art[y * fw + x] = src[rs * fw + x, fh + y]
                        cells.append(art)
            frames = len(cells)
            plans[oid] = {
                "d": d, "cw": cw, "ch": ch, "fw": fw, "fh": fh,
                "frames": frames, "frame_t": 1, "cells": cells,
                "uw": uw, "uh": uh,
                "res_name": "object_" + os.path.splitext(d["gfx"])[0],
            }
            print(f"id={oid} {d['name']} baked ray sheet: {frames} frames of {cw}x{ch}")
            continue
        for f in range(frames):
            art = []
            cx, cy = f % cols, f // cols
            for y in range(uh):
                for x in range(uw):
                    art.append(src[cx * fw + x, cy * fh + y])
            cells.append(art)
        plans[oid] = {
            "d": d, "cw": cw, "ch": ch, "fw": fw, "fh": fh,
            "frames": frames, "frame_t": frame_t, "cells": cells,
            "uw": uw, "uh": uh,
            "res_name": "object_" + os.path.splitext(d["gfx"])[0],
        }
        print(f"id={oid} {d['name']} gfx={d['gfx']} type={d['type']} "
              f"cell={cw}x{ch} frames={frames} frame_t={frame_t} spd={d['speed']}")

    # --- write sheets (reuse enemy PAL0; art indices copied 1:1) ------------
    os.makedirs(RES, exist_ok=True)
    for oid, p in sorted(plans.items()):
        cols, cw, ch = p["frames"], p["cw"], p["ch"]
        out = Image.new("P", (cols * cw, ch))
        out.putpalette([v for c in pal for v in c])
        data = [0] * (cols * cw * ch)
        for f, art in enumerate(p["cells"]):
            ox = f * cw
            for i, px in enumerate(art):
                x, y = i % p["uw"], i // p["uw"]
                data[y * cols * cw + ox + x] = px
        out.putdata(data)
        out.save(os.path.join(RES, p["res_name"] + ".png"))
        print(f"wrote res/{p['res_name']}.png {out.size}")

    # --- spawns -------------------------------------------------------------
    spawn_out = []
    for s in spawns_all:
        if s["id"] not in defs:
            continue
        p = plans[s["id"]]
        d = defs[s["id"]]
        x = s["x"] * 16
        if d["type"] == OBJ_BOSS_DOOR:
            # Static door: y is the tile row in the (already cropped) data.
            y = s["y"] * 16
        elif d["type"] in PLATFORM_TYPES:
            y = s["y"] * 16          # PC object.cpp:66: position.y = sy*TILESIZE
        else:
            y = s["y"] * 16
        if d["type"] in GRAVITY_TYPES:
            y, fell = drop_to_ground(grids[s["map"]], x, y, d["w"], d["h"])
            if fell:
                print(f"  !! spawn id={s['id']} map={s['map']} pos=({s['x']},{s['y']}): "
                      f"fell out of screen, omitted")
                continue
        def_idx = sorted(defs).index(s["id"])
        dx, dy, dm = s["dx"] & 0xFF, s["dy"] & 0xFF, s["dm"] & 0xFF
        if d["type"] in (OBJ_RAY_H, OBJ_DEATHRAY_H):
            dx = 1 if s["dir"] == 1 else 0     # ANIM_DIRECTION_RIGHT = 1 (rays carry their side here)
        spawn_out.append((def_idx, s["map"], x, y, dx, dy, dm))
        print(f"spawn def={def_idx} id={s['id']} type={d['type']} map={s['map']} "
              f"px=({x},{y}) bottom={y + d['h']}"
              + (f" dest=({dx},{dy}) destmap={dm}"
                 if d["type"] in TELEPORT_TYPES else ""))

    # --- {PFX}_objects.h/.c ------------------------------------------------
    ids_sorted = sorted(defs)
    h = f"""#ifndef {UP}_OBJECTS_H
#define {UP}_OBJECTS_H

#include <genesis.h>
#include "stage_data.h"

// Generated by tools/convert_objects.py from game_object_list.dat and
// map_object_data.dat. Types live in stage_data.h.

#define {UP}_OBJECT_DEF_COUNT {len(ids_sorted)}
#define {UP}_OBJECT_SPAWN_COUNT {len(spawn_out)}

extern const ObjectDef {PFX}_object_defs[{UP}_OBJECT_DEF_COUNT];
extern const ObjectSpawn {PFX}_object_spawns[{UP}_OBJECT_SPAWN_COUNT];

#endif
"""
    with open(os.path.join(SRC, f"{PFX}_objects.h"), "w") as f:
        f.write(h)

    lines = [f'#include "{PFX}_objects.h"\n', '#include "resources.h"\n\n']
    lines.append(f"const ObjectDef {PFX}_object_defs[{UP}_OBJECT_DEF_COUNT] = {{\n")
    for oid in ids_sorted:
        p = plans[oid]
        d = defs[oid]
        lines.append(
            f"    /* {ids_sorted.index(oid)} */ {{ &{p['res_name']}_sprite, "
            f"{d['type']}, {p['frames']}, {p['frame_t']}, "
            f"{d['w']}, {d['h']}, {d['speed']}, "
            f"{max(0, min(65535, d['timer']))}, "
            f"{max(0, min(65535, d['limit']))}, "
            f"{1 if d['anim_auto'] else 0}, {1 if d['anim_loop'] else 0} }},"
            f"  // {d['name']}\n")
    lines.append("};\n\n")
    lines.append(f"const ObjectSpawn {PFX}_object_spawns[{UP}_OBJECT_SPAWN_COUNT] = {{\n")
    for (di, mp, x, y, dx, dy, dm) in spawn_out:
        lines.append(f"    {{ {di}, {mp}, {x}, {y}, {dx}, {dy}, {dm} }},\n")
    lines.append("};\n")
    with open(os.path.join(SRC, f"{PFX}_objects.c"), "w") as f:
        f.writelines(lines)
    print(f"wrote src/{PFX}_objects.c/h ({len(ids_sorted)} defs, {len(spawn_out)} spawns)")

    # --- resources.res block -------------------------------------------------
    print("\n--- paste into res/resources.res ---")
    seen_res = set()
    for oid in ids_sorted:
        p = plans[oid]
        if p["res_name"] in seen_res:
            continue
        seen_res.add(p["res_name"])
        cw, ch = p["cw"], p["ch"]
        frames = p["frames"]
        if p["d"]["type"] == OBJ_BOSS_DOOR:
            # The boss door opens from its centre outward (PC draws it as map
            # tiles; object.c animates 16x16 slices instead). Declaring the
            # art as 16x16 cells makes rescomp cut the strip into one frame per
            # slice, so SPR_setFrame(i) picks slice i.
            cw, ch = 16, 16
            frames = p["fh"] // 16 if p["fh"] >= 16 else 3
            # one ANIMATION per slice: object.c picks slice i with
            # SPR_setAnim, which is how rescomp lays out a 1-frame-per-anim
            # door strip
            print(f'SPRITE {p["res_name"]}_sprite "{p["res_name"]}.png" '
                  f'{cw}p {ch}p FAST [' + "".join("[0]" for _ in range(frames)) + ']')
            continue
        t = ",".join(str(p["frame_t"]) for _ in range(p["frames"]))
        print(f'SPRITE {p["res_name"]}_sprite "{p["res_name"]}.png" '
              f'{cw}p {ch}p FAST [[{t}]]')
    return 0


if __name__ == "__main__":
    sys.exit(main())
