# RockBot — PlayStation 3 port

PSL1GHT / SDL 1.2 port of RockBot. All PS3-specific runtime code lives in
`ports/ps3/`; the shared engine only gained `#ifdef PS3` branches.

## Requirements

* A `ps3dev` installation (PPU GCC + PSL1GHT + portlibs), e.g. the latest
  all-in-one nightly from <https://github.com/ps3dev/ps3dev/releases>.
* RPCS3 (tested with 0.0.43) or a CFW PS3.

The prebuilt ps3dev tarball is enough; nothing has to be compiled by hand.
Tested with `ps3dev` nightly **2026-10-12** (PPU GCC 13.2.0, PSL1GHT
`f649a08`, ps3libraries `b2efde5`).

## Build

```bash
export PS3DEV=/path/to/ps3dev     # default: /mnt/ExtraData/Devel/SDK/PS3/ps3dev
ports/ps3/build.sh
```

Output:

```
build-ps3/PS3_GAME/
├── PARAM.SFO
├── ICON0.PNG
└── USRDIR/
    ├── EBOOT.BIN
    ├── fonts/
    ├── games/RockDroid1/
    └── shared/
```

## Run

RPCS3, directly:

```bash
rpcs3 build-ps3/PS3_GAME/USRDIR/EBOOT.BIN
```

Real console (CFW): copy the `PS3_GAME` contents to
`/dev_hdd0/game/ROCKBOT01/` (`PARAM.SFO` at the root, `USRDIR/` below it) and
launch it from the XMB.

## Controls

The PS3 pad is mapped through SDL_PSL1GHT's joystick numbers:

| Action        | Button   |
|---------------|----------|
| Jump          | Cross    |
| Attack        | Square   |
| Dash / slide  | Circle   |
| Shield        | Triangle |
| Previous / next weapon | L1 / R1 |
| Start / pause | Start    |
| Quit          | Select   |

Movement uses the **left analog stick** (axes 0/1), like the other console
ports. The D-pad is not used by the engine.

## Saves

`/dev_hdd0/tmp/rockbot` (created on first boot). The folder is writable without
an installed package and survives RPCS3 restarts.

## Port notes

* **Big endian.** PS3 is PowerPC64 big-endian. The `.dat` files are dumped
  little-endian by the editor, so the loaders reuse the Wii byte-swap path
  (`wii_convert_*`, widened to `defined(WII) || defined(PS3)`). Map v2, NPC
  and object lists, castle points and enemy extra data were added to that path.
* **No C++ exceptions/RTTI.** The static libstdc++ exception runtime in the
  current ps3dev release hangs during `__libc_init` on a large `.eh_frame`.
  Rockbot does not throw (all `throw` statements are commented out), so the PS3
  build uses `-fno-exceptions -fno-rtti`.
* **Recursive newlib locks.** The SDK's recursive lwmutex used by `atexit`
  fails during static construction and makes `__cxa_atexit` abort. Rockbot is
  single-threaded, so `ports/ps3/ps3_platform.cpp` replaces the recursive lock
  calls with no-ops.
* **Video.** The engine renders at 320x240 and `graphicslib.cpp` upscales it
  3x (960x720) into the centre of a 1280x720 framebuffer.
* **Packaging.** The ELF is stripped, run through `sprxlinker`, then turned
  into a fake-signed `EBOOT.BIN` with `fself`.
