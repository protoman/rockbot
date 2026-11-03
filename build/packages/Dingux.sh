#!/usr/bin/env bash
# Builds the Dingux package (CMake + Buildroot SDK) and zips it.
# Usage: ./Dingux.sh [1|2]     (asks which game if omitted)
# SDK location: see ../../build-dingux.sh (env OPEN_DINGUX_SDK).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
GAME_DIR=Rockbot$version_number
OUT_ZIP="$PWD/Rockbot${version_number}_Dingux_$VERSIONNAME.zip"

# Produces ../rockbot.dge
../../build-dingux.sh

rm -rf ./Dingux
mkdir -p "./Dingux/$GAME_DIR/games"

cp ../rockbot.dge "./Dingux/$GAME_DIR/"
rsync -r --exclude=.svn ../fonts "./Dingux/$GAME_DIR/"
rsync -r --exclude=.svn ../shared "./Dingux/$GAME_DIR/"
rsync -r --exclude=mp3 --exclude=.svn "../games/RockDroid$version_number" "./Dingux/$GAME_DIR/games"

cp "./Dingux/$GAME_DIR/games/RockDroid$version_number/images/icon_32px.png" "./Dingux/$GAME_DIR/rockbot.png"
find "./Dingux/$GAME_DIR" \( -name 'game*.sav' -o -name 'config*.sav' \) -delete

# Strip the staged copy (the build tree keeps the debug info).
STRIP=$(ls ${OPEN_DINGUX_SDK:-/mnt/ExtraData/Devel/SDK/opendingux-gcw0-2022-09-22/gcw0-toolchain}/bin/*-strip 2>/dev/null | head -1 || true)
[ -n "$STRIP" ] && "$STRIP" "./Dingux/$GAME_DIR/rockbot.dge" || true

rm -f "$OUT_ZIP"
( cd Dingux && zip -qr "$OUT_ZIP" "$GAME_DIR" )
echo "Package: $OUT_ZIP"
