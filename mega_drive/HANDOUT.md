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

- All nine selectable stages play (maps, parallax, enemies, objects, shots,
  HUD, menu, save/load, stage select). Bosses and music still out.
- **Stages 0..13 are converted** (14 of 20) and **all reachable**: 9..13 are
  castle #1 (LAVA PIT, SPACE BASE, POWER GRID, RECYCLE FACTORY, FROZEN LAIR).
  The select's centre slot becomes the castle once intro + 8 bosses are
  beaten (PC `can_access_castle`); it returns the first unbeaten castle stage,
  and a castle #1 stage shows the castle screen before the boss intro.
  `tools/add_stage.py N` does a whole stage; the asset recipe loops over
  `STAGES`. VRAM bases are derived per map (tileset -> bg -> anim), so FROZEN
  LAIR map 1's 202-tile tileset fits. ROM 1.31M. FROZEN LAIR is the palette
  outlier (56 colours, BUG-004) and the first WATER/ICE stage (minimal
  physics). Endings 2..5 play after stage 13 (`scene_play(2+player)`); the
  six portraits are in the build (visual pass pending).
- **Title** renders the layered PC art at ~58 colours over four palette lines
  (`tools/title64.py`), verified on hardware against the user's reference.
- **Colourcycle** (CRAM rotation instead of tile-frame animation) live in stages
  1, 2, 3, 6, 8; art width decides the technique (16 px strip = one frame).
- Room transitions: **no fallbacks left** (`make check-vram` reports 0).
- Bugs: BUG-001 fixed, BUG-002 closed by design, BUG-003 fixed, BUG-004 tracked
  (palette budget; DEEP JUNGLE is the outlier at 33 colours for 15 slots — every
  build prints a weighted error per stage).
- **Intro scene** (2026-10-19): plays the PC script 0 between the logos and the
  title (text pages + city parallax). START skips it. Emulator-verified
  2026-10-19: logos -> "20XX A.D." centered -> parallax city -> typed text ->
  START -> title. `scene_text_pos.position` must be read at byte 68 (64 is
  transition_type; reading it put every page bottom-left).
- **Title finished** (2026-10-19): layered art in `art/` — `title.png` (18,4),
  `sub_title.png` (162,38), `title_rock_and_beta.png` (148,48) — wired as
  `--part ...:0:1`, `:0:1`, `:1:1`. 17 colours, 0 folded tiles. Three `--rekey`
  lines in the Makefile brighten the pink and orange armour and warm the skin;
  the art files themselves are untouched.
- **Drops + progress** (2026-10-19): regular enemies drop PC-ratio pills (big/
  small energy, big/small weapon, 4.5s lifetime); a new SRAM item record
  (byte 88 + slot*12) persists lives, the three tanks and armor across stages;
  first clears of stages 1..8 show `YOU GOT <weapon>` for 5s.
- **Items + armor** (2026-10-19): menu has the PC 4-column weapon grid using
  the original 8x8 icon art (`menu_icons.png`, shared 15-colour palette;
  hand-owned source is `images/menu_icons.png` and overrides the converter);
  COIL FROG and JET EAGLE are selectable (stage 1 / 6), with item physics and
  ammo drain; ARMS armor swaps the full-charge shot for
  the per-player projectile (SUPER SHOT / SEMI / LASER / HADOUKEN); armor
  pickups show the CANOTUS dialog; SHORYUKEN (UP+DASH) is implemented. The
  save selector no longer draws "SLOT N" text over the icons (PC has no slot
  labels). Exact item-platform motion still needs playtesting.
- **Title/config** (2026-10-19): title menu has NEW / LOAD / CONFIG / ABOUT;
  CONFIG reuses the options screen; ABOUT shows a compact credits page.
  CONFIG now persists `game_finished`; only two players are selectable until
  the stage-13 ending plays, then all four (PC select_player).
- **AI + sub-bosses** (2026-10-19): AI actions 7 (invert shot), 8 (air walk),
  9 (fall to ground) and 18 (jump-attack) added; walk options 3 (saved point),
  5 (random direction) and 6 (opposite/away from screen centre) supported.
  Enemies carry `is_sub_boss` and `respawn_delay`; respawning enemies revive at
  their spawn point after the PC delay, sub-bosses always drop big energy, and
  boss doors wait until the boss/sub-boss left of them is dead.
- **Projectiles + weaknesses** (2026-10-19): player FOLLOW / CENTERED /
  ZIGZAG / BOMB / FREEZE / LASER trajectories implemented (SONAR bounces on
  solid tiles, DESERT STORM homes, MAGIC STAR tracks the player then flies,
  TIMED BOMB/DYNAMITE fuse + burst); enemy SIN / BOMB / FREEZE / CENTERED /
  ZIGZAG / FALL_BOMB no longer fall back to straight. Every enemy now carries
  the PC `weakness[9]` multipliers, and shots use the selected weapon's
  entry. Bomb bursts still use the small explosion puff, not the PC 56px art.
- **Map/stair fixes** (2026-10-19): links now carry the PC band width
  (`file_link.size`), reverse direction (`bidirecional`) and type, so the wide
  stairways in DEEP JUNGLE and SUN-GOD REALM trigger; door columns are added
  to the scroll lock at runtime and the SUN-GOD sub-boss doors moved up one
  tile with the map crop; falling through a stair now uses the PC
  `middle_y_point_lock` rule instead of the body centre. Reflected shots are
  harmless after the bounce and charged shots ignore shields (PC
  `vanishes_on_hit`); shielded enemy shots bounce off the player shield.
- **Death/clear flow** (2026-10-19): deaths freeze the world for 1.5s with
  explosion puffs, then respawn at the checkpoint; out of lives shows GAME OVER
  -> PRESS START (lives reset to 5, saved). Boss death plays a 2s explosion
  ring before the clear screen. `runStage` now returns LEFT/CLEARED/GAME_OVER and
  `playSession` sends clears/game overs to the stage select (intro stage game
  over restarts it; stage 13's ending returns to the title). Approximations:
  no PC ring-explosion art, no player_victory flash/pose, no fade.
- **Ending parade + credits** (2026-10-19): after the stage-13 scene the
  port plays the PC walking parade (the three `player_walking_layer*.png`
  pieces on two planes, the two front layers split by `HSCROLL_LINE` so they
  move at different speeds and the player sprite walks off-screen), then a
  boss parade (one boss sprite + stage/boss name per defined stage) and the
  scrolling `game_credits.txt`. START skips each part. The PC's full normal-
  enemy line is approximated by the bosses; the walking art is 15-colour
  decomposed (see `HANDOUT` ending notes).
- **Boss presentation** (2026-10-19): the stage boss now drops in from 160px
  above its spawn when it first reaches the screen; the boss dialog and HP row
  wait for the drop to finish and shots cannot damage it mid-drop. The PC
  white blink and intro-sprite animation are still approximated by the drop.
- **Water jump** (2026-10-19): jumping while the feet tile is WATER now uses
  the PC 0.9x initial speed with the 0.3x gravity already in place.
- **Status/feedback** (2026-10-19): DYNAMITE now arms the PC 3.5s freeze
  (weak enemies stop AI and are hidden, enemy shots clear) and the projectile
  lives as long as the effect; QUAKE shots shake both planes by the PC +/-2 px;
  zig-zag/laser projectiles flip their sprite when they reverse. ABOUT now has
  the PC site/bugs/Twitter/Facebook links, and EXTRAS has a CREDITS entry that
  runs the same scroll as the ending.
- **Transition/heap** (2026-10-20): DMA queue 500 -> 192 freed enough heap
  for the destination `MAP_create` (the Mummy stair fade was an out-of-memory
  fallback, fail code 3); the transition hscroll sign was fixed, stage_load
  starts at the exact arrival camera, and the post-load arrival redraw is
  skipped (it was the one-frame dark/orange flash). Normal display-off stage
  load remains for transitions.
- **Animation/objects** (2026-10-20): animated cells are rewritten to frame 0
  during stage_load, so Daisie's palms no longer freeze on one tile until the
  first animation tick; map objects now use PC `object::gravity()` (all types
  except the PC exclusion list) while on screen.
- **Overlapping map tiles** (2026-10-20): when PC tile1 and tile3 both
  cover one cell, the converter now keeps the tile with fewer transparent
  pixels (the one that fills the cell). This fixes Daisie's one-tile waterfall
  gap; Daisie palms also use their frame-0 animated cells from the start.
- **Water SFX** (2026-10-20): entering/leaving water plays the PC
  `water_enter` / `water_leave` samples; both are also in the EXTRAS SFX test.
- **More SFX** (2026-10-20): door opening, got-weapon and stage-select
  confirmation now use the PC samples; all are in the EXTRAS SFX test.
- **Door / weapon / stage-select SFX** (2026-10-20): boss doors, the
  first-clear weapon screen and stage-select movement/confirm now use the PC
  samples.
- **HUD weapon colour** (2026-10-20): the weapon gauge row is drawn with the
  player palette (PAL2); its green fill is mapped to palette index 13, so it
  follows the selected weapon/player colour instead of staying green.
- **Ladder/object fixes** (2026-10-20): bottom-of-map links now fire while
  the player is on a ladder or standing at the bottom edge; object gravity
  matches PC passability (only UNBLOCKED/WATER/SPIKE are passable, so boxes
  stop on stairs) and excludes all active-platform/block types. The
  overlapping-tile rule is restricted to two animated tiles (waterfall).
- **Block timers / lava** (2026-10-20): disappearing-block #1/#2/#3 now use
  their PC `obj_timer` as the initial hidden delay (1000/2000/3000 ms), so
  they no longer toggle in sync; active blocks use their own visible timer.
  Spike/lava terrain damage is now lethal (PC 188), not 4.
- **Menu polish** (2026-10-20): title defaults to LOAD when a save exists;
  the weapon menu shows a gauge bar, tank counts (E/W/S) and restores the
  panel before redrawing text so old glyphs no longer show through.
- **Menu/select follow-up** (2026-10-20): stage select defaults to the centre
  slot; the weapon menu borrows the panel's dark backdrop for CRAM0 while
  open, so font background pixels no longer create coloured text boxes.
- **Dialog/background + stage6** (2026-10-20): dialog boxes use system tile
  15 (tile 935 overlapped an animated slot and was overwritten); the
  overlapping-tile opacity rule is stage-2-only so Dynamite's map strips are
  untouched.
- Last commit: dialog background tile and stage6 overlap scope.
- Boss dialogs are in: shown when the boss first appears (stages 0..8).
  Stage-start dialogs are dead PC data (no reader, cutscene_pre/pos = -1).
- Audit (`tools/audit_data.py`): every stage reports clean except the WATER
  note (stage 3, and 13's water/ice). Armor/tanks/destructible/boss doors/
  beams/LR platform/castle teleporters and the missing AI actions are in.
  Approximations are listed in stage0_plan's "Data audit" section.

## Next work, in order

### 1. Ending — IMPLEMENTED (needs a visual pass)

PC scene lists 2..5 are the endings (Rockbot/Betabot/Candy/Kitty). They use
`MOVE_IMAGE` ops with 68x68 portraits over a clear-area colour, then text
pages. `scenes_data.c` already holds their op lists/text; `scene.c` skips the
image op.

Done (commit pending): `tools/convert_scenes.py` bakes the six ending
portraits (ids 6/8/9/10/11/12) to `res/scene_img*.png`, img2tiles decomposes
them, and `scenes_data.c` now has the `SceneImage` table. `scene.c` draws them
on BG_B (PAL2/PAL3), uses PAL1 for `clear_area` and keeps the text overlay on
BG_A (blank outside the box). Stage 13's clear plays `scene_play(2+player)`
before returning to the title. Lab-scene art (records 1-4/7) and animated
image movement are still unported. The rest of this section is the design it
was built from:
- BG_B = background/images, BG_A = text overlay. `clear_screen` fills BG_A
  with the blank tile (pixels 0 = transparent) and BG_B with the black tile.
  `text_play` keeps filling its own box with the black tile (PAL0 entry 1);
  outside it BG_A stays transparent and the art shows through.
- `clear_area` uses PAL1: set PAL1 entry 1 to the PC RGB and fill a BG_B
  rect with the solid tile. (PAL0 stays text/black.)
- Images use PAL2 and PAL3 (two concurrent max; endings never show 3:
  scene 3's second clear-area wipes before the third). Reset the line/tile
  allocation on CLEAR_SCREEN/CLEAR_AREA; load each image's tiles at
  `TILE_USER_INDEX` + running offset, draw its 9x9 map into BG_B at
  `x>>3, y>>3`.
- Art: `dr_destrin`, `rockbot`, `rockbot_left`, `betabot`, `candybot`,
  `kittybot` (68x68, 1.2k-3k colours) through `img2tiles.py` at 15 colours
  into `res/scene_img_*.png` + generated tiles/map; emit a `SceneImage`
  table from `tools/convert_scenes.py` (it currently skips those records).
- `MOVE_IMAGE` movement: ini -> dest by speed_x/y (sceneshow.cpp:160), step
  every `move_delay`; endings are static (ini == dest) except none, so the
  static draw is the requirement. If movement is added, erase the old rect
  with the clear-area tile before drawing the next (uniform background).
- Trigger: after stage 13's stage-clear, run scene `2 + player` (PC
  `game_scenes_map[ENDING_PLAYER1..4]`), then the title. No fades exist in the
  PC player — do not invent them.

### 2. Scenes — intro DONE; show-image is the ending's blocker

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

Done (commit `14f24cf2`): `tools/convert_scenes.py` emits the op lists, text
lines (verbatim, 8 px spacing) and clear areas to `src/scenes_data.*`; the
intro's two parallax layers are baked to 512 px and decomposed by
`img2tiles.py`. `src/scene.c` plays clear / text / clear-area / parallax and
aborts on START. `main.c` is now `showIntro() -> scene_play(0) -> showTitle()`.
Audio ops are stubbed; show-image / viewpoint / animation are skipped (their
art needs the colour-reduction pass: the 68x68 characters have 2-3k colours).
Next: that art pass, then the ending scenes (2..5) and the lab scene (1).

**User's decisions**: port the **intro** first, **text verbatim**, and it is
fine to **tighten the line spacing** to fit 224 lines.

### 3. Stages — done (0..13 reachable)

Castle flow is in: select centre unlocks when save bits 0..8 are set, shows
the castle stage's names, returns the first unbeaten castle #1 stage, and
`boss_intro_run()` draws `classic_style_castle.png` (four palette lines via
`tools/title64.py` + `tools/crop_image.py` bands) for 3s before the boss
intro. `save_slot_set_stage()` now records the *played* stage (it used to
hardcode 0, so boss stages never set their own bit).

Stages 14..19 are **placeholders** in the PC data (`Fire Bot`, `Stage [16]`,
…), and `game_properties_v301.dat` has `use_second_castle = 0`, so they are
not part of the game. Do not convert them unless the user says otherwise.

### 4. Ending — DONE (visual pass pending)

Scene lists 2..5, the three-layer walking parade, boss parade and credits are
in (`src/ending.c`). The PC full normal-enemy line is approximated by the
stage bosses. `game_finished` is set before the ending and unlocks players 3/4.
Known approximation: the walking layers are pinned to the MD crop/tile grid
(row 8 / row 15) and use a 320px repeat instead of the PC's exact layer offsets.

### 5. Options / config — DONE (2026-10-19)

Implemented in `src/config.c` with the user's simplifications: AUDIO
(SOUND ON/OFF), INPUT (TURBO, AUTO-CHARGE, BUTTONS — no analog/joystick),
EXTRAS (music/sfx test), LEAVE STAGE; no platform/language/performance/quit.
Opened with B from the in-game menu; stored in SRAM bytes 48..55.

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
