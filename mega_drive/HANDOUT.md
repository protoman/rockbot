# Handout — next session

Briefing for whoever picks this up. **Authoritative docs: `stage0_plan.md`
(milestones, conventions, recipes) and `roadmap.md` (open work, in order);
`bugs.md` for deviations and `lessons_learned.md` for traps.** This file is the
short version.

## Build & run

```bash
cd mega_drive
make                 # assets (auto when tools/art change) + ROM -> out/rom.bin, ~896K
make check-vram      # VRAM budget + colourcycle pool guard + transition fallbacks
make run             # builds, then ONE BlastEm launch (tools/emu.sh start)
make run EMU_ARGS=   # GL renderer instead of software
```

- Emulator: native BlastEm 1.0.0 (`tools/emu.sh`, `EMU_BIN`). No retry loop and
  no readiness wait any more — the native build does not wedge.
- **Launch discipline (user rule): never launch the emulator without saying so
  first**, one deliberate run per question, and stop it when done
  (`tools/emu.sh stop`). The user types while the window opens and keypresses
  reach the emulator.
- **Check the build really happened** before trusting a capture: `make` must
  exit 0 (the toolchain prints `Erro` in Portuguese, so grepping for "error"
  misses failures) and `out/rom.bin` must be newer than the newest source.
  Capture scripts must take the ROM path as an argument — a hardcoded path cost
  two wrong conclusions last session.

## State at hand-off

- All nine converted stages play (maps, parallax, enemies, objects, shots, HUD,
  menu, save/load, stage select). Bosses and music still out.
- **Title** renders the layered PC art at ~58 colours over four palette lines
  (`tools/title64.py`), verified on hardware against the user's reference.
- **Colourcycle** (CRAM rotation instead of tile-frame animation) live in stages
  1, 2, 3, 6, 8; art width decides the technique (16 px strip = one frame).
- Room transitions: **no fallbacks left** (`make check-vram` reports 0).
- Bugs: BUG-001 fixed, BUG-002 closed by design, BUG-003 fixed, BUG-004 tracked
  (palette budget; DEEP JUNGLE is the outlier at 33 colours for 15 slots — every
  build prints a weighted error per stage).
- Last commit: `4c16ef5f`, tree clean.

## Next work, in order

### 1. Title colours — waiting on the user's art

The user is redrawing `art/title_rock_and_beta.png` with a **15-colour palette**
so it can live on **one palette line**. Wire it as `:1:1`:

```
--part art/title.png:18,4:0:1
--part art/sub_title.png:162,38:0:1
--part art/title_rock_and_beta.png:148,48:1:1   # <- was 1:3
```

Why it matters: each 8x8 tile chooses its palette line, so colour-clustered
lines make neighbouring tiles use different shade sets — the "square patches"
the user reported. One line for the whole characters image removes that
completely, at the cost of flatter shading. `title64.py` now prints per-part
`N colours in M slots`; today it warns `part 2: 56 colours for 45 slots`.

Pick the 15 colours for *recognition* (armour base+shadow, skin, hair, grey,
black outline, white highlight), all on the MD ramp (`0 36 73 109 146 182 219
255`). Slot 0 is transparent, so 15 visible colours = "16-colour" art.

### 2. Scenes — port the intro (format mapped, nothing built yet)

The PC stores a scene as a **script of operations**, one table per op type,
assembled by name (`file/v4/file_scene_v4.h`, `sceneslib.cpp`,
`scenes/sceneshow.cpp`). Ops carry `blocking` (parallel allowed) and
`move_delay`. Data in `build/games/RockDroid1/scenes/`:

| file | size | contents |
|---|---|---|
| `game_scenes.dat` | 16 B | scene-list head |
| `scenes_list.dat` | 3072 B | scene records |
| `scenes_show_image.dat` | 1404 B | image + ini/dest pos, copy area, delay |
| `scenes_show_text.dat` | 1368 B | text file, position type, speed |
| `scenes_show_viewpoint.dat` | 104 B | viewport pans |
| `scenes_show_animation.dat` | 420 B | sprite animation |
| `scenes_clear_area.dat` | 44 B | colour-fill rectangles |
| `scenes_parallax.dat` | 552 B | parallax setup |
| `scenes_play_music/sfx.dat` | 180/64 B | audio cues (stub: no audio yet) |
| `scenes/text/*.txt` | — | the dialog text |

Plan: (a) `tools/convert_scenes.py` — tables to C, referenced art through the
four-line pipeline, text packed into one blob; verify offline by dumping the
intro scene's op list. (b) `src/scene.c` — player for show image (with fades),
show text, clear area, wait, blocking/parallel; viewpoint/parallax/animation
after. (c) wire `showIntro()` -> intro scene -> title.

**User's decisions**: port the **intro** first, **text verbatim**, and it is
fine to **tighten the line spacing** to fit 224 lines.

### 3. Remaining stages (11 of 20)

Converters are stage-parameterised (`convert_stage.py N`, enemies, objects,
`img2tiles` for the parallax). Write a script that adds a stage end to end:
converters, resources block, PAL3 pool budget, defs table, stage-select list.
Validate with `check-vram` and the palette error report — no emulator needed.
ROM is ~896K of 4MB, so ~100K per stage fits.

### 4. Ending

Falls out of the scene player plus a credits sequence; `ending/` art exists.

### 5. Options / config — **STOP AND ASK THE USER FIRST**

The user has the spec mapped and wants to hand it over. Do not start this
without it.

### 6. AI / bosses

`EnemyAi` tables + `eshot.c` exist; the port's enemies are much simpler than the
PC's `artificial_inteligence.cpp`. Incremental per enemy type; needs the user's
playtesting.

## Tools

- `tools/title64.py --part FILE:X,Y:FIRST:COUNT` + `--preview out.png` — the
  preview rebuilds the screen from exactly the emitted data; use it before any
  emulator run.
- `tools/check_vram.py` — tile budget, colourcycle pool guard, transition
  fallbacks.
- `tools/convert_stage.py N` — prints the stage's palette error.
- `tools/emu.sh {start|stop|shot|keys_up}`.

## Traps from last session (all in `lessons_learned.md`)

- `PAL_setColor()` / `PAL_getColor()` take a **global CRAM index** (PAL3 entry
  12 is CRAM 60); the tilemap's palette field is the 2-bit *line*, `<< 13`.
- A **queued CRAM DMA does not land** here: palettes with `CPU` writes, tiles
  with `DMA`.
- **A preview that shares the emitter's layout bug validates itself** — derive
  the preview from the emitted layout, not the intermediate state (#27).
- PC art backgrounds may be a **near miss of the colorkey** — `(72,126,124)` vs
  `(75,125,125)`; flood-fill with tolerance (`title64.py`).
- **One palette line per 8x8 tile** is the hardware rule: split art
  *spatially* (parts), not by colour clustering.
- Emulator palette readback differs from the MD ramp by ±10-15 grey levels
  (BlastEm's 3->8-bit expansion) — that is not a bug.
