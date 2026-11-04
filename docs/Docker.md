# Building with Docker

Every platform can be built and packaged inside a Docker image, so the build does not depend on what is
installed on the machine, and the whole build environment can be backed up and restored.

## Images

| Image | Platform | Base | Contents | Dockerfile |
|---|---|---|---|---|
| `rockbot-linux` | Linux x64: tar.gz, deb, AppImage | Ubuntu 22.04 | SDL2 dev libs, dpkg, appimagetool | `ports/linux/Dockerfile` |
| `rockbot-rpi` | Raspberry Pi armhf / arm64 | Debian bookworm | cross compilers, ARM SDL2 libs | `ports/raspberry/Dockerfile` |
| `rockbot-psp` | PSP | Ubuntu 26.04 | pspdev SDK | `ports/docker/Dockerfile` (target `psp`) |
| `rockbot-vita` | PS Vita | Ubuntu 26.04 | VitaSDK, SDL2 libs (vdpm) | same, target `vita` |
| `rockbot-switch` | Nintendo Switch | Ubuntu 26.04 | devkitA64, libnx, switch-sdl2 portlibs (devkitPro) | same, target `switch` |
| `rockbot-ps2` | PlayStation 2 | Ubuntu 26.04 | ps2dev SDK | same, target `ps2` |
| `rockbot-ps3` | PlayStation 3 | Ubuntu 26.04 | ps3dev (PSL1GHT) SDK | same, target `ps3` |
| `rockbot-wii` | Nintendo Wii and GameCube | Ubuntu 26.04 | devkitPro | same, target `wii` |
| `rockbot-windows` | Windows (MinGW from Linux) | Ubuntu 26.04 | llvm-mingw, SDL2/SDL3 Windows libs | same, target `windows` |
| `rockbot-dingux` | Dingoo A320 (Dingux) | Ubuntu 26.04 | OpenDingux 1.2.5 toolchain (GCC 4.7.1, 32-bit host, uClibc 0.9.31) | same, target `dingux` |
| `rockbot-pocketgo` | PocketGo | Ubuntu 26.04 | Buildroot toolchain | same, target `pocketgo` |
| `rockbot-pandora` | OpenPandora | Ubuntu 26.04 | GCC 16 cross compiler (built by `ports/pandora/build-toolchain.sh`) + OpenPandora sysroot (glibc 2.9, SDL 1.2), squashfs-tools | same, target `pandora` |
| `rockbot-android` | Android | Ubuntu 26.04 | JDK 21, Android SDK, NDK r29, commandergenius | `ports/android/Dockerfile` |

The Linux image stays on an older Ubuntu on purpose: the binaries it produces are distributed, and a
newer glibc would limit them to very recent systems. The SDK images use the newest Ubuntu LTS (the pspdev
nightly already needs glibc 2.38 or newer).

Each console SDK is baked into its image at the **same absolute path** the build scripts default to
(for example `/mnt/ExtraData/Devel/SDK/PSP/pspdev-v20261101/pspdev`), so `build-*.sh` and
`build/packages/*.sh` run unchanged inside the container.

## Where Docker keeps its data

The Docker data directory is `/mnt/ExtraData/Devel/SDK/Docker/data` (`data-root` in
`/etc/docker/daemon.json`, plus a systemd drop-in `RequiresMountsFor=/mnt/ExtraData` so Docker starts only
after the disk is mounted).

## Building the images

```
build/docker/build_images.sh            # every SDK image (psp vita switch ps2 ps3 wii windows dingux pocketgo pandora android)
build/docker/build_images.sh psp ps3    # only some
docker build -t rockbot-linux ports/linux
docker build -t rockbot-rpi ports/raspberry
```

`build_images.sh` takes the SDKs from `SDK_DIR` (default `/mnt/ExtraData/Devel/SDK`) through a Docker
named build context, so nothing is copied into the repository.

The Android image is built from scratch (cmdline tools, SDK platform and build-tools, NDK, a pinned commit
of the commandergenius fork without its ~20 application submodules). The local changes to commandergenius
(NDK 28+ fix, Rockbot button images, manifest and `Globals.java` tweaks) are patches in
`ports/android/overlay/`.

## Running a build

```
build/docker/run.sh <platform> <command...>
build/docker/run.sh linux ./build-linux.sh sdl2
build/docker/run.sh psp ports/psp/build.sh
build/docker/run.sh ps3 ports/ps3/build.sh --no-package
cd build/packages && ../docker/run.sh linux ./deb.sh 1
```

`run.sh` mounts the repository at the same path as on the host, runs as your user (files are not owned by
root) and keeps the directory you called it from. Builds inside Docker use their own directory where a
native build would clash (`build-linux-docker-sdl*`).

`build-raspberry.sh` is itself the launcher of the Raspberry Pi image: run it directly on the host.

## Full release in Docker

```
build/packages/release_all.sh --bump minor --game both --docker
build/packages/release_all.sh --bump none --game 1 --docker --only psp,ps3
```

`--docker` (or `ROCKBOT_DOCKER=1`) runs every target in its image; the Raspberry Pi targets run as they
are. See `docs/Releasing.md` for the rest of the release flow.

## Backup and restore

```
build/docker/export.sh                # every rockbot-* image -> SDK/Docker/export/<image>.tar.zst (~8 GB)
build/docker/import.sh                # load them all back, e.g. after reinstalling the machine
```

Copy `/mnt/ExtraData/Devel/SDK/Docker/export/` to the cloud. The repository itself is in git; the images
can also be rebuilt from the Dockerfiles and the SDK folders.

## Debugging the PSP on real hardware

`ports/psp/psplink.sh` serves a staged build to a PSP running psplink (`stage`, `host`, `shell`, `addr`).
A crash prints the exception and the address, which `addr` maps to a function. The game also writes
`rockbot_boot.log` and `rockbot_stdout.log` next to `EBOOT.PBP`; every 2 seconds the boot log has a `perf:`
line (fps, game ticks per second, time per frame step). Optional marker files next to `EBOOT.PBP`:
`stretch.txt` (stretch instead of 1:1 with borders), `rgb565.txt` (16-bit screen texture), `no_music.txt`.

## Where the console SDKs come from (to recreate them)

- **Vita:** `bootstrap-vitasdk.sh` from `github.com/vitasdk/vdpm` into `SDK/Vita/vitasdk`, then
  `vdpm install sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_gfx`.
- **Switch:** `docker pull devkitpro/devkita64`, run it once with
  `dkp-pacman -Sy --noconfirm switch-sdl2 switch-sdl2_image switch-sdl2_mixer switch-sdl2_ttf switch-sdl2_gfx switch-pkg-config`,
  copy `/opt/devkitpro` to `SDK/Switch/devkitpro` and replace `/opt/devkitpro` with that path in the
  scripts and `.pc` files (`grep -rIl /opt/devkitpro ... | xargs sed -i`). The Wii SDK is made the same way (`build-wii.sh`).
- **Pandora:** the OpenPandora SDK (`SDK/OpenPandora`, sysroot with glibc 2.9 and SDL 1.2) only ships GCC 4.3.3 /
  Sourcery 4.6.1, too old for the game's C++11. `ports/pandora/build-toolchain.sh` builds binutils and the
  newest GCC against that sysroot into `SDK/Pandora/gcc-<version>` (run it in a throwaway `ubuntu:26.04`
  container, see the header of the script).
