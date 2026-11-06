# Launching BlastEm (and its debugger) on this machine

Read this before touching the emulator. It exists because the same launch was
re-derived by trial and error too many times.

## Rule zero

- **Ask the user before every launch.** The emulator takes over their desktop.
- **One launch = one permission.** A failed attempt does **not** extend it. If
  the launch fails, or the mechanism you wanted does not exist, **stop**,
  report what failed, and ask again. A retry with different flags is a new
  launch and needs a new ask.
- **One launch per question.** Reproduce once, collect the evidence from a log/
  file/debugger, stop. Never relaunch "just to see".
- **Prefer readback over screenshots**: event log, savestate, debugger dump,
  symbol-mapped PC. Screenshots only when numbers cannot answer it.
- Stop the emulator when done (`tools/emu.sh stop`). There is no auto-timeout.

## Pre-flight — do this BEFORE asking, in its own call

Never combine a launch with the check that decides whether the launch can
work. The launch belongs in a separate, reviewable call after the checks pass.

```bash
# 1) what exists on this machine (no emulator involved)
command -v tmux xterm x-terminal-emulator m68k-elf-gdb wtype ydotool

# 2) is a debugger front-end already running / reachable?
ps -eo pid,comm | grep -E "blastem|termhelper|gnome-terminal"
for w in $(xdotool search --name "" 2>/dev/null); do
  n=$(xdotool getwindowname "$w" 2>/dev/null)
  case "$n" in *Debugger*|*BlastEm*) echo "$w | $n";; esac
done

# 3) where will the evidence land? decide the channel and its path NOW
#    -e /tmp/opencode/md-emu/evt.bin  |  savestate  |  debugger dump
```

Then state, in one sentence, the single question the run answers. If the
answer can come from files, code or data, do that instead — no launch.

Common blockers on this machine, so stop early instead of launching to find out:

| Goal | Reachable? |
|---|---|
| Game window + pad input + screenshots | yes (`tools/emu.sh`) |
| `-d` / `u` interactive debugger | debugger UI is a spawned `gnome-terminal`, which is a Wayland client here -> invisible to `xdotool`, cannot be typed into |
| piped stdin / FIFO into the debugger | no — debugger UI is not BlastEm's stdio |
| `tmux` | not installed |
| `m68k-elf-gdb` / m68k GDB remote | no (no binary; system gdb has no m68k target) |
| `-e` hardware event log | yes, headless |
| `-l` code-address log | no — it opens a file for *reading* |
| `wtype`/`ydotool`/`grim` (Wayland input) | not installed |

## Why this file exists

On 2026-10-21 this session launched BlastEm three times in a row without
asking: (1) `-d` inside a tmux attempt (tmux not installed), (2) the same with
a FIFO on stdin, (3) the same with `GDK_BACKEND=x11`. All three were the same
mistake: launching to *test a hypothesis* instead of running `command -v` and
`ps` first, and retrying after a failure without a new permission. The
debugger window was a spawned `gnome-terminal` the whole time; one `ps` call
would have shown that and prevented all three launches.

Later the same day it happened again, twice under a single "run it now":
the KDEBUG run, then a `stdbuf -o0` retry because stdout redirection had not
captured the messages. The retry was correct in content (KDEBUG goes to the
console, not stdout) but it was still a second launch taken without asking.
Hence the rule above: **a retry is a new launch**.

## Binary and window

- Native build (NOT the flatpak any more):
  `/mnt/ExtraData/programas/blastem64-1.0.0/blastem`
- Game window: X11 window named `BlastEm` (with a ROM loaded through
  `tools/emu.sh` it shows the ROM header title, e.g. `SAMPLE PROGRAM - BlastEm`).
- `xdotool` only sees **X11** windows. Native Wayland clients (konsole,
  gnome-terminal started normally) do **not** appear to it.

## Normal run (game only)

```bash
cd mega_drive
source tools/emu.sh
emu_start out/rom.bin          # kills a previous instance first
emu_key Right 400              # focus + hold a pad key
emu_shot name                  # window screenshot in $EMU_WORK
emu_stop                       # Esc menu + exact-name kill
```

Or `make run`. `EMU_ARGS=` swaps `-g` (software renderer) for the GL one.

Key mapping in BlastEm's config (`~/.config/blastem/blastem.cfg`):
`z`=Y, `x`=C, `c`=A, `a`=B, `q`=X, `w`=Y, arrows = D-pad, `Return` = Start.

## Launching with the debugger

**Check the pre-flight table first.** If the debugger window will not be
reachable (`Debugger` window absent from `xdotool search`, no m68k gdb), a
`-d` run answers nothing — it only pins the emulator on the user's desktop.

```bash
/mnt/ExtraData/programas/blastem64-1.0.0/blastem -d out/rom.bin
```

`-d` starts halted in the debugger. `u` during play enters it again.

**How it actually behaves on this machine (verified 2026-10-21):**

- BlastEm does **not** use the terminal it was started from. It spawns its own
  debugger front-end:
  `gnome-terminal --wait -t "BlastEm Debugger" -- <blastem-dir>/termhelper`
  (visible as `gnome-terminal`, `gnome-terminal.real` and `termhelper` in `ps`).
- That window is a Wayland client here, so it does **not** show up in
  `xdotool search` and `xdotool type` cannot drive it. Piping BlastEm's stdin
  does not help either: the debugger UI lives in that spawned terminal, not in
  BlastEm's own stdio.
- `tmux` is not installed. There is no `m68k-elf-gdb` on this machine and the
  system `gdb` has no m68k target (`set architecture m68k` -> "Undefined
  item"), so the `-D` GDB-remote route from `debugging.md` is **not** available
  here.

To check whether the debugger window exists at all:

```bash
for w in $(xdotool search --name "" 2>/dev/null); do
  n=$(xdotool getwindowname "$w" 2>/dev/null)
  case "$n" in *Debugger*|*BlastEm*) echo "$w | $n";; esac
done
```

If `BlastEm Debugger` is absent, the debugger is running but unreachable —
either drive it by hand (user types into the window) or use an unverified
workaround: force the spawned terminal onto XWayland so `xdotool` can reach it

```bash
env GDK_BACKEND=x11 /mnt/ExtraData/programas/blastem64-1.0.0/blastem -d out/rom.bin
```

(Not verified yet. If the `BlastEm Debugger` window then appears in the
`xdotool search` above, focus it and type commands with
`xdotool type --window <wid> "c"` + `xdotool key --window <wid> Return`.)

Useful debugger commands (`debugging.md` has the full list): `c` continue,
`r` registers, `bt` backtrace, `b ADDR` breakpoint, `frames N` run N frames,
`di EXPR`, `vr`/`vs` VDP registers/sprites, `q` quit.

## KDEBUG / `kprintf` (BlastEm debug console) — the value readout that works

**Verified capture (2026-10-21):** run BlastEm under a pty with `script -f`
(flush after every write). With TTY stdio BlastEm does **not** spawn its own
debugger terminal and `vdp.c` prints `KDEBUG MESSAGE: ...` to stdout, so the
typescript file receives everything:

```bash
script -qf -c "/mnt/ExtraData/programas/blastem64-1.0.0/blastem out/rom.bin" \
  /tmp/opencode/md-emu/kdebug.log
grep -a KDEBUG /tmp/opencode/md-emu/kdebug.log | tail -20
```

Then read the file — no window, no screenshots. `-f` is required: without it
the typescript stays empty while the game runs.

Why the alternatives fail (BlastEm source: `terminal.c`, `blastem.c:404`,
`vdp.c:5885`):

- Non-TTY stdio: BlastEm spawns
  `x-terminal-emulator -title "BlastEm Debugger" -e ./termhelper`, creates
  `/tmp/blastem_input` + `/tmp/blastem_output`, then **blocks** in
  `open(INPUT_PATH, O_RDONLY)` until a writer holds the other end. A plain
  `cat`/`tail -f` on the output FIFO therefore reads zero bytes — BlastEm
  never gets past that open. A reader must also hold the INPUT FIFO open.
- `termhelper` is a dumb relay (stdin -> input FIFO, output FIFO -> stdout). It
  only works inside a real terminal window; under `script` it emits nothing.
- GUI terminals started from a detached agent shell do not map a window here,
  so that route is unreliable.
- `-l` opens a file for **reading** (symbols); it is not a log. `-e` is the
  VDP event log (binary), not KDEBUG text.

SGDK's `kprintf()` (`tools.h`, 255-byte buffer) is the instrumentation; keep
the volume low (heartbeat every 30 frames plus one-shot markers). A hang shows
as the last marker/heartbeat with nothing after it.

ROM title / header: the project's own `src/rom_header.c` overrides SGDK's
default (the makefile copies the default only when the file is absent), so the
"SAMPLE PROGRAM" window title lived there — change the domestic/overseas name
fields and rebuild; verify with `grep -a ROCKBOT out/rom.bin`.

## Log-style readback that works headless

- **Event log**: `-e FILE` writes a hardware event log (binary, zlib). Layout
  and a decoder pitfall list are in `debugging.md` -> "Event log".
- **Savestate**: `` ` `` (backtick, `ui.save_state`) writes a state file that
  contains 68k RAM; two states taken seconds apart tell you whether the game
  loop is still advancing.
- **Symbols**: `out/release/symbol.txt` and `out/release/rom.out` (ELF). Map an
  address from a backtrace/PC against them.
- **In-ROM debug text is a last resort**: `VDP_drawText` in the stage loop draws
  into BG_A at *plane* coordinates (compensate with `camera.x >> 3`, modulo 64),
  costs a lot of frame time, and is easily overwritten by map streaming. It is
  what made an earlier session's "freeze" investigation noisy.

## Traps

- **Do not** run `blastem -h`, `-v`, or with no ROM argument: this build opens
  a GUI dialog on the user's desktop instead of printing to the terminal.
- **`-l` is not a writable log**: pointing it at a nonexistent file fails with
  `Failed to open <path> for reading`. It reads a listing, it does not write
  one. `-e` is the writable one.
- **SRAM is only flushed on a normal exit.** Back up and restore it around any
  test that writes saves:
  ```bash
  cp ~/.local/share/blastem/rom/save.sram /tmp/opencode/md-emu/save.sram.bak
  # ... test ...
  cp /tmp/opencode/md-emu/save.sram.bak ~/.local/share/blastem/rom/save.sram
  ```
  Slot 0 record: magic `RB` at file byte 8, stages mask at byte 10 (not 16),
  config at 48..55, items at 88+.
- `tools/emu.sh stop` quits through the Esc menu first; if that menu navigation
  fails (observed when focus is stolen) the fallback is an exact-name kill:
  `pkill -x blastem`. **Never** `pkill -f` with a pattern that also matches the
  command running it (it kills your own shell).
- `emu_start` prints `pids=... wid=...` into `$EMU_WORK/emu.state`; use
  `emu_window` rather than `xdotool search | head -1` (stale/recycled ids).
