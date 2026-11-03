#!/usr/bin/env bash
# Linux x86_64 tarball: builds natively (CMake, SDL2) and creates Rockbot<N>_Linux_x64_<version>.tar.gz
# Usage: ./linux.sh [1|2]     (asks which game if omitted)
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
source "$SCRIPT_DIR/common.sh"
choose_game "${1:-}" || exit 1

GAME_DIR="rockbot$version_number"
OUT="$PWD/Rockbot${version_number}_Linux_x64_$VERSIONNAME.tar.gz"

../../build-linux.sh sdl2

rm -rf ./linux
mkdir -p "./linux/$GAME_DIR"
cp ../rockbot "./linux/$GAME_DIR/rockbot"
stage_game_data "./linux/$GAME_DIR"

# The game looks for its data in the current directory, so start it through this launcher.
cat > "./linux/$GAME_DIR/rockbot.sh" <<'LAUNCHER'
#!/bin/sh
cd "$(dirname "$(readlink -f "$0")")" && exec ./rockbot "$@"
LAUNCHER
chmod +x "./linux/$GAME_DIR/rockbot.sh" "./linux/$GAME_DIR/rockbot"

cat > "./linux/$GAME_DIR/README.TXT" <<README
Rockbot $version_number $VERSIONNAME (Linux x86_64)

Run:   ./rockbot.sh
Needs: SDL2, SDL2_image, SDL2_mixer, SDL2_ttf and SDL2_gfx
       Debian/Ubuntu: sudo apt install libsdl2-2.0-0 libsdl2-image-2.0-0 libsdl2-mixer-2.0-0 libsdl2-ttf-2.0-0 libsdl2-gfx-1.0-0
Saves and settings are stored in ~/.rockdroid/
README

rm -f "$OUT"
tar -czf "$OUT" -C ./linux "$GAME_DIR"
echo "Package: $OUT"
