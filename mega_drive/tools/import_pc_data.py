#!/usr/bin/env python3
"""One-time import of the PC game data for the port + the 240 -> 224p conversion.

The Mega Drive build shows 14 tile rows (224p) while the PC maps are 15 rows
(240px). That crop used to happen inside tools/convert_*.py, which meant the
editor and the converters disagreed about what row 0 was: the editor edited
15-row data, the converters silently dropped the first row, and anything the
editor placed there never reached the ROM.

This tool moves that conversion into the DATA instead: it copies the PC game
folder into data/games/Rockbot1 and rewrites the copy so the game rows are
slots 0..13, which is exactly what the editor edits and what the converters
read (no crop at build time any more).

  data/games/Rockbot1/maps_v2.dat        tiles: row 0 dropped, rows 0..13 =
                                           old rows 1..14, slot 14 cleared
  data/games/Rockbot1/stages_v301.dat    link rows: 0 stays 0, else -1
  data/games/Rockbot1/map_npc_data.dat   spawn y: -1
  data/games/Rockbot1/map_object_data.dat spawn y: -1 (except platforms,
                                           which the PC already places by row)

The file structs keep their 15-slot layout (they come from the PC headers the
editor compiles against), so slot 14 stays as unused padding.

Run once:  python3 tools/import_pc_data.py [--force]
"""
import sys as _sys
# DISARMED (BUG-055, owner 2026-10-31): images/ and data/ are hand-edited now and
# must never be overwritten from the PC version again. The file stays only as
# documentation of how the first import was made.
if __name__ == "__main__":
    _sys.exit("%s is disarmed (BUG-055): the PC import is over, images/ and data/ are owned by hand edits" % __file__)

import os
import shutil
import struct
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
PCROOT = os.path.dirname(ROOT)

PC_DATA = os.path.join(PCROOT, "build/games/Rockbot1")
DATA = os.path.join(ROOT, "data/games/Rockbot1")
MARKER = os.path.join(ROOT, "data/.pc_import_done")

MAPS_PER_STAGE = 3
FS_MAX_STAGES = 20
MAP_STRUCT = 15078
TILES_OFF = 78
MAP_W = 200
MAP_H_SRC = 15
MAP_H = 14              # game rows after the conversion
TILE_STRUCT = 5

STAGE_STRUCT = 438
LINKS_OFF = 60
LINKS_N = 30
LINK_SZ = 10

NPC_SZ = 12
OBJ_SZ = 18

# PC OBJECT_TYPE values the converter treats as platforms (their Y is already
# the tile row in PC data); the boss door is excluded on purpose, the converter
# applied the crop to it.
PLATFORM_TYPES = (1, 2, 14, 10, 11, 22, 30)
BOSS_DOOR = 30

EMPTY_TILE = struct.pack("bbbbb", 0, -1, -1, -1, -1)


def read_links(stage_n):
    with open(os.path.join(DATA, "stages_v301.dat"), "rb") as f:
        f.seek(STAGE_STRUCT * stage_n + LINKS_OFF)
        raw = f.read(LINKS_N * LINK_SZ)
    out = []
    for i in range(LINKS_N):
        b = raw[i * LINK_SZ:(i + 1) * LINK_SZ]
        frm, to, ox, oy, dx, dy, size, bidir, door, typ = struct.unpack(
            "bbbbbbbbbb", b)
        if frm < 0 or to < 0:
            continue
        out.append((i, frm & 0xFF, to & 0xFF, ox & 0xFF, oy & 0xFF,
                    dx & 0xFF, dy & 0xFF, size & 0xFF, bidir, door, typ))
    return out


def convert_stage_links(stage_n):
    """Link rows: 0 stays 0, everything else moves up one (the crop)."""
    path = os.path.join(DATA, "stages_v301.dat")
    with open(path, "r+b") as f:
        base = STAGE_STRUCT * stage_n + LINKS_OFF
        f.seek(base)
        raw = bytearray(f.read(LINKS_N * LINK_SZ))
        moved = 0
        for i in range(LINKS_N):
            o = i * LINK_SZ
            frm, to = struct.unpack("bb", raw[o:o + 2])
            if frm < 0 or to < 0:
                continue
            oy, dy = raw[o + 3], raw[o + 5]
            noy = 0 if oy == 0 else oy - 1
            ndy = 0 if dy == 0 else dy - 1
            if noy != oy or ndy != dy:
                moved += 1
            raw[o + 3] = noy & 0xFF
            raw[o + 5] = ndy & 0xFF
        f.seek(base)
        f.write(raw)
    return moved


def convert_maps(stage_n):
    """Drop the top row of the three maps of this stage.

    If a link touches a map at row 0 the top row is first copied onto row 1 so
    its tiles stay reachable (same rule the converter used).
    """
    links = read_links(stage_n)
    path = os.path.join(DATA, "maps_v2.dat")
    stats = []
    with open(path, "r+b") as f:
        for m in range(MAPS_PER_STAGE):
            pos = MAP_STRUCT * (stage_n * MAPS_PER_STAGE + m)
            f.seek(pos)
            raw = bytearray(f.read(MAP_STRUCT))
            touches = any((frm == m and oy == 0) or (to == m and dy == 0)
                          for (_i, frm, to, _ox, oy, _dx, dy, *_rest) in links)
            tiles = raw[TILES_OFF:TILES_OFF + MAP_W * MAP_H_SRC * TILE_STRUCT]
            out = bytearray(MAP_W * MAP_H_SRC * TILE_STRUCT)
            for x in range(MAP_W):
                col = [tiles[(x * MAP_H_SRC + y) * TILE_STRUCT:
                             (x * MAP_H_SRC + y) * TILE_STRUCT + TILE_STRUCT]
                       for y in range(MAP_H_SRC)]
                if touches:
                    col[1] = col[0]
                for y in range(MAP_H):
                    out[(x * MAP_H_SRC + y) * TILE_STRUCT:
                        (x * MAP_H_SRC + y) * TILE_STRUCT + TILE_STRUCT] = col[y + 1]
                # slot 14 is padding: keep it empty
                out[(x * MAP_H_SRC + MAP_H) * TILE_STRUCT:
                    (x * MAP_H_SRC + MAP_H) * TILE_STRUCT + TILE_STRUCT] = EMPTY_TILE
            raw[TILES_OFF:TILES_OFF + MAP_W * MAP_H_SRC * TILE_STRUCT] = out
            f.seek(pos)
            f.write(raw)
            stats.append((m, touches))
    return stats


def convert_spawns(filename, sz, y_off, keep_types=()):
    path = os.path.join(DATA, filename)
    with open(path, "rb") as f:
        raw = bytearray(f.read())
    moved = 0
    for i in range(len(raw) // sz):
        o = i * sz
        oid = struct.unpack_from("<b", raw, o)[0]
        if oid < 0:
            continue
        y = struct.unpack_from("<h", raw, o + y_off)[0]
        if y <= 0:
            continue
        if keep_types:
            typ = struct.unpack_from("<b", raw, o + 60)[0] \
                if sz == OBJ_SZ else None
            if typ is not None and typ in keep_types and typ != BOSS_DOOR:
                continue
        struct.pack_into("<h", raw, o + y_off, y - 1)
        moved += 1
    with open(path, "wb") as f:
        f.write(raw)
    return moved


def object_types():
    """object id -> PC type, from game_object_list.dat."""
    path = os.path.join(DATA, "game_object_list.dat")
    with open(path, "rb") as f:
        blob = f.read()
    out = {}
    for i in range(len(blob) // 84):
        out[i] = struct.unpack_from("<b", blob, i * 84 + 60)[0]
    return out


def convert_object_spawns():
    types = object_types()
    path = os.path.join(DATA, "map_object_data.dat")
    with open(path, "rb") as f:
        raw = bytearray(f.read())
    moved = 0
    for i in range(len(raw) // OBJ_SZ):
        o = i * OBJ_SZ
        oid = struct.unpack_from("<b", raw, o)[0]
        if oid < 0:
            continue
        y = struct.unpack_from("<h", raw, o + 4)[0]
        if y <= 0:
            continue
        typ = types.get(oid, -1)
        # platforms (except the door) are placed by row on the PC already
        if typ in PLATFORM_TYPES and typ != BOSS_DOOR:
            continue
        struct.pack_into("<h", raw, o + 4, y - 1)
        moved += 1
    with open(path, "wb") as f:
        f.write(raw)
    return moved


def main():
    force = "--force" in sys.argv
    if os.path.exists(MARKER) and not force:
        sys.exit(f"{MARKER} exists: the data is already imported. "
                 f"Re-run with --force (this overwrites data/games).")
    if not os.path.isdir(PC_DATA):
        sys.exit(f"PC data not found: {PC_DATA}")

    if os.path.isdir(DATA) and os.listdir(DATA):
        backup = os.path.join(ROOT, "data",
                              "backup_pre_import_%s"
                              % time.strftime("%Y%m%d_%H%M%S"))
        shutil.copytree(DATA, backup)
        print(f"backup of the previous data: {backup}")

    if os.path.isdir(DATA):
        shutil.rmtree(DATA)
    shutil.copytree(PC_DATA, DATA)
    print(f"copied {PC_DATA} -> {DATA}")

    for stage in range(FS_MAX_STAGES):
        # maps first: the "link touches row 0" rule must see the ORIGINAL rows
        maps = convert_maps(stage)
        moved_links = convert_stage_links(stage)
        if stage < 14:
            print(f"stage {stage}: links moved {moved_links}, "
                  f"maps {[('dup' if t else 'crop') for _m, t in maps]}")

    npcs = convert_spawns("map_npc_data.dat", NPC_SZ, 4)
    objs = convert_object_spawns()
    print(f"spawns moved: {npcs} enemies, {objs} objects")

    with open(MARKER, "w") as f:
        f.write("PC data imported and converted to 14 game rows "
                "by tools/import_pc_data.py\n")
    print(f"done ({MARKER})")


if __name__ == "__main__":
    main()
