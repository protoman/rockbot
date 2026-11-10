#!/usr/bin/env python3
"""Offline check: a boss door makes its map column a scroll lock (like the PC).

BUG it reproduces (2026-10-30): "on the transition to the room with the door the
scroll does not respect the door being there as a scroll-lock" - CASTLE 5's boss
room is reached through a 16x64 door placed in a hole in the wall, and the view
scrolled over the closed door into the arena.

The PC counts a column as locked when EVERY row is a scroll-lock terrain OR is
covered by a boss-door object (`classMap::load_map`, classmap.cpp:173-190:
`obj_locked`). The port's scroll-lock tables are built by the converter from the
terrain only, so the door's column (FROZEN LAIR map 0 column 179: solid except
the 4 door rows) was missing and the camera could pass it.

This check recomputes the PC rule from the raw map + object data and compares it
with the generated `src/stage*_data.c` scroll-lock tables.

Usage: tools/check_door_scroll_lock.py     exit 1 when a door column is missing.
"""
import os
import re
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
DATA = os.path.join(ROOT, "data/games/Rockbot1")
SRC = os.path.join(ROOT, "src")

MAP_W, MAP_H, TILE = 200, 14, 16
MAP_REC = 15078
MAP_TILES_OFF = 78
OBJ_REC = 84
SPAWN_REC = 18
BOSS_DOOR = 30
# terrain ids that lock a column (PC classmap.cpp:186)
LOCK_TERRAIN = {1, 3, 4, 6, 10}     # SOLID, DOOR, SPIKE, ICE, SCROLL_LOCK


def read(path):
    with open(path, "rb") as fp:
        return fp.read()


def boss_doors(stage):
    """(map, x, y, w, h) of every boss door spawned in `stage`."""
    defs = read(os.path.join(DATA, "game_object_list.dat"))
    spawns = read(os.path.join(DATA, "map_object_data.dat"))
    doors = []
    for i in range(len(spawns) // SPAWN_REC):
        r = spawns[i * SPAWN_REC:(i + 1) * SPAWN_REC]
        (oid, _pad, sx, sy, _dir, _p2, _lx, _ly, _mdest,
         st, mp, _dl, _dm, _p3) = struct.unpack_from("<bbhhbBhhbbbbbb", r)
        if oid < 0 or st != stage:
            continue
        b = defs[oid * OBJ_REC:(oid + 1) * OBJ_REC]
        if struct.unpack_from("<b", b, 60)[0] != BOSS_DOOR:
            continue
        doors.append((mp, sx, sy,
                      struct.unpack_from("<b", b, 76)[0],
                      struct.unpack_from("<b", b, 77)[0]))
    return doors


def pc_locks(stage, doors):
    """PC wall_scroll_lock per map: terrain everywhere, door rows count as wall."""
    maps = read(os.path.join(DATA, "maps_v2.dat"))
    out = []
    for m in range(3):
        rec = maps[(stage * 3 + m) * MAP_REC:(stage * 3 + m + 1) * MAP_REC]

        def covered(x, y):
            # spawns store TILE positions (the PC multiplies by TILESIZE when it
            # builds the object): the door covers dw/16 columns, dh/16 rows
            for (dm, dx, dy, dw, dh) in doors:
                if dm != m:
                    continue
                if (dx <= x < dx + dw // TILE
                        and dy <= y < dy + dh // TILE):
                    return True
            return False

        cols = set()
        for x in range(MAP_W):
            ok = True
            for y in range(MAP_H):
                t = rec[MAP_TILES_OFF + (x * 15 + y) * 5] & 0xFF
                if t not in LOCK_TERRAIN and not covered(x, y):
                    ok = False
                    break
            if ok:
                cols.add(x)
        out.append(cols)
    return out


def port_locks(stage):
    text = open(os.path.join(SRC, "stage%d_data.c" % stage),
                encoding="utf-8", errors="replace").read()
    m = re.search(r"scroll_lock\[(\d+)\]\[(\d+)\] = \{(.*?)\n\};", text, re.S)
    if not m:
        return None
    counts = [int(x) for x in re.findall(
        r"\d+", re.search(r"scroll_lock_n\[\d+\] = \{([^}]*)\}", text).group(1))]
    rows = re.findall(r"\{([^}]*)\}", m.group(3))
    return [set(int(x) for x in re.findall(r"\d+", r)[:counts[i]])
            for i, r in enumerate(rows)]


def main():
    errors = 0
    for stage in range(14):
        doors = boss_doors(stage)
        if not doors:
            continue
        want = pc_locks(stage, doors)
        got = port_locks(stage)
        if got is None:
            continue
        for m in range(3):
            missing = sorted(want[m] - got[m])
            extra = sorted(got[m] - want[m])
            if missing:
                print("FAIL  stage %d map %d: door column(s) %s not a scroll "
                      "lock (door at %s)"
                      % (stage, m, missing,
                         [d for d in doors if d[0] == m]))
                errors += 1
            if extra:
                print("FAIL  stage %d map %d: locked column(s) %s the PC does "
                      "not lock" % (stage, m, extra))
                errors += 1
        if not errors:
            print("ok    stage %d: %s (door columns included)"
                  % (stage, [sorted(want[m] - set()) for m in range(3)]))
    print("\n%d door scroll-lock mismatch(es)" % errors)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
