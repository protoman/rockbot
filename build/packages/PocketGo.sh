#!/usr/bin/env bash
# Builds the PocketGo package (CMake + Buildroot SDK) and zips it.
# Usage: ./PocketGo.sh [1|2]     (asks which game if omitted)
# SDK location: see ../../build-pocketgo.sh (env POCKETGO_SDK).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
GAME_DIR=Rockbot$version_number
OUT_ZIP="$PWD/Rockbot${version_number}_PocketGo_$VERSIONNAME.zip"

# Produces ../rockbot_pocketgo
../../build-pocketgo.sh

rm -rf ./pocketgo
mkdir -p "./pocketgo/$GAME_DIR/games"

cp ../rockbot_pocketgo "./pocketgo/$GAME_DIR/"
rsync -r --exclude=.svn ../fonts "./pocketgo/$GAME_DIR/"
rsync -r --exclude=.svn ../shared "./pocketgo/$GAME_DIR/"
rsync -r --exclude=.svn "../games/RockDroid$version_number" "./pocketgo/$GAME_DIR/games"

cp "./pocketgo/$GAME_DIR/games/RockDroid$version_number/images/icon_32px.png" "./pocketgo/$GAME_DIR/rockbot.png"
find "./pocketgo/$GAME_DIR" \( -name 'game*.sav' -o -name 'config*.sav' \) -delete

# Strip the staged copy (the build tree keeps the debug info).
STRIP=$(ls ${POCKETGO_SDK:-/mnt/ExtraData/Devel/SDK/PocketGo/output/host}/bin/*-strip 2>/dev/null | head -1 || true)
[ -n "$STRIP" ] && "$STRIP" "./pocketgo/$GAME_DIR/rockbot_pocketgo" || true

rm -f "$OUT_ZIP"
( cd pocketgo && zip -qr "$OUT_ZIP" "$GAME_DIR" )
echo "Package: $OUT_ZIP"
