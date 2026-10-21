#!/usr/bin/env python3
"""Audit the PC stage data against what the Mega Drive port converts/runs.

Reports, for every stage/map:
  * terrain values (and which ones have no runtime behaviour),
  * map objects (id, type, name) and whether the converter accepts the type,
  * enemy NPCs (id) and the AI actions their table uses vs. the actions the
    port implements,
  * second background layer / auto-scroll flags that the port ignores.

Read-only: prints the report, writes nothing. Usage: tools/audit_data.py
"""
import os
import re
import struct
import sys
from collections import Counter, defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
DATA = os.path.join(ROOT, "data/games/RockDroid1")
SRC = os.path.join(ROOT, "src")

MAPS = 3
MAP_STRUCT = 15078
MAP_TILES_OFF = 78
MAP_W, MAP_H_SRC = 200, 15
TILE_STRUCT = 5
STAGES = 14

OBJ_SZ = 84
MAP_OBJ_SZ = 18
NPC_SZ = 12
AI_SZ = 528

# PC e_TERRAIN (defines.h:285) names + port support
TERRAIN = {
    0: "UNBLOCKED", 1: "SOLID", 2: "STAIR", 3: "DOOR", 4: "SPIKE",
    5: "WATER", 6: "ICE", 7: "MOVE_LEFT", 8: "MOVE_RIGHT", 9: "QUICKSAND",
    10: "SCROLL_LOCK", 11: "CHECKPOINT", 12: "EASYBLOCK", 13: "HARDBLOCK",
}
TERRAIN_SUPPORT = {
    0: "ok", 1: "ok", 2: "ok", 3: "MISSING (passable, no door logic)",
    4: "ok", 5: "minimal (sink + slow fall)", 6: "minimal (low friction)",
    7: "ok", 8: "ok", 9: "MISSING (no quicksand)", 10: "ok",
    11: "checkpoint terrain? (only object 29 used)", 12: "removed (no difficulty)",
    13: "removed (no difficulty)",
}

# PC OBJECT_TYPE (defines.h:363)
OBJ_TYPE = {
    0: "MOVING_PLATFORM_UPDOWN", 1: "MOVING_PLATFORM_LEFTRIGHT",
    2: "DISAPPEARING_BLOCK", 3: "ENERGY_TANK", 4: "WEAPON_TANK",
    5: "ENERGY_PILL_BIG", 6: "WEAPON_PILL_BIG", 7: "ENERGY_PILL_SMALL",
    8: "WEAPON_PILL_SMALL", 9: "LIFE", 10: "FALL_PLATFORM",
    11: "FLY_PLATFORM", 12: "ITEM_FLY", 13: "ITEM_JUMP",
    14: "ACTIVE_DISAPPEARING_BLOCK", 15: "ARMOR_ARMS", 16: "ARMOR_BODY",
    17: "ARMOR_LEGS", 18: "RAY_HORIZONTAL", 19: "RAY_VERTICAL",
    20: "TRACK_PLATFORM", 21: "BOSS_TELEPORTER", 22: "DESTRUCTIBLE_WALL",
    23: "SPECIAL_TANK", 24: "DEATHRAY_VERTICAL", 25: "DEATHRAY_HORIZONTAL",
    26: "FINAL_BOSS_TELEPORTER", 27: "ACTIVE_OPENING_SLIM_PLATFORM",
    28: "DAMAGING_PLATFORM", 29: "CHECKPOINT", 30: "BOSS_DOOR",
    31: "PLATFORM_TELEPORTER", 32: "STAGE_BOSS_TELEPORTER",
}
OBJ_SUPPORT = {
    1: "ok (LR platform)", 2: "ok (visible/hidden cycle)",
    3: "ok (stored, menu)", 4: "ok (stored, menu)",
    5: "ok", 6: "ok", 7: "ok", 8: "ok", 9: "ok (no lives model)",
    10: "ok", 11: "ok", 14: "ok (activated cycle)",
    15: "ok (armor)", 16: "ok (armor)", 17: "ok (armor)",
    18: "ok (beam)", 19: "ok (beam)", 21: "ok (castle teleporter)",
    22: "ok (destructible wall)", 23: "ok (special tank)",
    24: "ok (beam)", 25: "ok (beam)", 26: "ok (castle teleporter)",
    29: "ok", 30: "ok (boss door)", 31: "ok (platform teleporter)", 32: "ok",
}

AI_ACTION = {
    0: "WALK", 1: "FLY", 2: "JUMP", 3: "WAIT_RANGE", 4: "SAVE_POINT",
    5: "SHOT_AHEAD", 6: "SHOT_PLAYER_DIR", 7: "SHOT_INVERT",
    8: "AIR_WALK", 9: "FALL_TO_GROUND", 10: "TELEPORT", 11: "DASH",
    12: "GRAB_WALL", 13: "SPAWN_NPC", 14: "CHANGE_MOVE_TYPE",
    15: "REPLACE_NPC", 16: "CIRCLE_PLAYER", 17: "CHANGE_MOVE_REVERSE",
    18: "JUMP_ATTACK_UP", 19: "JUMP_ATTACK_AHEAD", 20: "WAIT_RANDOM",
    21: "MORPH_INTO_NPC", 22: "PLAY_SFX", 23: "WALL_WALK",
    24: "WALL_WALK_SHOOT", 25: "SHOT_MULTIPLE", 26: "EXPLODE_ITSELF",
    27: "THROW_ITEM",
}
AI_SUPPORT = {
    0: "ok", 1: "ok", 2: "ok", 3: "ok", 5: "ok", 6: "ok (turns if move_speed)",
    4: "MISSING", 7: "MISSING", 8: "MISSING", 9: "MISSING",
    10: "ok (blink near player)", 11: "ok (dash)", 12: "ok (cling)",
    13: "ok (spawn)", 14: "ok (instant)", 15: "MISSING",
    16: "MISSING", 17: "MISSING", 18: "MISSING", 19: "MISSING", 20: "MISSING",
    21: "MISSING", 22: "MISSING", 23: "MISSING", 24: "MISSING", 25: "MISSING",
    26: "MISSING", 27: "MISSING",
}


def cstr(b):
    return b.split(b"\0")[0].decode("latin1")


def read(path):
    return open(os.path.join(DATA, path), "rb").read()


def terrain_hist():
    out = {}
    maps = read("maps_v2.dat")
    for s in range(STAGES):
        out[s] = []
        for m in range(MAPS):
            off = (s * MAPS + m) * MAP_STRUCT
            raw = maps[off:off + MAP_STRUCT][MAP_TILES_OFF:]
            hist = Counter()
            for x in range(MAP_W):
                for y in range(MAP_H_SRC):
                    v = raw[(x * MAP_H_SRC + y) * TILE_STRUCT]
                    if v in (12, 13):
                        v = 0        # converter drops difficulty blocks
                    hist[v] += 1
            out[s].append(hist)
    return out


def object_table():
    blob = read("game_object_list.dat")
    types = []
    for i in range(len(blob) // OBJ_SZ):
        types.append((cstr(blob[i * OBJ_SZ:i * OBJ_SZ + 30]),
                      struct.unpack_from("<b", blob, i * OBJ_SZ + 60)[0]))
    return types


def object_spawns():
    mo = read("map_object_data.dat")
    out = []
    for i in range(len(mo) // MAP_OBJ_SZ):
        r = mo[i * MAP_OBJ_SZ:(i + 1) * MAP_OBJ_SZ]
        oid, pad, sx, sy, d, p2, lx, ly, md, st, mp, dl, dm, p3 = \
            struct.unpack_from("<bbhhbBhhbbbbbb", r)
        if oid != -1:
            out.append((st, mp, oid, dl))
    return out


def npc_spawns():
    d = read("map_npc_data.dat")
    out = []
    for i in range(len(d) // NPC_SZ):
        r = d[i * NPC_SZ:(i + 1) * NPC_SZ]
        nid = struct.unpack_from("<b", r, 0)[0]
        st = r[7]
        if nid >= 0:
            out.append((st, nid))
    return out


def ai_table():
    d = read("game_ai_list.dat")
    out = []
    for i in range(len(d) // AI_SZ):
        r = d[i * AI_SZ:(i + 1) * AI_SZ]
        # Same layout/sanitisation as tools/convert_enemies.py: 5 ints per
        # state from offset 32 (chance, action, go_to, delay, param), and
        # editor garbage in unused slots is ignored (chance out of 0..100
        # or action out of 0..23). Only states with chance > 0 are used.
        acts = []
        for j in range(20):
            ch, act = struct.unpack_from("<2i", r, 32 + j * 20)
            if 0 <= ch <= 100 and ch > 0 and 0 <= act <= 23:
                acts.append(act)
        out.append(acts)
    return out


def layer1_flags():
    maps = read("maps_v2.dat")
    out = {}
    for s in range(STAGES):
        out[s] = []
        for m in range(MAPS):
            off = (s * MAPS + m) * MAP_STRUCT
            raw = maps[off:off + MAP_STRUCT]
            # backgrounds[0] at 0, backgrounds[1] at 36 (see convert_stage)
            name = cstr(raw[36:66])
            speed = struct.unpack_from("<b", raw, 68)[0] if len(raw) > 68 else 0
            auto = raw[33]      # bg0 auto_scroll mode (PC E_BG_SCROLL_MODE)
            out[s].append((name, speed, auto))
    return out


def main():
    terr = terrain_hist()
    objs = object_table()
    osp = object_spawns()
    nsp = npc_spawns()
    ais = ai_table()
    layer1 = layer1_flags()

    missing_terrain = defaultdict(Counter)
    missing_objects = defaultdict(Counter)
    missing_ai = defaultdict(Counter)
    second_bg = defaultdict(list)

    print("=== terrain ===")
    for s in range(STAGES):
        vals = set()
        for h in terr[s]:
            vals |= set(h)
        weird = {v: TERRAIN_SUPPORT.get(v, "?") for v in vals
                 if TERRAIN_SUPPORT.get(v) not in ("ok",)
                 and v not in (12, 13)}
        print(f"stage {s}: {sorted(vals)}")
        for v in weird:
            missing_terrain[s][v] = sum(h[v] for h in terr[s])

    print("\n=== objects ===")
    for s in range(STAGES):
        used = Counter()
        for (st, mp, oid, dl) in osp:
            if st != s:
                continue
            typ = objs[oid][1] if oid < len(objs) else -1
            used[(oid, typ)] += 1
            if not OBJ_SUPPORT.get(typ, "MISSING").startswith("ok"):
                missing_objects[s][(oid, typ, objs[oid][0])] += 1
        names = ", ".join(f"id{oid}:{OBJ_TYPE.get(typ, typ)}x{n}"
                          for (oid, typ), n in sorted(used.items()))
        print(f"stage {s}: {names or '(none)'}")

    print("\n=== enemies / AI ===")
    for s in range(STAGES):
        used = Counter(nid for (st, nid) in nsp if st == s)
        acts = Counter()
        for nid in used:
            if nid < len(ais):
                for a in set(ais[nid]):
                    acts[a] += used[nid]
        bad = {a: n for a, n in acts.items()
               if not AI_SUPPORT.get(a, "MISSING").startswith("ok")}
        print(f"stage {s}: npcs {sorted(used)}")
        if bad:
            print(f"   AI actions missing: " +
                  ", ".join(f"{a}({AI_ACTION.get(a, a)})x{n}"
                            for a, n in sorted(bad.items())))
            for a, n in bad.items():
                missing_ai[s][a] += n

    print("\n=== second background layer (port reads bg0 only) ===")
    for s in range(STAGES):
        for m, (name, speed, auto) in enumerate(layer1[s]):
            if name:
                second_bg[s].append((m, name, speed))
                print(f"stage {s} map {m}: bg1 {name} speed {speed}")
            if auto:
                second_bg[s].append((m, "auto_scroll", auto))
                print(f"stage {s} map {m}: bg0 auto_scroll mode {auto}")

    print("\n=== summary ===")
    for s in range(STAGES):
        bits = []
        if missing_terrain[s]:
            bits.append("terrain " + ", ".join(
                f"{TERRAIN.get(v, v)}({n})" for v, n in missing_terrain[s].items()))
        if missing_objects[s]:
            bits.append("objects " + ", ".join(
                f"{OBJ_TYPE.get(t, t)}x{n}" for (oid, t, name), n in
                missing_objects[s].items()))
        if missing_ai[s]:
            bits.append("AI " + ", ".join(
                f"{AI_ACTION.get(a, a)}({n})" for a, n in missing_ai[s].items()))
        if second_bg[s]:
            bits.append(f"{len(second_bg[s])} second-bg map(s)")
        print(f"stage {s}: " + ("; ".join(bits) if bits else "clean"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
