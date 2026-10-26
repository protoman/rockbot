#!/usr/bin/env python3
"""Every player sprite must keep all 14 animations (src/player.h ANIM_*).

rescomp DROPS an animation whose frames are all transparent. Players 1 and 3 have
no art for SPECIAL (the shoryuken pose), so animation 10 vanished, everything
after it shifted down by one at runtime: WALK_ATTACK (12) played the JUMP_ATTACK
art (shooting while walking showed the jump-attack pose), GOT_WEAPON played the
walk-attack art, and JUMP_ATTACK (13) was out of range ("sprite guard: animation
out of range (index 13, 13 valid)").

Checks, in order:
  1. res/player_pN.png: every animation's used frames hold at least one opaque
     pixel (fails BEFORE the build, reproduces the bug offline);
  2. out/release/res/resources.s (after a build): numAnimation == 14 for every
     player sprite.
Exit 1 on any failure.
"""
import os
import re
import sys

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src = open(os.path.join(ROOT, "tools/convert_player.py")).read()
m = re.search(r"^ANIMS\s*=\s*\[(.*?)^\]", src, re.S | re.M)
anims = [(a, len(eval(b))) for a, b, _ in re.findall(
    r'\(\s*"(\w+)"\s*,\s*(\[[^\]]*\])\s*,\s*(\[[^\]]*\])\s*\)', m.group(1))]
bad = 0
for p in (1, 2, 3, 4):
    path = os.path.join(ROOT, "res", f"player_p{p}.png")
    im = Image.open(path).convert("P")
    px = im.load()
    for r, (name, n) in enumerate(anims):
        used = sum(1 for f in range(n) for y in range(32) for x in range(32)
                   if px[f * 32 + x, r * 32 + y] != 0)
        if used == 0:
            print(f"FAIL player_p{p}.png: animation {r} {name} is blank - rescomp "
                  f"drops it and every later animation shifts down by one")
            bad += 1
res = os.path.join(ROOT, "out/release/res/resources.s")
if os.path.exists(res):
    text = open(res).read()
    for p in (1, 2, 3, 4):
        mm = re.search(rf"^player_p{p}_sprite:\n(?:\s+dc\.\w+\s+[^\n]*\n){{3}}\s+dc\.w\s+(\d+)",
                       text, re.M)
        if mm and int(mm.group(1)) != len(anims):
            print(f"FAIL resources.s: player_p{p}_sprite has {mm.group(1)} animations, "
                  f"expected {len(anims)}")
            bad += 1
print("check_player_anims:", "OK" if not bad else f"{bad} problem(s)")
sys.exit(1 if bad else 0)
