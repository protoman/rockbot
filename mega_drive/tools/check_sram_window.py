#!/usr/bin/env python3
"""ROM data inside the SRAM window must never be read while SRAM is mapped.

The header declares SRAM at 0x200000-0x20FFFF. Past 2 MB the ROM overlaps it,
and `SRAM_enable()` ($A130F1 = 1) swaps that window to SRAM until it is
disabled. The port enabled it once in save_init() and left it on, so stage
map blocks placed in the window (stages 3..10 today) read as save bytes: a screen
of one repeated tile in SUN-GOD REALM / DARK MINE / CRYSTAL MAZE.

Fails when (a) any ROM symbol lies in the window, and (b) a source file calls
SRAM_enable()/SRAM_enableRO() outside src/sram_safe.h, or uses the SRAM_*
accessors without including it.
"""
import os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LO, HI = 0x200000, 0x20FFFF


def window_symbols():
    path = os.path.join(ROOT, "out/release/symbol.txt")
    if not os.path.exists(path):
        return None
    out = []
    for ln in open(path):
        p = ln.split()
        if len(p) >= 3:
            try:
                a = int(p[0], 16)
            except ValueError:
                continue
            if LO <= a <= HI:
                out.append(p[2])
    return out


def main():
    bad = []
    for fn in sorted(os.listdir(os.path.join(ROOT, "src"))):
        if not fn.endswith(".c") or fn == "sram_safe.h":
            continue
        s = open(os.path.join(ROOT, "src", fn)).read()
        if re.search(r"\bSRAM_enable(RO)?\s*\(", s) and "sram_safe.h" not in s:
            bad.append(f"{fn}: calls SRAM_enable() (leaves ROM above 2 MB shadowed)")
        if re.search(r"\bSRAM_(read|write)(Byte|Word|Long)\s*\(", s) and "sram_safe.h" not in s:
            bad.append(f"{fn}: SRAM_* accessors without sram_safe.h")
        if re.search(r"\bSRAM_(read|write)Long\s*\(", s):
            bad.append(f"{fn}: SRAM_*Long is not wrapped by sram_safe.h")
    syms = window_symbols()
    if syms:
        print(f"info: {len(syms)} ROM symbols sit in the SRAM window "
              f"({syms[0]} .. {syms[-1]}); safe only because SRAM stays unmapped")
    for b in bad:
        print("FAIL", b)
    if bad:
        return 1
    print("check_sram_window: OK")
    return 0


sys.exit(main())
