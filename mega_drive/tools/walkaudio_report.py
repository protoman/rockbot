#!/usr/bin/env python3
"""Reads the KDEBUG log of a -DMD_WALK_AUDIO_DIAG run and says where the ending's
walking-scene music state goes wrong.

    python3 tools/walkaudio_report.py /tmp/opencode/md-emu/kdebug.log

Verdict per line: the port flag (`playing`) against the Z80 driver (`xgm2`).
  OK        playing=1 xgm2=1   the driver thinks it plays (silence would be hardware/bus)
  NO-DRIVER playing=1 xgm2=0   the port believes it plays, the driver does not
  STOPPED   playing=0 xgm2=0   something stopped it (look at the line before)
"""
import re
import sys

LINE = re.compile(r"MUS (.*?): playing=(\d) xgm2=(\d) level=(\d) pending=(\d) same=(\d) sound=(\d)")


def main(path):
    rows = []
    for ln in open(path, errors="replace"):
        m = LINE.search(ln)
        if m:
            rows.append((m.group(1), *map(int, m.groups()[1:])))
    if not rows:
        print("no MUS lines: wrong build (needs -DMD_WALK_AUDIO_DIAG -DMD_DEBUG) or empty log")
        return 1
    first_bad = None
    for tag, playing, xgm2, level, pending, same, sound in rows:
        v = "OK" if (playing and xgm2) else ("NO-DRIVER" if playing else "STOPPED")
        if v != "OK" and first_bad is None:
            first_bad = tag
        extra = []
        if not sound:
            extra.append("SOUND OFF in config")
        if level == 0:
            extra.append("music volume 0")
        if not same:
            extra.append("current track is not END_ANIMATION")
        print(f"{v:10s} {tag:36s} playing={playing} xgm2={xgm2} level={level} "
              f"pending={pending} {' '.join(extra)}")
    print()
    print("first non-OK state:", first_bad or "none (driver reports music playing the whole walk)")
    if first_bad is None:
        print("-> the state is right but nothing is heard: use the step1/2/3 times "
              "the listener noted (see the ask).")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else "/tmp/opencode/md-emu/kdebug.log"))
