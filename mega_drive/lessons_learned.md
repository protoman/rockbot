# Lessons learned — RockBot → Mega Drive port

Real mistakes + fixes. Add to this on every session.

## Build / SDK

**1. `multiple definition of "rom_header"` at link**
- Error: `ld: out/release/src/boot/rom_head.o ... multiple definition of "rom_header"; sega.o:(.text.keepboot+0x100) first defined here`
- Cause: the `sgdk-skeleton` template shipped `src/boot/rom_head.c` (old SGDK format). The new `makefile.gen` already generates `src/rom_header.c` and embeds it via `sega.s`/`sega.o`.
- Fix: **delete** `src/boot/rom_head.c`. Never re-add. `makefile.gen` creates `src/rom_header.c` itself (rule at makefile.gen:210).

**2. `main()` needs the new signature**
- Error: LTO warning `-Wlto-type-mismatch` — `extern int main(bool hardReset)` vs `int main()`.
- Fix: always `int main(bool hardReset)`. On soft reset (`!hardReset`), call `SYS_hardReset()`.

**3. `make sgdk` fails: `SGDK/bin/xgm2tool.jar` does not exist**
- Cause: a pre-existing `sgdk/SGDK` clone on an old revision (missing that jar) plus locally modified files. `sgdk/Makefile` only clones when the dir is absent.
- Fix: `rm -rf sgdk/SGDK sgdk/sjasm` and re-run `make sgdk`. Let the Makefile clone the pinned revision (`SGDK_REV`).

**4. Build dies on timeout**
- Error: the shell kills the process at 120s; `nohup ... &` does **not** survive because the shell is still the process group parent.
- Fix: `setsid nohup make ... & disown` — detaches from the shell's process group.

**5. `bash -lc` does not apply `~/.bashrc`**
- `bash -l` reads `~/.profile`/`.bash_profile`, not `.bashrc`. To test env: `bash -ic '...'`.

## rescomp / assets

**6. `Can't open image 'res/res/upperland.png'`**
- Error: duplicated path. Paths **inside a `.res` file are relative to the `.res` directory**, not the project root.
- Fix: in `res/resources.res`, write `"upperland.png"`, not `"res/upperland.png"`.

**7. `SPR_setPosition(upperland_sprite, ...)` — wrong type**
- Error: `expected 'Sprite *' but argument is of type 'SpriteDefinition'`
- Cause: rescomp generates a `SpriteDefinition` (the asset). To position/animate you need an **instance**.
- Fix: `Sprite *s = SPR_addSprite(&upperland_sprite, x, y, TILE_ATTR(...));` then `SPR_setPosition(s,...)`, `SPR_setFrame(s,...)`. `SPR_addSprite` requires `SPR_init()` first.

**8. `VDP_clearPlane` — 2 arguments**
- Error: `too few arguments; expected 2, have 1`
- Signature: `void VDP_clearPlane(VDPPlane plane, bool wait)`. Use `VDP_clearPlane(BG_A, FALSE)` — FALSE = do not block waiting for DMA.

**9. Sprite palettes**
- Each `SpriteDefinition` carries its own palette. 3 sprites with different colors = **3 palette lines** (PAL0/PAL1/PAL2) and the matching `TILE_ATTR(PALn,...)` on each `SPR_addSprite`. Do not use one palette for all.

**25. `PAL_setColor()` takes a GLOBAL CRAM index, not palette + slot**
- `PAL_setColor(u16 index, u16 value)` / `PAL_getColor(u16 index)` index CRAM
  0..63: the four lines are 16 entries each, so PAL3's entry 12 is **CRAM 60**
  (`(3 << 4) + 12`).
- Writing `PAL_setColor(12, ...)` while meaning "PAL3 entry 12" silently
  rewrites **PAL0's** entry 12, and index 15 is PAL0's font white — the
  diagnostic text drawn on screen went completely invisible (black on black),
  which cost a debugging cycle. A tilemap cell's palette field is the other
  thing: that one *is* the 2-bit line number (`TILE_ATTR_FULL`/`<< 13`).

**26. A failing build is invisible if you only grep the log for "error"**
- This toolchain's messages are localised: gcc reports `Erro 1` (Portuguese), so
  `make ... | grep -iE 'error|Error'` matched nothing while every build was in
  fact failing. Four emulator runs then tested a *stale* ROM (it played the intro
  logo screen), and the conclusion drawn from them — "the palette line is not
  switching" — was wrong: the code had never been compiled.
- Always check make's exit status (`if ! make > log 2>&1; then ... fi`, or look
  for `Erro`) and, before launching an emulator, that `out/rom.bin` is **newer
  than the newest source file**. Two seconds of checking beats four bad runs.

## Runtime / drawing

**10. Fully black screen = 2 combined bugs (NOT the emulator)**
- (a) **`SPR_update()` never called.** The SGDK sprite engine is *deferred*: `SPR_addSprite`/`SPR_setPosition`/`SPR_setFrame` only write to RAM. Nothing appears on screen until `SPR_update()`, which must run **once per frame**, before `SYS_doVBlankProcess()`. Without it, sprites are invisible.
- (b) **`VDP_drawText` uses PAL0 by default.** `text_basetile` starts at 0 → palette 0. `PAL_setPalette(PAL0, upperland...)` overwrote the font palette → text invisible (font pixels mapped to the wrong colors).
- Symptom: 100% black screen (no sprites AND no text).
- Fix: call `SPR_update()` inside the frame wait; move sprites to PAL1/PAL2/PAL3 and keep PAL0 for the font.
- General rule: **black screen ≠ broken emulator.** Before blaming the emulator, check: valid ROM (SEGA header, checksum), `SPR_update()` in the loop, font palette, and that `VDP_setScreenWidth320/Height224` runs before any drawing.

**11. `VDP_drawText` takes TILE coordinates, not pixels**
- Error: text invisible even though `VDP_drawText` was called with no warning.
- Cause: I passed **pixel** values (66, 114, 183, 194, 204, 214) for `x`/`y`. That API uses **tiles** (8×8). Screen 320×224 = 40×28 tiles, so any y > 27 is off-plane and silently ignored (`VDP_drawTextBG` returns early when `y >= planeHeight`).
- Rule: `VDP_drawText(str, tileX, tileY)`. Convert with `tile = pixel / 8`.
- Correct reference: the official `hello-world` sample does `VDP_drawText("Hello world !", 12, 12)` = tile 12,12 (pixel 96,96).
- Note: the RockBot original (`graphLib.draw_text`) uses **pixels**. When porting, divide everything by 8.

## Colorkey / assets

**12. RockBot colorkey = (75,125,125) = 0x4B7D7D**
- Confirmed by the user: **this is the only transparent colorkey**. `defines.h:961` (`COLORKEY_R/G/B`).
- The others (`55,255,0`, `255,0,255`, `0,255,255` — `defines.h:1039-1049`) were for **colorcycle**, which is disabled in the game. **Ignore them.**
- The asset converter must map that RGB to index 0 (transparent), otherwise it becomes a solid color on the sprite (symptom: blue/box background behind the logo).

## Emulator

**13. RetroArch flatpak cannot read `/tmp`**
- Error: `[ERROR] [Content] Could not read content file`
- Fix: `flatpak run --filesystem=/tmp org.libretro.RetroArch ...` — the flatpak sandbox does not expose /tmp by default.

**14. RetroArch core URLs**
- `buildbot.libretro.com/stable/<ver>/linux/x86_64/...` → 404
- `raw.githubusercontent.com/libretro/libretro-database/master/dist/...` → 404
- **Works**: `https://buildbot.libretro.com/nightly/linux/x86_64/latest/<core>_libretro.so.zip`

**15. GitHub clones from this environment**
- `git clone https://` fails with `could not read Username for 'https://github.com'` (askpass). The user has to clone manually.
- `myself600/blastem` **no longer exists**. Correct repo: `libretro/blastem`.
- `kodi-game/game.libretro.blastem` is only the Kodi wrapper, not the emulator.

**16. blastem (standalone) crashed on ROM load**
- User report: worse than RetroArch, crashes at load. Not investigated — low priority while our own ROM bugs are the likely cause.

**17. Automating Kega for headless-ish testing**
- Launch detached: `setsid nohup flatpak run --branch=stable --arch=x86_64 --command=Fusion --file-forwarding com.carpeludum.KegaFusion @@ <rom> @@ > log 2>&1 < /dev/null & disown`
  Plain `nohup ... &` is killed when the tool shell times out; `setsid` detaches the process group.
- Key bindings live in `~/.var/app/com.carpeludum.KegaFusion/config/.Kega Fusion/Fusion.ini`:
  `Player1Keys=143,145,142,144,67,88,90,13,68,83,65,9,...`
  → Up/Down/Left/Right, A=**C** key, B=**X** key, C=**Z** key, **Start=Return**, X/Y/Z = D/S/A, Mode=Tab.
- Sending input: `xdotool key --window $WID Return` **did not** register. What works:
  `xdotool windowactivate --sync $WID` then `xdotool keydown --clearmodifiers Right` / `keyup` on the focused window.
- Screenshot: `DISPLAY=:0 timeout 25 spectacle -b -n -o <file>.png` then wait ~3s for the file to land.
- Find the window: `xdotool search --name "Fusion" | head -1`.

**18. rescomp `MAP` cannot use a separate tileset image**
- Error: `java.lang.RuntimeException: Can't find tile [0,0] in tileset, something wrong happened...`
  at `Map.java:172`, for `MAP name "map.png" tileset_id ...` when `tileset_id` came from a **different** image file.
- Cause: unresolved. The map's tile 0 was identical to tileset slot 0 (both all-index-0), palettes matched, and it failed with both `TILESET ... ALL` and `TILESET ... 0`.
- Fix: derive both resources **from the same image**, exactly as the official samples do
  (`sample/game/platformer`: `TILESET level_tileset "images/level.png"` + `MAP level_map "images/level.png" level_tileset`).
  A 3200x240 map used as its own tileset is fine — `ALL` dedups it to the ~41 tiles actually used.
- Consequence: `tools/convert_stage.py` only writes the map PNG. Do not emit a separate tileset PNG.

**19. `SPR_setAnim` takes 2 arguments in this SGDK revision**
- Error: `too many arguments to function 'SPR_setAnim'; expected 2, have 3`
- Signature is `void SPR_setAnim(Sprite *sprite, s16 anim)` — no `force` flag.
- For a hard reset of the frame counter use `SPR_setAnimAndFrame(sprite, anim, frame)`.

**20. SGDK `IMAGE`/`MAP` palette selection is encoded in the basetile**
- `VDP_drawImageEx(plane, image, basetile, x, y, loadpal, dma)` loads the palette to
  `(basetile >> 9) & 0x30` — i.e. the palette line bits (13-14) of `basetile` become the CRAM offset (0/16/32/48).
- `VDP_drawImage()` hardcodes `PAL0`, which collides with the text engine. Always use
  `VDP_drawImageEx(..., TILE_ATTR_FULL(PALn, FALSE, FALSE, FALSE, TILE_USER_INDEX), ...)` to pick the line.
- `VDP_drawText` uses `text_basetile` (PAL0 by default, set at boot) — keep PAL0 for the font.

## RockBot data / palette-swap

**21. Player sprites use runtime palette placeholders**
- `p1.png` contains pure **green (55,255,0)** and **magenta (255,0,255)** pixels. They are **not** colorkeys.
- `classplayer.cpp:1004` calls `change_surface_color(0/1/2, weapon_colors[n].colorN, ...)` at runtime:
  - `COLORKEY1 (55,255,0)`  ← `weapon_colors[0].color1` = **(143, 0, 119)**
  - `COLORKEY2 (255,0,255)` ← `weapon_colors[0].color2` = **(0, 115, 239)**
  - `COLORKEY3 (0,255,255)` ← `color3`, **not set** for player 0 (and absent from p1.png)
  Defaults are in `file/v4/file_game_v4.h:335` `file_player_v3_1(int n)`.
- `tools/convert_player.py` bakes these swaps in offline. Doing it once beats a runtime recolor pass.

**22. Negative tile indices are animated tiles, not empty**
- `classmap.cpp:296`: `if (pos_origin.x < -1 && pos_origin.y == 0) anim_tile_id = (pos_origin.x * -1) - 2;`
  So `x < -1 && y == 0` → animated tile (data in `anim_tiles.dat`, which is 0 bytes for RockDroid1 → none defined).
  `-1` in either coordinate means empty.
- `tools/convert_stage.py` treats any negative coordinate as empty.

**23. Rescomp `MAP`/`TILESET` must come from the same image file**
- Error: `RuntimeException: Can't find tile [0,0] in tileset` at `Map.java:172`.
- Cause: the map PNG and the tileset PNG were separate files with identical
  content. rescomp could not resolve the map's tile 0 against a tileset built
  from a different file, even though the palettes matched exactly.
- Fix: build the TILESET **from the map image itself** (as the SGDK samples do):
  `TILESET stage0_tiles0 "stage0_map0.png" FAST ALL` + `MAP stage0_map0 "stage0_map0.png" stage0_tiles0 FAST 0`.
  `FAST ALL` dedups it down to the ~40 tiles actually used, so no ROM waste.

**24. Stage maps are split across multiple map records; links cross them**
- `maps_v2.dat` holds 3 maps per stage (`FS_STAGE_MAX_MAPS`), and
  `stages_v301.dat` `links[30]` (`file_link`, 10 B at offset 60) warps between
  them. Fields are `Sint8 from_map, Sint8 to_map, st_position_uint8 origin,
  st_position_uint8 destiny, ...` — **the positions are unsigned bytes**, so
  the PC data stores values like -120 that actually mean 136.
- Stage 0 (CHAOS CITY) real progression, measured from the data:
  `link2: map 0 (136,14) -> map 1 (198,0)` and `link3: map 1 (1,0) -> map 0 (139,14)`.
  The full-height SOLID column at x=137..138 of map 0 is a **map divider**,
  not a wall to climb. The ladder at x=136 leads down to the link tile.
- Map 2 of stage 0 is entirely UNBLOCKED in the source data (placeholder).

**25. SGDK `VDP_drawText` is erased by the MAP engine**
- Symptom: debug text drawn on BG_A disappears every frame even though it is
  written after `MAP_scrollTo`.
- Cause: SGDK's MAP engine refreshes its plane cells during
  `SYS_doVBlankProcess()`, i.e. *after* the CPU write in the same frame.
- Workaround for debugging: draw on the WINDOW plane, or print via
  `KLog`/`KLog_U16` instead of on-screen text.

**26. `pkill -f "<pattern>"` can kill your own shell**
- `pkill -f` matches the **full command line** of every process, including the
  shell that is running the pkill (its `bash -c '...'` string contains the same
  pattern). Symptom: the command never reaches the next line, and the tool times out.
- Fix: use `pkill -x <comm>` (exact process name match), or find the PID with
  `pgrep` first and `kill` that.

**27. Variable jump height makes 1-frame jump taps useless**
- `player.c` halves `vy` when the jump button is released while rising
  (mirrors the original `character.cpp`). A 1-frame tap therefore only reaches
  ~14px instead of ~56px, and any "hold Right + tap jump" auto-test appears to
  be "stuck" against a 2-tile step that is actually trivially jumpable.
- For scripted emulator tests, hold the jump button ~0.3s.

**28. Never trust a "successful" build until you verify the artifact**
- `rtk grep ... | head` returns `head`'s exit status (0), **not** make's. Repeatedly I
  ran `make | grep -E "error|..." ; echo EXIT=$?` and got `EXIT=0` while the link had
  actually failed — so I kept iterating on a stale `out/rom.bin` and "verified" nothing.
- Rule: check the build with `make` alone and read its exit code, or confirm the
  artifact timestamp changed. `rtk stat -c '%y' out/rom.bin` before/after.

**29. Partial `edit` calls silently no-op; always re-read and verify after editing**
- Several `edit` calls returned "Could not find oldString" (aborted) while I assumed
  they had applied. Result: `player.c` kept the buggy version for many iterations, and
  `main.c`/`player.h` drifted apart (`player_get_box` vs `player_get_bounds`).
- Rule: after any edit, **re-read the affected region** (`Read` with offset/limit) or
  `grep` for the new symbol before building. For a whole-function rewrite, prefer
  `write` (full file) over `edit`, then `Read` to confirm.

**30. Full-box vertical collision freezes the player mid-air**
- `box_hits_solid(l, t, r, b)` on the **whole** box during the vertical pass treats a
  side-hit against a step as "landed": `dy > 0` → `on_ground = TRUE` → gravity disabled
  → the player hangs in the air and cannot advance.
- Fix: in the vertical pass only test the **row the feet cross** (falling) or the **row
  the head crosses** (rising). Keep the whole-box test for the horizontal pass only.

**31. `MAP_scrollTo` is incremental — a camera jump leaves stale columns**
- Symptom: after a respawn or a map link, the background "blinks"/shows tiles that had
  already scrolled off screen.
- Cause: `MAP_scrollTo` only loads the newly exposed edge. A jump of hundreds of pixels
  exposes a region the engine never refreshes.
- Fix: `MAP_scrollToEx(map, x, y, TRUE)` (`forceRedraw`) after any camera jump —
  respawn, map link, initial load. Keep the incremental `MAP_scrollTo` for the
  per-frame camera follow.

**32. Do not retry a failed `edit` from memory — re-read the file first**
- Symptom: long stretches of "Let me go. / Let me produce. / I need to break out."
  filler with no tool call, and the same `edit` retried many times, each failing with
  "Could not find oldString". Turns burned, no progress.
- Cause: when `edit` fails, the file does **not** match what I remember. Retrying with
  the same or an "imagined" string is guaranteed to fail again. The narration is
  me re-stating intent instead of gathering the missing fact.
- Rule, in order:
  1. `edit` fails → **`Read` the exact region** (with offset/limit) before any retry.
  2. For a whole function/block rewrite, use `write` (full file) instead of `edit`.
  3. After a successful `edit`, `Read` or `grep` the new symbol to confirm it landed.
  4. Never emit narration-only turns. One failed call = one read = one corrected call.

## Open / pending

- ROM checksum: `sizebnd -checksum` did not rewrite the field (writes `0xd271`, real is `0xa980`). Modern emulators (Kega/GPGX/blastem) do not validate it, so it is cosmetic. Investigate only if an emulator starts rejecting the ROM.
- Milestone 1 (intro logos) is running. Next: title screen + menu.
