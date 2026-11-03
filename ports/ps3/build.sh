#!/usr/bin/env bash
# Build and package RockBot for the PlayStation 3 (PSL1GHT).
#
# Usage:
#   ports/ps3/build.sh [--no-package]
#
# PS3DEV must point at a ps3dev installation (default
# /mnt/ExtraData/Devel/SDK/PS3/ps3dev). The nightly all-in-one release from
# https://github.com/ps3dev/ps3dev/releases includes the PPU toolchain,
# PSL1GHT and the SDL 1.2 portlibs.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PS3DEV="${PS3DEV:-/mnt/ExtraData/Devel/SDK/PS3/ps3dev}"
export PS3DEV
export PSL1GHT="$PS3DEV"

BUILD="$ROOT/build-ps3"
STAGE="$BUILD/PS3_GAME"
APPID="ROCKBOT01"

if [ ! -x "$PS3DEV/ppu/bin/ppu-g++" ]; then
    echo "error: PPU toolchain not found under $PS3DEV" >&2
    echo "       set PS3DEV to your ps3dev installation" >&2
    exit 1
fi

cmake -S "$ROOT" -B "$BUILD" \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/toolchain-ps3.cmake" \
    -DPS3DEV="$PS3DEV" \
    -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD" -j"$(nproc)"

if [ "${1:-}" = "--no-package" ]; then
    exit 0
fi

# --- Package a folder RPCS3 (and a CFW PS3 via /app_home) can boot ----------
rm -rf "$STAGE"
mkdir -p "$STAGE/USRDIR/games"

"$PS3DEV/ppu/bin/ppu-strip" --strip-all "$BUILD/rockbot" -o "$BUILD/rockbot_stripped"
cp "$BUILD/rockbot_stripped" "$BUILD/rockbot_linked"
"$PS3DEV/bin/sprxlinker" "$BUILD/rockbot_linked"
"$PS3DEV/bin/fself" "$BUILD/rockbot_linked" "$STAGE/USRDIR/EBOOT.BIN"

# The game is started without menus when only one game folder is present.
cp -r "$ROOT/build/games/RockDroid1" "$STAGE/USRDIR/games/"
cp -r "$ROOT/build/shared" "$STAGE/USRDIR/"
cp -r "$ROOT/build/fonts" "$STAGE/USRDIR/"

# SDL_image on PS3 fails on the game's single grayscale PNG ("Out of memory").
# Convert any grayscale PNG in the packaged data to RGB.
if command -v convert >/dev/null 2>&1; then
    find "$STAGE/USRDIR" -name '*.png' -print0 | while IFS= read -r -d '' png; do
        if file "$png" | grep -q 'grayscale'; then
            convert "$png" -type TrueColor "PNG24:$png.tmp" && mv "$png.tmp" "$png"
        fi
    done
fi

"$PS3DEV/bin/sfo" -f "$PS3DEV/bin/sfo.xml" \
    --title="RockBot" --appid="$APPID" "$STAGE/PARAM.SFO"
cp "$PS3DEV/bin/ICON0.PNG" "$STAGE/ICON0.PNG"

echo "PS3 package: $STAGE"
echo "Boot with: rpcs3 $STAGE/USRDIR/EBOOT.BIN"
