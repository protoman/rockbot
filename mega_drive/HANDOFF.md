# Handoff — Mega Drive port (2026-10-22, end of day)

Start here in a new session. `bugs.md` holds the full entries; `PLAN.md` holds
the roadmap; this file is the short state + what to do next.

## Where the tree is

- ROM built: `out/rom.bin` (normal build, no profiler), last commit
  `736867c0`.
- Today's commits, newest first:
  `736867c0` sprite-VRAM regression fix · `c28e1776` boss intro row + sub-boss
  revive · `8f2ad399` YOU GOT layout + autoscroll edge death · `b3d6feb8`
  jump/teleport rows + teleport facing + eshot sprite release · `92b9f829` door
  scroll sprite sync + stage-wide checkpoint + dialog panel padding ·
  `86037374` hand-drawn anim strips · `17f88398` animated tiles as one 4-tile
  block per slot (SEAHORSE 30 fps → 60) · `96537812` fish water rule + HUD bar
  frames + anim plane window · `b7380c19` transition heap fix · `bc13e1b0`
  water constraint + block/beam/break SFX.
- Only dirty files are the user's own (parent `game.cpp`,
  `build/games/RockDroid1/stages_v301.dat`, `mega_drive/TODO.txt`, a kate swap
  file). Do not commit those.

## Confirmed by play-test (do not regress)

- SEAHORSE tileset/fade/corruption after links (BUG-022/023/024).
- DESTRIN TANK → SHIP on death.
- Fish staying in water (BUG-018).
- HP bar fill range (BUG-032) — see the colours item below, that is different.
- Animated tiles across the whole map (`stage_map_patch` windowed coords).

## Just fixed, needs a re-test

**Sprite VRAM regression (`736867c0`)** — the invisible player + ADDRESS ERROR
in SEAHORSE came from the new jump/teleport sheet rows for *normal* enemies:
crowded rooms went 3 → 5 sheet rows and blew the 504-tile sprite pool.
- Extra rows are now emitted for bosses/sub-bosses only
  (`convert_enemies.py`, `boss_or_sub`).
- Sprite pool is `SPR_initEx(768)` (was 504) in `main.c`, `bossintro.c`,
  `ending.c` ×2, `saveselect.c`, `stageselect.c`; `tools/check_vram.py` uses
  `sprite_base = 1440 - 768`. Stage layout ends at 573; check_vram reports only
  `stage2 map2 tight (99 free)` which is still above the 82 it needs, so no fade
  fallbacks.

If the crash or the invisible player persists, the next step is to print the
sprite pool use (`SPR_getUsedVRAM` style counter) for SEAHORSE map 0, not to
guess.

## Open reports from the 2026-10-22 play-test (in the user's order)

1. **BETABOT colours wrong** (no charge state involved) —
   `res/enemy_betabot.png` + PAL0. Compare the sheet palette against
   `tools/pal_ui.py`; the art is imported by `tools/import_pc_art.py`. Look for a
   sheet whose palette differs (`convert_enemies.py` already warns "art PAL0
   differs from tools/pal_ui.py").
2. **HP bar broken (colours)** — `set_bar()` frame maths is correct
   (`pct*50/100`, 0 = empty); the bar parts are drawn with **PAL2** (weapon
   palette, `rebuild_player_palette()` writes 13/14/15 there). If the bar looks
   wrong right after a weapon change or a hit, PAL2 is the cause, not the frame.
   Boss bar is PAL0.
3. **Checkpoint does not animate when activated** — PC uses a *separate*
   animation object: `classMap::add_animation(ANIMATION_LOOP, &graphLib.checkpoint,
   pos, offset, 100, 0, ...)` on touch (`object.cpp`). The port only flips
   `o->started` and relies on the object's own frame list (`object.c` type 29).
   Port it as an animation (the `explosion` system in `explosion.c` is the
   closest pattern) instead of touching `is_looping()`.
4. **Slowdown in CHAOS CITY near the active platforms + small pill** — very
   likely the same sprite-VRAM pressure as the regression above; re-measure
   before adding code. `make profile` (MD_PROFILE) prints per-subsystem
   subticks/frame (1280 = one 60 Hz frame) and boots straight into the stage
   hardcoded in `main()` under `#ifdef MD_PROFILE` (currently stage 3 — change
   it to 0 for this test).
5. **Dialog text background** must be the panel colour; the A-button arrow must
   be white on a transparent background (`screenshots/bugs/boss_dialog_text_bg.png`).
   Suspects: `dlg_btn_a` art (colour index 0 / colorkey) and PAL3: the panel and
   the arrow share PAL3 (`dialogs.c`). The arrow's black pixels are probably
   drawn instead of left transparent.
6. **Explosions must move away from the centre** — PC animation frames expand
   outwards/directionally (`classmap.cpp` death animations, `graphLib.explosion32`
   / `bomb_explosion_surface`); the port's `explosion.c` spawns a puff centred on
   the body and never offsets frames. Fix = per-frame offsets from the PC anim
   data, not a new art.
7. **Stage-select font garbled** (`screenshots/bugs/stage_select_bad_font.png`) —
   BUG-014. `draw_names()` restores the bar rows from `cells[]` and `PAL0` is
   reloaded, so the suspect is the **font tile area**: the select loads
   `ssel_tiles_data` at 16..210 and the dialogs load the panel at 912..928, but
   the stage-clear text screen and the boss dialog both use `TILE_FONT_INDEX`
   (1440). Check whether any screen writes tiles above 1440 or changes the
   plane size (`VDP_setPlaneSize`) before the select draws.
8. **Per-stage intro scene missing before a SEAHORSE start** (only music) —
   the port plays `SCENE_OPENING_UNBEATEN` once (stage 0) and skips the
   per-stage intro scene. `scenes_data.c` / `scene.c` are the player;
   `tools/convert_scenes.py` generates from the PC tables (check whether the
   per-stage intro scene lists were converted).

## Next session, suggested order

1. Re-test the sprite-VRAM fix (crash + invisible player + CHAOS CITY slowdown
   in one run). Ask before launching.
2. Checkpoint animation object (item 3) and the dialog text/arrow (item 5) —
   both self-contained.
3. BETABOT colours (1) and HP bar palette (2) — compare PAL0/PAL2 write order
   with `tools/pal_ui.py` first, offline.
4. Explosion frames (6), stage-select font (7), per-stage intro scenes (8).

## Reminders

- Offline first: `make`, `tools/check_vram.py`, `make profile` for timings,
  `tools/audit_data.py` for data coverage. Ask before every BlastEm launch
  (`blastem_debug.md`).
- `make assets` regenerates `src/stage*_enemies.*`, `res/*` and now also
  rewrites the matching `SPRITE` lines in `res/resources.res` (the enemy sheets
  and their animation lists must stay in sync).
- Sprite pool is 768 tiles now; if a new sheet makes `SPR_addSprite` fail the
  symptoms are invisible sprites and ADDRESS ERROR crashes, not compile errors.
