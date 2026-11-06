#!/usr/bin/env python3
"""Every dbg_log() format string must fit the log line buffer (src/dbglog.c).

vsprintf has no bound. BUG: the 'LK' line grew to ~280 characters against a
160-byte stack buffer and smashed the stack the first time a room link fired
(address error, 2026-10-26). Worst case per conversion: %d/%u 6, %X 4 (u16),
%lu/%ld 11, %s 32.
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src = open(os.path.join(ROOT, "src/dbglog.c")).read()
bufdecl = re.search(r"char buf\[(\w+)\]", src).group(1)
mac = re.search(r"#define DBG_LINE_MAX (\d+)", src)
limit = int(mac.group(1)) if bufdecl == "DBG_LINE_MAX" and mac else int(bufdecl)
print(f"dbglog buffer: {bufdecl} = {limit} bytes")
bad = 0
for path in sorted(glob.glob(os.path.join(ROOT, "src", "*.c"))):
    text = open(path, errors="replace").read()
    for m in re.finditer(r"dbg_log\(\s*((?:\"(?:[^\"\\]|\\.)*\"\s*)+)", text):
        fmt = "".join(re.findall(r'"((?:[^"\\]|\\.)*)"', m.group(1)))
        w = len(re.sub(r"%[-0-9.]*l[ud]|%[-0-9.]*[sduxX]", "", fmt))
        for c in re.findall(r"%[-0-9.]*(l?[sduxX])", fmt):
            w += {"d": 6, "u": 5, "x": 4, "X": 4, "s": 32, "ld": 11, "lu": 11}.get(c, 6)
        w += 1
        line = text[:m.start()].count("\n") + 1
        if w > limit:
            print(f"{os.path.basename(path)}:{line}: worst case {w} > {limit}")
            bad += 1
        else:
            print(f"ok   {os.path.basename(path)}:{line}: worst case {w}")
sys.exit(1 if bad else 0)
