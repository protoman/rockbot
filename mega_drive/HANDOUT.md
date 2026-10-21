# Handout — next session

Briefing for whoever picks this up. **The authoritative docs are
`stage0_plan.md` (milestones, conventions, gotchas) and `roadmap.md` (open
work, in order)** — read those first; this file is the short version.

## Build & run

```bash
cd mega_drive
make run          # builds, then runs in BlastEm
```

`make run` builds and then runs `tools/emu.sh start-ready out/rom.bin`, which uses
the **native BlastEm 1.0.0** at `/mnt/ExtraData/programas/blastem64-1.0.0/blastem`
(the flathub flatpak is gone — its sandbox under Wayland wedged on startup). The
harness kills a previous instance and relaunches while the window comes up black,
so a bad start no longer needs a manual kill; there is no auto-timeout, so stop it
with `tools/emu.sh stop`. `-g` (software rendering) is still the default — it was
required for the flatpak; for GL on the native build use `make run EMU_ARGS=`.

ROM is `out/rom.bin` (== `out/release/rom.bin`), 1.0 M. Kega Fusion also works.

## Test harness

`tools/emu.sh` — source it, or use the CLI:
`emu_start [rom]`, `emu_start_ready [rom] [tries] [wait]`, `emu_stop`,
`emu_status`, `emu_window`, `emu_key KEY [ms]`, `emu_keys_up`, `emu_shot NAME`,
`emu_shot_blank`. BlastEm's keymap: `z` = pad A, `x` = pad B, `c` = pad C,
`enter` = START, arrows = d-pad. Screenshots go to `/tmp/opencode/md-emu/`.

Two things that waste time if forgotten:
- The emulator wedges with a black screen often. `emu_start_ready` retries;
  `emu_shot_blank` detects it. It is not the ROM.
- BlastEm sometimes misses a keyup, leaving a pad button stuck so the game
  spins in its "wait for release" loop (looks like a frozen menu). Call
  `emu_keys_up`.

### Reaching a specific stage or boss (learned the hard way)

Add a **temporary** hook at the top of `main()` that boots straight into a
stage, with the index in SRAM (word 200 = stage, low byte; word 206 = player):

```c
    save_init();
    { u8 tstage = (u8)(SRAM_readWord(200) & 0x0F);
      current_player = (u8)(SRAM_readWord(206) & 0x03);
      if (tstage <= 8) runStage(tstage); }
```

Seed it from the host (the file BlastEm actually uses):

```
~/.var/app/com.retrodev.blastem/data/blastem/rom/save.sram
slot 0 at byte 8: magic u16 0x5242, stages u16, player u16 (big-endian)
```

**Do not warp the player to a boss** — map 0 has a scroll-lock column at 179
(x=2864) that stops the camera at ~2557, so the arena is never visible. Instead
add a temporary **link** near the spawn (`src/stage0_data.c`, e.g.
`{ 0, 10, 8, 0, 182, 3 }` + `link_count` 2→3) or a second boss teleporter, and
walk into it: a link calls `stage_arrival_camera()` → `scroll_lock_span()`,
which lands the camera past the lock. That is how the boss fight was verified.

**Always revert temp hooks before committing** (and note that
`/tmp/opencode/main.c.pre_playtest` is a hook-free copy).

## Gotchas

- `make` re-runs the asset pipeline when anything in `tools/*.py` or
  `images/*.png` changes, **regenerating** `src/*_data.c` and
  `src/stage*_enemies.c`. Hand-edits to generated files do not survive.
- Deleting a `res/` PNG that `rescomp` still lists leaves a stale dependency —
  `rm -rf out/release/_deps/res out/release/res` if make complains.
- `stage_defs[]` needs `__attribute__((used))` or LTO drops stages 1..8.
- MD CRAM word is `(R<<1)|(G<<5)|(B<<9)` (white `0x0EEE`).

## What was done in the last session

All 9 selectable stages converted (was 3); weapons (buster + 8 boss weapons,
per-weapon palette/ammo/projectiles, menu switching); shield (ROCKBOT,
DOWN+A); enemy shield types; real HUD (weapon gauge, lives); plus fixes:
stage-start spawn position (was spawning on the ceiling / losing the player),
projectile flip when firing left, platform crush kill, large projectiles dying
at spawn, enemies spawning inside the floor, walkers turning in place at
cliffs, conveyor not carrying you into pits, the sprite flash on a room change,
and BlastEm's startup freeze. Commits `7c689221` … `aa4abaee`.

Verified in-emulator: spawn in all 9 stages, boss fight (MILONGA BOT renders,
walks, fires, HP row drops), shield frame, weapons' projectiles/palettes.

## Open work, in order

### 1. Room transition — the real scroll (designed, not started)

Today a link does a palette fade (safe, no more one-frame map swap). The PC
scrolls the old screen out and the new one in vertically
(`game.cpp:1067+`). Design, with the measured numbers, is in
`stage0_plan.md` → "Room transition":

- Build the temporary combined tilemap in RAM (40x56 = 4480 bytes), write the
  visible 40x28 slice to the plane each frame while sliding. Enemies/objects
  simply do not update during the transition.
- Only the two screens' tiles are needed. The new screen's tiles are copied to
  a spare VRAM base and its plane entries written with that offset; the old
  screen keeps base 16. The animated-tile block (496..927) has ~120 tiles of
  slack in the worst case.
- Blocker to know: each map has its **own tileset in the same 128-tile VRAM
  block** (stage tiles 16..143). A stage's three-map tile union is up to 193
  tiles (137/139/156/193/91/168/108/71/86 for stages 0..8). The alternative is
  one union tileset per stage shared by its three maps, which needs a
  converter + `resources.res` change (map PNGs become index maps into a
  separate tileset PNG — rescomp supports that).

### 2. Waterfall gap

Stage 2, map 2, **columns 133/134, row 7**: those two cells are *static* water
tiles inside an animated column (rows 0..6 and 8..14 are anim cells; columns
135/136 are fully animated). The PC's data is identical (`init_animated_tiles`
uses the same rule as our converter), so this is a *rendering* difference, not
a converter bug — compare with the PC at that spot; the anim art may span two
cells.

### 3. Still unverified (needs play)

Weapons in a stage where you legitimately own one (beat a stage first), spike
damage, the platform crush (stage 6 + a ceiling), the bosses for stages 1..8
(only MILONGA BOT was watched), the LARVA-in-the-ground report (both FLOWER
FIELDS LARVA spawns now sit correctly, likely already fixed).

### 4. Roadmap leftovers

`roadmap.md` items 11/12 (HUD ball dedup, colour cycling) — only worth it when
VRAM/ROM pressure bites; 10 (ending) needs the castle stages + final boss
first; 9 (music) is blocked on tooling (MOD→VGM, or NeuralNote needs a CLI
mode and a rebuild).
