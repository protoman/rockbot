#!/usr/bin/env python3
"""Vertical room transitions, offline (transition_plan.md, phase 1).

Builds tests/host/trans_sim.c (the REAL src/transition.c against a VDP model)
and runs it for every vertical link of every converted stage, both directions.
Per link it reports what the slide DISPLAYS, decoded back from the modelled
name table frame by frame:

  stutter    the picture offset must advance by <= STEP_MAX px on EVERY vblank
             (PC: 4 px per ~6 ms step, smooth); holds (delta 0) and big jumps fail
  tear       all 224 lines of a frame must show one consistent picture offset
  hmis       cells shown at the wrong source column (alignment inside a half)
  ends       first frame = outgoing screen at rest, last frame = incoming screen
  align      the link ladder must be at the same screen X in both halves
             (PC adjust_x does exactly this); computed from the arrival maths
  dma        bytes / queue entries sent in ONE vblank (budget below); the first slide
             frame (ring fill) is reported separately and must fit the queue (192)
  parallax   BG_B rows must show the art line the stage would (adjust_y, plane wrap)
  stale      a frame DISPLAYED in HSCROLL_LINE mode before the slide wrote the line table
             (the table in VRAM is garbage then: ladder 233 px off, 1-frame flash; 2026-11)
  vscroll    BG_A's vertical scroll must be written with the VSync variant
  spr        sprites on one scanline during the slide (hardware limit 20)

Scroll locks (which move the destination camera in the ROM) are NOT modelled:
the arrival camera is clamp(dest focus - screen x of the link), like
stage_arrival_camera_keep without scroll_lock_span.

usage: python3 tools/check_transitions.py [stage ...] [--only-fails] [--verbose]
"""
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))
import audit_links as al  # noqa: E402

OUT = "/tmp/opencode/host-tests"
SIM = os.path.join(OUT, "trans_sim")

STEP_MAX = 4           # px per vblank (PC: 4 px per step)
DMA_BYTES_MAX = 1024   # per vblank, well under the ~7 KB the VDP can take in vblank
DMA_ENTRIES_MAX = 8
SPR_LINE_MAX = 20


def build():
    os.makedirs(OUT, exist_ok=True)
    cmd = ["gcc", "-std=gnu11", "-O1", "-DMD_HOST_TEST", "-I", "tests/host", "-I", "src",
           "-I", "res", "-o", SIM, "src/transition.c", "tests/host/trans_sim.c"]
    r = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)
    if r.returncode:
        print(r.stderr)
        sys.exit(2)


def bg_adjust(n):
    """adjust_y per map from stageN_bg_info (parallax art offset)."""
    path = os.path.join(ROOT, "src", "stage%d_data.c" % n)
    m = re.search(r"stage%d_bg_info\[\d+\]\s*=\s*\{(.*?)\n\};" % n, open(path).read(), re.S)
    out = []
    if m:
        for row in re.findall(r"\{([^{}]*)\}", m.group(1)):
            v = [x.strip() for x in row.split(",") if x.strip()]
            out.append((int(v[0]), int(v[1])))        # present, adjust_y
    return out


def enemy_spawns(n):
    path = os.path.join(ROOT, "src", "stage%d_enemies.c" % n)
    if not os.path.exists(path):
        return []
    text = open(path).read()
    m = re.search(r"stage%d_enemy_spawns\[[^\]]*\]\s*=\s*\{(.*?)\n\};" % n, text, re.S)
    out = []
    if m:
        for row in re.findall(r"\{([^{}]*)\}", m.group(1)):
            v = [x.strip() for x in row.split(",") if x.strip()]
            if len(v) >= 4:
                out.append((int(v[0]), int(v[1]), int(v[2]), int(v[3])))   # def, map, x, y
    return out


def clamp(v, lo, hi):
    return max(lo, min(hi, v))


def link_region(st, px, py, py_centre, tx, ty, size, trans):
    """Mirror of stage.c link_region (trans 0 = T2B, 1 = B2T)."""
    lim1 = tx * 16
    lim2 = lim1 + (size or 1) * 16
    if px < lim1 or px > lim2:
        return False
    lim3 = ty * 16 + 8
    lim4 = ty * 16 + 16
    if trans == 0:
        lim3 += 8
        lim4 += 24
    else:
        lim3 -= 27
    if ty >= st["map_h"] - 1:
        return py_centre >= ty * 16
    return lim3 < py < lim4


def band_hit(st, cur_map, left, top, right, bottom):
    px = (left + right) >> 1
    py = bottom - 8
    for (mfrom, tx, ty, mto, dx, dy, size, bidir, ltype) in st["links"]:
        if mfrom == cur_map:
            t, u, trans = tx, ty, (0 if ty > dy else 1)
        elif mto == cur_map and bidir:
            t, u, trans = dx, dy, (0 if ty < dy else 1)
        else:
            continue
        if link_region(st, px, py, (top + bottom) >> 1, t, u, size, trans):
            return True
    return False


def handoff_jump(st, dmap, px, py):
    """main.c: after stage_load an arrival on row 0 / last row is nudged out of the
    link band in 8px steps (up to 4). The slide showed the player at `py`."""
    py_max = st["map_h"] * 16 - al.BOX_B
    p = py
    if py == 0 or py == py_max:
        step = 8 if py == 0 else -8
        for _ in range(4):
            if not band_hit(st, dmap, px + al.BOX_L, p + al.BOX_T, px + al.BOX_R, p + al.BOX_B):
                break
            ny = p + step
            if ny < 0 or ny > py_max:
                break
            p = ny
    return p - py


def nudge_before_slide():
    """main.c runs the band nudge before transition_scroll (rule arrival-nudge-before-slide)."""
    t = open(os.path.join(ROOT, "src", "main.c")).read()
    return bool(re.search(r"stage_link_band_hit\([^;]*;[\s\S]*?transition_scroll\(", t)) and \
        not re.search(r"stage_load\(to_map\);[\s\S]{0,400}stage_link_band_hit\(", t)


def scenarios(n, st):
    """Yield (label, t2b, width, from_cam, to_cam, old_shift, n_prev, align_err)."""
    w = st["width"]
    maxcam = w * 16 - 320
    spawns = enemy_spawns(n)
    for i, (mfrom, tx, ty, mto, dx, dy, size, bidir, ltype) in enumerate(st["links"]):
        if ltype >= 2:
            continue           # teleporters fade, no slide
        trips = [(mfrom, tx, ty, mto, dx, dy, "fwd")]
        if bidir:
            trips.append((mto, dx, dy, mfrom, tx, ty, "rev"))
        for smap, sx, sy, dmap, ax, ay, kind in trips:
            if not (ay == 0 or ay == st["map_h"] - 1):
                continue       # not a vertical arrival
            t2b = 1 if sy > ay else 0                       # PC: origin.y > destiny.y
            oy = al.arrival(st, smap, sx, sy)[1]   # where the old room draws him
            band = size * 8 - 13                            # player centred in the band
            px_src = sx * 16 + band
            focus_src = px_src + 13
            cam_src = clamp(focus_src - 160, 0, maxcam)
            keep_sx = clamp(focus_src - cam_src, 0, 319)
            px_dst, _, on_stair = al.arrival(st, dmap, ax, ay)
            if not on_stair:
                px_dst = ax * 16 + band
            focus_dst = px_dst + 13
            cam_dst = clamp(focus_dst - keep_sx, 0, maxcam)
            old_shift = (focus_dst - cam_dst) - keep_sx
            ocam = clamp(cam_src - old_shift, 0, maxcam)
            # ladder screen x in each half after the shift
            align_err = (focus_dst - cam_dst) - (focus_src - ocam)
            npv = 0
            for (d, m, x, y) in spawns:
                if m == dmap and -40 <= x - cam_dst < 336:
                    npv += 1
            label = "stage%d link%d %s m%d(%d,%d)->m%d(%d,%d) %s" % (
                n, i, kind, smap, sx, sy, dmap, ax, ay, "T2B" if t2b else "B2T")
            _, py_dst, _ = al.arrival(st, dmap, ax, ay)
            hj = handoff_jump(st, dmap, px_dst, py_dst) if not nudge_before_slide() else 0
            # stage_update_camera (dead zone +-12 around screen x 160) runs on the first
            # game frame: a player who arrived at another screen x drags the camera.
            cam_after = cam_dst
            if focus_dst > cam_dst + 160 + 12:
                cam_after = focus_dst - 160 - 12
            elif focus_dst < cam_dst + 160 - 12:
                cam_after = focus_dst - 160 + 12
            cam_after = clamp(cam_after, 0, maxcam)
            snap = cam_after - cam_dst
            bgs = bg_adjust(n)
            adj = (bgs[smap][1] if smap < len(bgs) else 0, bgs[dmap][1] if dmap < len(bgs) else 0)
            yield label, t2b, w, cam_src, cam_dst, old_shift, min(npv, 14), (align_err, hj, snap), (oy, adj)


def run(t2b, w, from_cam, to_cam, old_shift, n_prev, old_y=150, adj=(0, 0)):
    r = subprocess.run([SIM, str(t2b), str(w), str(w), str(from_cam), str(to_cam),
                        str(old_shift), str(n_prev), str(old_y), str(adj[0]), str(adj[1])], capture_output=True, text=True)
    res = {}
    for ln in r.stdout.splitlines():
        if ln.startswith("SLIDE"):
            res["p"] = [int(v) for v in ln.split("p=[")[1].rstrip("]").split(",") if v]
        elif ln.startswith("PLY"):
            res["ply_old"] = int(re.search(r"old=(-?\d+)", ln).group(1))
            res["ply"] = [int(v) for v in ln.split("y=[")[1].rstrip("]").split(",") if v]
        elif ln.startswith("STALE"):
            res["stale"] = int(re.search(r"frames=(\d+)", ln).group(1))
        elif ln.startswith("TEAR"):
            res["tear"] = int(ln.split("n=")[1])
        elif ln.startswith("HMIS"):
            a = re.findall(r"=(\d+)", ln)
            res["hmis_old"], res["hmis_new"] = int(a[0]), int(a[1])
        elif ln.startswith("DMA"):
            a = re.findall(r"=(\d+)", ln)
            res["dma_bytes"], res["dma_entries"] = int(a[0]), int(a[1])
            res["dma_first"] = (int(a[2]), int(a[3]))
        elif ln.startswith("SPR"):
            res["spr"] = int(ln.split("=")[1])
        elif ln.startswith("BGMIS"):
            res["bgmis"] = int(ln.split("=")[1])
        elif ln.startswith("VSIMM"):
            res["vs_imm"] = int(ln.split("=")[1])
        elif ln.startswith("RESULT"):
            res["ok"] = ln.endswith("1")
    return res


def verdict(t2b, res, extra):
    align_err, hj, snap = extra
    f = []
    if not res.get("ok"):
        return ["fallback-to-fade"]
    p = [v for v in res["p"] if v >= 0]
    # progress q: 0 = outgoing screen at rest, 224 = incoming screen at rest
    q = [v if t2b else 224 - v for v in p]
    deltas = [b - a for a, b in zip(q, q[1:])]
    holds = sum(1 for d in deltas if d == 0)
    biggest = max(deltas) if deltas else 0
    if holds or biggest > STEP_MAX:
        f.append("stutter(max step %dpx, %d held frames of %d)" % (biggest, holds, len(q)))
    if q and q[0] != 0:
        f.append("first-frame(q=%d)" % q[0])
    if q and q[-1] != 224:
        f.append("last-frame(q=%d)" % q[-1])
    ply = res.get("ply") or []
    if ply:
        # the player is carried from his old on-screen spot to the arrival spot (PC moves him
        # with the scroll); he must not vanish at frame 0 or jump between frames
        if abs(ply[0] - res["ply_old"]) > 6:
            f.append("player-start(first frame y=%d, was %d in the old room)" % (ply[0], res["ply_old"]))
        d = [abs(b - a) for a, b in zip(ply, ply[1:])]
        if d and max(d) > 6:
            f.append("player-jump(%dpx between frames)" % max(d))
    if res.get("stale"):
        f.append("stale-hscroll-table(%d displayed frame(s) in line mode before the table landed)" % res["stale"])
    if res["tear"]:
        f.append("tear(%d frames)" % res["tear"])
    if res["hmis_old"] or res["hmis_new"]:
        f.append("hmis(old %d, new %d cells)" % (res["hmis_old"], res["hmis_new"]))
    if align_err:
        f.append("align(%+dpx)" % align_err)
    if snap:
        f.append("camera-snap(%+dpx on the first game frame)" % snap)
    if hj:
        f.append("handoff(player y jumps %+dpx after the slide)" % hj)
    if res["dma_bytes"] > DMA_BYTES_MAX or res["dma_entries"] > DMA_ENTRIES_MAX:
        f.append("dma(%dB/%d entries per vblank)" % (res["dma_bytes"], res["dma_entries"]))
    # the parallax switches by 8px rows while BG_A's boundary moves by pixels: up to 7 lines
    # per frame are legitimately on the other side; anything above that is a wrong row
    if res.get("bgmis", 0) > 8 * len(p):
        f.append("parallax(%d lines show the wrong art row)" % res["bgmis"])
    if res.get("vs_imm"):
        f.append("vscroll-immediate(%d writes tear the previous frame)" % res["vs_imm"])
    if res["spr"] > SPR_LINE_MAX:
        f.append("spr(%d on a line)" % res["spr"])
    return f


def main(argv):
    only_fails = "--only-fails" in argv
    verbose = "--verbose" in argv
    wanted = [int(a) for a in argv[1:] if a.isdigit()] or list(range(14))
    build()
    total = bad = 0
    kinds = {}
    for n in wanted:
        st = al.parse_stage(n)
        if not st:
            continue
        for label, t2b, w, fc, tc, osh, npv, aerr, oy in scenarios(n, st):
            res = run(t2b, w, fc, tc, osh, npv, oy[0], oy[1])
            f = verdict(t2b, res, aerr)
            total += 1
            if f:
                bad += 1
            for x in f:
                k = x.split("(")[0]
                kinds[k] = kinds.get(k, 0) + 1
            if f or not only_fails:
                print("%-52s cam %4d->%4d shift %+4d prev %2d : %s"
                      % (label, fc, tc, osh, npv, ", ".join(f) if f else "ok"))
            if verbose and res.get("p"):
                print("    p:", res["p"][:12], "...")
    print("transitions checked: %d, failing: %d" % (total, bad))
    print("by kind:", ", ".join("%s=%d" % kv for kv in sorted(kinds.items())) or "none")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
