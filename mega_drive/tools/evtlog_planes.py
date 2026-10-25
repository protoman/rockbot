#!/usr/bin/env python3
"""Decode a BlastEm -e event log and report the two things BUG-044 cannot
explain: plane cells whose tile index is below the user area (0..15 system
stripes) or equal to the shared blank tile 702, and CRAM writes that happen
outside vblank (illegal on hardware, they corrupt the palette).

Log format: debugging.md ("Event log (-e) format").
"""
import sys
import zlib
from collections import Counter

FRAME_CYCLES = 488 * 262          # NTSC scanline budget


def events(data):
    assert data[:8] == b"BLSTEL\x02\x00", data[:8]
    flags = int.from_bytes(data[8:10], "big")
    name_len = data[10]
    body = data[11 + name_len:]
    raw = zlib.decompress(body)
    i = 0
    n = len(raw)
    while i < n:
        b = raw[i]
        i += 1
        if (b & 0xF0) == 0xF0:                     # 24 bit delta
            typ = b & 0x0F
            delta = int.from_bytes(raw[i:i + 3], "big")
            if delta & 0x800000:
                delta -= 1 << 24
            i += 3
        elif (b & 0xF0) == 0xE0:                   # u16 delta
            typ = b & 0x0F
            delta = int.from_bytes(raw[i:i + 2], "big")
            i += 2
        elif (b & 0xF0) == 0xD0:                   # MULTI
            typ = 0x0D
            delta = 0
        else:
            typ = b >> 4
            delta = (b & 0x0F) + 16
        if typ == 0x01:                            # ADJUST
            i += 4
            continue
        yield typ, delta, raw, i
        # payload length per type
        i += {0x00: 0, 0x02: 1, 0x03: 1, 0x04: 2, 0x05: 3, 0x06: 2,
              0x07: 1, 0x08: 1, 0x09: 5, 0x0A: 3, 0x0B: 3, 0x0C: 0,
              0x0D: 0, 0x0E: 0, 0x0F: 0}.get(typ, 0)


def main(path):
    data = open(path, "rb").read()
    plane_a, plane_b = 0xC000, 0xE000       # VDP regs 2/4 defaults
    cram = {}
    cyc = 0
    n_cram = 0
    late_cram = Counter()                  # scanline -> writes
    line0 = Counter()                      # CRAM[0] values seen
    cells = Counter()                      # (plane, cell, tile) for tile<16
    sky_cells = 0                          # cells pointing at 702
    words = 0
    total = 0
    bad_high = 0
    for typ, delta, raw, i in events(data):
        cyc += delta
        total += 1
        if typ == 0x04:                    # VDP register
            reg = raw[i]
            val = raw[i + 1]
            if reg == 2:
                plane_a = (val & 0x38) << 10
            elif reg == 4:
                plane_b = (val & 0x07) << 13
        elif typ == 0x0B:                  # VDP_INTRAM: CRAM if addr < 128
            addr = raw[i]
            val = int.from_bytes(raw[i + 1:i + 3], "big")
            if addr < 128:
                n_cram += 1
                cram[addr // 2] = val
                scan = (cyc % FRAME_CYCLES) // 488
                if 0 <= scan < 224:
                    late_cram[scan] += 1
                if addr == 0:
                    line0[val] += 1
        elif typ in (0x09, 0x0A):
            if typ == 0x09:
                addr = int.from_bytes(raw[i:i + 3], "big")
                val = int.from_bytes(raw[i + 3:i + 5], "big")
                words += 1
            else:
                val = int.from_bytes(raw[i + 1:i + 3], "big")
                addr = None
            if addr is not None:
                tile = val & 0x07FF
                base = plane_a if plane_a <= addr < plane_a + 0x1000 else (
                    plane_b if plane_b <= addr < plane_b + 0x1000 else None)
                if base is not None:
                    cell = (addr - base) // 2
                    if tile == 702:
                        sky_cells += 1
                    elif 0 < tile < 16:
                        cells[(base, cell, tile, (val >> 13) & 3)] += 1
        elif typ in (0x05, 0x06, 0x07, 0x08):
            if typ == 0x05:
                words += 1
                addr = int.from_bytes(raw[i:i + 2], "big")
                if plane_a <= addr < plane_a + 0x1000 or \
                        plane_b <= addr < plane_b + 0x1000:
                    bad_high += 1
    print(f"events={total} word_writes={words} frames~{cyc // FRAME_CYCLES}")
    print(f"CRAM writes={n_cram}  final CRAM[0]={cram.get(0)} "
          f"CRAM[1]={cram.get(1)} CRAM[15]={cram.get(15)} 0x0EEE={0x0EEE}")
    print("CRAM[0] values written:", dict(line0))
    print(f"CRAM writes during visible scanlines: {sum(late_cram.values())} "
          f"across {len(late_cram)} scanlines")
    print(f"plane cells with tile 1..15: {sum(cells.values())} "
          f"distinct={len(cells)}")
    for k, v in cells.most_common(8):
        print("   plane %04X cell %4d tile %d pal %d x%d" % (k[0], k[1], k[2],
                                                             k[3], v))
    print(f"plane cells pointing at the shared blank tile 702: {sky_cells}")
    print(f"plane byte writes (address may be a tile index too): {bad_high}")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "/tmp/opencode/md-ev/evt.bin")
