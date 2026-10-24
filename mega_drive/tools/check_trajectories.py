#!/usr/bin/env python3
"""Every projectile trajectory the enemy data uses must be implemented.

Report 2026-10-24: the wall shooter still fired straight ahead. Its projectile
(SMALL MISSILE EXACT, PC trajectory 15 = TRAJECTORY_TARGET_EXACT) had no define
in the port, so `eshot.c` fell through to LINEAR and the shot ignored the
player - with no warning anywhere.

Only projectiles an enemy actually asks for are checked (the per-stage tables
carry the whole list, most of it unused). Read-only.

Usage: tools/check_trajectories.py
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")

SHOT_ACTIONS = (5, 6, 7)        # AHEAD / PLAYER_DIRECTION / INVERT

# Trajectories knowingly not implemented yet: (name, why).
KNOWN_FALLBACK = {
    "LASER": "bounces off walls (PC projectilelib.cpp:791); flies straight",
    "CHAIN": "follows its owner like a flail (projectilelib.cpp:643); flies straight",
    "BOMB_RAIN": "spawns child bombs across the screen (projectilelib.cpp:927)",
    "LARGE_BEAM": "3-frame growing beam (projectilelib.cpp:162)",
    "BOUNCING": "gravity arc + bounce (projectilelib.cpp:876); flies straight",
}

PC_NAMES = [
    "LINEAR", "ARC", "SIN", "CHAIN", "QUAKE", "BOMB", "FOLLOW", "PROGRESSIVE",
    "FREEZE", "DIAGONAL_UP", "DIAGONAL_DOWN", "CENTERED", "ZIGZAG",
    "TARGET_DIRECTION", "ARC_TO_TARGET", "TARGET_EXACT", "FALL_BOMB", "LASER",
    "PUSH_BACK", "ARC_SMALL", "RING", "LIGHTING", "SPIRAL", "BOUNCING",
    "INVERSE_LINEAR", "DOUBLE_LINEAR", "DOUBLE_DIAGONAL", "BOMB_RAIN",
    "LARGE_BEAM", "PULL", "SLASH",
]


def referenced_ids(text):
    ids = set()
    for m in re.finditer(r"/\* \d+ \*/ \{\s*&(\w+)_sprite,\s*([^}]*)\}", text):
        values = [v.strip() for v in m.group(2).split(",")]
        if len(values) > 8:
            try:
                ids.add(int(values[8]))
            except ValueError:
                pass
    for m in re.finditer(r"\{\d+,(\d+),\d+,\d+,(-?\d+)\}", text):
        action, param = int(m.group(1)), int(m.group(2))
        if action in SHOT_ACTIONS and param >= 0:
            ids.add(param)
    return ids


def main():
    header = open(os.path.join(SRC, "stage_data.h")).read()
    defines = {name: int(value) for name, value in
               re.findall(r"#define\s+(TRAJ_\w+)\s+(\d+)", header)}
    eshot = open(os.path.join(SRC, "eshot.c")).read()
    handled = {defines[name] for name in defines
               if re.search(r"\b" + name + r"\b", eshot)}
    handled.add(0)                      # the default branch is LINEAR

    used = {}
    for path in sorted(glob.glob(os.path.join(SRC, "stage*_enemies.c"))):
        stage = os.path.basename(path).replace("_enemies.c", "")
        text = open(path).read()
        table = {}
        for line in text.splitlines():
            m = re.match(r"\s*\[\s*(\d+)\s*\]\s*=\s*\{\s*&\w+,\s*(\d+),\s*"
                         r"\d+,\s*\d+,\s*\d+,\s*(\d+),", line)
            if m:
                table[int(m.group(1))] = int(m.group(3))
        for pid in referenced_ids(text):
            if pid in table:
                used.setdefault(table[pid], set()).add(stage)

    bad = 0
    for traj in sorted(used):
        name = PC_NAMES[traj] if traj < len(PC_NAMES) else f"traj {traj}"
        stages = ", ".join(sorted(used[traj]))
        if traj in handled:
            print(f"ok        {traj:2d} {name:16s} ({stages})")
        elif name in KNOWN_FALLBACK:
            print(f"known     {traj:2d} {name:16s} ({stages}) - "
                  f"{KNOWN_FALLBACK[name]}")
        else:
            print(f"MISSING   {traj:2d} {name:16s} ({stages})")
            bad += 1

    if bad:
        print(f"\ncheck_trajectories: {bad} trajectory value(s) used by the "
              "data fall back to LINEAR")
        return 1
    print(f"\ncheck_trajectories: OK ({len(used)} used trajectory value(s))")
    return 0


if __name__ == "__main__":
    sys.exit(main())
