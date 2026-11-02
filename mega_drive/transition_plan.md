# Vertical room transitions - redesign plan (2026-11-01)

Owner report: the vertical transitions flicker, are sometimes misaligned and stutter. Worst case: the
first link in APE (stage4 link0: map 0 (60,0) -> map 2 (189,13), a ladder going DOWN; verify that APE = stage 4).
Goal: a slide that is as smooth as the PC's, ends exactly where the PC ends, and can be proven right offline.

## 1. What the PC does (game.cpp:1032 `transition_screen`, defines.h `TRANSITION_STEP 4`)

1. Build ONE tall surface (320 x 480): the current screen and the destination screen stacked
   (top-to-bottom: old above, new below; bottom-to-top the other way). The destination is drawn with
   `set_scrolling(adjust_x)` where `adjust_x = new_map_pos_x` (computed in the caller, see 3).
2. Loop `i = 0 .. RES_H/4 - 1` (60 steps of 4 px; 56 on the 224-line MD):
   - copy the 240-line window starting at `i*4` (T2B) or `RES_H - i*4` (B2T): **pure vertical offset,
     identical horizontal position for both halves** (this is the key PC property);
   - move the player by `-4 + extra` per step (`extra = 1` every 5th step, so the player drifts 4/5 px
     less than the scroll: he ends a little lower/higher than the pure shift) while his y > 6 (T2B) /
     < RES_H - 2 tiles (B2T);
   - draw both maps' objects/NPCs with the same offsets (`show_objects(-i*4)` and
     `temp_map->show_objects(RES_H + 4 - i*4 - 8, adjust_x)`), then foreground layers, HUD;
   - `delay(6)` ms per step (about 1 s total).
3. After the loop one more nudge of the player (`STEP - 2` up / `STEP` down), then the caller
   `set_current_map`, `set_scrolling(new_map_pos_x)`, and
   `player.x = |player.real_x + new_map_pos_x|` (**keeps the screen X**), y untouched.
   `new_map_pos_x = old scroll - teleporter_dist - (diff_h + 2)` clamped to [0, map_w].
4. Teleporter links do NOT slide: fade out, blank, place the player at `pos_destiny * TILESIZE`,
   centre the camera (left/right wall locks), fade in.
5. Parallax background: kept (`set_bg_scroll`) if the two maps share the same bg file, else reset to 0.

## 2. What the port does today and why it fails (src/transition.c 704 lines, src/main.c ~770-940)

| # | Defect | Symptom |
|---|---|---|
| 1 | step 8 px held 2 frames (`TRANS_STEP 8`, `TRANS_HOLD 2`) | 30 Hz, 8 px jumps = the stutter |
| 2 | full 42x28 BG_A tilemap + two 224-entry hscroll tables + BG_B rows re-sent by DMA queue **every step**, in the same vblank as `SPR_update` | queue overflow / late data = flicker |
| 3 | `VSCROLL` is never used (stays 0); the picture moves by rewriting the map | cause of 1 and 2 |
| 4 | the two halves have different horizontal scroll (`old_hs`/`new_hs`, per-scanline table) because the destination camera is chosen to keep the ladder column, and then `old_shift` moves the OUTGOING screen by the residual | the PC has no such residual: both halves share X; the port's extra offsets are where misalignment comes from |
| 5 | position/camera computed in three places: `main.c` (arrival px/py/stair scan/`keep_sx`), `transition_scroll` (`clamp_cam_x`, `old_shift`), `stage_load` + `stage_arrival_camera_at` | last slide frame != first game frame -> "one tile left" (BUG-050), ladder off by a column |
| 6 | up to 14 frozen preview sprites + outgoing enemies + player in the same scanlines | > 20 sprites/line: blinking |
| 7 | after the slide `stage_load` rebuilds everything (display off) and `player_place` + ladder nudge run afterwards | visible hitch / wrong first frame |
| 8 | incoming palette is the OUTGOING one, incoming bg falls back to no parallax when the tiles do not fit | colours wrong during the slide (accepted deviation, keep) |

## 3. Design

### 3.1 One arrival description, computed once (fixes 4, 5, 7)

New pure function (no VDP, no globals written) in `src/arrival.c/.h`:

    typedef struct { u8 to_map; s16 cam_x; s16 px, py; bool on_stair, hang; u8 dir; } Arrival;
    void arrival_compute(const Link *l, s16 old_cam_x, s16 old_px, s16 old_py, Arrival *out);

It contains what is now spread over `main.c` (stair scan, `stage_arrival_camera_keep`, band nudge).
Follow the PC rule: **screen X is kept**, `cam_x = px_dest - screen_x`, clamped to the destination map; if
clamping leaves a residual, the PC would move the player on screen, not shift one half of the slide:
so the residual is applied to the PLAYER during the slide (he ends where `px - cam_x` says), never to
the outgoing picture. Both halves then share ONE horizontal scroll, like the PC.
`main.c` calls `arrival_compute` once; `transition_scroll(&arr)` and `stage_load`/`player_place` use the
same struct. This function is host-testable (it already is, in spirit: `tools/audit_links.py`,
`tools/check_arrival_locks.py`).

### 3.2 Slide by vertical scroll, streaming rows (fixes 1, 2, 3)

Keep the existing VRAM layout (BG_A/BG_B 64x32 cells, no new VRAM). Treat each plane as a ring of 32 rows
(256 px) and scroll it with `VSCROLL_PLANE` (pixel exact):

- setup (display on, before frame 0): write the first 28 rows (old screen) into the ring, set
  `VDP_setVerticalScroll(plane, v0)`;
- every frame: advance `v` by `STEP` px (4 px/frame = 56 frames = 0.93 s, the PC's duration; 8 px/frame
  would be 0.47 s), and whenever `v` crosses a multiple of 8 px write ONE new row (64 words, ~130 B)
  of the stacked source (old rows then new rows) into ring slot `row mod 32`;
- one horizontal scroll value for the whole plane (both halves share X, 3.1): no per-line hscroll table,
  no 224-word DMA per frame. If the destination background needs a different parallax X, keep the
  small per-line table for BG_B only and update it only when its value changes;
- BG_B (parallax): same ring, same stepping; when both maps share the background file the plane is
  simply left alone (the PC keeps `bg_scroll`).

DMA per frame goes from about 2.4 KB to about 0.2 KB. The one thing to verify first is that the plane
height wraps at 256 px while the visible window is 224 px: ring slot `(v/8 + 28) mod 32` is written
before it is visible (a 4-row margin) - this is the invariant the harness checks.

### 3.3 Sprites (fixes 6)

- The outgoing screen's enemies/objects move with `old_dy` as now (cheap, `enemies_shift_y`).
- Incoming previews: budget by scanline, not by count: add at most as many as keep any row <= 20 sprites
  in total, nearest the player first; the rest simply appear after the slide (the PC also shows only
  what is on the temp map, but it is not limited by hardware).
- Player: moves like the PC (`-4 + extra` per step, 3.1), always last in the sprite list so he is never
  the one dropped.

### 3.4 Hand-off without a redraw (fixes 7)

The last slide frame already shows the destination at `Arrival.cam_x`. `stage_load` then only builds the
real `Map` and objects with the display ON and the plane left in place (the `stage_load_in_place` path that
was disabled because of tileset/palette ordering - to be re-enabled only after the harness proves the ring
content equals what `MAP_scrollToEx` would write; otherwise keep the display-off reload but make it start
from the same camera so the picture does not change, and measure the black frames).

## 4. Offline harness first (project rule: reproduce before fixing)

`tests/host/trans_sim.c` (+ `tools/check_transitions.py`): a model of the two planes (rings, vscroll, hscroll)
driven by the REAL `transition` stepping code compiled against stubs for the VDP calls (like
`tests/host/objects_cost*.c`). For every vertical link of every converted stage (data from `src/stageN_data.c`)
and both directions it checks, per frame:

1. each of the 28 visible rows holds the right map row of the right map at the right x (compare with a
   direct render of the stacked source);
2. a ring slot is never read before written, never overwritten while visible;
3. DMA bytes per frame <= budget (say 1 KB) and queue entries <= 8;
4. sprites per scanline <= 20 (from the sprite positions the code sets);
5. LAST frame == the arrival state (`cam_x`, player screen x/y within 1 px of what the first normal frame uses);
6. player screen x constant across the slide when the PC keeps it (no residual), y follows `-4+extra`.

Today's code can be run through the same harness first to get the failing baseline (items 1, 3, 5) for the
APE link; that is the "fails on the current tree" reproduction the bug workflow demands.

## 5. Phases (each one committed and measured)

1. **Harness + baseline** on the current code; APE stage4 link0..3 listed with what fails. (no behaviour change)
2. **Arrival struct** (3.1): move the maths out of `main.c`, the old slide uses it; harness item 5/6 go green. Ladder
   arrival tests (`audit_links.py`, `check_arrival_locks.py`) keep passing.
3. **Streaming vscroll slide** (3.2) behind a flag, harness items 1-4 green, then one emulator run per direction
   (script-driven with `MD_AUTOPILOT` + the link as `MD_TEST_*`, like the castle run) to confirm no flicker with a
   frame-by-frame capture compare.
4. **Sprite budget** (3.3) and **hand-off** (3.4), measure black frames at the end.
5. Remove the old per-step rewrite code, `TRANS_HOLD`, `old_shift`; update `bugs.md` (BUG-016/023/024/050 relatives).

## 6. Risks / open points

- Vertical ring needs the stacked source for BOTH maps at once: the current code already keeps them in
  `trans_buf` (2 x 28 x 42 words) - the ring reads from it, so RAM stays the same.
- Stage 2 map 2 -> map 1 does not fit in free VRAM (existing fallback to the fade stays).
- Incoming palette is still the outgoing one (accepted); a per-map palette would need a CRAM fade at the start, out of scope.
- Teleporter links keep the fade (as the PC).
- Time estimate: harness 1 session, arrival struct 1, streaming slide 1-2, polish 1.

## 7. What I need from the owner

- Confirm APE = stage 4 and that link0 (ladder down at column 60) is the case that looks worst.
- Which of the three symptoms is worst there (flicker / alignment / stutter), so the harness checks it first.
- OK to start with phase 1 (harness + baseline, no behaviour change)?
