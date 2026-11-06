# Building the Docker images

How the `rockbot-*` build images are made, and where each SDK comes from. To just **use** the images, see
[Docker.md](Docker.md).

> **Note: the example paths here are from the maintainer's own machine.** The SDKs live in
> `/mnt/ExtraData/Devel/SDK` and Docker's data in `/mnt/ExtraData/Devel/SDK/Docker/data` there. That is *only*
> an example of one setup: use any folders you like (see "Paths" below). The SDKs are not part of the
> repository and each one has to be downloaded from its own project.

## Paths

* **Docker's data directory** is a setting of your Docker daemon (`data-root` in `/etc/docker/daemon.json`),
  not of the images. Leave the default (`/var/lib/docker`) unless you need the images on another disk.
  (The maintainer moved it to a second disk and added a systemd drop-in `RequiresMountsFor=` so Docker
  starts only after that disk is mounted.)
* **`SDK_DIR`** is the folder that holds the SDKs on the host (default `/mnt/ExtraData/Devel/SDK`). Set it to
  wherever you keep them: `SDK_DIR=$HOME/sdk build/docker/build_images.sh`.
* **Inside the images** each SDK is copied to the *same absolute path* the build scripts default to
  (`/mnt/ExtraData/Devel/SDK/...`), so `build-*.sh` and `build/packages/*.sh` run unchanged in the container.
  This is a path inside the container: it does not matter where your SDKs are on the host, and nothing on your
  machine needs to exist at `/mnt/ExtraData`.
* `DOCKER_EXPORT_DIR` is where `export.sh` / `import.sh` write and read the image archives
  (default `/mnt/ExtraData/Devel/SDK/Docker/export`).

## Building the images

```
build/docker/build_images.sh            # every image (psp vita switch ps2 ps3 wii windows dingux pocketgo pandora android)
build/docker/build_images.sh psp ps3    # only some
docker build -t rockbot-linux ports/linux
docker build -t rockbot-rpi ports/raspberry
```

`build_images.sh` hands `SDK_DIR` to Docker as a named build context (`--build-context sdk=...`), so nothing is
copied into the repository. The Dockerfiles are `ports/docker/Dockerfile` (one target per console),
`ports/android/Dockerfile`, `ports/linux/Dockerfile` and `ports/raspberry/Dockerfile`.

| Image | Base | Dockerfile |
|---|---|---|
| `rockbot-linux` | Ubuntu 22.04 | `ports/linux/Dockerfile` |
| `rockbot-rpi` | Debian bookworm | `ports/raspberry/Dockerfile` |
| `rockbot-android` | Ubuntu 26.04 | `ports/android/Dockerfile` |
| all the console images | Ubuntu 26.04 | `ports/docker/Dockerfile` (target = platform) |

The Linux image stays on an older Ubuntu on purpose: the binaries it produces are distributed, and a newer
glibc would limit them to very recent systems. The SDK images use the newest Ubuntu LTS (the pspdev nightly
already needs glibc 2.38 or newer).

## Where each SDK comes from

Folders are relative to `SDK_DIR`. "Layout" is what `ports/docker/Dockerfile` copies into the image.

### No SDK to download

| Image | What it uses |
|---|---|
| `rockbot-linux` | Ubuntu packages (SDL2 and friends) and [appimagetool](https://github.com/AppImage/appimagetool/releases), downloaded by the Dockerfile |
| `rockbot-rpi` | Debian cross compilers and ARM SDL2 packages |
| `rockbot-android` | everything is downloaded while the image builds (see "Android" below) |
| `rockbot-windows` | the build script downloads the toolchain and libraries on first use (see "Windows") |

### PlayStation Portable: `PSP/pspdev-<tag>/pspdev`

[pspdev](https://github.com/pspdev/pspdev): download the prebuilt toolchain from the
[releases page](https://github.com/pspdev/pspdev/releases) (a `pspdev-*-x86_64.tar.gz`) and extract it, which
gives a `pspdev` folder. The Dockerfile expects the release tag in the path (for example
`PSP/pspdev-v20261101/pspdev`). Needs glibc 2.38 or newer.

### PlayStation Vita: `Vita/vitasdk`

[VitaSDK](https://vitasdk.org) ([vdpm](https://github.com/vitasdk/vdpm)):

```
git clone https://github.com/vitasdk/vdpm && cd vdpm
export VITASDK=<SDK_DIR>/Vita/vitasdk
./bootstrap-vitasdk.sh
$VITASDK/bin/vdpm sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_gfx
```

### Nintendo Switch: `Switch/devkitpro`

[devkitPro](https://devkitpro.org) ([installation](https://devkitpro.org/wiki/Getting_Started)): the easy way
is the [`devkitpro/devkita64`](https://hub.docker.com/r/devkitpro/devkita64) image. Run it once, install the
libraries and copy `/opt/devkitpro` out:

```
docker run --rm -it -v <SDK_DIR>/Switch:/out devkitpro/devkita64 bash
  dkp-pacman -Sy --noconfirm switch-sdl2 switch-sdl2_image switch-sdl2_mixer switch-sdl2_ttf switch-sdl2_gfx switch-pkg-config
  cp -a /opt/devkitpro /out/devkitpro
```

Then replace `/opt/devkitpro` with the new path in the scripts and `.pc` files
(`grep -rIl /opt/devkitpro <SDK_DIR>/Switch/devkitpro | xargs sed -i 's|/opt/devkitpro|<SDK_DIR>/Switch/devkitpro|g'`).

### Nintendo Wii and GameCube: `Wii/devkitpro-current`

[devkitPro](https://devkitpro.org) devkitPPC + libogc and the Wii SDL 1.2 ports. `build-wii.sh` installs it by
itself the first time it runs: it copies `/opt/devkitpro` out of the official
[`devkitpro/devkitppc`](https://hub.docker.com/r/devkitpro/devkitppc) Docker image into `WII_SDK` (default
`SDK_DIR/Wii/devkitpro-current`) and relocates its paths (Docker is used only for that copy). So run
`./build-wii.sh` once, then build the image. GameCube (`build-gamecube.sh`) uses the same folder.

### PlayStation 2: `PS2.new/ps2dev`

[ps2dev](https://github.com/ps2dev/ps2dev): build it from source as its README says, or take the toolchain
from the [`ps2dev/ps2dev`](https://hub.docker.com/r/ps2dev/ps2dev) Docker image (`/usr/local/ps2dev`). The
folder must contain `ee/`, `iop/`, `ps2sdk/` and `gsKit/`.

### PlayStation 3: `PS3/ps3dev`

[ps3dev](https://github.com/ps3dev/ps3dev): the prebuilt all-in-one nightly from the
[releases page](https://github.com/ps3dev/ps3dev/releases) is enough (PPU GCC, PSL1GHT, portlibs); nothing has
to be compiled. See `ports/ps3/README.md` for the version the port was tested with.

### Windows: `Windows/llvm-mingw-<date>-ucrt-ubuntu-22.04-x86_64` and `Windows/deps`

Both are created by `build-windows.sh` the first time it runs (on any Linux machine with `cmake make pkg-config
curl tar rsync zip`), into `WINDOWS_SDK` (default `SDK_DIR/Windows`):

* [llvm-mingw](https://github.com/mstorsjo/llvm-mingw/releases) (clang + LLD + MinGW-w64, UCRT);
* the official MinGW development packages of [SDL](https://github.com/libsdl-org/SDL/releases),
  [SDL_image](https://github.com/libsdl-org/SDL_image/releases),
  [SDL_mixer](https://github.com/libsdl-org/SDL_mixer/releases) and
  [SDL_ttf](https://github.com/libsdl-org/SDL_ttf/releases) (SDL2 and SDL3);
* [SDL2_gfx](https://sourceforge.net/projects/sdl2gfx/), built by the script.

So: run `./build-windows.sh` (and `./build-windows.sh sdl3`, `sdl2 i686` for the other variants) once, then build
the image. Details in `ports/windows/README.md`.

### Dingoo A320 (Dingux): `opendingux`

The legacy Dingoo A320 toolchain: OpenDingux 1.2.5 / GCC 4.7.1 / uClibc 0.9.31, made of **32-bit x86**
programs (the image installs the i386 libraries for it). The script checks for
`opendingux/usr/bin/mipsel-linux-gcc`. Sources: the [OpenDingux project](https://github.com/OpenDingux) and its
[toolchain / nightly builds page](https://pcercuei.gitlab.io/OpenDingux.html); the original Dingux toolchain is
referenced by the [opendingux-kernel README-A320](https://github.com/mthuurne/opendingux-kernel).
*(The maintainer's copy was downloaded a long time ago and these pages are the closest public sources found:
check that the toolchain you download matches the layout above.)*

### PocketGo: `PocketGo/output/host`

A [Buildroot](https://github.com/MiyooCFW/buildroot) SDK of the MiyooCFW project, which supports the PocketGo
(ARM, musl, static SDL 1.2). Build it with the PocketGo/musl defconfig and `make sdk`, and keep the
`output/host` folder (it must contain `share/buildroot/toolchainfile.cmake`). See the
[MiyooCFW build instructions](https://github.com/TriForceX/MiyooCFW/wiki/Build-Image-and-SDK). Buildroot host
tools carry the RUNPATH of the machine that built them, which is why the image sets `LD_LIBRARY_PATH`.

### OpenPandora: `OpenPandora/arm-angstrom-linux-gnueabi` and `Pandora/gcc-16.2.0`

* `OpenPandora/arm-angstrom-linux-gnueabi` is the **sysroot** (glibc 2.9, SDL 1.2) of the official OpenPandora
  SDK; download and install instructions are on the
  [Pandora wiki](https://pandorawiki.org/Cross-compiler) (it links the SDK installer).
* The SDK only ships GCC 4.3.3 / Sourcery 4.6.1, too old for the game's C++11, so `Pandora/gcc-16.2.0` is a
  newer GCC built against that sysroot by `ports/pandora/build-toolchain.sh` (downloads binutils and GCC from
  [ftp.gnu.org](https://ftp.gnu.org/gnu/); run it in a throwaway `ubuntu:26.04` container, the command is in
  the header of the script).

### Android

`ports/android/Dockerfile` builds the whole environment, no SDK folder is needed. It downloads the
[Android command line tools](https://developer.android.com/studio#command-line-tools-only) and, with them, the
SDK platforms and build-tools, the [NDK](https://developer.android.com/ndk/downloads), JDK 21 from Ubuntu, and a
pinned commit of Pelya's [commandergenius](https://github.com/pelya/commandergenius) (without its ~20
application submodules). The Rockbot changes (button images, manifest and `Globals.java` tweaks, the libmikmod
64-bit fix) are patches in `ports/android/overlay/`.

To move to a newer commandergenius: change `COMMANDERGENIUS_COMMIT` in the Dockerfile, check that the patches
still apply, rebuild the image (`build/docker/build_images.sh android`) and test the APK. The APK is signed
with your own keystore when you release (`build/packages/android_build.sh`).

## Backup and restore

```
build/docker/export.sh                # every rockbot-* image -> $DOCKER_EXPORT_DIR/<image>.tar.zst (~8 GB)
build/docker/import.sh                # load them all back, e.g. after reinstalling the machine
```

Copy the export folder to wherever you keep backups. The repository itself is in git; the images can also be
rebuilt from the Dockerfiles and the SDK folders above.
