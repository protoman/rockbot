# Releasing RockBot

Everything is driven from `build/packages/`:

```bash
build/packages/release_all.sh                      # asks: version bump (mini/minor/major) and game(s)
build/packages/release_all.sh --bump minor --game both
build/packages/release_all.sh --bump none --game 1 --only psp,ps3
build/packages/release_all.sh --list               # target names; --skip a,b / --dry-run also exist
```

The version lives in `build/packages/version_name.txt` (`M.NN.PPP`; `release.sh` bumps it and syncs
`file/version.h`). `release_all.sh` bumps it first, then runs every platform script for each chosen
game. A failing target does not stop the others: a summary is printed and a log per build is kept in
`build/packages/release_logs/<version>/`.

## Platform scripts

All take the game number (`1` or `2`) as the first argument (they ask when it is omitted) and write
`build/packages/Rockbot<N>_<Platform>_<version>.<ext>`:

| Target | Script | Output | Toolchain |
|---|---|---|---|
| PS2 | `PS2.sh` | `.zip` | ps2dev (`/mnt/ExtraData/Devel/SDK/PS2.new`) |
| PSP | `PSP.sh` | `.zip` | pspdev (`.../SDK/PSP`) |
| PS Vita | `Vita.sh` | `.vpk` | VitaSDK (`.../SDK/Vita/vitasdk`) |
| Switch | `Switch.sh` | `.zip` (folder for `/switch/rockbot/`) | devkitA64 (`.../SDK/Switch/devkitpro`) |
| PS3 | `PS3.sh` | `.pkg` + `.zip` | ps3dev (`.../SDK/PS3`) |
| Dingux | `Dingux.sh` | `.zip` | OpenDingux gcw0 (`.../SDK/opendingux-gcw0-*`) |
| PocketGo | `PocketGo.sh` | `.zip` | Buildroot (`.../SDK/PocketGo`) |
| OpenPandora | `Pandora.sh` | `.pnd` | GCC 16 built by `ports/pandora/build-toolchain.sh` + OpenPandora SDK sysroot (`.../SDK/Pandora`, `.../SDK/OpenPandora`) |
| Wii | `Wii.sh` | `.zip` (Homebrew Channel) | current devkitPro (`.../SDK/Wii/devkitpro-current`) |
| GameCube | `GameCube.sh` | `.zip` (SD card root) | same devkitPro as the Wii (`.../SDK/Wii`); Wii port with `-DGAMECUBE` |
| Linux x64 | `linux.sh` / `deb.sh` / `appimage.sh` | `.tar.gz` / `.deb` / `.AppImage` | host g++ + SDL2 dev packages |
| Windows | `win32.sh [1\|2] [sdl2\|sdl3] [x86_64\|i686]` | `_Win64_.zip` / `_Win32_.zip` | llvm-mingw (`.../SDK/Windows`) |
| Android | `Android.sh [1\|2] [--no-sign]` | `.apk` | commandergenius + NDK r29 + Gradle (`.../SDK/Android`) |
| Raspberry Pi | `Raspberry.sh [1\|2] [armhf\|arm64]` | `.tar.gz` | Docker (`ports/raspberry`) |

Notes:
- **AppImage:** `appimagetool` is downloaded into `/mnt/ExtraData/Devel/SDK/Linux` the first time.
- **Android:** native build with your SDK (`ANDROID_BASE` overrides the folder). The APK is signed with
  `~/.android/my-release-key.keystore` (`ANDROID_KEYSTORE` overrides) when `ANDROID_KEYSTORE_PASS` is
  set (a terminal run prompts for the password instead); otherwise, or with `--no-sign`, it is left
  unsigned. `ANDROID_BUILD=docker` uses the Docker build instead.
- **Wii:** builds `boot.dol` (the SDL 1.2 Wii ports) natively. If `WII_SDK` is missing it is installed once
  by copying devkitPro out of the `devkitpro/devkitppc` Docker image (the only use of Docker). Not tested
  on hardware or an emulator yet.
- **Raspberry Pi:** needs Raspberry Pi OS bookworm or newer and the `libsdl2*` packages.
- The Qt editor is not part of any release package.

## Adding a port

1. `ports/<name>/build.sh [1|2]` builds and stages the package; `build/packages/<Name>.sh` calls it and writes `Rockbot<N>_<Name>_<version>.<ext>` (copy `Vita.sh`).
2. Add `"<name>|./<Name>.sh"` to `TARGETS` in `build/packages/release_all.sh`.
3. For `--docker`: add a stage `<name>` to `ports/docker/Dockerfile`, add it to `build/docker/build_images.sh` and `build/docker/run.sh`. The target then runs in the image `rockbot-<name>` automatically.
4. Add a row to the table above and in `docs/Docker.md` and `docs/DockerImages.md`.

## Binary names

Every build writes `rockbot_<platform><ext>` (set once in `CMakeLists.txt`, `ROCKBOT_PLATFORM_NAME`):
`rockbot_linux`, `rockbot_windows.exe`, `rockbot_osx`, `rockbot_raspberry_<arch>`, `rockbot_playstation2.elf`,
`rockbot_playstation3`, `rockbot_psp`, `rockbot_vita`, `rockbot_switch.nro`, `rockbot_wii.dol`,
`rockbot_gamecube.dol`, `rockbot_dingux.dge`, `rockbot_pocketgo`, `rockbot_pandora`. The package scripts keep
that name inside the package, except where the loader demands a fixed one: `boot.dol` (Wii Homebrew
Channel), `EBOOT.PBP` (PSP), `EBOOT.BIN` (PS3) and `eboot.bin` (Vita). A new port gets its name from the same
block in `CMakeLists.txt`.
