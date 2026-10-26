#!/usr/bin/env python3
"""Which AI action OPTIONS the data uses, and which the port implements.

Report 2026-10-24: "do a thorough review of all AI to find cases like this one"
(the ANGRY BUNNY that TOP HAT spawns kept jumping straight up). The PC packs the
behaviour of an action into its `extra_parameter` (the option); the port keeps
that parameter in the same slot of the generated chain, so an unhandled option
silently falls back to a default - exactly how TO_ROOF became a plain hop
(APE BOT) and, later, AHEAD became a jump in place.

This prints every (action, option) pair the converted data uses, marks the ones
a constant name for in src/enemy.c, and lists the used options the port never
mentions. Read-only.

Usage: tools/audit_ai.py [--verbose]
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")

ACTIONS = {
    0: "WALK", 1: "FLY", 2: "JUMP", 3: "WAIT_RANGE", 4: "SAVE_POINT",
    5: "SHOT_AHEAD", 6: "SHOT_PLAYER", 7: "SHOT_INVERT", 8: "AIR_WALK",
    9: "FALL", 10: "TELEPORT", 11: "DASH", 12: "GRAB_WALL", 13: "SPAWN_NPC",
    14: "CHANGE_MOVE", 15: "REPLACE_NPC", 16: "CIRCLE_PLAYER",
    17: "CHANGE_MOVE_REV", 18: "JUMP_ATTACK_UP", 19: "JUMP_ATTACK_AHEAD",
    20: "WAIT_RANDOM", 21: "MORPH", 22: "PLAY_SFX", 23: "WALL_WALK",
    24: "WALL_WALK_SHOOT", 25: "SHOT_MULTIPLE", 26: "EXPLODE_ITSELF",
    27: "THROW_ITEM",
}

# Actions whose parameter is an OPTION index (the rest carry an id/range).
OPTION_ACTIONS = {
    0: "AI_WALK_", 1: "AI_FLY_", 2: "AI_JUMP_", 8: "AI_AIR_WALK_",
    10: "AI_TELEPORT_", 25: "AI_SHOT_MULTI_",
}

# PC enum option counts (defines.h), used to sanity-check the parameter.
OPTION_COUNT = {0: 8, 1: 26, 2: 11, 3: 0, 8: 3, 10: 7, 25: 7}


def port_constants():
    """Every AI_* constant name mentioned in enemy.c."""
    text = open(os.path.join(SRC, "enemy.c")).read()
    return set(re.findall(r"\bAI_[A-Z0-9_]+\b", text))


def main():
    verbose = "--verbose" in sys.argv
    known = port_constants()

    used = {}          # (action, param) -> set(stage)
    for path in sorted(glob.glob(os.path.join(SRC, "stage*_enemies.c"))):
        stage = os.path.basename(path).replace("_enemies.c", "")
        text = open(path).read()
        # 20-state chains and the 6 reaction slots share the {c,a,g,d,p} shape.
        for m in re.finditer(r"\{(\d+),(\d+),(\d+),(\d+),(-?\d+)\}", text):
            chance, action, go_to, delay, param = (int(g) for g in m.groups())
            if action == 0 and chance == 0 and param == 0 and delay in (0, 500):
                continue        # padding slots
            used.setdefault((action, param), set()).add(stage)

    # Option VALUES the port really handles: every `#define AI_<ACTION>_<NAME> n`
    # in enemy.c, plus 0 (the default/"ahead" branch). An option the data uses
    # that is in neither set falls into a default branch and silently does the
    # wrong thing (FLY DOWN flew horizontally; JUMP TO_ROOF was a plain hop).
    enemy_c = open(os.path.join(SRC, "enemy.c")).read()
    gaps = []

    missing = []
    for (action, param), stages in sorted(used.items()):
        if action not in ACTIONS:
            print(f"?? action {action} (param {param}) in {sorted(stages)}")
            missing.append((action, param))
            continue
        if action not in OPTION_ACTIONS:
            if verbose:
                print(f"ok   {ACTIONS[action]:18s} param {param:4d} "
                      f"({', '.join(sorted(stages))}) [id/range]")
            continue
        count = OPTION_COUNT.get(action, 0)
        if not 0 <= param < count:
            continue            # id/range parameter, not an option
        prefix = OPTION_ACTIONS[action]
        names = [k for k in known if k.startswith(prefix)]
        handled = {int(n) for n in re.findall(
            rf"#define {prefix}\w+\s+(\d+)", enemy_c)}
        if handled and param != 0 and param not in handled:
            gaps.append((ACTIONS[action], param, sorted(stages)))
        # The port names options without the ACTION_ part; accept any constant
        # that ends with the option index name is impossible, so just report
        # which option VALUES the data uses per action and let the human check.
        print(f"     {ACTIONS[action]:18s} option {param:2d} used by "
              f"{', '.join(sorted(stages))}")

    for name, param, stages in gaps:
        print(f"GAP  {name} option {param}: used by {', '.join(stages)} but "
              f"enemy.c has no case (falls into the default branch)")
    if missing:
        print(f"\naudit_ai: {len(missing)} unknown action(s)")
        return 1
    if gaps:
        print(f"\naudit_ai: {len(gaps)} option(s) used by the data are not handled")
        return 1
    print("\naudit_ai: option usage listed (compare with enemy.c's handling)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
