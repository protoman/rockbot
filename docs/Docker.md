# Building Rockbot packages with Docker

Every platform can be built and packaged inside a Docker image, so the build does not depend on what is
installed on your machine. This page is about **using** the images. How the images themselves are made
(SDKs, where to download them) is in [DockerImages.md](DockerImages.md).

## What you need

* Docker (with `buildx`), a clone of this repository, and the images below.
* The images are not published anywhere: either build them once ([DockerImages.md](DockerImages.md)) or load
  them from an archive somebody exported (see "Loading exported images" at the end).

## Images

| Image | Platform | Used for |
|---|---|---|
| `rockbot-linux` | Linux x64 | `tar.gz`, `.deb`, AppImage |
| `rockbot-rpi` | Raspberry Pi armhf / arm64 | `tar.gz` |
| `rockbot-android` | Android | `.apk` |
| `rockbot-psp` | PlayStation Portable | `EBOOT.PBP` |
| `rockbot-vita` | PlayStation Vita | `.vpk` |
| `rockbot-switch` | Nintendo Switch | `.nro` |
| `rockbot-ps2` | PlayStation 2 | `.elf` |
| `rockbot-ps3` | PlayStation 3 | `.pkg` / folder |
| `rockbot-wii` | Nintendo Wii and GameCube | Homebrew Channel app, `.dol` |
| `rockbot-windows` | Windows x64 / i686 | `.zip` (cross-built with MinGW) |
| `rockbot-dingux` | Dingoo A320 (Dingux) | `.dge` |
| `rockbot-pocketgo` | PocketGo | `rockbot_pocketgo` |
| `rockbot-pandora` | OpenPandora | `.pnd` |

Only the images for the platforms you want to build are needed. The Linux, Raspberry Pi, Android and
WebAssembly builds need no console SDK at all.

## Running a build

The repository is mounted inside the container at the same path as on your machine, the build runs as your
user (files are not owned by root), and the directory you called it from is kept.

```
build/docker/run.sh <platform> <command...>

build/docker/run.sh linux ./build-linux.sh sdl2
build/docker/run.sh psp ports/psp/build.sh
build/docker/run.sh ps3 ports/ps3/build.sh --no-package
cd build/packages && ../docker/run.sh linux ./deb.sh 1
```

Builds inside Docker use their own output directory where a native build would clash
(for example `res/linux-docker-sdl2`).

`build-raspberry.sh` is itself the launcher of the Raspberry Pi image: run it directly on the host.

## Full release in Docker

```
build/packages/release_all.sh --bump minor --game both --docker
build/packages/release_all.sh --bump none --game 1 --docker --only psp,ps3
```

`--docker` (or `ROCKBOT_DOCKER=1`) runs every target in its image; the Raspberry Pi targets run as they
are. See [Releasing.md](Releasing.md) for the rest of the release flow (versions, package names, adding a new
port).

## Debugging the PSP on real hardware

`ports/psp/psplink.sh` serves a staged build to a PSP running psplink (`stage`, `host`, `shell`, `addr`).
A crash prints the exception and the address, which `addr` maps to a function. The game also writes
`rockbot_boot.log` and `rockbot_stdout.log` next to `EBOOT.PBP`; every 2 seconds the boot log has a `perf:`
line (fps, game ticks per second, time per frame step). Optional marker files next to `EBOOT.PBP`:
`stretch.txt` (stretch instead of 1:1 with borders), `rgb565.txt` (16-bit screen texture), `no_music.txt`.

## Loading exported images

Somebody who built the images can export them to compressed archives, so the others do not have to build
them (about 8 GB for all of them):

```
build/docker/import.sh                          # loads every <image>.tar.zst found in the export folder
DOCKER_EXPORT_DIR=/path/to/archives build/docker/import.sh
```

The export side (`build/docker/export.sh`) is described in [DockerImages.md](DockerImages.md).
