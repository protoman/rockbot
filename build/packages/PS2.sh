#!/usr/bin/env bash
# Builds the PS2 package (CMake + ps2dev) and zips it.
# Usage: ./PS2.sh [1|2]     (asks which game if omitted)
# PS2DEV selects the SDK (see ../../build-ps2.sh).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
GAME_DIR=rockbot$version_number
OUT_ZIP="$PWD/Rockbot${version_number}_PS2_$VERSIONNAME.zip"

# Produces ../rockbot_playstation2.elf
../../build-ps2.sh

rm -rf ./PS2
mkdir -p "./PS2/$GAME_DIR/games" "./PS2/$GAME_DIR/modules" "./PS2/$GAME_DIR/data"

cp ../rockbot_playstation2.elf "./PS2/$GAME_DIR/"
cp ../../ports/ps2/modules/*.irx "./PS2/$GAME_DIR/modules/"

rsync -r --exclude=.svn ../fonts "./PS2/$GAME_DIR"
rsync -r --exclude=.svn ../shared "./PS2/$GAME_DIR"
rsync -r --exclude=.svn "../games/RockDroid$version_number" "./PS2/$GAME_DIR/games"

# Memory-card save icon (the game copies these on first run). icon.sys is
# optional: it is not tracked in the repo.
cp ./rockbot_ps2_icon.icn "./PS2/$GAME_DIR/shared/images/"
[ -f ./icon.sys ] && cp ./icon.sys "./PS2/$GAME_DIR/shared/images/" || echo "note: icon.sys not found, no save icon will be created"

find "./PS2/$GAME_DIR" \( -name 'game*.sav' -o -name 'config*.sav' \) -delete

rm -f "$OUT_ZIP"
( cd PS2 && zip -qr "$OUT_ZIP" "$GAME_DIR" )
echo "Package: $OUT_ZIP"
