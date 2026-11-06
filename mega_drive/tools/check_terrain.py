#!/usr/bin/env python3
"""Check that every terrain value the game data uses is implemented.

The port's physics only reacts to terrain ids it names in src/. A map that
introduces a value the engine does not handle behaves as UNBLOCKED, which is
the quietest failure mode there is (the player walks through a wall or never
sinks into quicksand). RockDroid2 is not converted yet, so it is audited from
the PC build tree: whatever its maps use has to be covered by the time its
stages are added.

Read-only. Exit code 1 on a terrain id with no implementation.
Usage: tools/check_terrain.py
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
SRC = os.path.join(ROOT, "src")

GAMES = [("RockDroid1", os.path.join(ROOT, "data/games/RockDroid1")),
         ("RockDroid2", os.path.join(ROCKBOT, "build/games/RockDroid2"))]

MAPS, MAP_STRUCT, MAP_TILES_OFF = 3, 15078, 78
MAP_W, MAP_H, TILE_STRUCT = 200, 15, 5

# PC e_TERRAIN (defines.h:285) and the port's behaviour. "code" is the token
# that must appear in src/ for the id to count as handled; a terrain the data
# cannot produce is listed as unused instead of implemented.
TERRAIN = {
    0: "UNBLOCKED", 1: "SOLID", 2: "STAIR", 3: "DOOR", 4: "SPIKE",
    5: "WATER", 6: "ICE", 7: "MOVE_LEFT", 8: "MOVE_RIGHT", 9: "QUICKSAND",
    10: "SCROLL_LOCK", 11: "CHECKPOINT", 12: "EASYBLOCK", 13: "HARDBLOCK",
}
# ids the port deliberately folds into something else (difficulty blocks are
# dropped by tools/convert_stage.py; CHECKPOINT terrain is a map object)
FOLDED = {12: "converter drops it (no difficulty)", 
          13: "converter drops it (no difficulty)",
          11: "unused by any map (checkpoints are object 37)"}


def used_terrains(path):
    maps = open(os.path.join(path, "maps_v2.dat"), "rb").read()
    stages = os.path.getsize(os.path.join(path, "stages_v301.dat")) // 438
    out = {}
    for s in range(stages):
        for m in range(MAPS):
            off = (s * MAPS + m) * MAP_STRUCT + MAP_TILES_OFF
            raw = maps[off:off + MAP_STRUCT - MAP_TILES_OFF]
            for i in range(MAP_W * MAP_H):
                v = raw[i * TILE_STRUCT]
                out.setdefault(v, set()).add(s)
    return stages, out


def defined_terrain():
    src = open(os.path.join(SRC, "stage_data.h")).read()
    return {int(v): name for name, v in
            re.findall(r"#define TERRAIN_(\w+)\s+(\d+)", src)}


def referenced(token):
    """TRUE if `src/*.c` uses the terrain id beyond its definition."""
    pat = re.compile(rf"\bTERRAIN_{token}\b")
    hits = 0
    for name in os.listdir(SRC):
        if not name.endswith(".c"):
            continue
        hits += len(pat.findall(open(os.path.join(SRC, name),
                                     errors="replace").read()))
    return hits


def main():
    defined = defined_terrain()
    missing = 0
    for game, path in GAMES:
        if not os.path.isdir(path):
            print(f"{game}: not present, skipped")
            continue
        stages, used = used_terrains(path)
        print(f"{game}: {stages} stages, terrain ids "
              f"{sorted(used)}")
        for v in sorted(used):
            name = TERRAIN.get(v, f"unknown {v}")
            if v in FOLDED:
                continue
            if defined.get(v) != name:
                print(f"  MISSING: id {v} ({name}) used in stage(s) "
                      f"{sorted(used[v])} - not in src/stage_data.h")
                missing += 1
                continue
            n = referenced(name)
            if n < 1:       # no code path mentions it
                print(f"  MISSING: id {v} ({name}) used in stage(s) "
                      f"{sorted(used[v])} - defined but never handled")
                missing += 1
        print("  handled: " + ", ".join(
            f"{v}:{TERRAIN.get(v, v)}" for v in sorted(used)
            if v in defined))
    if missing:
        print(f"check_terrain: {missing} terrain id(s) unhandled")
        return 1
    print("check_terrain: every terrain id in the data is handled")
    return 0


if __name__ == "__main__":
    sys.exit(main())
