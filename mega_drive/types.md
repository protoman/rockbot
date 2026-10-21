# types.md — PC → SGDK type equivalence (avoid overflow mistakes)

## The incident (2026-10-14, FIX-07 root cause)

`Player.x/y` were `fix16`. On PC the same fields were ordinary integers.
SGDK `fix16` is **10.6 format, NOT 16.16**: integer part is 10 bits signed
= range **±512 only**. Stage 0 is 3200 px wide, so as soon as the player
walked past x=512 the position wrapped negative (513 → -511, 512 → -512),
collision treated it as off-map (`TERRAIN_SOLID`) and snapped — the
"player blocked at tile ~32" oscillation. Fixed by storing world pixels
in plain `s16` (player.h).

## SGDK numeric formats

| Type      | Layout            | Integer range   | Creation            |
|-----------|-------------------|-----------------|---------------------|
| `fix16`   | s32, 10.6 (6 frac bits) | **±512**   | `FIX16(v)` = v*64   |
| `fix32`   | s32, 22.10 (10 frac bits) | ±2 M     | `FIX32(v)` = v*1024 |
| `fastfix16` | s32, 8.8        | ±128            | `FF16(v)`           |
| `fastfix32` | s32, 16.16      | ±32768          | `FF32(v)`           |

`F16_toInt()` returns `s16`. Never put a world coordinate in `fix16`.

## PC (RockBot / SDL) → SGDK mapping

| PC type                          | SGDK type | Note |
|----------------------------------|-----------|------|
| `Sint8/Uint8`                    | `s8/u8`   |      |
| `Sint16/Uint16`                  | `s16/u16` |      |
| `Sint32/Uint32`                  | `s32/u32` |      |
| `int`, `unsigned` (32-bit on PC)| `s32/u32` | m68k int is also 32-bit |
| `bool`                           | `bool`    | SGDK has its own `bool` |
| `double`/`float` coordinates     | **`s16` pixels** (positions) or `fix32` (sub-pixel) | never `fix16` for coords |
| SDL `SDL_Point`, etc.            | `Vect2D_s16` | |

## Rules of thumb

1. World/screen **positions** → `s16` pixels (max world 3200×224, fits s16).
2. **Speeds/accelerations** may use `fix16` only if |value| < 512
   (ours: max 6.0 — fine).
3. Need sub-pixel coordinates → `fix32` (22.10), not `fix16`.
4. Before choosing a fixed-point type: check the INTEGER range first
   (`FIX16_INT_BITS`/`FIX32_INT_BITS` in `inc/maths.h`), not the storage size.
5. Tile index = pixel/16; `u8 tile * 16 ≤ 4080` still fits `s16`.

## Port variable audit (current state, all OK)

| Variable                        | Type        | Max used | Margin |
|---------------------------------|-------------|----------|--------|
| `player.x/y`                    | `s16`       | 3200     | ok     |
| `player.vx/vy`                  | `fix16`     | ±6       | ok     |
| `camera` (`Vect2D_s16`)         | `s16`       | ~2880    | ok     |
| `MOVE_SPEED`/`ACCEL`/`GRAVITY`… | `fix16`     | 6        | ok     |
| link `to_x/to_y` (`u8 tile*16`) | `s16`       | 4080     | ok     |
| timers, pad, map index          | `s16`/`u16`/`u8` | tiny | ok     |
