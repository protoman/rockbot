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
  - spawn px: x = sx*16, y = max(0, sy-1)*16 (top PC row cropped).
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
    MD_LEVELS, ceil8, load_indexed, read_terrain, solid,
)

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
DATA = os.path.join(ROCKBOT, "build/games/RockDroid1")
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
OBJ_ENERGY_PILL_SMALL = 7
OBJ_LIFE = 9
OBJ_FALL_PLATFORM = 10
OBJ_FLY_PLATFORM = 11
OBJ_CHECKPOINT = 29
OBJ_STAGE_BOSS_TELEPORTER = 32

GRAVITY_TYPES = (OBJ_ENERGY_PILL_SMALL, OBJ_LIFE, OBJ_CHECKPOINT)


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
            "w": struct.unpack_from("<b", b, 76)[0],
            "h": struct.unpack_from("<b", b, 77)[0],
            "fd": struct.unpack_from("<h", b, 78)[0],
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


def drop_to_ground(grid, x, y, w, h):
    """PC object gravity at load: up to h steps of GRAV=4px.

    Destination bottom corners must both be passable; when either hits
    SOLID the object stops (may rest with a 1..GRAV-1 px hover, as PC).
    Returns (y, fell_out).
    """
    for _ in range(h):
        dest = y + GRAV
        row = (dest + h - 2) >> 4
        if row > RES_H:               # PC: falling out of screen is allowed
            y = dest
            continue
        c1 = (x + 2) >> 4
        c2 = (x + w - 2) >> 4
        if solid(grid, c1, row) or solid(grid, c2, row):
            break
        y = dest
        if y > RES_H:
            return y, True
    return y, y > RES_H


def main():
    blob = open(os.path.join(DATA, "game_object_list.dat"), "rb").read()
    spawns_all = read_spawns()

    used_ids = []
    for s in spawns_all:
        if s["id"] < 0 or s["id"] * OBJ_SZ >= len(blob):
            continue
        typ = struct.unpack_from("<b", blob, s["id"] * OBJ_SZ + 60)[0]
        if typ not in GRAVITY_TYPES and \
           typ not in (OBJ_FALL_PLATFORM, OBJ_FLY_PLATFORM, OBJ_STAGE_BOSS_TELEPORTER):
            print(f"skip spawn id={s['id']} map={s['map']} pos=({s['x']},{s['y']}): "
                  f"unsupported type {typ}")
            continue
        used_ids.append(s["id"])

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
        cw, ch = ceil8(fw), ceil8(fh)
        assert cw <= 32 and ch <= 64, f"{d['gfx']}: cell {cw}x{ch} too large"
        pw, ph = sheet.size
        assert pw % fw == 0 and ph == fh, \
            f"{d['gfx']}: png {pw}x{ph} is not a {fh}-tall strip of {fw}px frames"
        frames = pw // fw
        frame_t = max(1, round(d["fd"] * 6 / 100))
        src = sheet.load()
        cells = []
        for f in range(frames):
            art = []
            for y in range(fh):
                for x in range(fw):
                    art.append(src[f * fw + x, y])
            cells.append(art)
        plans[oid] = {
            "d": d, "cw": cw, "ch": ch, "fw": fw, "fh": fh,
            "frames": frames, "frame_t": frame_t, "cells": cells,
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
                x, y = i % p["fw"], i // p["fw"]
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
        if d["type"] in (OBJ_FALL_PLATFORM, OBJ_FLY_PLATFORM):
            y = s["y"] * 16          # PC object.cpp:66: position.y = sy*TILESIZE
        else:
            y = max(0, s["y"] - 1) * 16
        if d["type"] in GRAVITY_TYPES:
            y, fell = drop_to_ground(grids[s["map"]], x, y, d["w"], d["h"])
            if fell:
                print(f"  !! spawn id={s['id']} map={s['map']} pos=({s['x']},{s['y']}): "
                      f"fell out of screen, omitted")
                continue
        def_idx = sorted(defs).index(s["id"])
        dx, dy, dm = s["dx"] & 0xFF, s["dy"] & 0xFF, s["dm"] & 0xFF
        spawn_out.append((def_idx, s["map"], x, y, dx, dy, dm))
        print(f"spawn def={def_idx} id={s['id']} type={d['type']} map={s['map']} "
              f"px=({x},{y}) bottom={y + d['h']}"
              + (f" dest=({dx},{dy}) destmap={dm}" if d["type"] == OBJ_STAGE_BOSS_TELEPORTER else ""))

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
            f"{d['w']}, {d['h']}, {d['speed']} }},  // {d['name']}\n")
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
    for oid in ids_sorted:
        p = plans[oid]
        t = ",".join(str(p["frame_t"]) for _ in range(p["frames"]))
        print(f'SPRITE {p["res_name"]}_sprite "{p["res_name"]}.png" '
              f'{p["cw"]}p {p["ch"]}p FAST [[{t}]]')
    return 0


if __name__ == "__main__":
    sys.exit(main())
