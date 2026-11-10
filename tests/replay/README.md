# Replay tests

Deterministic runs of the game, used to check that a refactoring did not change behaviour.

* The game has a virtual clock and a scripted input (`aux_tools/replay.h`, PC builds only, off unless
  `ROCKBOT_REPLAY` is set). Nothing sleeps, so a run of minutes takes seconds.
* Each game frame writes one line to the trace: stage, map, scroll, the player (position, animation type/frame,
  direction, HP, jump/fall state, platform, shots) and every enemy on screen.
* The same script and the same build always give the same trace.

## Check a change

```
git worktree add /tmp/rockbot_old <commit before the change>      # or any other copy of the old code
# build both with the same options (cmake -S . -B <dir> -DUSE_SDL_VERSION=2 && cmake --build <dir>)
tests/replay/compare.sh /tmp/old_build/rockbot_linux /tmp/new_build/rockbot_linux
```

`same` = identical traces. `DIFFERENT` shows the first lines that differ (frame time, then what changed).

## One run

```
tests/replay/run.sh <binary> tests/replay/intro_rockbot1.replay /tmp/trace.txt [RockDroid1|RockDroid2] [stage]
```

The run uses an empty temporary HOME and the `games/` folder of this repository; your saves and config are not touched.

## Scripts

`<ms> +BUTTON|-BUTTON ... | END`, times in virtual milliseconds (see the header of `aux_tools/replay.h`). The
included script presses START for the player selection, then walks right pressing jump, shoot and dash at
pseudo-random moments, which also reaches stage select, enemies and deaths.

## Projectile paths

`compare.sh` also has two "proj" scenarios (`projectiles.replay`): with `ROCKBOT_REPLAY_PROJECTILES=1` the first frame moves
every projectile type of the game for up to 200 steps (both directions, with an owner and a target) and writes
`PRJ <id>/<direction> step=... pos=... moved=... fin=... refl=... dir=...` lines. Use it before touching
`projectile::move`/`draw`, whose trajectories the normal scenarios hardly reach.
