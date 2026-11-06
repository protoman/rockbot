#!/usr/bin/env python3
"""Scene ops used by the data vs the ops src/scene.c implements.

The PC scene format declares 11 ops; `src/scene.c` runs 8 of them. This walks
every scene in the game data, prints the op usage and fails when a scene the
port actually plays uses an op it cannot run (the converter refuses to emit
such a scene, this is the independent check).

Usage: tools/check_scenes.py
"""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import convert_scenes as cs   # noqa: E402  (needs the path above)


PLAYED = {0: "opening (SCENE_OPENING_UNBEATEN)",
          2: "ending player 1", 3: "ending player 2",
          4: "ending player 3", 5: "ending player 4"}


def main():
    scenes = cs.read_scenes()
    bad = 0
    for i, (name, ops) in enumerate(scenes):
        used = []
        for (typ, _seek, _delay, _rt, _rv, _bg) in ops:
            op = cs.OP_NAMES[typ] if typ < len(cs.OP_NAMES) else f"op{typ}"
            if op not in used:
                used.append(op)
        state = "played" if i in PLAYED else "unused"
        print(f"scene {i} [{state}] {name or '(unnamed)'}: {', '.join(used) or 'empty'}")
        if i in PLAYED:
            for op in used:
                if op not in cs.PLAYABLE:
                    print(f"   MISSING: {op} is used by a scene the port plays")
                    bad += 1
    if bad:
        print(f"\ncheck_scenes: {bad} unsupported op(s) in played scenes")
        return 1
    print("\ncheck_scenes: OK (every op a played scene uses is implemented)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
