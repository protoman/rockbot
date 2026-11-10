#!/usr/bin/env python3
"""Incremental asset pipeline (replaces the one-stamp Makefile recipe).

The editor rewrites every .dat file on save, and the old rule rebuilt the whole
pipeline whenever any of them was newer than a stamp. This driver decides by
CONTENT instead:

  * every step lists the inputs it really reads (files, directories, and for the
    stage converters just that stage's slice of maps_v2.dat / stages_v301.dat);
  * a step runs only when the hash of those inputs (plus its tool scripts)
    differs from the one recorded in res/.assets.cache.json;
  * after a run, every generated file whose bytes did not change gets its old
    mtime back, so rescomp / gcc downstream rebuild only what really changed;
  * res/.assets.stamp is touched only when some output changed.

usage: tools/build_assets.py [--force] [--list] [--only NAME[,NAME]]
"""
import glob
import hashlib
import json
import os
import re
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)                         # mega_drive/
PCROOT = os.path.dirname(ROOT)
PY = os.environ.get("PYTHON", "/home/iuri/python3/bin/python3")
DATA = "data/games/Rockbot1"
IMAGES = "images"
PCIMG = "../build/shared/images"
PCGAME = "../build/games/Rockbot1"
STAMP = "res/.assets.stamp"
CACHE = "res/.assets.cache.json"
def _stages():
    # same list the Makefile keeps (tools/add_stage.py appends to it)
    m = re.search(r"^STAGES\s*:=\s*(.*)$", open(os.path.join(ROOT, "Makefile")).read(), re.M)
    return [int(x) for x in (m.group(1).split() if m else [])]


STAGES = _stages()

STAGE_REC = 438              # sizeof(file_stage), stages_v301.dat
MAP_REC = 15078 * 3          # three file_map_v2 per stage, maps_v2.dat

os.chdir(ROOT)

# ---------------------------------------------------------------- hashing
_file_cache = {}


def file_digest(path):
    st = os.stat(path)
    key = (path, st.st_mtime_ns, st.st_size)
    d = _file_cache.get(key)
    if d is None:
        h = hashlib.sha1()
        with open(path, "rb") as f:
            for chunk in iter(lambda: f.read(1 << 20), b""):
                h.update(chunk)
        d = h.hexdigest()
        _file_cache[key] = d
    return d


def tree_files(spec, exclude=()):
    """Expand a file / directory / glob into a sorted list of files."""
    out = []
    for p in sorted(glob.glob(spec)) or [spec]:
        if os.path.isdir(p):
            for dp, dn, fn in os.walk(p):
                dn.sort()
                for f in sorted(fn):
                    out.append(os.path.join(dp, f))
        elif os.path.isfile(p):
            out.append(p)
    return [f for f in out if not any(f.endswith(e) or ("/" + e + "/") in f for e in exclude)]


def slice_digest(path, rec, idx):
    with open(path, "rb") as f:
        f.seek(rec * idx)
        return hashlib.sha1(f.read(rec)).hexdigest()


def script_closure(*names):
    """The scripts a step runs plus the tools modules they import."""
    seen, todo = set(), list(names)
    while todo:
        n = todo.pop()
        p = os.path.join("tools", n)
        if n in seen or not os.path.isfile(p):
            continue
        seen.add(n)
        for m in re.findall(r"^\s*(?:import|from)\s+(\w+)", open(p).read(), re.M):
            todo.append(m + ".py")
    return sorted(os.path.join("tools", n) for n in seen)


def hash_inputs(items):
    h = hashlib.sha1()
    for it in items:
        if isinstance(it, tuple):             # ("slice", path, rec, idx) or ("text", s)
            if it[0] == "slice":
                h.update(("%s:%d:%d=" % (it[1], it[2], it[3])).encode())
                h.update(slice_digest(*it[1:]).encode())
            else:
                h.update(it[1].encode())
        else:
            h.update(it.encode())
            if os.path.exists(it):
                h.update(file_digest(it).encode())
    return h.hexdigest()


# ---------------------------------------------------------------- step table
# Data the per-stage converters read. Everything else under DATA feeds the
# shared (menu / scene / HUD) converters.
STAGE_DATA = ["maps_v2.dat", "map_npc_data.dat", "map_object_data.dat",
              "game_enemy_list_3_1_2.dat", "game_ai_list.dat", "game_projectile_list.dat",
              "game_object_list.dat", "anim_block_list.dat", "stages_v301.dat"]
IMG_DIRS = [DATA + "/images", IMAGES, "art", PCIMG, PCGAME + "/images"]

steps = []                                    # (name, [argv...], inputs[list])
EXTERNAL_TOOL_STEPS = {"music", "players"}


def add(name, argv, inputs):
    steps.append((name, [PY] + argv, inputs))


def images():
    out = []
    for d in IMG_DIRS:
        out += [f for f in tree_files(d) if f.endswith(".png")]
    return out


def shared_data():
    """Every file of the game data the shared converters may read."""
    skip = set(STAGE_DATA) | {"music", "sfx"}
    out = []
    for e in sorted(os.listdir(DATA)):
        if e in skip:
            continue
        out += tree_files(os.path.join(DATA, e))
    return out


def common(*scripts):
    return script_closure(*scripts)


def build_steps():
    steps.clear()
    img = images()
    shared = shared_data()
    # --- shared art / data -----------------------------------------------
    add("assets", ["tools/convert_assets.py"], common("convert_assets.py") + img + shared)
    # --- per stage ----------------------------------------------------------
    for s in STAGES:
        rec = ("slice", DATA + "/stages_v301.dat", STAGE_REC, s)
        maps = ("slice", DATA + "/maps_v2.dat", MAP_REC, s)
        anim = [DATA + "/anim_block_list.dat"]
        add("stage%d" % s, ["tools/convert_stage.py", str(s)],
            common("convert_stage.py") + [rec, maps] + anim + img)
        add("enemies%d" % s, ["tools/convert_enemies.py", str(s)],
            common("convert_enemies.py") + [rec, maps] +
            [DATA + "/" + f for f in ("map_npc_data.dat", "game_enemy_list_3_1_2.dat", "game_ai_list.dat",
                                      "game_projectile_list.dat")] +
            tree_files(DATA + "/data") + img)
        add("objects%d" % s, ["tools/convert_objects.py", str(s)],
            common("convert_objects.py") + [rec, maps] +
            [DATA + "/map_object_data.dat", DATA + "/game_object_list.dat"] + img)
        for m in range(3):
            png = "res/stage%d_bg%d.png" % (s, m)
            # the converter only writes the maps that HAVE a background
            # (stage 0 map 2 has none), so a missing file is normal
            if not os.path.isfile(png):
                continue
            add("bg%d_%d" % (s, m), ["tools/img2tiles.py", png, "--name", "stage%d_bg%d" % (s, m)],
                common("img2tiles.py") + [png])
    # --- shared converters --------------------------------------------------
    for name, script in (("players", "convert_players.py"), ("weapons", "convert_weapons.py"),
                         ("menu_icons", "convert_menu_icons.py"),
                         ("menu_icons_gray", "convert_menu_icons_gray.py"),
                         ("menu_tanks", "convert_menu_tanks.py"),
                         ("tooltip", "convert_tooltip.py"), ("hud_icons", "convert_hud_icons.py"),
                         ("credits", "convert_credits.py"), ("player", "convert_player.py"),
                         ("save_icons", "convert_save_icons.py")):
        add(name, ["tools/" + script], common(script) + img + shared)
    add("starfield", ["tools/convert_starfield.py"],
        common("convert_starfield.py") + [PCIMG + "/star_field1.png", PCIMG + "/star_field2.png"])
    add("parade", ["tools/convert_parade.py"],
        common("convert_parade.py", "convert_enemies.py") + img + shared +
        [DATA + "/game_enemy_list_3_1_2.dat", DATA + "/stages_v301.dat"])
    tiles = [("gotbg", IMAGES + "/shared/config_bg.png", ["--crop-h", "224"]),
             ("dlg_panel", IMAGES + "/backgrounds/dialog.png", []),
             ("boss_intro_bg", IMAGES + "/backgrounds/boss_intro/boss_intro_bg.png", ["--crop-h", "224"]),
             ("psel_bg", IMAGES + "/backgrounds/player_selection.png", ["--crop-h", "224"]),
             ("psel_p1", IMAGES + "/backgrounds/player_select_p1.png", []),
             ("psel_p2", IMAGES + "/backgrounds/player_select_p2.png", []),
             ("psel_p3", IMAGES + "/backgrounds/player_select_p3.png", []),
             ("psel_p4", IMAGES + "/backgrounds/player_select_p4.png", [])]
    for name, png, extra in tiles:
        add("tiles_" + name, ["tools/img2tiles.py", png, "--name", name] + extra,
            common("img2tiles.py") + [png])
    add("stage_select", ["tools/convert_stage_select.py"], common("convert_stage_select.py") + img + shared)
    l2 = PCGAME + "/images/ending/player_walking_layer2.png"
    add("end_l2_pad", ["tools/pad_top.py", l2, "res/end_l2_pad.png"], common("pad_top.py") + [l2])
    # The layers scroll, so their strips must repeat over the 512 px plane (the
    # PC art repeats at 320, which does not divide 512 - the grass jumped).
    add("end_layers", ["tools/ending_layers.py"],
        common("ending_layers.py") + [l2] +
        [PCGAME + "/images/ending/player_walking_layer%d.png" % n
         for n in (1, 3)])
    for n in (1, 2, 3):
        png = "res/end_l%d_src.png" % n
        add("tiles_end_l%d" % n, ["tools/img2tiles.py", png, "--name", "end_l%d" % n, "--colors", "15"],
            common("img2tiles.py") + [png])
    add("save_selector", ["tools/convert_save_selector.py"], common("convert_save_selector.py") + img + shared)
    cm = PCGAME + "/images/backgrounds/classic_style_weapon_menu.png"
    add("tiles_menu", ["tools/img2tiles.py", cm, "--name", "menu", "--crop-top", "16"],
        common("img2tiles.py") + [cm])
    add("title64", ["tools/title64.py", "--name", "title64", "--crop-h", "224",
                    "--rekey", "219,36,182=255,36,182", "--rekey", "219,109,73=255,146,36",
                    "--rekey", "219,182,146=182,109,73",
                    "--part", "art/title.png:18,4:0:1", "--part", "art/sub_title.png:162,38:0:1",
                    "--part", "art/title_rock_and_beta.png:148,48:1:1"],
        common("title64.py") + ["art/title.png", "art/sub_title.png", "art/title_rock_and_beta.png"])
    castle = PCGAME + "/images/backgrounds/classic_style_castle.png"
    for i, (y, h) in enumerate(((8, 56), (64, 56), (120, 56), (176, 56))):
        add("castle_b%d" % i, ["tools/crop_image.py", castle, "res/castle_b%d.png" % i, "0", str(y), "320", str(h)],
            common("crop_image.py") + [castle])
    add("castle64", ["tools/title64.py", "--name", "castle64", "--crop-h", "224",
                     "--part", "res/castle_b0.png:0,0:0:1", "--part", "res/castle_b1.png:0,56:1:1",
                     "--part", "res/castle_b2.png:0,112:2:1", "--part", "res/castle_b3.png:0,168:3:1"],
        common("title64.py") + ["res/castle_b%d.png" % i for i in range(4)])
    add("dialogs", ["tools/convert_dialogs.py"], common("convert_dialogs.py") + img + shared)
    add("scenes", ["tools/convert_scenes.py"], common("convert_scenes.py") + img + shared)
    add("sfx", ["tools/convert_sfx.py"], common("convert_sfx.py") + tree_files(DATA + "/sfx"))
    add("energy_bars", ["tools/convert_energy_bars.py"], common("convert_energy_bars.py") + img + shared)
    add("music", ["tools/convert_music.py"],
        common("convert_music.py") + tree_files(DATA + "/music") + ["tools/mod2vgm/build/mod2vgm"])
    add("gpl", ["tools/gen_gpl.py"], common("gen_gpl.py") + img)


def scene_tile_steps():
    """Steps that depend on files the earlier steps generate (res/scene_*)."""
    out = []
    for n in ("scene_par0_l0", "scene_par0_l1"):
        png = "res/%s.png" % n
        out.append(("tiles_" + n, [PY, "tools/img2tiles.py", png, "--name", n],
                    script_closure("img2tiles.py") + [png]))
    for png in sorted(glob.glob("res/scene_img*.png")):
        if png.endswith(("_tiles.png", "_tiles_rgba.png")):
            continue
        n = os.path.basename(png)[:-4]
        out.append(("tiles_" + n, [PY, "tools/img2tiles.py", png, "--name", n],
                    script_closure("img2tiles.py") + [png]))
    return out


# ---------------------------------------------------------------- snapshot
OUT_ROOTS = ["res", "src"]


def snapshot():
    snap = {}
    for root in OUT_ROOTS:
        for dp, dn, fn in os.walk(root):
            for f in fn:
                p = os.path.join(dp, f)
                try:
                    st = os.stat(p)
                except OSError:
                    continue
                snap[p] = (st.st_mtime_ns, file_digest(p))
    return snap


# ---- hand-owned art -------------------------------------------------------
# Owner decision (2026-11-01): the art in res/ is edited BY HAND from now on, formats are
# final, and no converter may write it again. Every converter still runs (it also emits the
# generated C data and res/resources.res), but afterwards any art file that existed before
# the run and changed is put back byte for byte. Only the tile layouts rendered from the map
# data (res/stageN_mapM.png: map editor output) are exempt, so map edits still reach the ROM.
# `--force-art` disables the protection (regenerate everything from images/).
MAP_LAYOUT = re.compile(r"^res/stage\d+_map\d+\.png$")


def art_source():
    """`res` (default): art in res/ is hand-owned, converters never write it.
    `images`: images/ is the source again, converters regenerate res/ art
    (set art/art_source.txt to the single word `images` once the edits live there)."""
    try:
        return open(os.path.join(ROOT, "art", "art_source.txt")).read().split()[0].lower()
    except (OSError, IndexError):
        return "res"


def protected_art():
    out = {}
    for f in sorted(os.listdir("res")) if os.path.isdir("res") else []:
        p = "res/" + f
        if not f.endswith((".png", ".gpl", ".bin")) or MAP_LAYOUT.match(p) or not os.path.isfile(p):
            continue
        out[p] = open(p, "rb").read()
    return out


def restore_art(saved):
    n = 0
    for p, data in saved.items():
        try:
            same = open(p, "rb").read() == data
        except OSError:
            same = False
        if not same:
            open(p, "wb").write(data)
            n += 1
            print("[assets] kept hand-owned %s" % p)
    return n


def main():
    argv = sys.argv[1:]
    force = "--force" in argv
    only = None
    if "--only" in argv:
        only = set(argv[argv.index("--only") + 1].split(","))
    build_steps()
    cache = {}
    if os.path.exists(CACHE) and not force:
        try:
            cache = json.load(open(CACHE))
        except Exception:
            cache = {}
    if "--list" in argv:
        for n, _, _ in steps:
            print(n)
        return 0

    before = snapshot()
    art = {} if ("--force-art" in argv or art_source() == "images") else protected_art()
    frozen = {os.path.abspath(k) for k in art}
    ran, failed = 0, None
    t0 = time.time()

    def dir_digest(d):
        try:
            return hashlib.sha1("\n".join(sorted(os.listdir(d))).encode()).hexdigest()
        except OSError:
            return "-"

    def deps_digest(files, dirs):
        h = {}
        for f in files:
            h[f] = file_digest(f) if os.path.isfile(f) else "-"
        for d in dirs:
            h["dir:" + d] = dir_digest(d)
        return h

    def run_step(name, cmd, inputs):
        """Up to date when the command and the explicit slices are unchanged AND every file the
        converter really opened last time (measured by tools/_trace) still has the same bytes.
        The first run after a change of the command (or with no record) is a full run."""
        nonlocal ran
        if only and name not in only:
            return True
        sliced = {it[1] for it in inputs if isinstance(it, tuple) and it[0] == "slice"}
        base = hash_inputs([it for it in inputs if isinstance(it, tuple)] + [("text", " ".join(cmd))])
        entry = cache.get(name)
        if isinstance(entry, dict) and entry.get("base") == base:
            # hand-owned art is never an input: a converter that peeks at its own old output
            # (enemies: the row count of the existing sheet) must not re-run because the owner
            # painted that sheet
            live = [f for f in entry["files"] if os.path.abspath(f) not in frozen]
            if deps_digest(live, entry["dirs"]) == {k: v for k, v in entry["deps"].items()
                                                    if k in live or k.startswith("dir:")}:
                return True
        print("[assets] %s" % name, flush=True)
        trace = os.path.join("res", ".trace_%s.json" % name)
        env = dict(os.environ, ASSETS_TRACE=trace, ASSETS_ROOT=ROOT,
                   PYTHONPATH=os.path.join(ROOT, "tools", "_trace") + os.pathsep +
                   os.environ.get("PYTHONPATH", ""))
        r = subprocess.run(cmd, env=env, stdout=subprocess.DEVNULL if "-v" not in argv else None)
        if r.returncode != 0:
            print("[assets] %s FAILED (%d)" % (name, r.returncode), file=sys.stderr)
            return False
        try:
            t = json.load(open(trace))
            os.remove(trace)
            files = [f for f in t["files"] if os.path.normpath(f) not in {os.path.normpath(x) for x in sliced}
                     and os.path.abspath(f) not in frozen]
            dirs = t["dirs"]
            if name in EXTERNAL_TOOL_STEPS:
                # these run a child program (mod2vgm, g++) whose file reads the Python hook cannot
                # see: keep the static input list for them as well
                files = sorted(set(files) | {x for x in inputs if isinstance(x, str) and os.path.isfile(x)})
            cache[name] = {"base": base, "files": files, "dirs": dirs,
                           "deps": deps_digest(files, dirs)}
        except Exception:
            cache.pop(name, None)           # no trace (tool crashed late): run again next time
        ran += 1
        return True

    ok = True
    scene_done = False
    for name, cmd, inputs in steps:
        if not run_step(name, cmd, inputs):
            ok = False
            break
        # the scene tile steps need res/scene_*.png from convert_scenes
        if name == "scenes" and not scene_done:
            scene_done = True
            for n2, c2, i2 in scene_tile_steps():
                if not run_step(n2, c2, i2):
                    ok = False
                    break
            if not ok:
                break
    # stale steps for removed stages / images are harmless; keep the cache
    kept_art = restore_art(art)
    os.makedirs("res", exist_ok=True)
    after = snapshot()
    changed = 0
    for p, (mt, dg) in after.items():
        old = before.get(p)
        if old and old[1] == dg and old[0] != mt:
            os.utime(p, ns=(old[0], old[0]))          # same bytes: keep the old mtime
        elif not old or old[1] != dg:
            changed += 1
    removed = [p for p in before if p not in after]
    if ok:
        json.dump(cache, open(CACHE, "w"), indent=0, sort_keys=True)
        if changed or removed or not os.path.exists(STAMP):
            open(STAMP, "a").close()
            os.utime(STAMP, None)
    print("[assets] %d step(s) run, %d output file(s) changed, %d hand-owned kept, %.1fs"
          % (ran, changed, kept_art, time.time() - t0))
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
