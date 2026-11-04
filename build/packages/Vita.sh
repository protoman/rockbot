#!/usr/bin/env bash
# Builds the PS Vita package (CMake + VitaSDK) and copies the VPK.
# Usage: ./Vita.sh [1|2]     (asks which game if omitted)
# VITASDK selects the SDK (see ../../ports/vita/build.sh).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
OUT_VPK="$PWD/Rockbot${version_number}_Vita_$VERSIONNAME.vpk"

../../ports/vita/build.sh "$version_number"

BUILD_VPK="$SCRIPT_DIR/../../build-vita/Rockbot${version_number}.vpk"
if [ ! -f "$BUILD_VPK" ]; then
    echo "error: the VPK was not produced" >&2
    exit 1
fi
cp "$BUILD_VPK" "$OUT_VPK"
echo "Package: $OUT_VPK"
