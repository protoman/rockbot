#!/usr/bin/env python3
"""Verdict for the room-change background glitch (stairs_return), no pictures.

Usage: tools/glitch_log.py save.sram|logfile      (see tools/trans_log.py for how
       tools/glitch_log.py --selftest              the log reaches save.sram)

Per room change, in order of the run, the log carries:
  LK   the link decision            TRS  slide start (from/to)
  BGS  BG_B right after the slide, read back from VRAM, against what the slide
       meant to write (bad_rows > 0 -> the SLIDE wrote wrong parallax rows)
  BGC  BG_B after stage_load (and +3 frames), read back against the map's tiles
       (bad_rows > 0 -> stage_load / a late DMA left wrong rows)
  SLA  a slide frame with an odd VDP state (scroll mode changed, auto-increment
       != 2, DMA queue not empty after the flush) -> the one-frame glitch
Outcome -> next step:
  BGS bad, BGC ok    slide writes wrong rows, hidden by the reload: fix transition.c bg rows
  BGC bad            stage_load / plane left wrong: look at bg_load order and late DMA
  SLA present        a frame with a bad VDP state: the first SLA line names the frame
  all ok             the plane DATA is right; the picture is wrong via scroll values:
                     use the screenshots of the same run
A full log (>90% of the SRAM area) is reported first: later transitions are missing.
"""
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from trans_log import sram_text  # noqa: E402

CAP = 0x7F00 - 0x4000


def kv(line):
    return {k: v for k, v in re.findall(r"(\w+)=(-?[0-9A-Fa-f/]+)", line)}


def analyse(text):
    out, runs, cur = [], [], None
    for line in text.splitlines():
        if line.startswith("LK from="):
            cur = {"lk": kv(line), "bgs": None, "bgc": [], "sla": [], "lka": None}
            runs.append(cur)
        elif cur is None:
            continue
        elif line.startswith("BGS "):
            cur["bgs"] = kv(line)
        elif line.startswith("BGC "):
            cur["bgc"].append((line.split()[1], kv(line)))
        elif line.startswith("SLA "):
            cur["sla"].append(line)
        elif line.startswith("LKA "):
            cur["lka"] = kv(line)
    used = len(text.encode("latin1"))
    if used > CAP * 0.9:
        out.append(f"WARNING log is {100 * used // CAP}% full: later transitions may be missing")
    if not runs:
        out.append("no LK lines: is this a MD_LINK_DEBUG + MD_TRANS_DEBUG run?")
    bad_any = False
    for n, r in enumerate(runs):
        lk = r["lk"]
        v = []
        if r["bgs"] is None:
            v.append("BGS missing (slide did not run, palette fade fallback?)")
        elif int(r["bgs"]["bad_rows"]):
            bad_any = True
            v.append(f"SLIDE-END BG_B bad_rows={r['bgs']['bad_rows']} first={r['bgs']['first']}")
        if not r["bgc"]:
            v.append("BGC missing")
        for tag, b in r["bgc"]:
            if int(b["bad_rows"]):
                bad_any = True
                v.append(f"LOAD {tag} BG_B bad_rows={b['bad_rows']} first={b['first']} vs={b['vs']}")
        if r["sla"]:
            bad_any = True
            v.append(f"ODD FRAME: {r['sla'][0]} (+{len(r['sla']) - 1} more)")
        out.append(f"#{n} link {lk.get('from')}->{lk.get('to')} trans={lk.get('trans')}: "
                   + ("ok" if not v else "; ".join(v)))
    out.append("VERDICT: " + ("a probe fired, see lines above" if bad_any
                              else "plane data and VDP state clean in every transition"))
    return out


SELF = """LK from=0 to=1 trans=1 type=0 old_x=1563
TRS from=0 to=1 spare=326 cam=1293
BGS slide_end to=1 dest_bg=1 bad_rows=0 first=-1 vs_new=16
BGC +3 map=1 bad_rows=0 first=-1 vs=16 base=131 h=30
LKA scrolled=1 x=267
LK from=1 to=0 trans=0 type=0 old_x=267
BGS slide_end to=0 dest_bg=1 bad_rows=11 first=0 vs_new=15
BGC load map=0 bad_rows=0 first=-1 vs=15 base=131 h=30
BGC +3 map=0 bad_rows=0 first=-1 vs=15 base=131 h=30
SLA i=55 r11=03/07 r15=02 q=0
"""

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__); sys.exit(2)
    text = SELF if sys.argv[1] == "--selftest" else sram_text(open(sys.argv[1], "rb").read())
    print("\n".join(analyse(text)))
