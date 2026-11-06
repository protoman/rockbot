#!/usr/bin/env python3
"""Which projectile trajectories do the converted enemies/bosses (AI shot
actions and default shot) and the players (weapons, normal/full/armor shots) use,
and which of them does the runtime NOT implement? Read-only.

Enemy side: src/eshot.c `case TRAJ_*` plus the spawn-time ones; player side:
src/shot.c. A used trajectory that has no case runs as a straight LINEAR shot.
usage (from mega_drive/): python3 tools/audit_projectile_traj.py
"""
import glob, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "src")
NAMES = {0:'LINEAR',1:'ARC',2:'SIN',3:'CHAIN',4:'QUAKE',5:'BOMB',6:'FOLLOW',7:'PROGRESSIVE',
         8:'FREEZE',9:'DIAG_UP',10:'DIAG_DOWN',11:'CENTERED',12:'ZIGZAG',13:'TARGET_DIRECTION',
         14:'ARC_TO_TARGET',15:'TARGET_EXACT',16:'FALL_BOMB',17:'LASER',18:'PUSH_BACK',
         19:'ARC_SMALL',20:'RING',21:'LIGHTING',22:'SPIRAL',23:'BOUNCING',24:'INVERSE_LINEAR',
         25:'DOUBLE_LINEAR',26:'DOUBLE_DIAGONAL',27:'BOMB_RAIN',28:'LARGE_BEAM',29:'PULL',
         30:'SLASH',31:'UP',32:'DOWN',33:'FALL_AND_SEEK',34:'AHEAD_AND_BACK'}
TRAJ_BY_NAME = {v: k for k, v in NAMES.items()}

def handled(path):
    t = open(os.path.join(SRC, path)).read()
    out = set()
    for m in re.finditer(r'TRAJ_([A-Z_]+)', t):
        n = m.group(1)
        # TRAJ_ names in the code use the short MD spelling
        n = {'DIAG_UP': 'DIAG_UP', 'DIAG_DOWN': 'DIAG_DOWN', 'TARGET_DIRECTION': 'TARGET_DIRECTION'}.get(n, n)
        if n in TRAJ_BY_NAME:
            out.add(TRAJ_BY_NAME[n])
    return out

E = handled('eshot.c')
P = handled('shot.c')

used_enemy = {}
for f in sorted(glob.glob(os.path.join(SRC, 'stage*_enemies.c'))):
    t = open(f).read()
    stage = re.search(r'stage(\d+)_enemies', f).group(1)
    proj = {}
    for m in re.finditer(r'\[(\d+)\] = \{ &proj_\d+_sprite, (\d+), (\d+), (\d+), (\d+), (\d+), (\d+) \}', t):
        proj[int(m.group(1))] = int(m.group(6))
    ai = re.search(r'enemy_ai\[[^\]]*\]\[20\] = \{(.*?)\n\};', t, re.S)
    if not ai:
        continue
    for row in re.findall(r'^\s*\{ (.*?) \},\s*// (.*)$', ai.group(1), re.M):
        steps = re.findall(r'\{(-?\d+),(-?\d+),(-?\d+),(-?\d+),(-?\d+)\}', row[0])
        for chance, action, go, delay, param in steps:
            if int(action) in (5, 6, 7, 25) and int(param) >= 0:
                pid = int(param)
                if pid in proj:
                    used_enemy.setdefault(proj[pid], set()).add((int(stage), row[1].strip(), pid))

print('ENEMY projectile trajectories used by AI shot actions:')
bad = 0
for tr in sorted(used_enemy):
    ok = tr in E or tr in (0, 9, 10, 13, 14, 15)     # spawn-time (linear/diagonal/aimed) cases
    who = sorted({w[1] for w in used_enemy[tr]})
    print('  %-16s %-4s %s' % (NAMES.get(tr, tr), 'ok' if ok else 'MISSING', ', '.join(who)[:100]))
    bad += 0 if ok else 1

player_ids = {
    'weapons': {1: 1, 2: 2, 3: 3, 4: 4, 5: 5, 6: 6, 7: 7, 8: 8},   # weapon -> projectile (convert_weapons: id_projectile)
}
print('PLAYER projectile trajectories (weapon_data / normal / charged / armor ids):')
# trajectories of the projectile ids the player can fire
tbl = {}
t = open(os.path.join(SRC, 'stage0_enemies.c')).read()
for m in re.finditer(r'\[(\d+)\] = \{ &proj_\d+_sprite, (\d+), (\d+), (\d+), (\d+), (\d+), (\d+) \}', t):
    tbl[int(m.group(1))] = int(m.group(6))
wd = open(os.path.join(SRC, 'weapon_data.c')).read()
wp = [int(x) for x in re.findall(r'\{ "[^"]*", (\d+), \d+ \}', wd)]
for w, pid in enumerate(wp[:11]):
    tr = tbl.get(pid)
    ok = tr in P or tr in (0, 9, 10)
    print('  weapon %-2d proj %-3d %-12s %s' % (w, pid, NAMES.get(tr, tr), 'ok' if ok else 'MISSING'))
    bad += 0 if ok else 1
for pid in (10, 11, 20, 21, 22, 23, 24, 25, 26, 35):
    tr = tbl.get(pid)
    ok = tr in P or tr in (0,)
    print('  player proj %-3d %-12s %s' % (pid, NAMES.get(tr, tr), 'ok' if ok else 'MISSING'))
    bad += 0 if ok else 1
print('%d unimplemented trajectories' % bad)
sys.exit(1 if bad else 0)
