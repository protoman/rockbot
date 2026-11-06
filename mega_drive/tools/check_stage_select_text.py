#!/usr/bin/env python3
"""Offline check: the stage-select boss/stage names are drawn on a black band.

BUG it reproduces (2026-10-30, BUG-014 came back): after a stage, the name
lines were sometimes unreadable again. The names used the PC bottom-bar art as
their background and borrowed PAL1 (the grey ramp) for the font, so both the
cell colour and the palette depended on what the finished stage left in CRAM /
video memory.

The fix is the shape this check enforces:
  * the text palette is PAL0 = pal_ui, loaded with the CPU-safe loader
    (pal_safe.h) instead of being borrowed from a stage palette;
  * the band behind the names is painted with font spaces in palette 0
    (VDP_clearTextArea), i.e. solid black, before any name is drawn;
  * every row a name is written to lies inside that cleared band;
  * pal_ui entry 0 is black and entry 15 is the ink (the font's two colours).

Usage: tools/check_stage_select_text.py     exit 1 on a violation.
"""
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
SRC = os.path.join(ROOT, "src")


def read(path):
    with open(path, encoding="utf-8") as fp:
        return fp.read()


def function_body(text, name):
    m = re.search(r"^static\s+[\w \*]*\b%s\s*\([^;]*?\)\s*\{" % name, text,
                  re.M)
    if not m:
        return None
    depth, i = 0, m.end() - 1
    while i < len(text):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[m.end():i]
        i += 1
    return None


def const(text, name):
    m = re.search(r"#define\s+%s\s+(-?\d+)" % name, text)
    return int(m.group(1)) if m else None


def main():
    errors = []
    src = read(os.path.join(SRC, "stageselect.c"))
    body = function_body(src, "draw_names")
    if body is None:
        print("FAIL  stageselect.c: no draw_names() found")
        return 1

    if not re.search(r"pal_load\(\s*PAL0\s*,\s*pal_ui\.data\s*\)", body):
        errors.append("stageselect.c: draw_names must load PAL0 (pal_ui) with "
                      "pal_load() - a queued/inherited palette is what made the "
                      "names change colour after a stage")
    if not re.search(r"VDP_setTextPalette\(\s*PAL0\s*\)", body):
        errors.append("stageselect.c: draw_names must draw with PAL0 (pal_ui: "
                      "0 = black, 15 = ink)")
    if re.search(r"VDP_setTextPalette\(\s*PAL1\s*\)", body):
        errors.append("stageselect.c: draw_names must not borrow PAL1 (the "
                      "stage's CRAM line 1)")
    if re.search(r"VDP_setTileMapDataRect\(BG_A,\s*&cells\[", body):
        errors.append("stageselect.c: the PC bar art must not sit behind the "
                      "names (no black background there)")

    clear = re.search(r"VDP_clearTextArea\(\s*0\s*,\s*BAR_ROW0\s*\+\s*(\d+)\s*,"
                      r"\s*SSEL_MAP_W\s*,\s*BAR_ROWS\s*-\s*(\d+)\s*\)", body)
    first_draw = re.search(r"VDP_drawText\(", body)
    if not clear:
        errors.append("stageselect.c: draw_names must VDP_clearTextArea the name "
                      "band (solid black, palette 0) before drawing")
    elif first_draw and clear.start() > first_draw.start():
        errors.append("stageselect.c: the black band is cleared after the first "
                      "VDP_drawText")

    rows = [int(n) for n in re.findall(r"VDP_drawText\([^;]*?BAR_ROW0\s*\+\s*(\d+)",
                                       body)]
    if clear and rows:
        first = int(clear.group(1))
        height = (const(src, "BAR_ROWS") or 0) - int(clear.group(2))
        outside = [r for r in rows if not (first <= r < first + height)]
        if outside:
            errors.append("stageselect.c: name rows BAR_ROW0+%s are outside the "
                          "black band BAR_ROW0+%d..%d"
                          % (outside, first, first + height - 1))

    try:
        p = os.path.join(HERE, "pal_ui_data.py")
        ns = {}
        exec(compile(read(p), p, "exec"), ns)
        rgb = ns["PAL_UI_RGB"]
        if tuple(rgb[0]) != (0, 0, 0):
            errors.append("tools/pal_ui_data.py: pal_ui entry 0 must be black "
                          "for the name band, got %r" % (tuple(rgb[0]),))
        lum = sum(rgb[15]) / 3.0
        if lum < 128:
            errors.append("tools/pal_ui_data.py: pal_ui entry 15 must be a light "
                          "ink for the names, got %r (luma %.0f)"
                          % (tuple(rgb[15]), lum))
    except Exception as exc:            # noqa: BLE001 - report, do not crash
        errors.append("tools/pal_ui_data.py: cannot read PAL_UI_RGB (%s)" % exc)

    for e in errors:
        print("FAIL  " + e)
    print("%d stage-select text violation(s)" % len(errors))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
