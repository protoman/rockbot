#!/usr/bin/env python3
"""Put the black outline back on hand-edited enemy sheets (2026-11-01).

The converter gives every sheet two black palette entries: index 0 = transparent, index 1 =
the opaque black outline. The paint tool used for the hand edits merges identical colours on
save, so every outline pixel was written as index 0 and became transparent in game.

For each sheet: a pixel that is index 0 now and was index 1 in the last generated version
(git rev, default 96f34348~1) goes back to index 1. Pixels the owner redrew (not 0 now) are left.

usage: python3 tools/restore_outline.py [--rev REV] [--dry] sheet [sheet ...]   (names without .png)
"""
import io
import os
import subprocess
import sys

from PIL import Image

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main(argv):
    rev = "96f34348~1"
    dry = "--dry" in argv
    if "--rev" in argv:
        rev = argv[argv.index("--rev") + 1]
    names = [a for a in argv[1:] if not a.startswith("--") and a != rev]
    for n in names:
        path = os.path.join(ROOT, "res", n + ".png")
        old = Image.open(io.BytesIO(subprocess.check_output(
            ["git", "show", "%s:mega_drive/res/%s.png" % (rev, n)], cwd=ROOT)))
        new = Image.open(path)
        if old.size != new.size:
            print("%s: size changed %s -> %s, skipped" % (n, old.size, new.size))
            continue
        o, w = list(old.getdata()), list(new.getdata())
        fixed = [1 if (a == 1 and b == 0) else b for a, b in zip(o, w)]
        n_fix = sum(1 for a, b in zip(w, fixed) if a != b)
        print("%s: %d outline pixels restored" % (n, n_fix))
        if n_fix and not dry:
            new.putdata(fixed)
            new.save(path)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
