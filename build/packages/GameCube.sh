#!/usr/bin/env bash
# Nintendo GameCube package: cross-builds boot.dol (devkitPro, see ../../build-gamecube.sh) and creates
# Rockbot<N>_GameCube_<version>.zip. Unzip it into the root of the SD card (SD Gecko / SD2SP2); the
# folder rockbot<N>/ holds rockbot.dol and the game data. Run rockbot.dol with Swiss or any DOL loader.
# Usage: ./GameCube.sh [1|2]     (asks which game if omitted)
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1

GAME_DIR="rockbot$version_number"
OUT="$PWD/Rockbot${version_number}_GameCube_$VERSIONNAME.zip"
APP="./GameCube/$GAME_DIR"

../../build-gamecube.sh

rm -rf ./GameCube
mkdir -p "$APP"
cp ../../build-gamecube/boot.dol "$APP/rockbot.dol"
stage_game_data "$APP"

rm -f "$OUT"
( cd ./GameCube && zip -qr "$OUT" . )
echo "Package: $OUT"
