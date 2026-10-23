# Handoff — Mega Drive port (2026-10-23)

Start here in a new session. `bugs.md` has the entries, `pc_gaps.md` is the
PC-parity backlog, `PLAN.md` the roadmap.

## Where the tree is

- ROM built, `make` exit 0, `make assets` clean, ROM fresh. Checks green:
  `check_palettes`, `check_vram` (unchanged), `audit_data` (`option gaps: 0`,
  `animation index problems: 0`).
- Last commits: `mega drive:` series ending with the AI/anim + scene-music
  batch. Only dirty files are the user's own (parent `game.cpp`,
  `build/games/RockDroid1/stages_v301.dat`, `TODO.txt`).

## Done this session (2026-10-23, all offline)

1. **Walk-attack / jump-attack** player rows + pose selection; **STAIRS_SEMI**
   (frame 18) half-body pose when grabbing a ladder.
2. **Boss door** opening: the PC art has one frame, so the sprite slides into
   the floor and back around the scroll.
3. **Dialog palette leak**: the panel borrows PAL3 (also the parallax line) —
   the whole line is saved/restored now, so the stage colours no longer change
   after a boss dialog.
4. **YOU GOT screen**: real `config_bg.png` background + text at the PC's x.
5. **Sprite lifetime**: `sprites_begin/sprites_end` bump `spr_gen` and every
   module releases only same-generation handles. This fixed the invisible
   player + `SPR_update` crash after re-entering a stage (`SPR_end` leaves the
   freed Sprite structs in the heap block the new pool reuses).
6. **AI parity**: AI_JUMP options (incl. TO_ROOF → ceiling walk for APE),
   AI_FLY options (TO_RANDOM/DRILL_DOWN/PLAYER_DIR/OPPOSITE_WALL/OPPOSITE_SHOOT/
   RANDOM_X/RANDOM_Y), the twelve AI actions that had no code, and enemy
   ANIM_TYPE_TURN rows (BLUE SHIELDER's rotating shield).
7. **SPIKED CHAIN** (TRAJECTORY_CHAIN) extends/retracts instead of flying on.
8. **Scene PLAY_MUSIC / STOP_MUSIC** wired (converter emits `scene_music_ids[]`).
9. Quake screen shake for enemy QUAKE projectiles; boss-death ring (earlier).

10. **Transient effects** (`src/effects.c`): dash dust, water splash, water
    bubble, teleport beam — art converted by `tools/convert_assets.py`, frame
    sizes verified by `tools/check_fx.py` (run it after touching either). Water
    only exists in stages 3 and 13, which is where splash/bubble can be seen.
    Effects are cosmetic: when the 768-tile sprite pool is full `SPR_addSprite`
    returns NULL and the effect is skipped, never a crash.

11. **Weapon tooltip** (`src/tooltip.c` + `src/tooltip_data.*`): the
    START-menu pick arms the PC's box + weapon icon + name, the stage loop
    draws them at the PC's lower-left spot for 1.5 s and then calls
    `stage_force_redraw()` once to restore the map cells. Nothing is drawn
    while the menu is open. Art: `tools/convert_tooltip.py` (box 300x32 ->
    8 tiles + a 38x4 map; icons 18 x 16x16, per-icon 11-colour palette).
    RAM/VRAM cost: 12 tiles at 912..923 (the dialog panel window) and PAL3 for
    the 90 frames (saved/restored, colourcycle paused). `convert_tooltip.py
    --preview N` rebuilds the box + icon N from the emitted data; the
    converter also round-trips its own packed tile bytes on every run.

## Next, in order (see pc_gaps.md for evidence per item)

1. **Quicksand terrain** — deferred on purpose: no converted stage uses it
   (PC: 0.2px/frame sink + halved move speed), and `TERRAIN_DOOR` has no PC
    rule to port. Audited dead PC paths are closed in `pc_gaps.md`: scene
    `SHOW_ANIMATION` (its only op sits in scene 1, which no game-scene type
   maps to), `_dark_mode`, the quake-attack half and the white hit flash.
2. **Small real leftovers** (`pc_gaps.md`): the PC's special tooltip names for
   the tanks/COIL/JET, the "characters unlocked" warning before players 3/4,
   and the PC's extra water/ice movement rules.
4. Play-test everything above (ask before launching BlastEm; one run covers:
   walking shots, door slide, dialog → stage colours, YOU GOT screen, stage
   re-entry (no invisible player / no crash), APE ceiling jump, BLUE SHIELDER
   turn animation, SPIKED CHAIN, cutscene music, dash dust on every slide,
   water splash + rising bubble in stage 3 (the only early stage with water;
   13 also has it), teleport beam when a boss/castle teleporter warps, and the
   weapon tooltip after picking a weapon in the START menu — check the box,
   the icon and the name appear in the lower left for ~1.5 s, that the name
   sits on the box interior (not in black boxes) and that the map under it is
   clean again afterwards (the tooltip itself belongs to the map while it is
   up, and the background palette is the tooltip's for those 1.5 s).
   Effects are best effort on VRAM: if the 768-tile sprite pool is full the
   effect is silently skipped (SPR_addSprite NULL) — worth watching that the
   splash and the beam do appear rather than assuming.

## Reminders

- **Verify builds by `make`'s exit code** — the toolchain reports `erro:` in
  Portuguese, lowercase (lesson 26 in `lessons_learned.md`).
- `tools/audit_data.py` now prints AI **options**, object types, terrain and
  sprite animation indices: run it before claiming a feature is complete.
- A fully transparent sprite frame is fine (rescomp emits 0 VDP sprites).
- Sprite pool 768 tiles; ring explosion uses 72 of them.
