#!/usr/bin/env bash
# Build and package RockBot for the Nintendo Switch (devkitA64 + libnx + SDL2).
#
# Usage:
#   ports/switch/build.sh [1|2] [--no-package]
#
#   1|2           which game to package (default 1): build/games/RockDroid<N>
#   --no-package  only compile
#
# Output: res/switch/Rockbot<N>_Switch/   the folder to copy to the SD card as /switch/rockbot/
#                                           (rockbot_switch.nro + games/ shared/ fonts/; saves go there too)
#         res/switch/Rockbot<N>_Switch.zip
#
# DEVKITPRO must point at a devkitPro tree with devkitA64, libnx and the switch-sdl2* portlibs
# (default /mnt/ExtraData/Devel/SDK/Switch/devkitpro). It is made once by copying /opt/devkitpro out of the
# official devkitpro/devkita64 Docker image after `dkp-pacman -S switch-sdl2 switch-sdl2_image
# switch-sdl2_mixer switch-sdl2_ttf switch-sdl2_gfx switch-pkg-config` (see docs/Docker.md).

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DEVKITPRO="${DEVKITPRO:-/mnt/ExtraData/Devel/SDK/Switch/devkitpro}"
export DEVKITA64="$DEVKITPRO/devkitA64"
export PATH="$DEVKITA64/bin:$DEVKITPRO/tools/bin:$PATH"

GAME_NUMBER=1
PACKAGE=1
for arg in "$@"; do
    case "$arg" in
        1|2) GAME_NUMBER="$arg" ;;
        --no-package) PACKAGE=0 ;;
        *) echo "Usage: $0 [1|2] [--no-package]" >&2; exit 1 ;;
    esac
done

if [ ! -x "$DEVKITA64/bin/aarch64-none-elf-g++" ]; then
    echo "error: devkitA64 not found under $DEVKITPRO (set DEVKITPRO)" >&2
    exit 1
fi

"$ROOT/build/packages/sync_version.sh"   # file/version.h <- version_name.txt

BUILD="$ROOT/res/switch"
GAME_DATA="$ROOT/build/games/RockDroid${GAME_NUMBER}"
VERSION_NAME="0.0.0"
[ -f "$ROOT/build/packages/version_name.txt" ] && VERSION_NAME="$(tr -d '[:space:]' < "$ROOT/build/packages/version_name.txt")"

mkdir -p "$BUILD"
ICON="$BUILD/icon_${GAME_NUMBER}.jpg"
if command -v convert >/dev/null 2>&1 && [ -f "$GAME_DATA/images/logo.png" ]; then
    convert "$GAME_DATA/images/logo.png" -background '#001533' -resize 256x256 -gravity center -extent 256x256 \
        -flatten "$ICON"
else
    ICON=""
fi

cmake -S "$ROOT" -B "$BUILD" \
    -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/Switch.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DROCKBOT_VERSION_NUMBER="$GAME_NUMBER" \
    -DROCKBOT_VERSION_NAME="$VERSION_NAME" \
    -DROCKBOT_NRO_ICON="$ICON"
cmake --build "$BUILD" -j"$(nproc)"

[ "$PACKAGE" = 1 ] || exit 0

if [ ! -d "$GAME_DATA" ]; then
    echo "error: game data not found: $GAME_DATA" >&2
    exit 1
fi

STAGE="$BUILD/Rockbot${GAME_NUMBER}_Switch"
rm -rf "$STAGE"
mkdir -p "$STAGE/games"
cp "$BUILD/rockbot_switch.nro" "$STAGE/rockbot_switch.nro"
cp -r "$GAME_DATA" "$STAGE/games/"
cp -r "$ROOT/build/shared" "$ROOT/build/fonts" "$STAGE/"
find "$STAGE" \( -name '.svn' -o -name 'errors.log' -o -name '*.sav' \) -prune -exec rm -rf {} +

rm -f "$STAGE.zip"
( cd "$BUILD" && zip -qr "$(basename "$STAGE").zip" "$(basename "$STAGE")" )
echo "Switch package folder: $STAGE   (copy to the SD card as /switch/rockbot/)"
echo "Switch package zip: $STAGE.zip"
