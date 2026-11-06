#!/usr/bin/env bash
# Builds the OpenPandora package (.pnd = squashfs image + PXML + icon) with the CMake build.
# Usage: ./Pandora.sh [1|2]     (asks which game if omitted)
# SDK/toolchain location: see ../../build-pandora.sh. Needs mksquashfs (squashfs-tools).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
GAME_DIR=rockbot$version_number
OUT_PND="$PWD/Rockbot${version_number}_Pandora_$VERSIONNAME.pnd"
STAGE="$PWD/pandora/$GAME_DIR"

# Produces ../rockbot_pandora
../../build-pandora.sh

rm -rf "$STAGE"
mkdir -p "$STAGE"
cp -r ./pandora/project/. "$STAGE/"
cp ../rockbot_pandora "$STAGE/rockbot_pandora"
stage_game_data "$STAGE"
# SDL_gfx is not part of every firmware: ship it (game.sh adds ./lib to LD_LIBRARY_PATH)
PANDORA_SYSROOT="${PANDORA_SYSROOT:-/mnt/ExtraData/Devel/SDK/OpenPandora/arm-angstrom-linux-gnueabi}"
mkdir -p "$STAGE/lib"
cp -L "$PANDORA_SYSROOT/usr/lib/libSDL_gfx.so.0" "$STAGE/lib/"
cp "$STAGE/games/RockDroid$version_number/images/icon_32px.png" "$STAGE/icon.png" 2>/dev/null || cp "$STAGE/rockbot.png" "$STAGE/icon.png"

# Strip the staged copy (the build tree keeps the debug info).
STRIP="$(ls "${PANDORA_TOOLCHAIN:-/mnt/ExtraData/Devel/SDK/Pandora}"/*/bin/arm-angstrom-linux-gnueabi-strip /mnt/ExtraData/Devel/SDK/Pandora/*/bin/arm-angstrom-linux-gnueabi-strip 2>/dev/null | head -1 || true)"
[ -n "$STRIP" ] && "$STRIP" "$STAGE/rockbot_pandora" || true

# PXML with the release version (M.NN.PPP)
IFS=. read -r V_MAJ V_MIN V_MINI <<< "$(get_version)"
sed -e "s/@MAJOR@/$((10#$V_MAJ))/g" -e "s/@MINOR@/$((10#$V_MIN))/g" -e "s/@MINI@/$((10#$V_MINI))/g" \
    ./pandora/project/pxml.xml > "$STAGE/PXML.xml"
rm -f "$STAGE/pxml.xml"

# .pnd = filesystem image, then the PXML and the icon appended (what pnd_make does)
rm -f "$OUT_PND" "$OUT_PND.img"
mksquashfs "$STAGE" "$OUT_PND.img" -nopad -no-recovery -noappend -quiet
cat "$OUT_PND.img" "$STAGE/PXML.xml" "$STAGE/icon.png" > "$OUT_PND"
rm -f "$OUT_PND.img"
echo "Package: $OUT_PND"
