# PS3 port — handoff

Last updated: 2026-11-03. Read this before continuing the PS3 port.

## State in one line

Builds, boots, renders Stage 1 at 60 FPS, but the game **freezes after roughly
1–2 minutes**, and the normal intro/menu path is still broken. No `.pkg` has
been produced yet.

## Environment

- ps3dev: `/mnt/ExtraData/Devel/SDK/PS3/ps3dev`
  (nightly 2026-10-12: PPU GCC 13.2.0, PSL1GHT `f649a08`, ps3libraries `b2efde5`)
- Build: `PS3DEV=/mnt/ExtraData/Devel/SDK/PS3/ps3dev ports/ps3/build.sh`
- Output: `build-ps3/PS3_GAME/` (`USRDIR/EBOOT.BIN`, `PARAM.SFO`, data)
- Emulator: `/mnt/ExtraData/programas/rpcs3-v0.0.43-20559-a2c2ca2f_linux64.appimage`
- Firmware installed in `~/.config/rpcs3/dev_flash` (PS3UPDAT.pup was installed)
- RPCS3 config/state: `~/.config/rpcs3/`, logs in `~/.cache/rpcs3/RPCS3.log`
  and guest stdout in `~/.cache/rpcs3/TTY.log`
- Saves: `~/.config/rpcs3/dev_hdd0/tmp/rockbot/`

## Code map

- `ports/ps3/ps3_platform.{h,cpp}` — paths, pad defaults, save dir, the
  recursive-newlib-lock workaround.
- `ports/ps3/build.sh` — CMake cross build, strip, `sprxlinker`, `fself`,
  package, grayscale-PNG conversion.
- `cmake/toolchain-ps3.cmake` — PPU toolchain, `-fno-exceptions -fno-rtti`,
  no `-fstack-protector`.
- `graphicslib.cpp` — PS3 video mode 1280x720, 3x upscale of 320x240.
- `file/file_io.cpp` + `game_mediator.cpp` — big-endian swaps shared with Wii.
- `timerlib.cpp`, `aux_tools/fps_control.cpp`, `sdl_layer.cpp` — PS3 time/sleep
  handling.

Commits:

```
c7e31c7a ps3: keep game time relative, avoid raw SDL delays
86e24eb4 ps3: native PSL1GHT timer, convert grayscale PNGs
6ad23d76 ps3: add PSL1GHT port (latest ps3dev SDK)
```

## What works

- Cross-build and package.
- Boots in RPCS3 to the Upperland notice.
- Stage 1 loads, renders correctly at 720p, 60 FPS.
- Big-endian map/stage/enemy data loads.
- Audio backend opens (`cellAudio`).
- Idle animation, clouds and “MISSION STARTED!” play after the timer fix.

## Open bugs (priority order)

### 1. Stage freezes after 1–2 minutes

Symptom: everything stops, including idle animation and disappearing-block
timer. RPCS3 window still shows ~60 FPS (last frame is just re-presented).

Already tried, did **not** fix:

- native `sysUsleep` in `timerLib::delay` / `SDLL_Delay`
- `fps_control::limit` using `timer.delay` instead of `SDL_Delay`
- relative game time (`now_ms - first_reading`), so 32-bit fields do not overflow
- RPCS3 audio `Renderer: Null`
- PPU/SPU LLVM and interpreter

Suspects not yet ruled out:

- another blocking `timer.delay`/`SDL_Delay`/`sysUsleep` call
- a deadlock in SDL audio/SDL_mixer or the pad thread
- the recursive-lock no-op workaround in `ps3_platform.cpp` interacting with
  libstdc++/SDL mutexes
- FPSCR/PPU emulation difference in LLVM

Debugging helper: `ports/ps3/README.md` explains logs. A temporary demo build
that boots straight into Stage 1 is the fastest repro:

- add `GAME_FLAGS[FLAG_QUICKLOAD] = true;` under `#ifdef PS3` in `main.cpp`
- in `game::quick_load_game`, skip `scenes.select_player()` and comment
  `show_ending()` on PS3
- rebuild, package to `/tmp`, run
- take two screenshots 5 s apart; identical MD5 = frozen

GDB attempt: RPCS3's `ppu-gdb` segfaults on this machine (Python 3.12 build),
and host GDB does not understand the 556-byte `g` packet. RPCS3's GDB server is
enabled by `PPU Debug: true`, port 2345. Decoding the raw RSP `g` packet
yourself is the next viable way to get the stuck PC; PC is the first 8 bytes.

### 2. Normal intro/menu path goes black

With `show_notice()` enabled, the notice displays but never advances (before
the timer fix). After the timer fix the notice can still block; skipping it
(`#ifndef PS3 show_notice();`) leaves a black screen after audio init, before
the title menu. TTY log stops after `audioPortStart: 0`.

Next: add TTY markers around `scenes.preloadScenes()`,
`scenes.game_scenes_show_unbeaten_intro()`, `scenes.main_screen()` and
`fio.have_one_save_file()` to find the blocking call. The demo quickload path
avoiding these works, so the stage/graphics path is fine.

### 3. Controller input not verified

- PS3 defaults are in `ports/ps3/ps3_platform.cpp`:
  Cross jump, Square attack, Circle dash, Triangle shield, L1/R1 weapons,
  Start, Select quit, left analog stick movement.
- RPCS3 `Keyboard` pad handler did **not** receive xdotool-injected keys in
  `--no-gui` or GUI mode.
- A virtual PS3 pad was created with `uinput`
  (`/tmp/opencode/vpad.py`, name `Sony PLAYSTATION(R)3 Controller`), RPCS3 saw
  it with `Handler: Evdev`, but Start did not reach the game.
- Real hardware/controller still untested.
- Input profile used during tests:
  `~/.config/rpcs3/input_configs/global/Default.yml`
  ```yaml
  Player 1 Input:
    Handler: Evdev
    Device: Sony PLAYSTATION(R)3 Controller
  ```
  Remove that file to restore RPCS3 defaults.

### 4. Grayscale PNG (fixed)

`icons_small_disabled.png` is the only grayscale PNG; SDL_image on PS3 fails it
with “Out of memory”. `ports/ps3/build.sh` converts grayscale PNGs in the
packaged data to RGB via ImageMagick. Keep that step.

## .pkg build (not done yet)

Use the PSL1GHT rules, same as `ppu_rules`:

```bash
PS3DEV=/mnt/ExtraData/Devel/SDK/PS3/ps3dev
CONTENTID=UP0001-ROCKBOT01_00-0000000000000000
PKGROOT=build-ps3/pkg
mkdir -p "$PKGROOT/USRDIR"
cp -r build-ps3/PS3_GAME/USRDIR/* "$PKGROOT/USRDIR/"
"$PS3DEV/bin/make_self_npdrm" build-ps3/rockbot_linked \
    "$PKGROOT/USRDIR/EBOOT.BIN" "$CONTENTID"
"$PS3DEV/bin/sfo" -f "$PS3DEV/bin/sfo.xml" \
    --title="RockBot" --appid="ROCKBOT01" "$PKGROOT/PARAM.SFO"
cp "$PS3DEV/bin/ICON0.PNG" "$PKGROOT/"
"$PS3DEV/bin/pkg" --contentid "$CONTENTID" "$PKGROOT/" \
    build-ps3/rockbot.gnpdrm.pkg
"$PS3DEV/bin/package_finalize" build-ps3/rockbot.gnpdrm.pkg
```

Notes:

- `make_self_npdrm` warning: output filename matters, do not rename EBOOT.BIN.
- Install on CFW PS3 through “Install Package Files”.
- Add this to `ports/ps3/build.sh` behind `--pkg` only after the runtime freeze
  is fixed, so a broken package is not distributed.

## Working tree

Do **not** commit or revert the pre-existing modified files
(`game.cpp`, `build/games/...`, `mega_drive/...`, `docs/RoadMap.txt`,
`scripts/ps2run.sh`). They are the owner’s work. The PS3 changes are isolated
in the commits above.
