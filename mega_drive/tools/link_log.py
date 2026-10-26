#!/usr/bin/env python3
"""Verdict for a MD_LINK_DEBUG run (room-change alignment + arrival on the ladder).

Usage: tools/link_log.py save.sram|logfile      (see tools/trans_log.py for how
the log reaches ~/.local/share/blastem/rom/save.sram)

For every room change it prints:
  - the decision: link, camera before/after, the screen X the player had and has;
  - whether the screens line up (screen X before == after, +-2 px);
  - the arrival (LKA): placed on a ladder? terrain under the feet (2 = stair);
  - the next 120 frames (LKF): did the player stay on the ladder or drop?
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from trans_log import sram_text  # noqa: E402


def kv(line):
    return {k: int(v) for k, v in re.findall(r"(\w+)=(-?\d+)", line)}


def main():
    txt = sram_text(open(sys.argv[1], "rb").read())
    runs, cur = [], None
    for line in txt.splitlines():
        if line.startswith("LK from="):
            cur = {"lk": kv(line), "a": None, "f": []}
            runs.append(cur)
        elif line.startswith("LKA") and cur:
            cur["a"] = kv(line)
        elif line.startswith("LKF") and cur:
            cur["f"].append(kv(line))
    if not runs:
        print("no LK lines found (is this a MD_LINK_DEBUG run?)")
        return 1
    for r in runs:
        k, a = r["lk"], r["a"]
        old_sx = k["old_x"] - k["cam_old"]
        new_sx = k["px"] - k["cam_new"]
        d = new_sx - old_sx
        print(f"link {k['from']}->{k['to']} trans={k['trans']} type={k['type']}: "
              f"player screen x {old_sx} -> {new_sx} (diff {d:+d}) "
              f"{'ALIGNED' if abs(d) <= 2 else 'MISALIGNED'}; "
              f"cam {k['cam_old']} -> {k['cam_new']}")
        print(f"   stair_col={k['stair_col']} on_stair={k['on_stair']} "
              f"to=({k['to_x']},{k['to_y']}) band_off={k['band_off']} "
              f"keep_sx={k['keep_sx']}")
        if a:
            print(f"   arrival: x={a['x']} y={a['y']} on_ladder={a['on_ladder']} "
                  f"cam={a['cam']} under={a['under']}"
                  f"{'' if a['on_ladder'] else '   <-- NOT on the ladder'}")
        if r["f"]:
            lad = [f["ladder"] for f in r["f"]]
            drop = next((f for f in r["f"] if not f["ladder"]), None)
            print(f"   follow-up: {len(r['f'])} samples, y {r['f'][0]['y']} -> "
                  f"{r['f'][-1]['y']}, ladder flag {lad[:12]}")
            if drop:
                print(f"   DROP at t={drop['t']}: x={drop['x']} y={drop['y']} "
                      f"vy={drop['vy']} under={drop['under']} cam={drop['cam']}")
    return 0


if __name__ == "__main__":
    sys.exit(main() if len(sys.argv) > 1 else print(__doc__) or 2)
