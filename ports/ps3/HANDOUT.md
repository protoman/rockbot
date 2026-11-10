# PS3 port — handoff

Last updated: 2026-11-10. Read this before continuing the PS3 port.

## State in one line

Builds, boots in RPCS3, plays the intro, reaches the title screen and a stage,
and is controllable with a keyboard/pad. The long-standing freeze is fixed.
Not yet verified on real hardware. No `.pkg` has been produced yet.

## Environment

- ps3dev: `/mnt/ExtraData/Devel/SDK/PS3/ps3dev`
  (nightly 2026-10-12: PPU GCC 13.2.0, PSL1GHT `f649a08`, ps3libraries `b2efde5`)
- Build: `PS3DEV=/mnt/ExtraData/Devel/SDK/PS3/ps3dev ports/ps3/build.sh`
- Output: `res/ps3/PS3_GAME/` (`USRDIR/EBOOT.BIN`, `PARAM.SFO`, data)
- Input test app: `ports/ps3/padtest/build.sh` → `res/ps3/padtest/PS3_GAME`
- Emulator: `/mnt/ExtraData/programas/rpcs3-v0.0.43-20559-a2c2ca2f_linux64.appimage`
  (`--no-gui <EBOOT.BIN>`). The first start after every rebuild recompiles the PPU
  code with LLVM and takes ~2–3 minutes; later starts are quick. Never start a second
  instance (it refuses/hangs) — check `ps -eo comm | grep -i '^rpcs3'` first.
- RPCS3 logs: `~/.cache/rpcs3/RPCS3.log`; guest stdout: `~/.cache/rpcs3/TTY.log`
  (`ps3_platform::log()` writes straight to the TTY, no stdio involved).
- Saves: `~/.config/rpcs3/dev_hdd0/tmp/rockbot/`
- RPCS3 keyboard profile: `~/.config/rpcs3/input_configs/global/Default.yml`
  (Handler: Keyboard). Without a profile RPCS3 has **no** key bindings at all
  ("Input configuration empty"). Default keys: arrows = D-pad, X = Cross (jump),
  Z = Square (attack), C = Circle (dash), V = Triangle (shield), Q/E = L1/R1,
  Enter = Start, Space = Select (quit), WASD = left stick.

## Code map

- `ports/ps3/ps3_platform.{h,cpp}` — paths, pad defaults, save dir, libc recursive
  locks, RSX command-buffer guard, libio pad reader, TTY `log()`.
- `ports/ps3/build.sh [1|2] [--no-package] [--pkg]` — CMake cross build, strip, `sprxlinker`,
  `fself`, package folder, grayscale-PNG conversion, optional `.pkg` installer.
- `build/packages/PS3.sh` — asks which game, builds, writes the `.pkg` and a zip.
- `ports/ps3/padtest/` — tiny SDL app that shows 16 boxes lit by the raw pad data and
  logs every pad report to the TTY. Use it to debug input problems.
- `cmake/toolchain-ps3.cmake` — PPU toolchain, `-fno-exceptions -fno-rtti`,
  no `-fstack-protector`.
- `file/fio_scenes.cpp` — scene `.dat` files are little-endian → byte-swapped here.
- `file/file_io.cpp` + `game_mediator.cpp` — other big-endian swaps shared with Wii.
- `inputlib.cpp` (`#ifdef PS3`) — polls the pad through libio.
- `sdl_layer.cpp` (`SDLL_Flip`), `graphicslib.cpp` — PS3 video: 1280x720, 3x upscale.

## What was wrong, and the fixes (all verified in RPCS3 unless noted)

1. **Freeze after a short time (root cause).** SDL_PSL1GHT initialises RSX with a
   **64 KB command buffer** and every `SDL_Flip` adds ~236 bytes through the system
   gcm calls (`gcmSetFlip`, `gcmSetWaitFlip`) which do not wrap the buffer. After
   exactly **256 flips** the buffer is full, the flip never completes and the game
   hangs (the window keeps re-presenting the last frame). RPCS3 splits the buffer in
   32 KB fragments and its context callback (`rsxContextCallback`) hops between
   them. Fix: `ps3_platform::rsx_command_buffer_guard()` is called before every flip
   and, when less than 8 KB is left, calls that callback itself and flushes. Do not
   use `rsxSetNopCommand` for this: it writes through a stale pointer after the wrap
   and overruns the buffer.
2. **Real recursive libc locks.** The old workaround turned every recursive lock into
   a no-op ("Rockbot is single-threaded"). It is not — SDL starts audio/timer
   threads — so `malloc` and `stdio` were unprotected. Replaced with lwmutex-based
   locks created lazily behind a spinlock (the libc ones depend on a meta lock that
   is created after the game's static constructors run, hence the original abort).
3. **Intro stuck on the first screen.** Scene files are little-endian and were read
   raw: `delay_after = 100` became 0x64000000 (~19 days). Now converted in
   `fio_scenes.cpp` (big-endian builds only).
4. **No input.** (a) RPCS3 needs a keyboard profile (see above). (b) SDL_PSL1GHT
   counts pads only during `SDL_Init`; RPCS3 connects the pad slightly later, so SDL
   saw 0 joysticks and delivered no events. The game now polls libio directly
   (`ps3_platform::pad_read`). `ioPadGetData` only fills data when the input changed
   (`len == 0` = no change), so the last state is kept. The polled state is turned
   into press/release *transitions* because the rest of the game expects events
   (otherwise a held key moves menu cursors every frame). Directions are bound to the
   D-pad (buttons 0–3) and the left stick adds movement.
5. **Crash on `graphicsLib::change_surface_color`.** RLE-accelerated surfaces have no
   pixel data until locked; `change_colorkey_color` wrote through a NULL pointer.
   It now locks the surface, and `put_pixel`/`get_pixel` refuse NULL pixels.
6. **Smeared/repeated tiles and backgrounds while scrolling (root cause of #5 too).**
   This SDL 1.3 snapshot's `SDL_DisplayFormat()` is `SDL_ConvertSurface(.., SDL_RLEACCEL)`,
   so every loaded image (tileset, backgrounds, the stage's cached map surface) became
   RLE-encoded. The old RLE blitter draws repeated, shifted copies when the source
   rectangle is offset on this big-endian build, and RLE surfaces have no pixel data
   until locked (the NULL write in `change_colorkey_color`, blank `SDL_SaveBMP` dumps).
   Fix in `sdl_layer.cpp` (PS3 only): `SDLL_DisplayFormat` converts to the video format
   without RLE, and `SDLL_SetColorKey`/`SDLL_SetAlpha` drop `SDL_RLEACCEL`.
   Verified in RPCS3: scrolling is correct.

## Real-console findings (2026-11-10, first tests on hardware)

What was wrong on a real PS3 (all fine in RPCS3), and what was done:

1. **Stage drew as the colour-key colour** (no tiles, no background): `read_stage_maps_v2` (and four more readers) opened
   their files with `in|binary|app`. Fixed: plain read mode. A failed read left the map at its defaults (colour -1,-1,-1).
2. **Byte-swap bugs** in `GameMediator`/`file_io` helpers: `short_to_little_endian` and `int_to_little_endian` shifted signed
   values (wrong whenever the low byte is >= 128: animated-tile frame delays, HP/ids >= 128); `walk_range` was swapped twice;
   `vulnerable_area` and the castle dialog string ids were never converted. All fixed (the 16/32-bit helpers use
   `__builtin_bswap`). Scenes, maps, game, projectile, player and AI data were audited and are complete. Saves/config are
   written and read natively (not portable to a PC).
3. **Save could not be overwritten** (errno 13 on `fopen("wb")` and `"r+b"`, `rename` over it gives EEXIST) for the
   `Rockbot1_00.sav` the first (frozen) session had created: mode 0644, uid 0, gid 65535, right size. Creating new files
   works. The cause of that one file is not proven (a file left in a bad state by the first session is the best guess).
   `file_io.cpp` now writes saves/config with fallbacks (wb, r+b, .tmp+rename, `<name>.new`; readers use the newest) and
   verifies the size on disk (PSL1GHT `librt/write.c` returns the byte count even when the lv2 write failed).
4. **Freeze after the boss** = the save-error screen waiting for a button without flipping the screen; the system callback
   (PS button) only ran in the flip. `ps3_platform::update()` now runs on every flip and every pad poll (throttled).
5. **Debugging on the console**: `PS3_TRACE("...")` writes progress lines to `/dev_hdd0/tmp/rockbot/rockbot_ps3.log`
   (buffered, fsync'ed on flush; also slow frames over 45 ms, sfx plays, RSX wraps, pad changes). Read it over FTP while
   the game runs. The game DATA is read from the installed `/dev_hdd0/game/ROCKBOT01/USRDIR/games/Rockbot1/`, not from the
   `ps3load` upload (which only sends the EBOOT); `ports/ps3/build.sh --no-package` does not refresh the EBOOT.
6. Stalls seen on hardware: ~20 frames at 56 ms after a `.mod` music load, 540-640 ms at the boss start (boss music/scene
   loading). Jumping itself showed no slow frames (threshold 45 ms).
7. Open: one unexplained power-off of the console while loading a game (no log survived; the log is now fsync'ed).

## Open items

- Everything above was confirmed in RPCS3 (menus, stage scrolling, attack, no freeze).
  A stage should still be played for several minutes on real hardware.
- **Endianness audit** — raw `fread` loads that still need byte swapping or a check:
  `file_npc_v3_1_2::vulnerable_area` (not converted), the projectile `trajectory`
  conversion is applied to a copy and discarded (`game_mediator.cpp`),
  `file_stage`/`file_stages`/`file_map`/`file_castle` are read raw, and the
  remaining `file_game` members (weapons, trophies) are not converted.
  Symptoms to expect: wrong timings/positions only on PS3/Wii.
- In-game controller remapping uses SDL joystick events, which do not exist on PS3
  (no SDL joysticks) — the "press a button" remap screens will not work.
- Real hardware: untested (a wireless pad connects late — that case is the reason
  for the libio polling).
- Installer: done. `build/packages/PS3.sh [1|2]` (or `ports/ps3/build.sh [1|2] --pkg`)
  creates `Rockbot<N>_PS3_<version>.pkg` (NPDRM EBOOT, content id
  `UP0001-ROCKBOT0<N>_00-0000000000000000`, title id `ROCKBOT0<N>`, APP_VER = MM.NN from
  `build/packages/version_name.txt`) and a zip of the bootable folder for RPCS3. Install the
  `.pkg` on a CFW/HEN PS3 with "Install Package Files". Not yet installed on real hardware.
  Gotchas with the ps3dev `pkg` tool: it segfaults on trees deeper than 6 folders (it puts
  ~0.5 MB of arrays on the stack per level) — the script raises the stack limit; and it
  silently drops everything past 1024 entries — the script refuses to build above 1000.
  `make_self_npdrm` output name matters, do not rename EBOOT.BIN.

## Debugging recipes that worked

- Log from anywhere with `ps3_platform::log("...")` and read `TTY.log`; bracket suspect
  calls with logs *before and after* — the last line tells you which call never returns.
- To see what the game really draws, save `game_screen` with `SDL_SaveBMP` to
  `/dev_hdd0/tmp/rockbot/` (visible under `~/.config/rpcs3/dev_hdd0/tmp/rockbot/`) and
  convert it with ImageMagick. Do not `SDL_SaveBMP` a surface that has a colorkey or is
  RLE-encoded: it comes out blank.
- A game that "freezes" while RPCS3 still shows ~60 FPS is stuck in a blocking call;
  RPCS3's syscall-usage table at the end of `RPCS3.log` shows what it was spinning on.
- Emulator quirk: the first run after each rebuild is slow (LLVM compile); wait for
  the log to start before judging a hang.

## Working tree

Do **not** commit or revert the owner's modified files (`game.cpp`,
`build/games/...`, `mega_drive/...`, `docs/RoadMap.txt`, `scripts/ps2run.sh`).
