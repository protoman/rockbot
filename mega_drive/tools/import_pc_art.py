"""One-shot: import every PC image into images/, mirroring the source tree.

The point: from now on hand edits live in images/, NOT in the PC port, and the
piece converters read images/. That's why the copies are pre-indexed here to
the MD palette the art actually uses at runtime -- you open them in GIMP,
pick the matching .gpl, and edit colours by index.

Rules:
  * original size is kept (240p art stays 240p; the converters do the 224 crop)
  * index 0 is the transparent/backdrop slot (alpha, a P-mode tRNS index, or
    the game colorkey with a small per-channel tolerance)
  * palette per file:
      sprites/enemies, sprites/objects, projectiles, animations,
      sprites/*.png (enemy sheets), faces, tilesets/{explosions,anim sprites}
                                                        -> PAL0  res/md_pal0.gpl
      sprites/p1..p4, ending, player faces/select/armor  -> PAL2  res/md_pal2.gpl
        (p1..p4 keep the green/magenta palette-swap placeholders on indices
         14/15; converters swap the weapon colour by index)
      tilesets/**                                       -> PAL1  res/md_pal1.gpl
      map_backgrounds/city_night.png                    -> BG0   res/md_bg0.gpl
      map_backgrounds/stars_bg.png, stars_fullsize.png  -> BG1   res/md_bg1.gpl
      logo.png                                          -> TITLE res/md_title.gpl
      everything else (screens, scenes, other bgs)      -> 16 colours derived
         from the image itself (each of those owns a palette in the port),
         and a matching <name>.gpl is written next to it.

Matching uses CIELAB distance, so a purple that is close to a palette purple
is not stolen by a numerically-closer grey.

Usage: python3 tools/import_pc_art.py
"""

import os
from PIL import Image
from collections import Counter

HERE = os.path.dirname(os.path.abspath(__file__))
PROJ = os.path.dirname(HERE)
PCROOT = os.path.dirname(PROJ)
SRC = os.path.join(PCROOT, "build", "games", "RockDroid1", "images")
SHARED = os.path.join(PCROOT, "build", "shared", "images")
OUT = os.path.join(PROJ, "images")
RES = os.path.join(PROJ, "res")

COLORKEY = (75, 125, 125)      # the only game colorkey (defines.h)
# Some art paints its backdrop with the colorkey off by a few units: the
# title logo.png uses #487e7c (72,126,124). Accept a small per-channel
# tolerance so those become transparent too instead of surviving as a grey
# slab (the nearest palette entry). <=6 only catches the intended cluster
# (logo/death_animation/palmtree anim tiles) plus isolated 1px fringe.
COLORKEY_TOLERANCE = 6
CONFIG_BG = (5, 29, 65)        # PC CONFIG_BGCOLOR: what the title fills its
                               # transparent background with at runtime
SCREEN_DIRS = ("backgrounds", "map_backgrounds", "scenes")   # opaque screens
SCREEN_FILL = {"logo.png": CONFIG_BG}
SKIP_NAMES = ("license.txt",)
SKIP_PREFIX = ("icon", "icons")  # android/app icons, not MD art


# ---------------------------------------------------------------- palettes
def load_gpl(path):
    """GIMP palette -> list of (r, g, b) in index order."""
    cols = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line[0] == "#" or line.startswith(("GIMP", "Name:", "Columns:")):
                continue
            p = line.split()
            if len(p) >= 3:
                cols.append(tuple(int(v) for v in p[:3]))
    return cols


PAL0 = load_gpl(os.path.join(RES, "md_pal0.gpl"))
PAL1 = load_gpl(os.path.join(RES, "md_pal1.gpl"))
PAL2 = load_gpl(os.path.join(RES, "md_pal2.gpl"))
BG0 = load_gpl(os.path.join(RES, "md_bg0.gpl"))
BG1 = load_gpl(os.path.join(RES, "md_bg1.gpl"))
TITLE = load_gpl(os.path.join(RES, "md_title.gpl"))

# Player sheets p1..p4 carry two palette-swap placeholders (green/magenta).
# They keep their own indices in the hand art so converters can swap the
# weapon colour BY INDEX instead of looking for exact RGB values in quantized
# output (which destroyed them).
PLACEHOLDER_COLOR1 = (55, 255, 0)      # classplayer.cpp COLORKEY1
PLACEHOLDER_COLOR2 = (255, 0, 255)     # classplayer.cpp COLORKEY2
PLACEHOLDER_COLOR3 = (0, 255, 255)     # classplayer.cpp COLORKEY3
# player sheets keep the three swap placeholders on indices 13/14/15; the
# body palette is derived from each player's own art (players have different
# colours, so a single shared PAL2 was wrong).
PLAYER_PLACEHOLDERS = (PLACEHOLDER_COLOR1, PLACEHOLDER_COLOR2, PLACEHOLDER_COLOR3)
PLAYER_OVERRIDES = {PLACEHOLDER_COLOR1: 13, PLACEHOLDER_COLOR2: 14,
                    PLACEHOLDER_COLOR3: 15}
FACE_PAL = {}                          # player n -> face palette (eyes reuse it)

# ---------------------------------------------------------------- colour
MD_LEVELS = (0, 36, 73, 109, 146, 182, 219, 255)


def md_round(v):
    return min(MD_LEVELS, key=lambda x: abs(x - v))


def _lab_dist(a, b):
    la, lb = lab(a), lab(b)
    return ((la[0] - lb[0]) ** 2 + (la[1] - lb[1]) ** 2
            + (la[2] - lb[2]) ** 2) ** 0.5


def _srgb_lab(rgb):
    def f(v):
        v /= 255.0
        return v / 12.92 if v <= 0.04045 else ((v + 0.055) / 1.055) ** 2.4
    r, g, b = (f(v) for v in rgb)
    x = (r * 0.4124 + g * 0.3576 + b * 0.1805) / 0.95047
    y = (r * 0.2126 + g * 0.7152 + b * 0.0722) / 1.00000
    z = (r * 0.0193 + g * 0.1192 + b * 0.9505) / 1.08883

    def g2(t):
        return t ** (1.0 / 3.0) if t > 0.008856 else 7.787 * t + 16.0 / 116.0
    fx, fy, fz = g2(x), g2(y), g2(z)
    return (116 * fy - 16, 500 * (fx - fy), 200 * (fy - fz))


_CACHE = {}


def lab(c):
    v = _CACHE.get(c)
    if v is None:
        v = _CACHE[c] = _srgb_lab(c)
    return v


def nearest(c, palette, first=1, last=None):
    """Index of the closest palette entry (never 0 unless the palette is 1 long).
    last: highest index to consider (exclusive bound is last+1)."""
    lc = lab(c)
    best, bi = 1e18, first
    for i in range(first, (len(palette) if last is None else last + 1)):
        p = palette[i]
        lp = lab(p)
        d = (lc[0] - lp[0]) ** 2 + (lc[1] - lp[1]) ** 2 + (lc[2] - lp[2]) ** 2
        if d < best:
            best, bi = d, i
    return bi


def auto_palette(im, size=15, merge_t=14.0, exclude=()):
    """`size` colours for a screen image that owns its palette.

    MEDIANCUT alone splits by pixel population and happily returns several
    near-identical colours (e.g. 5,6,7,9 of player_select_p3 all ~dLAB 7-13),
    wasting slots and flattening the art. Here MEDIANCUT only SEEDS the
    palette; entries closer than `merge_t` (CIELAB) are merged, and the freed
    slots are refilled with the image's most frequent colours that are still
    `merge_t` apart, so the palette stays representative AND distinct. The
    threshold relaxes only if the image cannot supply `size` distinct colours.
    """
    skip = {COLORKEY} | set(exclude)
    px = [p[:3] for p in im.convert("RGB").getdata() if p[:3] not in skip]
    if not px:
        return [(0, 0, 0)] * size

    tmp = Image.new("RGB", (len(px), 1))
    tmp.putdata(px)
    q = tmp.quantize(colors=size, method=Image.MEDIANCUT)
    qp = q.getpalette()[:size * 3]
    qp = qp + [0] * (size * 3 - len(qp))
    seed = [(qp[i * 3], qp[i * 3 + 1], qp[i * 3 + 2]) for i in range(size)]

    kept = []
    for c in seed:                      # merge near-duplicates
        if all(_lab_dist(c, k) >= merge_t for k in kept):
            kept.append(c)

    hist = Counter((md_round(p[0]), md_round(p[1]), md_round(p[2])) for p in px)
    ranked = [c for c, _ in hist.most_common()]
    for t in (merge_t, merge_t * 0.6, merge_t * 0.3, 0.0):
        if len(kept) >= size:
            break
        for c in ranked:                # refill with distinct colours
            if len(kept) >= size:
                break
            if c in kept:
                continue
            if all(_lab_dist(c, k) >= t for k in kept):
                kept.append(c)
    while len(kept) < size:
        kept.append((0, 0, 0))
    return kept[:size]


def write_gpl(path, name, cols):
    with open(path, "w") as f:
        f.write("GIMP Palette\nName: %s\nColumns: 4\n#\n" % name)
        for i, c in enumerate(cols):
            f.write("%3d %3d %3d\t%d\n" % (c[0], c[1], c[2], i))


# ---------------------------------------------------------------- rules
def palette_for(rel):
    """(palette or None, label, exact-index overrides); None -> derive."""
    parts = rel.split("/")
    d = parts[0]
    base = parts[-1].lower()
    if rel == "logo.png":
        return TITLE, "PAL_TITLE", None
    if d == "tilesets":
        return PAL1, "PAL1", None
    if d == "projectiles" or d == "animations":
        return PAL0, "PAL0", None
    if d in ("sprites", "faces"):
        if d == "sprites" and base.startswith("p") and base[1:2] in "1234":
            return None, "PLAYER_SHEET", None      # handled in main()
        if d == "faces" and base[:2] in ("p1", "p2", "p3", "p4"):
            return None, "PLAYER_FACE", None       # handled in main()
        return PAL0, "PAL0", None
    if d == "ending":
        # ending art is a SCENE (own palette); if it ever composites the
        # in-game player sprite, that sprite stays PAL2 separately.
        return None, "auto16", None
    if d == "map_backgrounds":
        if base == "city_night.png":
            return BG0, "BG0", None
        if base.startswith("stars"):
            return BG1, "BG1", None
    # player_* SCREENS (player_select_pN, player_armor_pieces,
    # player_faces_front) are full-screen art with their own palette, not the
    # player sprite: forcing PAL2 wasted most entries on duplicated blacks
    # and left the art with ~9 colours. Give them an own 15-colour palette.
    # (sprites/p1..p4 and faces/p1..p4 stay PAL2 above.)
    return None, "auto16", None


def border_colour(im):
    """Opaque colour of the 1px border ring (the art's own backdrop)."""
    rgba = im.convert("RGBA")
    w, h = rgba.size
    px = rgba.load()
    ring = [(x, y) for x in range(w) for y in (0, h - 1)]
    ring += [(x, y) for y in range(h) for x in (0, w - 1)]
    cnt = Counter()
    for x, y in ring:
        r, g, b, a = px[x, y]
        if a >= 128 and (r, g, b) != COLORKEY:
            cnt[(r, g, b)] += 1
    if cnt:
        return cnt.most_common(1)[0][0]
    cnt = Counter(c for c in rgba.convert("RGB").getdata() if c != COLORKEY)
    return cnt.most_common(1)[0][0] if cnt else (0, 0, 0)


def is_colorkey(c):
    """Exact game colorkey plus a small per-channel tolerance for art whose
    backdrop is the colorkey a few units off (see COLORKEY_TOLERANCE)."""
    return (abs(c[0] - COLORKEY[0]) <= COLORKEY_TOLERANCE
            and abs(c[1] - COLORKEY[1]) <= COLORKEY_TOLERANCE
            and abs(c[2] - COLORKEY[2]) <= COLORKEY_TOLERANCE)


def convert(im, palette, fill=None, overrides=None, last=None):
    """-> P image. fill=None: index 0 = transparent. fill=RGB: opaque backdrop
    (screen art -- a plane's index 0 is the backdrop, not transparency).
    overrides: exact source RGB -> palette index, applied before nearest-match
    (keeps the player swap placeholders on their dedicated indices)."""
    alpha = None
    if im.mode in ("RGBA", "LA"):
        alpha = im.convert("RGBA").getchannel("A")
    elif im.mode == "P" and "transparency" in im.info:
        # Indexed source with a tRNS transparency index (NOT the RGB colorkey):
        # e.g. backgrounds/menu.png marks palette index 0 (= grey 97,97,97) as
        # transparent. Without this the transparent pixels survived as grey
        # (their palette colour), because only the exact colorkey RGB was
        # punched.
        t = im.info["transparency"]
        trans = {t} if isinstance(t, int) else set(t)
        alpha = Image.new("L", im.size)
        alpha.putdata([0 if v in trans else 255 for v in im.getdata()])
    rgb = im.convert("RGB")
    src = list(rgb.getdata())
    overrides = overrides or {}
    if alpha is not None:
        a = list(alpha.getdata())
        if fill is None:
            src = [COLORKEY if a[i] < 128 else src[i] for i in range(len(src))]
        else:
            src = [fill if a[i] < 128 else src[i] for i in range(len(src))]
    out = Image.new("P", im.size)
    out.putpalette(sum([list(c) for c in palette], []) + [0] * (768 - 3 * len(palette)))
    # reserved indices are not art colours: PAL0 index 15 is the font white
    # (text only); player sheets pass last=12 (13/14/15 are placeholders).
    if last is None and palette is PAL0:
        last = 14
    idx = []
    lut = {}
    for c in src:
        if is_colorkey(c):
            idx.append(0)
            continue
        i = overrides.get(c)
        if i is None:
            i = lut.get(c)
        if i is None:
            i = lut[c] = nearest(c, palette, last=last)
        idx.append(i)
    out.putdata(idx)
    if fill is None:
        out.info["transparency"] = 0
    return out


def main():
    n, per, skipped, auto = 0, {}, [], []
    for dp, dn, fn in os.walk(SRC):
        for f in sorted(fn):
            rel = os.path.relpath(os.path.join(dp, f), SRC).replace("\\", "/")
            base = os.path.basename(rel).lower()
            if base in SKIP_NAMES or base.startswith(SKIP_PREFIX) or base.endswith(".xcf"):
                skipped.append(rel)
                continue
            try:
                im = Image.open(os.path.join(dp, f))
            except Exception:
                skipped.append(rel)
                continue
            palette, label, overrides = palette_for(rel)
            last = None
            if label == "PLAYER_SHEET":
                body = auto_palette(im, 13, exclude=PLAYER_PLACEHOLDERS)
                palette = list(body) + list(PLAYER_PLACEHOLDERS)
                overrides = PLAYER_OVERRIDES
                last = 12
            elif label == "PLAYER_FACE":
                pn = base[1:2]
                if base.endswith("_eyes.png"):
                    palette = FACE_PAL.get(pn) or auto_palette(im, 15)
                else:
                    palette = auto_palette(im, 15)
                    FACE_PAL[pn] = palette
            elif palette is None:
                palette = auto_palette(im)
                auto.append(rel)
                label = "auto16"
            if rel == "logo.png":
                # index 0 is the transparent/backdrop slot; on a plane it
                # renders as this entry, so make it the title's config blue
                palette = list(palette)
                palette[0] = CONFIG_BG
            dst = os.path.join(OUT, rel)
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            # Screen art keeps index 0 transparent: on a plane index 0 shows
            # the backdrop, which is exactly how the port draws them (the
            # title's dark blue IS the backdrop colour, not art).
            convert(im, palette, overrides=overrides, last=last).save(dst)
            if label == "auto16" or label in ("PLAYER_SHEET", "PLAYER_FACE"):
                write_gpl(dst[:-4] + ".gpl", rel, palette)
            per[label] = per.get(label, 0) + 1
            n += 1
    # shared tree (intro logos, HUD) -> images/shared, own tree preserved
    if os.path.isdir(SHARED):
        for dp, dn, fn in os.walk(SHARED):
            for f in sorted(fn):
                if not f.lower().endswith(".png"):
                    continue
                rel = os.path.relpath(os.path.join(dp, f), SHARED)
                try:
                    im = Image.open(os.path.join(dp, f))
                except Exception:
                    continue
                # HUD balls are PAL0 in the port (tools/pal_ui.py); the rest
                # (intro logos, cutscene art) own their palette.
                palette, label = (PAL0, "PAL0") if "ball" in f.lower() else (auto_palette(im), "auto16")
                dst = os.path.join(OUT, "shared", rel)
                os.makedirs(os.path.dirname(dst), exist_ok=True)
                convert(im, palette).save(dst)
                if label == "auto16":
                    write_gpl(dst[:-4] + ".gpl", "shared/" + rel, palette)
                per[label] = per.get(label, 0) + 1
                n += 1
    lines = ["# images/ -- hand-owned MD art",
             "",
             "Imported once from the PC port by tools/import_pc_art.py. Original",
             "resolution kept (240p stays 240p -- the converters crop to 224).",
             "Index 0 is the transparent/backdrop slot.",
             "",
             "| palette used | files | GIMP palette to load |",
             "|---|---|---|",
             "| PAL0 enemies/objects/shots/anim/HUD | %d | res/md_pal0.gpl |" % per.get("PAL0", 0),
             "| PAL1 stage tileset | %d | res/md_pal1.gpl |" % per.get("PAL1", 0),
             "| PAL2 player | %d | res/md_pal2.gpl (p1-p4: idx 14=green, 15=magenta placeholders) |" % per.get("PAL2", 0),
             "| BG0 city night | %d | res/md_bg0.gpl |" % per.get("BG0", 0),
             "| BG1 stars | %d | res/md_bg1.gpl |" % per.get("BG1", 0),
             "| PAL_TITLE logo | %d | res/md_title.gpl |" % per.get("PAL_TITLE", 0),
             "| own 16 colours (screens) | %d | <file>.gpl beside it |" % per.get("auto16", 0),
             "",
             "Total: %d files. Skipped (not MD art): %s" % (n, ", ".join(skipped) or "none"),
             ""]
    with open(os.path.join(OUT, "PALETTES.md"), "w") as f:
        f.write("\n".join(lines))
    print("imported %d files -> images/" % n)
    for k in sorted(per):
        print("   %-10s %d" % (k, per[k]))
    print("   skipped:", ", ".join(skipped) or "none")
    print("   auto-palette files (own .gpl written):", ", ".join(auto) or "none")


if __name__ == "__main__":
    main()
