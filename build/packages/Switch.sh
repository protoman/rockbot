#!/usr/bin/env bash
# Builds the Nintendo Switch package (CMake + devkitA64) and copies the zip.
# Usage: ./Switch.sh [1|2]     (asks which game if omitted)
# DEVKITPRO selects the SDK (see ../../ports/switch/build.sh).
# The zip holds a Rockbot<N>_Switch folder: copy it to the SD card as /switch/rockbot/ (Homebrew Menu).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
OUT_ZIP="$PWD/Rockbot${version_number}_Switch_$VERSIONNAME.zip"

../../ports/switch/build.sh "$version_number"

BUILD_ZIP="$SCRIPT_DIR/../../build-switch/Rockbot${version_number}_Switch.zip"
if [ ! -f "$BUILD_ZIP" ]; then
    echo "error: the package was not produced" >&2
    exit 1
fi
cp "$BUILD_ZIP" "$OUT_ZIP"
echo "Package: $OUT_ZIP"
