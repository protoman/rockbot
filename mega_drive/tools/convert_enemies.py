#!/usr/bin/env python3
"""Convert stage 0 enemies + the normal projectile for the Mega Drive port.

Reads (never invents values):
  build/games/RockDroid1/game_enemy_list_3_1_2.dat   6048B records (g++ probe)
  build/games/RockDroid1/map_npc_data.dat            12B records
  build/games/RockDroid1/game_projectile_list.dat    104B records
  images/sprites/enemies/*.png                       hand art (indexed PAL0)
  images/projectiles/projectile_normal.png           hand art (indexed PAL0)
  build/games/RockDroid1/maps_v2.dat                 terrain for the ground drop

Emits:
  res/enemy_<gfx>.png      SGDK SPRITE sheets: row0 = STAND, row1 = WALK
  res/shot.png             8x8 cell, 6x6 art (top-left), projectile_normal
  src/stage0_enemies.c/h   EnemyDef + EnemySpawn tables + shared PAL0 palette
  a SPRITE block to paste into res/resources.res (printed)

Simplifications (per user: precompute in converter, simple runtime AI):
  - difficulty: the port has no difficulty setting, so spawns are kept
    unless they are hard-only (difficulty_level >= 2; 0=easy, 1=normal,
    2=hard, defines.h:1314). PC rule classmap.cpp:541-546 (mode+level).
  - boss (is_boss) hp is overridden to BOSS_INITIAL_HP = 36 (PC
    classnpc.cpp:707 set_stage_boss() ignores file_npc.hp.total).
  - AI state tables come from game_ai_list.dat (528B records, loaded by
    game_mediator.cpp:262 — NOT game_ai_list_v3.dat, which the editor
    keeps and which ships with empty stage-0 AIs). AI index = enemy id
    (classnpc inherits artificial_inteligence; _number = npc id).
  - walkers are pre-dropped onto the ground (PC
    initialize_npc_position_to_ground, character.cpp:3213).
  - patrol limits precomputed from file_npc.walk_range: spawn +/- range.
  - art is copied 1:1 from the indexed hand art (images/, PAL0): the palette
    is taken from the art and pixel INDICES are preserved, never re-quantized
    (index 0 = transparent, 15 = font white stays text-only).

Usage: python3 tools/convert_enemies.py
"""
import os
import re
import struct
import sys
from collections import Counter
from PIL import Image

# PAL0 is the shared UI/enemy palette (single source of truth)
from pal_ui import PAL_UI_RGB, to_vdp

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
MAPS_PER_STAGE = 3
MAP_W = 200
MAP_H_SRC = 15          # file slot stride; slot 14 is unused padding
MAP_H = 14              # MD rows
MAP_STRUCT = 15078
MAP_TILES_OFF = 78
TILE_STRUCT = 5

NPC_SZ = 6048
STAGE_SZ = 438          # file_stage
STAGE_BOSS_OFF = 360    # file_boss { Sint8 id_npc; Sint8 id_weapon; char name[20]; }
STAGE_BOSS_ID = -1      # set in main() from the stage record
PROJ_SZ = 104
PROJ_MAX = 23          # 0..19 enemy shots; 20..22 armor charged shots
MAP_NPC_SZ = 12
AI_SZ = 528            # file_artificial_inteligence (name30+pad + 20x20 + 6x16)
AI_STATES = 20
AI_ACTIONS_OFF = 32    # name[30] + 2 pad
MAX_AI_REACTIONS = 6
AI_REACTIONS_OFF = AI_ACTIONS_OFF + AI_STATES * 20   # 432

# PC defines.h:438, applied by classnpc.cpp:707 set_stage_boss()
BOSS_INITIAL_HP = 36

# PC defines.h AI_ACTION_LIST + AI_ACTION_GOTO_LIST (goto 0 = chance roll)
AI_GOTO_CHANCE = 0

ANIM_STAND, ANIM_WALK, ANIM_JUMP, ANIM_ATTACK, ANIM_TELEPORT = 0, 1, 2, 3, 4
ANIM_TURN = 13       # PC ANIM_TYPE_TURN: the turn-around animation
ANIM_INTRO = 16     # bosses only (PC classMap::boss_show_intro_sprites)
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
        # The list is positional: a few late records (59/60 = UFO/FLYBOT) carry
        # 0xFF in the id byte but are still real entries — trust the index.
        assert b[0] == eid or b[0] == 0xFF, f"record {eid}: id byte {b[0]} != {eid}"
        stand, walk, attack, jump, teleport, intro = [], [], [], [], [], []
        turn = []
        for anim, out in ((ANIM_STAND, stand), (ANIM_WALK, walk),
                          (ANIM_ATTACK, attack), (ANIM_JUMP, jump),
                          (ANIM_TELEPORT, teleport), (ANIM_INTRO, intro),
                          (ANIM_TURN, turn)):
            for f in range(10):
                o = 78 + (anim * 10 + f) * 14
                used, dur, px = b[o], struct.unpack_from("<H", b, o + 2)[0], b[o + 4]
                rect = struct.unpack_from("<4h", b, o + 6)
                if used:
                    out.append((px, dur, rect))
        is_boss = int(eid == STAGE_BOSS_ID)
        hp = struct.unpack_from("<h", b, 64)[0]
        if is_boss:
            hp = BOSS_INITIAL_HP    # PC classnpc.cpp:707, file value unused
        defs[eid] = {
            "id": eid,
            "name": cstr(b, 3, 30),
            "gfx": cstr(b, 33, 30),
            "hp": hp,
            # e_shield_types (defines.h:603): which directions/poses are
            # immune to shots - see character::is_shielded (character.cpp:3357).
            # Several Helmut-type enemies are SHIELD_STAND (hide in the shell).
            "shield_type": b[5961],
            # weakness[i] = weapon i: (weapon_id, damage_multiplier, effect)
            # at 5964+3i; PC multiplies projectile damage by it
            # (classplayer.cpp:747). Index 0 is the buster. MILONGA BOT is
            # weak to the buster (x3).
            "dmg_mul": b[5965],
            "weak": [b[5965 + 3 * i] for i in range(9)],
            "speed": b[69],
            "proj0": b[1],
            "walk_range": struct.unpack_from("<h", b, 70)[0],
            "fw": struct.unpack_from("<b", b, 5958)[0],
            "fh": struct.unpack_from("<b", b, 5959)[0],
            "ia": struct.unpack_from("<b", b, 5962)[0],
            "fly": b[5963],
            "bgx": struct.unpack_from("<h", b, 6022)[0],
            "bgy": struct.unpack_from("<h", b, 6024)[0],
            "is_boss": is_boss,
            "is_sub_boss": b[6027],
            "respawn_delay": struct.unpack_from("<i", b, 6028)[0],
            "stand": stand,
            "walk": walk,
            "attack": attack,
            "jump": jump,
            "teleport": teleport,
            "intro": intro,
            "turn": turn,
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


def read_stage_boss_id():
    """PC stage_data.boss.id_npc: the ONE enemy that is the stage boss.

    The NPC record also carries an is_boss flag, but that is per enemy TYPE and
    several castle stages place other bosses as regular enemies; using it made
    the first such enemy (APE BOT in stage 13) the boss intro sprite and gave it
    the drop-in/HP bar. classmap.cpp:561: only boss.id_npc becomes the stage
    boss, other boss-flagged types are just set_is_boss(true).
    """
    path = os.path.join(DATA, "stages_v301.dat")
    blob = open(path, "rb").read()
    return struct.unpack_from("<b", blob, STAGE_SZ * STAGE_INDEX + STAGE_BOSS_OFF)[0]


def read_ai_states():
    """game_ai_list.dat per enemy id -> list of 20 sanitized states.

    The file carries uninitialized garbage in unused slots (editor
    leftovers); PC tolerates it because sane chance sums already reach 100
    before them. We sanitize the same way: out-of-range fields -> 0.
    """
    path = os.path.join(DATA, "game_ai_list.dat")
    blob = open(path, "rb").read()
    out = {}
    for eid in range(len(blob) // AI_SZ):
        b = blob[eid * AI_SZ:(eid + 1) * AI_SZ]
        states = []
        for i in range(AI_STATES):
            chance, action, go_to, delay, param = struct.unpack_from(
                "<5i", b, AI_ACTIONS_OFF + i * 20)
            if not (0 <= chance <= 100):
                chance = 0
            if not (0 <= action <= 23):
                action = 0
            if not (0 <= go_to <= 20):
                go_to = 0
            if not (0 <= delay <= 20000):
                delay = 0
            if not (-128 <= param <= 127):
                param = 0
            states.append((chance, action, go_to, delay, param))
        out[eid] = states
    return out


def read_ai_reactions():
    """game_ai_list.dat reactions -> 6 per enemy id.

    PC file_artificial_inteligence.reactions[MAX_AI_REACTIONS] (file_game_v4.h:
    884), 16 bytes each: action, go_to, go_to_delay, extra_parameter.
    action 0 = slot inactive. The 6 slots are indexed by AI_REACTION_* in the
    order artificial_inteligence.cpp:check_ai_reaction tests them:
    PLAYER_ON_RANGE, HIT, DEAD, PLAYER_SAME_Y, PLAYER_CLOSE, PLAYER_SAME_X.
    """
    path = os.path.join(DATA, "game_ai_list.dat")
    blob = open(path, "rb").read()
    out = {}
    for eid in range(len(blob) // AI_SZ):
        b = blob[eid * AI_SZ:(eid + 1) * AI_SZ]
        rows = []
        for i in range(MAX_AI_REACTIONS):
            action, go_to, delay, param = struct.unpack_from(
                "<4i", b, AI_REACTIONS_OFF + i * 16)
            if not (0 <= action <= 27):
                action = 0
            if not (0 <= go_to <= 20):
                go_to = 0
            if not (0 <= delay <= 20000):
                delay = 0
            if not (-128 <= param <= 127):
                param = 0
            rows.append((action, go_to, delay, param))
        out[eid] = rows
    return out


def read_terrain():
    """MD grid per map: the 14 game rows (tools/import_pc_data.py did the crop)."""
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
            grids.append([col[:MAP_H] for col in col_major])   # 14 game rows
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


def read_projectile_list():
    """game_projectile_list.dat (file_projectilev3, 104B): full table."""
    blob = open(os.path.join(DATA, "game_projectile_list.dat"), "rb").read()
    out = []
    for i in range(len(blob) // PROJ_SZ):
        b = blob[i * PROJ_SZ:(i + 1) * PROJ_SZ]
        w, h = struct.unpack_from("<2h", b, 60)
        out.append({
            "id": i, "name": cstr(b, 0, 30), "gfx": cstr(b, 30, 30),
            "w": w, "h": h, "traj": b[66], "max_shots": b[67],
            "speed": b[68], "dmg": b[69],
        })
    return out


def emit_explosion_sprite():
    """Enemy death puff: images/animations/explosion_32.png is a PC-indexed
    PAL0 sheet of 3 32x32 frames (PC classmap.cpp:2232 adds it at the dead
    NPC's position with an (-8,-8) adjust, 80ms per frame, 2 repeats).
    Copied 1:1 like the other hand art; indices 0 stay transparent."""
    path = os.path.join(IMAGES, "animations", "explosion_32.png")
    if not os.path.exists(path):
        return None
    im, _ = load_indexed(path)
    aw, ah = im.size
    frames = max(1, aw // 32)
    out = Image.new("P", (frames * 32, 32))
    out.putpalette(im.getpalette())
    data = [0] * (frames * 32 * 32)
    src = im.load()
    for y in range(min(32, ah)):
        for x in range(aw):
            data[y * frames * 32 + x] = src[x, y]
    out.putdata(data)
    out.save(os.path.join(RES, "explosion.png"))
    print(f"wrote res/explosion.png {out.size} ({frames} frames)")
    return frames


def emit_proj_sprite(rec, palette, save=True):
    """One SGDK SPRITE sheet for a projectile (PAL0). Handles horizontal or
    vertical frame strips (PC uses width/size.width for frames, but several
    projectiles ship as vertical strips, e.g. big_missile 28x196 = 7x28x28).
    Returns (name, cw, ch, frames, times) or None."""
    w, h = rec["w"], rec["h"]
    if w <= 0 or h <= 0 or w > 64 or h > 64 or not rec["gfx"]:
        return None
    path = os.path.join(IMAGES, "projectiles", rec["gfx"])
    if not os.path.exists(path):
        return None
    im, _ = load_indexed(path)
    aw, ah = im.size
    hf, vf = aw // w, ah // h
    if vf > 1 and hf <= 1:
        frames, vert = vf, True
    elif hf >= 1:
        frames, vert = hf, False
    else:
        frames, vert = 1, False
    cw, ch = ceil8(w), ceil8(h)
    out = Image.new("P", (frames * cw, ch))
    out.putpalette([v for c in palette for v in c])
    data = [0] * (frames * cw * ch)
    src = im.load()
    for f in range(frames):
        fx, fy = (0, f * h) if vert else (f * w, 0)
        for y in range(min(h, ah - fy)):
            for x in range(min(w, aw - fx)):
                data[y * frames * cw + f * cw + x] = src[fx + x, fy + y]
    out.putdata(data)
    name = "proj_%d" % rec["id"]
    if save:
        out.save(os.path.join(RES, name + ".png"))
    times = [6] * frames if frames > 1 else [0]
    return name, cw, ch, frames, times


# ---------------------------------------------------------------- sheets

def load_indexed(path):
    """Indexed hand art (images/): palette + per-pixel INDICES. The art is
    already quantized to PAL0 by tools/import_pc_art.py, so the converter
    copies indices 1:1 instead of rebuilding/quantizing colours (which
    destroyed the verified output)."""
    im = Image.open(path)
    assert im.mode == "P", f"{path}: not indexed (run tools/import_pc_art.py)"
    pal = [tuple(im.getpalette()[i * 3:i * 3 + 3]) for i in range(16)]
    return im, pal


# ---------------------------------------------------------------- main

def update_res_lines(lines):
    """Rewrite the SPRITE lines of res/resources.res in place.

    The enemy sheets gained the jump and teleport animation rows (BUG-011), so
    every SPRITE line must list five animations - otherwise rescomp slices the
    sheet rows into the wrong animations. Only lines that already exist are
    replaced; add_stage.py still inserts whole blocks for new stages."""
    path = os.path.join(ROOT, "res", "resources.res")
    text = open(path).read()
    n = 0
    for sym, line in lines.items():
        pat = re.compile(r"^SPRITE " + re.escape(sym) + r"\b.*$", re.M)
        text, k = pat.subn(lambda m: line, text)
        n += k
    open(path, "w").write(text)
    print(f"resources.res: {n} SPRITE line(s) updated")


def main():
    blob = open(os.path.join(DATA, "game_enemy_list_3_1_2.dat"), "rb").read()
    global STAGE_BOSS_ID
    STAGE_BOSS_ID = read_stage_boss_id()
    print(f"stage {STAGE_INDEX} boss npc id {STAGE_BOSS_ID}")
    spawns_all = read_spawns()
    ai_states = read_ai_states()
    ai_react = read_ai_reactions()

    used_ids = []
    for s in spawns_all:
        if s["id"] < 0 or s["id"] * NPC_SZ >= len(blob):
            continue
        if s["diffL"] >= 2:                   # hard-only spawn: skip
            print(f"skip spawn id={s['id']} map={s['map']} pos=({s['x']},{s['y']}): difficulty_level {s['diffL']} (hard-only)")
            continue
        used_ids.append(s["id"])

    # AI actions SPAWN_NPC / REPLACE_NPC / MORPH carry the spawned NPC id in
    # extra_parameter; that enemy may not be placed on the map, so add its def
    # (art + stats + AI table) to this stage's bundle.
    extra_ids = set()
    for eid in set(used_ids):
        for (ch, act, g, delay, param) in ai_states.get(eid, []):
            if ch > 0 and act in (13, 15, 21) and 0 <= param < len(blob) // NPC_SZ:
                extra_ids.add(param)
        # REACTION_DEAD can REPLACE_NPC/MORPH to a form that is never placed on
        # the map (DESTRIN TANK -> DESTRIN SHIP), so its target joins the
        # bundle too.
        for (act, g, delay, param) in ai_react.get(eid, []):
            if act in (13, 15, 21) and 0 <= param < len(blob) // NPC_SZ:
                extra_ids.add(param)
    used_ids = sorted(set(used_ids) | extra_ids)
    defs = read_enemy_defs(blob, set(used_ids))
    grids = read_terrain()

    # --- sheets -------------------------------------------------------------
    res_names = {}          # id -> resources.res SPRITE name
    sheet_imgs = {}         # id -> indexed sheet (filled after palette)
    plans = {}              # id -> dict with cells/times/meta
    art_pal = None          # PAL0 as the hand art carries it
    for eid, d in sorted(defs.items()):
        path = os.path.join(IMAGES, "sprites/enemies", d["gfx"])
        sheet, spal = load_indexed(path)
        if art_pal is None:
            art_pal = spal
        elif spal != art_pal:
            print(f"  !! {path}: palette differs from the first enemy sheet")
        fw, fh = d["fw"], d["fh"]
        cw, ch = ceil8(fw), ceil8(fh)
        if cw > 128 or ch > 128:
            print(f"  !! skip enemy {eid} {d['name']} gfx={d['gfx']}: "
                  f"cell {cw}x{ch} > 64 (needs a multi-part sprite)")
            continue

        def frames_of(anim):
            return [(px, dur) for (px, dur, _rect) in anim]

        stand = frames_of(d["stand"]) or frames_of(d["walk"])
        walk = frames_of(d["walk"]) or stand
        attack = frames_of(d["attack"])   # may be empty (no attack anim)
        # The jump/teleport rows cost sprite VRAM in every sheet that carries
        # them, so only the enemies whose AI really uses those actions get one
        # (the extra row would otherwise push crowded stages past the 504-tile
        # sprite pool). The row index is stored in the def: the order is
        # stand, walk, attack, [jump], [teleport].
        acts = {a[1] for a in ai_states.get(eid, [])}
        # Only bosses/subs get the extra rows: a stage boss is alone on screen,
        # while a normal enemy sheet with 5 rows pushed crowded rooms past the
        # sprite VRAM pool (invisible player + garbage frame pointers).
        boss_or_sub = bool(d["is_boss"] or d["is_sub_boss"])
        needs_jump = boss_or_sub and 2 in acts
        needs_tele = boss_or_sub and 10 in acts
        jump = frames_of(d["jump"]) if needs_jump else []
        teleport = frames_of(d["teleport"]) if needs_tele else []
        jump_anim = 3 if needs_jump else 255
        teleport_anim = (4 if needs_jump else 3) if needs_tele else 255
        # Boss landing intro (PC boss_show_intro_sprites): only bosses/subs,
        # and only when the file has the INTRO frames.
        intro = frames_of(d["intro"]) if (d["is_boss"] or d["is_sub_boss"]) else []
        # ANIM_TYPE_TURN: the flying shielder turns by rotating its shield, so
        # the row matters for the AI turn options (WALK TURN / FLY TURN).
        turn = frames_of(d["turn"])
        nrows = 3 + (1 if needs_jump else 0) + (1 if needs_tele else 0)
        intro_anim = nrows if intro else 255
        turn_anim = (nrows + (1 if intro else 0)) if turn else 255
        turn_ms = sum(du for _, du in turn)
        intro_ms = sum(du for _, du in intro)
        # all stand frames / all walk frames, times in 1/60s
        def times(fr):
            return [max(1, round(dur * 6 / 100)) for _, dur in fr]

        # union collision rect over the frames we ship
        rects = [r for (_, _, r) in d["stand"] + d["walk"]]
        bl = min(r[0] for r in rects)
        bt = min(r[1] for r in rects)
        br = max(r[0] + r[2] for r in rects)
        bb = max(r[1] + r[3] for r in rects)

        cols = max(len(stand), len(walk), len(attack), len(jump),
                   len(teleport), len(intro), 1)
        src = sheet.load()
        cells = []
        rows = [stand, walk, attack]
        if needs_jump:
            rows.append(jump or stand)
        if needs_tele:
            rows.append(teleport or stand)
        if intro:
            rows.append(intro)
        if turn:
            rows.append(turn)
        for row_frames in rows:
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
            "stand": stand, "walk": walk, "attack": attack,
            "jump": jump, "teleport": teleport,
            "jump_anim": jump_anim, "teleport_anim": teleport_anim,
            "intro_anim": intro_anim, "intro_ms": intro_ms,
            "turn_anim": turn_anim, "turn_ms": turn_ms,
            "turn_t": times(turn) if turn else [1],
            "nrows": nrows + (1 if intro else 0) + (1 if turn else 0),
            "stand_t": times(stand), "walk_t": times(walk),
            "attack_t": times(attack),
            "jump_t": times(jump) if jump else [1],
            "teleport_t": times(teleport) if teleport else [1],
            "attack_ms": sum(du for _, du in attack),
            "cells": cells,
            "box": (bl, bt, br - bl, bb - bt),
            "res_name": "enemy_" + os.path.splitext(d["gfx"])[0],
        }
        res_names[eid] = plans[eid]["res_name"]
        print(f"id={eid} {d['name']} gfx={d['gfx']} hp={d['hp']} mul={d['dmg_mul']} "
              f"spd={d['speed']} fly={d['fly']} ia={d['ia']} cell={cw}x{ch} "
              f"box=(bl,bt,w,h)={plans[eid]['box']} "
              f"stand={len(stand)}f walk={len(walk)}f bg=({d['bgx']},{d['bgy']})")

    # --- shot ---------------------------------------------------------------
    shot_img, _ = load_indexed(os.path.join(IMAGES, "projectiles/projectile_normal.png"))
    semi_path = os.path.join(IMAGES, "projectiles/charged_semi.png")
    semi_img = load_indexed(semi_path)[0] if os.path.exists(semi_path) else shot_img
    # full charge (id 10 "Charged") — art colors must join the shared pool
    full_img, _ = load_indexed(os.path.join(IMAGES, "projectiles/charged_shot.png"))
    proj = open(os.path.join(DATA, "game_projectile_list.dat"), "rb").read()
    p0 = proj[0:PROJ_SZ]
    shot_speed = p0[68]
    shot_dmg = p0[69]
    print(f"projectile 0: '{cstr(p0,0,30)}' speed={shot_speed} damage={shot_dmg} "
          f"max_shots={p0[67]} size={struct.unpack_from('<hh', p0, 60)}")

    # --- shared palette -----------------------------------------------------
    # PAL0 is taken from the hand art itself (all enemy/projectile sheets are
    # imported to the same table by tools/import_pc_art.py). It must equal the
    # tools/pal_ui.py table (HUD balls use pal_ui.png) -- warn otherwise.
    palette = list(art_pal or PAL_UI_RGB)
    if palette != list(PAL_UI_RGB):
        print("  !! art PAL0 differs from tools/pal_ui.py -- HUD balls will "
              "not match; regenerate pal_ui_data.py")

    print(f"PAL0 (from art): {len(palette)} entries")

    # --- write sprite sheets ------------------------------------------------
    # A sheet is shared by every stage that spawns the art (the castle stages
    # spawn the eight bosses as plain enemies). Those stages want 3 rows while
    # the boss stage wants 5 (jump/teleport/intro), so with a naive write the
    # LAST stage converted (13) left 3-row sheets behind while stages 1..8's
    # defs still pointed at rows 3/4: SPR_setAnim(3) on a 3-animation sheet is
    # out of range and SGDK then reads a garbage animation pointer - that was
    # the APE BOT line-1111 crash (it fires when he jumps). Keep the TALLEST
    # version of each art; the shorter plan's def never references the extra
    # rows (its anim indices are 255) and its SPRITE line is dropped with it.
    os.makedirs(RES, exist_ok=True)
    kept = {}
    for eid, p in sorted(plans.items()):
        cols, cw, ch = p["cols"], p["cw"], p["ch"]
        rows = len(p["cells"])          # stand, walk, attack, jump, teleport
        path = os.path.join(RES, p["res_name"] + ".png")
        if os.path.exists(path):
            try:
                old_rows = Image.open(path).height // ch
            except Exception:
                old_rows = 0
            if old_rows > rows:
                kept[p["res_name"]] = old_rows
                print(f"keep res/{p['res_name']}.png ({old_rows} rows, this "
                      f"stage needs {rows})")
                continue
        out = Image.new("P", (cols * cw, rows * ch))
        out.putpalette([v for c in palette for v in c])
        data = [0] * (cols * cw * rows * ch)
        for r, row in enumerate(p["cells"]):
            for c, art in enumerate(row):
                ox, oy = c * cw, r * ch
                for i, px in enumerate(art):
                    x, y = i % p["fw"], i // p["fw"]
                    data[(oy + y) * cols * cw + ox + x] = px
        out.putdata(data)
        out.save(path)
        print(f"wrote res/{p['res_name']}.png {out.size}")

    # --- projectile sprites (data-driven, PAL0 hand art) -------------------
    proj_recs = read_projectile_list()
    proj_spr = {}
    for rec in proj_recs[:PROJ_MAX]:
        info = emit_proj_sprite(rec, palette, save=(STAGE_INDEX == 0))
        if info is None:
            if rec["gfx"]:
                print(f"  !! proj[{rec['id']}] skipped (gfx={rec['gfx']} "
                      f"size={rec['w']}x{rec['h']})")
            continue
        proj_spr[rec["id"]] = info
        print(f"wrote res/{info[0]}.png ({info[3]}f {info[1]}x{info[2]}) "
              f"traj={rec['traj']} spd={rec['speed']} dmg={rec['dmg']} "
              f"max_shots={rec['max_shots']}")

    # shot: 6x6 art in an 8x8 cell (SGDK cells like multiples of 8)
    out = Image.new("P", (8, 8))
    out.putpalette([v for c in palette for v in c])
    data = [0] * 64
    src = shot_img.load()
    for y in range(shot_img.size[1]):
        for x in range(shot_img.size[0]):
            data[y * 8 + x] = src[x, y]
    out.putdata(data)
    out.save(os.path.join(RES, "shot.png"))
    print("wrote res/shot.png 8x8")

    # enemy projectile id 11 "Semi-Charged": 2 frames of 16x16 (32x16 art)
    sw, sh = semi_img.size
    nsemi = max(1, sw // 16)
    out = Image.new("P", (nsemi * 16, 16))
    out.putpalette([v for c in palette for v in c])
    data = [0] * (nsemi * 16 * 16)
    src = semi_img.load()
    for y in range(min(sh, 16)):
        for x in range(min(sw, nsemi * 16)):
            data[y * nsemi * 16 + x] = src[x, y]
    out.putdata(data)
    out.save(os.path.join(RES, "enemy_shot_semi.png"))
    print(f"wrote res/enemy_shot_semi.png {out.size}")

    # Fully-charged projectile (game_projectile_list id 10 "Charged",
    # player_list_v3_1_1 player 0 full_charged_projectile_id): art is
    # 62x25 = 2 frames of 31x25, declared size 31x30; frames are laid out
    # width/size.width (projectilelib.cpp:66) at 100ms each
    # (PROJECTILE_DEFAULT_ANIMATION_TIME, defines.h:644).
    fw, fh = 31, 30
    nfull = full_img.size[0] // fw                  # 2 frames
    cw, ch = ceil8(fw), ceil8(fh)                   # 32x32 SGDK cell
    out = Image.new("P", (nfull * cw, ch))
    out.putpalette([v for c in palette for v in c])
    data = [0] * (nfull * cw * ch)
    src = full_img.load()
    for f in range(nfull):
        for y in range(min(full_img.size[1], ch)):
            for x in range(fw):
                data[y * nfull * cw + f * cw + x] = src[f * fw + x, y]
    out.putdata(data)
    out.save(os.path.join(RES, "shot_charge_full.png"))
    print(f"wrote res/shot_charge_full.png {out.size} ({nfull} frames of {fw}x{fh} in {cw}x{ch} cells)")

    # --- spawns -------------------------------------------------------------
    spawn_out = []
    for s in spawns_all:
        if s["id"] not in plans:      # def skipped (e.g. oversized sprite)
            continue
        d = defs[s["id"]]
        p = plans[s["id"]]
        box_l, box_t, box_w, box_h = p["box"]
        x = s["x"] * 16 + d["bgx"]
        y = s["y"] * 16 + d["bgy"]
        if not d["fly"]:
            y, ok = drop_to_ground(grids[s["map"]], x + box_l, y, box_t, box_h)
            if not ok:
                print(f"  !! spawn id={s['id']} map={s['map']} pos=({s['x']},{s['y']}): no ground found")
        x_min = max(0, x - d["walk_range"])
        x_max = min(MAP_W * 16 - box_w, x + d["walk_range"])
        def_idx = sorted(plans).index(s["id"])
        spawn_out.append((def_idx, s["map"], x, y, x_min, x_max, s["dir"]))
        print(f"spawn def={def_idx} id={s['id']} map={s['map']} px=({x},{y}) "
              f"range=[{x_min},{x_max}] dir={s['dir']}")

    # complete projectile table (sane ids 0..PROJ_MAX-1); no per-stage filtering
    proj_info = {rec["id"]: rec for rec in proj_recs[:PROJ_MAX]
                 if rec["id"] in proj_spr}
    for pid in sorted(proj_info):
        r = proj_info[pid]
        print(f"proj[{pid}] '{r['name']}' traj={r['traj']} spd={r['speed']} "
              f"dmg={r['dmg']} max={r['max_shots']} size={r['w']}x{r['h']}")

    # --- {PFX}_enemies.h/.c -----------------------------------------------
    ids_sorted = sorted(plans)
    h = f"""#ifndef {UP}_ENEMIES_H
#define {UP}_ENEMIES_H

#include <genesis.h>
#include "stage_data.h"

// Generated by tools/convert_enemies.py from game_enemy_list_3_1_2.dat and
// map_npc_data.dat. Types live in stage_data.h.

#define {UP}_ENEMY_DEF_COUNT {len(ids_sorted)}
#define {UP}_ENEMY_SPAWN_COUNT {len(spawn_out)}
#define {UP}_ENEMY_PROJ_MAX {PROJ_MAX}

extern const EnemyDef {PFX}_enemy_defs[{UP}_ENEMY_DEF_COUNT];
extern const EnemySpawn {PFX}_enemy_spawns[{UP}_ENEMY_SPAWN_COUNT];
extern const EnemyAi {PFX}_enemy_ai[{UP}_ENEMY_DEF_COUNT][20];
extern const EnemyAi {PFX}_enemy_react[{UP}_ENEMY_DEF_COUNT][6];
extern const EnemyProj {PFX}_enemy_proj[{UP}_ENEMY_PROJ_MAX];

#endif
"""
    with open(os.path.join(SRC, f"{PFX}_enemies.h"), "w") as f:
        f.write(h)

    def_idx_name = {eid: i for i, eid in enumerate(ids_sorted)}
    lines = [f'#include "{PFX}_enemies.h"\n', '#include "resources.h"\n\n']
    lines.append(f"const EnemyDef {PFX}_enemy_defs[{UP}_ENEMY_DEF_COUNT] = {{\n")
    for eid in ids_sorted:
        p = plans[eid]
        d = defs[eid]
        bl, bt, bw, bh = p["box"]
        shot0 = d["proj0"] if d["proj0"] in proj_info else 0
        lines.append(
            f"    /* {def_idx_name[eid]} */ {{ &{p['res_name']}_sprite, "
            f"{eid}, {d['hp']}, {d['dmg_mul']}, {d['speed']}, {d['fly']}, {d['is_boss']}, "
            f"{d['is_sub_boss']}, {max(0, d['respawn_delay'])}, "
            f"{shot0}, {d['walk_range']}, "
            f"{bl}, {bt}, {bw}, {bh}, "
            f"{len(p['stand'])}, {len(p['walk'])}, {len(p['attack'])}, "
            f"{p['jump_anim']}, {p['teleport_anim']}, "
            f"{p['intro_anim']}, {p['intro_ms']}, "
            f"{p['attack_ms']}, {d['shield_type']}, "
            f"{{ {', '.join(str(x & 0xFF) for x in d['weak'])} }}, "
            f"{p['turn_anim']}, {p['turn_ms']} }},"
            f"  // {d['name']}\n")
    lines.append("};\n\n")
    lines.append(f"const EnemySpawn {PFX}_enemy_spawns[{UP}_ENEMY_SPAWN_COUNT] = {{\n")
    for (di, mp, x, y, xm, xx, dr) in spawn_out:
        lines.append(f"    {{ {di}, {mp}, {x}, {y}, {xm}, {xx}, {dr} }},\n")
    lines.append("};\n\n")
    lines.append(f"const EnemyAi {PFX}_enemy_ai[{UP}_ENEMY_DEF_COUNT][20] = {{\n")
    for eid in ids_sorted:
        states = ai_states.get(eid, [(0, 0, 0, 0, 0)] * 20)
        rows = ", ".join(
            f"{{{ch},{act},{g},{delay},{param}}}"
            for (ch, act, g, delay, param) in states)
        lines.append(f"    {{ {rows} }},  // {defs[eid]['name']}\n")
    lines.append("};\n\n")
    lines.append(f"const EnemyAi {PFX}_enemy_react[{UP}_ENEMY_DEF_COUNT][6] = {{\n")
    for eid in ids_sorted:
        rows = ", ".join(
            f"{{0,{act},{g},{delay},{param}}}"
            for (act, g, delay, param) in ai_react.get(eid, [(0, 0, 0, 0)] * 6))
        lines.append(f"    {{ {rows} }},  // {defs[eid]['name']}\n")
    lines.append("};\n\n")
    lines.append(f"const EnemyProj {PFX}_enemy_proj[{UP}_ENEMY_PROJ_MAX] = {{\n")
    for pid in sorted(proj_info):
        r = proj_info[pid]
        lines.append(f"    [{pid}] = {{ &proj_{pid}_sprite, {r['speed']}, "
                     f"{r['dmg']}, {r['w']}, {r['h']}, {r['traj']}, "
                     f"{r['max_shots']} }},\n")
    lines.append("};\n")
    with open(os.path.join(SRC, f"{PFX}_enemies.c"), "w") as f:
        f.writelines(lines)
    active = {eid: [r for r in ai_react.get(eid, []) if r[0] > 0]
              for eid in ids_sorted}
    active = {k: v for k, v in active.items() if v}
    print(f"wrote src/{PFX}_enemies.c/h ({len(ids_sorted)} defs, "
          f"{len(spawn_out)} spawns, {len(active)} with reactions)")
    for eid, rows in active.items():
        print(f"  reactions {defs[eid]['name']}: {rows}")

    # --- resources.res block ------------------------------------------------
    print("\n--- res/resources.res (rewritten in place) ---")
    res_lines = {}
    for eid in ids_sorted:
        p = plans[eid]
        st = ",".join(str(t) for t in p["stand_t"])
        wk = ",".join(str(t) for t in p["walk_t"])
        ak = ",".join(str(t) for t in p["attack_t"]) or "0"
        anims = [f"[{st}]", f"[{wk}]", f"[{ak}]"]
        if p["jump_anim"] != 255:
            anims.append("[" + ",".join(str(t) for t in p["jump_t"]) + "]")
        if p["teleport_anim"] != 255:
            anims.append("[" + ",".join(str(t) for t in p["teleport_t"]) + "]")
        if p["intro_anim"] != 255:
            anims.append("[" + ",".join(str(t) for t in
                                          [max(1, round(du * 6 / 100))
                                           for _, du, _r in p["d"]["intro"]]) + "]")
        if p["turn_anim"] != 255:
            anims.append("[" + ",".join(str(t) for t in p["turn_t"]) + "]")
        line = (f'SPRITE {p["res_name"]}_sprite "{p["res_name"]}.png" '
                f'{p["cw"]}p {p["ch"]}p FAST [' + "".join(anims) + ']')
        print(line)
        # Rewrite the existing line in place: the sheet has 3/4/5 rows, so the
        # animation lists must match or rescomp mis-slices the frames. When a
        # taller sheet was kept from an earlier stage, its line must stay too.
        if p["res_name"] in kept:
            continue
        res_lines[line.split()[1]] = line
    if STAGE_INDEX == 0:
        emit_explosion_sprite()
        for pid in sorted(proj_spr):
            name, cw, ch, frames, times = proj_spr[pid]
            tl = ",".join(str(t) for t in times)
            print(f'SPRITE {name}_sprite "{name}.png" {cw}p {ch}p FAST [[{tl}]]')
    print('SPRITE shot_sprite "shot.png" 8p 8p FAST [[1]]')
    print('SPRITE enemy_shot_semi_sprite "enemy_shot_semi.png" 16p 16p FAST [[6,6]]')
    print('SPRITE shot_charge_full_sprite "shot_charge_full.png" 32p 32p FAST [[6,6]]')
    # Enemy death puff (PC classmap.cpp:2232 add_animation(explosion32, 80ms, 2))
    print('SPRITE explosion_sprite "explosion.png" 32p 32p FAST [[5]]')
    print(f"shot_speed={shot_speed} shot_dmg={shot_dmg}")
    update_res_lines(res_lines)
    return 0


if __name__ == "__main__":
    sys.exit(main())
