#!/usr/bin/env python3
"""Reads the BUG-045 log (build: -DMD_TEST_ENDING -DMD_WALK_DIAG -DMD_TILE_DUMP, run under
`script -qf`) and prints, per boot, a verdict on every suspect:

  queue entries (Q)        bank crossings / RAM sources / zero lengths
  step timeline (CK vt=)   which step stalls (vblanks per step)
  readback (VER)           what landed after the flush / after the maps / per frame
  experiments (EXP A..G)   which loading method lands, and the time it takes
  frame timing (TIME)      sprite update vs vblank sub-ticks, slow frames
  Z80/XGM2 (CK xgm2 ...)   DMA wait time, bus state

usage: python3 tools/walkdiag_report.py [/tmp/opencode/md-emu/kdebug.log]
"""
import re
import sys

path = sys.argv[1] if len(sys.argv) > 1 else "/tmp/opencode/md-emu/kdebug.log"
lines = [l.split("KDEBUG MESSAGE: ", 1)[1].rstrip("\r\n") for l in open(path, errors="replace")
         if "KDEBUG MESSAGE: " in l]
boots, cur = [], None
for l in lines:
    if l.startswith("=== BOOT"):
        cur = []
        boots.append(cur)
    elif cur is not None:
        cur.append(l)
print(f"{len(lines)} log lines, {len(boots)} boot(s)")


def bad(vl, key):
    m = re.search(rf"VER {re.escape(key)}: bad=(\d+)/(\d+)", vl)
    return (int(m.group(1)), int(m.group(2))) if m else None


for bi, b in enumerate(boots, 1):
    print(f"\n===== boot {bi} =====")
    # --- queue entries
    for l in b:
        m = re.match(r"Q (.*): src=([0-9A-F]+) len=(\d+) dst=([0-9A-F]+) bank_cross=(\d)", l)
        if m:
            src = int(m.group(2), 16)
            flags = []
            if m.group(5) == "1": flags.append("CROSSES 128KB BANK")
            if src >= 0x400000: flags.append("SOURCE IN RAM")
            if int(m.group(3)) == 0: flags.append("ZERO LENGTH")
            print(f"  Q {m.group(1):12s} src={src:06X} len={m.group(3)} dst={m.group(4)} {' '.join(flags) or 'ok'}")
    # --- timeline
    prev = None
    print("  steps (vt = frame counter, +n = vblanks since the previous step):")
    for l in b:
        m = re.match(r"CK ([^:]+): vt=(\d+) imask=(\d+) vbp=(\w+) autoflush=(\d) dmaq=(\d+)/(\d+) "
                     r"bytes=(\d+) heap=(\d+)/(\d+) z80bus=(\d) xgm2=(\d)", l)
        if m:
            vt = int(m.group(2))
            d = "" if prev is None else f" +{vt - prev}"
            note = "  <-- STALL" if prev is not None and vt - prev >= 8 and "VER" not in l else ""
            print(f"    {m.group(1):28s} vt={vt}{d} imask={m.group(3)} dmaq={m.group(6)} "
                  f"heap={m.group(9)}/{m.group(10)} z80bus={m.group(11)} xgm2={m.group(12)}{note}")
            prev = vt
    for l in b:
        if "dmawait" in l:
            print("   ", l[:160])
            break
    # --- does DMA work at each step? (PROBE lines are printed at every checkpoint)
    print("  DMA probe per step (immediate ROM-source / immediate RAM-source / queued):")
    first_fail = None
    for l in b:
        m = re.match(r"PROBE ([^:]+): dma_rom=(\w+) dma_ram=(\w+) queued_rom=(\S+)", l)
        if m:
            print(f"    {m.group(1):28s} rom={m.group(2)} ram={m.group(3)} queued={m.group(4)}")
            if first_fail is None and "FAIL" in l:
                first_fail = m.group(1)
    print(f"  => first step where a DMA probe fails: {first_fail or 'none'}")
    for l in b:
        if l.startswith("LAYER"):
            print("   ", l)
    sts = [(re.match(r"CK ([^:]+): status=(\w+)", l)) for l in b]
    sts = [m for m in sts if m]
    if sts:
        print("  VDP status per step (b1 = DMA busy, b8 = FIFO full): " +
              ", ".join(f"{m.group(1)}={m.group(2)}" for m in sts[:12]))
    # --- readback summary
    def summ(tag):
        out = []
        for cls in ("tiles L3", "tiles L2", "tiles L1", "PAL0", "PAL1", "PAL2", "PAL3",
                    "map BG_B L3", "map BG_A L2", "map BG_A L1"):
            for l in b:
                r = bad(l, f"{tag} {cls}")
                if r:
                    out.append(f"{cls}={'OK' if r[0] == 0 else f'BAD {r[0]}/{r[1]}'}")
                    break
        return ", ".join(out) if out else "(not logged)"
    for tag in ("after flush", "after maps", "0", "1", "2", "30", "60"):
        s = summ(tag)
        if s != "(not logged)":
            print(f"  VER {tag:12s}: {s}")
    # --- experiments
    print("  experiments (a rung 'lands' when its own items read back OK):")
    items = {"A": ["tiles L3", "tiles L2", "tiles L1", "PAL0", "PAL1", "PAL2", "PAL3"],
             "B": ["tiles L3", "PAL0"], "C": ["tiles L2", "PAL1"],
             "D": ["tiles L1", "PAL3", "PAL2"], "F": ["tiles L1", "PAL3"], "G": ["PAL3"],
             "H": ["tiles L1", "PAL3"], "I": ["tiles L1", "PAL3"],
             "E (all by CPU)": ["tiles L3", "tiles L2", "tiles L1", "PAL0", "PAL1", "PAL2", "PAL3"]}
    for ex, cls in items.items():
        res = []
        for c in cls:
            for l in b:
                r = bad(l, f"EXP {ex} {c}")
                if r:
                    res.append((c, r[0]))
                    break
        if not res:
            continue
        ok = all(n == 0 for _, n in res)
        det = ", ".join(f"{c}:{'ok' if n == 0 else 'BAD'+str(n)}" for c, n in res)
        info = [l for l in b if l.startswith(f"EXP {ex[0]} ") and "VER" not in l]
        print(f"    EXP {ex:16s} {'LANDS' if ok else 'FAILS':6s} {det}   {info[0][6:140] if info else ''}")
    # --- frame timing
    spr, vb = [], []
    for l in b:
        m = re.match(r"TIME f=(\d+) spr=(\d+) vblank=(\d+)", l)
        if m:
            spr.append(int(m.group(2)))
            vb.append(int(m.group(3)))
    if spr:
        print(f"  TIME (sub-ticks, 1280 = one frame): sprite update avg={sum(spr)//len(spr)} "
              f"max={max(spr)}; vblank wait+flush avg={sum(vb)//len(vb)} max={max(vb)}")
    slow = [l for l in b if l.startswith("SLOWFRAME")]
    print(f"  SLOWFRAME lines: {len(slow)} (verification frames are slow by design); first: "
          f"{slow[0] if slow else '-'}")
    for l in b:
        if "hscroll words" in l or "sprite0" in l:
            print("   ", l)
            if "tile" in l:
                break
print("\nverdict guide: queued flush FAILS + immediate DMA FAILS + CPU LANDS -> load the walking "
      "graphics by CPU; DMA LANDS only with music paused/stopped -> Z80 driver/DMA protection; "
      "RAM-source DMA LANDS -> ROM-source DMA problem; everything FAILS -> VDP state broken "
      "(see CK regs).")
