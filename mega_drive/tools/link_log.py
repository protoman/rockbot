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
        elif line.startswith("TRL") and cur:
            cur["trl"] = kv(line)
        elif line.startswith("LKN") and cur:
            m = re.match(r"LKN (\S+) fail=(\d+) failmap=(\d+) freekb=(\d+)", line)
            if m:
                cur["n"] = (m.group(1), int(m.group(2)), int(m.group(3)), int(m.group(4)))
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
        if "was_ladder" in k:
            print(f"   before: on_ladder={k['was_ladder']} dst_tx={k['dst_tx']} "
                  f"terrain at the arrival cell={k['t_dst']} (2 = stair)")
        t = r.get("trl")
        if t:
            print(f"   slide: ocam={t['ocam']} dcam={t['dcam']} old_hs={t['old_hs']} "
                  f"new_hs={t['new_hs']} (screen x of an origin column c = c*16-"
                  f"ocam; of a dest column = c*16-dcam)")
        elif k["type"] < 2:
            print("   slide: no TRL line -> transition_scroll did not reach the "
                  "destination load (fade fallback?)")
        if r.get("n"):
            rows, fc, fm, fk = r["n"]
            print(f"   terrain around the feet (rows above/at/below): {rows}; "
                  f"transition fail code={fc} map={fm} free={fk}KB")
        if a:
            print(f"   arrival: x={a['x']} y={a['y']} on_ladder={a['on_ladder']} "
                  f"cam={a['cam']} under={a['under']}"
                  f"{'' if a['on_ladder'] else '   <-- NOT on the ladder'}")
        if r["f"]:
            lad = [f["ladder"] for f in r["f"]]
            drop = next((f for f in r["f"] if not f["ladder"]), None)
            pads = sorted({f.get('pad', 0) for f in r['f']})
            print(f"   pad values seen: {[hex(x) for x in pads]}")
            print(f"   follow-up: {len(r['f'])} samples, y {r['f'][0]['y']} -> "
                  f"{r['f'][-1]['y']}, ladder flag {lad[:12]}")
            if drop:
                print(f"   DROP at t={drop['t']}: x={drop['x']} y={drop['y']} "
                      f"vy={drop['vy']} under={drop['under']} cam={drop['cam']}")
    return 0


if __name__ == "__main__":
    sys.exit(main() if len(sys.argv) > 1 else print(__doc__) or 2)
