#!/usr/bin/env bash
# Builds the PSP package (CMake + pspdev SDK) and zips it.
# Usage: ./PSP.sh [1|2]     (asks which game if omitted)
# PSPDEV selects the SDK (see ../../ports/psp/build.sh).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1
GAME_DIR=Rockbot$version_number
OUT_ZIP="$PWD/Rockbot${version_number}_PSP_$VERSIONNAME.zip"

# Configures, builds EBOOT.PBP and stages psp_dist/$GAME_DIR
../../ports/psp/build.sh "$version_number"

STAGE="psp_dist/$GAME_DIR"
find "$STAGE" \( -name '.svn' -o -name 'game*.sav' -o -name 'config*.sav' \) -prune -exec rm -rf {} +

rm -f "$OUT_ZIP"
( cd psp_dist && zip -qr "$OUT_ZIP" "$GAME_DIR" )
echo "Package: $OUT_ZIP"
