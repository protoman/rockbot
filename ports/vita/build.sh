#!/usr/bin/env bash
# Build and package RockBot for the PlayStation Vita (VitaSDK, SDL2).
#
# Usage:
#   ports/vita/build.sh [1|2] [--no-package]
#
#   1|2           which game to package (default 1): build/games/Rockbot<N>
#   --no-package  only compile
#
# Output: res/vita/Rockbot<N>.vpk  (install with VitaShell, or drop it on Vita3K).
#         The game data is inside the VPK (app0:); saves and rockbot_boot.log go to ux0:data/rockbot/.
#
# VITASDK must point at a VitaSDK installation (default /mnt/ExtraData/Devel/SDK/Vita/vitasdk).
# Install: https://vitasdk.org (bootstrap script), then `vdpm install sdl2 sdl2_image sdl2_mixer sdl2_ttf sdl2_gfx`.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export VITASDK="${VITASDK:-/mnt/ExtraData/Devel/SDK/Vita/vitasdk}"
export PATH="$VITASDK/bin:$PATH"

GAME_NUMBER=1
PACKAGE=1
for arg in "$@"; do
    case "$arg" in
        1|2) GAME_NUMBER="$arg" ;;
        --no-package) PACKAGE=0 ;;
        *) echo "Usage: $0 [1|2] [--no-package]" >&2; exit 1 ;;
    esac
done

if [ ! -x "$VITASDK/bin/arm-vita-eabi-g++" ]; then
    echo "error: VitaSDK not found under $VITASDK (set VITASDK)" >&2
    exit 1
fi

"$ROOT/build/packages/sync_version.sh"   # file/version.h <- version_name.txt

BUILD="$ROOT/res/vita"
GAME_DATA="$ROOT/build/games/Rockbot${GAME_NUMBER}"
# Vita title ids are exactly 9 characters: 4 letters + 5 digits.
TITLE_ID="ROCK0000${GAME_NUMBER}"
TITLE="RockBot ${GAME_NUMBER}"

cmake -S "$ROOT" -B "$BUILD" \
    -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" -j"$(nproc)"

[ "$PACKAGE" = 1 ] || exit 0

if [ ! -d "$GAME_DATA" ]; then
    echo "error: game data not found: $GAME_DATA" >&2
    exit 1
fi

# APP_VER is MM.NN, taken from build/packages/version_name.txt
APP_VER="01.00"
VERSION_FILE="$ROOT/build/packages/version_name.txt"
if [ -f "$VERSION_FILE" ]; then
    IFS=. read -r v_major v_minor _ <<< "$(tr -d '[:space:]' < "$VERSION_FILE")"
    APP_VER="$(printf '%02d.%02d' "$((10#$v_major))" "$((10#$v_minor))")"
fi

STAGE="$BUILD/vpk_${GAME_NUMBER}"
rm -rf "$STAGE"
mkdir -p "$STAGE/sce_sys" "$STAGE/games"
cp -r "$GAME_DATA" "$STAGE/games/"
cp -r "$ROOT/build/shared" "$ROOT/build/fonts" "$STAGE/"
find "$STAGE" \( -name '.svn' -o -name 'errors.log' -o -name '*.sav' \) -prune -exec rm -rf {} +

# LiveArea icon (128x128) from the game logo
if command -v convert >/dev/null 2>&1 && [ -f "$GAME_DATA/images/logo.png" ]; then
    convert "$GAME_DATA/images/logo.png" -background '#001533' -resize 128x128 \
        -gravity center -extent 128x128 "PNG8:$STAGE/sce_sys/icon0.png"
else
    echo "error: ImageMagick (convert) is needed for the Vita icon" >&2
    exit 1
fi

vita-mksfoex -s TITLE_ID="$TITLE_ID" -s APP_VER="$APP_VER" "$TITLE" "$BUILD/param_${GAME_NUMBER}.sfo"

# one -a src=dest per file (everything under the stage folder except sce_sys, which is added below)
ARGS=()
while IFS= read -r -d '' f; do
    ARGS+=(-a "$f=${f#"$STAGE"/}")
done < <(find "$STAGE" -type f ! -path "$STAGE/sce_sys/*" -print0)
ARGS+=(-a "$STAGE/sce_sys/icon0.png=sce_sys/icon0.png")

OUT="$BUILD/Rockbot${GAME_NUMBER}.vpk"
rm -f "$OUT"
vita-pack-vpk -s "$BUILD/param_${GAME_NUMBER}.sfo" -b "$BUILD/eboot.bin" "${ARGS[@]}" "$OUT"
echo "Vita package: $OUT"
