#!/usr/bin/env python3
"""Convert the PC scene script tables into compact C for the scene player.

The PC stores a scene as a list of operations (file/v4/file_scene_v4.h),
one table per operation type, assembled by name in scenes/sceneshow.cpp.
This tool reads the tables in build/games/RockDroid1/scenes/ and writes:

  src/scenes_data.h/.c   scene op lists, text lines, clear areas, parallax
  res/scene_par<N>_l<L>.png   parallax layer art baked to 512 px (plane width)

The baked art is decomposed by tools/img2tiles.py afterwards (Makefile), so
the tiles/map come from the same emitter as the stage backgrounds.

Only the intro is wired right now. Show-image / viewpoint / animation records
are NOT converted: their art needs a colour-reduction pass first (the 68x68
character images have thousands of colours). Audio records are parsed (so the
op list is complete) but the player stubs them.

Usage: python3 tools/convert_scenes.py
"""
import os
import struct
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ROCKBOT = os.path.dirname(ROOT)
DATA = os.path.join(ROOT, "data/games/RockDroid1/scenes")
IMAGES = os.path.join(ROOT, "data/games/RockDroid1/images/scenes")
RES = os.path.join(ROOT, "res")
SRC = os.path.join(ROOT, "src")

# C++ struct sizes (4-byte alignment, x86 little endian)
SCENE_STRUCT = 512      # name[30] + pad + 20 * file_scene_object(24)
SCENE_OBJ_OFF = 32
SCENE_OBJ_N = 20
TEXT_STRUCT = 72        # name[30] pad + 6 ids + x/y/transition/position
TEXT_IDS_OFF = 32
TEXT_POS_OFF = 56       # x, y; position_type sits at 68
IMAGE_STRUCT = 108      # name[30] + file[30] + 4 pos ints + copy area + delay
CLEAR_STRUCT = 44       # name[30] + 7 * Uint16
PAR_STRUCT = 552        # name[30] pad + 5 ints + 10*(30 + 4*4) + 4*40
PAR_NAME_OFF = 52
PAR_SPEED_OFF = 352
PAR_ADJY_OFF = 392
PAR_ADJH_OFF = 432
MAX_LAYERS = 10
PLANE_W = 512           # a BG plane is 64 tiles wide

SCENE_TEXT_LINES = 6
SCENE_TEXT_RECORDS = 19  # text/0.txt .. text/18.txt

# format_v4::e_SCENETYPE
OP_NAMES = ["CLEAR_SCREEN", "CLEAR_AREA", "MOVE_IMAGE", "MOVE_VIEWPOINT",
            "SHOW_ANIMATION", "PLAY_SFX", "PLAY_MUSIC", "STOP_MUSIC",
            "SHOW_TEXT", "SUBSCENE", "PARALLAX", "COUNT"]


def cstr(raw):
    return raw.split(b"\0")[0].decode("latin1")


def read(name, size):
    path = os.path.join(DATA, name)
    data = open(path, "rb").read()
    if len(data) % size:
        sys.exit(f"{name}: size {len(data)} is not a multiple of {size}")
    return [data[i:i + size] for i in range(0, len(data), size)]


# Ops src/scene.c implements. MOVE_VIEWPOINT / SHOW_ANIMATION / SUBSCENE are
# declared by the PC format but no scene in this data plays them except the
# LAB opening (scene 1), which the port does not run; they are listed so a
# future data export fails loudly instead of being skipped silently.
PLAYABLE = {"CLEAR_SCREEN", "CLEAR_AREA", "MOVE_IMAGE", "SHOW_TEXT",
            "PARALLAX", "PLAY_MUSIC", "STOP_MUSIC", "PLAY_SFX"}
KNOWN_UNUSED = {"MOVE_VIEWPOINT", "SHOW_ANIMATION", "SUBSCENE"}
PLAYED_SCENES = {0, 2, 3, 4, 5}       # SCENE_OPENING_UNBEATEN + the endings


def report_ops(scenes):
    for i, (name, ops) in enumerate(scenes):
        for (typ, seek, delay, rt, rv, bg) in ops:
            op = OP_NAMES[typ] if typ < len(OP_NAMES) else f"op{typ}"
            if op in PLAYABLE:
                continue
            if i in PLAYED_SCENES:
                sys.exit(f"scene {i} ({name}) uses {op}, which src/scene.c "
                         "does not implement - implement it or drop the scene")
            if op not in KNOWN_UNUSED:
                sys.exit(f"scene {i} ({name}) uses unknown op {op}")


def read_scenes():
    out = []
    for i, r in enumerate(read("scenes_list.dat", SCENE_STRUCT)):
        name = cstr(r[:30])
        ops = []
        for j in range(SCENE_OBJ_N):
            b = SCENE_OBJ_OFF + j * 24
            typ = r[b]
            seek, delay, repeat_type, repeat_val = struct.unpack_from("<iiii", r, b + 4)
            bg = r[b + 20]
            if seek < 0:
                continue
            if seek > 255:
                sys.exit(f"scene {i}: seek {seek} does not fit u8")
            ops.append((typ, seek, delay, repeat_type, repeat_val, bg))
        out.append((name, ops))
    return out


def read_texts():
    out = []
    for i, r in enumerate(read("scenes_show_text.dat", TEXT_STRUCT)):
        name = cstr(r[:30])
        x, y = struct.unpack_from("<ii", r, TEXT_POS_OFF)
        # transition_type is at 64, position_type at 68 (the first parse read
        # transition by mistake: every intro text is text_position_type_centered)
        position, = struct.unpack_from("<i", r, TEXT_POS_OFF + 12)
        out.append({"name": name, "position": position, "x": x, "y": y})
    return out


def read_clears():
    out = []
    for r in read("scenes_clear_area.dat", CLEAR_STRUCT):
        x, y, w, h, red, g, b = struct.unpack_from("<7H", r, 30)
        out.append({"name": cstr(r[:30]), "x": x, "y": y, "w": w, "h": h,
                    "r": red, "g": g, "b": b})
    return out


def read_images():
    out = []
    for r in read("scenes_show_image.dat", IMAGE_STRUCT):
        name = cstr(r[:30])
        fn = cstr(r[30:60])
        ini_x, ini_y, dx, dy = struct.unpack_from("<4i", r, 60)
        cx, cy, cw, ch = struct.unpack_from("<4i", r, 76)
        move_delay = r[92]
        blocking = r[96]
        loop, move_type = struct.unpack_from("<2i", r, 100)
        out.append({"name": name, "file": fn,
                    "x": ini_x, "y": ini_y, "dx": dx, "dy": dy,
                    "cx": cx, "cy": cy, "cw": cw, "ch": ch,
                    "delay": move_delay, "blocking": blocking,
                    "loop": loop, "move_type": move_type})
    return out


# PC scenePlayMusic records: name[30] + filename[30]. The port plays its own
# converted tracks, so the filename maps to a MUSIC_* id here (music.c is the
# table id -> resource). A record with no mapping plays nothing (and is
# reported at convert time).
SCENE_MUSIC_MAP = {
    "intro_scene.mod": "MUSIC_INTRO_SCENE",
    "daisie.mod": "MUSIC_STAGE2",            # DAISIE BOT's stage music
    "rockbot_end_animation.mod": "MUSIC_END_ANIMATION",
}


def read_play_music():
    out = []
    for r in read("scenes_play_music.dat", 60):
        out.append((cstr(r[:30]), cstr(r[30:60])))
    return out


def read_parallax():
    out = []
    for r in read("scenes_parallax.dat", PAR_STRUCT):
        frame_delay, loop, direction, duration, tbd = struct.unpack_from("<5i", r, 32)
        names = [cstr(r[PAR_NAME_OFF + i * 30:PAR_NAME_OFF + (i + 1) * 30])
                 for i in range(MAX_LAYERS)]
        speed = struct.unpack_from("<10i", r, PAR_SPEED_OFF)
        adj_y = struct.unpack_from("<10i", r, PAR_ADJY_OFF)
        adj_h = struct.unpack_from("<10i", r, PAR_ADJH_OFF)
        layers = [{"file": names[i], "speed": speed[i], "adjust_y": adj_y[i],
                   "adjust_h": adj_h[i]} for i in range(MAX_LAYERS) if names[i]]
        out.append({"name": cstr(r[:30]), "frame_delay": frame_delay,
                    "direction": direction, "duration": duration,
                    "layers": layers})
    return out


def least_period(im):
    """Smallest horizontal period, or None. Baked so the plane wrap is
    seamless: a 384 px layer whose art repeats every 256 px must be tiled by
    256, not by 384 (the seam would jump 128 px)."""
    w, h = im.size
    data = im.tobytes()
    stride = w * 4
    first = data[:stride]
    for p in range(8, w):
        if first[p * 4:] != first[:stride - p * 4]:
            continue
        if all(data[y * stride + p * 4:(y + 1) * stride] ==
               data[y * stride:(y + 1) * stride - p * 4] for y in range(h)):
            return p
    return None


def bake(src_name, out_name):
    im = Image.open(os.path.join(IMAGES, src_name)).convert("RGBA")
    h = (im.height + 7) & ~7
    out = Image.new("RGBA", (PLANE_W, h), (0, 0, 0, 255))
    period = least_period(im) if im.width <= PLANE_W else None
    if period:
        for x in range(0, PLANE_W, period):
            out.paste(im.crop((0, 0, min(period, PLANE_W - x), im.height)), (x, 0))
        note = f"period {period}px"
    else:
        out.paste(im.crop((0, 0, min(PLANE_W, im.width), im.height)), (0, 0))
        note = "cropped to plane width"
    # img2tiles punches the game colorkey (city_bg_3's sky) to index 0, so the
    # layer below shows through exactly as on the PC.
    out.save(os.path.join(RES, out_name + ".png"))
    print(f"  {out_name}: {src_name} {im.size} -> {out.size} ({note})")


def md_color(r, g, b):
    """PC 0..255 -> SGDK PAL_setColor's 0x0BGR (4 bits per channel)."""
    return ((b >> 4) << 8) | ((g >> 4) << 4) | (r >> 4)


def emit_c(scenes, texts, clears, pars, images, baked, music):
    h = ["#ifndef SCENES_DATA_H", "#define SCENES_DATA_H", "",
         "#include <genesis.h>", "",
         "// Generated by tools/convert_scenes.py from the PC scene tables.",
         "// Do not edit by hand.", "",
         "#define SCENE_TEXT_LINES %d" % SCENE_TEXT_LINES,
         "#define SCENE_TEXT_RECORDS %d" % SCENE_TEXT_RECORDS,
         "",
         "// format_v4::e_SCENETYPE",
         "enum {",
         "    SCENE_CLEAR_SCREEN, SCENE_CLEAR_AREA, SCENE_MOVE_IMAGE,",
         "    SCENE_MOVE_VIEWPOINT, SCENE_SHOW_ANIMATION, SCENE_PLAY_SFX,",
         "    SCENE_PLAY_MUSIC, SCENE_STOP_MUSIC, SCENE_SHOW_TEXT,",
         "    SCENE_SUBSCENE, SCENE_PARALLAX",
         "};",
         "",
         "enum {",
         "    SCENE_TEXT_DIALOG_BOTTOM, SCENE_TEXT_DIALOG_TOP, SCENE_TEXT_CENTERED,",
         "    SCENE_TEXT_CENTER_X, SCENE_TEXT_CENTER_Y, SCENE_TEXT_USER",
         "};",
         "",
         "typedef struct { u8 type; u8 seek; u16 delay; } SceneOp;",
         "typedef struct { const SceneOp *ops; u8 count; } SceneDef;",
         "",
         "typedef struct {",
         "    const u32 *tiles; u16 tiles_n;",
         "    const u16 *map; u8 map_w, map_h;",
         "    const Palette *pal;",
         "    s16 adjust_y; u8 speed;",
         "} SceneLayer;",
         "",
         "typedef struct {",
         "    const SceneLayer *layers; u8 count;",
         "    u16 frame_delay, duration; u8 direction;",
         "} SceneParallax;",
         "",
         "typedef struct { s16 x, y, w, h; u16 color; } SceneClear;",
         "typedef struct { s16 x, y; u8 position; } SceneTextPos;",
         "",
         "// MOVE_IMAGE record: art pointers are NULL when that image was not",
         "// converted (lab scene / unused records).",
         "typedef struct {",
         "    s16 x, y, dest_x, dest_y;",
         "    s16 copy_x, copy_y, copy_w, copy_h;",
         "    u16 move_delay;",
         "    const u32 *tiles; u16 tiles_n;",
         "    const u16 *map; u8 map_w, map_h;",
         "    const Palette *pal;",
         "} SceneImage;",
         "",
         "extern const SceneDef scene_defs[%d];" % len(scenes),
         "extern const SceneParallax scene_parallax[%d];" % max(1, len(pars)),
         "extern const SceneClear scene_clear[%d];" % max(1, len(clears)),
         "extern const SceneImage scene_images[%d];" % max(1, len(images)),
         "#define SCENE_MUSIC_COUNT %d" % max(1, len(music)),
         "extern const u8 scene_music_ids[SCENE_MUSIC_COUNT];",
         "extern const SceneTextPos scene_text_pos[SCENE_TEXT_RECORDS];",
         "extern const char *const scene_text[SCENE_TEXT_RECORDS][SCENE_TEXT_LINES];",
         "",
         "#endif"]

    c = ['#include "scenes_data.h"', '#include "resources.h"',
         '#include "music.h"']
    for i, p in enumerate(pars):
        for j in range(len(p["layers"])):
            c.append(f'#include "scene_par{i}_l{j}_tiles.h"')
            c.append(f'#include "scene_par{i}_l{j}_map.h"')
    for i in sorted(baked):
        c.append(f'#include "scene_img{i}_tiles.h"')
        c.append(f'#include "scene_img{i}_map.h"')
    c.append("")

    for i, (name, ops) in enumerate(scenes):
        c.append(f"// {name}")
        c.append(f"static const SceneOp scene{i}_ops[] = {{")
        for (typ, seek, delay, rt, rv, bg) in ops:
            c.append(f"    {{ {typ}, {seek}, {delay} }}, "
                     f"// {OP_NAMES[typ]}")
        c.append("};")
        c.append("")

    c.append("const SceneDef scene_defs[%d] = {" % len(scenes))
    for i, (name, ops) in enumerate(scenes):
        if ops:
            c.append(f'    {{ scene{i}_ops, {len(ops)} }}, // {name}')
        else:
            c.append(f'    {{ NULL, 0 }}, // {name}')
    c.append("};")
    c.append("")

    c.append("// PLAY_MUSIC op seek -> MUSIC_* id (music.c owns id -> resource).")
    if music:
        c.append(f"const u8 scene_music_ids[{len(music)}] = {{")
        for name, fn in music:
            mid = SCENE_MUSIC_MAP.get(fn.lower())
            if mid is None:
                print(f"  !! scene music '{name}' ({fn}) has no MUSIC_* mapping")
                mid = "MUSIC_NONE"
            c.append(f"    {mid}, // {name} = {fn}")
        c.append("};")
    else:
        c.append("const u8 scene_music_ids[1] = { MUSIC_NONE };")
    c.append("")

    for i, p in enumerate(pars):
        if len(p["layers"]) > 2:
            sys.exit(f"parallax {i}: {len(p['layers'])} layers, the port has "
                     "BG_A and BG_B only")
        c.append(f"// {p['name']}: {len(p['layers'])} layers, "
                 f"{p['frame_delay']}ms/frame, {p['duration']}ms")
        c.append(f"static const SceneLayer scene_par{i}_layers[] = {{")
        for j, l in enumerate(p["layers"]):
            up = f"scene_par{i}_l{j}".upper()
            c.append(f"    {{ scene_par{i}_l{j}_tiles_data, {up}_TILE_COUNT,")
            c.append(f"      scene_par{i}_l{j}_map, {up}_MAP_W, {up}_MAP_H,")
            c.append(f"      &scene_par{i}_l{j}_pal, {l['adjust_y']}, {l['speed']} }},")
        c.append("};")
        c.append(f"const SceneParallax scene_parallax[{max(1, len(pars))}] = {{")
        c.append(f"    {{ scene_par{i}_layers, {len(p['layers'])}, "
                 f"{p['frame_delay']}, {p['duration']}, {p['direction']} }},")
        c.append("};")
    if not pars:
        c.append("const SceneParallax scene_parallax[1];")
    c.append("")

    c.append(f"const SceneClear scene_clear[{max(1, len(clears))}] = {{")
    for cl in clears:
        c.append(f"    {{ {cl['x']}, {cl['y']}, {cl['w']}, {cl['h']}, "
                 f"0x{md_color(cl['r'], cl['g'], cl['b']):03X} }}, // {cl['name']}")
    c.append("};")
    c.append("")

    c.append(f"const SceneImage scene_images[{max(1, len(images))}] = {{")
    for i, im in enumerate(images):
        art = (f"scene_img{i}_tiles_data, SCENE_IMG{i}_TILE_COUNT, "
               f"scene_img{i}_map, SCENE_IMG{i}_MAP_W, SCENE_IMG{i}_MAP_H, "
               f"&scene_img{i}_pal") if i in baked else \
              "NULL, 0, NULL, 0, 0, NULL"
        c.append(f"    {{ {im['x']}, {im['y']}, {im['dx']}, {im['dy']}, "
                 f"{im['cx']}, {im['cy']}, {im['cw']}, {im['ch']}, "
                 f"{im['delay']}, {art} }},")
    c.append("};")
    c.append("")

    c.append(f"const SceneTextPos scene_text_pos[SCENE_TEXT_RECORDS] = {{")
    for t in texts:
        c.append(f"    {{ {t['x']}, {t['y']}, {t['position']} }}, // {t['name']}")
    c.append("};")
    c.append("")

    c.append("const char *const scene_text[SCENE_TEXT_RECORDS]"
             "[SCENE_TEXT_LINES] = {")
    for n in range(SCENE_TEXT_RECORDS):
        # The PC reads scenes/text/<language>/<n>.txt (fio_strings.cpp); the
        # files directly in scenes/text/ are stale leftovers from an older
        # script and are NOT what the game shows. Use the English set, which
        # is also the PC's fallback language.
        path = os.path.join(DATA, "text", "en", f"{n}.txt")
        if not os.path.exists(path):
            path = os.path.join(DATA, "text", f"{n}.txt")
        lines = open(path, "rb").read().decode("latin1").splitlines()
        if len(lines) > SCENE_TEXT_LINES:
            sys.exit(f"text/{n}.txt has {len(lines)} lines (max "
                     f"{SCENE_TEXT_LINES})")
        lines += [""] * (SCENE_TEXT_LINES - len(lines))
        out = []
        for line in lines:
            if any(ord(ch) < 32 or ord(ch) > 126 for ch in line):
                sys.exit(f"text/{n}.txt has a character outside the SGDK font: "
                         f"{line!r}")
            out.append('"' + line.replace("\\", "\\\\").replace('"', '\\"') + '"')
        c.append("    { " + ", ".join(out) + " },")
    c.append("};")

    open(os.path.join(SRC, "scenes_data.h"), "w").write("\n".join(h) + "\n")
    open(os.path.join(SRC, "scenes_data.c"), "w").write("\n".join(c) + "\n")
    print("wrote src/scenes_data.h/.c")


def main():
    scenes = read_scenes()
    report_ops(scenes)
    texts = read_texts()
    clears = read_clears()
    pars = read_parallax()

    os.makedirs(RES, exist_ok=True)
    os.makedirs(SRC, exist_ok=True)

    for i, p in enumerate(pars):
        for j, l in enumerate(p["layers"]):
            bake(l["file"], f"scene_par{i}_l{j}")

    images = read_images()
    # Only the portraits the endings (scenes 2..5) use. The other records are
    # the lab scene / unused; their art still needs a colour-reduction pass.
    baked = {6, 8, 9, 10, 11, 12}
    for i in sorted(baked):
        im = Image.open(os.path.join(IMAGES, images[i]["file"])).convert("RGBA")
        # Portrait backgrounds are the PC clear-area colour, fully opaque, so
        # no colorkey/alpha handling is needed.
        im.save(os.path.join(RES, f"scene_img{i}.png"))
        print(f"  scene_img{i}: {images[i]['file']} {im.size}")

    emit_c(scenes, texts, clears, pars, images, baked, read_play_music())

    print("\n--- scene table dump (verify offline) ---")
    for i, (name, ops) in enumerate(scenes):
        print(f"scene {i}: {name} ({len(ops)} ops)")
        for (typ, seek, delay, rt, rv, bg) in ops:
            extra = ""
            if typ == 8:  # SHOW_TEXT
                extra = f" lines={texts[seek]['name']!r}"
            print(f"  {OP_NAMES[typ]:16s} seek={seek:<3d} delay={delay:<5d}{extra}")
    for i, p in enumerate(pars):
        print(f"parallax {i}: {p['name']} {p['duration']}ms @ "
              f"{p['frame_delay']}ms/frame, dir {p['direction']}")
        for j, l in enumerate(p["layers"]):
            print(f"  layer {j}: {l['file']} y={l['adjust_y']} "
                  f"speed={l['speed']}")
    print(f"clear areas: {[(c['name'], c['x'], c['y'], c['w'], c['h']) for c in clears]}")
    print(f"text records: {SCENE_TEXT_RECORDS}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
