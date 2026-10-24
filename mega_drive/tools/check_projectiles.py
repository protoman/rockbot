#!/usr/bin/env python3
"""Every projectile an enemy asks for must exist in that stage's table.

Report 2026-10-24: "wall shooter still shoots straight". The data asks for
projectile 34 (SMALL MISSILE EXACT, TRAJECTORY_TARGET_EXACT) but the converter
stopped at PROJ_MAX 23, so `eshot_fire()` clamped the id to 0 and the enemy
fired projectile 0 - NORMAL, linear - with no warning.

The generated tables are indexed by the PC projectile id (no per-stage
renumbering), so every referenced id must be < the table size and have a
sprite. Read-only.

Usage: tools/check_projectiles.py
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")

SHOT_ACTIONS = (5, 6, 7)        # SHOT_PROJECTILE_AHEAD / PLAYER_DIRECTION / INVERT


def main():
    problems = 0
    for path in sorted(glob.glob(os.path.join(SRC, "stage*_enemies.c"))):
        name = os.path.basename(path).replace("_enemies.c", "")
        text = open(path).read()
        head = open(path.replace(".c", ".h")).read()

        maximum = int(re.search(r"#define\s+\w+_ENEMY_PROJ_MAX\s+(\d+)",
                                head).group(1))
        have = {int(m.group(1))
                for m in re.finditer(r"\[(\d+)\]\s*=\s*\{\s*&proj_", text)}

        # Enemies referencing an id: the def's `shot` field + AI shot actions.
        users = {}
        for m in re.finditer(r"/\* \d+ \*/ \{\s*&(\w+)_sprite,\s*([^}]*)\}", text):
            values = [v.strip() for v in m.group(2).split(",")]
            if len(values) > 8:
                try:
                    users.setdefault(int(values[8]), set()).add(m.group(1))
                except ValueError:
                    pass
        for m in re.finditer(r"\{\d+,(\d+),\d+,\d+,(-?\d+)\}", text):
            action, param = int(m.group(1)), int(m.group(2))
            if action in SHOT_ACTIONS and param >= 0:
                users.setdefault(param, set()).add(f"ai{action}")

        for pid in sorted(users):
            if pid >= maximum:
                print(f"{name}: projectile {pid} requested by "
                      f"{', '.join(sorted(users[pid]))} but the table has "
                      f"{maximum} entries -> fired as projectile 0 (NORMAL)")
                problems += 1
            elif pid not in have:
                print(f"{name}: projectile {pid} requested by "
                      f"{', '.join(sorted(users[pid]))} has no sprite")
                problems += 1

    if problems:
        print(f"\ncheck_projectiles: {problems} projectile reference(s) fall "
              "back to the normal shot")
        return 1
    print("check_projectiles: OK (every referenced projectile exists)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
