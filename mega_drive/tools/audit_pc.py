#!/usr/bin/env python3
"""Systematic PC-vs-port feature sweep.

The point of this tool is to stop discovering gaps by play-test only: for every
list the PC defines in its source (or its data), it reports what the port
implements, what it does not, and what that means for the player.

Checks
  1. player animation types      PC `enum ANIM_TYPE`  vs port `ANIM_*`
  2. player weapons              PC weapon table      vs port weapon_data[]
  3. ingame menu / HUD pieces    PC draw::weapon_menu_show_player / draw_hud
  4. config options              PC file_config       vs port Config
  5. player abilities            PC classplayer gates vs port player.c
  6. data-driven lists           reuses audit_data / check_terrain /
                                 check_projectiles / check_trajectories

Read-only. Exits 0 always: this is an inventory, not a gate (the individual
checks are the gates).

Usage: tools/audit_pc.py [--md]
"""
import glob
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")
PC = os.path.dirname(ROOT)          # parent: the PC source tree


def read(path):
    try:
        return open(path, errors="replace").read()
    except OSError:
        return ""


def section(title):
    print(f"\n== {title} " + "=" * max(0, 60 - len(title)))


# --- 1. animation types ---------------------------------------------------
# PC anim -> port ANIM_* (or None when the port folds it into another pose).
ANIM_MAP = {
    "ANIM_TYPE_STAND": "ANIM_STAND",
    "ANIM_TYPE_WALK": "ANIM_WALK",
    "ANIM_TYPE_JUMP": "ANIM_JUMP",
    "ANIM_TYPE_ATTACK": "ANIM_ATTACK",
    "ANIM_TYPE_TELEPORT": "ANIM_TELEPORT",
    "ANIM_TYPE_JUMP_ATTACK": "ANIM_JUMP_ATTACK",
    "ANIM_TYPE_WALK_ATTACK": "ANIM_WALK_ATTACK",
    "ANIM_TYPE_HIT": "ANIM_HIT",
    "ANIM_TYPE_STAIRS": "ANIM_STAIRS",
    "ANIM_TYPE_STAIRS_SEMI": None,       # frame 1 of ANIM_STAIRS
    "ANIM_TYPE_STAIRS_ATTACK": "ANIM_STAIRS_ATTACK",
    "ANIM_TYPE_STAIRS_MOVE": None,       # ANIM_STAIRS + flip toggle
    "ANIM_TYPE_SLIDE": "ANIM_SLIDE",
    "ANIM_TYPE_SHIELD": "ANIM_SHIELD",
    "ANIM_TYPE_SPECIAL_ATTACK": "ANIM_SPECIAL",
    "ANIM_TYPE_GOT_WEAPON": "ANIM_GOT_WEAPON",
    "ANIM_TYPE_THROW": None,
    "ANIM_TYPE_TURN": None,
    "ANIM_TYPE_MOVE_SEMI": None,
    "ANIM_TYPE_WALK_AIR": None,
    "ANIM_TYPE_INTRO": None,             # EnemyDef.intro_anim
    "ANIM_TYPE_VERTICAL_TURN": None,
    "ANIM_TYPE_ATTACK_THROW": None,
    "ANIM_TYPE_ATTACK_DIAGONAL_UP": None,
    "ANIM_TYPE_ATTACK_DIAGONAL_DOWN": None,
    "ANIM_TYPE_ATTACK_UP": None,
    "ANIM_TYPE_ATTACK_DOWN": None,
    "ANIM_TYPE_ATTACK_SPECIAL": None,
    "ANIM_TYPE_LOOK_UP": None,
    "ANIM_TYPE_GRAB_WALL": None,
    "ANIM_TYPE_HIT_SPECIAL": None,
    "ANIM_TYPE_HOLDING_HAND": None,
}

# Player poses the port has no art row for, but which change the GAME (not just
# the drawing): aiming up/down (8-way shots), looking up (jump direction), the
# throw pose. Diagonal shooting IS implemented with the normal attack art.
ANIM_IMPACT = {
    "ANIM_TYPE_ATTACK_UP": "shoot straight up",
    "ANIM_TYPE_ATTACK_DOWN": "shoot straight down",
    "ANIM_TYPE_ATTACK_DIAGONAL_UP": "shoot diagonal up (pos art)",
    "ANIM_TYPE_ATTACK_DIAGONAL_DOWN": "shoot diagonal down (pos art)",
    "ANIM_TYPE_LOOK_UP": "look up (camera/aim hint)",
    "ANIM_TYPE_GRAB_WALL": "grab a wall",
    "ANIM_TYPE_HOLDING_HAND": "co-op hand-holding",
    "ANIM_TYPE_THROW": "throw carried object",
}


def check_anims():
    pc = read(os.path.join(PC, "defines.h"))
    block = re.search(r"enum ANIM_TYPE \{(.*?)\};", pc, re.S)
    names = re.findall(r"\b(ANIM_TYPE_[A-Z0-9_]+)\b", block.group(1)) if block else []
    port = read(os.path.join(SRC, "player.h")) + read(os.path.join(SRC, "defines.h"))
    section("1. player animation types")
    missing = 0
    for n in names:
        if n.startswith(("ANIM_TYPE_PLACEHOLDER",)):
            continue
        target = ANIM_MAP.get(n, None)
        if target and re.search(r"#define\s+" + target + r"\b", port):
            continue
        missing += 1
        impact = ANIM_IMPACT.get(n)
        note = f"  -> {impact}" if impact else ""
        print(f"   absent: {n}{note}")
    print(f"   {len(names)} PC anim types, {missing} with no port pose")


# --- 2. weapons -----------------------------------------------------------
def check_weapons():
    section("2. player weapons")
    pc = read(os.path.join(PC, "file/fio_strings.cpp"))
    port = read(os.path.join(SRC, "weapon_data.c"))
    names = re.findall(r'"([^"]{3,})"', port)
    print(f"   port weapon_data: {len(names)} entries -> {', '.join(names[:12])}")
    used = set()
    for f in glob.glob(os.path.join(SRC, "stage*_enemies.c")):
        pass
    for f in glob.glob(os.path.join(SRC, "*.c")):
        used |= set(re.findall(r"player_weapon\(\)\s*==\s*(\d+)", read(f)))
    print(f"   weapon ids referenced by code: {sorted(used) or 'none'}")
    shot = read(os.path.join(SRC, "shot.c"))
    trajs = sorted(set(re.findall(r"TRAJ_[A-Z_]+", shot)))
    print(f"   player-shot trajectories implemented: {', '.join(trajs)}")


# --- 3. menu / HUD pieces -------------------------------------------------
def check_menu_hud():
    section("3. in-game menu and HUD")
    pc = read(os.path.join(PC, "graphic/draw.cpp"))
    port_menu = read(os.path.join(SRC, "menu.c"))
    port_hud = read(os.path.join(SRC, "hud.c"))
    items = [
        ("weapon grid + bars", "draw_weapon_bar", "draw_weapon_bar|draw_grid_bars"),
        ("lives 'x[n]'", r"items\.lifes", r"player_lives|buf\[2\]"),
        ("armor pieces icons", "ARMOR_PIECES_SPRITE", "armor_pieces|ARMOR"),
        ("tank counts", "energy_tanks", "tanks_hp"),
        ("tank USE", "use_tank", "player_use_tank"),
        ("HP gauge", "draw_hp", "set_bar|hud_update"),
        ("boss gauges", r"draw_boss|boss_hp", r"boss_parts|HUD_BAR_BOSS"),
    ]
    for label, pcpat, portpat in items:
        in_pc = re.search(pcpat, pc) is not None
        in_port = (re.search(portpat, port_menu) is not None or
                   re.search(portpat, port_hud) is not None)
        if in_pc and not in_port:
            print(f"   MISSING: {label}")
        elif in_pc:
            print(f"   ok:      {label}")


# --- 4. config ------------------------------------------------------------
def check_config():
    section("4. config / options")
    pc = read(os.path.join(PC, "file/v4/file_config_old.h"))
    port = read(os.path.join(SRC, "config.h"))
    fields = ["game_finished", "turbo_mode", "auto_charge_mode", "volume_sfx",
              "volume_music", "selected_language"]
    mapping = {"game_finished": "game_finished", "turbo_mode": "turbo",
               "auto_charge_mode": "autocharge", "volume_sfx": None,
               "volume_music": None, "selected_language": None}
    for f in fields:
        in_pc = re.search(r"\b" + f + r"\b", pc) is not None
        target = mapping[f]
        in_port = bool(target) and re.search(r"\b" + target + r"\b", port)
        if in_pc and not in_port:
            print(f"   MISSING: {f}")
        elif in_pc:
            print(f"   ok:      {f}")


# --- 5. player abilities --------------------------------------------------
def check_abilities():
    section("5. player abilities")
    port = read(os.path.join(SRC, "player.c"))
    abilities = [
        ("charge shot", "charge_frames"),
        ("semi/full charge levels", "CHARGE_FULL_FRAMES"),
        ("shield", "player_can_shield"),
        ("slide/dash", "ANIM_SLIDE"),
        ("diagonal shot", "player_can_diagonal"),
        ("double jump (LEGS)", "player_can_double_jump"),
        ("air dash (LEGS)", "player_can_air_dash"),
        ("shoryuken (LEGS)", "player_can_shoryuken"),
        ("armor abilities", "player_armor_ability"),
        ("ladder climb/attack", "ANIM_STAIRS_ATTACK"),
        ("water slowdown", "TERRAIN_WATER"),
        ("ice friction", "TERRAIN_ICE"),
        ("spike damage", "TERRAIN_SPIKE"),
        ("tank use", "player_use_tank"),
        ("1UP", "player_add_life|tanks_sp"),
        ("hit pushback", "hit_push"),
        ("invulnerability blink", "invuln"),
    ]
    for label, pat in abilities:
        print(f"   {'ok:     ' if re.search(pat, port) else 'MISSING:'} {label}")


# --- 6. data-driven lists -------------------------------------------------
def run(cmd):
    try:
        out = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True,
                             timeout=300)
        tail = [l for l in (out.stdout or "").splitlines() if l.strip()][-2:]
        return " / ".join(tail)
    except Exception as exc:            # noqa: BLE001
        return f"failed: {exc}"


def check_data():
    section("6. data-driven lists (existing checkers)")
    py = "/home/iuri/python3/bin/python3"
    for tool in ("audit_data.py", "check_terrain.py", "check_projectiles.py",
                 "check_trajectories.py", "check_vram.py", "audit_ai.py"):
        path = os.path.join(ROOT, "tools", tool)
        if os.path.exists(path):
            print(f"   {tool:22s} {run([py, path])}")


def main():
    print("PC feature sweep (port at " + ROOT + ", PC at " + PC + ")")
    check_anims()
    check_weapons()
    check_menu_hud()
    check_config()
    check_abilities()
    check_data()
    print("\nSee pc_gaps.md for the maintained list and fixes.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
