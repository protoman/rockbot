# Debugging — RockBot Mega Drive port

How to debug the SGDK ROM on this machine. Emulator of record: **BlastEm**
(flatpak `com.retrodev.blastem`). Kega Fusion is the fallback.

## BlastEm command line

```bash
flatpak run com.retrodev.blastem [OPTIONS] rom.bin [WIDTH] [HEIGHT]
```

| Flag | Use |
|---|---|
| `-d` | Enter debugger at startup. **Pauses the game** waiting for debugger input in a terminal window — looks "stuck" if you don't. BlastEm spawns its own terminal for it; driving it headless needs a pty/`xdotool`. |
| `-e FILE` | **Hardware event log** — every VDP/CRAM/DMA/68K access with cycle counts. Binary, zlib-compressed. Works headless (`timeout N flatpak run ... -e log.bin rom.bin`). |
| `-l` | Log 68K code addresses |
| `-s FILE` | Load GST savestate |
| `-D` | GDB remote stub (see below) |
| `-r J/U/E` | Force region |

## Interactive hotkeys (during gameplay)

| Key | Action |
|---|---|
| `u` | Enter debugger |
| `c` | CRAM (palette) viewer |
| `v` | VRAM viewer |
| `b` | Plane viewer |
| `p` | Screenshot |
| `tab` | Soft reset |
| `` ` `` / `l` | Save / load state |

## Native debugger commands (GDB-style, via `-d` or `u`)

```
help                          list commands
continue / c                  resume
frames N                      run N video frames then stop   <- step through time
breakpoint ADDR / b ADDR      68K breakpoint
watchpoint ADDR [SIZE]        watch 68K memory OR VDP registers/RAM
print[/x] EXPR / p/x EXPR     print value (e.g. VDP regs)
vr                            VDP register info
vs                            VDP sprite list
bt                            backtrace
co BP  ...  end               run command list each time BP hits
commands BP ... end           same, newer syntax
set MEM|NAME VALUE            poke
di[/x] EXPR                   print expr on every BP hit
softreset                     reset console
quit / q                      exit
```

Scripting: the debugger has a Turing-complete expression language (`if`,
`while`, `function NAME ... end`, `variable`, `array`).

**Gotchas**: `-d` blocks the game until the debugger terminal gets input;
piped stdin does NOT reach it (needs a real TTY; `script -qec` was not enough
under flatpak — it detaches). Interactive use or `xdotool` typing into the
spawned terminal window are the practical options. For headless work use
`-e` event logs instead.

## GDB source-level debugging (works for this project)

We have `m68k-elf-gdb` in MarsDev and link `out/release/rom.out` (ELF with
symbols) alongside `out/rom.bin`:

```bash
# terminal 1
flatpak run com.retrodev.blastem -D mega_drive/out/rom.bin
# terminal 2
m68k-elf-gdb mega_drive/out/release/rom.out
(gdb) target remote | flatpak run com.retrodev.blastem -D mega_drive/out/rom.bin
(gdb) break PAL_setPalette        # or any SGDK/project symbol
```

BlastEm halts at ROM entry until GDB connects. Breakpoints can then be set on
SGDK symbols (need SGDK built with debug info for full source lines —
currently only our `rom.out` has symbols; SGDK libmd is optimized).

## Event log (`-e`) format

File: `BLSTEL\x02\x00` + `u16 flags` + `u8 name_len` + ROM name,
then **zlib stream** of events.

Event header (after inflate):
- `< 0xE0` nibble form: `type:4, delta:4` where real delta = nibble + 16 (16..31)
- `0xE0 | type`: + `u16 delta` (big-endian)
- `0xF0 | type`: + `u8 hi` + `u16 lo` → 24-bit signed delta
- `0xD0 | n`: MULTI — next event repeated `n + 1` extra times with same delta

Types (`event_log.h`):
```
0 FLUSH  1 ADJUST(+u32, subtracts from cycle)  2 PSG(+u8)  3 YM(+u8)
4 VDP_REG(+u8 reg, +u8 value)
5 VRAM_BYTE(+u16 addr, +u8 val)   6 VRAM_BYTE_DELTA(+u8 delta, +u8 val)
7 VRAM_BYTE_ONE(+u8 val)          8 VRAM_BYTE_AUTO(+u8 val)
9 VRAM_WORD(+u24 addr, +u16 val) 10 VRAM_WORD_DELTA(+u8 delta, +u16 val)
11 VDP_INTRAM(+u8 addr, +u16 val)  <- CRAM if addr<128, else VSRAM
12 STATE(+)   13 MULTI
```
All u16/u32 payloads are **big-endian**. CRAM byte addr `2*n` = palette n,
color n's color value; backdrop = CRAM word 0 (reg 7 selects it, default 0).

Decoded parser used for the white-screen hunt: see git history of this
session; key pitfalls: VRAM_BYTE_DELTA payload is **2** bytes (not 3);
ADJUST payload stays in-stream for the caller (4 bytes, BE); STATE = 13-byte
header + `size` bytes of state (rare in file mode — if seen mid-stream,
suspect parser desync first).

## Known debugging stories

### White screen on intro (FIND-08 follow-up)

- Symptom: backdrop white ~1s after boot; rewriting PAL0 later fixes it.
- Isolation (comment-only binary search in `main.c`/`showIntro()`):
  culprit = `PAL_setPalette(PAL0, font_pal, CPU)` in `main()` — the CPU
  transfer method runs during **active display**; writing CRAM outside
  vblank is illegal on hardware and BlastEm models the corruption.
  SGDK docs: only `DMA_QUEUE` (flushed in vblank) and CPU-with-display-off
  are safe for CRAM.
- `VDP_setBackgroundColor(0)` alone: harmless. `palette_grey`-equivalent
  data: harmless. The **timing/method** is the bug, not the data.
- Delay (~1s): SGDK boot itself spends ~4s (VRAM clear + font load);
  the corrupted write only becomes visible once display re-enables /
  the queue/idle loop runs. See event-log timeline notes in session.
- Lesson: never call `PAL_setPalette`/`PAL_setColors` with `CPU` from
  main-loop code while the display is on. Use `DMA_QUEUE` + ensure a
  `SYS_doVBlankProcess()` follows, or disable the display first.
