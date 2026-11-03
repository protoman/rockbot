#!/usr/bin/env bash
# Builds RockBot for the PlayStation 3 (CMake + ps3dev/PSL1GHT) and creates:
#   - Rockbot<N>_PS3_<version>.pkg   installer for a PS3 with custom firmware / HEN
#                                     (copy it to a USB drive: Install Package Files)
#   - Rockbot<N>_PS3_<version>.zip   the bootable folder, for RPCS3 (rpcs3 <folder>/USRDIR/EBOOT.BIN)
# Usage: ./PS3.sh [1|2]     (asks which game if omitted)
# PS3DEV selects the SDK (see ../../ports/ps3/build.sh).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1

GAME_DIR="Rockbot$version_number"
OUT_PKG="$PWD/Rockbot${version_number}_PS3_$VERSIONNAME.pkg"
OUT_ZIP="$PWD/Rockbot${version_number}_PS3_$VERSIONNAME.zip"

# Compiles, stages build-ps3/PS3_GAME and builds the .pkg
../../ports/ps3/build.sh "$version_number" --pkg

BUILD_DIR="$SCRIPT_DIR/../../build-ps3"
if [ ! -f "$BUILD_DIR/Rockbot${version_number}.pkg" ]; then
    echo "error: the installer was not produced" >&2
    exit 1
fi
cp "$BUILD_DIR/Rockbot${version_number}.pkg" "$OUT_PKG"

# Bootable folder for RPCS3 / a CFW console through /app_home
rm -rf "./PS3/$GAME_DIR"
mkdir -p "./PS3/$GAME_DIR"
cp -r "$BUILD_DIR/PS3_GAME/." "./PS3/$GAME_DIR/"
rm -f "$OUT_ZIP"
( cd PS3 && zip -qr "$OUT_ZIP" "$GAME_DIR" )

echo "Installer: $OUT_PKG"
echo "Folder zip: $OUT_ZIP"
