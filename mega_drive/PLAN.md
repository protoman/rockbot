# Port plan (roadmap)

Working list for the Mega Drive port. Derived from `pc_features_vs_md.md`
(refreshed 2026-10-22). One line per item, with the state kept current; the
detailed traces live in `bugs.md` and `HANDOUT.md`.

## 1. Water / terrain constraints

- [ ] Water enemies must not leave water (and land enemies must not enter it):
      PC `character.cpp:1921-1923` + AI move checks. **BUG-018**.

## 2. Sound effects the PC has and the port does not

- [ ] Disappearing block: appear SFX + the 5-frame appear animation
      (`object.cpp:607`, `object.cpp:1021`).
- [ ] Object break SFX for destructible walls (`object.cpp:1785`).
- [ ] Beam SFX for rays / lava beams (`object.cpp:1102`).
- [ ] Stage-selected SFX in the stage select (`sceneslib.cpp:648`).
- [ ] Teleporter-activated SFX (`game.cpp:2208`).
- [ ] Water enter/leave splash for the player (check: may already exist).

## 3. Small features

- [ ] Weapon tooltip when switching weapons (`draw.cpp:841`).
- [ ] Water bubble animation on BG (`classmap.cpp:1215`).
- [ ] Teleport-small colour flash (`draw.cpp:429`).
- [ ] Dark mode (draw only animations/projectiles) (`game.h:273`).
- [ ] Autoscroll out-of-screen damage (`game.cpp:1654`).
- [ ] Ground-quake `block_jump` after a quake hit (`classplayer.cpp:281`).

## 4. Ending / scene player

- [ ] Scene ops still missing: MOVE_VIEWPOINT, SHOW_ANIMATION, SUBSCENE,
      PLAY_MUSIC / PLAY_SFX / STOP_MUSIC (`scenes/sceneshow.cpp:41`).

## 5. Boss behaviour

- [ ] Boss intro sprites (fall-in + intro frames) `draw.cpp:280`.
- [ ] Boss ground shake on landing `classmap.cpp:2428`.
- [ ] Sub-boss revive after the boss dies `classmap.cpp:2201`.
- [ ] Boss teleport/jump animation rows (BUG-011, unblocks BUG-010).

## 6. Enemy AI (low priority: the shipped data does not use them)

The port handles every action the stages 0..13 AI data references
(walk/fly/jump/wait/shoot/invert/air-walk/fall/teleport/dash/grab-wall/
spawn/replace/change-move/jump-attack-up). Still absent, for future data:
jump family (to player/point/random, ahead, long ahead, once, up, teleport
in/out, to roof), quake attack, accelerate up/down, circle player, wall walk
(+shoot), multi-projectile spreads, morph, explode itself, throw item, play
sfx, wait random, change-animation, fly drill/sin/dash/opposite-wall.

## 7. Runtime verification (one instrumented run)

- [ ] BUG-022 SEAHORSE: wrong/incomplete tileset at stage start.
- [ ] BUG-023 SEAHORSE: fade on every link.
- [ ] BUG-024 SEAHORSE: tileset corrupt after the second link / after a ladder.
- [ ] Re-check the 2026-10-22 fixes in game: BUG-021, 025, 026, 027, 028, 029,
      030, 031.

## Out of scope (decided)

Difficulty select, cheats menu, game picker, CLI flags, static tile3 foreground
art, rain/lightning overlays, save copy, language/platform/performance, the
XP/shop/heart-piece systems.
