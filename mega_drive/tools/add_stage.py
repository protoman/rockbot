#!/usr/bin/env python3
"""Add one converted stage end to end.

Runs the four per-stage converters + the background tile decomposition, then
wires the result into the build:

  res/resources.res   SPRITE block from the enemy/object converters +
                      TILESET/PALETTE/MAP for the 3 maps + bg palettes
  src/stage_data.c/h  include, stage_defs[] entry, STAGE_DEFINED_COUNT
  Makefile            stage index in STAGES (the asset loop)

Idempotent: converters are just re-run; the resources block is only inserted
once (skipped if "# ---- STAGE N (" exists), and the stage index is only added
to STAGES if missing.

Usage: python3 tools/add_stage.py N [--no-convert]
"""
import os
import re
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
PY = sys.executable
RES = os.path.join(ROOT, "res")
SRC = os.path.join(ROOT, "src")
MARKER = "# ---- CONVERTED STAGES END (tools/add_stage.py inserts above) ----"


def run(args, capture=False):
    print("  $", " ".join(args[1:]))
    r = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
    if r.returncode != 0:
        sys.stdout.write(r.stdout)
        sys.stderr.write(r.stderr)
        sys.exit(f"add_stage: {' '.join(args[1:])} failed")
    if capture:
        return r.stdout
    # keep the useful lines only
    for line in r.stdout.splitlines():
        if re.search(r"palette|unique tiles|wrote src/stage|map [0-9]:|"
                     r"bg[0-9]:|skip spawn|error|Error|warn", line):
            print("   ", line)


def stage_name(n):
    with open(os.path.join(ROOT, "data/games/Rockbot1/stages_v301.dat"), "rb") as f:
        f.seek(438 * n)
        return f.read(30).split(b"\0")[0].decode("latin1")


def symbols(text):
    out = set()
    for line in text.splitlines():
        m = re.match(r"\s*(SPRITE|TILESET|PALETTE|MAP)\s+(\w+)", line)
        if m:
            out.add(m.group(2))
    return out


def block(n, name):
    """Resources block from the converter output + the generated files."""
    enemy = run([PY, "tools/convert_enemies.py", str(n)], capture=True)
    obj = run([PY, "tools/convert_objects.py", str(n)], capture=True)

    existing = symbols(open(os.path.join(RES, "resources.res")).read())
    sprites = []
    for out in (enemy, obj):
        for line in out.splitlines():
            if line.startswith("SPRITE "):
                sym = line.split()[1]
                if sym not in existing and line not in sprites:
                    sprites.append(line)

    lines = [f"# ---- STAGE {n} ({name}) ----"]
    lines += sprites
    for m in range(3):
        lines += [
            f'TILESET stage{n}_tiles{m} "stage{n}_map{m}.png" FAST ALL',
            f'PALETTE stage{n}_pal{m}   "stage{n}_map{m}.png"',
            # NONE, not FAST: a compressed MAP makes MAP_create() allocate
            # its blocks buffer from the heap (~2.9 KB for a 45-block map) on
            # top of sizeof(Map) (~580 B, it carries the 240-entry hscroll
            # table). With the transition buffers in BSS the largest free
            # block is only ~3.2 KB, so compressed maps failed to create and
            # the game crashed drawing a NULL map (BUG-040).
            f'MAP     stage{n}_map{m}   "stage{n}_map{m}.png"  stage{n}_tiles{m} NONE 0',
        ]
    for m in range(3):
        if os.path.exists(os.path.join(RES, f"stage{n}_bg{m}_tiles.png")):
            lines.append(f'PALETTE stage{n}_bg{m}_pal "stage{n}_bg{m}_tiles.png"')
    return "\n".join(lines) + "\n"


def patch_resources(n, name):
    path = os.path.join(RES, "resources.res")
    text = open(path).read()
    if f"# ---- STAGE {n} (" in text:
        print(f"resources.res: stage {n} already present")
        return
    if MARKER not in text:
        sys.exit("add_stage: marker missing from resources.res")
    text = text.replace(MARKER, block(n, name) + "\n" + MARKER, 1)
    open(path, "w").write(text)
    print(f"resources.res: inserted stage {n}")


def patch_registry(n):
    hpath = os.path.join(SRC, "stage_data.h")
    h = open(hpath).read()
    m = re.search(r"#define STAGE_DEFINED_COUNT (\d+)", h)
    count = max(int(m.group(1)), n + 1)
    h = h.replace(m.group(0), f"#define STAGE_DEFINED_COUNT {count}")
    open(hpath, "w").write(h)

    cpath = os.path.join(SRC, "stage_data.c")
    c = open(cpath).read()
    if f'#include "stage{n}_data.h"' not in c:
        incs = list(re.finditer(r'#include "stage(\d+)_data\.h"\n', c))
        last = max(incs, key=lambda x: int(x.group(1))).end()
        c = c[:last] + f'#include "stage{n}_data.h"\n' + c[last:]
    if f"&stage{n}_def," not in c:
        defs = list(re.finditer(r"\n\s*&stage(\d+)_def,", c))
        last = max(defs, key=lambda x: int(x.group(1))).end()
        c = c[:last] + f"\n    &stage{n}_def," + c[last:]
    open(cpath, "w").write(c)
    print(f"stage_data: STAGE_DEFINED_COUNT={count}, stage {n} registered")


def patch_makefile(n):
    path = os.path.join(ROOT, "Makefile")
    text = open(path).read()
    m = re.search(r"^STAGES := (.*)$", text, re.M)
    stages = [int(v) for v in m.group(1).split()]
    if n in stages:
        print(f"Makefile: stage {n} already in STAGES")
        return
    stages = sorted(set(stages + [n]))
    text = text[:m.start()] + "STAGES := " + " ".join(map(str, stages)) + text[m.end():]
    open(path, "w").write(text)
    print(f"Makefile: STAGES := {' '.join(map(str, stages))}")


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    n = int(sys.argv[1])
    name = stage_name(n)
    print(f"stage {n}: {name}")

    if "--no-convert" not in sys.argv:
        run([PY, "tools/convert_stage.py", str(n)])
        for m in range(3):
            p = os.path.join(RES, f"stage{n}_bg{m}.png")
            if os.path.exists(p):
                run([PY, "tools/img2tiles.py", p,
                     "--name", f"stage{n}_bg{m}"])

    patch_resources(n, name)
    patch_registry(n)
    patch_makefile(n)
    print(f"done: stage {n}. Run `make` (assets + ROM), then `make check-vram`.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
