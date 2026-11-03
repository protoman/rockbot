# RockBot — Windows (cross-built from Linux)

RockBot for Windows is built on Linux with [llvm-mingw](https://github.com/mstorsjo/llvm-mingw)
(clang + LLD + MinGW-w64, UCRT). No root, no `apt install`, no Windows machine needed.

```bash
./build-windows.sh                  # SDL2, 64-bit  -> build/rockbot.exe + DLLs
./build-windows.sh sdl2 i686        # 32-bit
./build-windows.sh sdl3             # SDL3
build/packages/win32.sh [1|2]       # build + build/packages/Rockbot<N>_Win32_<version>.zip
```

## SDK layout (same place as the other platform SDKs)

```
/mnt/ExtraData/Devel/SDK/Windows            (override with WINDOWS_SDK=...)
├── llvm-mingw-<date>-ucrt-<host>/          cross toolchain, downloaded on first use
└── deps/mingw-<arch>-sdl<N>/prefix         SDL, SDL_image, SDL_mixer, SDL_ttf (official
                                            *-devel-*-mingw releases) and SDL2_gfx (built)
```

Use another toolchain with `MINGW_ROOT=/path/to/toolchain` (needs `bin/<triplet>-g++`).
Host tools needed: cmake, make, pkg-config, curl, tar, rsync, zip.

## Notes

- The executable links the C++ runtime statically and imports only the Windows UCRT
  (Windows 10, or Windows 7/8.1 with the UCRT update) plus `SDL2.dll`, `SDL2_image.dll`,
  `SDL2_mixer.dll` and `SDL2_ttf.dll` (shipped in the zip). SDL2_gfx is linked in.
  `libxmp.dll` is shipped too: SDL_mixer loads it at run time for the `.mod`/`.s3m` music
  (it comes from the official SDL_mixer runtime zip, not from the -devel package).
- Smoke-tested with Wine (`SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy wine rockbot.exe` from the
  unzipped folder): the game starts, loads its data and the music, and keeps running.
- SDL1 is not supported on Windows; the Qt editor is not cross-built.
- SDL3: `sdl3-gfx` has no Windows release, so zoom/rotate effects are unavailable there.
- `build/packages/version_name.txt` is the version in the zip name; `file/version.h` is kept in
  sync by the build scripts.
