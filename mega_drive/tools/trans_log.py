#!/usr/bin/env python3
"""Verdict from a MD_TRANS_DEBUG run (src/transition.c probes), no pictures.

Usage: tools/trans_log.py LOGFILE      (text log, or BlastEm's save.sram: the
                                        probes write ASCII at byte 0x4000)
       tools/trans_log.py --selftest

The game cannot be read through KDEBUG here, so a run writes its lines to SRAM
and the file ~/.local/share/blastem/rom/save.sram is read after a NORMAL exit.

Reads the `TRS` / `TR` / `TRF` lines of every transition and answers, in order:
  1. Did the outgoing tileset survive?   (out_bad at pre / post-load / end)
  2. Did every incoming tile reach VRAM? (in_bad at post-load / end; vram vs ram)
  3. Does plane A hold what trans_buf says, every frame?  (plane_bad per frame)
  4. Is the VDP configuration stable?     (r15 auto-increment must be 2, r16 plane
     size, r17/r18 window must be 0 after the frame)
Each failed check names the first frame/slot and prints the values to compare.
"""
import re
import sys

TR = re.compile(r"\bTR(F|S)?\s+(.*)")


def parse(text):
    runs, cur = [], None
    for line in text.splitlines():
        m = TR.search(line)
        if not m:
            continue
        kind, body = m.group(1) or "", m.group(2)
        kv = dict(re.findall(r"(\w+)=([0-9A-Fa-f/]+)", body))
        if kind == "S" and body.startswith("from="):
            cur = {"start": kv, "checks": [], "frames": {}, "raw": []}
            runs.append(cur)
        if cur is None:
            continue
        cur["raw"].append(body)
        if kind == "F":
            i = int(kv["i"])
            cur["frames"].setdefault(i, {}).update(kv)
        elif kind == "":
            cur["checks"].append((body.split()[0], body, kv))
    return runs


def verdict(run):
    out = []
    s = run["start"]
    out.append(f"transition {s.get('from')}->{s.get('to')} spare={s.get('spare')}")
    for tag, body, kv in run["checks"]:
        if "out_bad" in kv or "in_bad" in kv:
            key = "out_bad" if "out_bad" in kv else "in_bad"
            bad = int(kv[key].split("/")[0])
            out.append(f"  {'FAIL' if bad else 'ok  '} {tag:10s} {key}={kv[key]}")
        if " in slot=" in body:
            out.append("       " + body)
    firstbad = None
    for i in sorted(run["frames"]):
        f = run["frames"][i]
        pb = int(f.get("plane_bad", "0"))
        if pb and firstbad is None:
            firstbad = i
            out.append(f"  FAIL plane A frame {i}: {pb} cells differ, first "
                       f"r={f.get('r')} c={f.get('c')} vram={f.get('vram')} "
                       f"want={f.get('want')}")
    if firstbad is None and run["frames"]:
        out.append(f"  ok   plane A matches trans_buf in all {len(run['frames'])} frames")
    regs = {(f.get("r15"), f.get("r16"), f.get("r17"), f.get("r18"))
            for f in run["frames"].values()}
    out.append(f"  regs (r15,r16,r17,r18) seen: {sorted(regs)}")
    if any(f.get("r15") not in ("02", None) for f in run["frames"].values()):
        out.append("  FAIL auto-increment left != 2 after a frame")
    sample = [(i, f.get("in_bad"), f.get("out_bad")) for i, f in
              sorted(run["frames"].items()) if f.get("in_bad") != "0/8"
              or f.get("out_bad") != "0/8"]
    if sample:
        out.append(f"  FAIL resident-tile samples differ at frames "
                   f"{[x[0] for x in sample][:8]} (in/out per frame: {sample[:4]})")
    return out


def sram_text(raw):
    """Text of the probe log from a text file or a raw SRAM image."""
    if b"TRS" in raw and len(raw) < 1 << 20 and raw[:4] != b"\0\0\0\0":
        txt = raw.decode("latin1")
        if "TRS from=" in txt:
            return txt
    region = raw[0x4000:0x8000]
    for cand in (region, region.replace(b"\x00", b""),
                 raw[0x4000::2], raw[0x4001::2]):
        if b"TRS from=" in cand:
            return cand.replace(b"\x00", b"\n").decode("latin1")
    return ""


SELF = """TRS from=0 to=1 spare=387 cam=2880 dcam=0 t2b=1 qmax=80
TR pre out_bad=0/131 first=65535
TRS tiles_used=77 numTile=77 dest_bg=0 bg_new=0
TR post-load in slot=390 tile=3 vram=5E60 5E60 5E60 5E60 ram=1111 2222 3333 4444
TR post-load in_bad=2/77
TR post-load out_bad=0/131 first=65535
TRF i=0 base=0 r15=02 r16=11 r17=00 r18=00 r11=03 q=3
TRF i=0 plane_bad=0 first r=0 c=0 vram=0000 want=0000 in_bad=0/8 out_bad=0/8
TRF i=1 base=1 r15=02 r16=11 r17=00 r18=00 r11=03 q=3
TRF i=1 plane_bad=5 first r=10 c=7 vram=5E60 want=21A3 in_bad=1/8 out_bad=0/8
"""

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__); sys.exit(2)
    if sys.argv[1] == "--selftest":
        text = SELF
    else:
        raw = open(sys.argv[1], "rb").read()
        text = sram_text(raw)
    runs = parse(text)
    if not runs:
        print("no TRS/TR/TRF lines found (is this a MD_TRANS_DEBUG run?)")
        sys.exit(1)
    for r in runs:
        print("\n".join(verdict(r)))
