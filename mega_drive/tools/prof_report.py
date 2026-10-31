#!/usr/bin/env python3
"""Summarise PROF lines of a -DMD_PROFILE run: work per frame by player column.

    python3 tools/prof_report.py [kdebug.log]
work = every mark except `wait` (idle); > 1280 subticks means the frame misses 60 Hz.
Marks share indices (objupd=obj+effects, plr=player+eshots, hud=hud+pshot).
"""
import re, sys
R = re.compile(r"PROF map=(\d+) obj=(\d+) npc=(\d+) hot=\S+ px=(-?\d+) py=(-?\d+) setup=(\d+) objupd=(\d+) plr=(\d+) hud=(\d+) cam=(\d+) anim=(\d+) npcs=(\d+) shot=(\d+) spr=(\d+) wait=(\d+) over=(\d+)")
rows = []
eprof = {}
cnt = {}
phase = 0
PH = {8: "sfx-muted", 9: "music-stopped", 10: "audio-off", 11: "audio-restored", 12: "enemy-marks", 0: "all", 1: "no-objects", 2: "no-enemies", 3: "no-obj+enemies", 4: "no-door-sync", 5: "no-solid-scan", 6: "prologue-only", 7: "empty-loop"}
for ln in open(sys.argv[1] if len(sys.argv) > 1 else "/tmp/opencode/md-emu/kdebug.log", errors="replace"):
    e = re.search(r"EPROF pre=(\d+) ai=(\d+) rest=(\d+) grav=(\d+) draw=(\d+) water=(\d+)", ln)
    if e: eprof.setdefault(phase, []).append(tuple(map(int, e.groups())))
    c = re.search(r"CNT act=(\d+) vdp=(\d+)", ln)
    if c: cnt.setdefault(phase, []).append(tuple(map(int, c.groups())))
    e = re.search(r"EPROF pre=(\d+) ai=(\d+) rest=(\d+) grav=(\d+) draw=(\d+) water=(\d+)", ln)
    if e: eprof.setdefault(phase, []).append(tuple(map(int, e.groups())))
    c = re.search(r"CNT act=(\d+) vdp=(\d+)", ln)
    if c: cnt.setdefault(phase, []).append(tuple(map(int, c.groups())))
    a = re.search(r"ABLATE phase=(\d+)", ln)
    if a: phase = int(a.group(1))
    m = R.search(ln)
    if m: rows.append(tuple(map(int, m.groups())) + (phase,))
print("map col  obj npc | work  objfx enemies plr+esh  spr  cam | over phase")
for (mp, ob, npc, px, py, setup, o, p, h, cam, an, en, sh, spr, wait, over, ph) in rows:
    if o > 5000: continue            # boot second
    work = setup + o + p + h + cam + an + en + sh + spr
    print(f"{mp:3d} {px//16:4d} {ob:4d} {npc:3d} | {work:5d} {o:5d} {en:6d} {p:7d} {spr:4d} {cam:4d} | {over:2d} {PH[ph]}{'  <-- slow' if work > 1280 else ''}")

# ---- per-phase averages + verdict (ablation ladder) -------------------------
def summary():
    by = {}
    for r in rows:
        if r[6] > 5000: continue
        (mp, ob, npc, px, py, setup, o, p, h, cam, an, en, sh, spr, wait, over, ph) = r
        work = setup + o + p + h + cam + an + en + sh + spr
        by.setdefault(PH[ph], []).append((work, o, en, over))
    print("\nphase            n  work  objfx  enemies  frames-over-budget/60")
    avg = {}
    for k, v in by.items():
        n = len(v); a = [sum(x[i] for x in v) / n for i in range(4)]
        avg[k] = a
        print(f"{k:15s} {n:3d} {a[0]:5.0f} {a[1]:6.0f} {a[2]:8.0f} {a[3]:8.0f}")
    if "all" in avg:
        a = avg["all"]
        print(f"\nbudget 1280: 'all' averages {a[0]:.0f} ({'OVER' if a[0] > 1280 else 'fits'}), "
              f"{a[3]:.0f}/60 frames over")
        for k, lab in (("no-objects", "objects"), ("sfx-muted", "(sfx) -"), ("music-stopped", "(music) -"), ("audio-off", "(all audio) -"), ("no-enemies", "enemies"),
                       ("no-door-sync", "door sync"), ("no-solid-scan", "solid scan"), ("prologue-only", "-> objects minus prologue"), ("empty-loop", "-> objects minus loop overhead")):
            if k in avg: print(f"  {lab:10s} costs ~{a[0]-avg[k][0]:.0f} subticks/frame")
def extras():
    v = eprof.get(12) or [x for l in eprof.values() for x in l]
    if v:
        n = len(v); a = [sum(x[i] for x in v) / n for i in range(6)]
        print(f"\nenemy marks (timer reads inflate them ~3x): pre={a[0]:.0f} ai={a[1]:.0f} "
              f"rest={a[2]:.0f} gravity={a[3]:.0f} sprite={a[4]:.0f} water={a[5]:.0f}")
    allc = [x for l in cnt.values() for x in l]
    if allc:
        print(f"sprites: active max {max(x[0] for x in allc)}, VDP sprites max {max(x[1] for x in allc)} (limit 80)")
    print("""
Reading guide (decided before the run):
  'empty-loop' still high             -> loop overhead / solid scan: restructure the loop
  'prologue-only' ~ all objects       -> the looping-anim prologue is the cost: hoist it
  door sync / solid scan big          -> cache those
  audio phases cheaper by >~100       -> XGM2/PCM bus time is the slowdown: fewer/cheaper SFX
  'all' fits, over~0 in every phase   -> autopilot misses the trigger: play rom_manual.bin
  enemy ai dominates                  -> the drill AI chain is the cost""")
extras()
summary()
