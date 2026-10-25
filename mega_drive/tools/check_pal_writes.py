#!/usr/bin/env python3
"""Every CPU-method CRAM write must go through src/pal_safe.h.

A CPU-method palette write is a VDP address-port write followed by data words.
If a DMA is still running, or the vblank handler sets the address port in
between, the data lands at whatever CRAM entry the VDP holds at that moment -
a WRONG entry, not just a torn frame. Measured on this port (BUG-044
follow-up): the boot grey ramp read back at entries 8..15 on one boot and
10..15 on the next, i.e. the destination slid by a different amount each time.

pal_load()/pal_load_n()/pal_color() wait the DMA out and keep interrupts off
for the whole sequence. Calling PAL_setPalette/PAL_setColors/PAL_setColor with
`CPU` directly in src/*.c is therefore a bug even when the display happens to
be off, because the call site can move.

Usage: tools/check_pal_writes.py     (exit 1 if a bare CPU write is found)
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CALL = re.compile(r"PAL_set(?:Palette|Colors|Color)\s*\([^;]*\)", re.S)


def main():
    bad = 0
    for path in sorted(glob.glob(os.path.join(ROOT, "src", "*.c"))):
        if os.path.basename(path) == "pal_safe.h":
            continue
        text = open(path, encoding="utf-8", errors="replace").read()
        for m in CALL.finditer(text):
            call = m.group(0)
            if "DMA_QUEUE" in call or "DMA" == call.strip():
                continue
            if re.search(r"\bCPU\b", call):
                line = text.count("\n", 0, m.start()) + 1
                print(f"{os.path.basename(path)}:{line}: bare CPU palette write"
                      f" -> use pal_load()/pal_load_n()/pal_color()")
                for l in call.splitlines():
                    print("    " + l.strip())
                bad += 1
    if bad:
        print(f"\n{bad} CPU palette write(s) outside src/pal_safe.h")
        return 1
    print("palette writes: all CPU-method writes go through src/pal_safe.h")
    return 0


if __name__ == "__main__":
    sys.exit(main())
