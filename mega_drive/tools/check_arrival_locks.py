#!/usr/bin/env python3
"""Offline model of stage_arrival_camera_keep() (src/stage.c) against the PC
scroll lock rule: for a link arrival, the camera must stay >= (c<<4)+2 for the
nearest locked column c left of the player and <= (c<<4)-307 for the nearest
one on his right. Prints every (stage, link, keep_sx) that breaks it, for the
current algorithm (cur) and the proposed one (fix).
usage (from mega_drive/): python3 tools/check_arrival_locks.py [fix|cur]
"""
import re, glob, sys

def load(sn):
    t = open(f'src/stage{sn}_data.c').read()
    n = re.search(r'scroll_lock\[(\d+)\]\[(\d+)\] = \{(.*?)\n\};', t, re.S)
    mx = int(n.group(2))
    rows = re.findall(r'\{([^}]*)\}', n.group(3))
    cnt = re.search(r'scroll_lock_n\[\d+\] = \{([^}]*)\}', t).group(1)
    cnt = [int(x) for x in re.findall(r'\d+', cnt)]
    locks = [set(int(x) for x in re.findall(r'\d+', r)[:cnt[i]]) for i, r in enumerate(rows)]
    m = re.search(rf'stage{sn}_links\[\d+\] = \{{(.*?)\}};', t, re.S)
    ls = []
    for line in m.group(1).splitlines():
        line = line.split('//')[0].strip().rstrip(',')
        v = [int(x) for x in re.findall(r'-?\d+', line)]
        if len(v) >= 6: ls.append(v)
    return locks, ls

def cur(locks, focus, cam):
    cam = max(0, min(cam, 200*16-320))
    cl = (cam + 14) >> 4; cr = (cam + 306) >> 4
    for c in range(cl + 1, cr):
        if c not in locks: continue
        if focus >= (c << 4) + 8:
            if cam < (c << 4) + 2: cam = (c << 4) + 2
        else:
            if cam > (c << 4) - 307: cam = (c << 4) - 307
    return max(0, min(cam, 200*16-320))

def fix(locks, focus, cam):
    """Mirror of scroll_lock_span() in src/stage.c (applied after every camera change)."""
    cam = max(0, min(cam, 200*16-320))
    fc = (focus - 8) >> 4
    lo, hi = -32000, 32000
    for c in range(fc, max(fc - 25, -1), -1):
        if c in locks: lo = (c << 4) + 2; break
    for c in range(fc + 1, fc + 25):
        if c in locks: hi = (c << 4) - 307; break
    if lo > hi: cam = (lo + hi) >> 1
    elif cam < lo: cam = lo
    elif cam > hi: cam = hi
    return max(0, min(cam, 200*16-320))

def truth_ok(locks, focus, cam):
    left = [c for c in locks if focus >= (c << 4) + 8]
    right = [c for c in locks if focus < (c << 4) + 8]
    ok = True
    if left and cam < (max(left) << 4) + 2: ok = False
    if right and cam > (min(right) << 4) - 307: ok = False
    return ok

algo = fix if (len(sys.argv) > 1 and sys.argv[1] == 'fix') else cur
bad = 0
for f in sorted(glob.glob('src/stage[0-9]*_data.c'), key=lambda x: int(re.findall(r'\d+', x)[0])):
    sn = int(re.findall(r'\d+', f)[0])
    try: locks, ls = load(sn)
    except Exception as e: continue
    for i, l in enumerate(ls):
        mf, tx, ty, mt, dx, dy = l[:6]
        for (mm, fx) in ((mt, dx), (mf, tx)):
            for keep in range(0, 320, 20):
                focus = fx * 16 + 8
                cam = algo(locks[mm], focus, focus - keep)
                # a lock pair closer than a screen cannot satisfy both
                if not truth_ok(locks[mm], focus, cam):
                    left = [c for c in locks[mm] if focus >= (c << 4) + 8]
                    right = [c for c in locks[mm] if focus < (c << 4) + 8]
                    if left and right and ((min(right) << 4) - 307) < ((max(left) << 4) + 2):
                        continue
                    bad += 1
                    print(f'stage {sn} link {i}(#{i+1}) -> map {mm} focus col {fx} keep {keep}: cam {cam}')
print('violations', bad)
