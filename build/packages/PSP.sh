#!/bin/bash
# Builds the PSP package (CMake + pspdev SDK) and zips it.
# Usage: ./PSP.sh [1|2]     (asks for the version if omitted)
# PSPDEV selects the SDK (see ../../ports/psp/build.sh).
set -euo pipefail
cd "$(dirname "$0")"

version_number="${1:-}"
if [ -z "$version_number" ]; then
    read -r -p "What is the version number you are building? " version_number
fi
if [ "$version_number" != "1" ] && [ "$version_number" != "2" ]; then
    echo "Invalid version number"
    exit 1
fi

VERSIONNAME=$(tr -d '[:space:]' < "version_name_v$version_number.txt")
GAME_DIR=Rockbot$version_number
OUT_ZIP="$PWD/Rockbot${version_number}_PSP_$VERSIONNAME.zip"

# Configures, builds EBOOT.PBP and stages psp_dist/$GAME_DIR
../../ports/psp/build.sh "$version_number"

STAGE="psp_dist/$GAME_DIR"
find "$STAGE" \( -name '.svn' -o -name 'game*.sav' -o -name 'config*.sav' \) -prune -exec rm -rf {} +

rm -f "$OUT_ZIP"
( cd psp_dist && zip -qr "$OUT_ZIP" "$GAME_DIR" )
echo "Package: $OUT_ZIP"
